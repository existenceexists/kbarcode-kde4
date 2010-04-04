/***************************************************************************
                          batchassistant.cpp  -  description
                             -------------------
    begin                : Sun Mar 20 2005
    copyright            : (C) 2005 by Dominik Seichter
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

#include "batchassistant.h"
#include "batchassistant.moc"
#include "barcodeprinterdlg.h"
#include "batchprinter.h"
#include "csvfile.h"
#include "definition.h"
#include "encodingcombo.h"
#include "printersettings.h"
#include "printlabeldlg.h"
#include "smalldialogs.h"
#include "sqltables.h"
#include "tokenprovider.h"
#include "xmlutils.h"

#include <qbuffer.h>
#include <qcheckbox.h>
#include <qclipboard.h>
#include <qcursor.h>
#include <qdom.h>
#include <q3header.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <q3sqlselectcursor.h> 
#include <qtooltip.h>
#include <QGroupBox>
#include <q3widgetstack.h>
//Added by qt3to4:
#include <QHBoxLayout>
#include <QList>
#include <QSqlQuery>
#include <QSqlCursor>
#include <QFrame>
#include <QSqlError>
#include <QVBoxLayout>

#include <kabc/addressee.h>
#include <kabc/addresseelist.h>
#include <kabc/addressbook.h>
#include <kabc/stdaddressbook.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kcompletion.h>
#include <kconfiggroup.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kimageio.h>
#include <klineedit.h>
#include <k3listbox.h>
#include <k3listview.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kmenu.h>
#include <kpushbutton.h>
#include <kurlrequester.h>
#include <q3table.h>
#include <q3vbox.h>
#include <kglobal.h>

#define PNG_FORMAT "PNG"

class AddressListViewItem : public KListViewItem {
public:
    AddressListViewItem(QListView *parent, KABC::Addressee & addr )
        : KListViewItem( parent ), m_address( addr )
        {
            this->setText( 0, m_address.givenName() );
            this->setText( 1, m_address.familyName() );
            this->setText( 2, m_address.preferredEmail() );
        }
    
    const KABC::Addressee & address() const {
        return m_address;
    }

private:
    KABC::Addressee m_address;

};

BatchAssistant::BatchAssistant( QWidget* parent )
    : KAssistant( parent )
{
    setupPage1();
    setupPage2();
    setupPage3();
    setupPage4();
    setupPage5();
    setupPage10();

    compGroup = new KCompletion();

    enableControls();
    setupSql();

    show();
}

BatchAssistant::~BatchAssistant()
{
    delete compGroup;
}

void BatchAssistant::setupPage1()
{
    page1 = new QWidget( this );
    QVBoxLayout* pageLayout = new QVBoxLayout( page1, 11, 6, "pageLayout");

    QLabel* label = new QLabel( i18n("<qt>This assistant will guide you through the process "
				     "of printing many labels with KBarcode.<br>The first step "
				     "is to select the KBarcode label file you want to print.</qt>"), page1 );
    pageLayout->addWidget( label );
   
    m_url = new KUrlRequester( page1 );
    m_url->setMode( KFile::File | KFile::ExistingOnly | KFile::LocalOnly );
    m_url->setFilter( "*.kbarcode" );

    label = new QLabel( i18n("&Filename:"), page1 );
    label->setBuddy( m_url );

    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding );

    pageLayout->addWidget( label );
    pageLayout->addWidget( m_url );
    pageLayout->addItem( spacer );

    addPage( page1, i18n("File Selection") );

    connect( m_url, SIGNAL( textChanged( const QString & ) ), this, SLOT( enableControls() ) );
}

void BatchAssistant::setupPage2()
{
    page2 = new QWidget( this );
    QVBoxLayout* pageLayout = new QVBoxLayout( page2, 11, 6, "pageLayout");

    QButtonGroup* group = new QButtonGroup( page2 );
    QVBoxLayout* button_layout=new QVBoxLayout;
    radioSimple = new QRadioButton( i18n("Print &labels without data"));
	button_layout->addWidget(radioSimple);
    radioSqlArticles = new QRadioButton( i18n("Print &articles from KBarcodes SQL database"));
	button_layout->addWidget(radioSqlArticles);
    radioVarImport = new QRadioButton( i18n("Import &variables and print"));
	button_layout->addWidget(radioVarImport);
    radioAddressBook = new QRadioButton( i18n("Print &contacts from your addressbook"));
	button_layout->addWidget(radioAddressBook);
    radioSimple->setChecked( true );
	group->setLayout(button_layout);

    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding );
    pageLayout->addWidget( group );
    pageLayout->addItem( spacer );

    connect( radioSimple, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( radioSqlArticles, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( radioVarImport, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( radioAddressBook, SIGNAL( clicked() ), this, SLOT( enableControls() ) );

    addPage( page2, i18n("Data Source") );
}

void BatchAssistant::setupPage3()
{
    page3 = new QWidgetStack( this );

    setupStackPage1();
    setupStackPage2();
    setupStackPage3();
    setupStackPage4();

    addPage( page3, i18n("Print Data") );
}

void BatchAssistant::setupPage4()
{
    page4 = new QVBox( this );
    page4->setSpacing( 5 );

    QHBox* hbox = new QHBox( page4 );
    hbox->setSpacing( 5 );
    
    buttonTableInsert = new KPushButton( i18n("Insert Row"), hbox );
    buttonTableInsert->setIconSet( BarIconSet( "edit" ) );
    buttonTableRemove = new KPushButton( i18n("Delete Row"), hbox );
    buttonTableRemove->setIconSet( BarIconSet( "editdelete") );

    m_varTable = new QTable( page4 );
    m_varTable->setReadOnly( false );
    m_varTable->setSelectionMode( QTable::SingleRow );

    addPage( page4, i18n("Import Variables") );

    connect( buttonTableInsert, SIGNAL( clicked() ), this, SLOT( slotTableInsert() ) );
    connect( buttonTableRemove, SIGNAL( clicked() ), this, SLOT( slotTableRemove() ) );
}

void BatchAssistant::setupPage5()
{
    TokenProvider serial( this );

    page5 = new QVBox( this );

    new QLabel( i18n( "<qt>KBarcode has support for placing serial numbers on labels. "
		      "If you did not use the [serial] token on your label in "
		      "a text field or a barcode, you can skip this page.<br>"
		      "Serial start is a free form start value containing at least one "
		      "number. This number is increased for every printed label on the "
		      "print out.</qt>"), page5 );

    QHBox* hbox = new QHBox( page5 );
    hbox->setSpacing( 5 );
    
    new QLabel( i18n( "Serial start:" ), hbox );
    serialStart = new KLineEdit( serial.serial(), hbox );

    serialInc = new KIntNumInput( 1, hbox );
    serialInc->setLabel( i18n( "Serial increment:" ), Qt::AlignLeft | Qt::AlignVCenter );
    serialInc->setRange( 1, 10000, 1, false );

    addPage( page5, i18n("Serial Number") );
}

void BatchAssistant::setupPage10()
{
    page10 = new QWidget( this );
    QVBoxLayout* pageLayout = new QVBoxLayout( page10, 11, 6, "pageLayout");

    QButtonGroup* group = new QButtonGroup( page10 );
	QVBoxLayout* button_layout = new QVBoxLayout;
    
    radioPrinter = new QRadioButton( i18n("&Print to a system printer or to a file"));
	button_layout->addWidget(radioPrinter);
    radioImage = new QRadioButton( i18n("&Create images"));
	button_layout->addWidget(radioImage);

    imageBox = new QVBox;
    imageBox->setMargin( 10 );
	button_layout->addWidget(imageBox);
    radioBarcode = new QRadioButton( i18n("Print to a special &barcode printer"));
	button_layout->addWidget(radioBarcode);
	group->setLayout(button_layout);

    QHBox* directoryBox = new QHBox( imageBox );
    directoryBox->setSpacing( 5 );
    QLabel* label = new QLabel( i18n("Output &Directory:"), directoryBox );
    imageDirPath = new KUrlRequester( directoryBox );
    imageDirPath->setMode( KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );
    label->setBuddy( directoryBox );

    QHBox* formatBox = new QHBox( imageBox );
    label = new QLabel( i18n("Output File &Format:"), formatBox );

    QStringList formats = KImageIO::types( KImageIO::Writing );
    comboFormat = new KComboBox( false, formatBox );
    comboFormat->insertStringList( formats );
    if( formats.contains( PNG_FORMAT ) )
	comboFormat->setCurrentItem( formats.findIndex( PNG_FORMAT ) );
    label->setBuddy( comboFormat );

    QButtonGroup* imageNameGroup = new  QButtonGroup( i18n("&Filename:"), imageBox );
    QVBoxLayout* image_button_layout = new QVBoxLayout;
    radioImageFilenameArticle = new QRadioButton( i18n("Use &article number for filename"));
	image_button_layout->addWidget(radioImageFilenameArticle);
    radioImageFilenameBarcode = new QRadioButton( i18n("Use &barcode number for filename"));
	image_button_layout->addWidget(radioImageFilenameBarcode);
    radioImageFilenameCustom  = new QRadioButton( i18n("Use &custom filename:"));
	image_button_layout->addWidget(radioImageFilenameCustom);
    editImageFilename = new KLineEdit( imageNameGroup );
    radioImageFilenameBarcode->setChecked( true );
	image_button_layout->addWidget(editImageFilename);
	imageNameGroup->setLayout(image_button_layout);

    labelInfo = new QLabel( page10 );

    radioPrinter->setChecked( true );

    checkKeepOpen = new QCheckBox( i18n("&Keep window open after printing."), page10 );

    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding );
    pageLayout->addWidget( group );
    pageLayout->addItem( spacer );
    pageLayout->addWidget( labelInfo );
    pageLayout->addWidget( checkKeepOpen );

    connect( radioPrinter, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( radioImage, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( radioBarcode, SIGNAL( clicked() ), this, SLOT( enableControls() ) );

    connect( radioImageFilenameArticle, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( radioImageFilenameBarcode, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( radioImageFilenameCustom, SIGNAL( clicked() ), this, SLOT( enableControls() ) );

    connect( imageDirPath, SIGNAL( textChanged( const QString & ) ), this, SLOT( enableControls() ) );

    addPage( page10, i18n("Output Device") );
}

void BatchAssistant::setupStackPage1()
{
    stack1 = new QVBox( page3, "stack1" );
    stack1->setSpacing( 5 );

    QHBox* hbox = new QHBox( stack1 );
    hbox->setSpacing( 5 );

    new QLabel( i18n( "Customer name and no.:" ), hbox );
    customerName = new KComboBox( false, hbox );
    customerId = new KComboBox( false, hbox );

    QHBox* hButtonBox = new QHBox( stack1 );
    hButtonBox->setSpacing( 5 );

    buttonAdd = new KPushButton( i18n( "&Add..." ), hButtonBox );
    buttonImport = new KPushButton( i18n("&Import..."), hButtonBox );
    buttonEdit = new KPushButton( i18n( "&Edit..." ), hButtonBox );
    buttonRemove = new KPushButton( i18n("&Remove" ), hButtonBox );
    buttonRemoveAll = new KPushButton( i18n("R&emove All"), hButtonBox );

    KMenu* mnuImport = new KMenu( this );
    mnuImport->insertItem( i18n("Import from File ..."), this, SLOT( loadFromFile() ) );
    mnuImport->insertItem( i18n("Import from Clipboard ..."), this, SLOT( loadFromClipboard() ) );
    mnuImport->insertItem( i18n("Import barcode_basic"), this, SLOT( addAllItems() ) );
    buttonImport->setPopup( mnuImport );

    sqlList = new KListView( stack1 );
    sqlList->addColumn( i18n("Index") );
    sqlList->addColumn( i18n("Number of Labels") );
    sqlList->addColumn( i18n("Article Number") );
    sqlList->addColumn( i18n("Group") );
    sqlList->setAllColumnsShowFocus( true );
    connect( sqlList, SIGNAL(doubleClicked(QListViewItem*,const QPoint &,int)),
             this, SLOT(changeItem(QListViewItem*,const QPoint &,int)));

    connect( customerName, SIGNAL( activated(int) ), this, SLOT( customerNameChanged(int) ) );
    connect( customerId, SIGNAL( activated(int) ), this, SLOT( customerIdChanged(int) ) );
    connect( buttonAdd, SIGNAL( clicked() ), this, SLOT( addItem() ) );
    connect( buttonEdit, SIGNAL( clicked() ), this, SLOT( editItem() ) );
    connect( buttonRemove, SIGNAL( clicked() ), this, SLOT( removeItem() ) );
    connect( buttonRemoveAll, SIGNAL( clicked() ), sqlList, SLOT( clear() ) );
    connect( buttonRemoveAll, SIGNAL( clicked() ), this, SLOT( enableControls() ) );

    page3->addWidget( stack1 );
}

void BatchAssistant::setupStackPage2()
{
    stack2 = new QHBox( page3, "stack2" );
    stack2->setSpacing( 5 );

    QButtonGroup* group = new QButtonGroup( stack2 );
    QVBoxLayout* group_layout = new QVBoxLayout;
    radioImportManual = new QRadioButton( i18n("Enter &data manually") );
    group_layout->addWidget(radioImportManual);
    radioImportSql = new QRadioButton( i18n("Import variables from a &SQL table") );
    group_layout->addWidget(radioImportSql);
    labelSqlQuery = new QLabel( i18n("Please enter a sql &query:") );
    group_layout->addWidget(labelSqlQuery);
    importSqlQuery = new KLineEdit;
    group_layout->addWidget(importSqlQuery);
    labelSqlQuery->setBuddy( importSqlQuery );

    radioImportCSV = new QRadioButton( i18n("Import from a &CSV file") );
    group_layout->addWidget(radioImportCSV);
    labelCsvFile= new QLabel( i18n("Please select a csv &file:") );
    group_layout->addWidget(labelCsvFile);
    importCsvFile = new KUrlRequester;
    group_layout->addWidget(importCsvFile);
    labelCsvFile->setBuddy( importCsvFile );
    labelEncoding = new QLabel( i18n("&Encoding:") );
    group_layout->addWidget(labelEncoding);
    comboEncoding = new EncodingCombo;
    group_layout->addWidget(comboEncoding);
    labelEncoding->setBuddy( comboEncoding );
    radioImportManual->setChecked( true );
    group->setLayout(group_layout);

    QVBox* box = new QVBox( stack2 );
    box->setSpacing( 5 );

    new QLabel( i18n("Available Variables:"), box );
    m_varList = new KListBox( box );

    connect( radioImportManual, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( radioImportSql, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( radioImportCSV, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( importSqlQuery, SIGNAL( textChanged( const QString & ) ), this, SLOT( enableControls() ) );
    connect( importCsvFile, SIGNAL( textChanged( const QString & ) ), this, SLOT( enableControls() ) );

    page3->addWidget( stack2 );
}

void BatchAssistant::setupStackPage3()
{
    stack3 = new QVBox( page3, "stack3" );

    numLabels = new KIntNumInput( 1, stack3 );
    numLabels->setRange( 1, 100000, 1, true );
    numLabels->setLabel( i18n("&Number of labels to print:"), Qt::AlignLeft | Qt::AlignVCenter );

    page3->addWidget( stack3 );
}

void BatchAssistant::setupStackPage4()
{
    stack4 = new QWidget( page3, "stack4" );
    
    QHBoxLayout* mainLayout = new QHBoxLayout( stack4 );

    QVBox* list1 = new QVBox( stack4 );
    QVBox* list2 = new QVBox( stack4 );

    QFrame* buttons = new QFrame( stack4 );
    buttons->setMargin( 10 );

    QVBoxLayout* layout = new QVBoxLayout( buttons );
    QSpacerItem* spacer1 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding );
    QSpacerItem* spacer2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding );

    buttonAddAllAddress  = new KPushButton( buttons );
    buttonAddAddress = new KPushButton( buttons );
    buttonRemoveAddress = new KPushButton( buttons );;
    buttonRemoveAllAddress = new KPushButton( buttons );

    buttonAddAllAddress->setIconSet( BarIconSet( "2rightarrow" ) );
    buttonAddAddress->setIconSet( BarIconSet( "1rightarrow" ) );
    buttonRemoveAddress->setIconSet( BarIconSet( "1leftarrow" ) );
    buttonRemoveAllAddress->setIconSet( BarIconSet( "2leftarrow" ) );

    QToolTip::add( buttonAddAllAddress, i18n("Add all contacts to the list of contacts which will be printed.") );
    QToolTip::add( buttonAddAddress, i18n("Add selected contacts to the list of contacts which will be printed.") );
    QToolTip::add( buttonRemoveAddress, i18n("Remove selected contacts from the list of contacts which will be printed.") );
    QToolTip::add( buttonRemoveAllAddress, i18n("Remove all contacts from the list of contacts which will be printed.") );

    layout->addItem( spacer1 );
    layout->addWidget( buttonAddAllAddress );
    layout->addWidget( buttonAddAddress );
    layout->addWidget( buttonRemoveAddress );
    layout->addWidget( buttonRemoveAllAddress );
    layout->addItem( spacer2 );

    mainLayout->addWidget( list1 );
    mainLayout->addWidget( buttons );
    mainLayout->addWidget( list2 );

    mainLayout->setStretchFactor( list1, 2 );
    mainLayout->setStretchFactor( list2, 2 );

    new QLabel( i18n("All Addresses"), list1 );
    new QLabel( i18n("Selected Addresses"), list2 );

    listAddress = new KListView( list1 );
    listAddress->addColumn( i18n("Given Name"), 0 );
    listAddress->addColumn( i18n("Family Name"), 1 );
    listAddress->addColumn( i18n("Email Address"), 2 );
    listAddress->setMultiSelection( true );
    listAddress->setAllColumnsShowFocus( true );

    listAddress->setColumnWidthMode( 0, QListView::Maximum );
    listAddress->setColumnWidthMode( 1, QListView::Maximum );
    listAddress->setColumnWidthMode( 2, QListView::Maximum );

    listSelectedAddress = new KListView( list2 );
    listSelectedAddress->addColumn( i18n("Given Name"), 0 );
    listSelectedAddress->addColumn( i18n("Family Name"), 1 );
    listSelectedAddress->addColumn( i18n("Email Address"), 2 );
    listSelectedAddress->setMultiSelection( true );
    listSelectedAddress->setAllColumnsShowFocus( true );

    listSelectedAddress->setColumnWidthMode( 0, QListView::Maximum );
    listSelectedAddress->setColumnWidthMode( 1, QListView::Maximum );
    listSelectedAddress->setColumnWidthMode( 2, QListView::Maximum );

    connect( buttonAddAddress, SIGNAL( clicked() ), this, SLOT( slotAddAddress() ) );
    connect( buttonRemoveAddress, SIGNAL( clicked() ), this, SLOT( slotRemoveAddress() ) );
    connect( buttonAddAllAddress, SIGNAL( clicked() ), this, SLOT( slotAddAllAddress() ) );
    connect( buttonRemoveAllAddress, SIGNAL( clicked() ), this, SLOT( slotRemoveAllAddress() ) );

    page3->addWidget( stack4 );
}

void BatchAssistant::setupSql()
{
    SqlTables* tables = SqlTables::getInstance();
    if( !tables->isConnected() )
        return;

    QSqlCursor cur( TABLE_CUSTOMER );
    cur.select();
    customerId->clear();
    customerName->clear();
    while ( cur.next() ) {
        customerId->addItem( cur.value("customer_no" ).toString() );
        customerName->addItem( cur.value("customer_name" ).toString() );
    }
}

void BatchAssistant::enableControls()
{
    setAppropriate( page4, radioVarImport->isChecked() );

    radioSqlArticles->setEnabled( SqlTables::getInstance()->isConnected() );
    radioImportSql->setEnabled( SqlTables::getInstance()->isConnected() );

    importCsvFile->setEnabled( radioImportCSV->isChecked() );
    labelCsvFile->setEnabled( radioImportCSV->isChecked() );
    importSqlQuery->setEnabled( radioImportSql->isChecked() );
    labelSqlQuery->setEnabled( radioImportSql->isChecked() );
    labelEncoding->setEnabled( radioImportCSV->isChecked() );
    comboEncoding->setEnabled( radioImportCSV->isChecked() );

    buttonRemove->setEnabled( sqlList->childCount() );
    buttonRemoveAll->setEnabled(sqlList->childCount() );
    buttonEdit->setEnabled( sqlList->childCount() );

    imageBox->setEnabled( radioImage->isChecked() );

    if( radioImportSql->isChecked() )
	setNextEnabled( page3, !importSqlQuery->text().isEmpty() );
    else if( radioImportCSV->isChecked() )
	setNextEnabled( page3, !importCsvFile->url().isEmpty() );
    else if( radioImportManual->isChecked() )
	setNextEnabled( page3, true );

    editImageFilename->setEnabled( radioImageFilenameCustom->isChecked() );
    radioImageFilenameArticle->setEnabled( radioSqlArticles->isChecked() );

    setNextEnabled( page1, !m_url->url().isEmpty() );
    
    if( radioAddressBook->isChecked() )
        setNextEnabled( page3, listSelectedAddress->childCount() );

    if( radioImage->isChecked() )
	setFinishEnabled( page10, !imageDirPath->url().isEmpty() );
    else
	setFinishEnabled( page10, true );
}

void BatchAssistant::showPage( QWidget* p )
{
    if( p == page3 )
    {
	if( radioSqlArticles->isChecked() )
	    page3->raiseWidget( stack1 );
	else if( radioVarImport->isChecked() )
	{
	    page3->raiseWidget( stack2 );
	    fillVarList();
	}
	else if( radioSimple->isChecked() )
	    page3->raiseWidget( stack3 );
        else if( radioAddressBook->isChecked() )
        {
            page3->raiseWidget( stack4 );
            fillAddressList();
        }
    }
    else if( p == page4 )
	if( !fillVarTable() )
	    return;


    KAssistant::showPage( p );
}

void BatchAssistant::accept()
{
    printNow( QString::null );
}

void BatchAssistant::printNow( const QString & printer, bool bUserInteraction )
{
    BatchPrinter* batch = NULL;
    QPrinter* prn = NULL;
    int batchType = 0;

    // let's check if the label file does even exist!
    if( !QFile::exists( m_url->url() ) ) 
    {
        KMessageBox::error( this, QString( i18n("The label file %1 was not found") ).arg( m_url->url()) );
        return;
    }

    if( radioPrinter->isChecked() )
    {
	int move = 0;
        if( bUserInteraction ) 
        {
            PrintLabelDlg pld( this );
            pld.setLabelsEnabled( false );
            if( pld.exec() != QDialog::Accepted )
		return;
	    
            move = pld.position();
            PrinterSettings::getInstance()->getData()->border = pld.border();
        }
	
	prn = PrinterSettings::getInstance()->setupPrinter( m_url->url(), this, !printer.isEmpty(), printer );
	if( !prn ) 
	    return;

	batch = new BatchPrinter( prn, this );
	batch->setMove( move );
     
	batchType = BatchPrinter::POSTSCRIPT;
    }
    else if( radioBarcode->isChecked() )
    {
	BarcodePrinterDlg dlg(this);
	if( dlg.exec() != QDialog::Accepted )
	    return;

	batch = new BatchPrinter( dlg.printToFile() ? dlg.fileName() : dlg.deviceName(), 
				  dlg.outputFormat(), this );
	batchType = BatchPrinter::BCP;
    }
    else if( radioImage->isChecked() )
    {
	batch = new BatchPrinter( imageDirPath->url(), this );
	if( radioImageFilenameArticle->isChecked() )
	    batch->setImageFilename( BatchPrinter::E_ARTICLE );
	else if( radioImageFilenameBarcode->isChecked() )
	    batch->setImageFilename( BatchPrinter::E_BARCODE );
	else if( radioImageFilenameCustom->isChecked() )
	{
	    batch->setImageFilename( BatchPrinter::E_CUSTOM );
	    batch->setImageCustomFilename( editImageFilename->text() );
	}

	batchType =  BatchPrinter::IMAGE;
    }

    if( !checkKeepOpen->isChecked() )
        KAssistant::accept();

    KApplication::setOverrideCursor( QCursor( Qt::ArrowCursor ), true );
    setupBatchPrinter( batch, batchType );
    KApplication::restoreOverrideCursor();

    delete prn;
    delete batch;
}

void BatchAssistant::setupBatchPrinter( BatchPrinter* batch, int m )
{
    Definition* def = NULL;
    QString description;
    bool kbarcode18;

    fillByteArray();
    QDomDocument doc( "KBarcodeLabel" );
    if ( !doc.setContent( m_bytearray ) ) 
        return;
    
    XMLUtils util;
    util.readXMLHeader( &doc, description, kbarcode18, &def );

    QBuffer buffer( m_bytearray );
    if( !buffer.open( QIODevice::ReadOnly ) )
        return;

    batch->setBuffer( &buffer );
    batch->setSerial( serialStart->text(), serialInc->value() );
    batch->setName( m_url->url() );
    batch->setDefinition( def );
    batch->setImageFormat( comboFormat->currentText() );

    if( radioSqlArticles->isChecked() )
    {
	int labels = 0;
	batch->setCustomer( customerId->currentText() );
	
        // sort by group
	sqlList->setSorting( 3, true );
	sqlList->sort();

	QList<BatchPrinter::data>* dlist = new QList<BatchPrinter::data>;
	QListViewItem* item = sqlList->firstChild();
	while( item ) 
	{
	    BatchPrinter::data m_data;
	    m_data.number = item->text( 1 ).toInt();
	    labels += m_data.number;
	    m_data.article_no = item->text( 2 );
	    m_data.group = item->text( 3 );
	    
	    dlist->append( m_data );
	    item = item->nextSibling();
	};

	batch->setData( dlist );
	batch->setLabels( labels );
    }
    else if( radioSimple->isChecked() )
    {
	batch->setLabels( numLabels->value() );

	// do a dirty drick, TODO: refactor BatchPrinter in the future
	QList<BatchPrinter::data>* dlist = new QList<BatchPrinter::data>;
	BatchPrinter::data m_data;
	m_data.number = numLabels->value();
	dlist->append( m_data );

	batch->setData( dlist );	
    }
    else if( radioVarImport->isChecked() )
    {
	TVariableList* tVariableList = new TVariableList;
	for( int i=0; i<m_varTable->numRows(); i++ )
	{
	    QMap<QString, QString> map;
	    for( int z=0; z<m_varTable->numCols(); z++ )
		map[ m_varTable->horizontalHeader()->label( z ) ] = m_varTable->text( i, z );
	    tVariableList->append( map );
	}

	batch->setData( tVariableList );
    }
    else if( radioAddressBook->isChecked() )
    {
        KABC::AddresseeList* list = new KABC::AddresseeList;
        QListViewItem* item = listSelectedAddress->firstChild();
        while( item ) 
        {
            list->append( static_cast<AddressListViewItem*>(item)->address() );
            item = item->nextSibling();
        }

        batch->setData( list );
    }

    if( m == BatchPrinter::POSTSCRIPT )    
        batch->start();
    else if( m == BatchPrinter::IMAGE )
        batch->startImages();
    else if( m == BatchPrinter::BCP )
        batch->startBCP();

    delete def;
}


void BatchAssistant::addItem()
{
    DSSmallDialogs::AddItemsDialog aid( this );
    aid.setGroupCompletion( compGroup );
    connect( &aid, SIGNAL( add( const QString &, const QString &, int) ),
             this, SLOT( slotAddItem( const QString &, const QString &, int) ) );

    aid.exec();
}

bool BatchAssistant::slotAddItem( const QString & article, const QString & group, int count )
{
    return this->addItem( article, group, count, true );
}

bool BatchAssistant::addItem( const QString & article, const QString & group, int count, bool msg )
{
    if( !article.isEmpty() && !existsArticle( article ) ) {
        if( msg )
            KMessageBox::error( this, i18n("Please enter a valid article ID") );
        return false;
    }

    QString temp;
    temp.sprintf("%0*i", 5, sqlList->childCount() + 1 );

    KListViewItem* item = new KListViewItem( sqlList, temp, QString( "%1" ).arg( count ),
                          article, group );
    sqlList->insertItem( item );

    addGroupCompletion( group );
    enableControls();

    return true;
}

void BatchAssistant::addGroupCompletion( const QString & group )
{
    if( !group.isEmpty() ) 
    {
        QStringList slist = compGroup->items();
        if(!slist.contains( group ) )
            compGroup->addItem( group );
    }
}

bool BatchAssistant::existsArticle( const QString & article )
{
    if( article.isEmpty() )
        return false;

    QSqlQuery query( "select uid from barcode_basic where article_no='" + article + "'" );
    while ( query.next() )
        return true;

    return false;
}

void BatchAssistant::editItem()
{
    QListViewItem* item = sqlList->selectedItem();
    if( item )
        changeItem( item, QPoint(0,0), 0 );
}

void BatchAssistant::changeItem( QListViewItem* item, const QPoint &, int )
{
    if(!item)
        return;

    DSSmallDialogs::AddItemsDialog aid( item->text( 2 ), item->text( 3 ),
                                        item->text( 1 ).toInt(), this, "aid" );
    aid.setGroupCompletion( compGroup );

    if( aid.exec() == QDialog::Accepted ) 
    {
        item->setText( 1, QString::number( aid.count() ) );
        item->setText( 2, aid.articleNo() );
        item->setText( 3, aid.groupName() );
        addGroupCompletion( aid.groupName() );
	enableControls();
    }
}

void BatchAssistant::removeItem() 
{
    QListViewItem* item = sqlList->firstChild();
    while( item ) 
    {
        if( item->isSelected() ) 
	{
            QListViewItem* it = item->nextSibling();
            delete item;

            while( it ) 
	    {
                int a = it->text( 0 ).toInt();
                QString temp;
                temp.sprintf("%0*i", 5, a - 1 );
                it->setText( 0, temp );
                it = it->nextSibling();
            }

            break;
        } else
            item = item->nextSibling();
    }

    enableControls();
}

void BatchAssistant::customerIdChanged( int index ) 
{
    customerName->setCurrentItem( index );
    enableControls();
}

void BatchAssistant::customerNameChanged( int index ) 
{
    customerId->setCurrentItem( index );
    enableControls();
}

void BatchAssistant::addAllItems() 
{
    DSSmallDialogs::AddAllDialog* dlg = new DSSmallDialogs::AddAllDialog( this );
    if( dlg->exec() == QDialog::Accepted )
    {
	QString temp;
	QString group = dlg->groupName();
	const QString num = QString::number( dlg->numberLabels() );

	QSqlQuery query("SELECT article_no FROM " TABLE_BASIC );
	while( query.next() ) 
	{
	    temp.sprintf("%0*i", 5, sqlList->childCount() + 1 );
	    new KListViewItem( sqlList, temp, num, query.value( 0 ).toString(), group );
	}

        enableControls();
    }
}

void BatchAssistant::loadFromFile() 
{
    QString f = KFileDialog::getOpenFileName( 0, 0, this );
    if( !f.isEmpty() )
        loadFromFile( f );
}

void BatchAssistant::loadFromClipboard() 
{
    QClipboard *cb = KApplication::clipboard();
    loadData( cb->text() );
}

void BatchAssistant::loadFromFile( const QString & url ) 
{
    QByteArray data;
    QFile file( url );
    
    if( !file.open( QIODevice::ReadOnly ) ) 
    {
        qDebug("Unable to open file: %s", url.toLatin1() );
        return;
    }

    data = file.readAll();

    loadData( QString( data ) );
}

void BatchAssistant::loadData( const QString & data ) 
{
    labelprinterdata* lpdata = PrinterSettings::getInstance()->getData();
    if( lpdata->separator.isEmpty() ) 
    {
        KMessageBox::sorry( this, i18n("Separator is empty. Please set it to a value.") );
        return;
    }

    KConfigGroup config = KGlobal::config()->group("FileFormat");
    int pos[3] = { config.readEntry("Data0", 0 ),
                   config.readEntry("Data1", 1 ),
                   config.readEntry("Data2", 2 ) };

    bool custom_article_no = lpdata->useCustomNo;
    QBuffer buf( data.utf8() );
    CSVFile file( buf );

    QStringList list, dropped;
    QString article, quantity, group;

    while( file.isValid() && !file.isEof() )
    {
	list = file.readNextLine();
	while( list.count() < 3 )
	    list.append( QString::null );

	if( pos[0] == 0 )
	    quantity = list[0];
	else if( pos[0] == 1 )
	    article = list[0];
	else
	    group = list[0];

	if( pos[1] == 0 )
	    quantity = list[1];
	else if( pos[1] == 1 )
	    article = list[1];
	else
	    group = list[1];

	if( pos[2] == 0 )
	    quantity = list[2];
	else if( pos[2] == 1 )
	    article = list[2];
	else
	    group = list[2];

	// data[0] == quantity
	// data[1] == article_no
	// data[2] == group

	bool qint = false;
	(void)quantity.toInt( &qint );

	if( qint && custom_article_no ) {
	    qint = false;
	    QSqlQuery query("SELECT article_no FROM customer_text WHERE article_no_customer='" + article + "'" );
	    while( query.next() ) {
		article = query.value( 0 ).toString();
		qint = true;
		break;
	    }
	}

	if( qint ) // && existsArticle( article )
	{    
	    if( !addItem( QString( article ), QString( group ), quantity.toInt(), false ) )
		dropped.append( quantity + lpdata->separator +  article + lpdata->separator + group );
	}
    }

    if( !dropped.isEmpty() )
        KMessageBox::informationList( this, i18n("<qt>The following items can not be added:" )+ "</qt>", dropped );
    
    enableControls();
}

void BatchAssistant::fillByteArray()
{
    if( m_bytearray_filename != m_url->url() )
    {
	QFile f( m_url->url() );
	if ( !f.open( QIODevice::ReadOnly ) )
	{
	    m_bytearray_filename = QString::null;
	    m_bytearray.resize( 0 );
	    return ;
	}

	m_bytearray = f.readAll();
	f.close();
    }
}

void BatchAssistant::fillVarList()
{
    fillByteArray();
    QDomDocument doc( "KBarcodeLabel" );
    if ( !doc.setContent( m_bytearray ) ) 
        return;
    
    XMLUtils util;
    DocumentItemList list;
    list.setAutoDelete( true );

    TokenProvider token( this );
    Definition* def = NULL;

    QString description;
    bool kbarcode18;
    util.readXMLHeader( &doc, description, kbarcode18, &def );
    util.readDocumentItems( &list, &doc, &token, kbarcode18 );

    token.setCurrentDocumentItems( list );

    QStringList vars = token.listUserVars();
    m_varList->clear();
    m_varList->insertStringList( vars );
    m_varTable->setNumCols( vars.count() );
    for( unsigned int i = 0; i < vars.count(); i++ )
    {
	vars[i] = vars[i].right( vars[i].length() - 1 );
	m_varTable->horizontalHeader()->setLabel( i, vars[i] );
    }

    delete def;
}

void BatchAssistant::fillAddressList()
{
    KABC::AddressBook* ab = KABC::StdAddressBook::self();
    listAddress->clear();
    
    KABC::AddressBook::Iterator it;
    listAddress->setUpdatesEnabled( false );
    for ( it = ab->begin(); it != ab->end(); ++it ) 
        new AddressListViewItem( listAddress, *it );
    listAddress->setUpdatesEnabled( true );
}

bool BatchAssistant::fillVarTable()
{
    // Clear the table
    m_varTable->setNumRows( 0 );

    if( radioImportSql->isChecked() )
    {
	int y = 0;
	int x;
	QSqlSelectCursor query( importSqlQuery->text(), SqlTables::getInstance()->database() );
	query.select();
	if( query.lastError().type() != QSqlError::None )
	{
	    KMessageBox::error( this, i18n("<qt>Can't execute SQL query:<br>") + query.lastError().text() + "</qt>" );
	    return false;
	}

	if( m_varTable->numRows() < query.size() )
	    m_varTable->setNumRows( query.size() );

	while( query.next() )
	{
	    for( x=0;x<m_varTable->numRows();x++ )
		m_varTable->setText( y, x, query.value( m_varTable->horizontalHeader()->label( x ) ).toString() );

	    y++;
	}
    }
    else if( radioImportCSV->isChecked() )
    {
	CSVFile file( importCsvFile->url() );
        file.setEncoding( comboEncoding->currentText() );

	QStringList heading;
	QStringList data;
	int i = 0;

        file.setCSVFile(true);
	if( !file.isValid() )
	{
	    KMessageBox::error( this, QString( i18n("Can't open file: %1") ).arg( importCsvFile->url() ) );
	    return false;
	}

	while( !file.isEof() )
	{
	    if( heading.isEmpty() )
		heading = file.readNextLine();
            else
	    {
		data = file.readNextLine();

		// add 100 rows to get a reasonable speed
		if( m_varTable->numRows() <= i )
		    m_varTable->setNumRows( i + 100 );	      

                printf("datacount=%i\n", data.count() );
		for( unsigned int z = 0; z < data.count(); z++ )
		{
                    printf("numRows=%i\n", m_varTable->numCols() );
		    for( int x = 0; x < m_varTable->numCols(); x++ )
                    {
                        printf("horizontal header=%s\n", m_varTable->horizontalHeader()->label( x ).toLower().toLatin1() );
                        printf("heading=%s\n", heading[z].toLower().toLatin1() );
			if( m_varTable->horizontalHeader()->label( x ).toLower() == heading[z].toLower() )
			{
                            printf("Reading: (%s)\n", data[z].toLatin1());
			    m_varTable->setText( i, x, data[z] );
			    break;
			}
                    }
		}

		if( data.count() )
		    i++;
            }
	}

	m_varTable->setNumRows( i );
    }

    return true;
}

void BatchAssistant::slotTableInsert()
{
    m_varTable->insertRows( m_varTable->numRows(), 1 );
}

void BatchAssistant::slotTableRemove()
{
    QTableSelection sel = m_varTable->selection( m_varTable->currentSelection() );
    m_varTable->removeRow( sel.topRow() );
}

void BatchAssistant::setFilename( const QString & url )
{
    m_url->setURL( url );
    enableControls();
}

void BatchAssistant::setImportSqlQuery( const QString & query )
{
    radioImportCSV->setChecked( false );
    radioImportManual->setChecked( false );
    radioImportSql->setChecked( true );

    radioVarImport->setChecked( true );
    radioSqlArticles->setChecked( false );
    radioSimple->setChecked( false );

    importSqlQuery->setText( query );

    enableControls();

    showPage( page3 );
    showPage( page4 );
}

void BatchAssistant::setImportCsvFile( const QString & filename )
{
    radioImportCSV->setChecked( true );
    radioImportManual->setChecked( false );
    radioImportSql->setChecked( false );

    radioVarImport->setChecked( true );
    radioSqlArticles->setChecked( false );
    radioSimple->setChecked( false );

    importCsvFile->setURL( filename );

    enableControls();

    showPage( page3 );
    showPage( page4 );
}

void BatchAssistant::setNumLabels( const int n )
{
    numLabels->setValue( n );
    radioSimple->setChecked( true );
    radioSqlArticles->setChecked( false );
    radioVarImport->setChecked( false );
    enableControls();
}

void BatchAssistant::setOutputFormat( const int e )
{
    radioBarcode->setChecked( false );
    radioImage->setChecked( false );
    radioPrinter->setChecked( false );

    switch( e )
    {
	case BatchPrinter::BCP:
	    radioBarcode->setChecked( true );
	    break;
	case BatchPrinter::IMAGE:
	    radioImage->setChecked( true );
	    break;
	default:
	case BatchPrinter::POSTSCRIPT:
	    radioPrinter->setChecked( true );
	    break;
    }

    enableControls();
}

void BatchAssistant::setSerialNumber( const QString & val, int inc )
{
    serialInc->setValue( inc );
    serialStart->setText( val );

    // Not needed here: enableControls();
}

void BatchAssistant::slotAddAddress()
{
    moveAddress( listAddress, listSelectedAddress );
    enableControls();
}

void BatchAssistant::slotAddAllAddress()
{
    moveAddress( listAddress, listSelectedAddress, true );
    enableControls();
}

void BatchAssistant::slotRemoveAddress()
{
    moveAddress( listSelectedAddress, listAddress );
    enableControls();
}

void BatchAssistant::slotRemoveAllAddress()
{
    moveAddress( listSelectedAddress, listAddress, true );
    enableControls();
}

void BatchAssistant::moveAddress( QListView* src, QListView* dst, bool bAll )
{
    QListViewItem* item = src->firstChild();
    QListViewItem* cur;

    while( item ) 
    {
        if( bAll || item->isSelected() )
        {
            cur = item;
            item = item->nextSibling();

            src->takeItem( cur );
            dst->insertItem( cur );
            cur->setSelected( false );
        }
        else
            item = item->nextSibling();
    }
}
