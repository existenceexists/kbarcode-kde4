/***************************************************************************
                          dsrichtext.cpp  -  description
                             -------------------
    begin                : Fre Okt 17 2003
    copyright            : (C) 2003 by Dominik Seichter
    email                : domseichter@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "dsrichtext.h"

#ifdef QT_TEXT_BUG

// Qt includes
#include <qfontmetrics.h>
#include <qpainter.h>
#include <qregexp.h>
//Added by qt3to4:
#include <QList>

// for DSREPLACE
#include "sqltables.h"

/*
EXAMPLE TEXT:
    "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\"font-size:10pt;font-family:Nimbus Sans l\">"
    "<p>A little bit <span style=\"font-style:italic;font-weight:600\">formated</span><span style=\"color:#4400ff\"> "
    "richtext</span> which should be printed to a <span style=\"font-weight:600\">QPrinter</span> with <span style=\"text-decoration:underline\">QSimpleRichText</span>. </p>"
    "<p><span style=\"font-family:Times New Roman;font-size:16pt\">Printing should work in ScreenResolution as well as in HighResolution, but as you can see it will only work in ScreenResolution.</span></p>"
    "</body></html>";

Another example
<html><head><meta name="qrichtext" content="1" /></head><body style="font-size:10pt;font-family:Nimbus Sans l">
<p align="center"><span style="font-size:12pt;font-weight:600;text-decoration:underline">Text test </span><span style="font-size:12pt;font-weight:600;text-decoration:underline;color:#ff0000">label</span></p>
<p>Resolution <span style="font-weight:600">formated</span> 108dpi. This text &amp; text should now overlap with the first line. 4  &lt; 6a</p>
<p><span style="font-family:Nimbus Sans L;font-weight:600">A</span><span style="font-family:Nimbus Sans L"> longer paragraph with some text, so that I can test wether wordwrap is working. Another &quot;line&quot; to test line spacing without a bigger font (like the one found in the next sentence) messing up the spacing.</span><span style="font-family:Courier [Adobe];font-size:16pt"> Some text &quot;in&quot; a bigger courier font.</span><span style="font-family:Nimbus Sans L"> Also this paragraph should be justified as blockquote.</span><span style="font-family:Nimbus Sans L;color:#ff0004"> I hope it works</span><span style="font-family:Nimbus Sans L"> and KBarcodes text redering will be better than the one found in Qt.</span></p>
</body></html>

*/

DSRichText::DSRichText( const QString & t )
    : text( t )
{
    if( text.indexOf("<html>") == -1 )
        text = "<html><head><meta name=\"qrichtext\" content=\"1\" /></head><body style=\"font-size:10pt;font-family:Nimbus Sans l\"><p>"
               + t + "</p></body></html>";

    //qDebug( text );

    start = end = pos = 0;
    x = y = 0;
    sx = sy = 1.0;
    m_base = QFont();

    QString tmp = parse( text, "<body ", ">", 0 );
    if( !tmp.isNull() ) {
        pos += tmp.length();
        tmp = tmp.left( tmp.length() - 1 );
        tmp = tmp.mid( 5, tmp.length() - 5 );
        m_base = parseStyle( parse( tmp, "style=\"", "\"", pos ), &m_color );
    }

    pos = text.indexOf( "<p", 0 ); // initalize pos correctly
    while( parseParagraph() ); // empty while loop
}

DSRichText::~DSRichText()
{
}

void DSRichText::initFormat( formated_word* f, int alignment )
{
    f->text = QString::null;
    f->line = false;
    f->alignment = alignment;
    f->font = m_base;
    f->color = m_color;
}

void DSRichText::initLine( QList<formated_line>* l )
{
    formated_line li;
    li.width = 0;
    li.ascent = 0;
    li.leading = 0;
    li.lineSpacing = 0;
    li.line = false;
    l->append( li );
}

void DSRichText::updateSpacing( QList<formated_line>* l, QFontMetrics* fm )
{
    l->last().lineSpacing = (l->last().lineSpacing < fm->lineSpacing()) ? fm->lineSpacing() : l->last().lineSpacing;
    l->last().ascent = (l->last().ascent < fm->ascent()) ? fm->ascent() : l->last().ascent;
    l->last().leading = (l->last().leading < fm->leading()) ? fm->leading() : l->last().leading;
}

