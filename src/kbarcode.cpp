/***************************************************************************
                          kbarcode.cpp  -  description
                             -------------------
    begin                : Mit Jan 15 2003
    copyright            : (C) 2003 by Dominik Seichter
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

#include "kbarcode.h"
#include "barkode.h"
/*#include "barcodegenerator.h"*/
#include "batchassistant.h"
/*#include "labeleditor.h"
#include "databasebrowser.h"
#include "csvimportdlg.h"*/
#include "sqltables.h"
#include "kbarcodesettings.h"

// Qt includes
// #include <q3groupbox.h>// -!F:
// #include <Qt3Support>
// #include <Q3GroupBox>
// #include <qlayout.h>
#include <QGroupBox>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QString>

// KDE includes
#include <kaction.h>
#include <KActionCollection>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kpushbutton.h>
#include <kmessagebox.h>

KBarcode::KBarcode( QWidget *parent, Qt::WFlags f)
    : MainWindow( parent, f )
{
    kbarcodeDirectoryName = QString("kbarcode009");// -!F: delete in the end
    
    QGroupBox* w = new QGroupBox(this);
    /*QVBoxLayout* layout = new QVBoxLayout(this);*/
    QVBoxLayout* vLayout = new QVBoxLayout();
    setCentralWidget( w );

    buttonSingle = new KPushButton( i18n("Barcode &Generator..."), w );
    buttonSingle->setEnabled( Barkode::haveBarcode() );
    buttonEditor = new KPushButton( i18n("&Label Editor..."), w );
    buttonBatch = new KPushButton( i18n("&Batch Printing..."), w );
    buttonData = new KPushButton( i18n("Edit SQL &Tables..."), w );
    buttonData->setEnabled( false );
    
    /*buttonSingle->setIconSet( BarIconSet( "barcode" ) );// -!F: original, delete
    buttonEditor->setIconSet( BarIconSet( "edit" ) );
    buttonBatch->setIconSet( BarIconSet( "fileprint" ) );*/
    /*KIconLoader *iconLoader = new KIconLoader;// -!F: delete
    iconLoader->addExtraDesktopThemes();
    buttonSingle->setIcon( KIcon( "view-barcode", iconLoader ) );
    buttonEditor->setIcon( KIcon( "document-edit", iconLoader ) );
    buttonBatch->setIcon( KIcon( "document-print", iconLoader ) );*/
    buttonSingle->setIcon( KIcon( "view-barcode" ) );
    buttonEditor->setIcon( KIcon( "document-edit" ) );
    buttonBatch->setIcon( KIcon( "document-print" ) );


    vLayout->addWidget( buttonSingle );
    vLayout->addWidget( buttonEditor );
    vLayout->addWidget( buttonBatch );
    vLayout->addWidget( buttonData );
    
    QLayout * wLayout = w->layout();
    if (wLayout == 0) {
        w->setLayout(vLayout);
    } else {
        wLayout->addItem(vLayout);
    }

    connect( buttonSingle, SIGNAL( clicked() ), this, SLOT( startBarcode() ) );
    connect( buttonEditor, SIGNAL( clicked() ), this, SLOT( startLabelEditor() ) );
    connect( buttonBatch, SIGNAL( clicked() ), this, SLOT( startBatchPrint() ) );
    connect( SqlTables::getInstance(), SIGNAL( connectedSQL() ), this, SLOT( enableData() ) );
    
    setupActions();
    show();

    /*KAction* editLabelDefAct = new KAction(i18n("&Edit Label Definitions"), "",// -!F: original, delete
                                0, this, SLOT(editLabelDef()), actionCollection(), "design" );*/
    KAction* editLabelDefAct = new KAction(this);
    editLabelDefAct->setText(i18n("&Edit Label Definitions"));
    actionCollection()->addAction("editLabelDefAct", editLabelDefAct);
    connect(editLabelDefAct, SIGNAL(triggered(bool)), this, SLOT(editLabelDef()));

    /*KAction* editArticleAct = new KAction(i18n("&Edit Articles"), "",// -!F: original, delete
                                0, this, SLOT(editArticles()), actionCollection(), "design" );*/
    KAction* editArticleAct = new KAction(this);
    editArticleAct->setText(i18n("&Edit Articles"));
    actionCollection()->addAction("editArticleAct", editArticleAct);
    connect(editArticleAct, SIGNAL(triggered(bool)), this, SLOT(editArticles()));

    /*KAction* editCustomerAct = new KAction(i18n("&Edit Customers"), "",// -!F: original, delete
                                0, this, SLOT(editCustomers()), actionCollection(), "design" );*/
    KAction* editCustomerAct = new KAction(this);
    editCustomerAct->setText(i18n("&Edit Customers"));
    actionCollection()->addAction("editCustomerAct", editCustomerAct);
    connect(editCustomerAct, SIGNAL(triggered(bool)), this, SLOT(editCustomers()));

    /*KAction* editCustomerTextAct = new KAction(i18n("&Edit Customer Text"), "",// -!F: original, delete
                                0, this, SLOT(editCustomerText()), actionCollection() );*/
    KAction* editCustomerTextAct = new KAction(this);
    editCustomerTextAct->setText(i18n("&Edit Customer Text"));
    actionCollection()->addAction("editCustomerTextAct", editCustomerTextAct);
    connect(editCustomerTextAct, SIGNAL(triggered(bool)), this, SLOT(editCustomerText()));

    /*KAction* importCSVAct = new KAction(i18n("&Import CSV File..."), "",// -!F: original, delete
                                0, this, SLOT(importCSV()), actionCollection() );*/
    KAction* importCSVAct = new KAction(this);
    importCSVAct->setText(i18n("&Import CSV File..."));
    actionCollection()->addAction("importCSVAct", importCSVAct);
    connect(importCSVAct, SIGNAL(triggered(bool)), this, SLOT(importCSV()));
    
    KMenu* data = new KMenu( buttonData );
    /*editLabelDefAct->plug( data );// -!F: original, delete
    editArticleAct->plug( data );
    editCustomerAct->plug( data );
    editCustomerTextAct->plug( data );
    buttonData->setPopup( data );
    data->insertSeparator();
    importCSVAct->plug( data );*/
    data->addAction(editLabelDefAct);
    data->addAction(editArticleAct);
    data->addAction(editCustomerAct);
    data->addAction(editCustomerTextAct);
    data->addSeparator();
    data->addAction(importCSVAct);
    buttonData->setMenu(data);

    enableData();
}

