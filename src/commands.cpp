/***************************************************************************
                          commands.cpp  -  description
                             -------------------
    begin                : Don Dez 19 2002
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

#include "commands.h"
#include "mycanvasview.h"

#include "barkode.h"
#include "tcanvasitem.h"
#include "rectitem.h"
#include "textitem.h"
#include "imageitem.h"
#include "barcodeitem.h"
#include "lineitem.h"
//NY26
#include "textlineitem.h"
//NY26

#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QList>

// KDE includes
#include <kapplication.h>
//Added by qt3to4:
#include <QPixmap>
#include <krandom.h>

QPoint getFreePos( QGraphicsScene* c )
{
    MyCanvas* canvas = (MyCanvas*)c;
    
    if( !c->width() && !c->height() )
        return QPoint( canvas->rect().x(), canvas->rect().y() );

    // TODO: fix for positions on label        
    int x = KRandom::random() % canvas->rect().width() - 20;
    int y = KRandom::random() % canvas->rect().height() - 20;

    if( x > 0 && y > 0 )
        return QPoint( canvas->rect().x() + x, canvas->rect().y() + y );
    else
        return QPoint( canvas->rect().x(), canvas->rect().y() );
}
        
CommandUtils::CommandUtils(TCanvasItem* item)
    : QObject()
{
    m_canvas_item = item;
    m_canvas_item->addRef();

    c = m_canvas_item->scene();
    /* NOT NEEDED:
    if( m_canvas_item && m_canvas_item->item() )
        connect( m_canvas_item->item(), SIGNAL( destroyed() ), this, SLOT( documentItemDeleted() ) );
    */
}

CommandUtils::~CommandUtils()
{
    m_canvas_item->remRef();
}

bool CommandUtils::canvasHasItem()
{
    if( m_canvas_item && c )
    {
        QList<QGraphicsItem *> list = c->items();
        for( int i=0;i<list.count();i++)
            if( m_canvas_item == list[i] )
                return true;
    }
    
    return false;
}


void CommandUtils::documentItemDeleted()
{
    /** the document item got deleted, so that we can assume the TCanvasItem 
      * was deleted to (as it usually deltes the document item)
      */
    /* NOT NEEDED
    m_canvas_item = NULL;
    c = NULL;
    
    qDebug("Document item removed from list!");
    delete this;
    */
}

NewItemCommand::NewItemCommand( MyCanvasView* view, const QString & name, QUndoCommand* parent )
    : QObject(), QUndoCommand( name + "\n" + name, parent )
{
    cv = view;
    m_name = name;
    m_point = getFreePos( cv->scene() );
    m_item = NULL;
    m_object = NULL;
}

NewItemCommand::~NewItemCommand()
{
    m_item->remRef();
}

void NewItemCommand::redo()
{
    if( !m_item )
    {
        create();

        if( m_object )
        {
            m_item = new TCanvasItem( cv );
            m_item->setItem( m_object );
            m_object->setAdditionOrder( cv->additionOrder() );
            cv->incrementAdditionOrder();
            /*m_item->setPos( m_point.x(), m_point.y() );*/// setPos() must be called after item is added to scene
            m_object->move( m_point.x() - cv->getTranslation().x(), m_point.y() - cv->getTranslation().y() );
	    m_item->addRef();

	    /* NOT NEEDED:
            connect( m_object, SIGNAL( destroyed() ), this, SLOT( documentItemDeleted() ) );
	    */
        }
    }

    if( m_item )
    {
        cv->scene()->addItem( m_item );
        m_item->setPos( m_point.x(), m_point.y() );// setPos() must be called after item is added to scene
        m_item->show();
        m_item->update();
        cv->setCurrent( m_item );
    }
}

void NewItemCommand::undo()
{
    if( m_item ) {
        /*m_item->hide();*/// removing an item that is hidden causes a runtime error
        cv->scene()->removeItem( m_item );
    }
}

void NewItemCommand::documentItemDeleted()
{
    /** the document item got deleted, so that we can assume the TCanvasItem 
      * was deleted to (as it usually deltes the document item)
      */

    /* NOT NEEDED:
    m_item = NULL;
    m_object = NULL;
    
    qDebug("NewItemCommand: Document item removed from list!");
    delete this;
    */
}


void ResizeCommand::setRect( int cx, int cy, int cw, int ch )
{
    // make sure that the item cannot 
    // be resized to a negative value
    if( cw <= 0 )
	cw = orect.width();

    if( ch <= 0 )
	ch = orect.height();

    rect = QRect( cx, cy, cw, ch );
}

