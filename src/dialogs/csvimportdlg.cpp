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

#include "dialogs/csvimportdlg.h"
#include "printersettings.h"
#include "sqltables.h"
#include "encodingcombo.h"
#include "csvfile.h"

// Qt includes
#include <qcheckbox.h>
#include <qcursor.h>
#include <qfile.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qsqlquery.h>
#include <qradiobutton.h>
#include <QTextStream>
#include <QTableWidget>
#include <QHeaderView>
#include <QWidget>
#include <QDebug>
#include <QListWidgetItem>
//Added by qt3to4:
#include <QHBoxLayout>
#include <QGridLayout>
#include <QList>
#include <QVBoxLayout>

// KDE includes
#include <kapplication.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <klistwidget.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kpushbutton.h>
#include <kurlrequester.h>

// import from labelprinter.cpp
extern QString removeQuote( QString text, QString quote );

const char* NOFIELD = "<NONE>";

CSVImportDlg::CSVImportDlg(QWidget *parent, const char *name )
    : KPageDialog( parent )
{
    setFaceType( KPageDialog::Tabbed );
    if ( name != 0 ) {
        setObjectName( name );
    }
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
    connect( table->horizontalHeader(), SIGNAL( sectionClicked( int ) ), this, SLOT( updateCol( int ) ) );
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
    
    noWidthItemsSelected = true;// There are no items added to listWidth.
        
    show();
}

CSVImportDlg::~CSVImportDlg()
{
}

void CSVImportDlg::createPage1()
{
    
    QFrame* box = new QFrame();
    QVBoxLayout* layout = new QVBoxLayout( box );
    layout->setContentsMargins( 6, 6, 6, 6 );
    layout->setSpacing( 6 );
    QGridLayout* grid = new QGridLayout();

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
    /*spinLoadOnly->setLabel( i18n("Load only a number of datasets:"), Qt::AlignLeft | Qt::AlignVCenter );*/// when using Qt::AlignVCenter the label is not displayed
    spinLoadOnly->setLabel( i18n("Load only a number of datasets:"), Qt::AlignLeft | Qt::AlignTop );
    spinLoadOnly->setRange( 0, 10000, 1 );
    spinLoadOnly->setSliderEnabled( false );
    checkLoadAll->setChecked( true );

    table = new QTableWidget( box );

    frame = new QFrame( box );
    QHBoxLayout* layout2 = new QHBoxLayout( frame );
    layout2->setContentsMargins( 6, 6, 6, 6 );
    layout2->setSpacing( 6 );
    
    spinCol = new KIntNumInput( frame );
    /*spinCol->setLabel( i18n("Column:"), Qt::AlignLeft | Qt::AlignVCenter );*/// when using Qt::AlignVCenter the label is not displayed
    spinCol->setLabel( i18n("Column:"), Qt::AlignLeft | Qt::AlignTop );
    spinCol->setRange( 0, 0, 1 );
    spinCol->setSliderEnabled( false );

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
    addPage( box, i18n("&Import Data") );
}

