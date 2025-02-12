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
#include "dialogs/barcodeprinterdlg.h"
#include "batchprinter.h"
#include "csvfile.h"
#include "definition.h"
#include "encodingcombo.h"
#include "printersettings.h"
#include "dialogs/printlabeldlg.h"
#include "dialogs/smalldialogs.h"
#include "sqltables.h"
#include "tokenprovider.h"
#include "xmlutils.h"

#include <qbuffer.h>
#include <qcheckbox.h>
#include <qclipboard.h>
#include <qcursor.h>
#include <qdom.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <QSqlQueryModel>
#include <QDebug>
#include <QTreeWidget>
#include <QAbstractItemView>
#include <QStringList>
#include <QTableWidget>
#include <QLabel>
#include <QWidget>
#include <QButtonGroup>
//Added by qt3to4:
#include <QHBoxLayout>
#include <QList>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QTreeWidgetItem>
#include <QFrame>
#include <QSqlError>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QStackedWidget>

#include <kabc/addressee.h>
#include <kabc/addresseelist.h>
#include <Akonadi/Contact/ContactSearchJob>
#include <kapplication.h>
#include <kcombobox.h>
#include <kcompletion.h>
#include <kconfiggroup.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kimageio.h>
#include <klineedit.h>
#include <klistwidget.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kmenu.h>
#include <kpushbutton.h>
#include <kurlrequester.h>
#include <kglobal.h>
#include <KUrl>
#include <kpagewidgetmodel.h>
#include <KAction>
#include <kdialog.h>

#define PNG_FORMAT "PNG"

