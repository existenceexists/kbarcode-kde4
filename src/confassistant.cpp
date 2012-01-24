/***************************************************************************
                          confassistant.cpp  -  description
                             -------------------
    begin                : Son Jun 16 2002
    copyright            : (C) 2002 by Dominik Seichter
    email                : domseichter@web.de
 ****************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "confassistant.h"
#include "sqltables.h"
#include "printersettings.h"
#include "mainwindow.h"

// Qt includes
#include <q3buttongroup.h>
#include <qcheckbox.h>
#include <qcursor.h>
#include <qlayout.h>
#include <qsqldatabase.h>
#include <qradiobutton.h>
#include <qprinter.h>
//#include <q3textbrowser.h>
#include <QTextBrowser>
#include <QSqlError>
//Added by qt3to4:
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

// KDE includes
#include <kapplication.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kurllabel.h>
#include <kstandarddirs.h>

#include <QDebug>// -!F: delete

const char* description = I18N_NOOP(
        "KBarcode is a barcode and label printing application for KDE 3. It can "
        "be used to print every thing from simple business cards up to complex "
        "labels with several barcodes (e.g. article descriptions). KBarcode "
        "comes with an easy to use WYSIWYG label designer, a setup assistant, "
        "batch import of labels (directly from the delivery note), thousands "
        "of predefined labels, database management tools and translations "
        "in many languages. Even printing more than 10.000 labels in one go is "
        "no problem for KBarcode. Additionally it is a simply xbarcode "
        "replacement for the creation of barcodes. All major types of barcodes "
        "like EAN, UPC, CODE39 and ISBN are supported." );

ConfAssistant::ConfAssistant( QWidget* parent, QString name, bool modal )
    : KAssistantDialog( parent )
{
    setObjectName(name);
    setModal(modal);
    setCaption( i18n( "Configure KBarcode" ) );
    setWindowIcon( KIcon( KStandardDirs::locate( "appdata", QString( "hi16-app-kbarcode.png" ) ) ) );// -!F: 

    //qDebug() << "point 1";
    setupPage1();
    //qDebug() << "point 2";
    setupPage0();
    //qDebug() << "point 3";
    setupPage2();
    //qDebug() << "point 4";
    setupPage3();
    //qDebug() << "point 5";

    /*setNextEnabled( page_2, false );*/
    //page_2->enableButtonOk(false);// -!F: done: What is the right replacement of setNextEnabled( page_2, false ) ?
    
    /*helpButton()->hide();*/
    //enableLinkedHelp(false);// -!F: Is this the right replacement of helpButton()->hide() ?
    showButton(KDialog::Help, false);// -!F: Is this the right replacement of helpButton()->hide() ?
    
    //qDebug() << "point 6";
    connect( buttonCreate, SIGNAL( clicked() ), this, SLOT( create() ) );
    connect( buttonExample, SIGNAL( clicked() ), this, SLOT( example() ) );
    connect( checkDatabase, SIGNAL( clicked() ), this, SLOT( useDatabase() ) );
    //qDebug() << "point 7";
    page2FinishButtonEnable = false;
    //page2FinishButtonEnable = true;
    //page2NextButtonEnable = true;
    page2NextButtonEnable = false;
    page3FinishButtonEnable = true;
}

ConfAssistant::~ConfAssistant()
{ }

void ConfAssistant::accept()
{
    sqlwidget->save( checkDatabase->isChecked() );

    KAssistantDialog::accept();
}

void ConfAssistant::setupPage1()
{
    page = new QWidget( this );
    //pageLayout = new QVBoxLayout( page, 11, 6, "pageLayout");
    pageLayout = new QVBoxLayout(page);
    pageLayout->setContentsMargins(11, 11, 11, 11);
    pageLayout->setSpacing(6);
    pageLayout->setObjectName("pageLayout_1");

    //Layout8 = new QHBoxLayout( 0, 0, 6, "Layout8");
    Layout8 = new QHBoxLayout();
    Layout8->setContentsMargins(0, 0, 0, 0);
    Layout8->setSpacing(6);
    Layout8->setObjectName("Layout8");

    //Layout7 = new QVBoxLayout( 0, 0, 6, "Layout7");
    Layout7 = new QVBoxLayout();
    Layout7->setContentsMargins(0, 0, 0, 0);
    Layout7->setSpacing(6);
    Layout7->setObjectName("Layout7");

    logo = new QLabel( page );
    /*logo->setPixmap( KStandardDirs::locate("data", "kbarcode/logo.png") );*/// -!F: 
    logo->setPixmap( KStandardDirs::locate( "appdata", "logo.png" ) );// -!F: 
    QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
    pageLayout->addWidget( logo );
    Layout7->addItem( spacer );
    Layout8->addLayout( Layout7 );

    TextLabel2_2 = new QLabel( page );
    //TextLabel2_2->setMaximumWidth(700);// -!F: delete
    TextLabel2_2->setMinimumHeight(350);// -!F: keep
    TextLabel2_2->setWordWrap(true);
    TextLabel2_2->setText( i18n( "<qt><h1>Welcome to KBarcode</h1><br><br>") +
        i18n( description ) + "</qt>" );
    Layout8->addWidget( TextLabel2_2 );

    pageLayout->addLayout( Layout8 );

    KUrlLabel1 = new KUrlLabel( page );
    KUrlLabel1->setText( "http://www.kbarcode.net" );
    KUrlLabel1->setUrl("http://www.kbarcode.net");
    pageLayout->addWidget( KUrlLabel1 );
    KPageWidgetItem * page_1Item = addPage( page, i18n( "Welcome" ) );
    page_1Item->setObjectName("page_1");
}

