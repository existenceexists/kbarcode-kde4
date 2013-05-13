/***************************************************************************
                          newlabel.cpp  -  description
                             -------------------
    begin                : Son Mai 5 2002
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

#include "newlabel.h"
#include "definitiondialog.h"
#include "sqltables.h"

// Qt includes
#include <qcheckbox.h>
#include <QGroupBox>
#include <qlabel.h>
#include <qlayout.h>
#include <qsqlquery.h>
//Added by qt3to4:
#include <QHBoxLayout>
#include <QGridLayout>
#include <QVBoxLayout>

// KDE includes
#include <klocale.h>
#include <kcombobox.h>
#include <kpushbutton.h>

#ifndef I2S
    #define I2S(x) QString("%1").arg(x)
#endif // I2S

NewLabel::NewLabel( QWidget* parent, bool change, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    setModal( true );
    setWindowTitle( i18n( "New Label" ) );
    curid = 0;
    types = 0;
    
    NewLabelLayout = new QVBoxLayout( this );
    NewLabelLayout->setObjectName( "NewLabelLayout" );
    NewLabelLayout->setContentsMargins( 11, 11, 11, 11 );
    NewLabelLayout->setSpacing( 6 );

    TextLabel1 = new QLabel( this );
    TextLabel1->setObjectName( "TextLabel1" );
    if( !change )
        TextLabel1->setText( i18n( "<h1>Create a new Label</h1><br><br>" ) );
    else
        TextLabel1->setText( i18n( "<h1>Change Label Size</h1><br><br>" ) );

    NewLabelLayout->addWidget( TextLabel1 );

    QGroupBox* group1 = new QGroupBox( this );
    group1->setTitle( i18n( "Label" ) );
    QVBoxLayout* group1MainLayout = new QVBoxLayout;
    group1->setLayout( group1MainLayout );
    group1->layout()->setSpacing( 6 );
    group1->layout()->setContentsMargins( 11, 11, 11, 11 );
    QGridLayout* group1Layout = new QGridLayout();
    group1Layout->setAlignment( Qt::AlignTop );
    group1MainLayout->addLayout( group1Layout );
    
    TextLabel2 = new QLabel( group1 );
    TextLabel2->setObjectName( "TextLabel2" );
    TextLabel2->setText( i18n( "Producer:" ) );

    TextLabel3 = new QLabel( group1 );
    TextLabel2->setObjectName( "TextLabel3" );
    TextLabel3->setText( i18n( "Type:" ) );

    comboProducer = new KComboBox( FALSE, group1 );
    comboProducer->setObjectName( "comboProducer" );
    comboType = new KComboBox( FALSE, group1 );
    comboType->setObjectName( "comboType" );

    /*checkEmpty = new QCheckBox( i18n("&Start with an empty label"), group1 );
    checkEmpty->setEnabled( !change );*/
    
    /*group1Layout->addWidget( checkEmpty, 0, 0, 1, 2 );*/
    group1Layout->addWidget( TextLabel2, 1, 0 );
    group1Layout->addWidget( TextLabel3, 2, 0 );
    group1Layout->addWidget( comboProducer, 1, 1 );
    group1Layout->addWidget( comboType, 2, 1 );
    
    NewLabelLayout->addWidget( group1 );

    Layout2 = new QHBoxLayout();
    Layout2->setObjectName( "Layout2" );
    Layout2->setContentsMargins( 0, 0, 0, 0 );
    Layout2->setSpacing( 6 );

    QSpacerItem* spacer_2 = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout2->addItem( spacer_2 );
    NewLabelLayout->addLayout( Layout2 );

    TextLabel4 = new QLabel( this );
    preview = new LabelPreview( this );

    QHBoxLayout* hlayout = new QHBoxLayout();
    hlayout->setContentsMargins( 6, 6, 6, 6 );
    hlayout->setSpacing( 6 );
    hlayout->addWidget( TextLabel4 );
    hlayout->addWidget( preview );
    NewLabelLayout->addLayout( hlayout );
    QSpacerItem* spacer_3 = new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
    NewLabelLayout->addItem( spacer_3 );

    Layout1 = new QHBoxLayout();
    Layout1->setObjectName( "Layout1" );
    Layout1->setContentsMargins( 0, 0, 0, 0 );
    Layout1->setSpacing( 6 );

    buttonOwnFormat = new KPushButton( this );
    buttonOwnFormat->setText( i18n( "&Add own Label Definition" ) );
    Layout1->addWidget( buttonOwnFormat );

    QSpacerItem* spacer_4 = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout1->addItem( spacer_4 );

    buttonOk = new KPushButton( this );
    buttonOk->setText( i18n( "&OK" ) );
    buttonOk->setDefault( true );
    Layout1->addWidget( buttonOk );

    buttonCancel = new KPushButton( this );
    buttonCancel->setText( i18n( "&Cancel" ) );
    Layout1->addWidget( buttonCancel );
    NewLabelLayout->addLayout( Layout1 );

    connect( comboProducer, SIGNAL( activated(int) ), this, SLOT( updateType() ) );
    connect( comboProducer, SIGNAL( activated(int) ), this, SLOT( updateText() ) );
    connect( comboType, SIGNAL( activated(int) ), this, SLOT( updateText() ) );
    /*connect( checkEmpty, SIGNAL( clicked() ), this, SLOT( updateText() ) );*/
    
    connect( buttonOk, SIGNAL( clicked() ), this, SLOT(accept() ) );
    connect( buttonCancel, SIGNAL( clicked() ), this, SLOT(reject() ) );
    connect( buttonOwnFormat, SIGNAL( clicked() ), this, SLOT(add() ) );
    fillData();
    updateType();
    updateText();
}

