/***************************************************************************
                          mycanvasview.cpp  -  description
                             -------------------
    begin                : Die Apr 23 2002
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

#include "mycanvasview.h"
#include "commands.h"
#include "definition.h"
#include "labeleditor.h"
#include "measurements.h"
#include "kbarcodesettings.h"

// Qt includes
#include <qcursor.h>
#include <qpainter.h>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QBrush>
#include <QRectF>
#include <QUndoCommand>
#include <QDebug>

// KDE includes
#include <kruler.h>
#include <kstatusbar.h>
#include <kundostack.h>

MyCanvas::MyCanvas( QObject* parent )
    : QGraphicsScene( parent )
{
    m_grid = false;

    setBackgroundBrush( QBrush( Qt::lightGray ) );
    
    setSceneRect( 0, 0, 0, 0 );// Resize width and height to be 0 
}

MyCanvas::~MyCanvas()
{
}

void MyCanvas::drawBackground( QPainter * painter, const QRectF & rect )
{
    QGraphicsScene::drawBackground( painter, rect );

    QRect shadow1( int(m_rect.x() + m_rect.width()), m_rect.y() + 5, 5, m_rect.height() );
    QRect shadow2( m_rect.x()+ 5, int(m_rect.y() + m_rect.height()), m_rect.width(), 5 );

    // draw background shadow
    if( rect.intersects( shadow1 ) )
        painter->fillRect( shadow1, Qt::black );

    if( rect.intersects( shadow2 ) )
        painter->fillRect( shadow2, Qt::black );

    if( rect.intersects( m_rect ) ) {
        painter->fillRect( m_rect, Qt::white );

        if( m_grid ) {
            painter->translate( m_rect.x(), m_rect.y() );

            painter->setPen( QPen( KBarcodeSettings::getInstance()->gridColor(), 0 ) );
	    for( int y = 0; y <= m_rect.height(); y += KBarcodeSettings::getInstance()->gridSize() ) 
		for( int x = 0; x <= m_rect.width(); x += KBarcodeSettings::getInstance()->gridSize() )
		    painter->drawPoint( x, y );

            painter->translate( -m_rect.x(), -m_rect.y() );
        }
    }
}

MyCanvasView::MyCanvasView( Definition* d, MyCanvas* c, QWidget* parent, Qt::WFlags f)
    : QGraphicsView( c, parent )
{
    setWindowFlags( f );
    
    statusbar = 0;
    m_additionOrder = 1;
    canv = c;
    compressedCommandIsInProgress = false;
    m_commandId = 0;

    rulerv = new KRuler( Qt::Vertical, this );
    rulerh = new KRuler( Qt::Horizontal, this );
    if( Measurements::measurementSystem() == Measurements::Metric ) {
        rulerv->setRulerMetricStyle( KRuler::Millimetres );
        rulerh->setRulerMetricStyle( KRuler::Millimetres );
    } else {
        rulerv->setRulerMetricStyle( KRuler::Inch );
        rulerh->setRulerMetricStyle( KRuler::Inch );
    }
    rulerv->setMaxValue( 0 );// KRuler::setMaxValue() is deprecated in KDE 4.6 but not in KDE 4.9 doc!, so the note "deprecated" assigned to this method is probably a bug?
    rulerh->setMaxValue( 0 );// KRuler::setMaxValue() is deprecated in KDE 4.6 but not in KDE 4.9 doc!, so the note "deprecated" assigned to this method is probably a bug?

    viewport()->setMouseTracking( true );
    setDefinition( d );
}

MyCanvasView::~MyCanvasView()
{
}

void MyCanvasView::snapPoint(QPoint * point, TCanvasItem* item )
{
    // move item to next grid position
    // TODO: align the top left to the grid not the current mouse position!

    /*
    */

    if( item )
    {
	int difx = point->x() - (int)item->x();
	int dify = point->y() - (int)item->y();

	int x = (int)item->x();
	int y = (int)item->y();

	int grid = KBarcodeSettings::getInstance()->gridSize();
	int modx = x % grid;
	int mody = y % grid;

        x -= modx;
	y -= mody;

	if(modx >= grid / 2)
	    x += grid;

	if(mody >= grid / 2)
	    y += grid;

	point->setX( x + difx );
	point->setY( y + dify );
    }
    else
    {
	point->setX(point->x() - point->x() % KBarcodeSettings::getInstance()->gridSize());
	point->setY(point->y() - point->y() % KBarcodeSettings::getInstance()->gridSize());
    }
}