void CSVImportDlg::createPage2()
{
    labelprinterdata* lb = PrinterSettings::getInstance()->getData();
    QFrame* mainBox = new QFrame();
    QVBoxLayout* layout = new QVBoxLayout( mainBox );
    layout->setContentsMargins( 6, 6, 6, 6 );
    layout->setSpacing( 6 );
    QSpacerItem* spacer1 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    QSpacerItem* spacer2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );

    QGroupBox* buttonGroup = new QGroupBox( i18n("File Format:"), mainBox );
    QHBoxLayout* group_layout = new QHBoxLayout;
    radioCSVFile = new QRadioButton( i18n("&CSV File") );
    group_layout->addWidget(radioCSVFile);
    radioFixedFile = new QRadioButton( i18n("File with &fixed field width") );
    group_layout->addWidget(radioFixedFile);
    buttonGroup->setLayout(group_layout);
    QWidget* hboxFrame = new QWidget( mainBox );
	QHBoxLayout* hboxFrameLayout = new QHBoxLayout;
	hboxFrame->setLayout(hboxFrameLayout);

    groupCSV = new QGroupBox( i18n("CSV File"));
	hboxFrameLayout->addWidget(groupCSV);
    groupFixed = new QGroupBox( i18n("Fixed Field Width File"));
	hboxFrameLayout->addWidget(groupFixed);

    QVBoxLayout *groupCSVLaout = new QVBoxLayout;
    groupCSV->setLayout(groupCSVLaout);
    groupCSVLaout->setSpacing( 6 );
    groupCSVLaout->setContentsMargins( 11, 11, 11, 11 );

    QVBoxLayout* vbox = new QVBoxLayout();
    groupCSVLaout->addLayout( vbox );
    QGridLayout* grid = new QGridLayout();
    grid->setSpacing( 6 );
    grid->setContentsMargins( 11, 11, 11, 11 );
    
    QLabel* label1 = new QLabel( groupCSV );
    label1->setText( i18n("Comment:") );
    grid->addWidget( label1, 0, 0 );

    comment = new KLineEdit( lb->comment, groupCSV );
    grid->addWidget( comment, 0, 1 );

    QLabel* label2 = new QLabel( groupCSV );
    label2->setText( i18n( "Separator:" ) );
    grid->addWidget( label2, 1, 0 );

    separator = new KLineEdit( lb->separator, groupCSV );
    grid->addWidget( separator, 1, 1 );

    QLabel* label3 = new QLabel( groupCSV );
    label3->setText( i18n("Quote Character:") );
    grid->addWidget( label3, 2, 0 );

    quote  = new KLineEdit( lb->quote, groupCSV );
    grid->addWidget( quote, 2, 1 );

    vbox->addLayout( grid );
    vbox->addItem( spacer1 );

    QHBoxLayout* groupFixedLayoutMain = new QHBoxLayout;
    groupFixed->setLayout( groupFixedLayoutMain );
    groupFixedLayoutMain->setSpacing( 6 );
    groupFixedLayoutMain->setContentsMargins( 11, 11, 11, 11 );
    QHBoxLayout* groupFixedLayout = new QHBoxLayout();
    groupFixedLayoutMain->addLayout( groupFixedLayout );
    groupFixedLayout->setAlignment( Qt::AlignTop );

    listWidth = new KListWidget( groupFixed );

    buttonAdd = new KPushButton( groupFixed );
    buttonAdd->setText( i18n( "&Add Field" ) );

    buttonRemove = new KPushButton( groupFixed );
    buttonRemove->setText( i18n( "&Remove Field" ) );

    spinNumber = new KIntNumInput( groupFixed );
    spinNumber->setMinimum( 0 );
    spinNumber->setValue( 1 );
    spinNumber->setFocus();

    QVBoxLayout* layout2 = new QVBoxLayout();
    layout2->setContentsMargins( 6, 6, 6, 6 );
    layout2->setSpacing( 6 );
    layout2->addWidget( buttonAdd );
    layout2->addWidget( buttonRemove );
    layout2->addWidget( spinNumber );
    layout2->addItem( spacer2 );
    
    groupFixedLayout->addWidget( listWidth );
    groupFixedLayout->addLayout( layout2 );

    layout->addWidget( buttonGroup );
    layout->addWidget( hboxFrame );

    radioCSVFile->setChecked( true );
    
    addPage( mainBox, i18n("&Import Settings") );
}

