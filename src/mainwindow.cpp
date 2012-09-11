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
#include "sqltables.h"
#include "confassistant.h"
#include "printersettings.h"
#include "kbarcodesettings.h"
#include "barkode.h"

// Qt includes
#include <qcheckbox.h>
#include <QTextBrowser>
#include <qsqldatabase.h>
#include <QString>

// KDE includes
#include <kaction.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
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
#include <KActionCollection>
#include <KStandardAction>

// -!F: added by Frank, keep it:
#include <QList>
#include <QDebug>
#include <khelpmenu.h>

bool MainWindow::autoconnect = true;
bool MainWindow::startassistant = true;

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags f, Qt::WidgetAttribute waf)
    : KXmlGuiWindow(parent,f)
{
    setAttribute( waf );
    
    connectAct = 0;
    first = false;
    loadConfig();

    setAutoSaveSettings( QString("Window") + QString(objectName()), true );
    connect( kapp, SIGNAL( aboutToQuit() ), this, SLOT( saveConfig() ) );

    if( first && startassistant ) {
        assistant();
        startassistant = false;
    }
    
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupActions() // -!F:
{
    /*kbarcodeDirectoryName = directoryName;*/// -!F: delete
    
    /*KAction* quitAct = KStandardAction::quit(kapp, SLOT(quit()), actionCollection());*/
    KStandardAction::quit(kapp, SLOT(quit()), actionCollection());
    /*KAction* closeAct = KStandardAction::close( this, SLOT( close() ), actionCollection());*/
    KStandardAction::close( this, SLOT( close() ), actionCollection());
    /*KAction* configureAct = KStandardAction::preferences( KBarcodeSettings::getInstance(), SLOT(configure()), actionCollection() );*/
    KStandardAction::preferences( KBarcodeSettings::getInstance(), SLOT(configure()), actionCollection() );
    /*KAction* assistantAct = new KAction( i18n("&Start Configuration Assistant..."), BarIcon("assistant"), 0, this,
                                SLOT(assistant()), actionCollection());*/
    KAction* assistantAct = new KAction(this);
    assistantAct->setText(i18n("&Start Configuration Assistant..."));
    //assistantAct->setIcon(BarIcon("/usr/share/app-install/icons/assistant.png"));// -!F: delete
    //assistantAct->setIcon(BarIcon("/usr/share/icons/crystalsvg/16x16/actions/wizard.png"));// -!F: delete
    /*assistantAct->setIcon(BarIcon("/usr/share/icons/crystalsvg/16x16/actions/wizard.png"));*/// -!F: delete
    assistantAct->setIcon(BarIcon("tools-wizard"));// -!F: keep
    actionCollection()->addAction("assistant", assistantAct);
    connect(assistantAct, SIGNAL(triggered(bool)), this, SLOT(assistant()));
    
    /*connectAct = new KAction(i18n("&Connect to Database"), BarIcon("connect_no"), 0, this, SLOT(connectMySQL()),
                                actionCollection(),"connect" );*/
    connectAct = new KAction(this);
    connectAct->setText(i18n("&Connect to Database"));
    /*connectAct->setIcon(BarIcon("/usr/share/icons/crystalsvg/16x16/actions/connect_no.png"));*/// -!F: delete
    connectAct->setIcon(BarIcon("network-connect"));// -!F: keep
    actionCollection()->addAction("connect", connectAct);
    connect(connectAct, SIGNAL(triggered(bool)), this, SLOT(connectMySQL()));

    /*KAction* newTablesAct = new KAction( i18n("&Create Tables"), "", 0, this,
                                SLOT(newTables()), actionCollection(), "tables" );*/
    KAction* newTablesAct = new KAction(this);
    newTablesAct->setText(i18n("&Create Tables"));
    actionCollection()->addAction("tables", newTablesAct);
    connect(newTablesAct, SIGNAL(triggered(bool)), this, SLOT(newTables()));

    /*importLabelDefAct = new KAction( i18n("&Import Label Definitions"), "", 0, SqlTables::getInstance(),
                                SLOT(importLabelDef()), actionCollection(), "import" );*/
    importLabelDefAct = new KAction(this);
    importLabelDefAct->setText(i18n("&Import Label Definitions"));
    actionCollection()->addAction("importLabelDef", importLabelDefAct);
    connect(importLabelDefAct, SIGNAL(triggered(bool)), SqlTables::getInstance(), SLOT(importLabelDef()));

    /*importExampleAct = new KAction( i18n("&Import Example Data"), "", 0, SqlTables::getInstance(),
                                SLOT(importExampleData()), actionCollection(), "import" );*/
    importExampleAct = new KAction(this);
    importExampleAct->setText(i18n("&Import Example Data"));
    actionCollection()->addAction("importExample", importExampleAct);
    connect(importExampleAct, SIGNAL(triggered(bool)), SqlTables::getInstance(), SLOT(importExampleData()));
    
    /*KMenu* file = new KMenu( this );
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
    hlpMenu->insertItem( i18n("&Donate..."), this, SLOT( donations() ), 0, -1, 0 );*/
    
    /*menuBar()->insertItem( i18n("&File"), file );
    menuBar()->insertItem( i18n("&Settings"), settings );
    menuBar()->insertItem( i18n("&Help"), hlpMenu );*/

    /*closeAct->plug( file );
    quitAct->plug( file );*/

    /*configureAct->plug( settings );
    assistantAct->plug( settings );
    connectAct->plug( settings );*/
    /*(new KActionSeparator( this ))->plug( settings );
    newTablesAct->plug( settings );
    importLabelDefAct->plug( settings );
    importExampleAct->plug( settings );*/

    SqlTables* tables = SqlTables::getInstance();
    if( tables->getData().autoconnect && autoconnect && !first ) {
        tables->connectMySQL();
        autoconnect = false;
    }

    connectAct->setEnabled( !SqlTables::isConnected() );
    importLabelDefAct->setEnabled( !connectAct->isEnabled() );
    importExampleAct->setEnabled( !connectAct->isEnabled() );
    
    // Set window icon.
    //setWindowIcon(KIcon(this->kbarcodeDirectoryName + QString("/hi16-app-kbarcode.png")));
    /*if (!KGlobal::dirs()->addResourceDir(
            "appdata", QString("/home/fanda/programovani/c++/frank_scripts/kbarcode/executables/share/apps/") + // -!F:
            this->kbarcodeDirectoryName)) {
        qDebug() << QString("The addition of the following folder as a resource dir failed: ") 
            + QString("/home/fanda/programovani/c++/frank_scripts/kbarcode/executables/share/apps/");
    }*/// -!F: delete
    setWindowIcon( KIcon( KStandardDirs::locate( "appdata", QString("hi16-app-kbarcode.png") ) ) );
    
    // Let KDE4 create the main window.
    /*setupGUI(Default, this->kbarcodeDirectoryName + QString("/mainwindowui.rc"));*/// -!F: delete
    setupGUI(Default, KStandardDirs::locate( "appdata", QString("mainwindowui.rc")));
    
    createCustomHelpMenu();
}

void MainWindow::createCustomHelpMenu()
{
    /* Adjust the help menu of the window's menu bar created automatically by setupGUI():*/
    
    KAction* helpAct = new KAction(this);
    helpAct->setText(i18n("&Help"));
    helpAct->setIcon(KIcon("help-contents"));
    helpAct->setShortcut(Qt::Key_F1);
    actionCollection()->addAction("helpAct", helpAct);
    connect(helpAct, SIGNAL(triggered(bool)), this, SLOT(appHelpActivated()));
    
    /*// actionMapAct made by Frank:
    KAction* actionMapAct = new KAction(this);
    actionMapAct->setText(i18n("&Action Map..."));
    actionCollection()->addAction("actionMapAct", actionMapAct);
    connect(actionMapAct, SIGNAL(triggered(bool)), this, SLOT(slotFunctionMap()));*/
    
    KAction* systemCheckAct = new KAction(this);
    systemCheckAct->setText(i18n("&System Check..."));
    /*systemCheckAct->setIcon(KIcon("/usr/share/icons/crystalsvg/16x16/devices/system.png"));*/// -!F:
    systemCheckAct->setIcon(KIcon("computer"));// -!F: keep
    actionCollection()->addAction("systemCheckAct", systemCheckAct);
    connect(systemCheckAct, SIGNAL(triggered(bool)), this, SLOT(showCheck()));
    
    KAction* barcodeHelpAct = new KAction(this);
    barcodeHelpAct->setText(i18n("&Barcode Help..."));
    barcodeHelpAct->setIcon(KIcon("view-barcode"));// -!F: keep
    actionCollection()->addAction("barcodeHelpAct", barcodeHelpAct);
    connect(barcodeHelpAct, SIGNAL(triggered(bool)), this, SLOT(startInfo()));
    
    KAction* donateAct = new KAction(this);
    donateAct->setText(i18n("&Donate..."));
    actionCollection()->addAction("donateAct", donateAct);
    connect(donateAct, SIGNAL(triggered(bool)), this, SLOT(donations()));
    
    
    QList<QAction *> menuBarActionsList = menuBar()->actions();
    
    QAction * oldHelpMenu = menuBarActionsList.takeAt( menuBarActionsList.size() - 1 );// Remove help menu.
    QList<QAction *> oldHelpMenuActionsList = oldHelpMenu->menu()->actions();
    for( int i = 0; i < oldHelpMenuActionsList.size(); i++ ) {// Avoid memory leaks and ambiguous shortcuts.
        oldHelpMenuActionsList[i]->setEnabled( false );
        delete oldHelpMenuActionsList[i];
    }
    delete oldHelpMenu->menu();
    
    menuBar()->clear();
    
    KHelpMenu * helpMenuTmp = new KHelpMenu( this, KCmdLineArgs::aboutData() );
    KMenu * hlpMenu = helpMenuTmp->menu();
    hlpMenu->removeAction(hlpMenu->actions()[0]);
    //hlpMenu->insertAction(hlpMenu->actions()[0], actionMapAct);// Action Map was removed in the last published version
    hlpMenu->insertAction(hlpMenu->actions()[1], systemCheckAct);
    hlpMenu->insertSeparator(hlpMenu->actions()[1]);
    hlpMenu->insertAction(hlpMenu->actions()[1], donateAct);
    hlpMenu->insertSeparator(hlpMenu->actions()[1]);
    hlpMenu->insertAction(hlpMenu->actions()[0], barcodeHelpAct);
    hlpMenu->insertAction(hlpMenu->actions()[0], helpAct);
    hlpMenu->actions()[12]->setIcon( KIcon ( KStandardDirs::locate(
            "appdata", QString( "hi16-app-kbarcode.png" ) ) ) );
    
    for( int i = 0; i < menuBarActionsList.size(); i++ ) {
        menuBar()->addAction( menuBarActionsList[i] );
    }
    menuBar()->addMenu( hlpMenu );
}

void MainWindow::loadConfig()
{
    KConfigGroup config = KGlobal::config()->group("Assistant");

    bool first = config.readEntry("firststart2", true );

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
    ConfAssistant* wiz = new ConfAssistant( 0, QString("wiz"), true );
    if( wiz->exec() == QDialog::Accepted && wiz->checkDatabase->isChecked() ) {
        connectAct->setEnabled( !SqlTables::getInstance()->connectMySQL() );
        importLabelDefAct->setEnabled( !connectAct->isEnabled() );
        importExampleAct->setEnabled( !connectAct->isEnabled() );
    }

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
    QTextBrowser* b = new QTextBrowser( 0 );
    b->setObjectName("b");
    b->setText( MainWindow::systemCheck() );
    b->resize( 320, 240 );
    b->show();
}

void MainWindow::startInfo()
{
    /*QString info = KStandardDirs::locate("appdata", "barcodes.html");// -!F: uncomment this and comment out the following locate():*/
    QString info = KStandardDirs::locate("appdata", 
        "/home/fanda/programovani/c++/frank_scripts/kbarcode/downloads/kbarcode-2.0.7/kbarcode/barcodes.html");// -!F:
    if( !info.isEmpty() )
        KToolInvocation::invokeBrowser( info );
}

bool MainWindow::newTables()
{
    return SqlTables::getInstance()->newTables();
}

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

#include "mainwindow.moc"