void ConfAssistant::setupPage0()
{
    QWidget* page_0 = new QWidget( this );
    //QVBoxLayout* pageLayout = new QVBoxLayout( page_0, 11, 6, "pageLayout");
    pageLayout = new QVBoxLayout(page_0);
    pageLayout->setContentsMargins(11, 11, 11, 11);
    pageLayout->setSpacing(6);
    pageLayout->setObjectName("pageLayout_0");

    //QTextBrowser* b = new QTextBrowser( page_0, "b" );
    QTextBrowser* b = new QTextBrowser(page_0);
    b->setObjectName("b");
    b->setText( MainWindow::systemCheck() );

    pageLayout->addWidget( b );
    
    KPageWidgetItem * page_0Item = addPage( page_0, i18n("System Check") );
    page_0Item->setObjectName("page_0");
}

void ConfAssistant::setupPage2()
{
    /*page_2 = new QWidget( this );*/
    page_2 = new QWidget( this );
    /*pageLayout_2 = new QVBoxLayout( page_2, 11, 6, "pageLayout_2");*/
    pageLayout_2 = new QVBoxLayout(page_2);
    pageLayout_2->setContentsMargins(11, 11, 11, 11);
    pageLayout_2->setSpacing(6);
    pageLayout_2->setObjectName("pageLayout_2");

    //qDebug() << "point b 1";
    checkDatabase = new QCheckBox( page_2 );
    checkDatabase->setText( i18n("&Use database with KBarcode") );
    checkDatabase->setChecked( true );/**/

    //qDebug() << "point b 2";
    sqlwidget = new SqlWidget( true, page_2, "sqlwidget" );
    connect( sqlwidget, SIGNAL( databaseWorking( bool ) ), this, SLOT( testSettings( bool ) ) );
    
    //qDebug() << "point b 3";
    QSpacerItem* spacer_5 = new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
    pageLayout_2->addWidget( checkDatabase );
    pageLayout_2->addWidget( sqlwidget );
    /*qDebug() << "point b 3 1";*/
    //qDebug() << "point b 3 2";
    pageLayout_2->addItem( spacer_5 );
    //qDebug() << "point b 3 3";
    
    page_2->setLayout(pageLayout_2);

    KPageWidgetItem * page_2Item = addPage( page_2, i18n( "Database" ) );
    page_2Item->setObjectName("page_2");
}

void ConfAssistant::setupPage3()
{
    //qDebug() << "point setupPage3 1";
    page_3 = new QWidget( this );
    //pageLayout_3 = new QVBoxLayout( page_3, 11, 6, "pageLayout_3");
    pageLayout_3 = new QVBoxLayout(page_3);
    pageLayout_3->setContentsMargins(11, 11, 11, 11);
    pageLayout_3->setSpacing(6);
    pageLayout_3->setObjectName("pageLayout_3");

    TextLabel1_2 = new QLabel( page_3 );
    TextLabel1_2->setText( i18n( "KBarcode can create the required SQL tables for you.<br>KBarcode will add also some Label Definitions to the tables.<br>After that you can fill the tables with some example data." ) );
    //TextLabel1_2->setAlignment( int( Qt::WordBreak | Qt::AlignVCenter ) );
    TextLabel1_2->setAlignment(Qt::AlignVCenter);
    TextLabel1_2->setWordWrap(true);
    pageLayout_3->addWidget( TextLabel1_2 );

    //Layout5_2 = new QVBoxLayout( 0, 0, 6, "Layout5_2");
    Layout5_2 = new QVBoxLayout();
    Layout5_2->setContentsMargins(0, 0, 0, 0);
    Layout5_2->setSpacing(6);
    Layout5_2->setObjectName("Layout5_2");

    buttonCreate = new KPushButton( page_3 );
    buttonCreate->setText( i18n( "&Create Tables" ) );
    Layout5_2->addWidget( buttonCreate );

    buttonExample = new KPushButton( page_3 );
    buttonExample->setEnabled( false );
    buttonExample->setText( i18n( "&Add Example Data" ) );
    Layout5_2->addWidget( buttonExample );
    QSpacerItem* spacer_6 = new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Layout5_2->addItem( spacer_6 );
    pageLayout_3->addLayout( Layout5_2 );
    KPageWidgetItem * page_3Item = addPage( page_3, i18n( "Create Tables" ) );
    page_3Item->setObjectName("page_3");
}

