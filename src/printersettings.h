/***************************************************************************
                          printersettings.h  -  description
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

#ifndef PRINTERSETTINGS_H
#define PRINTERSETTINGS_H

#include <qobject.h>
#include <qstring.h>
#include <qprinter.h>

class KUrl;

typedef struct labelprinterdata {
   int articleEvent1;
   int articleEvent2;
   int articleEvent3;
   int articleEvent4;

   int groupEvent1;
   int groupEvent2;
   int groupEvent3;
   int groupEvent4;

   bool useCustomNo;

   int quality; // printer quality; enum PrinterMode
   bool border;
   int format; // index of the page format
   
   QString comment;
   QString separator;
   QString quote;
};

typedef struct PageFormatInfo
{
    QPrinter::PageSize qprinter;
    const char* shortName; // Short name
    const char* descriptiveName; // Full name, which will be translated
    double width; // in mm
    double height; // in mm
};

class QPrinter;
class KComboBox;
class PrinterSettings: public QObject  {
    Q_OBJECT
    public:
        enum { High = QPrinter::HighResolution, Middle, VeryHigh };
        enum { TEC, ZEBRA, INTERMEC, EPCL };

        PrinterSettings();
        ~PrinterSettings();

        labelprinterdata* getData() const {
            return lpdata;
        }

        int getQuality() const;

        // Get the size of the current selected page format
        double pageWidth() const;
        double pageHeight() const;

        void insertPageFormat( KComboBox* combo );
        static PrinterSettings* getInstance();

        // don't forget to delete printer
        QPrinter* setupPrinter( const KUrl & url, QWidget* parent, bool immediately = false, const QString & prn = QString::null );
        
    public slots:
        void loadConfig();
        void saveConfig();

    private:
        labelprinterdata* lpdata;

        static PrinterSettings* printerInstance;
};

#endif