void MyCanvasView::mouseMoveEvent(QMouseEvent* e)
{
    rulerh->slotNewValue( e->x() + rulerh->offset() );// offset is negative so we must use plus
    rulerv->slotNewValue( e->y() + rulerv->offset() );// offset is negative so we must use plus
    
    QPoint mappedEventPosition = mapToScene( e->pos() ).toPoint();

    if( statusbar ) {
        LabelUtils l;
        int x = (int)l.pixelToMm( mappedEventPosition.x(), this, LabelUtils::DpiX );
        int y = (int)l.pixelToMm( mappedEventPosition.y(), this, LabelUtils::DpiY );
        statusbar->changeItem( i18n("Position: ") +
                        QString( "%1%2 x %3%4" ).arg( x )
                        .arg( Measurements::system() ).arg( y ).arg( Measurements::system()), mouseid );
    }

    updateCursor( mappedEventPosition );

    // if no mouse button is pressed bail out now
    if( !(e->buttons() & Qt::LeftButton ) ) {
        (void)updateCursor( mappedEventPosition, true );
        return;
    }

    bool shift_pressed = e->modifiers() & Qt::ShiftModifier;

    TCanvasItem* moving = getActive();
    if( moving && !moving->item()->locked() ) {
        QPoint p = matrix().inverted().map(mappedEventPosition);
            
        if( !compressedCommandIsInProgress ) {
            compressedCommandIsInProgress = true;
        }
        
        if( m_mode == Barcode || m_mode == Inside ) {
            
            QPoint new_pt = QPoint(p.x() - delta_pt.x(),p.y() - delta_pt.y());
            if( canv->grid() ) {
                snapPoint(&new_pt, moving);
            }
            LabelUtils l;
            QPoint pmm;
            pmm.setX( (int)l.pixelToMm( new_pt.x() - getTranslation().x(), this, LabelUtils::DpiX ) * 1000 );
            pmm.setY( (int)l.pixelToMm( new_pt.y() - getTranslation().y(), this, LabelUtils::DpiY ) * 1000 );
            
            // Move the item(s)
            MoveMultipleCommand* mv = new MoveMultipleCommand( 
                pmm.x() - moving->item()->rectMM().x(), 
                pmm.y() - moving->item()->rectMM().y(), 
                getSelected(), 
                getCommandId() );
            history->push( mv );
            
        } else {
            
            if( canv->grid() ) {
                snapPoint(&p, NULL ) ;
            }
            
	    LabelUtils l;

	    QPoint pmm;
	    pmm.setX( (int)l.pixelToMm( p.x() - getTranslation().x(), this, LabelUtils::DpiX ) * 1000 );
	    pmm.setY( (int)l.pixelToMm( p.y() - getTranslation().y(), this, LabelUtils::DpiY ) * 1000 );

	    QRect rmm = moving->item()->rectMM();
            ResizeCommand* mv = new ResizeCommand( moving, shift_pressed, getCommandId() );

            switch( m_mode ) {
                case RightMiddle:
                    mv->setRect( rmm.x(), rmm.y(), pmm.x() - rmm.x(), rmm.height() );
                    break;
                case LeftMiddle:
                    mv->setRect( pmm.x(), rmm.y(),  rmm.width()+ (rmm.x() - pmm.x()), rmm.height() );
                    break;
                case BottomMiddle:
                    mv->setRect( rmm.x(), rmm.y(), rmm.width(), pmm.y() - rmm.y());
                    break;
                case TopMiddle:
                    mv->setRect( rmm.x(), pmm.y(), rmm.width(), rmm.height()+ (rmm.y() - pmm.y()));
                    break;
                case BottomLeft:
                    mv->setRect( pmm.x(), rmm.y(), rmm.width()+ (rmm.x() - pmm.x()), pmm.y() - rmm.y() );
                    break;
                case BottomRight:
                    mv->setRect( rmm.x(), rmm.y(), pmm.x() - rmm.x(), pmm.y() - rmm.y() );
                    break;
                case TopLeft:
                    mv->setRect( pmm.x(), pmm.y(), rmm.width()+ (rmm.x() - pmm.x()), rmm.height()+ (rmm.y() - pmm.y()));
                    break;
                case TopRight:
                    mv->setRect( rmm.x(), pmm.y(), pmm.x() - rmm.x(),rmm.height()+ (rmm.y() - pmm.y()) );
                    break;
                default:
                    break;
            }
            history->push( mv );
        }
        
        moving_start = p;

        emit movedSomething();
    }
}

