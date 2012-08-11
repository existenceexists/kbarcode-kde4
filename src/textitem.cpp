/***************************************************************************
                         textitem.cpp  -  description
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

#include "textitem.h"
#include "labelutils.h"
#include "labeleditor.h"
#include "tokenprovider.h"

// we need sin and cos
#include <math.h>

#include <qapplication.h>
#include <qdom.h>
#include <QPaintDevice>
#include <QX11Info>
#include <qpainter.h>
#include <QPicture>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

#include <kapplication.h>
#include <qmatrix.h>

TextItem::TextItem() 
    : DocumentItem()
{
    init();
}
    
void TextItem::init()
{
    m_text = QString::null;
    m_rotation = 0.0;

    setBorder( false );
    setRect( QRect( 0, 0, 100, 100 ) );
}

void TextItem::draw(QPainter* painter)
{
    bool autosize = false;
    const QString t = tokenProvider() ? tokenProvider()->parse( m_text ) : m_text;

    if( t != m_text )    
        autosize = true;
    
    QPaintDevice* device = DocumentItem::paintDevice();
    
    double scalex = (double)device->logicalDpiX() / (double)QX11Info::appDpiX();
    double scaley = (double)device->logicalDpiY() / (double)QX11Info::appDpiY();

    /*QColorGroup cg;*/// -!F: original, delete
    /*Q3SimpleRichText srt( t, painter->font() );*/// -!F: original
    QTextDocument srt;
    srt.setHtml( t );
    srt.setDefaultFont( painter->font() );
    srt.documentLayout()->setPaintDevice( device );// -!F: ?

    /*
    int width = (rect().width() < (int)((double)srt.widthUsed()*scalex) && autosize) ? srt.widthUsed() : rect().width();
    int height = (rect().height() < (int)((double)srt.height()*scaley) && autosize) ? srt.height() : rect().height();
    */

    int width = (int)((double)rect().width() / scalex);
    int height = (int)((double)rect().height() / scaley);
    
    painter->save();

    int w = rect().width();
    int h = rect().height();

    switch( (int)m_rotation ) 
    {
        default:
        case 0:
            /*painter->translate( rect().x(), rect().y() );*/// -!F: original
            break;
        case 90:
            /*painter->translate( rect().x() + w, rect().y() );*/// -!F: original
            painter->translate( w, 0 );
            qSwap( w, h );
            break;
        case 180:
            /*painter->translate( rect().x() + w, rect().y() + h );*/// -!F: original
            painter->translate( w, h );
            break;
        case 270:
            /*painter->translate( rect().x(), rect().y() + h );*/// -!F: original
            painter->translate( 0, h );
            qSwap( w, h );
            break;
    };

    painter->rotate( m_rotation );

    if( !TextItem::IsQtTextRenderingBroken() )
    {
        painter->setPen( Qt::black );
        /*srt.setWidth( painter, w );
        srt.draw( painter, 0, 0, QRect( 0, 0, w, h ), cg );*/// -!F: keep
        srt.setTextWidth( srt.size().width() );
        srt.drawContents( painter, QRect( 0, 0, w, h ) );
    }
    else
    {
        QRect r( 0, 0, width, height );
        QPicture picture;
        QPainter p( &picture );
        LabelUtils::renderString( &p, t, r, scalex, scaley );
        p.end();
        painter->drawPicture( 0, 0, picture );
    }
    
    painter->restore();
    
    DocumentItem::drawBorder( painter );    
}

