/***************************************************************************
                         barcodeprinterdlg.cpp  -  description
                             -------------------
    begin                : Fri Oct 01 2004
    copyright            : (C) 2004 by Dominik Seichter
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

#include "barcodeprinterdlg.h"
#include "printersettings.h"

#include <kcombobox.h>
#include <klocale.h>
#include <kurlrequester.h>

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QGridLayout>

BarcodePrinterDlg::BarcodePrinterDlg( QWidget *parent )
        : KDialog( parent )
{
    setCaption( i18n( "Barcode Printer" ) );
    setButtons( KDialog::Ok | KDialog::Cancel );
    setDefaultButton( KDialog::Ok );

    QWidget* page = new QWidget( this );
    setMainWidget( page );

    QGridLayout* layout = new QGridLayout( page );

    QLabel* label = new QLabel( i18n( "&Output Format:" ), page );
    comboFormat = new KComboBox( false, page );
    label->setBuddy( comboFormat );

    checkFile = new QCheckBox( i18n( "&Print to File" ), page );

    label2 = new QLabel( i18n( "&Filename:" ), page );
    requester = new KUrlRequester( page );
    label2->setBuddy( requester );

    label3 = new QLabel( i18n( "&Device:" ), page );
    comboDevice = new KComboBox( true, page );
    label3->setBuddy( comboDevice );

    layout->addWidget( label, 0, 0 );
    layout->addWidget( comboFormat, 0, 1, 1, 2 );

    layout->addWidget( checkFile, 1, 1, 1, 2 );
    layout->addWidget( label2, 2, 0 );
    layout->addWidget( requester, 2, 1 );
    layout->addWidget( label3, 3, 0 );
    layout->addWidget( comboDevice, 3, 1, 1, 2 );

    comboFormat->addItem( i18n( "TEC Printer (TEC)" ) );
    comboFormat->addItem( i18n( "Zebra Printer (ZPL)" ) );
    comboFormat->addItem( i18n( "Intermec Printer (IPL)" ) );
    comboFormat->addItem( i18n( "EPCL Printer (EPCL)" ) );

    // do not translate unix devicenames...
    comboDevice->addItem( "/dev/lp0" );
    comboDevice->addItem( "/dev/lp1" );
    comboDevice->addItem( "/dev/lp2" );
    comboDevice->addItem( "/dev/usb/lp0" );
    comboDevice->addItem( "/dev/usb/lp1" );
    comboDevice->addItem( "/dev/usb/lp2" );

    page->setLayout( layout );

    connect( checkFile, SIGNAL( clicked() ), this, SLOT( enableControls() ) );

//    KFileDialog fd( QString::null, "*.zpl|Zebra Printer Language (*.zpl)\n*.ipl|Intermec Printer Language (*.ipl)", this, "fd", true );

    enableControls();
}


BarcodePrinterDlg::~BarcodePrinterDlg()
{
}

void BarcodePrinterDlg::enableControls()
{
    bool b = checkFile->isChecked();
    label2->setEnabled( b );
    requester->setEnabled( b );

    label3->setEnabled( !b );
    comboDevice->setEnabled( !b );
}

int BarcodePrinterDlg::outputFormat() const
{
    switch ( comboFormat->currentIndex() )
    {

        case 0:
            return PrinterSettings::TEC;

        case 1:
            return PrinterSettings::ZEBRA;

        case 2:
            return PrinterSettings::INTERMEC;

        case 3:
            return PrinterSettings::EPCL;

        default:
            return -1;
    }
}

bool BarcodePrinterDlg::printToFile() const
{
    return checkFile->isChecked();
}

const QString BarcodePrinterDlg::deviceName() const
{
    return comboDevice->currentText();
}

const QString BarcodePrinterDlg::fileName() const
{
    return requester->url().url();
}

#include "barcodeprinterdlg.moc"
