/***************************************************************************
                      definitiondialog.cpp  -  description
                             -------------------
    begin                : Don Apr 18 12:34:56 CEST 2002
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

#include "definitiondialog.h"
#include "printersettings.h"
#include "labelutils.h"
#include "definition.h"

#include <kcombobox.h>
#include <kpushbutton.h>
#include <krestrictedline.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
//Added by qt3to4:
#include <QHBoxLayout>
#include <QPaintEvent>
#include <QPixmap>
#include <QVBoxLayout>

// KDE includes
#include <klocale.h>

LabelPreview::LabelPreview( QWidget* parent )
    : QWidget( parent )
{
    // For old DIN A4 preview
    PrinterSettings* ps = PrinterSettings::getInstance();
    setFixedSize( QSize(  ((int)ps->pageWidth() + 20)/2, ((int)ps->pageHeight() + 20)/2 ) );
    m_prv_enabled = true;
}

LabelPreview::~LabelPreview()
{ }

void LabelPreview::paintEvent( QPaintEvent* )
{
    QPainter p( this );
    p.fillRect( 0, 0, width(), height(), QBrush( Qt::white ) );
    p.setPen( QPen( Qt::black ) );
    p.translate( 10, 10 );

    p.fillRect( 0, 0, 210/2, 297/2, QBrush( Qt::gray ) );
    p.drawRect( 0, 0, 210/2, 297/2 );

    if(!m_prv_enabled)
        return;
        
//    int numv = measure.num_v > 0 ? measure.num_v : 1;
    for( int v = 0; v < measure.numV(); v++ ) {
        for( int h = 0; h < measure.numH(); h++ ) {
            if( !v && !h )
                p.setPen( QPen( Qt::red ) );
            else
                p.setPen( QPen( Qt::black ) );

            p.fillRect( int(measure.gapLeftMM() + (measure.gapHMM() * h)) / 2,
                        int(measure.gapTopMM() + (measure.gapVMM() * v)) / 2 ,
                        (int)measure.widthMM() / 2 , (int)measure.heightMM() / 2, QBrush( Qt::white ) );

            p.drawRect( int(measure.gapLeftMM() + (measure.gapHMM() * h)) / 2,
                        int(measure.gapTopMM() + (measure.gapVMM() * v)) / 2,
                        (int)measure.widthMM() / 2, (int)measure.heightMM() / 2 );
        }
    }
}

/*****************************************/
DefinitionDialog::DefinitionDialog( QWidget* parent, bool modal, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    setModal( modal );
    resize( 465, 345 );
    setWindowTitle( i18n( "Add Label Definition" ) );
    QHBoxLayout* layout = new QHBoxLayout( this );
    layout->setContentsMargins( 6, 6, 6, 6 );
    layout->setSpacing( 6 );
    DefinitionDialogLayout = new QVBoxLayout();
    DefinitionDialogLayout->setObjectName( "DefinitionDialogLayout" );
    DefinitionDialogLayout->setContentsMargins( 11, 11, 11, 11 );
    DefinitionDialogLayout->setSpacing( 6 );

    Layout17 = new QHBoxLayout();
    Layout17->setObjectName( "Layout17" );
    Layout17->setContentsMargins( 0, 0, 0, 0 );
    Layout17->setSpacing( 6 );

    Layout13 = new QVBoxLayout();
    Layout13->setObjectName( "Layout13" );
    Layout13->setContentsMargins( 0, 0, 0, 0 );
    Layout13->setSpacing( 6 );

    TextLabel1 = new QLabel( this );
    TextLabel1->setObjectName( "TextLabel1" );
    TextLabel1->setText( i18n( "Producer:" ) );
    Layout13->addWidget( TextLabel1 );

    TextLabel2 = new QLabel( this );
    TextLabel2->setObjectName( "TextLabel2" );
    TextLabel2->setText( i18n( "Type:" ) );
    Layout13->addWidget( TextLabel2 );

    TextLabel3 = new QLabel( this );
    TextLabel3->setObjectName( "TextLabel3" );
    TextLabel3->setText( QString( i18n( "Width (in %1):" ) ).arg( Measurements::system() ) );
    Layout13->addWidget( TextLabel3 );

    TextLabel4 = new QLabel( this );
    TextLabel4->setObjectName( "TextLabel4" );
    TextLabel4->setText( QString( i18n( "Height (in %1):" ) ).arg( Measurements::system() ) );
    Layout13->addWidget( TextLabel4 );

    TextLabel5 = new QLabel( this );
    TextLabel5->setObjectName( "TextLabel5" );
    TextLabel5->setText( i18n( "Horizontal Gap:" ) );
    Layout13->addWidget( TextLabel5 );

    TextLabel6 = new QLabel( this );
    TextLabel6->setObjectName( "TextLabel6" );
    TextLabel6->setText( i18n( "Vertical Gap:" ) );
    Layout13->addWidget( TextLabel6 );

    TextLabel7 = new QLabel( this );
    TextLabel7->setObjectName( "TextLabel7" );
    TextLabel7->setText( i18n( "Top Gap:" ) );
    Layout13->addWidget( TextLabel7 );

    TextLabel8 = new QLabel( this );
    TextLabel8->setObjectName( "TextLabel8" );
    TextLabel8->setText( i18n( "Left Gap:" ) );
    Layout13->addWidget( TextLabel8 );

    TextLabel9 = new QLabel( this );
    TextLabel9->setObjectName( "TextLabel9" );
    TextLabel9->setText( i18n("Number Horizontal:") );
    Layout13->addWidget( TextLabel9 );

    TextLabel10 = new QLabel( this );
    TextLabel10->setObjectName( "TextLabel10" );
    TextLabel10->setText( i18n("Number Vertical:") );
    Layout13->addWidget( TextLabel10 );
     
    Layout17->addLayout( Layout13 );
    Layout14 = new QVBoxLayout(); 
    Layout14->setObjectName( "Layout14" );
    Layout14->setContentsMargins( 0, 0, 0, 0 );
    Layout14->setSpacing( 6 );

    comboProducer = new KComboBox( FALSE, this );
    comboProducer->setObjectName( "comboProducer" );
    comboProducer->setEditable( TRUE );
    Layout14->addWidget( comboProducer );

    comboType = new KComboBox( FALSE, this );
    comboType->setObjectName( "type" );
    comboType->setEditable( TRUE );
    Layout14->addWidget( comboType );

    editWidth = new KRestrictedLine( this );
    editWidth->setText( i18n( "0" ) );
    editWidth->setValidChars( i18n( "0123456789." ) );
    Layout14->addWidget( editWidth );

    editHeight = new KRestrictedLine( this );
    editHeight->setText( i18n( "0" ) );
    editHeight->setValidChars( i18n( "0123456789." ) );
    Layout14->addWidget( editHeight );

    editHGap = new KRestrictedLine( this );
    editHGap->setText( i18n( "0" ) );
    editHGap->setValidChars( i18n( "0123456789." ) );
    Layout14->addWidget( editHGap );

    editVGap = new KRestrictedLine( this );
    editVGap->setText( i18n( "0" ) );
    editVGap->setValidChars( i18n( "0123456789." ) );
    Layout14->addWidget( editVGap );

    editTGap = new KRestrictedLine( this );
    editTGap->setText( i18n( "0" ) );
    editTGap->setValidChars( i18n( "0123456789." ) );
    Layout14->addWidget( editTGap );

    editLGap = new KRestrictedLine( this );
    editLGap->setText( i18n( "0" ) );
    editLGap->setValidChars( i18n( "0123456789." ) );
    Layout14->addWidget( editLGap );

    editNumH = new KRestrictedLine( this );
    editNumH->setText( i18n( "0" ) );
    editNumH->setValidChars( "0123456789" );
    Layout14->addWidget( editNumH );
    
    editNumV = new KRestrictedLine( this );
    editNumV->setText( i18n( "0" ) );
    editNumV->setValidChars( "0123456789" );
    Layout14->addWidget( editNumV );

    Layout17->addLayout( Layout14 );
    QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout17->addItem( spacer );
    DefinitionDialogLayout->addLayout( Layout17 );
    QSpacerItem* spacer_2 = new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
    DefinitionDialogLayout->addItem( spacer_2 );

    Layout16 = new QHBoxLayout();
    Layout16->setObjectName( "Layout16" );
    Layout16->setContentsMargins( 0, 0, 0, 0 );
    Layout16->setSpacing( 6 );
    buttonInfo = new KPushButton( i18n("More &Information"), this );
    Layout16->addWidget( buttonInfo );

    QSpacerItem* spacer_3 = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout16->addItem( spacer_3 );

    buttonAdd = new KPushButton( this );
    buttonAdd->setText( i18n( "&Add" ) );
    Layout16->addWidget( buttonAdd );

    buttonCancel = new KPushButton( this );
    buttonCancel->setText( i18n( "&Cancel" ) );
    Layout16->addWidget( buttonCancel );
    DefinitionDialogLayout->addLayout( Layout16 );

    preview = new LabelPreview( this );
    layout->addLayout( DefinitionDialogLayout );
    layout->addWidget( preview );

    l = new QLabel( this );
    setExtension( l );
    setOrientation( Qt::Vertical );

    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( buttonAdd, SIGNAL( clicked() ), this, SLOT( add() ) );

    connect( editWidth, SIGNAL( textChanged(const QString &) ), this, SLOT( updatePreview() ) );
    connect( editHeight, SIGNAL( textChanged(const QString &) ), this, SLOT( updatePreview() ) );
    connect( editHGap, SIGNAL( textChanged(const QString &) ), this, SLOT( updatePreview() ) );
    connect( editVGap, SIGNAL( textChanged(const QString &) ), this, SLOT( updatePreview() ) );
    connect( editTGap, SIGNAL( textChanged(const QString &) ), this, SLOT( updatePreview() ) );
    connect( editLGap, SIGNAL( textChanged(const QString &) ), this, SLOT( updatePreview() ) );
    connect( editNumH, SIGNAL( textChanged(const QString &) ), this, SLOT( updatePreview() ) );
    connect( editNumV, SIGNAL( textChanged(const QString &) ), this, SLOT( updatePreview() ) );

    comboProducer->setInsertPolicy( QComboBox::NoInsert );
    comboType->setInsertPolicy( QComboBox::NoInsert );

    comboProducer->clear();
    QStringList list = Definition::getProducers();
    comboProducer->addItems( list );

    types = new QStringList[list.count()];
    for( int i = 0; i < list.count(); i++ )
        types[i] = Definition::getTypes( list[i] );

    connect( comboProducer, SIGNAL( textChanged(const QString &) ), this, SLOT( updateType() ) );
    connect( comboType, SIGNAL( textChanged(const QString &) ), this, SLOT( updateText() ) );
    connect( buttonInfo, SIGNAL( clicked() ), this, SLOT( toggleExtension() ) );

    drawGraphic();
    updateType();

    showExtension( false );
}