void ResizeCommand::redo()
{
    if( canvasHasItem() ) {
        if( m_shift && rect.width() ) {
            double r = (double)orect.height() / (double)orect.width();
            rect.setWidth( int(rect.height() / r) );
        }

	m_canvas_item->moveMM( rect.x(), rect.y() );
	m_canvas_item->setSizeMM( rect.width(), rect.height() );
        m_canvas_item->update();

	if( m_canvas_item->item()->rtti() == eRtti_Image )
	{
	    ImageItem* item = static_cast<ImageItem*>(m_canvas_item->item());
	    item->updateImage();
	}
    }
}

void ResizeCommand::undo()
{
    if( canvasHasItem() ) {
	m_canvas_item->moveMM( orect.x(), orect.y() );
	m_canvas_item->setSizeMM( orect.width(), orect.height() );
        m_canvas_item->update();        

	if( m_canvas_item->item()->rtti() == eRtti_Image )
	{
	    ImageItem* item = static_cast<ImageItem*>(m_canvas_item->item());
	    item->updateImage();
	}
    }
}

bool ResizeCommand::mergeWith( const QUndoCommand* other )
{
    if( other->id() != id() ) {
        return false;
    }
    rect = static_cast<const ResizeCommand*>(other)->rect;
    return true;
}

void MoveCommand::redo()
{
    if( canvasHasItem() )
    {
	m_canvas_item->moveByMM( x, y );
        m_canvas_item->update();        
    }
}

void MoveCommand::undo()
{
    if( canvasHasItem() ) 
    {
	m_canvas_item->moveByMM( -x, -y );
	m_canvas_item->update();        
    }
}

bool MoveCommand::mergeWith( const QUndoCommand* other )
{
    if( other->id() != id() ) {
        return false;
    }
    x += static_cast<const MoveCommand*>(other)->x;
    y += static_cast<const MoveCommand*>(other)->y;
    return true;
}

MoveMultipleCommand::MoveMultipleCommand( int cx, int cy, TCanvasItemList list, int id, QUndoCommand* parent )
    : QUndoCommand( parent )
{
    x = cx;
    y = cy;
    m_id = id;
    setText( name() + "\n" + name() );
    m_items_list = new TCanvasItemList;
    for( int i = 0; i < list.count(); i++ ) {
        m_items_list->append( list[i] );
        list[i]->addRef();
    }
    c = m_items_list->last()->scene();
}

MoveMultipleCommand::~MoveMultipleCommand() 
{
    if( m_items_list ) {
        if( !m_items_list->isEmpty() ) {
            for( int i = 0; i < m_items_list->count(); i++ ) {
                (*m_items_list)[i]->remRef();
            }
        }
        delete m_items_list;
        m_items_list = NULL;
    }
}

bool MoveMultipleCommand::canvasHasItem( TCanvasItem* item )
{
    if( item && c )
    {
        QList<QGraphicsItem *> list = c->items();
        for( int i=0;i<list.count();i++)
            if( item == list[i] )
                return true;
    }
    
    return false;
}

void MoveMultipleCommand::redo()
{
    if( m_items_list && !m_items_list->isEmpty() ) {
        for( int i = 0; i < m_items_list->count(); i++ ) {
            if( canvasHasItem( (*m_items_list)[i] ) )
            {
                (*m_items_list)[i]->moveByMM( x, y );
                (*m_items_list)[i]->update();
            }
        }
    }
}

void MoveMultipleCommand::undo()
{
    if( m_items_list && !m_items_list->isEmpty() ) {
        for( int i = 0; i < m_items_list->count(); i++ ) {
            if( canvasHasItem( (*m_items_list)[i] ) )
            {
                (*m_items_list)[i]->moveByMM( -x, -y );
                (*m_items_list)[i]->update();
            }
        }
    }
}

bool MoveMultipleCommand::mergeWith( const QUndoCommand* other )
{
    if( other->id() != id() ) {
        return false;
    }
    x += static_cast<const MoveMultipleCommand*>(other)->x;
    y += static_cast<const MoveMultipleCommand*>(other)->y;
    return true;
}

ChangeZCommand::ChangeZCommand( int z, TCanvasItem* it )
    : CommandUtils( it )
{
    m_z = z;
    m_oldz = (int)m_canvas_item->zValue();
    setText( name() + "\n" + name() );
}

void ChangeZCommand::redo()
{
   if( canvasHasItem() )
    {
        m_canvas_item->setZ( m_z );
        m_canvas_item->update();
    }
}