class AddressListViewItem : public QTreeWidgetItem {
public:
    AddressListViewItem(QTreeWidget *parent, KABC::Addressee & addr )
        : QTreeWidgetItem( parent ), m_address( addr )
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
    : KAssistantDialog( parent )
{
    setupPage1();
    setupPage2();
    setupPage3();
    setupPage4();
    setupPage5();
    setupPage10();

    compGroup = new KCompletion();

    setupSql();

    show();
    enableControls();
}

BatchAssistant::~BatchAssistant()
{
    delete compGroup;
}

void BatchAssistant::setupPage1()
{
    page1 = new QWidget( this );
    QVBoxLayout* pageLayout = new QVBoxLayout( page1 );
    pageLayout->setContentsMargins( 11, 11, 11, 11 );
    pageLayout->setSpacing( 6 );
    pageLayout->setObjectName( "page1Layout" );

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

    KPageWidgetItem * page1Item = addPage( page1, i18n("File Selection") );
    page1Item->setObjectName("page1");

    connect( m_url, SIGNAL( textChanged( const QString & ) ), this, SLOT( enableControls() ) );
}

void BatchAssistant::setupPage2()
{
    page2 = new QWidget( this );
    QVBoxLayout* pageLayout = new QVBoxLayout( page2 );
    pageLayout->setContentsMargins( 11, 11, 11, 11 );
    pageLayout->setSpacing( 6 );
    pageLayout->setObjectName( "page2Layout" );

    QGroupBox* group = new QGroupBox( page2 );
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

    KPageWidgetItem * page2Item = addPage( page2, i18n("Data Source") );
    page2Item->setObjectName("page2");
}

void BatchAssistant::setupPage3()
{
    page3 = new QStackedWidget( this );

    setupStackPage1();
    setupStackPage2();
    setupStackPage3();
    setupStackPage4();

    page3Item = addPage( page3, i18n("Print Data") );
    page3Item->setObjectName("page3");
}

void BatchAssistant::setupPage4()
{
    page4 = new QWidget( this );
    QVBoxLayout* page4_layout = new QVBoxLayout;
    page4_layout->setSpacing( 5 );

    QWidget* hbox = new QWidget;
    page4_layout->addWidget(hbox);
    QHBoxLayout* hbox_layout = new QHBoxLayout;
    hbox_layout->setSpacing( 5 );
    
    buttonTableInsert = new KPushButton( i18n("Insert Row") );
    hbox_layout->addWidget(buttonTableInsert);
    buttonTableInsert->setIcon( KIcon( "document-edit" ) );
    buttonTableRemove = new KPushButton( i18n("Delete Row") );
    hbox_layout->addWidget(buttonTableRemove);
    buttonTableRemove->setIcon( KIcon( "edit-delete") );
    hbox->setLayout(hbox_layout);

    m_varTable = new QTableWidget;
    page4_layout->addWidget(m_varTable);
    m_varTable->setSelectionBehavior(QTableWidget::SelectRows);
    m_varTable->setSelectionMode(QTableWidget::SingleSelection);
    page4->setLayout(page4_layout);

    page4Item = addPage( page4, i18n("Import Variables") );
    page4Item->setObjectName("page4");

    connect( buttonTableInsert, SIGNAL( clicked() ), this, SLOT( slotTableInsert() ) );
    connect( buttonTableRemove, SIGNAL( clicked() ), this, SLOT( slotTableRemove() ) );
}

void BatchAssistant::setupPage5()
{
    TokenProvider serial( this );

    page5 = new QWidget( this );
    QVBoxLayout* page5_layout = new QVBoxLayout;

    QLabel *label = new QLabel( i18n( "<qt>KBarcode has support for placing serial numbers on labels. "
		      "If you did not use the [serial] token on your label in "
		      "a text field or a barcode, you can skip this page.<br>"
		      "Serial start is a free form start value containing at least one "
		      "number. This number is increased for every printed label on the "
		      "print out.</qt>") );
    label->setWordWrap( true );
    page5_layout->addWidget(label);

    QWidget* hbox = new QWidget;
    QHBoxLayout* hbox_layout = new QHBoxLayout;
    page5_layout->addWidget(hbox);
    hbox_layout->setSpacing( 5 );
    
    label = new QLabel( i18n( "Serial start:" ) );
    hbox_layout->addWidget(label);
    serialStart = new KLineEdit( serial.serial() );
    hbox_layout->addWidget(serialStart);

    serialInc = new KIntNumInput( 1 );
    hbox_layout->addWidget(serialInc);
    serialInc->setLabel( i18n( "Serial increment:" ), Qt::AlignLeft | Qt::AlignTop );
    serialInc->setRange( 1, 10000, 1 );
    serialInc->setSliderEnabled( false );
    hbox->setLayout(hbox_layout);
    page5->setLayout(page5_layout);

    KPageWidgetItem * page5Item = addPage( page5, i18n("Serial Number") );
    page5Item->setObjectName("page5");
}

void BatchAssistant::setupPage10()
{
    page10 = new QWidget( this );
    QVBoxLayout* pageLayout = new QVBoxLayout( page10 );
    pageLayout->setContentsMargins( 11, 11, 11, 11 );
    pageLayout->setSpacing( 6 );
    pageLayout->setObjectName( "page10Layout" );

    QGroupBox* group = new QGroupBox( page10 );
    QButtonGroup* buttonGroup = new QButtonGroup( page10 );
    QVBoxLayout* button_layout = new QVBoxLayout;
    
    radioPrinter = new QRadioButton( i18n("&Print to a system printer or to a file"));
    buttonGroup->addButton(radioPrinter);
    button_layout->addWidget(radioPrinter);
    radioImage = new QRadioButton( i18n("&Create images"));
    buttonGroup->addButton(radioImage);

    imageBox = new QWidget;
	QVBoxLayout* imageBox_layout = new QVBoxLayout;
    imageBox_layout->setContentsMargins( 10, 10, 10, 10 );
	imageBox->setLayout(imageBox_layout);
	
    radioBarcode = new QRadioButton( i18n("Print to a special &barcode printer"));
    buttonGroup->addButton(radioBarcode);
    button_layout->addWidget(radioImage);
    button_layout->addWidget(imageBox);
    button_layout->addWidget(radioBarcode);
    group->setLayout(button_layout);

    QWidget* directoryBox = new QWidget;
	QHBoxLayout* directoryBox_layout = new QHBoxLayout;
	directoryBox->setLayout(directoryBox_layout);
	imageBox_layout->addWidget(directoryBox);// -!F: keep
    directoryBox_layout->setSpacing( 5 );
    QLabel* label = new QLabel( i18n("Output &Directory:") );
	directoryBox_layout->addWidget(label);
    imageDirPath = new KUrlRequester;
	directoryBox_layout->addWidget(imageDirPath);
    imageDirPath->setMode( KFile::Directory | KFile::ExistingOnly | KFile::LocalOnly );
    label->setBuddy( directoryBox );

    QWidget* formatBox = new QWidget;
	QHBoxLayout* formatBox_layout = new QHBoxLayout;
	formatBox->setLayout(formatBox_layout);
	imageBox_layout->addWidget(formatBox);
    label = new QLabel( i18n("Output File &Format:") );
	formatBox_layout->addWidget(label);

    QStringList formats = KImageIO::types( KImageIO::Writing );
    comboFormat = new KComboBox( false );
	formatBox_layout->addWidget(comboFormat);
    comboFormat->insertItems( 0, formats );
    if( formats.contains( PNG_FORMAT ) )
	  comboFormat->setCurrentIndex( formats.indexOf( PNG_FORMAT ) );
    else if( formats.contains( QString( PNG_FORMAT ).toLower() ) ) {
        comboFormat->setCurrentIndex( formats.indexOf( QString( PNG_FORMAT ).toLower() ) );
    }
    label->setBuddy( comboFormat );

    QGroupBox* imageNameGroup = new QGroupBox( i18n("&Filename:") );
	imageBox_layout->addWidget(imageNameGroup);
    QVBoxLayout* image_button_layout = new QVBoxLayout;
    QButtonGroup* buttonGroup2 = new QButtonGroup( page10 );
    radioImageFilenameArticle = new QRadioButton( i18n("Use &article number for filename"));
    buttonGroup2->addButton(radioImageFilenameArticle);
    image_button_layout->addWidget(radioImageFilenameArticle);
    radioImageFilenameBarcode = new QRadioButton( i18n("Use &barcode number for filename"));
    buttonGroup2->addButton(radioImageFilenameBarcode);
    image_button_layout->addWidget(radioImageFilenameBarcode);
    radioImageFilenameCustom  = new QRadioButton( i18n("Use &custom filename:"));
    buttonGroup2->addButton(radioImageFilenameCustom);
    image_button_layout->addWidget(radioImageFilenameCustom);
    editImageFilename = new KLineEdit( imageNameGroup );
    radioImageFilenameBarcode->setChecked( true );
    image_button_layout->addWidget(editImageFilename);
    imageNameGroup->setLayout(image_button_layout);

    labelInfo = new QLabel;
	pageLayout->addWidget(labelInfo);

    radioPrinter->setChecked( true );

    checkKeepOpen = new QCheckBox( i18n("&Keep window open after printing.") );
	pageLayout->addWidget(checkKeepOpen);

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

    KPageWidgetItem * page10Item = addPage( page10, i18n("Output Device") );
    page10Item->setObjectName("page10");
}

void BatchAssistant::setupStackPage1()
{
    stack1 = new QWidget;
    stack1->setObjectName( "stack1" );
	QVBoxLayout* stack1_layout = new QVBoxLayout;
	stack1->setLayout(stack1_layout);
    stack1_layout->setSpacing( 5 );

    QWidget* hbox = new QWidget;
	QHBoxLayout* hbox_layout = new QHBoxLayout;
    hbox_layout->setSpacing( 5 );
	hbox->setLayout(hbox_layout);

    hbox_layout->addWidget( new QLabel( i18n( "Customer name and no.:" ) ));
    customerName = new KComboBox( false );
	hbox_layout->addWidget(customerName);
    customerId = new KComboBox( false );
	hbox_layout->addWidget(customerId);
        stack1_layout->addWidget(hbox);

    QWidget* hButtonBox = new QWidget;
	QHBoxLayout* hButtonBox_layout = new QHBoxLayout;
	hButtonBox->setLayout(hButtonBox_layout);
    hButtonBox_layout->setSpacing( 5 );

    buttonAdd = new KPushButton( i18n( "&Add..." ) );
	hButtonBox_layout->addWidget(buttonAdd);
    buttonImport = new KPushButton( i18n("&Import...") );
	hButtonBox_layout->addWidget(buttonImport);
    buttonEdit = new KPushButton( i18n( "&Edit..." ) );
	hButtonBox_layout->addWidget(buttonEdit);
    buttonRemove = new KPushButton( i18n("&Remove" ) );
	hButtonBox_layout->addWidget(buttonRemove);
    buttonRemoveAll = new KPushButton( i18n("R&emove All") );
	hButtonBox_layout->addWidget(buttonRemoveAll);
    stack1_layout->addWidget(hButtonBox);

    KAction* importFromFileAct = new KAction( this );
    importFromFileAct->setText( i18n( "Import from File ..." ) );
    connect( importFromFileAct, SIGNAL( triggered( bool ) ), this, SLOT( loadFromFile() ) );

    KAction* importFromClipboardAct = new KAction( this );
    importFromClipboardAct->setText( i18n( "Import from Clipboard ..." ) );
    connect( importFromClipboardAct, SIGNAL( triggered( bool ) ), this, SLOT( loadFromClipboard() ) );

    KAction* importBarcode_basicAct = new KAction( this );
    importBarcode_basicAct->setText( i18n( "Import barcode_basic" ) );
    connect( importBarcode_basicAct, SIGNAL( triggered( bool ) ), this, SLOT( addAllItems() ) );
    
    KMenu* mnuImport = new KMenu( this );
    mnuImport->addAction( importFromFileAct );
    mnuImport->addAction( importFromClipboardAct );
    mnuImport->addAction( importBarcode_basicAct );
    buttonImport->setMenu( mnuImport );

    sqlList = new QTreeWidget( stack1 );
	QTreeWidgetItem* header = new QTreeWidgetItem;
	header->setText(0, i18n("Index"));
	header->setText(1, i18n("Number of Labels") );
	header->setText(2,  i18n("Article Number") );
	header->setText(3, i18n("Group") );
	sqlList->setHeaderItem(header);
    sqlList->setAllColumnsShowFocus( true );
    sqlList->resizeColumnToContents( 0 );
    sqlList->resizeColumnToContents( 1 );
    sqlList->resizeColumnToContents( 2 );
    sqlList->resizeColumnToContents( 3 );
    stack1_layout->addWidget( sqlList );
    connect( sqlList, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ),
             this, SLOT( changeItem( QTreeWidgetItem *, int ) ) );