DefinitionDialog::~DefinitionDialog()
{
    delete [] types;
}
                                            
void DefinitionDialog::add()
{
    Definition::write( getCurrentMeasure(), comboType->currentText(), comboProducer->currentText() );

    accept();
}

void DefinitionDialog::updateType()
{
    comboType->clear();
    if( comboProducer->currentText() == comboProducer->itemText( comboProducer->currentIndex() ) )
        if( comboProducer->currentIndex() < comboProducer->count())
            comboType->addItems( types[ comboProducer->currentIndex() ] );
    comboType->addItem( i18n( "Custom" ), 0 );
}

const Measurements DefinitionDialog::getCurrentMeasure()
{
    Measurements m;
    m.setGapLeft( editLGap->text().toDouble() );
    m.setGapTop( editTGap->text().toDouble() );
    m.setWidth( editWidth->text().toDouble() );
    m.setHeight( editHeight->text().toDouble() );
    m.setGapV( editVGap->text().toDouble() );
    m.setGapH( editHGap->text().toDouble() );
    m.setNumH( editNumH->text().toInt() );
    m.setNumV( editNumV->text().toInt() );

    return m;
}

void DefinitionDialog::updateText()
{
    Definition   d( comboProducer->currentText(), comboType->currentText() );
    Measurements m = d.getMeasurements();

    editTGap->setText( I2S( m.gapTop() ) );
    editLGap->setText( I2S( m.gapLeft() ) );
    editWidth->setText( I2S( m.width() ) );
    editHeight->setText( I2S( m.height() ) );
    editVGap->setText( I2S( m.gapV() ) );
    editHGap->setText( I2S( m.gapH() ) );
    editNumH->setText( I2S( m.numH() ) );
    editNumV->setText( I2S( m.numV() ) );

    updatePreview();
}