void ChangeZCommand::undo()
{
   if( canvasHasItem() )
    {
        m_canvas_item->setZ( m_oldz );
        m_canvas_item->update();
    }
}

void LockCommand::redo()
{
    if( canvasHasItem() )
    {
        m_canvas_item->item()->setLocked( m_locked );
        m_canvas_item->update();
    }
}

void LockCommand::undo()
{
    if( canvasHasItem() )
    {
        m_canvas_item->item()->setLocked( !m_locked );
        m_canvas_item->update();
    }
}

PictureCommand::PictureCommand( double r, bool mh, bool mv, EImageScaling s, ImageItem* it, QUndoCommand* parent ) 
    : QUndoCommand( parent ), CommandUtils( it->canvasItem() )
{
    rotate = r;
    mirrorv = mv;
    mirrorh = mh;
    scaling = s;
    
    orotate = it->rotation();
    omirrorv = it->mirrorVertical();
    omirrorh = it->mirrorHorizontal();
    oscaling = it->scaling();
    
    oexpression = it->expression();
    opixmap = it->pixmap();
    opixserial = opixmap.serialNumber();

    oldsize.setWidth( it->rect().width() );
    oldsize.setHeight( it->rect().height() );

    m_item = it;
    setText( name() + "\n" + name() );
}

void PictureCommand::setExpression( const QString & expr )
{
    expression = expr;
}

void PictureCommand::setPixmap( const QPixmap & pix )
{
    pixmap = pix;
    pixserial = pixmap.serialNumber();
}

void PictureCommand::redo()
{
    if( canvasHasItem() ) {
        m_item->setRotation( rotate );
        m_item->setMirrorVertical( mirrorv );
        m_item->setMirrorHorizontal( mirrorh );
        m_item->setScaling( scaling );
	m_item->setExpression( expression );
	m_item->setPixmap( pixmap );

	if( !pixmap.isNull() && pixserial != opixserial )
	    m_item->canvasItem()->setSize( pixmap.width(), pixmap.height() );
    }
}

void PictureCommand::undo()
{
    if( canvasHasItem() ) {
        m_item->setRotation( orotate );
        m_item->setMirrorVertical( omirrorv );
        m_item->setMirrorHorizontal( omirrorh );
        m_item->setScaling( oscaling );
	m_item->setExpression( oexpression );
	m_item->setPixmap( opixmap );
	m_item->canvasItem()->setSize( oldsize.width(), oldsize.height() );
    }
}

TextChangeCommand::TextChangeCommand( TextItem* it, QString t, QUndoCommand* parent )
    : QUndoCommand( parent ), CommandUtils( it->canvasItem() )
{
    m_item = it;
    text = t;
    oldtext = m_item->text();
    setText( name() + "\n" + name() );
}

void TextChangeCommand::redo()
{
    if( canvasHasItem() )
        m_item->setText( text );
}

void TextChangeCommand::undo()
{
    if( canvasHasItem() )
        m_item->setText( oldtext );
}

TextRotationCommand::TextRotationCommand( double rot, TextItem* t, QUndoCommand* parent )
    : QUndoCommand( parent ), CommandUtils( t->canvasItem() ), m_item( t )
{
    rot1 = rot;
    rot2 = t->rotation();
    setText( name() + "\n" + name() );
}

void TextRotationCommand::redo()
{
    m_item->setRotation( rot1 );
}

void TextRotationCommand::undo()
{
    m_item->setRotation( rot2 );
}

//NY24
TextLineChangeCommand::TextLineChangeCommand( TextLineItem* it, QString t, int font , int magvert, int maghor, QUndoCommand* parent )
    : QUndoCommand( parent ), CommandUtils( it->canvasItem() )
{
    m_item = it;
    text = t;
    oldtext = m_item->text();
    m_font = font;
    m_mag_vert = magvert;
    m_mag_hor = maghor;
    setText( name() + "\n" + name() );
}

void TextLineChangeCommand::redo()
{
    if( canvasHasItem() ){
        m_item->setText( text );
        m_item->setFont(m_font);
        m_item->setMagVert(m_mag_vert);
        m_item->setMagHor(m_mag_hor);
    }
}

void TextLineChangeCommand::undo()
{
    if( canvasHasItem() )
        m_item->setText( oldtext );
}
//NY24

BarcodeCommand::BarcodeCommand( BarcodeItem* bcode, Barkode* d, QUndoCommand* parent )
    : QUndoCommand( parent ), CommandUtils( bcode->canvasItem() )
{
    m_item = bcode;
    olddata = *bcode;
    data = d;
    setText( name() + "\n" + name() );
}

