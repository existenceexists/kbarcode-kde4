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
#include <klineedit.h>
#include <knuminput.h>

// QT includes
#include <qcheckbox.h>
#include <kglobal.h>

KBarcodeSettings* KBarcodeSettings::m_instance = 0;
int KBarcodeSettings::gridsize = 30;
bool KBarcodeSettings::newdlg = true;
/*QColor KBarcodeSettings::gridcolor = Qt::black;*/
QColor KBarcodeSettings::gridcolor = Qt::black;
QString KBarcodeSettings::dateFormat = "";

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
    /*gridcolor = config.readEntry("gridcolor", &tmpc );*/
    QColor gridcolor = config.readEntry("gridcolor", tmpc );
    /**gridcolor = gridcolor1.rgb();*/
    newdlg = config.readEntry("AskNewDlg", true );
    dateFormat = config.readEntry("DateFormat", "dd-MM-yyyy" );
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
