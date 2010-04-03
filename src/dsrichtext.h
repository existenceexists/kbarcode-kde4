/***************************************************************************
                          dsrichtext.h  -  description
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

#ifndef DSRICHTEXT_H
#define DSRICHTEXT_H

#define QT_TEXT_BUG
/*#undef QT_TEXT_BUG*/
#ifdef QT_TEXT_BUG

#include <qcolor.h>
#include <qfont.h>
#include <qstring.h>
#include <QList>

class DSRichText;

/** This class represents one word,
  * formated with a font, a color and an
  * alignment.
  */
struct formated_word {
    QString text;
    QFont font;
    QColor color;
    int alignment;
    bool line;
};

typedef QList<formated_word> WordList;

/** This structure represents a single line.
  * Every line contains a list of formated_words.
  * Wordwrapping has been applied before the line
  * was created.
  * lineSpacing, ascent and leading come from the
  * biggest font in the line.
  */ 
struct formated_line {
    int width;
    int lineSpacing;
    int ascent;
    int leading;
    bool line;

    WordList formats;
};
typedef QList<formated_line> LineList;

class QColor;
class QPainter;

/** A richtext rendering class, which draws a piece
  * of HTML formated richtext on any QPainter.
  * This class works with all printer resolutions compared
  * to the buggy QSimpleRichText
  *
  * rendering the richtext happens in  3 steps:
  * <ul>
  * <li> the HTML data is parsed into text_format structures. Each
  *       of these text_format structures may represend multiple lines, but
  *       also single words. This depends on how long a formated text is.
  *       this step happens in the constructor and in parseParagraph()</li>
  * <li> the text_format structures are parsed into line structures.
  *       In this step the wordwrapping is calculated. Each line structure
  *       has again text_format structures. This is necessary because a line
  *       of text may of course contain different formattings. A line struct is always
  *       a single line of drawn text, not more and not less.
  *       happens in draw() and fillLines()</li>
  * <li> the line structure is drawn line by line using QPainter.
  *       happens in draw()</li>
  * </ul>
  *
  * Printing to the screen is easy and the usage does not differ from
  * QSimpleRichText. Drawing on QPrinter is a little bit more complicated.
  * The most important thing is that you may not use QPainter::scale().
  * A small example on how to print on a QPrinter:
  * <pre>
  *  QPrinter printer( QPrinter::HighResolution );
  *  QPainter painter( &printer );
  *  m_scalex = printer->logicalDpiX() / (double)QX11Info::appDpiX();
  *  m_scaley = printer->logicalDpiY() / (double)QX11Info::appDpiY();
  *
  *  DSRichText dsr( "&lt;html&gt;&lt;p&gt;Hallo World&lt;/p&gt;&lt;/html&gt;" );
  *  dsr.setWidth( 200 ); // in screenresolution coordinates
  *  dsr.setHeight( 80 );  // in screenresolution coordinates
  *  painter.translate( 10 * scalex, 10 * scaley ); // draw at 10, 10 instead of 0, 0
  *  dsr.scale( scalex, scaley );
  *  dsr.draw( &painter );
  * </pre>
  *
  *@author Dominik Seichter
  */
class DSRichText {
    public:
        /** construct a DSRichText object. @p t is formated text as produces by QTextEdit.
          */
        DSRichText( const QString & t );
        ~DSRichText();

        void setX( int x );
        void setY( int y );
        
        /** draw the formated text on the QPainter* @p p
          */
        void draw( QPainter* p );
        /** set the width of the clipping rectangle
          */
        void setWidth( int width );
        /** set the height of the clipping rectangle
          */
        void setHeight( int height );
        /** scale everything. This is necessary to print on devices
          * with another resolution than the screen. QPainter::scale() won't work.
          */
        void setScale( double x, double y ) {
            sx = x; sy = y;
        };

    private:
        /** parse a single pare of <p></p> elements
          */
        bool parseParagraph();
        /** remove <p> and </p> from the QString @p t and return it.
          * This function is also responsible for getting the
          * correct alignment of the paragraph.
          */
        QString parseParagraphTag( const QString & t, int* alignment );
        /** parse the align attribute of a <p> tag and return the corresponding Qt alignment value.
          */
        int parseAlignment( const QString & align );
        /** parse the css style attribute of a <span> tag and return a matching QFont for these
          * style. The font color is saved in the QColor* @p color.
          */
        QFont parseStyle( const QString & s, QColor* color );
        QString parse( const QString & t, const QString & find, const QString & end, int start );
        void parseWords( const QString & t, formated_word* w, WordList* words );
        void initFormat( formated_word* f, int alignment );
        void initLine( QList<formated_line>* l );
        void updateSpacing( QList<formated_line>* l, QFontMetrics* fm );
        /** draw the line @p line justified as blockquote
          */
        void drawJustified( formated_line* line );
        /** calculate the y-deviation needed, because of different font sizes in this line
          */
        inline int yDeviation( const formated_line* line );
        /** replace HTML escape sequences such as &lt; to their real character meaning (i.e. < )
          */
        QString replaceEscapeSequences( const QString & t );

        /** fill the line structures with data
          */
        void fillLines();

        int pos; // current position in text
        int start; // start of a tag
        int end; // end of a tag
        int x; // x position
        int y; // y position
        int w; // width of the text element
        int h; // height of the text element
        int xpos; // x position at the beginning
        int ypos; // y position at the beginning
        
        double sx;
        double sy;
        
        QString text;
        QFont m_base;
        QColor m_color;
        QPainter* painter;

        QList<LineList> line_p;
        QList<WordList> word_p;
};

#endif // QT_TEXT_BUG
#endif
