/***************************************************************************
                         textlineitem.cpp  -  description
                             -------------------
    begin                : Do Sep 2 2004
    copyright            : (C) 2004 by Dominik Seichter
    email                : domseichter@web.de
 ***************************************************************************/

/***************************************************************************

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

 ***************************************************************************/

#include "textlineitem.h"
#include "labelutils.h"
#include "tokenprovider.h"

#include <qapplication.h>
#include <qdom.h>
#include <QPaintDevice>
#include <QX11Info>
#include <qpainter.h>
#include <q3simplerichtext.h>

#include <kapplication.h>

TextLineItem::TextLineItem ()
    : DocumentItem()
{
    init();
}

void TextLineItem::init()
{
    m_text = QString::null;
    m_font = 0;
    m_rotate = 0;
    m_mag_vert = 1;
    m_mag_hor = 1;


    setBorder( false );
    setRect( QRect( 0, 0, 100, 20 ) );
}

/**
 *
 * @param painter
 */
void TextLineItem::draw(QPainter* painter)
{
    QString text = tokenProvider() ? tokenProvider()->parse( m_text ) : m_text;
    QColorGroup cg;
    Q3SimpleRichText srt( text, painter->font() );
    
    QPaintDevice* device = DocumentItem::paintDevice();
    double scalex = (double)device->logicalDpiX() / (double)QX11Info::appDpiX();
    double scaley = (double)device->logicalDpiY() / (double)QX11Info::appDpiY();
    int width = (rect().width() > 0) ? (int)((double)rect().width() / scalex) : srt.widthUsed();
    int height = (rect().height() > 0) ? (int)((double)rect().height() / scaley): srt.height();

    /*QRect r( (int)((double)rect().x() / scalex ), (int)((double)rect().y() / scaley), width, height );*/// -!F: original
    QRect r( 0, 0, width, height );

    srt.setWidth( painter, width );

    painter->save();

    if( !TextLineItem::IsQtTextRenderingBroken() )
    {
        painter->scale( scalex, scaley );
        painter->setPen( Qt::black );
        /*srt.draw( painter, (int)((double)rect().x() / scalex ), (int)((double)rect().y() / scaley), r, cg );*/// -!F: original
        srt.draw( painter, 0, 0, r, cg );
    }
    else
    {
        LabelUtils::renderString( painter, text, r, scalex, scaley );
    }

    painter->restore();

    DocumentItem::drawBorder( painter );
}

void TextLineItem::drawZpl( QTextStream* stream )
{
    // simply remove all html tags....
    QString data = m_text.replace( QRegExp("<[^>]*>"), "" );
    *stream << ZPLUtils::fieldOrigin( rect().x(), rect().y() );
    *stream << ZPLUtils::font( QFont() ); // todo: select a valid font

    stream -> flush();
}

void TextLineItem::drawIpl( QTextStream* stream, IPLUtils* utils )
{
    int counter = utils->counter();

    // simply remove all html tags....
    QString data = m_text.replace( QRegExp("<[^>]*>"), "" );

    QString s = QString("H%1;").arg( counter ); // field number

    s += utils->fieldOrigin( rect().x(), rect().y() );

    s += QString("c%1;").arg( 2 ); // font
    s += QString("h%1;").arg( 2 ); // vertical magnification ("height")
    s += QString("w%1;").arg( 2 ); // horicontyl magnification ("width")
    s += QString("d0,%1;").arg( data.length() ); // max length of data !

    *stream << utils->field( s );
    utils->addValue( data );

    stream -> flush();
}

void TextLineItem::drawEPcl( QTextStream* stream )
{
    // TODO: parse text field HTML

	// Need to wrap string if too long...
	//

	// break string into lines
    QString t = m_text.replace( QRegExp("<[^>]*>"), "" );
    // @DOM:
    // TODO:
    // provide an API which simplifies wordwrapping for
    // barcode printers
	/*QStringList lines = QStringList::split( "\n", t );*/// -!F: original, delete
	QStringList lines = t.split("\n");

	QStringList::Iterator line = lines.begin();
	for( int i=0; line != lines.end(); ++line, ++i ){
		QString s = QString("T %1").arg( rect().x() + 1 );
		s += QString(" %1 0 0 0 50 1").arg( rect().y()+50+1 + i*52 );
		s += QString(" %1").arg( *line );
		*stream << EPCLUtils::field( s );
	}

        stream -> flush();
}

void TextLineItem::loadXML (QDomElement* element)
{
    QDomNode n = element->firstChild();
    while( !n.isNull() ) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if( !e.isNull() )
            if( e.tagName() == "text" )
            {
                setFont(e.attribute( "font", "0").toInt() );
                setMagVert(e.attribute( "vert_mag", "1").toInt() );
                setMagHor(e.attribute( "hor_mag", "1").toInt() );
                setText( e.text() );
                break;
            }
        n = n.nextSibling();
    }

    DocumentItem::loadXML( element);
}

void TextLineItem::saveXML (QDomElement* element)
{

    QDomElement texttag = element->ownerDocument().createElement( "text" );
    texttag.appendChild( element->ownerDocument().createTextNode( text() ) );
    element->appendChild( texttag );
    element->setAttribute( "font", m_font );
    element->setAttribute( "hor_mag", m_mag_hor );
    element->setAttribute( "vert_mag", m_mag_vert );
    DocumentItem::saveXML( element);
}

QString TextLineItem::text () const
{
    return m_text;
}

void TextLineItem::setText (const QString & text)
{
    m_text = text;
}

void TextLineItem::setFont (int count)
{
    m_font = count;
}

int TextLineItem::getFont () const
{
    return m_font;
}

void TextLineItem::setMagVert (int count)
{
    m_mag_vert = count;
}

int TextLineItem::getMagVert () const
{
    return m_mag_vert;
}

void TextLineItem::setMagHor (int count)
{
    m_mag_hor = count;
}

int TextLineItem::getMagHor () const
{
    return m_mag_hor;
}

bool TextLineItem::s_qt_broken_init = false;
bool TextLineItem::s_qt_broken = false;

bool TextLineItem::IsQtTextRenderingBroken()
{
    if( !s_qt_broken_init )
    {
        QString version( qVersion() );
        int major = version.section( ".", 0, 0 ).toInt();
        int minor = version.section( ".", 1, 1 ).toInt();
        int bugfix = version.section( ".", 2, 2 ).toInt();

        if( major >= 3 )
        {
            if( major == 3 && minor < 3 )
                s_qt_broken = true;
            else if( major == 3 && minor == 3 )
            {
                if( bugfix < 2 )
                    s_qt_broken = true;
                else
                    s_qt_broken = false;
            }
            else
                s_qt_broken =  false;
        }
        else
            s_qt_broken = true;

        s_qt_broken_init = true;
        return s_qt_broken;
    }
    else
        return s_qt_broken;
}

