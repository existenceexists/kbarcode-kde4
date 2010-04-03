/***************************************************************************
                          zplutils.h  -  description
                             -------------------
    begin                : Son Okt 12 2003
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

#ifndef ZPLUTILS_H
#define ZPLUTILS_H

#include <qpaintdevice.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <QTextStream>

class QColor;
class QFont;
class QImage;
class QSize;

class BarcodePrinterDevice : public QPaintDevice {
    public:
        BarcodePrinterDevice( double dpix, double dpiy );
        
        bool paintingActive() const { return false; }

   protected:
        virtual int metric( int e ) const;

        QPaintEngine* paintEngine() { return NULL; }
        QPaintEngine* paintEngine() const { return NULL; }

    private: 
        double m_resolution_x;
        double m_resolution_y;
};

/**
  * This class provides functions
  * to convert KBarcode label data
  * into Zebra Printer Language.
  *
  *@author Dominik Seichter
  */
class ZPLUtils {
    public: 
        static QString footer();
        static QString header();
        
        static QString encoding( const QString & encoding );
        static QString fieldData( const QString & data );
        static QString fieldOrigin( int x, int y );
        static QString font( const QFont & font );

    private:
        static void fillEncodings();
        
    protected:
        static QMap<QString,QString> encodings;
};

/**
  * This class provides functions
  * to convert KBarcode label data
  * into Intermec printer language.
  *
  * Thanks to Erich Kitzmller for providing
  * me with information and an examplefor this stuff.
  *@author Dominik Seichter
  */
class IPLUtils {
    public:
        IPLUtils();
        
        void addValue( const QString & v );
        int counter();
        
        QString encoding( const QString & type );
        QString footer();
        QString header();

        QString field( const QString & data );
        QString fieldOrigin( int x, int y );
    
    private:
        int m_counter;
        QStringList m_values;
        static QMap<QString,QString> encodings;
                
        /** fill the map to match kbarcode encoding types with IPL types */
        void fillEncodings();
};

/**
  * This class provides functions
  * to convert KBarcode label data
  * into Eltron printer control language (EPCL).
  *
  *@author Brian Glass
  */
#define EPCLWidth 646
#define EPCLHeight 1030

class EPCLUtils {
    public:
        static QString encoding( const QString & type );
        static QString footer();
        static QString header();

        static QString field( const QString & data );
        
        
        EPCLUtils( QTextStream* stream, QPaintDevice* source );

        void setTextField( int x, int y, int width, int height, const QString & text );
        void setBarcode( int x, int y, int h, const QString & value, const QString & type );
        void setRect( int x, int y, const QSize size, bool circle, int width );        
        void setImage( int x, int y, const QImage* image );
        void close();
                
    private:
        static void fillEncodings();
        static QMap<QString,QString> encodings;
        
        inline void setField( const QString & data );
        void fillImageBuffer( void );
};

#endif