    connect( customerName, SIGNAL( activated(int) ), this, SLOT( customerNameChanged(int) ) );
    connect( customerId, SIGNAL( activated(int) ), this, SLOT( customerIdChanged(int) ) );
    connect( buttonAdd, SIGNAL( clicked() ), this, SLOT( addItem() ) );
    connect( buttonEdit, SIGNAL( clicked() ), this, SLOT( editItem() ) );
    connect( buttonRemove, SIGNAL( clicked() ), this, SLOT( removeItem() ) );
    connect( buttonRemoveAll, SIGNAL( clicked() ), sqlList, SLOT( clear() ) );
    connect( buttonRemoveAll, SIGNAL( clicked() ), this, SLOT( enableControls() ) );

    page3->layout()->addWidget( stack1 );
}

void BatchAssistant::setupStackPage2()
{
    stack2 = new QWidget;
    stack2->setObjectName( "stack2" );
    QHBoxLayout* stack2_layout = new QHBoxLayout;
    stack2->setLayout(stack2_layout);
    stack2_layout->setSpacing( 5 );
    page3->layout()->addWidget(stack2);

    QGroupBox* group = new QGroupBox( stack2 );
    QVBoxLayout* group_layout = new QVBoxLayout;
    radioImportManual = new QRadioButton( i18n("Enter &data manually") );
    group_layout->addWidget(radioImportManual);
    radioImportSql = new QRadioButton( i18n("Import variables from a &SQL table") );
    group_layout->addWidget(radioImportSql);
    labelWarningSqlQuery = new QLabel( 
        i18n("<qt>The query you enter can damage data in the database " 
        "so only SQL statements that don't modify the database " 
        "(i.e. SELECT statements) should be used.</qt>") );
    labelWarningSqlQuery->setWordWrap( true );
    group_layout->addWidget(labelWarningSqlQuery);
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
    comboEncoding = new EncodingCombo( group );
    group_layout->addWidget(comboEncoding);
    labelEncoding->setBuddy( comboEncoding );
    radioImportManual->setChecked( true );
    group->setLayout(group_layout);
    stack2_layout->addWidget( group );

    QWidget* box = new QWidget;
    QVBoxLayout* box_layout = new QVBoxLayout;
    box->setLayout(box_layout);
    box_layout->setSpacing( 5 );
    QLabel* label;
    label = new QLabel( i18n("Available Variables:") );
    box_layout->addWidget(label);
    m_varList = new KListWidget( box );
    box_layout->addWidget( m_varList );
    stack2_layout->addWidget( box );

    connect( radioImportManual, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( radioImportSql, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( radioImportCSV, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( importSqlQuery, SIGNAL( textChanged( const QString & ) ), this, SLOT( enableControls() ) );
    connect( importCsvFile, SIGNAL( textChanged( const QString & ) ), this, SLOT( enableControls() ) );
}

void BatchAssistant::setupStackPage3()
{
    stack3 = new QWidget;
    stack3->setObjectName( "stack3" );
    QVBoxLayout* stack3_layout = new QVBoxLayout;
    stack3->setLayout(stack3_layout);

    numLabels = new KIntNumInput( 1, stack3 );
    numLabels->setRange( 1, 100000, 1 );
    numLabels->setSliderEnabled( true );
    numLabels->setLabel( i18n("&Number of labels to print:"), Qt::AlignLeft | Qt::AlignTop );
    stack3_layout->addWidget( numLabels );

    page3->layout()->addWidget( stack3 );
}

void BatchAssistant::setupStackPage4()
{
    stack4 = new QWidget;
    stack4->setObjectName( "stack4" );
    page3->layout()->addWidget(stack4);
    
    QHBoxLayout* mainLayout = new QHBoxLayout( stack4 );
    stack4->setLayout( mainLayout );

    QWidget* list1 = new QWidget;
    QVBoxLayout* list1_layout = new QVBoxLayout;
    list1->setLayout(list1_layout);

    QWidget* list2 = new QWidget;
    QVBoxLayout* list2_layout = new QVBoxLayout;
    list2->setLayout(list2_layout);

    QFrame* buttons = new QFrame( stack4 );
    QVBoxLayout* layout = new QVBoxLayout( buttons );
    layout->setContentsMargins( 10, 10, 10, 10 );
    buttons->setLayout( layout );
    QSpacerItem* spacer1 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding );
    QSpacerItem* spacer2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding );

    buttonAddAllAddress  = new KPushButton( buttons );
    buttonAddAddress = new KPushButton( buttons );
    buttonRemoveAddress = new KPushButton( buttons );
    buttonRemoveAllAddress = new KPushButton( buttons );

    buttonAddAllAddress->setIcon( KIcon( "arrow-right-double" ) );
    buttonAddAddress->setIcon( KIcon( "arrow-right" ) );
    buttonRemoveAddress->setIcon( KIcon( "arrow-left" ) );
    buttonRemoveAllAddress->setIcon( KIcon( "arrow-left-double" ) );

    buttonAddAllAddress->setToolTip( i18n("Add all contacts to the list of contacts which will be printed.") );
    buttonAddAddress->setToolTip( i18n("Add selected contacts to the list of contacts which will be printed.") );
    buttonRemoveAddress->setToolTip( i18n("Remove selected contacts from the list of contacts which will be printed.") );
    buttonRemoveAllAddress->setToolTip( i18n("Remove all contacts from the list of contacts which will be printed.") );

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

    list1_layout->addWidget(new QLabel( i18n("All Addresses") ));
    
    list2_layout->addWidget(new QLabel( i18n("Selected Addresses") ));

    listAddress = new QTreeWidget;
    list1_layout->addWidget(listAddress);
    QTreeWidgetItem* header = new QTreeWidgetItem;
    header->setText(0, i18n("Given Name"));
    header->setText(1, i18n("Family Name") );
    header->setText(2,  i18n("Email Address") );
    listAddress->setHeaderItem(header);
    listAddress->setSelectionMode( QAbstractItemView::MultiSelection );
    listAddress->setAllColumnsShowFocus( true );

    listAddress->resizeColumnToContents( 0 );
    listAddress->resizeColumnToContents( 1 );
    listAddress->resizeColumnToContents( 2 );

    listSelectedAddress = new QTreeWidget;
    list2_layout->addWidget(listSelectedAddress);
    QTreeWidgetItem* header2 = new QTreeWidgetItem;
    header2->setText( 0, i18n("Given Name") );
    header2->setText( 1, i18n("Family Name") );
    header2->setText( 2, i18n("Email Address") );
    listSelectedAddress->setHeaderItem( header2 );
    listSelectedAddress->setSelectionMode( QAbstractItemView::MultiSelection );
    listSelectedAddress->setAllColumnsShowFocus( true );

    listSelectedAddress->resizeColumnToContents( 0 );
    listSelectedAddress->resizeColumnToContents( 1 );
    listSelectedAddress->resizeColumnToContents( 2 );

    connect( buttonAddAddress, SIGNAL( clicked() ), this, SLOT( slotAddAddress() ) );
    connect( buttonRemoveAddress, SIGNAL( clicked() ), this, SLOT( slotRemoveAddress() ) );
    connect( buttonAddAllAddress, SIGNAL( clicked() ), this, SLOT( slotAddAllAddress() ) );
    connect( buttonRemoveAllAddress, SIGNAL( clicked() ), this, SLOT( slotRemoveAllAddress() ) );

}

void BatchAssistant::setupSql()
{
    SqlTables* tables = SqlTables::getInstance();
    if( !tables->isConnected() )
        return;

    QSqlQuery cur( "select customer_no, customer_name from " TABLE_CUSTOMER );
    customerId->clear();
    customerName->clear();
    while ( cur.next() ) {
        customerId->addItem( cur.value(0).toString() );
        customerName->addItem( cur.value(1).toString() );
    }
}

void BatchAssistant::enableControls()
{
    setAppropriate( page4Item, radioVarImport->isChecked() );

    radioSqlArticles->setEnabled( SqlTables::getInstance()->isConnected() );
    radioImportSql->setEnabled( SqlTables::getInstance()->isConnected() );

    importCsvFile->setEnabled( radioImportCSV->isChecked() );
    labelCsvFile->setEnabled( radioImportCSV->isChecked() );
    importSqlQuery->setEnabled( radioImportSql->isChecked() );
    labelSqlQuery->setEnabled( radioImportSql->isChecked() );
    labelWarningSqlQuery->setEnabled( radioImportSql->isChecked() );
    labelEncoding->setEnabled( radioImportCSV->isChecked() );
    comboEncoding->setEnabled( radioImportCSV->isChecked() );

    buttonRemove->setEnabled( sqlList->topLevelItemCount() );
    buttonRemoveAll->setEnabled(sqlList->topLevelItemCount() );
    buttonEdit->setEnabled( sqlList->topLevelItemCount() );

    imageBox->setEnabled( radioImage->isChecked() );

    if( radioImportSql->isChecked() ) {
	page3NextButtonEnable = !importSqlQuery->text().isEmpty();
    } else if( radioImportCSV->isChecked() ) {
	page3NextButtonEnable = !importCsvFile->url().isEmpty();
    } else if( radioImportManual->isChecked() ) {
	page3NextButtonEnable = true;
    }

    editImageFilename->setEnabled( radioImageFilenameCustom->isChecked() );
    radioImageFilenameArticle->setEnabled( radioSqlArticles->isChecked() );

    page1NextButtonEnable = !m_url->url().path().isEmpty();
    
    if( radioAddressBook->isChecked() ) {
        page3NextButtonEnable = listSelectedAddress->topLevelItemCount();
    }

    if( radioImage->isChecked() ) {
	page10FinishButtonEnable = !imageDirPath->url().isEmpty();
    } else {
	page10FinishButtonEnable = true;
    }
    
    QString currentPageName = currentPage()->objectName();
    if( currentPageName == QString( "page1" ) ) {
        enableButton( KDialog::User2, page1NextButtonEnable );
    } else if( currentPageName == QString( "page3" ) ) {
        enableButton( KDialog::User2, page3NextButtonEnable );
    } else if( currentPageName == QString( "page10" ) ) {
        enableButton( KDialog::User1, page10FinishButtonEnable );
    }
}

void BatchAssistant::configureCurrentPage( KPageWidgetItem* page )
{
    if( page->objectName() == QString( "page3" ) )
    {
	if( radioSqlArticles->isChecked() )
	    page3->setCurrentWidget( stack1 );
	else if( radioVarImport->isChecked() )
	{
	    page3->setCurrentWidget( stack2 );
	    fillVarList();
	}
	else if( radioSimple->isChecked() )
	    page3->setCurrentWidget( stack3 );
        else if( radioAddressBook->isChecked() )
        {
            page3->setCurrentWidget( stack4 );
            fillAddressList();
            enableControls();
        }
    }
    else if( page->objectName() == QString( "page4" ) ) {
	if( !fillVarTable() ) {
	    back();
        }
    }
    else if( page->objectName() == QString( "page10" ) ) {
        enableButton( KDialog::User1, page10FinishButtonEnable );
    }
}

/* This slot is called when a user clicks the next button.*/
void BatchAssistant::next() 
{
    KAssistantDialog::next();
    
    configureCurrentPage(currentPage());
}

/* This slot is called when a user clicks the back button.*/
void BatchAssistant::back() 
{
    KAssistantDialog::back();
    
    configureCurrentPage(currentPage());
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
    if( !QFile::exists( m_url->url().path() ) ) 
    {
        KMessageBox::error( this, QString( i18n("The label file %1 was not found") ).arg( m_url->url().path() ) );
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
	
	prn = PrinterSettings::getInstance()->setupPrinter( KUrl(), this, !printer.isEmpty(), printer );
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
	batch = new BatchPrinter( imageDirPath->url().path(), this );
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
        KAssistantDialog::accept();

    KApplication::changeOverrideCursor( QCursor( Qt::ArrowCursor ) );
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

    QBuffer buffer( & m_bytearray );
    if( !buffer.open( QIODevice::ReadOnly ) )
        return;

    batch->setBuffer( &buffer );
    batch->setSerial( serialStart->text(), serialInc->value() );
    batch->setName( m_url->url().path() );
    batch->setDefinition( def );
    batch->setImageFormat( comboFormat->currentText() );

    if( radioSqlArticles->isChecked() )
    {
	int labels = 0;
	batch->setCustomer( customerId->currentText() );
	
        // sort by group
	sqlList->setSortingEnabled( true );
	sqlList->sortItems( 3, Qt::AscendingOrder );

        int itemIndex = 0;
	QList<BatchPrinter::data>* dlist = new QList<BatchPrinter::data>;
	QTreeWidgetItem* item = sqlList->topLevelItem( itemIndex );
	while( item ) 
	{
	    BatchPrinter::data m_data;
	    m_data.number = item->text( 1 ).toInt();
	    labels += m_data.number;
	    m_data.article_no = item->text( 2 );
	    m_data.group = item->text( 3 );
	    
	    dlist->append( m_data );
            itemIndex = itemIndex + 1;
	    item = sqlList->topLevelItem( itemIndex );
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
	for( int i=0; i<m_varTable->rowCount(); i++ )
	{
	    QMap<QString, QString> map;
	    for( int z=0; z<m_varTable->columnCount(); z++ ) {
		  QTableWidgetItem* item = m_varTable->item(i, z);
		  if(item)
			map[ m_varTable->horizontalHeaderItem( z )->text() ] = item->text();
		  else
			map[ m_varTable->horizontalHeaderItem( z )->text() ] = "";
		}
	    tVariableList->append( map );
	}

	batch->setData( tVariableList );
    }
    else if( radioAddressBook->isChecked() )
    {
        KABC::AddresseeList* list = new KABC::AddresseeList;
        int itemIndex = 0;
        QTreeWidgetItem* item = listSelectedAddress->topLevelItem( itemIndex );
        while( item )
        {
            list->append( static_cast<AddressListViewItem*>(item)->address() );
            itemIndex = itemIndex + 1;
            item = listSelectedAddress->topLevelItem( itemIndex );
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
    temp.sprintf("%0*i", 5, sqlList->topLevelItemCount() + 1 );

    QStringList itemLabels;
    itemLabels << temp << QString( "%1" ).arg( count ) << article << group;
    QTreeWidgetItem* item = new QTreeWidgetItem( itemLabels );
    sqlList->addTopLevelItem( item );
    
    sqlList->resizeColumnToContents( 0 );
    sqlList->resizeColumnToContents( 1 );
    sqlList->resizeColumnToContents( 2 );
    sqlList->resizeColumnToContents( 3 );

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
    QList<QTreeWidgetItem *> itemsList = sqlList->selectedItems();
    if( itemsList.isEmpty() ) {
        return;
    }
    QTreeWidgetItem * item = itemsList.first();
    if( item )
        changeItem( item, 0 );
}

void BatchAssistant::changeItem( QTreeWidgetItem* item, int column )
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
        
        sqlList->resizeColumnToContents( 0 );
        sqlList->resizeColumnToContents( 1 );
        sqlList->resizeColumnToContents( 2 );
        sqlList->resizeColumnToContents( 3 );
    }
}

void BatchAssistant::removeItem() 
{
    int itemIndex = 0;
    QTreeWidgetItem* item = sqlList->topLevelItem( itemIndex );
    while( item ) 
    {
        if( item->isSelected() ) 
	{
            itemIndex = itemIndex + 1;
            QTreeWidgetItem* it = sqlList->topLevelItem( itemIndex );
            delete item;

            while( it ) 
	    {
                int a = it->text( 0 ).toInt();
                QString temp;
                temp.sprintf("%0*i", 5, a - 1 );
                it->setText( 0, temp );
                itemIndex = itemIndex + 1;
                it = sqlList->topLevelItem( itemIndex );
            }

            break;
        } else {
            itemIndex = itemIndex + 1;
            item = sqlList->topLevelItem( itemIndex );
        }
    }

    enableControls();
}

void BatchAssistant::customerIdChanged( int index ) 
{
    customerName->setCurrentIndex( index );
    enableControls();
}

void BatchAssistant::customerNameChanged( int index ) 
{
    customerId->setCurrentIndex( index );
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
	    temp.sprintf("%0*i", 5, sqlList->topLevelItemCount() + 1 );
            QStringList labelsList;
            labelsList << temp << num << query.value( 0 ).toString() << group;
	    new QTreeWidgetItem( sqlList, labelsList );
	}

        enableControls();
        
        sqlList->resizeColumnToContents( 0 );
        sqlList->resizeColumnToContents( 1 );
        sqlList->resizeColumnToContents( 2 );
        sqlList->resizeColumnToContents( 3 );
    }
}

void BatchAssistant::loadFromFile() 
{
    QString f = KFileDialog::getOpenFileName( KUrl(), QString(), this );
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
        qDebug("Unable to open file: %s", qPrintable( url ) );
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
    QByteArray baData = data.toUtf8();
    QBuffer buf( & baData );
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
    if( m_bytearray_filename != m_url->url().path() )
    {
	QFile f( m_url->url().path() );
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

    TokenProvider token( this );
    Definition* def = NULL;

    QString description;
    bool kbarcode18;
    util.readXMLHeader( &doc, description, kbarcode18, &def );
    util.readDocumentItems( &list, &doc, &token, kbarcode18 );

    token.setCurrentDocumentItems( list );

    QStringList vars = token.listUserVars();
    m_varList->clear();
    m_varList->addItems( vars );
    m_varTable->setColumnCount( vars.count() );
    for( int i = 0; i < vars.count(); i++ )
    {
	/*vars[i] = vars[i].right( vars[i].length() - 1 );*/// -!F: why? this line strips the first character which results in "rticle_desc" for QString "article_desc" etc. 
	QTableWidgetItem * item = new QTableWidgetItem( vars[i] );
        m_varTable->setHorizontalHeaderItem( i, item );
    }

    delete def;
}

void BatchAssistant::fillAddressList()
{
    Akonadi::ContactSearchJob *job = new Akonadi::ContactSearchJob( this );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( contactSearchJobResult( KJob* ) ) );
    
    listAddress->clear();
}

void BatchAssistant::contactSearchJobResult( KJob *job )
{
    Akonadi::ContactSearchJob *searchJob = qobject_cast<Akonadi::ContactSearchJob*>( job );
    KABC::Addressee::List contacts = searchJob->contacts();
    
    KABC::Addressee::List::Iterator it;
    listAddress->setUpdatesEnabled( false );
    for ( it = contacts.begin(); it != contacts.end(); ++it ) {
        bool isAlreadyInListSelectedAddress = false;
        for( int i = 0; i < listSelectedAddress->topLevelItemCount(); i++ ) {
            if( (*it).uid() == ((AddressListViewItem*) listSelectedAddress->topLevelItem( i ))->address().uid() ) {
                isAlreadyInListSelectedAddress = true;
                break;
            }
        }
        if( !isAlreadyInListSelectedAddress ) {
            new AddressListViewItem( listAddress, *it );
        }
    }
    listAddress->setUpdatesEnabled( true );
}

bool BatchAssistant::fillVarTable()
{
    int resultTableDialog = KDialog::None;
    if( m_varTable->rowCount() != 0 ) {
        KDialog dialog( this );
        dialog.setCaption( i18n( "What to do with already inserted data?" ) );
        dialog.setButtons( KDialog::User1 | KDialog::User2 | KDialog::User3 );
        dialog.setButtonText( KDialog::User1, i18n( "Append" ) );
        dialog.setButtonText( KDialog::User2, i18n( "Replace" ) );
        dialog.setButtonText( KDialog::User3, i18n( "Keep" ) );
        dialog.setDefaultButton( KDialog::User3 );
        dialog.setWindowModality( Qt::WindowModal );
        QLabel label(
            i18n( "<qt>There are already data in the table widget " 
            "becouse you visited this page before. <br>" 
            "<b>What do you want to do with the data?</b><br><br>"
            "If you press \"Keep\" data in the table widget will not be deleted " 
            "and not reloaded.<br><br>"
            "If you press \"Replace\" data in the table widget will be deleted " 
            "and if you checked the radio button \"Import variables from a SQL table\" " 
            "or \"Import from a CSV file\" data will also be loaded from an SQL database " 
            "or a CSV file.<br><br>"
            "If you press \"Append\" data in the table widget will not be deleted " 
            "and if you checked the radio button \"Import variables from a SQL table\" " 
            "or \"Import from a CSV file\" other data will also be loaded from an SQL database " 
            "or a CSV file and appended to the data already present in the table widget.<br><br>"
            "If you don't know what to do press \"Keep\".</qt>" ),
            & dialog );
        label.setWordWrap( true );
        dialog.setMainWidget( & label );
        /*connect( & dialog, SIGNAL( buttonClicked( KDialog::ButtonCode ) ), & dialog, SLOT( done( int ) ) );*/// We can't do this becouse KDialog::ButtonCode argument is incompatible with int argument (although KDialog::ButtonCode is actually an int)
        connect( & dialog, SIGNAL( buttonClicked( KDialog::ButtonCode ) ), this, SLOT( slotDataDialogButtonClicked( KDialog::ButtonCode ) ) );
        connect( this, SIGNAL( signalDataDialogButtonClicked( int ) ), & dialog, SLOT( done( int ) ) );
        resultTableDialog = dialog.exec();
    }
    
    int initialRowCount = 0;
    if( resultTableDialog == KDialog::User3 ) {// Keep
        return true;
    } else if( resultTableDialog == KDialog::User2 ) {// Replace
        // Clear the table
        m_varTable->clearContents();
        m_varTable->setRowCount( 0 );
        initialRowCount = 0;
    } else if( resultTableDialog == KDialog::User1 ) {// Append
        initialRowCount = m_varTable->rowCount();
    }// If resultTableDialog is not one of the values then dialog was not displayed.
    
    if( radioImportSql->isChecked() )
    {
        if( !SqlTables::getInstance()->database().isValid() )
        {
            KMessageBox::error( this, i18n("<qt>Can't connect to a database.</qt>") );
            return false;
        }
	QSqlQueryModel queryModel;
        queryModel.setQuery( importSqlQuery->text(), SqlTables::getInstance()->database() );
	if( queryModel.lastError().type() != QSqlError::NoError )
	{
	    KMessageBox::error( this, i18n("<qt>Can't execute SQL query:<br>") + queryModel.lastError().text() + "</qt>" );
	    return false;
	}

        m_varTable->setRowCount( queryModel.rowCount() + initialRowCount );

	for( int y = 0; y < m_varTable->rowCount(); y++ )
	{
	    for( int x=0;x<m_varTable->columnCount();x++ ) {
                QTableWidgetItem * item = new QTableWidgetItem();
                if( item ) {
                    item->setText( queryModel.record( y ).value( m_varTable->horizontalHeaderItem( x )->text() ).toString() );
                    m_varTable->setItem( y + initialRowCount, x, item );
                }
            }
	}
    }
    else if( radioImportCSV->isChecked() )
    {
	CSVFile file( importCsvFile->url().path() );
        file.setEncoding( comboEncoding->currentText() );

	QStringList heading;
	QStringList data;
	int i = initialRowCount;

        file.setCSVFile(true);
	if( !file.isValid() )
	{
	    KMessageBox::error( this, QString( i18n("Can't open file: %1") ).arg( importCsvFile->url().path() ) );
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
		if( m_varTable->rowCount() <= i )
		    m_varTable->setRowCount( i + 100 );	      

                printf("datacount=%i\n", data.count() );
		for( int z = 0; z < data.count(); z++ )
		{
                    printf("numRows=%i\n", m_varTable->columnCount() );
		    for( int x = 0; x < m_varTable->columnCount(); x++ )
                    {
                        printf("horizontal header=%s\n", qPrintable( m_varTable->horizontalHeaderItem( x )->text().toLower() ) );
                        printf("heading=%s\n", qPrintable( heading[z].toLower() ) );
			if( m_varTable->horizontalHeaderItem( x )->text().toLower() == heading[z].toLower() )
			{
                            QTableWidgetItem * item = m_varTable->item( i, x );
                            if( item ) {
                                printf("Reading: (%s)\n", qPrintable( data[z] ));
                                item->setText( data[z] );
                                break;
                            }
			}
                    }
		}

		if( data.count() )
		    i++;
            }
	}

	m_varTable->setRowCount( i );
    }
    m_varTable->resizeColumnsToContents();

    return true;
}

