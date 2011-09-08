/* mainwindow.cpp - KBarcode Main Window Base
 * 
 * Copyright 2003-2008 Dominik Seichter, domseichter@web.de
 * Copyright 2008 Váradi Zsolt Gyula, karmaxxl@gmail.com
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "mainwindow.h"
/*#include "sqltables.h"
#include "confassistant.h"
#include "printersettings.h"
#include "kbarcodesettings.h"
#include "barkode.h"
*/
// Qt includes
#include <qcheckbox.h>
#include <q3textbrowser.h>
#include <qsqldatabase.h>

// KDE includes
#include <kaction.h>
#include <kapplication.h>
#include <kconfiggroup.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <krun.h>
#include <kglobal.h>
#include <ktoolinvocation.h>

bool MainWindow::autoconnect = true;
bool MainWindow::startassistant = true;

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags f)
    : KMainWindow(parent,f)
{
    connectAct = 0;
    first = false;
    loadConfig();

    setAutoSaveSettings( QString("Window") + name, true );
    connect( kapp, SIGNAL( aboutToQuit() ), this, SLOT( saveConfig() ) );

    if( first && startassistant ) {
        assistant();
        startassistant = false;
    }
    
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupActions()
{
    KAction* quitAct = KStandardAction::quit(kapp, SLOT(quit()), actionCollection());
    KAction* closeAct = KStandardAction::close( this, SLOT( close() ), actionCollection(), "close" );
    /*KAction* configureAct = KStandardAction::preferences( KBarcodeSettings::getInstance(), SLOT(configure()), actionCollection() );*/
    KAction* assistantAct = new KAction( i18n("&Start Configuration Assistant..."), BarIcon("assistant"), 0, this,
                                SLOT(assistant()), actionCollection(), "assistant" );
    /*connectAct = new KAction(i18n("&Connect to Database"), BarIcon("connect_no"), 0, this, SLOT(connectMySQL()),
                                actionCollection(),"connect" );*/


    KAction* newTablesAct = new KAction( i18n("&Create Tables"), "", 0, this,
                                SLOT(newTables()), actionCollection(), "tables" );

    /*importLabelDefAct = new KAction( i18n("&Import Label Definitions"), "", 0, SqlTables::getInstance(),
                                SLOT(importLabelDef()), actionCollection(), "import" );

    importExampleAct = new KAction( i18n("&Import Example Data"), "", 0, SqlTables::getInstance(),
                                SLOT(importExampleData()), actionCollection(), "import" );*/
                                
    KMenu* file = new KMenu( this );
    KMenu* settings = new KMenu( this );
    KMenu* hlpMenu = helpMenu();
    int helpid = hlpMenu->idAt( 0 );
    hlpMenu->removeItem( helpid );
    hlpMenu->insertItem( SmallIconSet("contents"), i18n("&Help"),
        this, SLOT(appHelpActivated()), Qt::Key_F1, -1, 0 );
    hlpMenu->insertSeparator(-1);
    hlpMenu->insertItem( i18n("&Action Map..."), this, SLOT( slotFunctionMap() ), 0, -1, 0 );
    hlpMenu->insertSeparator(-2);
    hlpMenu->insertItem( SmallIconSet("system"), i18n("&System Check..."), this, SLOT(showCheck() ), 0, -1, 0 );
    hlpMenu->insertItem( SmallIconSet("barcode"), i18n("&Barcode Help..."), this, SLOT( startInfo() ), 0, -1, 0 );
    hlpMenu->insertItem( i18n("&Donate..."), this, SLOT( donations() ), 0, -1, 0 );

    menuBar()->insertItem( i18n("&File"), file );
    menuBar()->insertItem( i18n("&Settings"), settings );
    menuBar()->insertItem( i18n("&Help"), hlpMenu );

    closeAct->plug( file );
    quitAct->plug( file );

    /*configureAct->plug( settings );
    assistantAct->plug( settings );
    connectAct->plug( settings );*/
    (new KActionSeparator( this ))->plug( settings );
    newTablesAct->plug( settings );
    /*importLabelDefAct->plug( settings );
    importExampleAct->plug( settings );*/

    /*SqlTables* tables = SqlTables::getInstance();
    if( tables->getData().autoconnect && autoconnect && !first ) {
        tables->connectMySQL();
        autoconnect = false;
    }

    connectAct->setEnabled( !SqlTables::isConnected() );
    importLabelDefAct->setEnabled( !connectAct->isEnabled() );
    importExampleAct->setEnabled( !connectAct->isEnabled() );*/
}
/*
void MainWindow::loadConfig()
{
    KConfigGroup config = KGlobal::config()->group("Assistant");

    first = config.readEntry("firststart2", true );

    SqlTables* tables = SqlTables::getInstance();
    if( tables->getData().autoconnect && !first && autoconnect && connectAct ) {
        connectAct->setEnabled( !tables->connectMySQL() );
        importLabelDefAct->setEnabled( !connectAct->isEnabled() );
        importExampleAct->setEnabled( !connectAct->isEnabled() );

        autoconnect = false;
    }

    KBarcodeSettings::getInstance()->loadConfig();
}

void MainWindow::saveConfig()
{
    KConfigGroup config = KGlobal::config()->group("Assistant");

    config.writeEntry("firststart2", false );

    PrinterSettings::getInstance()->saveConfig();
    SqlTables::getInstance()->saveConfig();
    KBarcodeSettings::getInstance()->saveConfig();

    config.sync();
}

void MainWindow::assistant()
{
    // FIXME: create an assistant
    ConfAssistant* wiz = new ConfAssistant( 0, "wiz", true );
    if( wiz->exec() == QDialog::Accepted && wiz->checkDatabase->isChecked() )
        SqlTables::getInstance()->connectMySQL();

    delete wiz;
}

void MainWindow::connectMySQL()
{
    connectAct->setEnabled( !SqlTables::getInstance()->connectMySQL() );
    importLabelDefAct->setEnabled( !connectAct->isEnabled() );
    importExampleAct->setEnabled( !connectAct->isEnabled() );

    if( !connectAct->isEnabled() )
        emit connectedSQL();
}
*/
void MainWindow::appHelpActivated()
{
	// TODO: the documentation should be rewritten, along with this link
    KMessageBox::information( this, i18n(
        "<qt>The KBarcode2 documentation is available as PDF for download on our webpage.<br><br>") +
        "<a href=\"http://www.kbarcode.net/17.0.html\">" +
        i18n("Download Now") + "</a></qt>",
        QString::null, QString::null, KMessageBox::AllowLink );
}

void MainWindow::showCheck()
{
    QTextBrowser* b = new QTextBrowser( 0, "b" );
    b->setText( MainWindow::systemCheck() );
    b->resize( 320, 240 );
    b->show();
}

void MainWindow::startInfo()
{
	QString info = locate("appdata", "barcodes.html");
    if( !info.isEmpty() )
        KToolInvocation::invokeBrowser( info );
}
/*
bool MainWindow::newTables()
{
    return SqlTables::getInstance()->newTables();
}
*/
void MainWindow::donations()
{
    // orig =https://www.paypal.com/xclick/business=domseichter%40web.de&item_name=Support+KBarcode+Development&item_number=0&image_url=http%3A//www.kbarcode.net/themes/DeepBlue/images/logo.gif&no_shipping=1&return=http%3A//www.kbarcode.net&cancel_return=http%3A//www.kbarcode.net&cn=Suggestions%2C+Comments%3F&tax=0&currency_code=EUR
    QString url = "https://www.paypal.com/xclick/business=domseichter@web.de&item_name=Support+KBarcode+Development&item_number=0&image_url=www.kbarcode.net/themes/DeepBlue/images/logo.gif&no_shipping=1&return=www.kbarcode.net&cancel_return=www.kbarcode.net&cn=Suggestions,+Comments,&tax=0&currency_code=EUR";
    
    KMessageBox::information( this, i18n(
        "<qt>It is possible to support the further development of KBarcode through donations. "
        "PayPal will be used for processing the donation.<br><br>" ) +
        "<a href=\"" + url + "\">" +  
        i18n("Donate Now") + "</a></qt>", QString::null, QString::null, KMessageBox::AllowLink );
}
/*
QString MainWindow::systemCheck()
{
	// TODO: break i18n puzzles
	// TODO: rename Barkode to something more visual
	
    bool gnubarcode = !Barkode::haveGNUBarcode();
    bool pdf = !Barkode::havePDFBarcode();
    bool tbarcode = !Barkode::haveTBarcode();
    bool tbarcode2 = !Barkode::haveTBarcode2();
    bool pure = !Barkode::havePurePostscriptBarcode();

    QString text;

    text.append( i18n("<p><h3>Barcode Support</h3></p>") );
    text.append( "<p>GNU Barcode: ");
    text.append(  gnubarcode ? i18n("<b>No</b><br />") : i18n("<b>Found</b><br />") );
    text.append( "PDF417 Encode: ");
    text.append( pdf ? i18n("<b>No</b><br />") : i18n("<b>Found</b><br />") );
    text.append( "TBarcode: ");
    text.append( tbarcode ? i18n("<b>No</b><br />") : i18n("<b>Found</b><br />") );
    text.append( "TBarcode2: ");
    text.append( tbarcode2 ? i18n("<b>No</b><br />") : i18n("<b>Found</b><br />") );
    text.append( "Pure Postscript Barcode Writer: ");
    text.append( pure ? i18n("<b>No</b><br />") : i18n("<b>Found</b><br />") );

    if( gnubarcode && tbarcode && pdf )
        text.append( i18n("<p>To get <b>barcode support</b> you have to either install <i>GNU Barcode</i>, <i>TBarcode</i> or <i>PDF417 Enc</i>.</p>") );

    text.append( i18n("<p><h3>Database Support</h3></p>") );

    QStringList list = QSqlDatabase::drivers();

    if( list.count() ) {
        text.append( "<ul>" );
        QStringList::Iterator it = list.begin();
        while( it != list.end() ) {
            text.append( i18n("<li>Driver found: ") + *it + "</li>" );
            ++it;
        }
        text.append( "</ul>" );
    } else
        text.append( i18n("<p><b>No database drivers found. SQL database support is disabled.</b></p>") );

    return text;
}
*/
#include "mainwindow.moc"