void DSRichText::fillLines()
{
    for( int z = 0; z < word_p.count(); z++ ) {
        WordList words = word_p[z];
        LineList lines;
        initLine( &lines );
        for( int i = 0; i < words.count(); i++ ) {
            formated_word word = words[i];
            lines.last().line = lines.last().line | word.line;

            QFontMetrics fm( word.font );
            updateSpacing( &lines, &fm );

            int tw = fm.width( word.text );
            // word does not fit in the current line, create a new one
            if( lines.last().width + tw >= w ) 
                initLine( &lines );

            // TODO: check here for words longer than one line
            lines.last().formats.append( word );
            lines.last().width += tw;
        }
        line_p.append( lines );
    }
}

void DSRichText::draw( QPainter* p )
{
    /* don't try to draw if there is no space to draw */
    if( !w || !h )
        return;
        
    fillLines();

    painter = p;
    painter->save();
    painter->setClipRect( xpos, ypos, int(w*sx), int(h*sy) );

    for( int z = 0; z < line_p.count(); z++ ) {
        LineList lines = line_p[z];
        if( lines.count() && z )
            y += int( lines[0].lineSpacing * 0.5);

        for( int i = 0; i < lines.count(); i++ ) {
            formated_line l = lines[i];
            
            if( l.formats.count() && l.formats[0].alignment == Qt::AlignJustify && i != lines.count() - 1 ) {
                // last line in a paragraph is not justified (in blocksatz mode)!
                drawJustified( &l );
            } else {
                for( int z = 0; z < l.formats.count(); z++ ) {
                    formated_word f = l.formats[z];
            
                    painter->setFont( f.font );
                    painter->setPen( QPen( f.color ) );

                    int a = f.alignment;
                    if( a == Qt::AlignRight ) {
                        a = Qt::AlignLeft;
                        if( !x )
                            x = w - l.width;
                    } else if( a == Qt::AlignHCenter ) {
                        a = Qt::AlignLeft;
                        if( !x )
                            x = ( w - l.width ) / 2;
                    } 

                    int ya = yDeviation( &l );
                    painter->drawText( xpos + int(x*sx), ypos + int((y+ya)*sy), int(l.width*sx), int(l.lineSpacing * sy), a, f.text );
                    x +=  painter->fontMetrics().width( f.text );
                }
            }

            x = 0;
            y += l.lineSpacing;
        }
    }

    painter->restore();
}

void DSRichText::drawJustified( formated_line* line )
{
    int all = 0;
    for( int z = 0; z < line->formats.count(); z++ ) {
        line->formats[z].text = line->formats[z].text.trimmed();
        QFontMetrics fm( line->formats[z].font );
        all += fm.width( line->formats[z].text );
    }

    int x = 0;
    int space = (w - all) / (line->formats.count() - 1);
    for( int z = 0; z < line->formats.count(); z++ ) {
        painter->setFont( line->formats[z].font );
        painter->setPen( QPen( line->formats[z].color ) );

        int ya = yDeviation( line );
        int tw = painter->fontMetrics().width(line->formats[z].text);
        painter->drawText( int(x*sx), int((y+ya)*sy), int(tw*sx), int(line->lineSpacing * sy), Qt::AlignLeft, line->formats[z].text );
        x += tw;
        x += space;
    }

}

bool DSRichText::parseParagraph()
{
    int alignment = 0;
    formated_word f;
    QString d = parse( text, "<p", "</p>", pos );
    //qDebug("D=" + d );
    //qDebug("POS=%i", pos );

    pos += d.length();
    if( d.isNull() )
        return false;

    d = parseParagraphTag( d, &alignment );

    WordList words;
    QString data;
    initFormat( &f, alignment );

    if( d.isEmpty() ) {
        // found empty paragraph
        words.append( f );
        word_p.append( words );
        return true;
    }

    for( int i = 0; i < d.length(); ) {
        if( d[i] == '<' || i == (d.length() - 1) ) {
            if( i == (d.length() - 1) )
                data.append( d[i] );

            parseWords( data, &f, &words );
            initFormat( &f, alignment );
            data = QString::null;

            // bail out now, otherwise there is
            // and endless loop for e.g. <p>a</p>
            if( i == (d.length() - 1) )
                break;

            if( d[i] == '<' ) {
                QString span = d.mid( i, d.indexOf( ">", i ) - i + 1 );
                i += span.length();

                if( span.startsWith( "<span " ) ) {
                    initFormat( &f, alignment );
                    f.font = parseStyle( parse( span, "style=\"", "\"", 0 ), &f.color );
                }
            }
        } else {
            data.append( d[i] );
            i++;
        }
    }

    word_p.append( words );
    return true;
}