NewLabel::~NewLabel()
{
    if( types )
        delete [] types;
}

void NewLabel::fillData()
{
    comboProducer->clear();
    QStringList list = Definition::getProducers();
    comboProducer->addItems( list );

    if( comboProducer->count() ) {
        types = new QStringList[comboProducer->count()];
        for( int i = 0; i < comboProducer->count(); i++ )
            types[i] = Definition::getTypes( comboProducer->itemText( i ) );
    }
}

void NewLabel::updateType()
{
    comboType->clear();
    if( types )
        comboType->addItems( types[ comboProducer->currentIndex() ] );
}

/*void NewLabel::updateText()
{
    comboProducer->setEnabled( !checkEmpty->isChecked() );
    comboType->setEnabled( !checkEmpty->isChecked() );
    TextLabel2->setEnabled( !checkEmpty->isChecked() );
    TextLabel3->setEnabled( !checkEmpty->isChecked() );
    
    if(!checkEmpty->isChecked())
    {
        Definition d( comboProducer->currentText(), comboType->currentText() );
        TextLabel4->setText( QString(i18n(
            "<b>Format:</b><br>\nWidth: ") + I2S(d.getMeasurements().width()) +
            i18n("%1<br>Height: ") + I2S(d.getMeasurements().height()) +
            i18n("%2<br>Horizontal Gap: ") + I2S(d.getMeasurements().gapH()) +
            i18n("%3<br>Vertical Gap: ") + I2S(d.getMeasurements().gapV()) +
            i18n("%4<br>Top Gap: ") + I2S(d.getMeasurements().gapTop()) +
            i18n("%5<br>Left Gap: ") + I2S(d.getMeasurements().gapLeft()) + "%6<br>"
            ).arg( Measurements::system() )
            .arg( Measurements::system() )
            .arg( Measurements::system() )
            .arg( Measurements::system() )
            .arg( Measurements::system() )
            .arg( Measurements::system() ) );
    
        preview->setRect( QRect( (int)d.getMeasurements().gapLeftMM(), 
                                 (int)d.getMeasurements().gapTopMM(), 
                                 (int)d.getMeasurements().widthMM(), 
                                 (int)d.getMeasurements().heightMM() ) );
        preview->setMeasurements( d.getMeasurements() );
        preview->setPrvEnabled( true );        
        preview->repaint();
        curid = d.getId();
    }
    else
    {
        TextLabel4->setText( i18n("No label selected.") );
        preview->setPrvEnabled( false );
        preview->repaint();
    }
}*/

void NewLabel::updateText()
{
    Definition d( comboProducer->currentText(), comboType->currentText() );
    TextLabel4->setText( QString(i18n(
        "<b>Format:</b><br>\nWidth: ") + I2S(d.getMeasurements().width()) +
        i18n("%1<br>Height: ") + I2S(d.getMeasurements().height()) +
        i18n("%2<br>Horizontal Gap: ") + I2S(d.getMeasurements().gapH()) +
        i18n("%3<br>Vertical Gap: ") + I2S(d.getMeasurements().gapV()) +
        i18n("%4<br>Top Gap: ") + I2S(d.getMeasurements().gapTop()) +
        i18n("%5<br>Left Gap: ") + I2S(d.getMeasurements().gapLeft()) + "%6<br>"
        ).arg( Measurements::system() )
        .arg( Measurements::system() )
        .arg( Measurements::system() )
        .arg( Measurements::system() )
        .arg( Measurements::system() )
        .arg( Measurements::system() ) );
    
    preview->setRect( QRect( (int)d.getMeasurements().gapLeftMM(), 
                             (int)d.getMeasurements().gapTopMM(), 
                             (int)d.getMeasurements().widthMM(), 
                             (int)d.getMeasurements().heightMM() ) );
    preview->setMeasurements( d.getMeasurements() );
    preview->setPrvEnabled( true );        
    preview->repaint();
    curid = d.getId();
}

bool NewLabel::isInCombo( QComboBox* combo, QString text )
{
    if( combo->count() == 0 )
        return false;

    for( int i = 0; i < combo->count(); i++ )
        if( combo->itemText( i ) == text )
            return true;
    return false;
}

void NewLabel::setLabelId( int id )
{
    if( SqlTables::isConnected() ) {
        QString name;
        QSqlQuery query("SELECT type FROM " TABLE_LABEL_DEF " WHERE label_no='" + QString::number( id ) + "'" );
        while( query.next() )
            name = query.value( 0 ).toString();

        for( int i = 0; i < comboProducer->count(); i++ ) {
            comboProducer->setCurrentIndex( i );
            updateType();
            for( int z = 0; z < comboType->count(); z++ )
                if( comboType->itemText( z ) == name ) {
                    comboProducer->setCurrentIndex( i );
                    comboType->setCurrentIndex( z );
                    return;
                }
        }
    }

    comboProducer->setCurrentIndex( 0 );
    updateType();
}

void NewLabel::add()
{
    DefinitionDialog* d = new DefinitionDialog( this );
    if( d->exec() == QDialog::Accepted ){
        fillData();
        updateType();
        updateText();
    }
    delete d;
}

/*bool NewLabel::empty() const
{
    return checkEmpty->isChecked();
}*/


#include "newlabel.moc"