void TextItem::drawPreview(QPainter* painter)
{
    bool autosize = false;
    const QString t = tokenProvider() ? tokenProvider()->parse( m_text ) : m_text;

    if( t != m_text )    
        autosize = true;
    
    QPaintDevice* device = DocumentItem::paintDevice();
    
    double scalex = (double)device->logicalDpiX() / (double)QX11Info::appDpiX();
    double scaley = (double)device->logicalDpiY() / (double)QX11Info::appDpiY();

    /*QColorGroup cg;*/// -!F: original, delete
    /*Q3SimpleRichText srt( t, painter->font() );*/// -!F: original
    QTextDocument srt;
    srt.setHtml( t );
    srt.setDefaultFont( painter->font() );
    srt.documentLayout()->setPaintDevice( device );// -!F: ?

    /*
    int width = (rect().width() < (int)((double)srt.widthUsed()*scalex) && autosize) ? srt.widthUsed() : rect().width();
    int height = (rect().height() < (int)((double)srt.height()*scaley) && autosize) ? srt.height() : rect().height();
    */

    int width = (int)((double)rect().width() / scalex);
    int height = (int)((double)rect().height() / scaley);
    
    painter->save();

    int w = rect().width();
    int h = rect().height();

    switch( (int)m_rotation ) 
    {
        default:
        case 0:
            painter->translate( rect().x(), rect().y() );
            break;
        case 90:
            painter->translate( rect().x() + w, rect().y() );
            qSwap( w, h );
            break;
        case 180:
            painter->translate( rect().x() + w, rect().y() + h );
            break;
        case 270:
            painter->translate( rect().x(), rect().y() + h );
            qSwap( w, h );
            break;
    };

    painter->rotate( m_rotation );

    if( !TextItem::IsQtTextRenderingBroken() )
    {
        painter->setPen( Qt::black );
        /*srt.setWidth( painter, w );
        srt.draw( painter, rect().x(), rect().y(), QRect( rect().x(), rect().y(), w, h ), cg );*/// -!F: keep
        //srt.setTextWidth( w );
        srt.drawContents( painter, QRect( 0, 0, w, h ));
    }
    else
    {
        QRect r( 0, 0, width, height );
        QPicture picture;
        QPainter p( &picture );
        LabelUtils::renderString( &p, t, r, scalex, scaley );
        p.end();
        painter->drawPicture( rect().x(), rect().y(), picture );
    }
    
    painter->restore();
    
    DocumentItem::drawBorderPreview( painter );    
}

void TextItem::drawZpl( QTextStream* stream )
{
    // simply remove all html tags....
    QString data = m_text.replace( QRegExp("<[^>]*>"), "" );
    *stream << ZPLUtils::fieldOrigin( rect().x(), rect().y() );
    *stream << ZPLUtils::font( QFont() ); // todo: select a valid font
    
    stream -> flush();
}

void TextItem::drawIpl( QTextStream* stream, IPLUtils* utils )
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

void TextItem::drawEPcl( QTextStream* stream )
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

void TextItem::loadXML (QDomElement* element)
{
    QDomNode n = element->firstChild();
    while( !n.isNull() ) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if( !e.isNull() )
            if( e.tagName() == "text" )
            {
                setText( e.text() );
                break;
            }
        n = n.nextSibling();
    }
    
    m_rotation = element->attribute("rotation", "0.0").toDouble();
    DocumentItem::loadXML( element );
}

void TextItem::saveXML (QDomElement* element)
{
    QDomElement texttag = element->ownerDocument().createElement( "text" );
    texttag.appendChild( element->ownerDocument().createTextNode( text() ) );
    element->appendChild( texttag );
    element->setAttribute( "rotation", m_rotation );
    
    DocumentItem::saveXML( element );
}

QString TextItem::text () const 
{
    return m_text;    
}

void TextItem::setText (const QString & text) 
{
    m_text = text;    
}

bool TextItem::s_qt_broken_init = false;
bool TextItem::s_qt_broken = false;

bool TextItem::IsQtTextRenderingBroken()
{
    if( !s_qt_broken_init )
    {
        QString version( qVersion() );
//        qDebug( "Detected Qt Version: %s", version.toLatin1() );
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
        
        if( s_qt_broken )
            qDebug("Qt's text redering is broken - using KBarcode fix.");
                
        s_qt_broken_init = true;
        return s_qt_broken;
    }
    else
        return s_qt_broken;
}

