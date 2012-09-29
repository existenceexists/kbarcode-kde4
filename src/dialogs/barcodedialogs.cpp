/***************************************************************************
                          barcodedialogs.cpp  -  description
                             -------------------
    begin                : Fre Sep 5 2003
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

#include "barcodedialogs.h"
#include "barkode.h"
#include "purepostscript.h"
#include "tbarcode2.h"

// Qt includes
#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QToolTip>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>

// KDE includes
#include <kcombobox.h>
#include <knuminput.h>
#include <klocale.h>
#include <kcolorbutton.h>


AdvancedBarcodeDialog::AdvancedBarcodeDialog( QString type, QWidget* parent )
        : KPageDialog( parent )

{
    setFaceType( KPageDialog::Tabbed );
    setCaption( i18n( "Barcode Settings" ) );
    setButtons( KDialog::Ok | KDialog::Cancel );
    setDefaultButton( KDialog::Ok );

    if ( Barkode::hasFeature( type, PDF417BARCODE ) )
    {
        PDF417BarcodeDlg* dlg = new PDF417BarcodeDlg( this );
        addPage( dlg, i18n( "PDF417" ) );
        list.append(( BarcodeDlgBase* )dlg );
    }

    if ( Barkode::hasFeature( type, DATAMATRIX ) )
    {
        DataMatrixDlg* dlg = new DataMatrixDlg( this );
        addPage( dlg, i18n( "DataMatrix" ) );
        list.append(( BarcodeDlgBase* )dlg );
    }

    if ( Barkode::hasFeature( type, TBARCODEADV ) )
    {
        TBarcodeDlg* dlg = new TBarcodeDlg( this );
        addPage( dlg, i18n( "TBarcode" ) );
        list.append(( BarcodeDlgBase* )dlg );
    }

    if ( Barkode::hasFeature( type, PUREADV ) )
    {
        PurePostscriptDlg* dlg = new PurePostscriptDlg( this );
        addPage( dlg, i18n( "Barcode Writer in Pure Postscript" ) );
        list.append(( BarcodeDlgBase* )dlg );
    }

    if ( Barkode::hasFeature( type, COLORED ) )
    {
        ColorDlg* dlg = new ColorDlg( this );
        addPage( dlg, i18n( "Colors" ) );
        list.append(( BarcodeDlgBase* )dlg );
    }

    SequenceDlg* dlg = new SequenceDlg( this );

    addPage( dlg, i18n( "Sequence" ) );
    list.append(( BarcodeDlgBase* )dlg );
}

AdvancedBarcodeDialog::~AdvancedBarcodeDialog()
{
}

void AdvancedBarcodeDialog::setData( Barkode* b )
{
    for ( int i = 0; i < list.count(); i++ )
    {
        BarcodeDlgBase* bdb = list.at( i );
        bdb->setData( b );
    }
}

void AdvancedBarcodeDialog::getData( Barkode* b )
{
    for ( int i = 0; i < list.count(); i++ )
    {
        BarcodeDlgBase* bdb = list.at( i );
        bdb->getData( b );
    }
}

TBarcodeDlg::TBarcodeDlg( QWidget *parent )
        : QWidget( parent )
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout -> setSpacing( 6 ); // TODO: should we hardcode the spacing and margin values?

    setEnabled( Barkode::haveTBarcode() || Barkode::haveTBarcode2() );

    spinModule = new KDoubleNumInput( this );
    spinModule->setLabel( i18n( "Module width (mm):" ), Qt::AlignLeft | Qt::AlignTop );
    spinModule->setRange( 0.190, 1.500, 0.001, true );

    spinHeight = new KIntNumInput( this );
    spinHeight->setLabel( i18n( "Barcode Height (mm):" ), Qt::AlignLeft | Qt::AlignTop );
    spinHeight->setRange( 1, 1000, 10 );
    spinHeight->setSliderEnabled( false );

    checkEscape = new QCheckBox( i18n( "&Translate escape sequences" ), this );
    checkAbove = new QCheckBox( i18n( "&Text above barcode" ), this );
    checkAutoCorrect = new QCheckBox( i18n( "&Auto correction" ), this );

    comboCheckSum = new KComboBox( false, this );

    QLabel* label = new QLabel( i18n( "&Checksum calculation method:" ), this );
    label->setBuddy( comboCheckSum );

    QHBoxLayout * hbox = new QHBoxLayout( this );

    hbox->addWidget( label );
    hbox->addWidget( comboCheckSum );

    layout->addWidget( spinModule );
    layout->addWidget( spinHeight );
    layout->addWidget( checkEscape );
    layout->addWidget( checkAbove );
    layout->addWidget( checkAutoCorrect );
    layout->addLayout( hbox );

    spinModule->setToolTip( i18n( "<qt>Change the module width used by TBarcode. Take a look into the "
                                     "TBarcode documentation for details. Normally you do not want to change "
                                     "this value.</qt>" ) );

    setLayout( layout );
}

void TBarcodeDlg::setData( Barkode* b )
{
    TBarcodeOptions* options = dynamic_cast<TBarcodeOptions*>( b->engine()->options() );

    if ( options )
    {
        map.clear();

        spinModule->setValue( options->moduleWidth() );
        spinHeight->setValue( options->height() );
        checkEscape->setChecked( options->escape() );
        checkAbove->setChecked( options->above() );
        checkAutoCorrect->setChecked( options->autocorrect() );

        map.insert( 0, i18n( "No Checksum" ) );
        map.insert( 1, i18n( "Default Checksum Method" ) );

        if ( Barkode::hasFeature( b->type(), MODULOALLCHECK ) ||
                Barkode::hasFeature( b->type(), MODULO10CHECK ) )
            map.insert( 2, i18n( "Modulo 10 Checksum" ) );

        if ( Barkode::hasFeature( b->type(), MODULOALLCHECK ) ||
                b->type() == "b13" || // EAN 13
                b->type() == "b14" || // EAN 13
                b->type() == "b15" || // EAN 13
                b->type() == "b18" ) // CodaBar (2 width)
            map.insert( 3, i18n( "Module 43 (suggested for Code39 and Logmars, 1 digit)" ) );

        if ( Barkode::hasFeature( b->type(), MODULOALLCHECK ) )
            map.insert( 4, i18n( "Modula 47 (2 digits)" ) );

        if ( b->type() == "b21" ) // Deutsche Post Leitcode
            map.insert( 5, i18n( "Deutsche Post Leitcode" ) );

        if ( b->type() == "b22" ) // Deutsche Post Identcode
            map.insert( 6, i18n( "Deutsche Post Identcode" ) );

        if ( b->type() == "b1" )  // Code11
        {
            map.insert( 7, i18n( "Code 11 (1 digit)" ) );
            map.insert( 8, i18n( "Code 11 (2 digits)" ) );
        }

        if ( Barkode::hasFeature( b->type(), POSTNETCHECK ) )
            map.insert( 9, i18n( "USPS Postnet" ) );

        if ( b->type() == "b47" )  // MSI
        {
            map.insert( 10, i18n( "MSI (1 digit)" ) );
            map.insert( 11, i18n( "MSI (2 digits)" ) );
        }

        if ( b->type() == "b46" ) // Plessey
            map.insert( 12, i18n( "Plessey" ) );

        if ( Barkode::hasFeature( b->type(), EAN8CHECK ) )
            map.insert( 13, i18n( "EAN 8" ) );

        if ( Barkode::hasFeature( b->type(), EAN13CHECK ) )
            map.insert( 14, i18n( "EAN 13" ) );

        if ( Barkode::hasFeature( b->type(), UPCACHECK ) )
            map.insert( 15, i18n( "UPC A" ) );

        if ( Barkode::hasFeature( b->type(), UPCECHECK ) )
            map.insert( 16, i18n( "UPC E" ) );

        if ( b->type() == "b16" ) // EAN 128
            map.insert( 17, i18n( "EAN 128" ) );

        if ( Barkode::hasFeature( b->type(),  CODE128CHECK ) )
            map.insert( 18, i18n( "Code 128" ) );

        if ( b->type() == "b70" ) // Royal Mail 4 State
            map.insert( 19, i18n( "Royal Mail 4 State" ) );

        comboCheckSum->clear();

        comboCheckSum->insertItems( 0, map.values() );

        int currentIndex = comboCheckSum->findText( map[options->checksum()] );

        comboCheckSum->setCurrentIndex( currentIndex );

    }
}

void TBarcodeDlg::getData( Barkode* b ) const
{
    TBarcodeOptions* options = dynamic_cast<TBarcodeOptions*>( b->engine()->options() );

    if ( options )
    {
        options->setModuleWidth( spinModule->value() );
        options->setEscape( checkEscape->isChecked() );
        options->setAbove( checkAbove->isChecked() );
        options->setAutocorrect( checkAutoCorrect->isChecked() );
        options->setCheckSum( map.key( comboCheckSum->currentText() ) );
        options->setHeight( spinHeight->value() );
    }
}

PDF417BarcodeDlg::PDF417BarcodeDlg( QWidget *parent )
        : QWidget( parent )
{
    QVBoxLayout* layout = new QVBoxLayout( this );

    setEnabled( Barkode::haveTBarcode() || Barkode::havePDFBarcode() );

    spinRow = new KIntNumInput( this );
    spinRow->setLabel( i18n( "Rows:" ),  Qt::AlignLeft | Qt::AlignTop );
    spinRow->setRange( 0, 90, 1 );
    spinRow->setSliderEnabled( true );
    
    spinCol = new KIntNumInput( this );
    spinCol->setLabel( i18n( "Columns:" ),  Qt::AlignLeft | Qt::AlignTop );
    spinCol->setRange( 0, 30, 1 );
    spinCol->setSliderEnabled( true );
    
    spinErr = new KIntNumInput( this );
    spinErr->setLabel( i18n( "Error correction level:" ),  Qt::AlignLeft | Qt::AlignTop );
    spinErr->setRange( 1, 8, 1 );
    spinCol->setSliderEnabled( true );
    
    layout->addWidget( spinRow );
    layout->addWidget( spinCol );
    layout->addWidget( spinErr );

    setLayout( layout );
}

void PDF417BarcodeDlg::setData( Barkode* b )
{
    PDF417Options* options = dynamic_cast<PDF417Options*>( b->engine()->options() );

    if ( options )
    {
        spinRow->setValue( options->row() );
        spinCol->setValue( options->col() );
        spinErr->setValue( options->err() );
    }
}

void PDF417BarcodeDlg::getData( Barkode* b ) const
{
    PDF417Options* options = dynamic_cast<PDF417Options*>( b->engine()->options() );

    if ( options )
    {
        options->setRow( spinRow->value() );
        options->setCol( spinCol->value() );
        options->setErr( spinErr->value() );
    }
}

DataMatrixDlg::DataMatrixDlg( QWidget *parent )
        : QWidget( parent )
{
    QHBoxLayout* datamLayout = new QHBoxLayout( this );

    comboDataMatrix = new KComboBox( false, this );

    datamLayout->addWidget( new QLabel( i18n( "Data Matrix symbol sizes (rows x cols):" ), this ) );
    datamLayout->addWidget( comboDataMatrix );
    datamLayout->addItem( new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Minimum ) );

    comboDataMatrix->addItem( i18n( "Automatic calculation" ) );
    comboDataMatrix->addItem( "10 x 10" );
    comboDataMatrix->addItem( "12 x 12" );
    comboDataMatrix->addItem( "14 x 14" );
    comboDataMatrix->addItem( "16 x 16" );
    comboDataMatrix->addItem( "18 x 18" );
    comboDataMatrix->addItem( "20 x 20" );
    comboDataMatrix->addItem( "22 x 22" );
    comboDataMatrix->addItem( "24 x 24" );
    comboDataMatrix->addItem( "26 x 26" );
    comboDataMatrix->addItem( "32 x 32" );
    comboDataMatrix->addItem( "36 x 36" );
    comboDataMatrix->addItem( "40 x 40" );
    comboDataMatrix->addItem( "44 x 44" );
    comboDataMatrix->addItem( "48 x 48" );
    comboDataMatrix->addItem( "52 x 52" );
    comboDataMatrix->addItem( "64 x 64" );
    comboDataMatrix->addItem( "72 x 72" );
    comboDataMatrix->addItem( "80 x 80" );
    comboDataMatrix->addItem( "88 x 88" );
    comboDataMatrix->addItem( "96 x 96" );
    comboDataMatrix->addItem( "104 x 104" );
    comboDataMatrix->addItem( "120 x 120" );
    comboDataMatrix->addItem( "132 x 132" );
    comboDataMatrix->addItem( "144 x 144" );
    comboDataMatrix->addItem( "8 x 18" );
    comboDataMatrix->addItem( "8 x 32" );
    comboDataMatrix->addItem( "12 x 26" );
    comboDataMatrix->addItem( "12 x 36" );
    comboDataMatrix->addItem( "16 x 36" );
    comboDataMatrix->addItem( "16 x 48" );

    setLayout( datamLayout );
}

void DataMatrixDlg::setData( Barkode* b )
{
    comboDataMatrix->setCurrentIndex( b->datamatrixSize() );
}

void DataMatrixDlg::getData( Barkode* b ) const
{
    b->setDatamatrixSize( comboDataMatrix->currentIndex() );
}

SequenceDlg::SequenceDlg( QWidget *parent )
        : QWidget( parent )
{
    QVBoxLayout* layout = new QVBoxLayout( this );

    checkSequence = new QCheckBox( i18n( "&Enable sequence" ), this );

    radioNumbers = new QRadioButton( i18n( "Iterate over numbers 0-9" ), this );
    radioAlpha = new QRadioButton( i18n( "Iterate over characters A-Z" ), this );
    radioAlphaNum = new QRadioButton( i18n( "Iterate over A-Z, 0-9" ), this );

    spinStep = new KIntNumInput( this );
    spinStep->setLabel( i18n( "Step:" ), Qt::AlignLeft | Qt::AlignTop );
    spinStep->setRange( -100, 100, 1 );
    spinStep->setSliderEnabled( false );
    
    spinStart = new KIntNumInput( 1, this );
    spinStart->setLabel( i18n( "Start:" ), Qt::AlignLeft | Qt::AlignTop );
    spinStart->setRange( -100000, 100000, 1 );
    spinStart->setSliderEnabled( false );
    
    layout->addWidget( checkSequence );
    layout->addWidget( radioNumbers );
    layout->addWidget( radioAlpha );
    layout->addWidget( radioAlphaNum );
    layout->addWidget( spinStep );
    layout->addWidget( spinStart );

    setLayout( layout );

    connect( checkSequence, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( radioNumbers, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( radioAlpha, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( radioAlphaNum, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
}

void SequenceDlg::setData( Barkode* b )
{
    checkSequence->setChecked( b->sequenceEnabled() );
    spinStep->setValue( b->sequenceStep() );
    spinStart->setValue( b->sequenceStart() );

    if ( b->sequenceMode() == NUM )
        radioNumbers->setChecked( true );
    else if ( b->sequenceMode() == ALPHA )
        radioAlpha->setChecked( true );
    else if ( b->sequenceMode() == ALPHANUM )
        radioAlphaNum->setChecked( true );

    enableControls();
}

void SequenceDlg::getData( Barkode* b ) const
{
    b->setSequenceEnabled( checkSequence->isChecked() );

    b->setSequenceStep( spinStep->value() );
    b->setSequenceStart( spinStart->value() );

    if ( radioNumbers->isChecked() )
        b->setSequenceMode( NUM );
    else if ( radioAlpha->isChecked() )
        b->setSequenceMode( ALPHA );
    else if ( radioAlphaNum->isChecked() )
        b->setSequenceMode( ALPHANUM );
}

void SequenceDlg::enableControls()
{
    radioNumbers->setEnabled( checkSequence->isChecked() );
    radioAlpha->setEnabled( checkSequence->isChecked() );
    // FIXME: enable/fix or kill this button
    radioAlphaNum->setEnabled( false ); //checkSequence->isChecked() );
    spinStep->setEnabled( checkSequence->isChecked() );
    spinStart->setEnabled( checkSequence->isChecked() && radioNumbers->isChecked() );
}

ColorDlg::ColorDlg( QWidget *parent )
        : QWidget( parent )
{
    QGridLayout* layout = new QGridLayout( this );

    buttonBarColor = new KColorButton( this );
    buttonBackColor = new KColorButton( this );
    buttonTextColor = new KColorButton( this );

    layout->addWidget( new QLabel( i18n( "Bar Color:" ), this ), 0, 0 );
    layout->addWidget( new QLabel( i18n( "Background Color:" ), this ), 1, 0 );
    layout->addWidget( new QLabel( i18n( "Text Color:" ), this ), 2, 0 );
    layout->addWidget( buttonBarColor, 0, 1 );
    layout->addWidget( buttonBackColor, 1, 1 );
    layout->addWidget( buttonTextColor, 2, 1 );

    setLayout( layout );
}

void ColorDlg::setData( Barkode* b )
{
    buttonBarColor->setColor( b->foreground() );
    buttonBackColor->setColor( b->background() );
    buttonTextColor->setColor( b->textColor() );
}

void ColorDlg::getData( Barkode* b ) const
{
    b->setForeground( buttonBarColor->color() );
    b->setBackground( buttonBackColor->color() );
    b->setTextColor( buttonTextColor->color() );
}

PurePostscriptDlg::PurePostscriptDlg( QWidget *parent )
        : QWidget( parent )
{
    QVBoxLayout* layout = new QVBoxLayout( this );

    setEnabled( Barkode::havePurePostscriptBarcode() );
    checkChecksum = new QCheckBox( i18n( "Enable &Checksum" ), this );

    layout->addWidget( checkChecksum );

    setLayout( layout );
}

void PurePostscriptDlg::setData( Barkode* b )
{
    PurePostscriptOptions* options = dynamic_cast<PurePostscriptOptions*>( b->engine()->options() );

    if ( options )
    {
        checkChecksum->setChecked( options->checksum() );
        checkChecksum->setEnabled( Barkode::hasFeature( b->type(), MODULOALLCHECK ) );
    }
}

void PurePostscriptDlg::getData( Barkode* b ) const
{
    PurePostscriptOptions* options = dynamic_cast<PurePostscriptOptions*>( b->engine()->options() );

    if ( options )
        options->setChecksum( checkChecksum->isChecked() );
}


#include "barcodedialogs.moc"