void CSVImportDlg::settingsChanged()
{
    CSVFile file( requester->url().path() );
    QStringList list;

    int i = 0;
    int z;

    initCsvFile( &file );

    table->clear();
    table->setColumnCount( 0 );
    table->setRowCount( 0 );

    if( !file.isValid() ) {
        return;
    }
    
    while( !file.isEof() )
    {
        list = file.readNextLine();

        if( table->columnCount() < (int)list.count() ) {
            int oldColumnCount = table->columnCount();
            if ( oldColumnCount <= 0 ) {
                oldColumnCount = 1;
            }
            table->setColumnCount( list.count() );
            // Now set all the horizontal header items so that we can get an item later in the method setCol() .
            // Otherwise we would get a runtime error if we would call the method setCol() .
            for( int j = oldColumnCount - 1; j < list.count(); j++ ) {
                QString numberString;
                QTableWidgetItem * headerItem = new QTableWidgetItem( numberString.setNum( j + 1 ) );
                table->setHorizontalHeaderItem( j, headerItem );
            }
        }
            
        if( table->rowCount() <= i ) {
            // add 100 rows to get a reasonable speed
            table->setRowCount( i + 100 );
        }

        for( z = 0; z < table->columnCount(); z++ ) {
            if ( z < list.count() ) {// There should be a text in the table cell.
                QTableWidgetItem * item = new QTableWidgetItem( list[z] );
                Qt::ItemFlags itemFlags = item->flags() & ~Qt::ItemIsEditable;// Set the item as noneditable
                item->setFlags( itemFlags );
                table->setItem( i, z, item );
            } else {// There should be no text in the table cell.
                QTableWidgetItem * item = new QTableWidgetItem();
                Qt::ItemFlags itemFlags = item->flags() & ~Qt::ItemIsEditable;
                item->setFlags( itemFlags );
                table->setItem( i, z, item );
            }
        }
        
        if( !checkLoadAll->isChecked() && i > spinLoadOnly->value() ) {
            break;
        }

        i++;
    }
    
    table->setRowCount( i );
    if ( table->columnCount() > 0 ) {
        spinCol->setRange( 1, table->columnCount(), 1 );
    } else {
        spinCol->setRange( 0, 0, 1 );
    }
    spinCol->setSliderEnabled( false );
       
    enableControls();
}

void CSVImportDlg::setCol()
{
    QString text = comboField->currentText();
    int v = spinCol->value() - 1;
    if( text == NOFIELD ) {
        table->horizontalHeaderItem( v )->setText( QString::number( v + 1 ) );
    }
    else {
        for( int i = 0; i < table->horizontalHeader()->count(); i++ ) {
            if( table->horizontalHeaderItem( i )->text() == text ) {
                table->horizontalHeaderItem( i )->setText( QString::number( i + 1 ) );
            }
        }
        table->horizontalHeaderItem( v )->setText( text );
    }
}

QString CSVImportDlg::getDatabaseName() 
{
    bool b = comboSQL->currentIndex() == (comboSQL->count()-1);

    databaseName->setEnabled( b );
    return b ? databaseName->text() : comboSQL->currentText();
}

void CSVImportDlg::updateFields()
{
    // also enables databaseName if necessary
    QString name = getDatabaseName();

    comboField->clear();
    comboField->addItem( NOFIELD );
    QSqlQuery query( SqlTables::getInstance()->driver()->showColumns( name ) );
    while( query.next() )
        comboField->addItem( query.value( 0 ).toString() );

    for( int i = 0; i < table->horizontalHeader()->count(); i++ )
        table->horizontalHeaderItem( i )->setText( QString::number( i + 1 ) );
}

