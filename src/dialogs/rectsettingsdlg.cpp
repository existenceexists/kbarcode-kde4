/***************************************************************************
                          rectsettingsdlg.cpp  -  description
                             -------------------
    begin                : Mit Jun 18 2003
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

#include "rectsettingsdlg.h"

// Qt includes
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpen.h>
//Added by qt3to4:
#include <QPixmap>
#include <QGridLayout>
#include <QFrame>

// KDE includes
#include <kcolorbutton.h>
#include <kcombobox.h>
#include <klocale.h>
#include <knuminput.h>
#include <kdialog.h>

void fillLineCombo( KComboBox* box )
{
    /* A small helper function to fill
     * a combobox with all by Qt
     * supported pen styles.
     */

    QPainter p;
    for( int i = 1; i < 6; i++ ) {
        QPixmap pixmap( 60, 20 );
        pixmap.fill( Qt::white );
        p.begin( &pixmap );
        p.setPen( QPen( Qt::black, 3, (Qt::PenStyle)i ) );
        p.drawLine( 5, 10, 55, 10 );
        p.end();

        box->insertItem( pixmap );
    }
}

RectSettingsDlg::RectSettingsDlg(QWidget *parent )
    : KDialog( parent )
{
    setCaption( i18n("Settings") );
    /*setButtons( KDialog::Ok | KDialog::Cancel );*/// -!F: Is it needed? try to uncomment
    /*setDefaultButton( KDialog::Ok );*/// -!F: Is it needed? try to uncomment
    
    /*QFrame* frame = plainPage();*/// -!F: original, delete
    QWidget *frame = new QWidget( this );
    setMainWidget( frame );
    QGridLayout* grid = new QGridLayout( frame, 6, 6 );
    QSpacerItem* spacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum );

    
    buttonBorder = new KColorButton( frame );
    buttonFill = new KColorButton( frame );

    spinWidth = new KIntNumInput( frame );
    spinWidth->setRange( 1, 100, 1, false );

    comboLine = new KComboBox( false, frame );
    fillLineCombo( comboLine );
    
    checkBorder = new QCheckBox( i18n("&Enable Border"), frame );
    
    grid->addWidget( new QLabel( i18n("Fill Color:"), frame ), 0, 0 );
    grid->addWidget( buttonFill, 0, 1 );
    grid->addItem( spacer, 1, 0 );
    grid->addWidget( checkBorder, 2, 0 );            
    grid->addWidget( new QLabel( i18n("Border Color:"), frame ), 3, 0 );
    grid->addWidget( buttonBorder, 3, 1 );
    grid->addWidget( new QLabel( i18n("Border Width:"), frame ), 4, 0 );
    grid->addWidget( spinWidth, 4, 1 );
    grid->addWidget( new QLabel( i18n("Line Style:"), frame ), 5, 0 );
    grid->addWidget( comboLine, 5, 1 );

    connect( checkBorder, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
}

RectSettingsDlg::~RectSettingsDlg()
{
}

void RectSettingsDlg::enableControls()
{
    buttonBorder->setEnabled( checkBorder->isChecked() );
    spinWidth->setEnabled( checkBorder->isChecked() );
    comboLine->setEnabled( checkBorder->isChecked() );
}

void RectSettingsDlg::setBorderColor( const QColor & c )
{
    buttonBorder->setColor( c );
}

void RectSettingsDlg::setFillColor( const QColor & c )
{
    buttonFill->setColor( c );
}

void RectSettingsDlg::setBorderWidth( int w )
{
    spinWidth->setValue( w );
}

void RectSettingsDlg::setPenStyle( int s )
{
    if( s ) {
        comboLine->setCurrentIndex( s - 1 );
        checkBorder->setChecked( true );
    } else
        checkBorder->setChecked( false );

    enableControls();    
}

const QColor RectSettingsDlg::borderColor() const
{
    return buttonBorder->color();
}

const QColor RectSettingsDlg::fillColor() const
{
    return buttonFill->color();
}

int RectSettingsDlg::borderWidth() const
{
    return spinWidth->value();
}

int RectSettingsDlg::penStyle() const
{
    return checkBorder->isChecked() ? comboLine->currentItem() + 1 : 0;
}

LineSettingsDlg::LineSettingsDlg(QWidget *parent )
    : KDialog( parent )
{
    setCaption( i18n("Settings") );
    /*setButtons( KDialog::Ok | KDialog::Cancel );*/// -!F: Is it needed? try to uncomment
    /*setDefaultButton( KDialog::Ok );*/// -!F: Is it needed? try to uncomment
    
    /*QFrame* frame = plainPage();*/// -!F: original, delete
    QWidget *frame = new QWidget( this );
    setMainWidget( frame );
    QGridLayout* grid = new QGridLayout( frame, 6, 6 );

    buttonColor = new KColorButton( frame );

    spinWidth = new KIntNumInput( frame );
    spinWidth->setRange( 1, 100, 1, false );

    comboLine = new KComboBox( false, frame );
    fillLineCombo( comboLine );    

    grid->addWidget( new QLabel( i18n("Color:"), frame ), 0, 0 );
    grid->addWidget( buttonColor, 0, 1 );
    grid->addWidget( new QLabel( i18n("Line Width:"), frame ), 1, 0 );
    grid->addWidget( spinWidth, 1, 1 );
    grid->addWidget( new QLabel( i18n("Line Style:"), frame ), 2, 0 );
    grid->addWidget( comboLine, 2, 1 );
}

LineSettingsDlg::~LineSettingsDlg()
{
}

QPen LineSettingsDlg::pen() const
{
    return QPen( buttonColor->color(), spinWidth->value(), (Qt::PenStyle)(comboLine->currentItem() + 1) );
}

void LineSettingsDlg::setPen( const QPen p )
{
    buttonColor->setColor( p.color() );
    spinWidth->setValue( p.width() );
    comboLine->setCurrentIndex( p.style() - 1 );
}

#include "rectsettingsdlg.moc"
