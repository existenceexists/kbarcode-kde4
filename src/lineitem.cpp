/***************************************************************************
                         lineitem.cpp  -  description
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

#include <qstring.h>

#include "documentitem.h"
#include "lineitem.h"

#include <qpainter.h>

LineItem::LineItem()
    : DocumentItem()
{
    init();
}

void LineItem::init()
{
    setRect( QRect( 0, 0, 20, 20 ) );
}

void LineItem::draw(QPainter* painter)
{
    painter->save();
    painter->setPen( pen() );
    painter->drawLine( rect().x(), rect().y(), rect().x() + rect().width(), rect().y() + rect().height() );
    painter->restore();
}

void LineItem::drawZpl( QTextStream* )
{
    qDebug("LineItem not implemented for ZPL");
}

void LineItem::drawEPcl( QTextStream* )
{
    qDebug("LineItem not implemented for EPCL");
}

void LineItem::drawIpl(  QTextStream*, IPLUtils* )
{
    qDebug("LineItem not implemented for IPL");
}

void LineItem::loadXML(QDomElement* element)
{
    DocumentItem::loadXML(element);
}

void LineItem::saveXML(QDomElement* element)
{
    DocumentItem::saveXML(element);
}