void DefinitionDialog::updatePreview()
{
    Measurements m = getCurrentMeasure();

    QPalette paletteEditHeight;
    paletteEditHeight.setColor( editHeight->foregroundRole(), Qt::black );
    editHeight->setPalette( paletteEditHeight );
    
    QPalette paletteEditWidth;
    paletteEditWidth.setColor( editWidth->foregroundRole(), Qt::black );
    editWidth->setPalette( paletteEditWidth );
    
    QPalette paletteEditVGap;
    paletteEditVGap.setColor( editVGap->foregroundRole(), Qt::black );
    editVGap->setPalette( paletteEditVGap );
    
    QPalette paletteEditHGap;
    paletteEditHGap.setColor( editHGap->foregroundRole(), Qt::black );
    editHGap->setPalette( paletteEditHGap );
    
    QPalette paletteEditTGap;
    paletteEditTGap.setColor( editTGap->foregroundRole(), Qt::black );
    editTGap->setPalette( paletteEditTGap );
    
    QPalette paletteEditLGap;
    paletteEditLGap.setColor( editLGap->foregroundRole(), Qt::black );
    editLGap->setPalette( paletteEditLGap );
    
    QPalette paletteEditNumH;
    paletteEditNumH.setColor( editNumH->foregroundRole(), Qt::black );
    editNumH->setPalette( paletteEditNumH );
    
    QPalette paletteEditNumV;
    paletteEditNumV.setColor( editNumV->foregroundRole(), Qt::black );
    editNumV->setPalette( paletteEditNumV );
    
    // Mark errors in Red
    if( m.heightMM() > PrinterSettings::getInstance()->pageHeight() ) {
        paletteEditHeight.setColor( editHeight->foregroundRole(), Qt::red );
        editHeight->setPalette( paletteEditHeight );
    }

    if( m.widthMM() > PrinterSettings::getInstance()->pageWidth() ) {
        paletteEditWidth.setColor( editWidth->foregroundRole(), Qt::red );
        editWidth->setPalette( paletteEditWidth );
    }

    if( m.gapVMM() < m.heightMM() ) {
        paletteEditVGap.setColor( editVGap->foregroundRole(), Qt::red );
        editVGap->setPalette( paletteEditVGap );
    }

    if( m.gapHMM() < m.widthMM() ) {
        paletteEditHGap.setColor( editHGap->foregroundRole(), Qt::red );
        editHGap->setPalette( paletteEditHGap );
    }

    if( m.gapTopMM() >  PrinterSettings::getInstance()->pageHeight() - m.heightMM() ) {
        paletteEditTGap.setColor( editTGap->foregroundRole(), Qt::red );
        editTGap->setPalette( paletteEditTGap );
    }

    if( m.gapLeftMM() > PrinterSettings::getInstance()->pageWidth() - m.widthMM() ) {
        paletteEditLGap.setColor( editLGap->foregroundRole(), Qt::red );
        editLGap->setPalette( paletteEditLGap );
    }

    if( m.gapLeftMM() + m.numH() * m.gapHMM() > PrinterSettings::getInstance()->pageWidth() ) {
        paletteEditNumH.setColor( editNumH->foregroundRole(), Qt::red );
        editNumH->setPalette( paletteEditNumH );
    }

    if( m.gapTopMM() + m.numV() * m.gapVMM() > PrinterSettings::getInstance()->pageHeight() ) {
        paletteEditNumV.setColor( editNumV->foregroundRole(), Qt::red );
        editNumV->setPalette( paletteEditNumV );
    }

    preview->setRect( QRect( (int)m.gapLeftMM(), (int)m.gapTopMM(), (int)m.widthMM(), (int)m.heightMM() ) );
    preview->setMeasurements( m );
    preview->repaint();
}

