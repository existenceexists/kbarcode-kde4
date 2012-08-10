/***************************************************************************
                         propertywidget.h  -  description
                             -------------------
    begin                : Do Sep 10 2004
    copyright            : (C) 2004 by Dominik Seichter
    email                : domseichter@web.de
 ***************************************************************************/

/***************************************************************************
                                                                          
    This program is free software; you can redistribute it and/or modify  
    it under the terms of the GNU General Public License as published by  
    the Free Software Foundation; either version 2 of the License, or     
    (at your option) any later version.                                   
                                                                          
 ***************************************************************************/

#include "propertywidget.h"
#include "documentitem.h"
#include "rectitem.h"
#include "commands.h"
#include "sqltables.h"
#include "barcodecombo.h"
#include "multilineeditdlg.h"
#include "textitem.h"
#include "imageitem.h"
#include "measurements.h"
#include "mycanvasview.h"
//NY20
#include "textlineitem.h"
#include "textlineedit.h"
//NY20
#include "tokendialog.h"

#include <kcolorbutton.h>
#include <kcombobox.h>
#include <QUndoCommand>
#include <kiconloader.h>
#include <kimageio.h>
#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>
#include <ktextedit.h>
#include <kurlrequester.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qradiobutton.h>
#include <qsqlquery.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QPixmap>
#include <kmessagebox.h>

#define IMAGE_INTERNAL i18n("Image Saved in KBarcode")

void FillLineCombo( KComboBox* box )
{
    /* A small helper function to fill
     * a combobox with all by Qt
     * supported pen styles.
     */

    // TODO: Use the enum for Qt line styles here!

    QPainter p;
    for( int i = 1; i < 6; i++ ) {
        QPixmap pixmap( 60, 20 );
        pixmap.fill( Qt::white );
        p.begin( &pixmap );
        p.setPen( QPen( Qt::black, 3, (Qt::PenStyle)i ) );
        p.drawLine( 5, 10, 55, 10 );
        p.end();

        box->addItem( QIcon( pixmap ), QString() );
    }
}

PropertyWidget::PropertyWidget(QWidget* parent )
 : QWidget( parent )
{
    grid = new QGridLayout( this );
}

