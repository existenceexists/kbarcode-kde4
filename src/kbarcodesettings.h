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

#ifndef KBARCODESETTINGS_H
#define KBARCODESETTINGS_H

#include <qobject.h>
#include <QColor>

/**
    A singleton which is responsible for loading and saving KBarcodes settings.
    It emits also signals when some properties change.

    @author Dominik Seichter
*/
class KBarcodeSettings : public QObject
{
    Q_OBJECT
    public:
        enum PurePostscriptMethod { Automatic = 0, Libpostscriptbarcode, KBarcodes, Custom };
        static KBarcodeSettings* getInstance();

        static const QString getDateFormat() {
            return dateFormat;
        }

        int gridSize() const { return gridsize; }
        const QColor & gridColor() const { return gridcolor; }
        bool newDialog() const { return newdlg; }
        
        /* The following functions are assosiated with 
         * setting path to the Barcode Writer In Pure Postscript library used by KBarcode-kde4.*/
        QString getPurePostscriptFilePath() const { return purePostscriptFilePath; }
        void setPurePostscriptFilePath( QString path ) { purePostscriptFilePath = path; }
        QString getCustomPurePostscriptFilePath() const { return customPurePostscriptFilePath; }
        void setCustomPurePostscriptFilePath( QString path ) { customPurePostscriptFilePath = path; }
        int getPurePostscriptMethod() const { return purePostscriptMethod; }
        void setPurePostscriptMethod( int method ) { purePostscriptMethod = method; }
        QString getLibpostscriptbarcodeFilePath() const { return libpostscriptbarcodeFilePath; }
        /* determinePurePostscriptFilePath() determines the path 
         * to the library Barcode Writer In Pure Postscript 
         * depending on the method KBarcode-kde4 is configured to use. 
         * It tries to return path to a default existing file, 
         * if a file assosiated with a given method is not found. */
        QString determinePurePostscriptFilePath( int purePostscriptMethod, QString customPath );
        int getPurePostscriptVersion(QString filename);
        void emitPurePostscriptFileChanged();
        
    public slots:
        void loadConfig();
        void saveConfig();
        void configure();

    signals:
        /** Emitted when the user changes the grid size.
          */
        void updateGrid( int );
        /** Emitted when the user using configuration dialog changes the path 
         * to the file that is used as Barcode Writer In Pure Postscript library.
          */
        void purePostscriptFileChanged();

    private:
        KBarcodeSettings();
        ~KBarcodeSettings();

        static KBarcodeSettings* m_instance;

        // LabelEditor settings:
        static int gridsize;
        static bool newdlg;
        static QColor gridcolor;
        static QString dateFormat;
        
        static QString purePostscriptFilePath;
        static QString customPurePostscriptFilePath;
        static int purePostscriptMethod;
        static QString libpostscriptbarcodeFilePath;
};

#endif
