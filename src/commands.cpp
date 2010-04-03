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


// KDE includes
#include <kapplication.h>
//Added by qt3to4:
#include <QPixmap>
#include <krandom.h>

QPoint getFreePos( QCanvas* c )
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

    c = m_canvas_item->canvas();
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
        QCanvasItemList list = c->allItems();
        for( unsigned int i=0;i<list.count();i++)
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

NewItemCommand::NewItemCommand( MyCanvasView* view, const QString & name )
    : QObject(), KCommand()
{
    cv = view;
    m_name = name;
    m_point = getFreePos( cv->canvas() );
    m_item = NULL;
    m_object = NULL;
}

NewItemCommand::~NewItemCommand()
{
    m_item->remRef();
}

void NewItemCommand::execute()
{
    if( !m_item )
    {
        create();

        if( m_object )
        {
            m_item = new TCanvasItem( cv );
            m_item->setItem( m_object );
            m_item->move( m_point.x(), m_point.y() );
	    m_item->addRef();

	    /* NOT NEEDED:
            connect( m_object, SIGNAL( destroyed() ), this, SLOT( documentItemDeleted() ) );
	    */
        }
    }

    if( m_item )
    {
        m_item->setCanvas( cv->canvas() );
        m_item->show();
        m_item->update();
        cv->setCurrent( m_item );
    }
}

void NewItemCommand::unexecute()
{
    if( m_item ) {
        m_item->setCanvas( NULL );
        m_item->hide();
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

void ResizeCommand::execute()
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

void ResizeCommand::unexecute()
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

void MoveCommand::execute()
{
    if( canvasHasItem() )
    {
	m_canvas_item->moveByMM( x, y );
        m_canvas_item->update();        
    }
}

void MoveCommand::unexecute()
{
    if( canvasHasItem() ) 
    {
	m_canvas_item->moveByMM( -x, -y );
	m_canvas_item->update();        
    }
}

ChangeZCommand::ChangeZCommand( int z, TCanvasItem* it )
    : CommandUtils( it )
{
    m_z = z;
    m_oldz = (int)m_canvas_item->z();
}

void ChangeZCommand::execute()
{
   if( canvasHasItem() )
    {
        m_canvas_item->setZ( m_z );
        m_canvas_item->update();
    }
}

void ChangeZCommand::unexecute()
{
   if( canvasHasItem() )
    {
        m_canvas_item->setZ( m_oldz );
        m_canvas_item->update();
    }
}

void LockCommand::execute()
{
    if( canvasHasItem() )
    {
        m_canvas_item->item()->setLocked( m_locked );
        m_canvas_item->update();
    }
}

void LockCommand::unexecute()
{
    if( canvasHasItem() )
    {
        m_canvas_item->item()->setLocked( !m_locked );
        m_canvas_item->update();
    }
}

PictureCommand::PictureCommand( double r, bool mh, bool mv, EImageScaling s, ImageItem* it ) 
    : CommandUtils( it->canvasItem() )
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

void PictureCommand::execute()
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

void PictureCommand::unexecute()
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

TextChangeCommand::TextChangeCommand( TextItem* it, QString t )
    : CommandUtils( it->canvasItem() )
{
    m_item = it;
    text = t;
    oldtext = m_item->text();
}

void TextChangeCommand::execute()
{
    if( canvasHasItem() )
        m_item->setText( text );
}

void TextChangeCommand::unexecute()
{
    if( canvasHasItem() )
        m_item->setText( oldtext );
}

TextRotationCommand::TextRotationCommand( double rot, TextItem* t  )
    : CommandUtils( t->canvasItem() ), m_item( t )
{
    rot1 = rot;
    rot2 = t->rotation();
}

void TextRotationCommand::execute()
{
    m_item->setRotation( rot1 );
}

void TextRotationCommand::unexecute()
{
    m_item->setRotation( rot2 );
}

//NY24
TextLineChangeCommand::TextLineChangeCommand( TextLineItem* it, QString t, int font , int magvert, int maghor)
    : CommandUtils( it->canvasItem() )
{
    m_item = it;
    text = t;
    oldtext = m_item->text();
    m_font = font;
    m_mag_vert = magvert;
    m_mag_hor = maghor;
}

void TextLineChangeCommand::execute()
{
    if( canvasHasItem() ){
        m_item->setText( text );
        m_item->setFont(m_font);
        m_item->setMagVert(m_mag_vert);
        m_item->setMagHor(m_mag_hor);
    }
}

void TextLineChangeCommand::unexecute()
{
    if( canvasHasItem() )
        m_item->setText( oldtext );
}
//NY24

BarcodeCommand::BarcodeCommand( BarcodeItem* bcode, Barkode* d )
    : CommandUtils( bcode->canvasItem() )
{
    m_item = bcode;
    olddata = *bcode;
    data = d;
}

void BarcodeCommand::execute()
{
    if( canvasHasItem() ) {
        m_item->setData( *data );
        m_item->updateBarcode();
    }
}

void BarcodeCommand::unexecute()
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
    if( m_canvas_item && canvasHasItem() && m_canvas_item->canvas() == 0 )
    {
        DocumentItem* item = m_canvas_item->item();
        if( item )
            item->disconnect( item, SIGNAL( destroyed() ), this, 0 );
        delete m_canvas_item;
    }
}

void DeleteCommand::execute()
{
    if( canvasHasItem() ) {
        m_canvas_item->setCanvas( 0 );
        m_canvas_item->hide();
    }
}

void DeleteCommand::unexecute()
{
    // canvasHasItem won't work here
    if( m_canvas_item ) {
        m_canvas_item->setCanvas( c );
        m_canvas_item->show();
    }
}

BorderCommand::BorderCommand( bool border, const QPen & pen, DocumentItem* item )
    : CommandUtils( item->canvasItem() )
{
    m_new_border = border;
    m_new_pen = pen;
    m_item = item;
}

void BorderCommand::execute()
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

void BorderCommand::unexecute()
{
    if( canvasHasItem() )
    {
        m_item->setBorder( m_old_border );
        m_item->setPen( m_old_pen );
        
        m_canvas_item->update();
    }
}

FillCommand::FillCommand( QColor c, RectItem* r )
    : CommandUtils( r->canvasItem() )
{
    fill = c;
    m_item = r;
}

void FillCommand::execute()
{
    if( canvasHasItem() ) {
        fill2 = m_item->color();
        m_item->setColor( fill );
    }
}

void FillCommand::unexecute()
{
    if( canvasHasItem() ) {
        m_item->setColor( fill2 );
    }
}

void ScriptCommand::execute()
{
    if( canvasHasItem() )
    {
	m_old_script = m_canvas_item->item()->visibilityScript();
        m_canvas_item->item()->setVisibilityScript( m_script );
        m_canvas_item->update();
    }
}

void ScriptCommand::unexecute()
{
    if( canvasHasItem() )
    {
        m_canvas_item->item()->setVisibilityScript( m_old_script );
        m_canvas_item->update();
    }
}

#include "commands.moc"
