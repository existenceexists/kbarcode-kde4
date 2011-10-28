/***************************************************************************
                          printersettings.cpp  -  description
                             -------------------
    begin                : Don Jan 16 2003
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

#include "printersettings.h"
#include "kbarcode.h"

// QT includes
#include <QtGui/QPrinter>
#include <QtGui/QPrintDialog>

// KDE includes
#include <kapplication.h>
#include <kcombobox.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <klocale.h>
#include <kurl.h>

// Stolen from koffice/lib/kofficecore/koGlobal.cc
// and modified afterwards

// - the comments "should be..." indicates the exact values if the inch sizes would be multiplied by 25.4 mm/inch

// paper formats ( mm ) - public for compat reasons, but DO NOT USE in new programs !
// See KoPageFormat's methods instead.

const PageFormatInfo pageFormatInfo[]=
{
    { QPrinter::A3,           "A3",           I18N_NOOP("ISO A3"),       297.0,  420.0 },
    { QPrinter::A4,           "A4",           I18N_NOOP("ISO A4"),       210.0,  297.0 },
    { QPrinter::A5,           "A5",           I18N_NOOP("ISO A5"),       148.0,  210.0 },
    { QPrinter::Letter,       "Letter",       I18N_NOOP("US Letter"),    216.0,  279.0 }, // should be 215.9 mm x 279.4 mm
    { QPrinter::Legal,        "Legal",        I18N_NOOP("US Legal"),     216.0,  356.0 }, // should be 215.9 mm x 335.6 mm
    { QPrinter::A4,           "Screen",       I18N_NOOP("Screen"),       297.0,  210.0 }, // Custom, so fall back to A4
    { QPrinter::A4,           "Custom",       I18N_NOOP("Custom"),       210.0,  297.0 }, // Custom, so fall back to A4
    { QPrinter::B5,           "B5",           I18N_NOOP("ISO B5"),       182.0,  257.0 },
    { QPrinter::Executive,    "Executive",    I18N_NOOP("US Executive"), 191.0,  254.0 }, // should be 190.5 mm x 254.0 mm
    { QPrinter::A0,           "A0",           I18N_NOOP("ISO A0"),       841.0, 1189.0 },
    { QPrinter::A1,           "A1",           I18N_NOOP("ISO A1"),       594.0,  841.0 },
    { QPrinter::A2,           "A2",           I18N_NOOP("ISO A2"),       420.0,  594.0 },
    { QPrinter::A6,           "A6",           I18N_NOOP("ISO A6"),       105.0,  148.0 },
    { QPrinter::A7,           "A7",           I18N_NOOP("ISO A7"),        74.0,  105.0 },
    { QPrinter::A8,           "A8",           I18N_NOOP("ISO A8"),        52.0,   74.0 },
    { QPrinter::A9,           "A9",           I18N_NOOP("ISO A9"),        37.0,   52.0 },
    { QPrinter::B0,           "B0",           I18N_NOOP("ISO B0"),      1030.0, 1456.0 },
    { QPrinter::B1,           "B1",           I18N_NOOP("ISO B1"),       728.0, 1030.0 },
    { QPrinter::B10,          "B10",          I18N_NOOP("ISO B10"),       32.0,   45.0 },
    { QPrinter::B2,           "B2",           I18N_NOOP("ISO B2"),       515.0,  728.0 },
    { QPrinter::B3,           "B3",           I18N_NOOP("ISO B3"),       364.0,  515.0 },
    { QPrinter::B4,           "B4",           I18N_NOOP("ISO B4"),       257.0,  364.0 },
    { QPrinter::B6,           "B6",           I18N_NOOP("ISO B6"),       128.0,  182.0 },
    { QPrinter::C5E,          "C5",           I18N_NOOP("ISO C5"),       163.0,  229.0 }, // Some sources tells: 162 mm x 228 mm
    { QPrinter::Comm10E,      "Comm10",       I18N_NOOP("US Common 10"), 105.0,  241.0 }, // should be 104.775 mm x 241.3 mm
    { QPrinter::DLE,          "DL",           I18N_NOOP("ISO DL"),       110.0,  220.0 },
    { QPrinter::Folio,        "Folio",        I18N_NOOP("US Folio"),     210.0,  330.0 }, // should be 209.54 mm x 330.2 mm
    { QPrinter::Ledger,       "Ledger",       I18N_NOOP("US Ledger"),    432.0,  279.0 }, // should be 431.8 mm x 297.4 mm
    { QPrinter::Tabloid,      "Tabloid",      I18N_NOOP("US Tabloid"),   279.0,  432.0 }  // should be 297.4 mm x 431.8 mm
};
// end stolen

PrinterSettings* PrinterSettings::printerInstance = 0;

PrinterSettings::PrinterSettings()
    :QObject( 0 )
{
    setObjectName("printersettings");
    lpdata = new labelprinterdata;
    loadConfig();

    connect( kapp, SIGNAL( aboutToQuit() ), this, SLOT( saveConfig() ) );
}

PrinterSettings::~PrinterSettings()
{
}

PrinterSettings* PrinterSettings::getInstance()
{
    if( !printerInstance )
        printerInstance = new PrinterSettings();

    return printerInstance;
}

void PrinterSettings::loadConfig()
{
    KConfigGroup config = KGlobal::config()->group( "Printer" );

    /*lpdata->articleEvent1 = (enum linebreak)config.readEntry("articleEvent1", NO_BREAK );// -!F: delete these lines that are commented out
    lpdata->articleEvent2 = (enum linebreak)config.readEntry("articleEvent2", NO_BREAK );
    lpdata->articleEvent3 = (enum linebreak)config.readEntry("articleEvent3", NO_BREAK );
    lpdata->articleEvent4 = (enum linebreak)config.readEntry("articleEvent4", NO_BREAK );
    lpdata->groupEvent1 = (enum linebreak)config.readEntry("groupEvent1", NO_BREAK );
    lpdata->groupEvent2 = (enum linebreak)config.readEntry("groupEvent2", NO_BREAK );
    lpdata->groupEvent3 = (enum linebreak)config.readEntry("groupEvent3", NO_BREAK );
    lpdata->groupEvent4 = (enum linebreak)config.readEntry("groupEvent4", NO_BREAK );
    lpdata->useCustomNo = config.readEntry("UseArticleCustomerNo", false );
    lpdata->quality = config.readEntry( "quality", Middle );*/
    lpdata->articleEvent1 = (enum linebreak)config.readEntry("articleEvent1", int(NO_BREAK) );
    lpdata->articleEvent2 = (enum linebreak)config.readEntry("articleEvent2", int(NO_BREAK) );
    lpdata->articleEvent3 = (enum linebreak)config.readEntry("articleEvent3", int(NO_BREAK) );
    lpdata->articleEvent4 = (enum linebreak)config.readEntry("articleEvent4", int(NO_BREAK) );
    lpdata->groupEvent1 = (enum linebreak)config.readEntry("groupEvent1", int(NO_BREAK) );
    lpdata->groupEvent2 = (enum linebreak)config.readEntry("groupEvent2", int(NO_BREAK) );
    lpdata->groupEvent3 = (enum linebreak)config.readEntry("groupEvent3", int(NO_BREAK) );
    lpdata->groupEvent4 = (enum linebreak)config.readEntry("groupEvent4", int(NO_BREAK) );
    lpdata->useCustomNo = config.readEntry("UseArticleCustomerNo", false );
    lpdata->quality = config.readEntry( "quality", int(Middle) );
    
    if( lpdata->quality != High && lpdata->quality != Middle  && lpdata->quality != VeryHigh )
        lpdata->quality = Middle;

    config = KGlobal::config()->group( "BatchPrinting" );
    
    lpdata->comment = config.readEntry("comment", QString("#") );
    lpdata->separator = config.readEntry("separator", QString(";") );
    lpdata->quote = config.readEntry("quote", QString(""));
    lpdata->border = config.readEntry("border", false );
    lpdata->format = config.readEntry("PageSize", int(-1) );
    
    // get default page size from KDE
    if( lpdata->format == -1 )
        for( unsigned int i = 0; i < sizeof( pageFormatInfo ) / sizeof( PageFormatInfo ); i++ )
            if( pageFormatInfo[i].qprinter == KGlobal::locale()->pageSize() ) {
                lpdata->format = i;
                break;
            }

}

