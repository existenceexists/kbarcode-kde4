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
#include <QString>

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
#include <KActionCollection>
#include <KStandardAction>

// -!F:
#include <QDebug>
#include <QList>
#include <QKeySequence>
#include <QShortcut>

bool MainWindow::autoconnect = true;
bool MainWindow::startassistant = true;

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags f)
    : KXmlGuiWindow(parent,f)
{
    connectAct = 0;
    first = false;
    loadConfig();

    setAutoSaveSettings( QString("Window") + QString(objectName()), true );
    connect( kapp, SIGNAL( aboutToQuit() ), this, SLOT( saveConfig() ) );

    if( first && startassistant ) {
        /*assistant();*/
        startassistant = false;
    }
    
}

MainWindow::~MainWindow()
{
}
// -!F:
void MainWindow::setupActions(QString directoryName=QString())
{
    kbarcodeDirectoryName = directoryName;// -!F:
    
    /*KAction* quitAct = KStandardAction::quit(kapp, SLOT(quit()), actionCollection());*/
    KStandardAction::quit(kapp, SLOT(quit()), actionCollection());
    /*KAction* closeAct = KStandardAction::close( this, SLOT( close() ), actionCollection());*/
    KStandardAction::close( this, SLOT( close() ), actionCollection());
    /*KAction* configureAct = KStandardAction::preferences( KBarcodeSettings::getInstance(), SLOT(configure()), actionCollection() );*/
    /*KAction* assistantAct = new KAction( i18n("&Start Configuration Assistant..."), BarIcon("assistant"), 0, this,
                                SLOT(assistant()), actionCollection());*/
    KAction* assistantAct = new KAction(this);
    assistantAct->setText(i18n("&Start Configuration Assistant..."));
    assistantAct->setIcon(BarIcon("/usr/share/app-install/icons/assistant.png"));// -!F:
    actionCollection()->addAction("assistant", assistantAct);
    connect(assistantAct, SIGNAL(triggered(bool)), this, SLOT(assistant()));
    
    /*connectAct = new KAction(i18n("&Connect to Database"), BarIcon("connect_no"), 0, this, SLOT(connectMySQL()),
                                actionCollection(),"connect" );*/
    connectAct = new KAction(this);
    connectAct->setText(i18n("&Connect to Database"));
    connectAct->setIcon(BarIcon("/usr/share/icons/crystalsvg/16x16/actions/connect_no.png"));// -!F:
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

    /*importExampleAct = new KAction( i18n("&Import Example Data"), "", 0, SqlTables::getInstance(),
                                SLOT(importExampleData()), actionCollection(), "import" );*/
                                
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
    
    /*KMenu* hlpMenu = helpMenu();
    QAction* firstAction = hlpMenu->actions()[0];
    hlpMenu->removeAction(firstAction);
    menuBar()->removeAction(hlpMenu->menuAction());
    
    KAction* helpAct = new KAction(this);
    helpAct->setText(i18n("&Help"));
    helpAct->setIcon(KIcon("help-contents"));// -!F:
    helpAct->setShortcut(Qt::Key_F1);
    actionCollection()->addAction("helpAct", helpAct);
    connect(helpAct, SIGNAL(triggered(bool)), this, SLOT(appHelpActivated()));
    
    hlpMenu->insertAction(hlpMenu->actions()[0], helpAct);
    menuBar()->addMenu(hlpMenu);*/
    

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

    /*SqlTables* tables = SqlTables::getInstance();
    if( tables->getData().autoconnect && autoconnect && !first ) {
        tables->connectMySQL();
        autoconnect = false;
    }

    connectAct->setEnabled( !SqlTables::isConnected() );
    importLabelDefAct->setEnabled( !connectAct->isEnabled() );
    importExampleAct->setEnabled( !connectAct->isEnabled() );*/
    
    // Set window icon. -!F:
    //setWindowIcon(KIcon(this->kbarcodeDirectoryName + QString("/hi16-app-kbarcode.png")));
    if (KGlobal::dirs()->addResourceDir(
            "appdata", QString("/home/fanda/tmp/share/apps/") +
            this->kbarcodeDirectoryName)) {
        setWindowIcon(KIcon(KStandardDirs::locate(
            "appdata", QString("hi16-app-kbarcode.png"))));
    }
    /*setWindowIcon(KIcon(KStandardDirs::locate(
            "appdata", QString("share/apps/") + this->kbarcodeDirectoryName +
            QString("/hi16-app-kbarcode.png"))));*/
    /*setWindowIcon(KIcon(QString("share/apps/") + this->kbarcodeDirectoryName +
            QString("/hi16-app-kbarcode.png")));
    qDebug() << QString("share/apps/") + this->kbarcodeDirectoryName +
            QString("/hi16-app-kbarcode.png");
    qDebug() << "KStandardDirs::locate = " << KStandardDirs::locate(
            "appdata", QString("share/apps/") + this->kbarcodeDirectoryName +
            QString("/hi16-app-kbarcode.png"));*/
    
    //QWidget * mainMenuBar = menuWidget();
    //const QString helpName = QString("helpMenu");
    //QString("help")
    //QList<KMenu *> 
    /*if (!(menuBar()->findChildren<KMenu *>()).isEmpty()) {
        qDebug() << "helpMenu found";
    }*/
    //KMenu* hlpMenu1 = helpMenu();
    /*QList<QAction *> helpMenuActionsList1 = hlpMenu1->actions();
    qDebug() << "helpMenuActionsList1 size = " << helpMenuActionsList1.size();
    hlpMenu1->removeAction(helpMenuActionsList1[0]);*/
    
    KAction* helpAct = new KAction(this);
    helpAct->setText(i18n("&Help"));
    helpAct->setIcon(KIcon("help-contents"));
    helpAct->setShortcut(Qt::Key_F1);
    actionCollection()->addAction("helpAct", helpAct);
    connect(helpAct, SIGNAL(triggered(bool)), this, SLOT(appHelpActivated()));
    
    //helpMenu()->insertAction(helpMenu()->actions()[0], helpAct);
    //hlpMenu1->insertAction(hlpMenu1->actions()[0], helpAct);
    
    /*QList<QAction *> menuBarActionsList = menuBar()->actions();
    //qDebug() << "menuBarActionsList size = " << menuBarActionsList.size();*/
    //menuBar()->clear();
    //menuBar()->addMenu(menuBarChildrenList[0]);
    /*menuBar()->addMenu(menuBarChildrenList[1]);*/
    /*menuBar()->addAction(menuBarActionsList[0]);
    menuBar()->addAction(menuBarActionsList[1]);*/
    //menuBar()->addMenu(hlpMenu1);
    
    /*QShortcut f1Shortcut(helpMenu()->actions()[0]->shortcut(), this);
    QKeySequence f1ShortcutKeySequence = f1Shortcut.key();
    //helpMenu()->actions()[0]->setShortcut(QKeySequence());
    //releaseShortcut(QShortcut(helpMenu()->actions()[0]->shortcut(), this).id());
    //qDebug() << "QShortcut" << helpMenu()->actions()[0]->shortcut().toString() << QShortcut(helpMenu()->actions()[0]->shortcut(), this).id();
    qDebug() << "F1 QShortcut" << f1ShortcutKeySequence.toString() << f1Shortcut.id();*/
    //QShortcut(helpMenu()->actions()[0]->shortcut(), this).disconnect();
    //QShortcut(helpMenu()->actions()[0]->shortcut(), this).setEnabled(false);
    
    /*KMenu* hlpMenu2 = helpMenu();
    QList<QAction *> helpMenuActionsList2 = hlpMenu2->actions();
    //KAction * firstAction = (KAction *) helpMenuActionsList[0];
    QAction * firstAction2 = helpMenuActionsList2[0];
    firstAction2->blockSignals(true);
    hlpMenu2->removeAction(firstAction2);
    actionCollection()->removeAction(firstAction2);*/
    
    //KStandardAction::help(this, SLOT(appHelpActivated()), actionCollection());
    //KStandardAction::helpContents(this, SLOT(appHelp()), actionCollection());
    /*KAction * helpAct = KStandardAction::helpContents(this, SLOT(appHelpActivated()), actionCollection());
    helpAct->setShortcut(QKeySequence());*/
    /*connect(helpAct, SIGNAL(triggered(bool)), this, SLOT(appHelpActivated()));*/
    /*KAction * helpAct = KStandardAction::create(KStandardAction::HelpContents,
        this, SLOT(appHelpActivated()), actionCollection());*/
    /*helpAct->setShortcut(QKeySequence(Qt::Key_F1));
    actionCollection()->addAction("helpAct", helpAct);
    connect(helpAct, SIGNAL(triggered(bool)), this, SLOT(appHelpActivated()));*/
    //KStandardAction::keyBindings( (QObject*) guiFactory(), SLOT( configureShortcuts() ), actionCollection() );
    
    setupGUI(Default, this->kbarcodeDirectoryName + QString("/mainwindowui.rc"));
    
    QAction *helpContentsAction = actionCollection()->action("help_contents");
    if (!(helpContentsAction == 0)) {
        helpContentsAction->setEnabled(false);
        delete helpContentsAction;
    };
    //KAction * helpAct = KStandardAction::helpContents(this, SLOT(appHelpActivated()), actionCollection());
    //helpMenu()->insertAction(helpMenu()->actions()[0], helpAct);
    //releaseShortcut(QShortcut(helpMenu()->actions()[0]->shortcut(), this).id());
    //releaseShortcut(QShortcut(helpMenu()->actions()[0]->shortcut(), this).id());
    /*if (!(f1Shortcut.disconnect())) {
        qDebug() << "QShortcut not disconnected";
    }
    //QShortcut(helpMenu()->actions()[0]->shortcut(), this).setEnabled(false);
    f1Shortcut.setEnabled(false);
    
    //if ((QShortcut(helpMenu()->actions()[0]->shortcut(), this).isEnabled())) {
    if (f1Shortcut.isEnabled()) {
        qDebug() << "F1 QShortcut is enabled";
    }*/
    //QObject::disconnect(QShortcut(helpMenu()->actions()[0]->shortcut(), this), 0, 0, 0);
    
    
    //KAction * action1 = (KAction *) helpMenu()->actions()[0];
    //KShortcut f1Shortcut2 = action1->shortcut();
    
    
    /*KMenu* hlpMenu = helpMenu();*/
    /*if ((hlpMenu->actions()).isEmpty()) {
        qDebug() << "helpMenu has no actions";
    }*/
    /*QAction* firstAction = (hlpMenu->actions())[0];
    hlpMenu->removeAction(firstAction);*/
    //menuBar()->removeAction(hlpMenu->menuAction());*/
    /*QList<QAction *> helpMenuActionsList = hlpMenu->actions();
    KAction * firstAction = (KAction *) helpMenuActionsList[0];*/
    //QAction * firstAction = helpMenuActionsList[0];
    
    /*KShortcut f1Shortcut2 = firstAction->shortcut();
    f1Shortcut2.remove(QKeySequence(Qt::Key_F1));
    f1Shortcut2.setPrimary(QKeySequence());
    f1Shortcut2.setAlternate(QKeySequence());
    firstAction->setShortcut(f1Shortcut2);*/
    //firstAction->setShortcut(QKeySequence());
    //firstAction->setShortcut(f1Shortcut);
    //firstAction->setShortcut(
    //firstAction->setVisible(false);
    /*if ((firstAction->shortcut()).isEmpty()) {
        qDebug() << "KShortcut is empty";
    }
    
    KShortcut helpShortcut = KStandardShortcut::help();
    helpShortcut.remove(QKeySequence(Qt::Key_F1));
    helpShortcut.setPrimary(QKeySequence());
    helpShortcut.setAlternate(QKeySequence());
    firstAction->setShortcut(helpShortcut);
    //firstAction->disconnect();
    //setShortcutEnabled(f1Shortcut.id(), false);
    //firstAction->blockSignals(true);
    hlpMenu->removeAction(firstAction);
    actionCollection()->removeAction(firstAction);
    //delete firstAction;
    //firstAction->~QAction();
    //setShortcutEnabled(f1Shortcut.id(), false);*/
    
    /*KAction* helpAct = new KAction(this);
    helpAct->setText(i18n("&Help"));
    helpAct->setIcon(KIcon("help-contents"));// -!F:
    helpAct->setShortcut(QKeySequence(Qt::Key_F1));
    //helpAct->setShortcut(f1Shortcut2);
    //helpAct->setShortcut(firstAction->shortcut());
    actionCollection()->addAction("helpAct", helpAct);
    connect(helpAct, SIGNAL(triggered(bool)), this, SLOT(appHelpActivated()));*/
    
    //hlpMenu->insertAction(hlpMenu->actions()[0], helpAct);
    
    //menuBar()->removeAction(hlpMenu->menuAction());
    //menuBar()->removeAction(hlpMenu);
    //QList<QMenu *> menuBarChild = menuBar()->findChildren<QMenu *>();
    //qDebug() << "menuBarChild size = " << menuBarChild.size();
    //QList<QMenu *> menuBarChildrenList = menuBarChild[0]->findChildren<QMenu *>();
    //qDebug() << "menuBarChildrenList size = " << menuBarChildrenList.size();
    /*QList<QAction *> menuBarActionsList = menuBar()->actions();
    qDebug() << "menuBarActionsList size = " << menuBarActionsList.size();
    menuBar()->clear();
    //menuBar()->addMenu(menuBarChildrenList[0]);
    //menuBar()->addMenu(menuBarChildrenList[1]);
    menuBar()->addAction(menuBarActionsList[0]);
    menuBar()->addAction(menuBarActionsList[1]);
    menuBar()->addMenu(hlpMenu);
    menuBar()->addAction(menuBarActionsList[3]);*/
    
    //QShortcut f1Shortcut2(helpMenu()->actions()[0]->shortcut(), this);
    //setShortcutEnabled(f1Shortcut2.id(), false);
    
    /*KAction* helpAct = new KAction(this);
    helpAct->setText(i18n("&Help"));
    helpAct->setIcon(KIcon("help-contents"));// -!F:
    helpAct->setShortcut(Qt::Key_F1);
    actionCollection()->addAction("helpAct", helpAct);
    connect(helpAct, SIGNAL(triggered(bool)), this, SLOT(appHelpActivated()));
    
    hlpMenu->insertAction(hlpMenu->actions()[0], helpAct);
    //menuBar()->addMenu(hlpMenu);*/
    /*if ((menuBar()->findChildren<QMenu *>()).isEmpty()) {
        qDebug() << "children not found";
    }*/
    
    /*KMenu * hlpMenu = customHelpMenu();
    hlpMenu->insertAction(hlpMenu->actions()[0], helpAct);
    menuBar()->addMenu(hlpMenu);*/
    
    QList<QAction *> menuBarActionsList = menuBar()->actions();
    qDebug() << "menuBarActionsList size = " << menuBarActionsList.size();
    
    menuBar()->clear();
    
    KMenu * hlpMenu = customHelpMenu();
    hlpMenu->removeAction(hlpMenu->actions()[0]);
    hlpMenu->insertAction(hlpMenu->actions()[0], helpAct);
    
    menuBar()->addAction(menuBarActionsList[0]);
    menuBar()->addAction(menuBarActionsList[1]);
    menuBar()->addAction(menuBarActionsList[2]);
    menuBar()->addMenu(hlpMenu);
}

