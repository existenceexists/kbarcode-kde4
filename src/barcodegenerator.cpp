/***************************************************************************
                          barcodegenerator.cpp  -  description
                             -------------------
    begin                : Son Dez 29 2002
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

#include "barcodegenerator.h"
#include "barcodeitem.h"
#include "printersettings.h"
#include "barcodecombo.h"
#include "mimesources.h"
#include "tokenprovider.h"

// KDE includes
#include <kapplication.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kimageio.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qprinter.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>

// Qt includes
#include <qclipboard.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <QPaintDevice>
#include <QPicture>
#include <QDesktopWidget>
#include <QScrollArea>
#include <QX11Info>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QString>
#include <QMimeData>

BarcodeGenerator::BarcodeGenerator( QWidget* parent )
    : QDialog( parent )
{
    setWindowTitle( i18n( "Barcode Generator" ) );

    BarcodeGeneratorLayout = new QHBoxLayout( this );
    BarcodeGeneratorLayout->setObjectName( "BarcodeGeneratorLayout" );
    BarcodeGeneratorLayout->setContentsMargins( 11, 11, 11, 11 );
    BarcodeGeneratorLayout->setSpacing( 6 );
    Layout5 = new QVBoxLayout();
    Layout5->setObjectName( "Layout5" );
    Layout5->setContentsMargins( 0, 0, 0, 0 );
    Layout5->setSpacing( 6 );
    Layout6 = new QVBoxLayout();
    Layout6->setObjectName( "Layout2" );
    Layout6->setContentsMargins( 0, 0, 0, 0 );
    Layout6->setSpacing( 6 );
    widget = new BarcodeWidget( this );

    m_token = new TokenProvider( KApplication::desktop()->screen() );
    widget->setTokenProvider( m_token );

    buttonGenerate = new KPushButton( this );
    buttonGenerate->setText( i18n( "&Generate" ) );
    buttonGenerate->setEnabled( Barkode::haveBarcode() );
    buttonGenerate->setIcon( KIcon("view-barcode") );
    
    buttonSave = new KPushButton( this );
    buttonSave->setText( i18n( "&Save" ) );
    buttonSave->setEnabled( false );
    buttonSave->setIcon( KIcon("document-save") );
        
    buttonCopy = new KPushButton( this );
    buttonCopy->setText( i18n("&Copy") );
    buttonCopy->setEnabled( false );
    buttonCopy->setIcon( KIcon("edit-copy") );
    
    buttonPrint = new KPushButton( this );
    buttonPrint->setText( i18n("&Print") );
    buttonPrint->setEnabled( false );
    buttonPrint->setIcon( KIcon("document-print") );
    
    buttonClose = new KPushButton( this );
    buttonClose->setText( i18n("&Close" ) );
    buttonClose->setIcon( KIcon("dialog-close") );
    

    sv = new QScrollArea( this );
    
    barcode = new QLabel( NULL );
    connect( buttonGenerate, SIGNAL( clicked() ), this, SLOT( generate() ) );
    connect( buttonSave, SIGNAL( clicked() ), this, SLOT( save() ) );
    connect( buttonPrint, SIGNAL( clicked() ), this, SLOT( print() ) );
    connect( buttonCopy, SIGNAL( clicked() ), this, SLOT( copy() ) );

    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding );

    Layout5->addWidget( buttonGenerate );
    Layout5->addWidget( buttonSave );
    Layout5->addWidget( buttonPrint );
    Layout5->addWidget( buttonCopy );
    Layout5->addItem( spacer );
    Layout5->addWidget( buttonClose );
    
    Layout6->addWidget( widget );
    Layout6->addWidget( sv );
    
    BarcodeGeneratorLayout->addLayout( Layout6 );
    BarcodeGeneratorLayout->addLayout( Layout5 );
    BarcodeGeneratorLayout->setStretchFactor( Layout6, 2 );

    connect( buttonClose, SIGNAL( clicked() ), this, SLOT( close() ) );
    
    buttonGenerate->setDefault( true );
    
    show();
}

BarcodeGenerator::~BarcodeGenerator()
{
    delete m_token;
}

void BarcodeGenerator::generate()
{        
    Barkode d;
    widget->getData( d );
    d.setTokenProvider( m_token );
    d.update( KApplication::desktop() );

    delete barcode;
    barcode = new QLabel( NULL );
    barcode->setPixmap( d.pixmap() );
    sv->setWidget( barcode );

    buttonSave->setEnabled( !barcode->pixmap()->isNull() );
    buttonPrint->setEnabled( !barcode->pixmap()->isNull() );
    buttonCopy->setEnabled( !barcode->pixmap()->isNull() );
}

void BarcodeGenerator::save()
{
    Barkode bc;
    widget->getData( bc );

    if(!bc.isValid()) 
    {
        KFileDialog fd( KUrl("kfiledialog:///save_image"), 
                        KImageIO::pattern( KImageIO::Writing ), this );
        fd.setMode( KFile::File | KFile::LocalOnly );
        fd.setOperationMode( KFileDialog::Saving );       
        
        if( fd.exec() == QDialog::Accepted ) 
        {
            QString path = fd.selectedUrl().path();
            QString extension = KImageIO::typeForMime( path ).first();
        
            if( extension.isNull() )
                extension = KImageIO::typeForMime( fd.currentFilter() ).first(); 

        
            bc.setTokenProvider( m_token );
            bc.update( KApplication::desktop() );

            if(!bc.pixmap().save( path, extension.toLatin1().data(), 0 ))
                KMessageBox::error( this, i18n("An error occurred during saving the image") );
        }
    }        
}

void BarcodeGenerator::print()
{
    Barkode d;
    widget->getData( d );
    
    if( d.isValid() )
        return;

    QPrinter* printer = PrinterSettings::getInstance()->setupPrinter( KUrl("kfiledialog:///kbarcode"), this );
    if( !printer )
        return;

    // unless we can center the barcode
    printer->setFullPage( false );
    
    double scalex = (double)printer->logicalDpiX() / (double)QX11Info::appDpiX();
    double scaley = (double)printer->logicalDpiY() / (double)QX11Info::appDpiY();
    
    QPicture picture;
    QPainter p( printer );
    p.scale( scalex, scaley );
    // TODO: center barcode

    TokenProvider tp( printer );
    
    d.setTokenProvider( &tp );
    d.update( printer );

    picture = d.picture();
    p.drawPicture( QPoint( 0, 0 ), picture );
    p.end();
    
    delete printer;
}

void BarcodeGenerator::copy()
{
    if( barcode->pixmap()->isNull() )
        return;
        
    Barkode bc;
    widget->getData( bc );

    BarcodeItem* item = new BarcodeItem( bc );
    DocumentItemList list;
    list.append( item );
    DocumentItemDrag* drag = new DocumentItemDrag();
    drag->setDocumentItem( &list );

    kapp->clipboard()->setMimeData( drag );
}

#include "barcodegenerator.moc"
