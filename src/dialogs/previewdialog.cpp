/***************************************************************************
                          previewdialog.cpp  -  description
                             -------------------
    begin                : Die Dez 10 2002
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

#include "previewdialog.h"
#include "label.h"
#include "sqltables.h"
#include "labeleditor.h"
#include "measurements.h"

// Qt includes
#include <qiodevice.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <q3scrollview.h>
#include <QSqlQueryModel>
#include <QPaintDevice>
#include <QDesktopWidget>
//Added by qt3to4:
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPixmap>
#include <QSqlQuery>
#include <QVBoxLayout>

// KDE includes
#include <kabc/addresseedialog.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kconfiggroup.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>
#include <kpushbutton.h>
#include <kglobal.h>

int PreviewDialog::customer_index = 0;
int PreviewDialog::m_index = 1;
QString PreviewDialog::group = "";
QString PreviewDialog::article = "";
        
PreviewDialog::PreviewDialog( QIODevice* device, Definition* d, QString filename, QWidget *parent)
    : QDialog( parent, Qt::WDestructiveClose )
{
    setModal( true );
    file = device;
    def = d;
    m_filename = filename;
    
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );

    QHBoxLayout* Layout = new QHBoxLayout( this );
    Layout->setContentsMargins( 6, 6, 6, 6 );
    Layout->setSpacing( 6 );
    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->setContentsMargins( 6, 6, 6, 6 );
    vbox->setSpacing( 6 );
    QVBoxLayout* buttons = new QVBoxLayout();
    buttons->setContentsMargins( 6, 6, 6, 6 );
    buttons->setSpacing( 6 );
    QGridLayout* grid = new QGridLayout();

    customerName = new KComboBox( false, this );
    customerId = new KComboBox( false, this );

    groupName = new KLineEdit( group, this );
    articleId = new KLineEdit( article, this );
    if( SqlTables::isConnected() ) {
        KCompletion* comp = articleId->completionObject();
        QSqlQuery query( "select article_no from barcode_basic"  );
        QStringList slist;
        while ( query.next() )
            slist.append( query.value(0).toString() );
        slist.sort();
        comp->setItems( slist );
    }

    spinIndex = new KIntNumInput( this );
    spinIndex->setRange( 1, 100000, 1 );
    spinIndex->setSliderEnabled( false );
    spinIndex->setValue( m_index );

    serialStart = new KLineEdit( this );

    serialInc = new KIntNumInput( this );
    serialInc->setRange( 1, 10000, 1 );
    serialInc->setSliderEnabled( false );

    lineAddr = new KLineEdit( this );
    lineAddr->setReadOnly( true );
    
    buttonAddr = new KPushButton( i18n("Select &Address"), this );

    buttonUpdate = new KPushButton( i18n("&Update"), this );
    buttonClose = new KPushButton( i18n("&Close"), this );
    buttonClose->setIcon( KIcon("dialog-close") );
    
    grid->addWidget( new QLabel( i18n("Customer Name and No.:"), this ), 0, 0 );
    grid->addWidget( customerName, 0, 1 );
    grid->addWidget( customerId, 0, 2 );
    grid->addWidget( new QLabel( i18n("Article Number:"), this ), 1, 0 );
    grid->addWidget( articleId, 1, 1, 1, 2 );
    grid->addWidget( new QLabel( i18n("Group:"), this ), 2, 0 );
    grid->addWidget( groupName, 2, 1, 1, 2 );
    grid->addWidget( new QLabel( i18n("Index:"), this ), 3, 0 );
    grid->addWidget( spinIndex, 3, 1, 1, 2 );
    grid->addWidget( new QLabel( i18n("Serial start:"), this ), 4, 0 );
    grid->addWidget( serialStart, 4, 1, 1, 2 );
    grid->addWidget( new QLabel( i18n( "Serial increment:" ), this ), 5, 0 );
    grid->addWidget( serialInc, 5, 1, 1, 2 );
    grid->addWidget( new QLabel( i18n( "Addressbook entry:" ), this ), 6, 0 );
    grid->addWidget( lineAddr, 6, 1 );
    grid->addWidget( buttonAddr, 6, 2 );
    Q3ScrollView* sv = new Q3ScrollView( this );

    preview = new QLabel( sv->viewport() );
    /*sv->addChild( preview );*/// This line causes a badly displayed preview.
    
    QPixmap pix( (int)d->getMeasurements().width( this ), (int)d->getMeasurements().height( this ) );
    pix.fill( Qt::white );
    preview->setPixmap( pix );
        
    vbox->addLayout( grid );
    vbox->addWidget( sv );

    buttons->addWidget( buttonUpdate );
    buttons->addWidget( buttonClose );
    buttons->addItem( spacer );
    
    Layout->addLayout( vbox );
    Layout->addLayout( buttons );

    connect( buttonClose, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( buttonAddr, SIGNAL( clicked() ), this, SLOT( selectAddress() ) );
    // TODO: ... Done.
    // remove this ugly hack! The finished label is only shown when I call
    // updateChanges() twice. I have no idea why! ... Fixed. No need to call it twice now. The problem has been fixed by QT/KDE, not by me.
    connect( buttonUpdate, SIGNAL( clicked() ), this, SLOT( updatechanges() ) );
    /*connect( buttonUpdate, SIGNAL( clicked() ), this, SLOT( updatechanges() ) );*/// -!F: original, delete
    // end ugly hack
    
    connect( customerName, SIGNAL( activated(int) ), this, SLOT( customerNameChanged(int) ) );
    connect( customerId, SIGNAL( activated(int) ), this, SLOT( customerIdChanged(int) ) );

    KConfigGroup config = KGlobal::config()->group( "PreviewDialog" );
    
    resize( config.readEntry( "width", width() ), config.readEntry( "height", height() ) );
    
    if( SqlTables::isConnected() )
        setupSql();
}