QFont DSRichText::parseStyle( const QString & s, QColor* color )
{
    QString style = QString( s );

    style = style.left( style.length() - 1 );
    if( style.startsWith("style=\"" ) )
        style = style.mid( 7, style.length() - 7 );

    QFont f = m_base;
    *color = m_color;
    
    for ( int i = 0; i < style.count(';')+1; i++ ) {
        QString s = style.section( ';', i, i );
        if( s.isEmpty() )
            continue;
                    
        if ( s.startsWith("font-size:" ) ) {
            f.setPointSize( s.mid(10, s.length() - 12).toInt() );
        } else if ( s.startsWith("font-family:" )  ) {
            f.setFamily( s.right( s.length() - 12 ) );
        } else if( s.startsWith( "color:" ) ) {
            color->setNamedColor( s.right( s.length() - 6 ) );
        } else if( s.startsWith("text-decoration:") ) {
            if( s.endsWith( "underline" ) )
                f.setUnderline( true );
/*#if QT_VERSION >= 0x030200
       else if( s.endsWith( "overline" ) )
                f.setOverline( true );
#endif */
            else if( s.endsWith( "line-through" ) )
                f.setStrikeOut( true );
        } else if( s.startsWith( "font-style:") ) {
            if( s.endsWith( "italic" ) || s.endsWith( "oblique" ) )
                f.setItalic( true );
        } else if( s.startsWith( "font-weight:" ) ) {
            bool ok = false;
            int n = s.right( s.length() - 12 ).toInt( &ok );
            if( s.endsWith( "bold" ) )
                f.setBold( true );
            else if( ok )
                f.setWeight( n / 8 ); // convert CSS values to Qt values
        }
    }

    return f;
}

QString DSRichText::parse( const QString & t, const QString & find, const QString & find2, int start )
{
    int s = t.indexOf( find, start );
    if( s == -1 || s < start )
        return QString::null;

    int pend = t.indexOf( find2, s + find.length() );
    if( pend == -1 || pend < (signed int)(s + find.length()) )
        return QString::null;

    QString text = t.mid( s, pend - s + find2.length() );
    return text;
}

void DSRichText::setX( int x )
{
    xpos = x;
}

void DSRichText::setY( int y )
{
    ypos = y;
}

void DSRichText::setWidth( int width )
{
    w = width;
}

void DSRichText::setHeight( int height )
{
    h = height;
}

int DSRichText::parseAlignment( const QString & align )
{
    QString a = QString( align );
    if( a.endsWith("\"") )
        a = a.left( a.length() - 1 ); 

    if( a.startsWith("align=\"") )
        a = a.mid( 7, a.length() - 7 );

    if( a == "left" )
        return Qt::AlignLeft;
    else if( a == "center" )
        return Qt::AlignHCenter;
    else if( a == "right" )
        return Qt::AlignRight;
    else if( a == "justify" )
        return Qt::AlignJustify;

    return 0;
}

void DSRichText::parseWords( const QString & t, formated_word* w, WordList* words )
{
    int p = 0;
    for( int i = 0; i < t.length(); i++ ) {
        if( (t[i].isSpace() && p != i) || i == t.length() - 1 ) {
            formated_word word = *w;

            word.text = replaceEscapeSequences( t.mid( p, i + 1 - p ) );

            p = i + 1;
            words->append( word );
        }
    }
}

inline int DSRichText::yDeviation( const formated_line* line )
{
    // leading = 1 (almost ever)
    // linespacing = leading + height
    // height = ascent + descent + 1
    if( line->lineSpacing != painter->fontMetrics().lineSpacing() ) {
        return line->ascent + line->leading - painter->fontMetrics().ascent() - painter->fontMetrics().leading();
    } else {
        return 0;
    }
}

QString DSRichText::replaceEscapeSequences( const QString & t )
{
    QString tmp = QString( t );
    tmp = tmp.replace( DSREPLACE( "&lt;" ), "<" );
    tmp = tmp.replace( DSREPLACE( "&gt;" ), ">" );
    tmp = tmp.replace( DSREPLACE( "&amp;" ), "&" );
    tmp = tmp.replace( DSREPLACE( "&quot;" ), "\"" );
    return tmp;
}

QString DSRichText::parseParagraphTag( const QString &t, int* alignment )
{
    QString d = QString( t );
    if( d.startsWith("<p>") ) {
        d = d.mid( 3, d.length() - 3 );
    } else if( d.startsWith("<p ") ) {
        *alignment = parseAlignment( parse( d, "align=\"", "\"", 0 ) );
        if( d.contains( ">" ) ) {
            int x = d.indexOf(">" ) + 1;
            d = d.mid( x, d.length() - x );
        }
    }

    if( d.endsWith("</p>") )
        d = d.left( d.length() - 4 ); // strlen("</p>");

    return d;
}

#endif // QT_TEXT_BUG


