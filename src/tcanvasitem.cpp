/***************************************************************************
                         tcanvasitem.cpp  -  description
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

#include "tcanvasitem.h"
#include "mycanvasitem.h"
#include "mycanvasview.h"
#include "imageitem.h"

#include <qpainter.h>
//Added by qt3to4:
#include <QPixmap>
#include <labelutils.h>

TCanvasItem::TCanvasItem( MyCanvasView* cv )
    : QGraphicsRectItem(),
      ReferenceCounted()
{
    setFlag ( QGraphicsItem::ItemIsSelectable, true );
    m_view = cv;
    m_item = NULL;
    m_activeItem = false;
    show();
}

TCanvasItem::~TCanvasItem()
{
    if( m_item )
        delete m_item;
}

void TCanvasItem::setZ( double z )
{
    QGraphicsRectItem::setZValue( z );

    if( m_item )
        m_item->setZ( (int)z );
}

void TCanvasItem::setSize( int width, int height )
{
    prepareGeometryChange();
    QGraphicsRectItem::setRect( 0, 0, width, height );
    
    if( m_item )
    {
        m_item->setSize( width, height );
        
        if( m_item->rtti() == eRtti_Image )
        {
            ImageItem* img = static_cast<ImageItem*>(m_item);
            img->updateImage();
        }
        
        update();
    }
}

void TCanvasItem::setSizeMM( int w, int h )
{
    if( m_item )
    {
        m_item->setSizeMM( w, h );
        prepareGeometryChange();
        QGraphicsRectItem::setRect( 0, 0, m_item->boundingRect().width(), m_item->boundingRect().height() );
    }
}

void TCanvasItem::moveBy( double dx, double dy )
{
    QGraphicsRectItem::moveBy( dx, dy );
    
    if( m_item )
    {
        m_item->move( (int)x() - m_view->getTranslation().x(), (int)y() - m_view->getTranslation().y() );
        update();
    }
}

void TCanvasItem::moveByMM( int x, int y )
{
    moveMM( m_item->rectMM().x() + x, m_item->rectMM().y() + y );
}

void TCanvasItem::moveMM( int x, int y )
{
    if( m_item )
    {
	LabelUtils l;
	// the QCanvasRectangle::move is done before the real move
        // as we get dancing TCanvasItems on the screen otherwise
        QGraphicsRectItem::setPos( l.mmToPixel( x / 1000.0, m_view, LabelUtils::DpiX ) + m_view->getTranslation().x(), 
				l.mmToPixel( y / 1000.0, m_view, LabelUtils::DpiY ) + m_view->getTranslation().y() );
        m_item->moveMM( x, y );
    }
}

void TCanvasItem::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{   
    painter->save();
    
    if( m_item )
    {
        if( zValue() != (int)m_item->z() )
            QGraphicsRectItem::setZValue( m_item->z() );
        
        painter->save();
	painter->setClipRect( boundingRect() );
        m_item->draw( painter );
        painter->restore();
    }
    
    // draw edges
    if( isSelected() ) {
        const QPixmap* spot = SpotProvider::getInstance()->spot();

        // top left
        painter->drawPixmap( 0, 0, *spot );
        // bottom left
        painter->drawPixmap( 0, rect().height()-SPOTSIZE, *spot );
        // top right
        painter->drawPixmap( rect().width()-SPOTSIZE, 0, *spot );
        // bottom left
        painter->drawPixmap( rect().width()-SPOTSIZE, rect().height()-SPOTSIZE, *spot );

        if( (rect().width() - 2 * SPOTSIZE ) / 2 > SPOTSIZE ) {
            // top middle
            painter->drawPixmap( (rect().width()-SPOTSIZE)/2, 0, *spot );
            // bottom middle
            painter->drawPixmap( (rect().width()-SPOTSIZE)/2, rect().height()-SPOTSIZE, *spot );
        }

        if( (rect().height() - 2 * SPOTSIZE ) / 2 > SPOTSIZE ) {
            // left middle
            painter->drawPixmap( 0, (rect().height()-SPOTSIZE)/2, *spot );
            // right middle
            painter->drawPixmap( rect().width() - SPOTSIZE, (rect().height()-SPOTSIZE)/2, *spot );
        }
    }
    painter->restore();
}

void TCanvasItem::setItem (DocumentItem* item) 
{
    if( m_item )
        m_item->setCanvasItem( NULL );
        
    m_item = item;    
    
    if( m_item )
    {
        m_item->setCanvasItem( this );
        this->setZValue( m_item->z() );

        prepareGeometryChange();
        QGraphicsRectItem::setRect( 0, 0, m_item->boundingRect().width(), m_item->boundingRect().height() );
        QGraphicsRectItem::setPos( m_item->boundingRect().x() + m_view->getTranslation().x(), m_item->boundingRect().y() + m_view->getTranslation().y() );
        update();
    }
}

DocumentItem* TCanvasItem::item () const
{
    return m_item;
}

void TCanvasItem::update()
{
    QGraphicsRectItem::update();
}

int TCanvasItem::rtti() const
{
    if( m_item )
        return m_item->rtti();
    else
        return 0;
}

MyCanvasView* TCanvasItem::canvasView() const
{
    return m_view;
}



void TCanvasItem::show()
{
    if (!isVisible())
    {
        this->addRef();
        QGraphicsRectItem::show();
    }
}

void TCanvasItem::hide()
{
    if (refCount()==1)
    {
        if (isVisible())
        {
            this->remRef();
        }
    }
    else
    {
        if (isVisible())
        {
            ((QGraphicsItem*) this)->hide(); 
            this->remRef();
        }
    }
}

bool TCanvasItem::isActiveItem()
{
    return m_activeItem;
}

void TCanvasItem::setActiveItem( bool makeActive )
{
    m_activeItem = makeActive;
}