PreviewDialog::~PreviewDialog()
{
    KConfigGroup config = KGlobal::config()->group( "PreviewDialog" );

    config.writeEntry( "width", width() );
    config.writeEntry( "height", height() );
    config.sync();
}

void PreviewDialog::setupSql()
{
    QSqlQueryModel model;
    model.setQuery( "SELECT * FROM customer" );
    customerId->clear();
    customerName->clear();
    for ( int rowNumber = 0; rowNumber < model.rowCount(); rowNumber++ ) {
        customerId->addItem( model.record( rowNumber ).value( "customer_no" ).toString() );
        customerName->addItem( model.record( rowNumber ).value( "customer_name" ).toString() );
    }

    customerId->setCurrentIndex( customer_index );
    customerName->setCurrentIndex( customer_index );
}

void PreviewDialog::selectAddress()
{
    m_address = KABC::AddresseeDialog::getAddressee( this );
    if( !m_address.isEmpty() ) 
        lineAddr->setText( m_address.realName() );
    else
        lineAddr->setText( QString::null );
}

void PreviewDialog::updatechanges()
{
    QPixmap pix( (int)def->getMeasurements().width( this ), (int)def->getMeasurements().height( this ) );
    pix.fill( Qt::white );

    QPainter painter( &pix );
    
    Label* l = new Label( def, file, m_filename, KApplication::desktop(),
        customerId->currentText(), articleId->text(), groupName->text() );
    l->setIndex( spinIndex->value() - 1 );
    l->setSerial( serialStart->text(), serialInc->value() );
    l->setAddressee( &m_address );
    l->draw( &painter, 0, 0 );

    preview->setPixmap( pix );

    delete l;
        
    // keep settings for next the
    // next time the dialog is shown
    group = groupName->text();
    article = articleId->text();
    customer_index = customerId->currentIndex();
    m_index = spinIndex->value();
}

void PreviewDialog::customerIdChanged( int index )
{
    customerName->setCurrentIndex( index );
}

void PreviewDialog::customerNameChanged( int index )
{
    customerId->setCurrentIndex( index );
}

#include "previewdialog.moc"