void MyCanvasView::mousePressEvent(QMouseEvent* e)
{
    QPoint mappedEventPosition = mapToScene( e->pos() ).toPoint();

    setActive( scene()->itemAt( mappedEventPosition ), (e->modifiers() & Qt::ControlModifier) );

    if( getActive() ) {
        moving_start = matrix().inverted().map( mappedEventPosition );
        m_mode = updateCursor( mappedEventPosition );
        old = getActive()->pos();
        delta_pt=QPoint(mappedEventPosition.x() - old.x(),mappedEventPosition.y() - old.y());
    }

    if( e->button() == Qt::RightButton && getActive() )
        emit showContextMenu( e->globalPos() );
}

void MyCanvasView::mouseReleaseEvent(QMouseEvent* e)
{
    QPoint mappedEventPosition = mapToScene( e->pos() ).toPoint();
    
    if( e->button() != Qt::LeftButton || getSelected().isEmpty() )
        return;

    if( compressedCommandIsInProgress ) {
        compressedCommandIsInProgress = false;
        incrementCommandId();
    }

    updateCursor( mappedEventPosition );
}

int MyCanvasView::getCommandId()
{
    return m_commandId;
}

void MyCanvasView::incrementCommandId()
{
    m_commandId++;
}

void MyCanvasView::mouseDoubleClickEvent(QMouseEvent* e)
{
    QPoint mappedEventPosition = mapToScene( e->pos() ).toPoint();
    
    setActive( 0 );
    QList<QGraphicsItem *> list = scene()->items();
    for( int z = MyCanvasView::getHighestZ( list ); z >= MyCanvasView::getLowestZ( list ); z-- )    
        for( int i = 0; i < list.count(); i++ ) {
            if( list[i]->zValue() == z && isInside( mappedEventPosition, list[i] ) ) {
                setActive( list[i] );
                emit doubleClickedItem( getActive() );
                return;
            }
        }
}

bool MyCanvasView::isInside( QPoint p, QGraphicsItem* item )
{
    if( !item->isVisible() )
        return false;

    return item->contains( item->mapFromScene( p ) );
}

int MyCanvasView::isEdge( QPoint p, QGraphicsItem* item )
{
    if( !isInside( p, item ) )
        return Outside;

    QRectF rF = QRectF( item->pos().x(), item->pos().y(), item->boundingRect().width(), item->boundingRect().height() );
    QRect r = mapFromScene( rF ).boundingRect();
    
    p = mapFromScene( p );
    
    int rh = r.y() + r.height();
    int rw = r.x() + r.width();
    if( p.x() > r.x() && p.x() < r.x() + SPOTSIZE ) {
        // Left
        if( p.y() > r.y() && p.y() < r.y() + SPOTSIZE )
            return TopLeft;

        if( p.y() >  rh - SPOTSIZE && p.y() < rh )
            return BottomLeft;

        if( (r.height() - 2 * SPOTSIZE ) / 2 > SPOTSIZE ) {
            // Middle
            int start = ( r.y() + (r.height() - SPOTSIZE)/2 );
            if( p.y() >  start && p.y() < start + SPOTSIZE )
                return LeftMiddle;
        }

    }

    if( p.y() > r.y() && p.y() < r.y() + SPOTSIZE ) {
        // Top
        if( (r.width() - 2 * SPOTSIZE ) / 2 > SPOTSIZE ) {
            // Middle
            int start = ( r.x() + (r.width() - SPOTSIZE)/2 );
            if( p.x() >  start && p.x() < start + SPOTSIZE )
                return TopMiddle;
        }
    }

    if( p.y() > rh - SPOTSIZE && p.y() < rh ) {
        // Bottom
        if( (r.width() - 2 * SPOTSIZE ) / 2 > SPOTSIZE ) {
            // Middle
            int start = ( r.x() + (r.width() - SPOTSIZE)/2 );
            if( p.x() >  start && p.x() < start + SPOTSIZE )
                return BottomMiddle;
        }
    }

    if( p.x() > rw - SPOTSIZE && p.x() < rw ) {
        // Right
        if( p.y() > r.y() && p.y() < r.y() + SPOTSIZE )
            return TopRight;

        if( p.y() >  rh - SPOTSIZE && p.y() < rh )
            return BottomRight;

        if( (r.height() - 2 * SPOTSIZE ) / 2 > SPOTSIZE ) {
            // Middle
            int start = ( r.y() + (r.height() - SPOTSIZE)/2 );
            if( p.y() >  start && p.y() < start + SPOTSIZE )
                return RightMiddle;
        }
    }

    return Inside;
}