void DefinitionDialog::drawGraphic()
{
    QMatrix wm;
    wm.rotate( 90 );

    QPixmap pic( 450, 330 );
    pic.fill( Qt::gray );

    QPainter p( &pic );
    p.setPen( Qt::black );

    p.fillRect( 60, 60, 450, 330, QColor( 255, 254, 217 ) );
    p.drawRect( 60, 60, 450, 330 );
    p.setBrush( Qt::white );

    p.setPen( Qt::DotLine );
    p.drawRoundRect( 90, 90, 150, 90 );
    p.drawRoundRect( 270, 90, 150, 90 );

    p.drawRoundRect( 90, 210, 150, 90 );
    p.drawRoundRect( 270, 210, 150, 90 );

    // Draw width
    p.setPen( QPen( Qt::red, 2 ) );
    p.drawLine( 90, 150, 240, 150 );

    // Draw numh
    p.drawLine( 90, 270, 450, 270 );

    // Draw Gap Top
    p.setPen( QPen( Qt::red, 2, Qt::DotLine ) );
    p.drawLine( 0, 60, 90, 60 );
    p.drawLine( 0, 90, 90, 90 );

    // draw Gap V
    p.drawLine( 0, 210, 90, 210 );

    // Draw height
    p.setPen( QPen( Qt::green, 2 ) );
    p.drawLine( 150, 90, 150, 180 );


    // Draw numv
    p.drawLine( 330, 90, 330, 330 );

    // Draw Gap Left
    p.setPen( QPen( Qt::green, 2, Qt::DotLine ) );
    p.drawLine( 60, 0, 60, 90 );
    p.drawLine( 90, 0, 90, 90 );

    // draw Gap H
    p.drawLine( 270, 0, 270, 90 );

    // draw Texts:
    p.setPen( Qt::black );
    p.drawText( 100, 140, i18n("Width") );
    p.drawText( 160, 120, i18n("Height") );
    p.drawText( 100, 260, i18n("Number of horizontal Labels") );

    QPixmap* pix = LabelUtils::drawString( i18n("Number of vertical Labels") );
    p.drawPixmap( 340, 110, pix->transformed( wm ) );
    delete pix;

    p.drawText( 5, 80, i18n("Gap Top") );

    pix = LabelUtils::drawString( i18n("Gap Left") );

    p.drawPixmap( 60, 5, pix->transformed( wm ) );
    delete pix;

    p.drawText( 5, 150, i18n("Vertical Gap") );
    p.drawText( 150, 50, i18n("Horizontal Gap") );

    p.end();

    l->setPixmap( pic );
}

void DefinitionDialog::toggleExtension()
{
    if( l->isVisible() ) {
        showExtension( false );
    } else
        showExtension( true );
}


#include "definitiondialog.moc"