void BatchAssistant::slotTableInsert()
{
    m_varTable->insertRow( m_varTable->rowCount() );
}

void BatchAssistant::slotTableRemove()
{
    m_varTable->removeRow( m_varTable->currentRow() );
}

void BatchAssistant::setFilename( const QString & url )
{
    m_url->setUrl( url );
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

    configureCurrentPage( page3Item );
    setCurrentPage( page3Item );
    configureCurrentPage( page4Item );
    setCurrentPage( page4Item );
}

void BatchAssistant::setImportCsvFile( const QString & filename )
{
    radioImportCSV->setChecked( true );
    radioImportManual->setChecked( false );
    radioImportSql->setChecked( false );

    radioVarImport->setChecked( true );
    radioSqlArticles->setChecked( false );
    radioSimple->setChecked( false );

    importCsvFile->setUrl( filename );

    enableControls();

    configureCurrentPage( page3Item );
    setCurrentPage( page3Item );
    configureCurrentPage( page4Item );
    setCurrentPage( page4Item );
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

void BatchAssistant::moveAddress( QTreeWidget* src, QTreeWidget* dst, bool bAll )
{
    int itemIndex = 0;
    QTreeWidgetItem* item = src->topLevelItem( itemIndex );
    QTreeWidgetItem* cur;

    while( item ) 
    {
        if( bAll || item->isSelected() )
        {
            cur = item;
            itemIndex = itemIndex + 1;
            item = src->topLevelItem( itemIndex );

            src->takeTopLevelItem( src->indexOfTopLevelItem( cur ) );
            dst->addTopLevelItem( cur );
            cur->setSelected( false );
        }
        else {
            itemIndex = itemIndex + 1;
            item = src->topLevelItem( itemIndex );
        }
    }
}

void BatchAssistant::slotDataDialogButtonClicked( KDialog::ButtonCode i )
{
    emit signalDataDialogButtonClicked( int( i ) );
}
