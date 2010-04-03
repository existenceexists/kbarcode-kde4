/***************************************************************************
                          barcodecombo.cpp  -  description
                             -------------------
    begin                : Son Apr 13 2003
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

#include "barcodecombo.h"
#include "barcodedialogs.h"
#include "barkode.h"
#include "tokendialog.h"

#include <pcre.h>

// Qt includes
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qregexp.h>
#include <qstring.h>
//Added by qt3to4:
#include <QGridLayout>
#if QT_VERSION >= 0x030100
    #include <ktextedit.h>
#else
    #include <q3textedit.h>
#endif

// KDE includes
#include <kiconloader.h>
#include <knuminput.h>
#include <klineedit.h>
#include <klocale.h>
#include <kpushbutton.h>

BarcodeValidator::BarcodeValidator( QObject* parent )
    : QValidator( parent )
{
    m_valid    = NULL;
    m_notValid = NULL;
}

bool BarcodeValidator::pcreValidate( QString* pattern, const QString & input ) const
{
    const char* error;
    const int   ovector_size = 12;
    int         erroffset;
    pcre*       compiled;
    int         ovector[ovector_size];
    int         result;

    if( !pattern || input.isEmpty() )
        return true;

    if( pattern->isEmpty() )
        return true;

    compiled = pcre_compile( pattern->toLatin1().data(), 0, &error, &erroffset, NULL );
    if( !compiled ) // ignore all errors
        return true; 

     result = pcre_exec( compiled, NULL, input.toLatin1(), input.length(), 0, 0, ovector, ovector_size );

     return (result >= 1);
}

QValidator::State BarcodeValidator::validate( QString & input, int & pos ) const
{
    if( (!m_valid && !m_notValid) || input.isEmpty() )
        return Acceptable;

    if( (m_valid && m_valid->isEmpty()) && (m_notValid && m_notValid->isEmpty()) )
        return Acceptable;

    if( pcreValidate( m_valid, input ) && !pcreValidate( m_notValid, input ) )
        return Acceptable;
    else
        return Intermediate;

    return Acceptable;
}

BarcodeCombo::BarcodeCombo(QWidget *parent)
    : KComboBox( false, parent )

{
    this->insertStringList( *Barkode::encodingTypes() );
}

BarcodeCombo::~BarcodeCombo()
{
}

const QString & BarcodeCombo::getEncodingType()
{
    return Barkode::typeFromName( currentText() );
}

void BarcodeCombo::setEncodingType( const QString & type )
{
    const QString name = Barkode::nameFromType( type );
    setCurrentItem( name, false );
}

BarcodeWidget::BarcodeWidget(QWidget *parent)
    : QWidget( parent ), m_validator( this )
{
    m_token = NULL;

    QGridLayout* grid = new QGridLayout( this, 6, 6 );

    labelStandard = new QLabel( i18n( "&Encoding Type:" ), this );
    grid->addWidget( labelStandard, 1, 0 );

    comboStandard = new BarcodeCombo( this );
    connect( comboStandard, SIGNAL( activated(int) ), this, SLOT( encodingChanged() ) );
    connect( comboStandard, SIGNAL( activated(int) ), this, SLOT( changed() ) );
    grid->addMultiCellWidget( comboStandard, 1, 1, 1, 3 );
    labelStandard->setBuddy( comboStandard );
    
    labelData = new QLabel( i18n( "&Value:" ), this );
    grid->addWidget( labelData, 2, 0 );

    data = new KLineEdit( this );

    labelData->setBuddy( data );
    connect( data, SIGNAL( textChanged( const QString & ) ), this, SLOT( changed() ) );
    connect( data, SIGNAL( textChanged( const QString & ) ), this, SLOT( slotValidateValue() ) );
    grid->addMultiCellWidget( data, 2, 2, 1, 3 );

#if QT_VERSION >= 0x030100
        multi = new KTextEdit( this );
#else
        multi = new QTextEdit( this );
#endif
    multi->setTextFormat( Qt::PlainText );
    multi->setWordWrapMode( QTextOption::NoWrap );
    multi->setEnabled( false );
    multi->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    multi->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    multi->hide();
    connect( multi, SIGNAL( textChanged() ), this, SLOT( changed() ) );    
    grid->addMultiCellWidget( multi, 3, 3, 1, 3 );
    
    checkText = new QCheckBox( this, "checkText" );
    checkText->setText( i18n( "&Display text" ) );
    checkText->setChecked( true );
    connect( checkText, SIGNAL( clicked() ), this, SLOT( changed() ) );
    grid->addWidget( checkText, 4, 1 );

    buttonAdvanced = new KPushButton( i18n("&Advanced..."), this );
//    buttonAdvanced->setEnabled( false );
    grid->addWidget( buttonAdvanced, 4, 2 );

    buttonToken = new KPushButton( i18n("&Insert Data Field..."), this );
    buttonToken->setIconSet( QIcon( SmallIcon("contents") ) );
    grid->addWidget( buttonToken, 4, 3 );
    
    spinMargin = new KIntNumInput( this );
    spinMargin->setLabel( i18n( "&Margin:" ), Qt::AlignLeft | Qt::AlignVCenter );
    spinMargin->setRange( 0, 10000, 1, false );
    spinMargin->setValue( 10 );
    connect( spinMargin, SIGNAL( valueChanged(int) ), this, SLOT( changed() ) );
    grid->addMultiCellWidget( spinMargin, 5, 5, 0, 1 );

    spinRotation = new KIntNumInput( this );
    spinRotation->setLabel( i18n( "&Rotation:" ), Qt::AlignLeft | Qt::AlignVCenter );
    spinRotation->setRange( 0, 360, 90, false );
    spinRotation->setValue( 0 );
    connect( spinRotation, SIGNAL( valueChanged(int) ), this, SLOT( changed() ) );
    grid->addMultiCellWidget( spinRotation, 5, 5, 2, 3 );

    spinScale = new KIntNumInput( spinMargin, 1000, this );
    spinScale->setLabel( i18n("&Scale (in permille):"), Qt::AlignLeft | Qt::AlignVCenter );
    spinScale->setRange( 100, 10000, 100, false );
    spinScale->setValue( 1000 );
    connect( spinScale, SIGNAL( valueChanged(int) ), this, SLOT( changed() ) );
    grid->addMultiCellWidget( spinScale, 6, 6, 0, 1 );

    spinCut = new KIntNumInput( spinRotation, 100, this );
    spinCut->setLabel( i18n("&Crop:"), Qt::AlignLeft | Qt::AlignVCenter );
    spinCut->setRange( 1, 100, 1, false );
    connect( spinCut, SIGNAL( valueChanged(int) ), this, SLOT( changed() ) );
    grid->addMultiCellWidget( spinCut, 6, 6, 2, 3 );

    connect( buttonAdvanced, SIGNAL( clicked() ), this, SLOT( advanced() ) );
    connect( buttonToken, SIGNAL( clicked() ), this, SLOT( tokens() ) );

    m_enabledata = true;
    m_multi = false;
    encodingChanged();
}

void BarcodeWidget::getData( Barkode & barcode )
{
    // make sure all changes are applied to the barcode
    // even if not all widgets have emitted their changed SIGNAL yet
    this->changed();

    barcode = m_barcode;
}

void BarcodeWidget::setData( const Barkode & b )
{
    comboStandard->setEncodingType( b.type() );
    encodingChanged();
    if( !m_multi )
        data->setText( b.value() );
    else
        multi->setText( b.value() );

    checkText->setChecked( b.textVisible() );

    spinMargin->setValue( b.quietZone() );
    spinRotation->setValue( b.rotation() );
    spinCut->setValue( int(b.cut()*100) );
    spinScale->setValue( int(b.scaling()*1000) );

    m_barcode = b;
}

void BarcodeWidget::setStandardEnabled( bool b )
{
    labelStandard->setEnabled( b );
    comboStandard->setEnabled( b );
}

void BarcodeWidget::setDataEnabled( bool b )
{
    labelData->setEnabled( b );
    m_enabledata = b;
    encodingChanged();
}

void BarcodeWidget::defaults()
{
    Barkode b; // get's automatically initialized with default values
    setData( b );
}

void BarcodeWidget::encodingChanged()
{
    QString* validator;
    QString* validatorNot;

    spinCut->setEnabled( Barkode::hasFeature( comboStandard->getEncodingType(), NOCUT ) ? false : true );
    if( !spinCut->isEnabled() )
        spinCut->setValue( 100 ); // TODO: Don't hardcode
    
    spinScale->setEnabled( Barkode::hasFeature( comboStandard->getEncodingType(), NOSCALE ) ? false : true );
    if( !spinScale->isEnabled() )
        spinScale->setValue( 1000 ); // TODO: Don't hardcode
        
    checkText->setEnabled( Barkode::hasFeature( comboStandard->getEncodingType(), NOTEXT ) ? false : true );

    if( Barkode::hasFeature( comboStandard->getEncodingType(), MULTILINE ) ) {
	buttonToken->setEnabled( m_enabledata );
        multi->setEnabled( m_enabledata );
        multi->show();
        multi->setFocus();
        data->setEnabled( false );
        m_multi = true;
    } else {
	buttonToken->setEnabled( m_enabledata );
        data->setEnabled( m_enabledata );
        multi->hide();
        data->setFocus();
        multi->setEnabled( false );
        m_multi = false;
    }

    validator    = Barkode::validatorFromType( comboStandard->getEncodingType() );
    validatorNot = Barkode::validatorNotFromType( comboStandard->getEncodingType() );
    if( validator || validatorNot )
    {
        m_validator.setRegExp( validator, validatorNot );
        data->setValidator( &m_validator );
    }
    else 
        data->setValidator( NULL );

    slotValidateValue();
}

void BarcodeWidget::advanced()
{
    AdvancedBarcodeDialog abd( comboStandard->getEncodingType(), this );
    abd.setData( &m_barcode );
    if( abd.exec() == QDialog::Accepted )
        abd.getData( &m_barcode );
}

void BarcodeWidget::tokens()
{
    TokenDialog tokendlg( m_token, this );
    if( tokendlg.exec() == QDialog::Accepted )
    {
	if( data->isEnabled() )
	    data->insert( tokendlg.token() );
	else
	    multi->insert( tokendlg.token() );
    }
}

void BarcodeWidget::slotValidateValue()
{
    QColor c = data->hasAcceptableInput() ? this->foregroundColor() : Qt::red;

    QPalette palette;
    palette.setColor(data->foregroundRole(), c);
    data->setPalette(palette);    
}

void BarcodeWidget::changed() 
{
    if( !m_multi )
        m_barcode.setValue( data->text() );
    else
        m_barcode.setValue( multi->toPlainText() );

    m_barcode.setType( comboStandard->getEncodingType() );
    m_barcode.setTextVisible( checkText->isChecked() );

    m_barcode.setQuietZone( spinMargin->value() );
    m_barcode.setRotation( spinRotation->value() );
    m_barcode.setCut( (double)spinCut->value()/100.0  );
    m_barcode.setScaling( (double)spinScale->value()/1000.0 );
}

#include "barcodecombo.moc"