void MyCanvasView::deleteCurrent()
{
    TCanvasItemList list = getSelected();
    if( !list.isEmpty() ) {
        QUndoCommand* mc = new QUndoCommand( i18n("Delete") );

        for( int i = 0; i < list.count(); i++ ) {
            new DeleteCommand( list[i], mc );
        }
        
        history->push( mc );
        setActive( 0 );
        scene()->update();
    }
}

void MyCanvasView::setCurrent( QGraphicsItem* item )
{
    setSelected( item );
    setActive( item );
}

void MyCanvasView::updateRuler()
{
    rulerh->setGeometry( 20, 0, width() - 20, 20 );
    rulerv->setGeometry( 0, 20, 20, height() - 20 );

    if( def ) {
        canv->setRect( QRect( translation.x(), translation.y(), (int)def->getMeasurements().width( this ), (int)def->getMeasurements().height( this )) );

        rulerv->setMaxValue( height() );// KRuler::setMaxValue() is deprecated in KDE 4.6 but not in KDE 4.9 doc!, so the note "deprecated" assigned to this method is probably a bug?
        rulerh->setMaxValue( width() );// KRuler::setMaxValue() is deprecated in KDE 4.6 but not in KDE 4.9 doc!, so the note "deprecated" assigned to this method is probably a bug?

        if( Measurements::measurementSystem() == Measurements::Metric ) {
            rulerh->setPixelPerMark( (1/ 25.4)* logicalDpiX() );
            rulerv->setPixelPerMark( (1/25.4)* logicalDpiY() );
        } 
    }

}

void MyCanvasView::resizeEvent( QResizeEvent * r )
{
    setUpdatesEnabled( false );
    QPoint old = translation;

    QGraphicsView::resizeEvent( r );

    reposition();
    updateRuler();

    repaint();

    old = translation - old;
    QList<QGraphicsItem *> list = scene()->items();
    for( int i = 0; i < list.count(); i++ )
        list[i]->moveBy( old.x(), old.y() );

    setUpdatesEnabled( true );
}

void MyCanvasView::reposition()
{
    /*
     * it is difficult to handle the repositioning
     * when a new scaling factor has been set.
     * Therefore we divide by the old factor
     * and multiply with the new one.
     * As a result we have the correct width.
     */
    int x = int((width() - (def->getMeasurements().width( this ))  ) / 2);
    int y = int((height() - (def->getMeasurements().height( this )) ) / 2);

    // move label 30 pixels away from top/left if necessary
    x = x > 0 ? x : 30;
    y = y > 0 ? y : 30;

    // make sure that there is some space around the label, therefore at 60 pixel
    int w = ( this->width() - 2 > def->getMeasurements().width( this ) ? this->width() - 2 : int(def->getMeasurements().width( this ) + 60) );
    int h = ( this->height() - 2 > def->getMeasurements().height( this ) ? this->height() - 2 : int(def->getMeasurements().height( this ) + 60) );

    scene()->setSceneRect( 0, 0, w, h );
    
    
    translation = QPoint( x, y );
}

