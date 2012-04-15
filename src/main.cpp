/***************************************************************************
                          main.cpp  -  description
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

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kimageio.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <QString>
#include <QtDebug>
#include <QImage>
#include <QVariant>

#include "kbarcode.h"
/*#include "labeleditor.h"
#include "barcodegenerator.h"
*/
#ifndef VERSION
    #define VERSION "Unknown Version"
#endif

void setupDirs()
{
    if( KStandardDirs::locate("appdata", "logo.png").isEmpty() ) {// -!F: see my todo file
        // Maybe KBarcode was not installed into $KDEDIR
        KStandardDirs* dirs = KGlobal::dirs();
        QString kdedir = dirs->kfsstnd_prefixes();

        if( !kdedir.contains( "/usr/local/" ) )
            dirs->addPrefix( "/usr/local/" );

        if( !kdedir.contains( "/usr/local/kde/" ) )
            dirs->addPrefix( "/usr/local/kde/" );

        if( !kdedir.contains( "/usr/" ) )
            dirs->addPrefix( "/usr/" );

        // Print a warning:
        qDebug() << "WARNING: Prefix changed: " << dirs->kfsstnd_prefixes();
    }
}

int main(int argc, char *argv[])
{
    char charApplicationFileName[] = "kbarcode017";// The name of the final executable file.
    QString applicationFileName( charApplicationFileName );
    
    KAboutData aboutData( charApplicationFileName, "kbarcode", ki18n("KBarcode"), VERSION,
            ki18n("KBarcode4 is a barcode and label printing application for KDE 4."),
            KAboutData::License_GPL, ki18n("(c) 2001-2008, Dominik Seichter"), KLocalizedString(),
        "http://www.kbarcode.net", "kbarcode-users@lists.sourceforge.net" );

    aboutData.addAuthor(ki18n("Dominik Seichter"), ki18n("Programmer"), "domseichter@web.de");
    aboutData.addAuthor(ki18n("Stefan \"Stonki\" Onken"),
        ki18n("Project Manager"),
        "support@stonki.de", "http://www.stonki.de" );
    aboutData.addAuthor(ki18n("Váradi Zsolt Gyula"), ki18n("KBarcode4 Developer"), "vzsolt@users.sourceforge.net");

    aboutData.addCredit(ki18n("Alessandro Rubini"), ki18n("Wrote GNU Barcode on which kbarcode is based."),
        "rubini@prosa.it", "http://arcana.linux.it/" );
    aboutData.addCredit(ki18n("Terry Burton"), ki18n("Author of Barcode Writer in Pure Postscript"),
                        "tez@terryburton.co.uk", "http://www.terryburton.co.uk/" );
    aboutData.addCredit(ki18n("Daniele Medri"), ki18n("Italian translation"), "madrid@linuxmeeting.net" );
    aboutData.addCredit(ki18n("Anton Vaaranmaa"),
         ki18n("Finnish and Swedish translation, created the KBarcode icon"), "antonv@postikaista.net" );
    aboutData.addCredit(ki18n("Miguel Revilla Rodr�uez"), ki18n("Spanish translation"), "yo@miguelrevilla.com" );
    aboutData.addCredit(ki18n("Sandor Jager"), ki18n("Hungarian translation"), "jager@puskas.hu" );
    aboutData.addCredit(ki18n("Daniel Etzold"),
        ki18n("Tab icon code stolen from his excellent app qtella."), "detzold@qtella.net" );
    aboutData.addCredit(ki18n("Richard J. Moore"), ki18n("Wrote the RichText KPart"), "rich@kde.org", "http://xmelegance.org/" );
    aboutData.addCredit(ki18n("Yann Bouan"), ki18n("French Translation"), "yann@bouan.net" );
    aboutData.addCredit(ki18n("Erich Kitzmüller"), ki18n("Help with ZPL and IPL code"), "kitzmueller@metasyst.at" );
    aboutData.addCredit(ki18n("Dag Nygren"), ki18n("Wrote many patches to improve KBarcode"), "dag@newtech.fi" );
    aboutData.addCredit(ki18n("Frank Schoolmeesters"), ki18n("Made the Netherlands translation"), "frank_schoolmeesters@fastmail.fm" );
    aboutData.addCredit(ki18n("John Volpe"), ki18n("Added lot's of useful data fields to kbarcode"), "jtvolpe@cape.com" );
    aboutData.addCredit(ki18n("Nyssa s.r.l."), ki18n("Added TEC barcode printer support"), "imorrison@nyssa.com.ar", "http://www.nyssa.com.ar" );
    aboutData.addCredit(ki18n("Brian Glass"), ki18n("Added EPCL barcode printer support"), "brian@glassbrian.com", "http://www.glassbrian.com" );
    
    aboutData.setProgramIconName( KStandardDirs::locate("data", applicationFileName + QString( "/hi16-app-kbarcode.png" ) ) );
    aboutData.setProgramLogo( QImage( KStandardDirs::locate("data", applicationFileName + QString( "/logo.png" ) ), "PNG" ) );

    KCmdLineArgs::init( argc, argv, &aboutData );
    
    KCmdLineOptions options;

    options.add( "+[file]", ki18n("Load the file in KBarcode (if --batch is specified, the file will be used in batchprinting mode)"), 0);
    options.add( "print", ki18n("Print any loaded files immediately on the default printer "
            "or on the printer specified by the --printer commandline option and exit afterwards"), 0 );
    options.add( "printer <printer>", ki18n("Printer/destination to print on"), 0);
    options.add( ":", ki18n("KBarcode Modes:"), 0 );
    options.add( "barcode", ki18n("Start KBarcode as xbarcode replacement"), 0 );
    options.add( "label", ki18n("Open the label editor"), 0 );
    options.add( "batch", ki18n("Start directly in batchprinting mode"), 0 );
    options.add( ":", ki18n("Additional options for batchprinting (--batch):"), 0 );
    options.add( "output <mode>", ki18n("Output format where mode is one of POSTSCRIPT|IMAGE|BARCODE"), "POSTSCRIPT" );
    options.add( "serialnumber <value>", ki18n("Set the serial number to be used"), 0 );
    options.add( "serialinc <value>", ki18n("Increase a previously specified serial number using this value for each printed label"), 0 );
    options.add( "numlabels <value>", ki18n("Print <value> labels without any data"), 0 );
    options.add( "importsql <query>", ki18n("Import variable data from a specified sql query"), 0 );
    options.add( "importcsv <csvfile>", ki18n("Import variable data from a specified csv file"), 0 );

    KCmdLineArgs::addCmdLineOptions( options );

    KApplication a;
    // setup standard dirs
    setupDirs();

    
    a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
    
    KBarcode* kbc = new KBarcode( 0 );
    
    if( kbc->parseCmdLine() )
        return 0;
    else
        return a.exec();
    
    /*return 0;*/
}