void CSVImportDlg::enableControls()
{
    bool b = table->rowCount() && table->columnCount();
    
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
    CSVFile file( requester->url().path() );
    QList<int> headers;
    QStringList list;
    QString name = getDatabaseName();

    QString q = "INSERT INTO " + name + " (";
    for( int c = 0; c < table->horizontalHeader()->count(); c++ ) {
        bool ok = true;
        table->horizontalHeaderItem( c )->text().toInt( &ok );
        if( !ok ) {
            q = q + table->horizontalHeaderItem( c )->text() + ",";
            headers << c;
        }
    }

    // remove last ","
    if( q.right( 1 ) == "," )
        q = q.left( q.length() - 1 );

    q = q + ") VALUES (";

    initCsvFile( &file );
    if( !file.isValid() )
        KMessageBox::error( this, i18n("Cannot load data from the file:") + requester->url().path() );

    

    KApplication::setOverrideCursor( QCursor( Qt::WaitCursor) );
    int queriesNum = 0;// Number of all sql queries.
    int queriesFailedNum = 0;// How many sql queries failed.
    while( !file.isEof() )
    {
        list = file.readNextLine();

        QString line = q;
        for( int c = 0; c < headers.count(); c++ )
            line.append( "'" + list[ headers[c] ] + "'" + "," );

        // remove last ","
        if( line.right( 1 ) == "," )
            line = line.left( line.length() - 1 );

        line = line + ");";

        QSqlQuery query;
        if( !query.exec( line ) ) {
            qDebug() << i18n("Could not import the following line:") + line;
            //KMessageBox::error( this, i18n("Could not import the following line:") + line );
            queriesFailedNum += 1;
        }
        queriesNum += 1;
    }

    KApplication::restoreOverrideCursor();
    
    // Show a message that informs how many sql queries were executed succesfully.
    // Destroy the csv import dialog dialog only if at least some of the sql queries were executed succesfully.
    if ( queriesFailedNum == 0 ) {// No sql queries failed.
        KMessageBox::information( this, i18n("Data was imported successfully.") + QString("\n")
            + i18np("%1 of %1 sql query was executed successfully.", 
            "All %1 of %1 sql queries were executed successfully.", queriesNum, queriesNum) );
        KPageDialog::accept();// Destroy the csv import dialog.
    } else if ( (queriesNum - queriesFailedNum) == 0 ) {// All sql queries failed.
        KMessageBox::information( this, i18n("No data was imported successfully.") + QString("\n")
            + i18np("%1 of %2 sql query failed.", 
            "All %1 of %2 sql queries failed.", queriesNum, queriesFailedNum, queriesNum)
            + QString("\n")
            + i18n("No sql query was executed successfully.") );
            // No not destroy the csv import dialog.
    } else {// Only some of the sql queries failed.
        KMessageBox::information( this, i18n("Only a part of data was imported successfully.") + QString("\n")
            + i18np("%1 of %2 sql queries failed.", "%1 of %2 sql queries failed.", 
            queriesFailedNum, queriesFailedNum, queriesNum)
            + QString("\n")
            + i18np("%1 of %2 sql query was executed successfully.", "%1 of %2 sql queries were executed successfully.", 
            queriesNum - queriesFailedNum, queriesNum - queriesFailedNum, queriesNum) );
        KPageDialog::accept();// Destroy the csv import dialog.
    }
}

void CSVImportDlg::addWidth()
{
    listWidth->addItem( QString("%1").arg(spinNumber->value()) );
    if ( noWidthItemsSelected ) {
        noWidthItemsSelected = false;
        listWidth->item( 0 )->setSelected( true );
    }
    settingsChanged();
}

void CSVImportDlg::removeWidth()
{
    if ( listWidth->count() <= 0) {
        return;
    }
    
    int i = 0;
    do {
        if(listWidth->item( i )->isSelected()) {
            QListWidgetItem * item = listWidth->item( i );
            listWidth->removeItemWidget( item );
            delete item;
            if ( listWidth->count() == 0 ) {//don't select anything
                noWidthItemsSelected = true;
            } else if ( i > 0 ) {
                listWidth->item( i-1 )->setSelected( true );
            } else {
                listWidth->item( i )->setSelected( true );
            }
            break;
        } else
            i++;
    } while( i < listWidth->count() );
    settingsChanged();
}

QList<int> CSVImportDlg::getFieldWidth()
{
    QList<int> list;

    for( int i=0;i<listWidth->count();i++ ) 
        list << listWidth->item( i )->text().toInt();

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