void MyCanvasView::setDefinition( Definition* d )
{
    def = d;
    reposition();
    updateRuler();
    repaint();
}

void MyCanvasView::selectAll()
{
    QList<QGraphicsItem *> list = scene()->items();
    for( int i = 0; i < list.count(); i++ )
        setSelected( list[i], true );
}

void MyCanvasView::deSelectAll()
{
    setSelected( 0 );
}

int MyCanvasView::getLowestZ( QList<QGraphicsItem *> list )
{
    int v = 0;
    for( int i = 0; i < list.count(); i++ )
        if( list[i]->zValue() < v )
            v = (int)list[i]->zValue();

    return v;
}

int MyCanvasView::getHighestZ( QList<QGraphicsItem *> list )
{
    int v = 0;
    for( int i = 0; i < list.count(); i++ )
        if( list[i]->zValue() > v )
            v = (int)list[i]->zValue();

    return v;
}

TCanvasItem* MyCanvasView::getActive()
{
    QList<QGraphicsItem *> list = scene()->items();
    for( int i = 0; i < list.count(); i++ )
        if( ((TCanvasItem*)list[i])->isActiveItem() )
            return (TCanvasItem*)list[i];

    return 0;
}

void MyCanvasView::setActive( QGraphicsItem* item, bool control )
{
    emit selectionChanged();
    QList<QGraphicsItem *> list = scene()->items();
    for( int i = 0; i < list.count(); i++ )
        ((TCanvasItem*)list[i])->setActiveItem( false );

    if( item )
        ((TCanvasItem*)item)->setActiveItem( true );

    setSelected( item, control );
}

int MyCanvasView::additionOrder()
{
    return m_additionOrder;
}

void MyCanvasView::incrementAdditionOrder()
{
    ++m_additionOrder;
}

DocumentItemList MyCanvasView::getAllItems()
{
    DocumentItemList l;

    QList<QGraphicsItem *> list = scene()->items();
    for( int i = 0; i < list.count(); i++ )
	l.append( ((TCanvasItem*)list[i])->item() );

    return l;
}

TCanvasItemList MyCanvasView::getSelected()
{
    TCanvasItemList l;
    QList<QGraphicsItem *> list = scene()->items();
    for( int i = 0; i < list.count(); i++ )
        if( list[i]->isSelected() )
            l.append( (TCanvasItem*)list[i] );

    return l;
}

void MyCanvasView::setSelected( QGraphicsItem* item, bool control )
{
    if( !control ) {
        QList<QGraphicsItem *> list = scene()->items();
        for( int i = 0; i < list.count(); i++ )
            list[i]->setSelected( false );
    }

    if( item )
        item->setSelected( true );
    
    ((LabelEditor*)parent())->enablePropertiesAndDelete();
}

int MyCanvasView::updateCursor( QPoint pos, bool pressed )
{
    if( !getActive() ) {
        this->setCursor( Qt::ArrowCursor );
        return -1;
    }
    
    int mode = isEdge( pos, getActive() );

    if( getActive()->item()->locked() )
    {
        pressed && mode != Outside ? setCursor( Qt::ForbiddenCursor ) : setCursor( Qt::ArrowCursor );
        return mode;        
    }

    if( getActive()->rtti() == eRtti_Barcode ) {
        pressed && mode != Outside ? setCursor( Qt::SizeAllCursor ) : setCursor( Qt::ArrowCursor );
        mode = Barcode;
        return mode;
    }

    switch( mode ) {
        case TopLeft:
        case BottomRight:
            this->setCursor( Qt::SizeFDiagCursor );
            break;
        case TopMiddle:
        case BottomMiddle:
            this->setCursor( Qt::SizeVerCursor );
            break;
        case TopRight:
        case BottomLeft:
            this->setCursor( Qt::SizeBDiagCursor );
            break;
        case RightMiddle:
        case LeftMiddle:
            this->setCursor( Qt::SizeHorCursor );
            break;
        case Inside:
            pressed ? setCursor( Qt::SizeAllCursor ) : setCursor( Qt::ArrowCursor );
            break;
        case Outside:
        default:
            this->setCursor( Qt::ArrowCursor );
            break;
    };

    return mode;
}


#include "mycanvasview.moc"
