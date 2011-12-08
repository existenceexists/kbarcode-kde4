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
        static KBarcodeSettings* getInstance();

        static const QString getDateFormat() {
            return dateFormat;
        }

        int gridSize() const { return gridsize; }
        const QColor & gridColor() const { return gridcolor; }
        bool newDialog() const { return newdlg; }
    public slots:
        void loadConfig();
        void saveConfig();
        void configure();

    signals:
        /** Emitted when the user changes the grid size
          */
        void updateGrid( int );

    private:
        KBarcodeSettings();
        ~KBarcodeSettings();

        static KBarcodeSettings* m_instance;

        // LabelEditor settings:
        static int gridsize;
        static bool newdlg;
        static QColor gridcolor;
        static QString dateFormat;
};

#endif
