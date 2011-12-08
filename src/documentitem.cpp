/***************************************************************************
                         documentitem.cpp  -  description
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

#include "documentitem.h"
#include "tcanvasitem.h"

#include "labelutils.h"

#include <kapplication.h>

#include <qpen.h>
#include <qpainter.h>
#include <qdom.h>

DocumentItem::DocumentItem()
    : QObject(), XMLUtils()
{ 
    init();
}

void DocumentItem::init()
{
    m_canvasitem = NULL;
    m_token = NULL;
    m_device = KApplication::desktop();
    
    m_z = 0;    
    m_border = true;
    m_locked = false;
    m_pen = QPen( Qt::black, 1, Qt::SolidLine );
    m_rect = QRect( 0, 0, 0, 0 );
    m_visibilityScript = "true";
}

void DocumentItem::drawBorder(QPainter* painter)
{
    if( m_border )
    {
        painter->save();
        painter->setPen( m_pen );
        painter->drawRect( rect() );
        painter->restore();
    }
}

void DocumentItem::loadXML (QDomElement* element)
{
    LabelUtils l;
    m_rect.setX( element->attribute( "x_mm", "0" ).toInt() );
    m_rect.setY( element->attribute( "y_mm", "0" ).toInt() );
    m_rect.setWidth( element->attribute( "width_mm", "-1" ).toInt() );
    m_rect.setHeight( element->attribute( "height_mm", "-1" ).toInt() );
    
    m_pen.setColor( readXMLColor( element, "line-color", Qt::black ) );
    m_pen.setStyle( (Qt::PenStyle)element->attribute( "line-style", "0" ).toInt() );
    m_pen.setWidth( element->attribute( "line-width", "1" ).toInt() );    
    m_border = (bool)element->attribute( "line-visible", "1" ).toInt();
    m_z = element->attribute( "z", "0" ).toInt();
    m_locked = (bool)element->attribute( "locked", "0" ).toInt();
    m_visibilityScript = element->attribute( "visibilityScript", "true" );
}
        
void DocumentItem::saveXML (QDomElement* element)
{
    element->setAttribute( "x_mm", m_rect.x() );
    element->setAttribute( "y_mm", m_rect.y() );
    element->setAttribute( "width_mm", m_rect.width() );
    element->setAttribute( "height_mm", m_rect.height() );
    
    writeXMLColor( element, "line-color", m_pen.color() );
    element->setAttribute( "line-style", (Qt::PenStyle)m_pen.style() );
    element->setAttribute( "line-width", m_pen.width() );
    element->setAttribute( "line-visible", (int)m_border );
    element->setAttribute( "z", m_z );
    element->setAttribute( "locked", (int)m_locked );
    element->setAttribute( "visibilityScript", m_visibilityScript );
}

void DocumentItem::move( int x, int y )
{
    LabelUtils l;
    moveMM( (int)(l.pixelToMm( x, m_device, LabelUtils::DpiX ) * 1000.0 ),
            (int)(l.pixelToMm( y, m_device, LabelUtils::DpiY ) * 1000.0 ) );
}

void DocumentItem::moveMM( int x, int y )
{
// Move is not relative!
/*    m_rect.setX( m_rect.x() + x );
    m_rect.setY( m_rect.y() + y );*/
    m_rect = QRect( x, y, m_rect.width(), m_rect.height() );
}

void DocumentItem::setSize( int w, int h )
{
    LabelUtils l;
    setSizeMM( (int)(l.pixelToMm( w, m_device, LabelUtils::DpiX ) * 1000.0 ),
               (int)(l.pixelToMm( h, m_device, LabelUtils::DpiY ) * 1000.0 ) );
}

void DocumentItem::setSizeMM( int w, int h )
{
    m_rect.setWidth( w );
    m_rect.setHeight( h );
}

void DocumentItem::setBoundingRect( const QRect & rect )
{
    QRect r;
    LabelUtils l;
    r.setX( (int)(l.pixelToMm( rect.x(), m_device, LabelUtils::DpiX ) * 1000.0 ) );
    r.setY( (int)(l.pixelToMm( rect.y(), m_device, LabelUtils::DpiY ) * 1000.0 ) );
    r.setWidth( (int)(l.pixelToMm( rect.width(), m_device, LabelUtils::DpiX ) * 1000.0 ) );
    r.setHeight( (int)(l.pixelToMm( rect.height(), m_device, LabelUtils::DpiY ) * 1000.0 ) );

    // avoid resizing to negative values
    setRectMM( r.normalize() );
}

QRect DocumentItem::boundingRect() const
{
    QRect r;
    
    LabelUtils l;
    r.setX( (int)(l.mmToPixel( m_rect.x(), m_device, LabelUtils::DpiX ) / 1000.0 ) );
    r.setY( (int)(l.mmToPixel( m_rect.y(), m_device, LabelUtils::DpiY ) / 1000.0 ) );
    r.setWidth( (int)(l.mmToPixel( m_rect.width(), m_device, LabelUtils::DpiX ) / 1000.0 ) );
    r.setHeight( (int)(l.mmToPixel( m_rect.height(), m_device, LabelUtils::DpiY ) / 1000.0 ) );

//    qDebug("bounding rect %i %i %i %i", r.x(), r.y(), r.width(), r.height() );
        
    return r;
}

void DocumentItem::setRect( const QRect & rect)
{
    // avoid resizing to negative values
    QRect r = rect.normalize();
    setBoundingRect( QRect( r.x() - m_pen.width(), r.y() - m_pen.width(), r.width() + 2 * m_pen.width(), r.height() + 2 * m_pen.width() ) );
}

QRect DocumentItem::rect() const
{
    QRect bound = boundingRect();
    QRect r( bound.x() + m_pen.width(), bound.y() + m_pen.width(), bound.width() - 2 * m_pen.width(), bound.height() - 2 * m_pen.width() );
//    qDebug("rect %i %i %i %i", r.x(), r.y(), r.width(), r.height() );
    return r;
}

void DocumentItem::setRectMM( const QRect & rect )
{
    // avoid resizing to negative values
    m_rect = rect.normalize();
}

QRect DocumentItem::rectMM() const
{
    return m_rect;
}
    
void DocumentItem::setBorder (bool b) 
{
    m_border = b;    
}

bool DocumentItem::border ()  const 
{
    return m_border;    
}

void DocumentItem::setPen( const QPen & pen )
{
    m_pen = pen;
}

QPen DocumentItem::pen() const
{
    return m_pen;
}

void DocumentItem::setCanvasItem( TCanvasItem* item )
{
    m_canvasitem = item;
}

TCanvasItem* DocumentItem::canvasItem() const
{
    return m_canvasitem;
}

int DocumentItem::z() const
{
    return m_z;
}

void DocumentItem::setZ( int z )
{
    m_z = z;
}

/*void DocumentItem::setPaintDevice( QPaintDevice* device )*/// -!F: original, delete, 
void DocumentItem::setPaintDevice( QDesktopWidget* device )
{
    m_device = device;
}

/*QPaintDevice* DocumentItem::paintDevice() const*/// -!F: original, delete, 
QDesktopWidget* DocumentItem::paintDevice() const
{
    return m_device;
}

void DocumentItem::setTokenProvider( TokenProvider* token )
{
    m_token = token;
}

TokenProvider* DocumentItem::tokenProvider() const
{
    return m_token;
}
    
void DocumentItem::setLocked( bool locked )
{
    m_locked = locked;
}

const bool DocumentItem::locked() const
{
    return m_locked;
}

