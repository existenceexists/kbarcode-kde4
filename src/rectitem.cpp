/***************************************************************************
                         rectitem.cpp  -  description
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

#include "rectitem.h"

#include <qdom.h>
#include <qpainter.h>

RectItem::RectItem()
    : DocumentItem()
{
    init();
}

void RectItem::loadXML (QDomElement* element)
{
    DocumentItem::loadXML( element );
    
    m_color = readXMLColor( element, "fill-color", Qt::white );
    m_filled = element->attribute( "filled", "0" ).toInt();
    m_circle = element->attribute( "circle", "0" ).toInt();
}

void RectItem::saveXML (QDomElement* element)
{
    DocumentItem::saveXML( element );
    
    writeXMLColor( element, "fill-color", m_color );
    element->setAttribute( "filled", m_filled );
    element->setAttribute( "circle", m_circle );
}

void RectItem::draw (QPainter* painter)
{
    if( m_circle )
    {
        if( m_filled )
        {
            painter->save();
            painter->setPen( QPen( Qt::NoPen ) );
            painter->setBrush( m_color );
            /*painter->drawEllipse( rect() );*/// -!F: original, delete
            painter->drawEllipse( QRect( 0, 0, rect().width(), rect().height() ) );
            painter->restore();
        }
    
        if( border() )
        {
            painter->save();
            painter->setPen( pen() );
            /*painter->drawEllipse( rect() );*/// -!F: original, delete
            painter->drawEllipse( QRect( 0, 0, rect().width(), rect().height() ) );
            painter->restore();
        }
    }
    else
    {
        if( m_filled ) {
            /*painter->fillRect( rect(), m_color );*/// -!F: original, delete
            painter->fillRect( QRect( 0, 0, rect().width(), rect().height() ), m_color );
        }
        
        DocumentItem::drawBorder( painter );
    }
}

void RectItem::drawZpl( QTextStream* stream )
{
    // TODO: refactor later and respect millimeters
    int thick = pen().width();
    
    *stream << ZPLUtils::fieldOrigin( rect().x(), rect().y() );
    if( m_circle )
        *stream << QString("~GE%1,%2,%3,B").arg( rect().width() ).arg( rect().height() ).arg( thick );
    else
        *stream << QString("~GB%1,%2,%3,B,0").arg( rect().width() ).arg( rect().height() ).arg( thick );

    stream -> flush();
}

void RectItem::drawIpl( QTextStream* stream, IPLUtils* utils )
{
    // TODO: refactor later and respect millimeters
    int thick = pen().width();
    
    if( m_circle )
        qDebug("Cirlce not implemented for IPL");    
    else
    {
        int counter = utils->counter();
        
        QString s = QString("W%1;").arg( counter ); // field number
    
        s += utils->fieldOrigin( rect().x(), rect().y() );
        s += QString("l%1;").arg( rect().width() ); // box length
        s += QString("h%1;").arg( rect().height() ); // box height
        s += QString("w%1;").arg( thick );
    
        *stream << utils->field( s );
    }
    
    stream -> flush();
}

void RectItem::drawEPcl( QTextStream* stream )
{
    QString s = QString("C %1").arg( rect().x()+1 );
    s += QString(" %1").arg( rect().y()+1 );
    s += QString(" %1").arg( rect().width() );
    s += QString(" %1").arg( rect().height() );
    s += QString(" %1 2").arg( pen().width() );

    *stream << EPCLUtils::field( s );
    
    stream -> flush();
}

void RectItem::init()
{
    m_color = Qt::white;
    m_filled = true;
    m_circle = false;
    setRect( QRect( 0, 0, 20, 20 ) );
}

void RectItem::setColor (const QColor & c) 
{
    m_color = c;    
}

QColor RectItem::color ()  const 
{
    return m_color;    
}

void RectItem::setFilled (bool f) 
{
    m_filled = f;    
}

bool RectItem::filled ()  const 
{
    return m_filled;    
}

void RectItem::setCircle (bool b) 
{
    m_circle = b;    
}

bool RectItem::circle ()  const 
{
    return m_circle;    
}

