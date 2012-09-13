/***************************************************************************
                          configdialog.cpp  -  description
                             -------------------
    begin                : Fre Apr 26 2002
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

#include "configdialog.h"
#include "printersettings.h"
#include "sqltables.h"

// Qt includes
#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QGroupBox>
#include <qsqldatabase.h>
#include <QToolTip>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QVBoxLayout>

// KDE includes
#include <kapplication.h>
#include <kconfiggroup.h>
#include <kcolorbutton.h>
#include <kiconloader.h>
#include <kimageio.h>
#include <kicon.h>
#include <klineedit.h>
#include <klocale.h>
#include <kcombobox.h>
#include <knuminput.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kglobal.h>
#include <kvbox.h>
#include <kpagewidgetmodel.h>
#include <kstandarddirs.h>

const QString cached = I18N_NOOP( "There are currently %1 cached barcodes." );

ConfigDialog::ConfigDialog( QWidget* parent )
        : KPageDialog( parent )
{
    setFaceType( KPageDialog::List );
    setCaption( i18n( "Configure KBarcode" ) );
    setButtons( KDialog::Ok | KDialog::Cancel );
    setDefaultButton( KDialog::Ok );
    setModal( true );

    setupTab2(); // Printer
    setupTab1(); // SQL
    setupTab4(); // label editor
    setupTab3(); // import
    setupTab5(); // on new
}

ConfigDialog::~ConfigDialog()
{
}

void ConfigDialog::setupTab1( )
{
    KVBox* page = new KVBox();
    KPageWidgetItem* item = addPage( page, i18n( "SQL Settings" ) );
    item->setIcon( KIcon( "network-connect" ) );

    sqlwidget = new SqlWidget( false, page );

    // TODO: test if it's needed here
    // QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding );
}

void ConfigDialog::setupTab2()
{
    labelprinterdata* lb = PrinterSettings::getInstance()->getData();

    QWidget * page = new QWidget();

    QVBoxLayout* layoutMain = new QVBoxLayout();
    QHBoxLayout* Layout0 = new QHBoxLayout();
    QHBoxLayout* Layout1 = new QHBoxLayout();
    layoutMain->addLayout( Layout0 );
    layoutMain->addLayout( Layout1 );

    // QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding );

    printerQuality = new KComboBox( false, this );
    printerQuality->addItem( i18n( "Medium Resolution (300dpi)" ) );
    printerQuality->addItem( i18n( "High Resolution (600dpi)" ) );
    printerQuality->addItem( i18n( "Very High Resolution (1200dpi)" ) );

    switch ( lb->quality )
    {

        case PrinterSettings::Middle:
            printerQuality->setCurrentIndex( 0 );
            break;

        case PrinterSettings::High:
            printerQuality->setCurrentIndex( 1 );
            break;

        case PrinterSettings::VeryHigh:
            printerQuality->setCurrentIndex( 2 );
            break;

        default:
            break;
    }

    pageFormat = new KComboBox( false, this );

    PrinterSettings::getInstance()->insertPageFormat( pageFormat );
    pageFormat->setCurrentIndex( lb->format );

    Layout0->addWidget( new QLabel( i18n( "Printer Resolution:" ), this ) );
    Layout0->addWidget( printerQuality );
    Layout1->addWidget( new QLabel( i18n( "Preview Page Format:" ), this ) );
    Layout1->addWidget( pageFormat );
    
    page->setLayout( layoutMain );
    KPageWidgetItem* item = addPage( page, i18n( "Print Settings" ) );
    item->setIcon( KIcon( "document-print" ) );
}

void ConfigDialog::setupTab3()
{
    labelprinterdata* lb = PrinterSettings::getInstance()->getData();

    QWidget* page = new QWidget();

    QGridLayout* grid = new QGridLayout();

    QLabel* label = new QLabel();
    label->setText( i18n( "Comment:" ) );
    grid->addWidget( label, 0, 0 );

    comment = new KLineEdit( lb->comment );
    grid->addWidget( comment, 0, 1 );

    label = new QLabel();
    label->setText( i18n( "Separator:" ) );
    grid->addWidget( label, 1, 0 );

    separator = new KLineEdit( lb->separator );
    grid->addWidget( separator, 1, 1 );

    label = new QLabel();
    label->setText( i18n( "Quote Character:" ) );
    grid->addWidget( label, 2, 0 );

    quote  = new KLineEdit( lb->quote );
    grid->addWidget( quote, 2, 1 );

    checkUseCustomNo = new QCheckBox( i18n( "&Use customer article no. for import" ) );
    checkUseCustomNo->setChecked( lb->useCustomNo );

    grid->addWidget( checkUseCustomNo, 3, 0, 1, 2 );

    QHBoxLayout* Layout1 = new QHBoxLayout();
    Layout1->addWidget( new QLabel( i18n( "File Format:" ) ) );

    combo1 = new KComboBox();
    combo1->addItem( i18n( "Quantity" ) );
    combo1->addItem( i18n( "Article Number" ) );
    combo1->addItem( i18n( "Group" ) );
    Layout1->addWidget( combo1 );

    combo2 = new KComboBox();
    combo2->addItem( i18n( "Quantity" ) );
    combo2->addItem( i18n( "Article Number" ) );
    combo2->addItem( i18n( "Group" ) );
    Layout1->addWidget( combo2 );

    combo3 = new KComboBox();
    combo3->addItem( i18n( "Quantity" ) );
    combo3->addItem( i18n( "Article Number" ) );
    combo3->addItem( i18n( "Group" ) );
    Layout1->addWidget( combo3 );

    grid->addLayout( Layout1, 4, 0, 1, 2 );
    QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
    grid->addItem( spacer, 5, 0 );

    KConfigGroup config = KGlobal::config()->group( "FileFormat" );
    combo1->setCurrentIndex( config.readEntry( "Data0", 0 ) );
    combo2->setCurrentIndex( config.readEntry( "Data1", 1 ) );
    combo3->setCurrentIndex( config.readEntry( "Data2", 2 ) );

    page->setLayout( grid );
    KPageWidgetItem* item = addPage( page, i18n( "Import" ) );
    item->setIcon( KIcon( "document-import" ) );
}

void ConfigDialog::setupTab4()
{
    QWidget* page = new QWidget( this );
    KPageWidgetItem* item = addPage( page, i18n( "Label Editor" ) );
    item->setIcon( KIcon( KStandardDirs::locate( "appdata", "hi32-app-kbarcode.png" ) ) );

    QGridLayout* tabLayout = new QGridLayout( page );

    checkNewDlg = new QCheckBox( page );
    checkNewDlg->setText( i18n( "&Create a new label on startup" ) );

    date = new KLineEdit( page );
    labelDate = new QLabel( page );

    connect( date, SIGNAL( textChanged( const QString & ) ), this, SLOT( updateDatePreview() ) );

    spinGrid = new KIntNumInput( 0, page );
    spinGrid->setLabel( i18n( "Grid:" ), Qt::AlignLeft | Qt::AlignTop );
    spinGrid->setRange( 2, 100, 1 );
    spinGrid->setSliderEnabled( false );

    colorGrid = new KColorButton( page );

    tabLayout->addWidget( checkNewDlg, 0, 0 );
    tabLayout->addWidget( spinGrid, 1, 0, 1, 2 );
    tabLayout->addWidget( new QLabel( i18n( "Grid Color:" ), page ), 2, 0 );
    tabLayout->addWidget( colorGrid, 2, 1 );
    tabLayout->addWidget( new QLabel( i18n( "Date Format:" ), page ), 3, 0 );
    tabLayout->addWidget( date, 3, 1 );
    tabLayout->addWidget( labelDate, 3, 2 );

    page->setLayout( tabLayout );
}

void ConfigDialog::setupTab5()
{
    labelprinterdata* lb = PrinterSettings::getInstance()->getData();

    KVBox* page = new KVBox();
    KPageWidgetItem* item = addPage( page, i18n( "On New" ) );
    item->setIcon( KIcon( "document-new" ) );

    QStringList alist, glist;
    alist.append( i18n( "No Line Break" ) );
    alist.append( i18n( "Line Break" ) );
    alist.append( i18n( "Insert Label X" ) );
    alist.append( i18n( "New Page" ) );
    alist.append( i18n( "Article No." ) );

    glist.append( i18n( "No Line Break" ) );
    glist.append( i18n( "Line Break" ) );
    glist.append( i18n( "Insert Label X" ) );
    glist.append( i18n( "New Page" ) );
    glist.append( i18n( "Group Name" ) );

    QGroupBox* bg = new QGroupBox( i18n( "On New Article" ), page );
    QGridLayout* bgLayout = new QGridLayout( bg );

    onNewArticle1 = new KComboBox( false, bg );
    onNewArticle2 = new KComboBox( false, bg );
    onNewArticle3 = new KComboBox( false, bg );
    onNewArticle4 = new KComboBox( false, bg );

    onNewArticle1->insertItems( 0, alist );
    onNewArticle2->insertItems( 0, alist );
    onNewArticle3->insertItems( 0, alist );
    onNewArticle4->insertItems( 0, alist );

    bgLayout->setColumnStretch( 1, 3 );

    bgLayout->addWidget( new QLabel( "1.", bg ), 0, 0 );
    bgLayout->addWidget( new QLabel( "2.", bg ), 1, 0 );
    bgLayout->addWidget( new QLabel( "3.", bg ), 2, 0 );
    bgLayout->addWidget( new QLabel( "4.", bg ), 3, 0 );

    bgLayout->addWidget( onNewArticle1, 0, 1 );
    bgLayout->addWidget( onNewArticle2, 1, 1 );
    bgLayout->addWidget( onNewArticle3, 2, 1 );
    bgLayout->addWidget( onNewArticle4, 3, 1 );

    bg->setLayout( bgLayout );

    QGroupBox* bg2 = new QGroupBox( i18n( "On New Group" ), page );
    QGridLayout* bg2Layout = new QGridLayout( bg2 );

    onNewGroup1 = new KComboBox( false, bg2 );
    onNewGroup2 = new KComboBox( false, bg2 );
    onNewGroup3 = new KComboBox( false, bg2 );
    onNewGroup4 = new KComboBox( false, bg2 );

    onNewGroup1->insertItems( 0, glist );
    onNewGroup2->insertItems( 0, glist );
    onNewGroup3->insertItems( 0, glist );
    onNewGroup4->insertItems( 0, glist );

    bg2Layout->setColumnStretch( 1, 3 );

    bg2Layout->addWidget( new QLabel( "1.", bg2 ), 0, 0 );
    bg2Layout->addWidget( new QLabel( "2.", bg2 ), 1, 0 );
    bg2Layout->addWidget( new QLabel( "3.", bg2 ), 2, 0 );
    bg2Layout->addWidget( new QLabel( "4.", bg2 ), 3, 0 );

    bg2Layout->addWidget( onNewGroup1, 0, 1 );
    bg2Layout->addWidget( onNewGroup2, 1, 1 );
    bg2Layout->addWidget( onNewGroup3, 2, 1 );
    bg2Layout->addWidget( onNewGroup4, 3, 1 );

    bg2->setLayout( bg2Layout );

    onNewArticle1->setCurrentIndex( lb->articleEvent1 );
    onNewArticle2->setCurrentIndex( lb->articleEvent2 );
    onNewArticle3->setCurrentIndex( lb->articleEvent3 );
    onNewArticle4->setCurrentIndex( lb->articleEvent4 );

    onNewGroup1->setCurrentIndex( lb->groupEvent1 );
    onNewGroup2->setCurrentIndex( lb->groupEvent2 );
    onNewGroup3->setCurrentIndex( lb->groupEvent3 );
    onNewGroup4->setCurrentIndex( lb->groupEvent4 );
}

void ConfigDialog::accept()
{
    KConfigGroup config = KGlobal::config()->group( "FileFormat" );

    config.writeEntry( "Data0", combo1->currentIndex() );
    config.writeEntry( "Data1", combo2->currentIndex() );
    config.writeEntry( "Data2", combo3->currentIndex() );

    sqlwidget->save();

    labelprinterdata* lpdata = PrinterSettings::getInstance()->getData();
    lpdata->comment = comment->text();
    lpdata->separator = separator->text();
    lpdata->quote = quote->text();
    lpdata->format = pageFormat->currentIndex();

    lpdata->articleEvent1 = onNewArticle1->currentIndex();
    lpdata->articleEvent2 = onNewArticle2->currentIndex();
    lpdata->articleEvent3 = onNewArticle3->currentIndex();
    lpdata->articleEvent4 = onNewArticle4->currentIndex();

    lpdata->groupEvent1 = onNewGroup1->currentIndex();
    lpdata->groupEvent2 = onNewGroup2->currentIndex();
    lpdata->groupEvent3 = onNewGroup3->currentIndex();
    lpdata->groupEvent4 = onNewGroup4->currentIndex();
    lpdata->useCustomNo = checkUseCustomNo->isChecked();

    switch ( printerQuality->currentIndex() )
    {

        case 0:
            lpdata->quality = PrinterSettings::Middle;
            break;

        case 1:
            lpdata->quality = PrinterSettings::High;
            break;

        case 2:
            lpdata->quality = PrinterSettings::VeryHigh;
            break;

        default:
            break;
    }

    QDialog::accept();
}

void ConfigDialog::updateDatePreview()
{
    labelDate->setText( i18n( "Preview: " ) + QDateTime::currentDateTime().toString( date->text() ) );
}

#include "configdialog.moc"
