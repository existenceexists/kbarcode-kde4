/***************************************************************************
                          csvimportdlg.cpp  -  description
                             -------------------
    begin                : Don Aug 21 2003
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

#include "csvimportdlg.h"
#include "printersettings.h"
#include "sqltables.h"
#include "encodingcombo.h"

// Qt includes
#include <qcheckbox.h>
#include <qcursor.h>
#include <qfile.h>
#include <q3frame.h>
#include <q3groupbox.h>
#include <qhbuttongroup.h>
#include <q3header.h>
#include <qlabel.h>
#include <qlayout.h>
#include <q3listbox.h>
#include <q3progressdialog.h>
#include <qsqlquery.h>
#include <q3table.h>
#include <qradiobutton.h>
#include <QTextStream>
#include <q3vbox.h>
//Added by qt3to4:
#include <QHBoxLayout>
#include <QGridLayout>
#include <QList>
#include <QVBoxLayout>

// KDE includes
#include <kapplication.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kpushbutton.h>
#include <kurlrequester.h>
#include "csvfile.h"

// import from labelprinter.cpp
extern QString removeQuote( QString text, QString quote );

const char* NOFIELD = "<NONE>";

CSVImportDlg::CSVImportDlg(QWidget *parent )
    : KPageDialog( parent )
{
    setFaceType( KPageDialog::Tabbed );
    setCaption( i18n("Import") );
    setButtons( KDialog::Ok | KDialog::Close );
    setDefaultButton( KDialog::Ok );
    setModal( false );
    showButtonSeparator( true );
    setButtonText( KDialog::Ok, i18n("&Import") );
    setButtonToolTip( KDialog::Ok, i18n("Import the selected file into your tables.") );
    
    createPage1();
    createPage2();

    connect( requester, SIGNAL( textChanged( const QString & ) ), this, SLOT( settingsChanged() ) );
    connect( buttonSet, SIGNAL( clicked() ), this, SLOT( setCol() ) );
    connect( comboSQL, SIGNAL( activated( int ) ), this, SLOT( updateFields() ) );
    connect( databaseName, SIGNAL( textChanged( const QString & ) ), this, SLOT( updateFields() ) );
    connect( comboEncoding, SIGNAL( activated( int ) ), this, SLOT( settingsChanged() ) );
    connect( table->horizontalHeader(), SIGNAL( clicked( int ) ), this, SLOT( updateCol( int ) ) );
    connect( radioCSVFile, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( radioFixedFile, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( buttonAdd, SIGNAL( clicked() ), this, SLOT( addWidth() ) );
    connect( buttonRemove,SIGNAL( clicked() ), this, SLOT( removeWidth() ) );
    connect( comment, SIGNAL( textChanged( const QString & ) ), this, SLOT( settingsChanged() ) );
    connect( quote, SIGNAL( textChanged( const QString & ) ), this, SLOT( settingsChanged() ) );
    connect( separator, SIGNAL( textChanged( const QString & ) ), this, SLOT( settingsChanged() ) );
    connect( checkLoadAll, SIGNAL( clicked() ), this, SLOT( enableControls() ) );

    updateFields();
    enableControls();
        
    show();
}

CSVImportDlg::~CSVImportDlg()
{
}

void CSVImportDlg::createPage1()
{
    QFrame* box = addPage( i18n("&Import Data") );
    QVBoxLayout* layout = new QVBoxLayout( box, 6, 6 );
    QGridLayout* grid = new QGridLayout( 2 );

    requester = new KUrlRequester( box );
    comboEncoding = new EncodingCombo( box );
    comboSQL = new KComboBox( false, box );
    comboSQL->addItem( TABLE_BASIC );
    comboSQL->addItem( TABLE_CUSTOMER );
    comboSQL->addItem( TABLE_CUSTOMER_TEXT );
    comboSQL->addItem( TABLE_LABEL_DEF );
    comboSQL->addItem( i18n("Other table...") );

    databaseName = new KLineEdit( box );
    checkLoadAll = new QCheckBox( i18n("&Load complete file into preview"), box );
    spinLoadOnly = new KIntNumInput( box );
    spinLoadOnly->setLabel( i18n("Load only a number of datasets:"), Qt::AlignLeft | Qt::AlignVCenter );
    spinLoadOnly->setRange( 0, 10000, 1, false );
    checkLoadAll->setChecked( true );

    table = new QTable( box );
    table->setReadOnly( true );

    frame = new QFrame( box );
    QHBoxLayout* layout2 = new QHBoxLayout( frame, 6, 6 );
    
    spinCol = new KIntNumInput( frame );
    spinCol->setLabel( i18n("Column:"), Qt::AlignLeft | Qt::AlignVCenter );
    spinCol->setRange( 0, 0, 0, false );

    comboField = new KComboBox( false, frame );
    buttonSet = new KPushButton( i18n("Set"), frame );
    
    layout2->addWidget( spinCol );
    layout2->addWidget( new QLabel( i18n("Database field to use for this column:"), frame ) );
    layout2->addWidget( comboField );
    layout2->addWidget( buttonSet );
    
    grid->addWidget( new QLabel( i18n("File to import:"), box ), 0, 0 );
    grid->addWidget( requester, 0, 1 );
    grid->addWidget( new QLabel( i18n("Encoding:"), box ), 1, 0 );
    grid->addWidget( comboEncoding, 1, 1 );
    grid->addWidget( new QLabel( i18n("Import into table:"), box ), 2, 0 );
    grid->addWidget( comboSQL, 2, 1 );
    grid->addWidget( new QLabel( i18n("Table Name:"), box ), 3, 0 );
    grid->addWidget( databaseName, 3, 1 );
    grid->addWidget( checkLoadAll, 4, 0 );
    grid->addWidget( spinLoadOnly, 4, 1 );

    layout->addLayout( grid );
    layout->addWidget( table );
    layout->setStretchFactor( table, 2 );
    layout->addWidget( frame );
}

void CSVImportDlg::createPage2()
{
    labelprinterdata* lb = PrinterSettings::getInstance()->getData();
    QFrame* mainBox = addPage( i18n("&Import Settings") );
    QVBoxLayout* layout = new QVBoxLayout( mainBox, 6, 6 );
    QSpacerItem* spacer1 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    QSpacerItem* spacer2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );

    QButtonGroup* buttonGroup = new QButtonGroup( i18n("File Format:"), mainBox );
    QHBoxLayout* group_layout = new QHBoxLayout;
    radioCSVFile = new QRadioButton( i18n("&CSV File") );
    group_layout->addWidget(radioCSVFile);
    radioFixedFile = new QRadioButton( i18n("File with &fixed field width") );
    group_layout->addWidget(radioFixedFile);
    buttonGroup->setLayout(group_layout);
    QHBox* hboxFrame = new QHBox( mainBox );

    groupCSV = new QGroupBox( i18n("CSV File"), hboxFrame );
    groupFixed = new QGroupBox( i18n("Fixed Field Width File"), hboxFrame );

    groupCSV->setColumnLayout(0, Qt::Vertical );
    groupCSV->layout()->setSpacing( 6 );
    groupCSV->layout()->setMargin( 11 );

    QVBoxLayout* vbox = new QVBoxLayout( groupCSV->layout() );
    QGridLayout* grid = new QGridLayout( 2, 2 );
    grid->setSpacing( 6 );
    grid->setMargin( 11 );
    
    QLabel* label = new QLabel( groupCSV );
    label->setText( i18n("Comment:") );
    grid->addWidget( label, 0, 0 );

    comment = new KLineEdit( lb->comment, groupCSV );
    grid->addWidget( comment, 0, 1 );

    label = new QLabel( groupCSV );
    label->setText( i18n( "Separator:" ) );
    grid->addWidget( label, 1, 0 );

    separator = new KLineEdit( lb->separator, groupCSV );
    grid->addWidget( separator, 1, 1 );

    label = new QLabel( groupCSV );
    label->setText( i18n("Quote Character:") );
    grid->addWidget( label, 2, 0 );

    quote  = new KLineEdit( lb->quote, groupCSV );
    grid->addWidget( quote, 2, 1 );

    vbox->addLayout( grid );
    vbox->addItem( spacer1 );

    groupFixed->setColumnLayout(0, Qt::Horizontal );
    groupFixed->layout()->setSpacing( 6 );
    groupFixed->layout()->setMargin( 11 );
    QHBoxLayout* groupFixedLayout = new QHBoxLayout( groupFixed->layout() );
    groupFixedLayout->setAlignment( Qt::AlignTop );

    listWidth = new KListBox( groupFixed );

    buttonAdd = new KPushButton( groupFixed );
    buttonAdd->setText( i18n( "&Add Field" ) );

    buttonRemove = new KPushButton( groupFixed );
    buttonRemove->setText( i18n( "&Remove Field" ) );

    spinNumber = new KIntNumInput( groupFixed );
    spinNumber->setMinValue( 0 );
    spinNumber->setValue( 1 );
    spinNumber->setFocus();

    QVBoxLayout* layout2 = new QVBoxLayout( 0, 6, 6 );
    layout2->addWidget( buttonAdd );
    layout2->addWidget( buttonRemove );
    layout2->addWidget( spinNumber );
    layout2->addItem( spacer2 );
    
    groupFixedLayout->addWidget( listWidth );
    groupFixedLayout->addLayout( layout2 );

    layout->addWidget( buttonGroup );
    layout->addWidget( hboxFrame );

    radioCSVFile->setChecked( true );
}

void CSVImportDlg::settingsChanged()
{
    CSVFile file( requester->url() );
    QStringList list;

    int i = 0;
    unsigned int z;

    initCsvFile( &file );

    table->setNumCols( 0 );
    table->setNumRows( 0 );

    if( !file.isValid() )
        return;
     
    while( !file.isEof() )
    {
        list = file.readNextLine();

        if( table->numCols() < (int)list.count() )
            table->setNumCols( list.count() );
            
        if( table->numRows() <= i )
            // add 100 rows to get a reasonable speed
            table->setNumRows( i + 100 );

        for( z = 0; z < list.count(); z++ )
            table->setText( i, z, list[z] );
            
        if( !checkLoadAll->isChecked() && i > spinLoadOnly->value() ) 
            break;

        i++;
    }
    
    table->setNumRows( i );
    spinCol->setRange( 1, table->numCols(), 1, false );
       
    enableControls();
}

void CSVImportDlg::setCol()
{
    QString text = comboField->currentText();
    int v = spinCol->value() - 1;
    if( text == NOFIELD )
        table->horizontalHeader()->setLabel( v, QString::number( v + 1 ) );
    else {
        for( int i = 0; i < table->horizontalHeader()->count(); i++ )
            if( table->horizontalHeader()->label( i ) == text )
                table->horizontalHeader()->setLabel( i, QString::number( i + 1 ) );
                
        table->horizontalHeader()->setLabel( v, text );
    }
}

QString CSVImportDlg::getDatabaseName() 
{
    bool b = comboSQL->currentItem() == (comboSQL->count()-1);

    databaseName->setEnabled( b );
    return b ? databaseName->text() : comboSQL->currentText();
}

void CSVImportDlg::updateFields()
{
    // also enables databaseName if necessary
    QString name = getDatabaseName();

    comboField->clear();
    comboField->add( NOFIELD );
    QSqlQuery query( SqlTables::getInstance()->driver()->showColumns( name ) );
    while( query.next() )
        comboField->addItem( query.value( 0 ).toString() );

    for( int i = 0; i < table->horizontalHeader()->count(); i++ )
        table->horizontalHeader()->setLabel( i, QString::number( i + 1 ) );
}

void CSVImportDlg::enableControls()
{
    bool b = table->numRows() && table->numCols();
    
    groupCSV->setEnabled( radioCSVFile->isChecked() );
    groupFixed->setEnabled( radioFixedFile->isChecked() );

    spinLoadOnly->setEnabled( !checkLoadAll->isChecked() );

    enableButtonOk( b );
    frame->setEnabled( b );
}

void CSVImportDlg::updateCol( int c )
{
    spinCol->setValue( ++c );
}

void CSVImportDlg::accept()
{
    CSVFile file( requester->url() );
    QHeader* h = table->horizontalHeader();
    QList<int> headers;
    QStringList list;
    QString name = getDatabaseName();
    int i = 0;

    QString q = "INSERT INTO " + name + " (";
    for( int c = 0; c < table->horizontalHeader()->count(); c++ ) {
        bool ok = true;
        h->label( c ).toInt( &ok );
        if( !ok ) {
            q = q + table->horizontalHeader()->label( c ) + ",";
            headers << c;
        }
    }

    // remove last ","
    if( q.right( 1 ) == "," )
        q = q.left( q.length() - 1 );

    q = q + ") VALUES (";

    initCsvFile( &file );
    if( !file.isValid() )
        KMessageBox::error( this, i18n("Cannot load data from the file:") + requester->url() );

    

    KApplication::setOverrideCursor( QCursor( Qt::WaitCursor) );
    while( !file.isEof() )
    {
        list = file.readNextLine();

        QString line = q;
        for( unsigned int c = 0; c < headers.count(); c++ )
            line.append( "'" + list[ headers[c] ] + "'" + "," );

        // remove last ","
        if( line.right( 1 ) == "," )
            line = line.left( line.length() - 1 );

        line = line + ");";

        QSqlQuery query;
        if( !query.exec( line ) )
            qDebug( i18n("Could not import the following line:") + line );
            //KMessageBox::error( this, i18n("Could not import the following line:") + line );
    }

    KApplication::restoreOverrideCursor();
    KMessageBox::information( this, i18n("Data was imported successfully.") );
    KDialogBase::accept();
}

void CSVImportDlg::addWidth()
{
    listWidth->insertItem( QString("%1").arg(spinNumber->value()), -1 );
    settingsChanged();
}

void CSVImportDlg::removeWidth()
{
    unsigned int i = 0;
    do {
        if(listWidth->isSelected( i )) {
            listWidth->removeItem( i );
            listWidth->setSelected( i-1, true );
            return;
        } else
            i++;
    } while( i < listWidth->count() );
    settingsChanged();
}

QList<int> CSVImportDlg::getFieldWidth()
{
    QList<int> list;

    for( unsigned int i=0;i<listWidth->count();i++ ) 
        list << listWidth->text( i ).toInt();

    return list;
}

void CSVImportDlg::initCsvFile( CSVFile* file )
{
    QList<int> width = getFieldWidth();

    file->setEncoding( comboEncoding->currentText() );
    file->setCSVFile( radioCSVFile->isChecked() );
    file->setComment( comment->text() );
    file->setSeparator( separator->text() );
    file->setQuote( quote->text() );
    file->setFieldWidth( width );
}


#include "csvimportdlg.moc"