PropertyBorder::PropertyBorder( QWidget* parent )
    : PropertyWidget( parent )
{
    checkBorder = new QCheckBox( this );
    checkBorder->setText( i18n("&Border Visible") );
    
    buttonColor = new KColorButton( this );

    spinWidth = new KIntNumInput( this );
    spinWidth->setRange( 1, 100, 1 );
    spinWidth->setSliderEnabled( false );

    comboLine = new KComboBox( false, this );
    FillLineCombo( comboLine );    

    grid->addWidget( checkBorder, 0, 0, 1, 2 );
    grid->addWidget( new QLabel( i18n("Color:"), this ), 1, 0 );
    grid->addWidget( buttonColor, 1, 1 );
    grid->addWidget( new QLabel( i18n("Line Width:"), this ), 2, 0 );
    grid->addWidget( spinWidth, 2, 1 );
    grid->addWidget( new QLabel( i18n("Line Style:"), this ), 3, 0 );
    grid->addWidget( comboLine, 3, 1 );
    
    
    connect( checkBorder, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
}

void PropertyBorder::applySettings( DocumentItem* item, QUndoCommand* command )
{
    BorderCommand* bc = new BorderCommand( checkBorder->isChecked(), QPen( buttonColor->color(), spinWidth->value(), (Qt::PenStyle)(comboLine->currentIndex() + 1) ), item, command );
}

void PropertyBorder::initSettings( DocumentItem* item )
{
    checkBorder->setChecked( item->border() );
    buttonColor->setColor( item->pen().color() );
    spinWidth->setValue( item->pen().width() );
    comboLine->setCurrentIndex( item->pen().style() - 1 );
    
    enableControls();
}

void PropertyBorder::enableControls()
{
    buttonColor->setEnabled( checkBorder->isChecked() );
    spinWidth->setEnabled( checkBorder->isChecked() );
    comboLine->setEnabled( checkBorder->isChecked() );
}


PropertyRotation::PropertyRotation( QWidget* parent )
    : PropertyWidget( parent )
{
    QLabel* label = new QLabel( i18n("&Rotation:"), this );
    comboRotation = new KComboBox( FALSE, this );
    label->setBuddy( comboRotation );

    comboRotation->addItem( i18n("0") );
    comboRotation->addItem( i18n("90") );
    comboRotation->addItem( i18n("180") );
    comboRotation->addItem( i18n("270") );

    grid->addWidget( label, 0, 0 );
    grid->addWidget( comboRotation, 0, 1  );

}

void PropertyRotation::applySettings( DocumentItem* item, QUndoCommand* command )
{
    TextItem* text = static_cast<TextItem*>(item);
    double rot = 0.0;

    if( comboRotation->currentIndex() == 1 )
        rot = 90.0;
    else if( comboRotation->currentIndex() == 2 )
        rot = 180.0;
    else if( comboRotation->currentIndex() ==  3 )
        rot = 270.0;

    TextRotationCommand* rc = new TextRotationCommand( rot, text, command );
}

void PropertyRotation::initSettings( DocumentItem* item )
{
    TextItem* text = static_cast<TextItem*>(item);

    if( text->rotation() == 0.0 )
        comboRotation->setCurrentIndex( 0 );
    else if( text->rotation() == 90.0 )
        comboRotation->setCurrentIndex( 1 );
    else if( text->rotation() == 180.0 )
        comboRotation->setCurrentIndex( 2 );
    else if( text->rotation() == 270.0 )
        comboRotation->setCurrentIndex( 3 );
}

PropertyFill::PropertyFill( QWidget* parent )
    : PropertyWidget( parent )
{
    buttonColor = new KColorButton( this );
    
    grid->addWidget( new QLabel( i18n("Color:"), this ), 0, 0 );
    grid->addWidget( buttonColor, 0, 1 );
}

void PropertyFill::applySettings( DocumentItem* item, QUndoCommand* command )
{
    RectItem* rect = static_cast<RectItem*>(item);
    FillCommand* fc = new FillCommand( buttonColor->color(), rect, command );
}

void PropertyFill::initSettings( DocumentItem* item )
{
    RectItem* rect = static_cast<RectItem*>(item);
    buttonColor->setColor( rect->color() );
}

PropertyBarcode::PropertyBarcode( TokenProvider* token, QWidget* parent )
    : PropertyWidget( parent )
{
    QLabel* TextLabel1 = new QLabel( this );
    TextLabel1->setObjectName( "TextLabel1" );
    TextLabel1->setText( i18n( "Barcode Settings:" ) );
            
    comboComplex = new KComboBox( FALSE, this );
    if( SqlTables::isConnected() )
    {
        QSqlQuery* query = new QSqlQuery("select encoding_type from barcode_basic group by encoding_type");
        while( query->next() )
            comboComplex->addItem( "Main:" + query->value( 0 ).toString() );
        
        delete query;
        
        query = new QSqlQuery("select customer_no, encoding_type from customer_text group by customer_no, encoding_type");
        while( query->next() )
            comboComplex->addItem( query->value( 0 ).toString() + ":" + query->value( 1 ).toString() );
        delete query;
    }
        
    comboComplex->addItem( "Static" );

    barcode = new BarcodeWidget( this );
    barcode->setTokenProvider( token );

    //connect( this, SIGNAL( defaultClicked() ), this, SLOT( defaults() ) );
    connect( comboComplex, SIGNAL( activated(int) ), this, SLOT(changedCombo() ) );
    
    grid->addWidget( TextLabel1, 0, 0 );
    grid->addWidget( comboComplex, 0, 1 );
    grid->addWidget( barcode, 1, 0, 1, 2 );
    
    changedCombo();
}

// void PropertyBarcode::defaults()
// {
//     comboComplex->setCurrentItem( 0 );
//     barcode->defaults();
// }

void PropertyBarcode::changedCombo()
{
    bool v = false;
    if( comboComplex->currentIndex() == comboComplex->count() - 1 )
        v = true;

    barcode->setDataEnabled( v );
    barcode->setStandardEnabled( v );
}

void PropertyBarcode::applySettings( DocumentItem* item, QUndoCommand* command )
{
    BarcodeItem* bcode = static_cast<BarcodeItem*>(item);

    Barkode* d = new Barkode();
    barcode->getData( *d );
            
    if( comboComplex->currentIndex() != comboComplex->count() - 1 ) {
        d->setType( getTypeFromCaption( comboComplex->currentText() ) );
        d->setValue( SqlTables::getInstance()->getBarcodeMaxLength( d->type() ) );
    }
    d->setDatabaseMode( comboComplex->currentText() );
    
    BarcodeCommand* bc = new BarcodeCommand( bcode, d, command );
}

void PropertyBarcode::initSettings( DocumentItem* item )
{
    BarcodeItem* bcode = static_cast<BarcodeItem*>(item);
    
    barcode->setData( *bcode );
    for( int i = 0; i < comboComplex->count(); i++ )
        if( comboComplex->itemText( i ).toLower() == bcode->databaseMode().toLower() )
            comboComplex->setCurrentIndex( i );

    changedCombo();
}

PropertyText::PropertyText( TokenProvider* token, QWidget* parent )
    : PropertyWidget( parent )
{
    m_editor = new MultiLineEditor( token, this );
    grid->addWidget( m_editor, 0, 0 );
}

void PropertyText::applySettings( DocumentItem* item, QUndoCommand* command )
{
    TextItem* text = static_cast<TextItem*>(item);
    
    TextChangeCommand* tc = new TextChangeCommand( text, m_editor->text(), command );
}

void PropertyText::initSettings( DocumentItem* item )
{
    TextItem* text = static_cast<TextItem*>(item);
    m_editor->setText( text->text() );
}

//NY21
PropertyTextLine::PropertyTextLine( TokenProvider* token, QWidget* parent )
    : PropertyWidget( parent )
{
    m_editor = new TextLineEditor( token, this );
    grid->addWidget( m_editor, 0, 0 );
}

void PropertyTextLine::applySettings( DocumentItem* item, QUndoCommand* command )
{
    TextLineItem* text = static_cast<TextLineItem*>(item);
    
    TextLineChangeCommand* tc = new TextLineChangeCommand( text, m_editor->text(), m_editor->getFontType(),m_editor->getVertMag(),m_editor->getHorMag(), command );
}

void PropertyTextLine::initSettings( DocumentItem* item )
{
    TextLineItem* text = static_cast<TextLineItem*>(item);
    m_editor->setText( text->text() );
    m_editor->setFontType(text->getFont());
    m_editor->setHorMag(text->getMagHor());
    m_editor->setVertMag(text->getMagVert());
}
//NY21
        
PropertySize::PropertySize( QWidget* parent )
    : PropertyWidget( parent )
{   
    const double low = -1000.0;
    const double max = 1000.0;
    QWidget* box = new QWidget( this );
	QVBoxLayout* box_layout = new QVBoxLayout;
	box->setLayout(box_layout);

    checkLock = new QCheckBox( i18n("&Protect item from being moved or resized"));
	box_layout->addWidget(checkLock);
    numTop = new KDoubleNumInput( low, max, 0.0, box, 0.2, 3 );
    box_layout->addWidget(numTop);
    numLeft = new KDoubleNumInput( low, max, 0.0, box, 0.2, 3 );
    box_layout->addWidget(numLeft);
    numHeight = new KDoubleNumInput( low, max, 0.0, box, 0.2, 3 );
    box_layout->addWidget(numHeight);
    numWidth = new KDoubleNumInput( low, max, 0.0, box, 0.2, 3 );
    box_layout->addWidget(numWidth);
    
    numTop->setSuffix( Measurements::system() );
    numLeft->setSuffix( Measurements::system() );
    numHeight->setSuffix( Measurements::system() );
    numWidth->setSuffix( Measurements::system() );
    
    /*numTop->setLabel( i18n("&Top:"), Qt::AlignLeft | Qt::AlignCenter );*/// -!F: original
    numTop->setLabel( i18n("&Top:"), Qt::AlignLeft | Qt::AlignTop );
    /*numLeft->setLabel( i18n("&Left:"), Qt::AlignLeft | Qt::AlignCenter );*/// -!F: original
    numLeft->setLabel( i18n("&Left:"), Qt::AlignLeft | Qt::AlignTop );
    /*numHeight->setLabel( i18n("&Height:"), Qt::AlignLeft | Qt::AlignCenter );*/// -!F: original
    numHeight->setLabel( i18n("&Height:"), Qt::AlignLeft | Qt::AlignTop );
    /*numWidth->setLabel( i18n("&Width:"), Qt::AlignLeft | Qt::AlignCenter );*/// -!F: original
    numWidth->setLabel( i18n("&Width:"), Qt::AlignLeft | Qt::AlignTop );
    
    grid->addWidget( box, 0, 0 );
    
    connect( checkLock, SIGNAL( clicked() ), this, SLOT( enableControls() )); 
}

void PropertySize::enableControls()
{
    numTop->setEnabled( !checkLock->isChecked() );
    numLeft->setEnabled( !checkLock->isChecked() );
    numHeight->setEnabled( !checkLock->isChecked() );
    numWidth->setEnabled( !checkLock->isChecked() );
}
        
void PropertySize::applySettings( DocumentItem* item, QUndoCommand* command )
{
    QRect r = item->rect();
    QPoint translation( 0, 0 );
    
    TCanvasItem* canvasItem = item->canvasItem();
    if( canvasItem )
    {
        MyCanvasView* view = canvasItem->canvasView();
        translation = view->getTranslation();
    }

    if( Measurements::measurementSystem() == Measurements::Metric )
    {
        r.setX( (int)(numLeft->value() * 1000));
        r.setY( (int)(numTop->value()  * 1000));
        r.setWidth( (int)(numWidth->value() * 1000));
        r.setHeight( (int)(numHeight->value() * 1000));
    }
    else
    {
        /* This conversion from int to millimeter should get its own
         * function or maybe even an own class.
         */
        #warning "test inch to mm!"
        r.setX( (int)(numLeft->value() * 1000 * 25.4000508001016 ) );
        r.setY( (int)(numTop->value()  * 1000 * 25.4000508001016 ) );
        r.setWidth( (int)(numWidth->value() * 1000 * 25.4000508001016 ) );
        r.setHeight( (int)(numHeight->value() * 1000 * 25.4000508001016 ) );
    }    
    
    if( canvasItem )
    {   
        if( item->rectMM().x() != r.x() || item->rectMM().y() != r.y() )
        {
            MoveCommand* mc = new MoveCommand( r.x(), r.y(), canvasItem, -1, command );
        }

        if( item->rectMM() != r )
        {
            ResizeCommand* rc = new ResizeCommand( canvasItem, false, -1, command );
            rc->setRect( r.x(), r.y(), r.width(), r.height() );
        }
                
        if( checkLock->isChecked() != item->locked() )
        {
            LockCommand* lc = new LockCommand( checkLock->isChecked(), canvasItem, command );
        }
    }
}

void PropertySize::initSettings( DocumentItem* item )
{
    QRect r = item->rectMM();
    QPoint translation( 0, 0 );
    
    TCanvasItem* canvasItem = item->canvasItem();
    if( canvasItem )
    {
        MyCanvasView* view = canvasItem->canvasView();
        translation = view->getTranslation();
    }
    
    if( Measurements::measurementSystem() == Measurements::Metric )
    {
        numLeft->setValue( r.x() / 1000.0 );
        numTop->setValue( r.y() / 1000.0 );
        numWidth->setValue( r.width() / 1000.0 );
        numHeight->setValue( r.height() / 1000.0 );
    }
    else // Imperial
    {
        numLeft->setValue( (r.x() / 1000.0) / 25.4000508001016 );
        numTop->setValue( (r.y() / 1000.0) / 25.4000508001016 );
        numWidth->setValue( (r.width() / 1000.0) / 25.4000508001016 );
        numHeight->setValue( (r.height() / 1000.0) / 25.4000508001016 );
    }
    
    checkLock->setChecked( item->locked() );
    enableControls();
}

PropertyImage::PropertyImage( TokenProvider* token, QWidget* parent )
    : PropertyWidget( parent ), m_token( token )
{
    QLabel* label = new QLabel( i18n("&Rotation:"), this );
    comboRotation = new KComboBox( FALSE, this );
    label->setBuddy( comboRotation );

    QGroupBox* groupFile = new QGroupBox( i18n("&Image"), this );
    QVBoxLayout* group_file_layout = new QVBoxLayout;
    radioImagePath = new QRadioButton( i18n("&Load image from path") );
    group_file_layout->addWidget(radioImagePath);
    imgUrl = new KUrlRequester;
    group_file_layout->addWidget(imgUrl);
    imgUrl->setFilter( KImageIO::pattern( KImageIO::Reading ) );
    imgUrl->setMode( KFile::File | KFile::ExistingOnly | KFile::LocalOnly );
    radioImageExpression = new QRadioButton( i18n("&Read image path from expression") );
    group_file_layout->addWidget(radioImageExpression);

    imgHBox = new QWidget;
    QHBoxLayout* imgHBox_layout = new QHBoxLayout;
	imgHBox->setLayout(imgHBox_layout);
    imgHBox_layout->setSpacing( 5 );
    group_file_layout->addWidget(imgHBox);
    groupFile->setLayout(group_file_layout);

    imgExpression = new KLineEdit;
	imgHBox_layout->addWidget(imgExpression);
    buttonToken = new KPushButton( i18n("&Insert Data Field..."));
	imgHBox_layout->addWidget(buttonToken);
    buttonToken->setIcon( KIcon("contents") );

    comboRotation->addItem( i18n("0") );
    comboRotation->addItem( i18n("90") );
    comboRotation->addItem( i18n("180") );
    comboRotation->addItem( i18n("270") );
    
    QGroupBox* group = new QGroupBox( i18n("&Size"), this );
    QVBoxLayout* group_layout = new QVBoxLayout;
    
    radioOriginal = new QRadioButton( i18n("&None") );
    group_layout->addWidget(radioOriginal);
    radioZoomed = new QRadioButton( i18n("&Zoom") );
    group_layout->addWidget(radioZoomed);
    radioScaled = new QRadioButton( i18n("S&cale") );
    group_layout->addWidget(radioScaled);
    group->setLayout(group_layout);
    checkMirrorH = new QCheckBox( i18n("Mirror &Horizontaly"), this );
    checkMirrorV = new QCheckBox( i18n("Mirror &Vertically"), this );

    grid->addWidget( groupFile, 0, 0, 2, 2 );
    grid->addWidget( group, 2, 0, 2, 2 );
    grid->addWidget( label, 4, 0 );
    grid->addWidget( comboRotation, 4, 1 );
    grid->addWidget( checkMirrorH, 5, 0, 1, 2 );
    grid->addWidget( checkMirrorV, 6, 0, 1, 2 );

    connect( radioImageExpression, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( radioImagePath, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( buttonToken, SIGNAL( clicked() ), this, SLOT( slotTokens() ) );
}

void PropertyImage::applySettings( DocumentItem* item, QUndoCommand* command )
{
    ImageItem* img = static_cast<ImageItem*>(item);
    EImageScaling scaling = eImage_Original;
    double rot = 0.0;
    QPixmap pix;

    if( radioImagePath->isChecked() ) 
    {
	if( !imgUrl->url().isEmpty() )
	{
	    if( !pix.load( imgUrl->url().path() ) )
		KMessageBox::error( NULL, QString( i18n("Image format not supported for file: %1") ).arg( imgUrl->url().path() ) );
	}
	else
	{
	    pix = img->pixmap();
	}
    }

    if( radioZoomed->isChecked() )
        scaling = eImage_Zoomed;
    else if( radioScaled->isChecked() )
        scaling = eImage_Scaled;
    
    if( comboRotation->currentIndex() == 1 )
        rot = 90.0;
    else if( comboRotation->currentIndex() == 2 )
        rot = 180.0;
    else if( comboRotation->currentIndex() ==  3 )
        rot = 270.0;
        
    PictureCommand* pc = new PictureCommand( rot, checkMirrorH->isChecked(), checkMirrorV->isChecked(), scaling, img, command );
    pc->setExpression( radioImageExpression->isChecked() ? imgExpression->text() : QString::null );
    pc->setPixmap( pix );
}

void PropertyImage::initSettings( DocumentItem* item )
{
    ImageItem* img = static_cast<ImageItem*>(item);
    QString expr = img->expression();

    if( !expr.isEmpty() )
    {
	radioImageExpression->setChecked( true );
	imgExpression->setText( expr );
    }
    else
    {
	// imgUrl->setURL( IMAGE_INTERNAL );
	radioImagePath->setChecked( true );
    }

    switch( img->scaling() )    
    {
        default:
        case eImage_Original:
            radioOriginal->setChecked( true );
            break;
        case eImage_Scaled:
            radioScaled->setChecked( true );
            break;
        case eImage_Zoomed:
            radioZoomed->setChecked( true );
            break;
    }
            
    if( img->rotation() == 0.0 )
        comboRotation->setCurrentIndex( 0 );
    else if( img->rotation() == 90.0 )
        comboRotation->setCurrentIndex( 1 );
    else if( img->rotation() == 180.0 )
        comboRotation->setCurrentIndex( 2 );
    else if( img->rotation() == 270.0 )
        comboRotation->setCurrentIndex( 3 );
        
    checkMirrorH->setChecked( img->mirrorHorizontal() );
    checkMirrorV->setChecked( img->mirrorVertical() );


    enableControls();
}

void PropertyImage::enableControls()
{
    imgHBox->setEnabled( radioImageExpression->isChecked() );
    imgUrl->setEnabled( radioImagePath->isChecked() );
}

void PropertyImage::slotTokens()
{
    TokenDialog tokendlg( m_token, this );
    if( tokendlg.exec() == QDialog::Accepted )
	imgExpression->insert( tokendlg.token() );
}

PropertyVisible::PropertyVisible( QWidget* parent )
    : PropertyWidget( parent )
{
    QLabel* label = new QLabel( i18n("Evaluate JavsScript code to define the visibility of this item:"), this );
    m_script = new KTextEdit( this );

    grid->addWidget( label, 0, 0 );
    grid->addWidget( m_script, 1, 0, 8, 3 );
}

void PropertyVisible::applySettings( DocumentItem* item, QUndoCommand* command )
{
    TCanvasItem* canvasItem = item->canvasItem();
    ScriptCommand* sc = new ScriptCommand( m_script->toPlainText(), canvasItem, command );
}

void PropertyVisible::initSettings( DocumentItem* item )
{
    m_script->setText( item->visibilityScript() );
}

#include "propertywidget.moc"
