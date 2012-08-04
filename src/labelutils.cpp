/***************************************************************************
                          labelutils.cpp  -  description
                             -------------------
    begin                : Sam Okt 26 2002
    copyright            : (C) 2002 by Dominik Seichter
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

#include "labelutils.h"
#include "dsrichtext.h" 

// Qt includes
#include <qbitmap.h>
#include <qimage.h>
#include <QPaintDevice>
#include <qpainter.h>
#include <QTextDocument>
//Added by qt3to4:
#include <QPixmap>

// KDE includes
#include <kapplication.h>
#include <kglobal.h>
#include <klocale.h>

// own includes
#include "printersettings.h"

#include <QDesktopWidget>// -!F: delete or keep ?

#define CONVERSION_FACTOR 25.4000508001016

LabelUtils::LabelUtils()
{
}

LabelUtils::~LabelUtils()
{ }

double LabelUtils::pixelToMm( double pixel, const QPaintDevice* device, int mode )
{
    if (device == 0)
    {
        /*device = (QPaintDevice *) KApplication::desktop();*/// -!F: keep
        device = KApplication::desktop();
    }
    
    if( mode == DpiX ) {
	return (pixel * CONVERSION_FACTOR) / (double)device->logicalDpiX();
    }
    else {
	return (pixel * CONVERSION_FACTOR) / (double)device->logicalDpiY();
    }
}

double LabelUtils::mmToPixel( double mm, const QPaintDevice* device, int mode )
{
    if( !mm )
        return 0;

    // We don't get valid metrics from the printer - and we want a better resolution
    // anyway (it's the PS driver that takes care of the printer resolution).

    if (device == 0)
    {
        /*device = (QPaintDevice *) KApplication::desktop();*/// -!F: keep
        device = KApplication::desktop();
    }
    
//    qDebug("DpiX=%i", pdm.logicalDpiX());
//    qDebug("DpiY=%i", pdm.logicalDpiY());
    if( mode == DpiX )
	return (mm / CONVERSION_FACTOR) * (double)device->logicalDpiX();
    else
	return (mm / CONVERSION_FACTOR) * (double)device->logicalDpiY();
}

double LabelUtils::pixelToPixelX( double unit, const QPaintDevice* src, const QPaintDevice* dest )
{
    return ( unit * (double)dest->logicalDpiX() ) / (double)src->logicalDpiX();
}

double LabelUtils::pixelToPixelY( double unit, const QPaintDevice* src, const QPaintDevice* dest )
{
    //return pixelToPixelX( unit, src, dest );
    return ( unit * (double)dest->logicalDpiY() ) / (double)src->logicalDpiY();
}

const QString LabelUtils::getTypeFromCaption( const QString & cap )
{
    // TODO: remove this function
    QString search = cap.right( cap.length() - cap.indexOf(":") - 1 ).toLower().trimmed();
    return search;
}

const QString LabelUtils::getModeFromCaption( const QString & cap )
{
    return cap.left( cap.indexOf(":") ).toLower().trimmed();
}

QSize LabelUtils::stringSize( const QString & t )
{
    /*Q3SimpleRichText srt( t, KApplication::font() );
    QSize s;
    s.setWidth( srt.widthUsed() );
    s.setHeight( srt.height() );*/
    QTextDocument srt;
    srt.setHtml( t );
    srt.setDefaultFont( KApplication::font() );
    
    QSize s;
    s.setWidth( srt.idealWidth() );
    s.setHeight( srt.size().height() );

    return s;
}

void LabelUtils::renderString( QPainter* painter, const QString & t, const QRect & rect, double scalex, double scaley )
{
    // DSRichText cannot calculate the width on its own
    /*Q3SimpleRichText srt( t, KApplication::font() );
    int width = (rect.width() > 0) ? rect.width() : srt.widthUsed();
    int height = (rect.height() > 0) ? rect.height() : srt.height();*/
    QTextDocument srt;
    srt.setHtml( t );
    srt.setDefaultFont( KApplication::font() );
    int width = (rect.width() > 0) ? rect.width() : srt.idealWidth();
    int height = (rect.height() > 0) ? rect.height() : srt.size().height();
    
    DSRichText r( t );
    r.setX( rect.x() );
    r.setY( rect.y() );
    r.setWidth( width );
    r.setHeight( height );
    r.setScale( scalex, scaley );
    r.draw( painter );    
}

QPixmap* LabelUtils::drawString( const QString & t, int w, int h, double rot )
{
    /*Q3SimpleRichText srt( t, KApplication::font() );

    int width = (w > 0) ? w : srt.widthUsed();
    int height = (h > 0) ? h : srt.height();
    srt.setWidth( width );*/
    QTextDocument srt;
    srt.setHtml( t );
    srt.setDefaultFont( KApplication::font() );
    
    int width = (w > 0) ? w : srt.idealWidth();
    int height = (h > 0) ? h : srt.size().height();
    srt.setTextWidth( width );
    
    QPixmap* pix;
    QPainter painter;
    if( rot == 0.0 ) {
        QBitmap bm( width, height );
        bm.fill( Qt::color0 ); //transparent
        painter.begin( &bm );
    
          painter.save();
            painter.setPen( Qt::color1 );
            /*QColorGroup cg;
            cg.setColor( QColorGroup::Foreground, Qt::color1 );
              cg.setColor( QColorGroup::Text, Qt::color1 );
              cg.setColor( QColorGroup::Base, Qt::color0 );*/// -!F: if the setting of colors turns out to be necessary then use a palette as shown at qt's porting4.html in Q3SimpleRichText's porting example.
    
            /*srt.draw( &painter, 0, 0, bm.rect(), cg );*/
            srt.drawContents( &painter, bm.rect() );
          painter.restore();
        painter.end();
        
        pix = new QPixmap( width, height );
        pix->fill( Qt::white );
        pix->setMask( bm );
            
        if( !pix->isNull() ) {
            painter.begin( pix );
            painter.setPen( Qt::black );
            QColorGroup cg;
            /*srt.draw( &painter, 0, 0, pix->rect(), cg );*/
            srt.drawContents( &painter, pix->rect() );
            painter.end();
        } 
    } else {
        /*int w2 = (w > 0) ? w : srt.widthUsed();
        int h2 = (h > 0) ? h : srt.height();*/
        int w2 = (w > 0) ? w : srt.idealWidth();
        int h2 = (h > 0) ? h : srt.size().height();

        QMatrix wm;
        wm.rotate( rot );

        QSize s = LabelUtils::stringSize( t );
        QPixmap* tmp = LabelUtils::drawString( t, s.width(), s.height() );

        QPixmap* p = new QPixmap( w2, h2 );
        p->fill( Qt::white );
        painter.begin( p );
        painter.drawPixmap( 0, 0, tmp->transformed( wm ) );
        painter.end();

        p->setMask( p->createHeuristicMask() );

        pix = p;
        delete tmp;
    }
    
    return pix;
}