KBarcode::~KBarcode()
{
    MainWindow::saveConfig();
}

void KBarcode::setupActions()
{
    MainWindow::setupActions(kbarcodeDirectoryName);
}

void KBarcode::startBarcode()
{
    /*new BarcodeGenerator();*/
}

void KBarcode::startLabelEditor()
{
    /*LabelEditor* ed = new LabelEditor( NULL, QString::null  );
    ed->startupDlg( LabelEditor::eCreateNewLabel, QString::null );*/
}

void KBarcode::startBatchPrint()
{
    /*new BatchAssistant( this );*/
}

void KBarcode::editArticles()
{
    /*new DatabaseBrowser( TABLE_BASIC, NULL );*/
}

void KBarcode::editCustomers()
{
    /*new DatabaseBrowser( TABLE_CUSTOMER, NULL );*/
}

void KBarcode::editCustomerText()
{
    /*new DatabaseBrowser( TABLE_CUSTOMER_TEXT, NULL );*/
}

void KBarcode::editLabelDef()
{
    /*new DatabaseBrowser( TABLE_LABEL_DEF, NULL );*/
}

void KBarcode::enableData()
{
    buttonData->setEnabled( SqlTables::getInstance()->isConnected() );
}

bool KBarcode::parseCmdLine()
{
    enum { BARCODE, LABEL, BATCH, NONE } mode = NONE;

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    /*BatchPrinter::EOutputFormat eFormat = BatchPrinter::POSTSCRIPT;
    LabelEditor* pEdit = NULL;*/
    QString serial;
    QString sqlquery   = QString::null;
    QString csvfile    = QString::null;
    QString printer    = QString::null;

    bool bPrintNow     = args->isSet("print");
    int serialinc      = 0;
    int numlabels      = -1;

    QString format = args->getOption("output");
    /*if( format.toUpper() == "BARCODE" )
	eFormat = BatchPrinter::BCP;
    else if( format.toUpper() == "IMAGE" )
	eFormat = BatchPrinter::IMAGE;
    else if( format.toUpper() == "POSTSCRIPT" )
	eFormat = BatchPrinter::POSTSCRIPT;
    else
    {
	KMessageBox::error( this, i18n("%1 is no valid output format for --output. Valid values are POSTSCRIPT, IMAGE and BARCODE.").arg( format ) );
    }*/

    if( args->isSet("barcode") )
    {
	mode = BARCODE;
	/*startBarcode();*/
    }

    if( args->isSet("batch") )
	mode = BATCH;
    
    if( args->isSet("label") )
	mode = LABEL;

    if( args->isSet("serialnumber") )
    {
	serial = args->getOption("serialnumber");
	if( args->isSet("serialinc") )
	    serialinc = args->getOption("serialinc").toInt();
    }

    if( args->isSet("numlabels") )
	numlabels = args->getOption("numlabels").toInt();

    if( args->isSet("importsql") )
	sqlquery = args->getOption("importsql");

    if( args->isSet("importcsv") ) 
	csvfile = args->getOption("importcsv");

    if( args->isSet("printer") )
	printer = args->getOption("printer");

    for( int i = 0; i < args->count(); i++) 
	if( mode == BATCH )
	{
	    /*BatchAssistant* b = new BatchAssistant();
	    b->setFilename( args->url( i ).path() );
	    b->setOutputFormat( eFormat );
	    b->setSerialNumber( serial, serialinc );
	    if( !sqlquery.isEmpty() )
		b->setImportSqlQuery( sqlquery );

	    if( !csvfile.isEmpty() )
		b->setImportCsvFile( csvfile );

	    if( numlabels != -1 )
		b->setNumLabels( numlabels );

	    if( bPrintNow )
            {
		b->printNow( printer, false );
                delete b;
            }*/
	}
	else
        {
	    /*pEdit = new LabelEditor( 0, args->url( i ).path() );
            if( bPrintNow )
                // TODO: use the printer set by the printer commandline option
                pEdit->print(); */
        }

    if( !args->count() && mode == LABEL )
    {
        /*pEdit = new LabelEditor();
        if( bPrintNow )
            // TODO: use the printer set by the printer commandline option
            pEdit->print(); */

    }

    int argc = args->count();
    args->clear();  // Free some memory

    // close after printing
    if( bPrintNow && argc )
    {
        // TODO: really close the whole application
        this->close();
        return true;
    }
    else
        return false;    
}

void KBarcode::importCSV()
{
    /*new CSVImportDlg( this );*/
}

bool KBarcode::isSQLConnected() const
{
    return SqlTables::isConnected();
}

bool KBarcode::connectSQL()
{
    return SqlTables::getInstance()->connectMySQL();
    /*return true;*/// -!F: delete
}

void KBarcode::showAssistant()
{
    MainWindow::assistant();
}

void KBarcode::showConfigure()
{
    KBarcodeSettings::getInstance()->configure();
}

#include "kbarcode.moc"
