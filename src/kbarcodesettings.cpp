/***************************************************************************
                          kbarcodesettings.h  -  description
                             -------------------
    begin                : Sat Jan 10 2004
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

#include "kbarcodesettings.h"
#include "dialogs/configdialog.h"
#include "sqltables.h"
#include "printersettings.h"

// KDE includes
#include <kapplication.h>
#include <kcolorbutton.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kstandarddirs.h>

// QT includes
#include <qcheckbox.h>
#include <QRegExp>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>

KBarcodeSettings* KBarcodeSettings::m_instance = 0;
int KBarcodeSettings::gridsize = 30;
bool KBarcodeSettings::newdlg = true;
QColor KBarcodeSettings::gridcolor = Qt::black;
QString KBarcodeSettings::dateFormat = "";
QString KBarcodeSettings::purePostscriptFilePath = "";
QString KBarcodeSettings::libpostscriptbarcodeFilePath = "/usr/share/libpostscriptbarcode/barcode.ps";
QString KBarcodeSettings::customPurePostscriptFilePath = "";
int KBarcodeSettings::purePostscriptMethod = KBarcodeSettings::Automatic;

KBarcodeSettings* KBarcodeSettings::getInstance()
{
    if( !m_instance )
        m_instance = new KBarcodeSettings();

    return m_instance;
}

KBarcodeSettings::KBarcodeSettings()
{

}

KBarcodeSettings::~KBarcodeSettings()
{

}

void KBarcodeSettings::saveConfig()
{
    KConfigGroup config = KGlobal::config()->group( "LabelEditor" );

    config.writeEntry("grid", gridsize );
    config.writeEntry("gridcolor", gridcolor );
    config.writeEntry("AskNewDlg", newdlg );
    config.writeEntry("DateFormat", dateFormat );
}

void KBarcodeSettings::loadConfig()
{
    KConfigGroup config = KGlobal::config()->group( "LabelEditor" );

    QColor tmpc( Qt::lightGray );

    gridsize = config.readEntry("grid", 5);
    gridcolor = config.readEntry("gridcolor", tmpc );
    newdlg = config.readEntry("AskNewDlg", true );
    dateFormat = config.readEntry("DateFormat", "dd-MM-yyyy" );
    
    KConfigGroup backends = KGlobal::config()->group( "Backends" );
    int a = KBarcodeSettings::Automatic;
    setPurePostscriptMethod( backends.readEntry("purePostscriptMethod", a) );
    setCustomPurePostscriptFilePath( backends.readEntry("purePostscriptFilePath", QString()) );
    purePostscriptFilePath = determinePurePostscriptFilePath( 
        getPurePostscriptMethod(), getCustomPurePostscriptFilePath() );
    
}

#define MAX_LINE_LENGTH 256
#define BWIPP_VERSION_START "% Barcode Writer in Pure PostScript - Version "

int KBarcodeSettings::getPurePostscriptVersion(QString filename)
{
    QFile pureFile( filename );
    if( pureFile.open( QIODevice::ReadOnly ) )
    {
        QString version(BWIPP_VERSION_START);
        QTextStream s( & pureFile );
        QString line = s.readLine(MAX_LINE_LENGTH);
        
        while( !line.isNull() )
        {
            if( line.startsWith( version ) ) 
            {
                QRegExp dateExp("(\\d{4,4})-(\\d{2,2})-(\\d{2,2})");
                if( dateExp.indexIn(line) > -1 ) {
                    QString date = dateExp.cap(1);
                    date.append(dateExp.cap(2));
                    date.append(dateExp.cap(3));
                    bool ok;
                    int dateNum = date.toInt(&ok);
                    if( !ok ) {
                        return 0;
                    }
                    return dateNum;
                } else {
                    return 0;
                }
            }
            line = s.readLine(MAX_LINE_LENGTH);
        }
    }
    return 0;
}

QString KBarcodeSettings::determinePurePostscriptFilePath( int purePostscriptMethod, QString customPath )
{
    QString p1 = getLibpostscriptbarcodeFilePath();
    QString p2 = KStandardDirs::locate( "appdata", "barcode.ps" );
    /* 1) Automatic - Let KBarcode-kde4 choose the newest version 
     * of Barcode Writer In Pure Postscript automatically. 
     * Either the version installed with the package libpostscriptbarcode 
     * or the one shipped with KBarcode-kde4 is used, 
     * depending on which one is the most up-to-date.*/
    if( purePostscriptMethod == KBarcodeSettings::Automatic ) {
        if( !QFile::exists(p1) ) {
            return p2;
        }
        int externalVersion = getPurePostscriptVersion(p1);
        int kbarcodesVersion = getPurePostscriptVersion(p2);
        if( externalVersion >= kbarcodesVersion ) {
            return p1;
        } else {
            return p2;
        }
    /* 2) Use the version installed with the package libpostscriptbarcode 
     * into /usr/share/libpostscriptbarcode/barcode.ps .*/
    } else if( purePostscriptMethod == KBarcodeSettings::Libpostscriptbarcode ) {
        if( !QFile::exists(p1) ) {
            return p2;
        }
        return p1;
    /* 3) Use the version shipped with KBarcode-kde4. 
     * The library is in the file barcode.ps which is installed 
     * into KBarcode-kde4's data directory.*/
    } else if( purePostscriptMethod == KBarcodeSettings::KBarcodes ) {
        return p2;
    /* 4) Set another version located on the local system 
     * in a file with a path specified by the argument customPath. */
    } else if( purePostscriptMethod == KBarcodeSettings::Custom ) {
        QFileInfo info( customPath );
        if( customPath.isEmpty() || !QFile::exists(customPath) || !info.isFile() ) {
            return p2;
        }
        return customPath;
    } else {
        return p2;
    }
}

void KBarcodeSettings::emitPurePostscriptFileChanged()
{
    emit purePostscriptFileChanged();
}

void KBarcodeSettings::configure()
{
    ConfigDialog* cd = new ConfigDialog( 0 );
    cd->spinGrid->setValue( gridsize );
    cd->colorGrid->setColor( gridcolor );
    cd->checkNewDlg->setChecked( newdlg );
    cd->date->setText( dateFormat );
    if( cd->exec() == QDialog::Accepted ) {
        PrinterSettings::getInstance()->saveConfig();
        SqlTables::getInstance()->saveConfig();

        int oldgrid = gridsize;
        QColor oldcolor = gridcolor;
        gridsize = cd->spinGrid->value();
        gridcolor = cd->colorGrid->color().rgb();
        // gridsize or gridcolor has been changed
        if( oldgrid != gridsize || oldcolor != gridcolor )
            emit updateGrid( gridsize );

        newdlg = cd->checkNewDlg->isChecked();
        dateFormat = cd->date->text();

        saveConfig();
    }
}

#include "kbarcodesettings.moc"