void ConfAssistant::testSettings( bool b )
{
    /*setNextEnabled( page_2, b );*/// -!F: done: What is the right replacement of setNextEnabled( page_2, b ) ?
    page2NextButtonEnable = b;
    enableButton(KDialog::User2, page2NextButtonEnable);
}

void ConfAssistant::create()
{
    KApplication::setOverrideCursor( Qt::WaitCursor );
    if(!SqlTables::getInstance()->newTables( sqlwidget->username(), sqlwidget->password(), sqlwidget->hostname(), sqlwidget->database(), sqlwidget->driver() ) )
    {
        KApplication::restoreOverrideCursor();
        return;
    }
    else
        KApplication::restoreOverrideCursor();

    QSqlDatabase dbInstance = QSqlDatabase::addDatabase( sqlwidget->driver() );
    QSqlDatabase* db = & dbInstance;
    db->setDatabaseName( sqlwidget->database() );
    db->setUserName( sqlwidget->username() );
    db->setPassword( sqlwidget->password() );
    db->setHostName( sqlwidget->hostname() );

    if( !db->open() )
        KMessageBox::error( this, i18n("<qt>Connection failed:<br>") + sqlwidget->database(),
              db->lastError().databaseText() + "</qt>" );

    if( db->open() ) {
        KApplication::setOverrideCursor( Qt::WaitCursor );
        SqlTables::getInstance()->importData(
            KStandardDirs::locate("appdata", "labeldefinitions.sql"), db );
        buttonExample->setEnabled( true );
        KApplication::restoreOverrideCursor();
    }

    db->close();
}

void ConfAssistant::example()
{
    QSqlDatabase dbInstance = QSqlDatabase::addDatabase( sqlwidget->driver() );
    QSqlDatabase* db = & dbInstance;
    db->setDatabaseName( sqlwidget->database() );
    db->setUserName( sqlwidget->username() );
    db->setPassword( sqlwidget->password() );
    db->setHostName( sqlwidget->hostname() );

    if( !db->open() )
        KMessageBox::error( this, i18n("<qt>Connection failed:<br>") + sqlwidget->database(),
              db->lastError().databaseText() + "</qt>" );


    SqlTables::getInstance()->importData(
        KStandardDirs::locate("appdata", "exampledata.sql"), db );
    KMessageBox::information( this, i18n("Example data has been imported.") );

    db->close();
}

/* configureCurrentPage() is called by the slots next() and back() */
void ConfAssistant::configureCurrentPage( KPageWidgetItem * page )
{
    //qDebug() << page->objectName();
    if( page->objectName() == QString("page_2") && !sqlwidget->driverCount() ) {
        KMessageBox::information( this, i18n(
            "There are no Qt SQL drivers installed. "
            "KBarcode needs those drivers to access the different SQL databases. "
            "This drivers are part of the Qt Source distribution and should also be part of "
            "your distribution. Please install them first.") );
    }
    
    if( page->objectName() == QString("page_2") ) {
        enableButton(KDialog::User1, page2FinishButtonEnable);
        enableButton(KDialog::User2, page2NextButtonEnable);
        //qDebug() << "page == page_2";
    }
    
    if ( page->objectName() == QString("page_3") ) {
        /*finishButton()->setEnabled( true );*/
	//showButton(KDialog::User1, true);
        //enableButton(KDialog::User1, true);
	enableButton(KDialog::User1, page3FinishButtonEnable);
	//qDebug() << "page == page_3";
    }
}

/* This slot is called when a user clicks the next button.*/
void ConfAssistant::next() 
{
    KAssistantDialog::next();
    
    configureCurrentPage(currentPage());
}

/* This slot is called when a user clicks the next button.*/
void ConfAssistant::back() 
{
    KAssistantDialog::back();
    
    configureCurrentPage(currentPage());
}

void ConfAssistant::useDatabase()
{
    /*setFinishEnabled( page_2, !checkDatabase->isChecked() );// -!F: done: I must probably modify the structure of this class and it's use to achieve the same result?
    setNextEnabled( page_2, false );
    setFinishEnabled( page_3, checkDatabase->isChecked() );// -!F: done: In the original code this IMHO conflicts with showPage().
    sqlwidget->setEnabled( checkDatabase->isChecked() );*/
    page2FinishButtonEnable = (!checkDatabase->isChecked());
    page2NextButtonEnable = false;
    //page2NextButtonEnable = !page2NextButtonEnable;
    enableButton(KDialog::User1, page2FinishButtonEnable);
    enableButton(KDialog::User2, page2NextButtonEnable);
    page3FinishButtonEnable = (checkDatabase->isChecked());// -!F: done: In the original code this IMHO conflicts with showPage().
    //enableButton(KDialog::User1, !checkDatabase->isChecked());
    //enableButton(KDialog::User2, false);
    sqlwidget->setEnabled( checkDatabase->isChecked() );
}


#include "confassistant.moc"