void BarcodeCommand::redo()
{
    if( canvasHasItem() ) {
        m_item->setData( *data );
        m_item->updateBarcode();
    }
}

void BarcodeCommand::undo()
{
    if( canvasHasItem() ) {
        m_item->setData( olddata );
        m_item->updateBarcode();
    }
}

void NewPictureCommand::create()
{
    ImageItem* r = new ImageItem();
    m_object = r;
}

NewRectCommand::NewRectCommand( MyCanvasView* v, bool circle )
    : NewItemCommand( v, i18n("New Rectangle") )
{
    m_circle = circle;
}

void NewRectCommand::create()
{
    RectItem* r = new RectItem();
    r->setCircle( m_circle );
    
    m_object = r;
}

NewLineCommand::NewLineCommand( MyCanvasView* v )
    : NewItemCommand( v, i18n("New Line") )
{
}

void NewLineCommand::create()
{
    m_object = new LineItem();
}

NewTextCommand::NewTextCommand( QString t, MyCanvasView* v, TokenProvider* token )
    : NewItemCommand( v, i18n("New Text") ),
      m_token( token )
{
    text = t;
}

void NewTextCommand::create()
{
    TextItem* t = new TextItem();
    t->setTokenProvider( m_token );
    t->setText( text );

    m_object = t;    
}

//NY25
NewTextLineCommand::NewTextLineCommand( QString t, MyCanvasView* v, TokenProvider* token )
    : NewItemCommand( v, i18n("New TextLine") ),
      m_token( token )
{
    text = t;
}

void NewTextLineCommand::create()
{
    TextLineItem* t = new TextLineItem();
    t->setTokenProvider( m_token );
    t->setText( text );

    m_object = t;    
}
//NY25

NewBarcodeCommand::NewBarcodeCommand( MyCanvasView* v, TokenProvider* token )
    : NewItemCommand( v, i18n("New Barcode") ),
      m_token( token )
{
}

void NewBarcodeCommand::create()
{
    m_object = new BarcodeItem();
    m_object->setTokenProvider( m_token );
}

DeleteCommand::~DeleteCommand()
{
    if( m_canvas_item && canvasHasItem() && m_canvas_item->scene() == 0 )
    {
        DocumentItem* item = m_canvas_item->item();
        if( item )
            item->disconnect( item, SIGNAL( destroyed() ), this, 0 );
        delete m_canvas_item;
    }
}

void DeleteCommand::redo()
{
    if( canvasHasItem() ) {
        /*m_canvas_item->hide();*/// hide() causes a runtime error in some special situations
        c->removeItem( m_canvas_item );
    }
}

void DeleteCommand::undo()
{
    // canvasHasItem won't work here
    if( m_canvas_item ) {
        c->addItem( m_canvas_item );
        m_canvas_item->show();
    }
}

BorderCommand::BorderCommand( bool border, const QPen & pen, DocumentItem* item, QUndoCommand* parent )
    : QUndoCommand( parent ), CommandUtils( item->canvasItem() )
{
    m_new_border = border;
    m_new_pen = pen;
    m_item = item;
    setText( name() + "\n" + name() );
}

void BorderCommand::redo()
{
    if( canvasHasItem() )
    {
        m_old_border = m_item->border();
        m_old_pen = m_item->pen();
        m_item->setBorder( m_new_border );
        m_item->setPen( m_new_pen );
        
        m_canvas_item->update();
    }
}

void BorderCommand::undo()
{
    if( canvasHasItem() )
    {
        m_item->setBorder( m_old_border );
        m_item->setPen( m_old_pen );
        
        m_canvas_item->update();
    }
}

FillCommand::FillCommand( QColor c, RectItem* r, QUndoCommand* parent )
    : QUndoCommand( parent ), CommandUtils( r->canvasItem() )
{
    fill = c;
    m_item = r;
    setText( name() + "\n" + name() );
}

void FillCommand::redo()
{
    if( canvasHasItem() ) {
        fill2 = m_item->color();
        m_item->setColor( fill );
    }
}

void FillCommand::undo()
{
    if( canvasHasItem() ) {
        m_item->setColor( fill2 );
    }
}

void ScriptCommand::redo()
{
    if( canvasHasItem() )
    {
	m_old_script = m_canvas_item->item()->visibilityScript();
        m_canvas_item->item()->setVisibilityScript( m_script );
        m_canvas_item->update();
    }
}

void ScriptCommand::undo()
{
    if( canvasHasItem() )
    {
        m_canvas_item->item()->setVisibilityScript( m_old_script );
        m_canvas_item->update();
    }
}

#include "commands.moc"