void MainWindow::loadConfig()
{
    /*KConfigGroup config = KGlobal::config()->group("Assistant");

    first = config.readEntry("firststart2", true );

    SqlTables* tables = SqlTables::getInstance();
    if( tables->getData().autoconnect && !first && autoconnect && connectAct ) {
        connectAct->setEnabled( !tables->connectMySQL() );
        importLabelDefAct->setEnabled( !connectAct->isEnabled() );
        importExampleAct->setEnabled( !connectAct->isEnabled() );

        autoconnect = false;
    }

    KBarcodeSettings::getInstance()->loadConfig();*/
}

void MainWindow::saveConfig()
{
    /*KConfigGroup config = KGlobal::config()->group("Assistant");

    config.writeEntry("firststart2", false );

    PrinterSettings::getInstance()->saveConfig();
    SqlTables::getInstance()->saveConfig();
    KBarcodeSettings::getInstance()->saveConfig();

    config.sync();*/
}

void MainWindow::assistant()
{
    // FIXME: create an assistant
    /*ConfAssistant* wiz = new ConfAssistant( 0, "wiz", true );
    if( wiz->exec() == QDialog::Accepted && wiz->checkDatabase->isChecked() )
        SqlTables::getInstance()->connectMySQL();

    delete wiz;*/
}

void MainWindow::connectMySQL()
{
    /*connectAct->setEnabled( !SqlTables::getInstance()->connectMySQL() );
    importLabelDefAct->setEnabled( !connectAct->isEnabled() );
    importExampleAct->setEnabled( !connectAct->isEnabled() );

    if( !connectAct->isEnabled() )
        emit connectedSQL();*/
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
    /*QTextBrowser* b = new QTextBrowser( 0, "b" );
    b->setText( MainWindow::systemCheck() );
    b->resize( 320, 240 );
    b->show();*/
}

void MainWindow::startInfo()
{
	QString info = KStandardDirs::locate("appdata", "barcodes.html");
    if( !info.isEmpty() )
        KToolInvocation::invokeBrowser( info );
}

bool MainWindow::newTables()
{
    /*return SqlTables::getInstance()->newTables();*/
    // Frank:
    return false;
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
	
    /*bool gnubarcode = !Barkode::haveGNUBarcode();
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

    return text;*/
    // Frank:
    return QString("Frank");
}

void MainWindow::slotFunctionMap()
{
    /*created by Frank*/
}

#include "mainwindow.moc"