void PrinterSettings::saveConfig()
{
    KConfigGroup config = KGlobal::config()->group( "Printer" );

    config.writeEntry("articleEvent1", lpdata->articleEvent1 );
    config.writeEntry("articleEvent2", lpdata->articleEvent2 );
    config.writeEntry("articleEvent3", lpdata->articleEvent3 );
    config.writeEntry("articleEvent4", lpdata->articleEvent4 );
    config.writeEntry("groupEvent1", lpdata->groupEvent1 );
    config.writeEntry("groupEvent2", lpdata->groupEvent2 );
    config.writeEntry("groupEvent3", lpdata->groupEvent3 );
    config.writeEntry("groupEvent4", lpdata->groupEvent4 );
    config.writeEntry("quality", lpdata->quality );
    config.writeEntry("UseArticleCustomerNo", lpdata->useCustomNo );
        
    config = KGlobal::config()->group( "BatchPrinting" );
    
    config.writeEntry("comment", lpdata->comment );
    config.writeEntry("separator", lpdata->separator );
    config.writeEntry("quote", lpdata->quote );
    config.writeEntry("border", lpdata->border );
    config.writeEntry("PageSize", lpdata->format );

    config.sync();
}

int PrinterSettings::getQuality() const
{
    return lpdata->quality == Middle || lpdata->quality == VeryHigh? High : lpdata->quality;
};

QPrinter* PrinterSettings::setupPrinter( const KUrl & url, QWidget* parent, bool immediately, const QString & prn )
{
    QPrinter* printer = new QPrinter( (enum QPrinter::PrinterMode)getQuality() );
    if( getData()->quality == Middle )
        printer->setResolution( 300 );
    else if( getData()->quality == VeryHigh )
        printer->setResolution( 1200 );

    printer->setFullPage( true ); // don't use built-in margin system

    QPrintDialog printDialog(printer, parent);
    if( !immediately && !printDialog.exec() )
        return 0;

    /*if( immediately && !prn.isEmpty() )
        printer->autoConfigure( prn );*/
    if( immediately && !prn.isEmpty() )
        printer->setPrinterName( prn );// -!F: Is this the right replacement of autoConfigure() ?

    if( !url.isValid() )
        printer->setOutputFileName( url.fileName() );
        
    return printer;
}

void PrinterSettings::insertPageFormat( KComboBox* combo )
{
    for( unsigned int i = 0; i < sizeof( pageFormatInfo ) / sizeof( PageFormatInfo ); i++ )
        combo->addItem( pageFormatInfo[i].descriptiveName );
}

double PrinterSettings::pageWidth() const
{
    return pageFormatInfo[lpdata->format].width;
}

double PrinterSettings::pageHeight() const
{
    return pageFormatInfo[lpdata->format].height;
}


#include "printersettings.moc"
