/***************************************************************************
                          zplutils.cpp  -  description
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

#include "zplutils.h"

// Qt includes
#include <qbuffer.h>
#include <qimage.h>
#include <QPaintDevice>
#include <qregexp.h>
#include <qstring.h>

// font table for IPL, thanks to Erich Kitzmueller
struct { int size; int c; int h; int w; } iplfonttable[] = {
    { 4, 7, 1, 1 },
    { 5, 0, 1, 1 },
    { 6, 1, 1, 1 },
    { 7, 2, 1, 1 },
    { 8, 20, 1, 1 },
    { 10, 24, 1, 1 },
    { 11, 23, 1, 1 },
    { 12, 21, 1, 1 },
    { 14, 2, 2, 2 },
    { 16, 20, 2, 2 },
    { 19, 22, 1, 1 }
};

BarcodePrinterDevice::BarcodePrinterDevice( double dpix, double dpiy )
    : QPaintDevice( 0 )
{
    m_resolution_x = dpix;
    m_resolution_y = dpiy;
}

int BarcodePrinterDevice::metric( int e ) const
{
    int val = 0;
    switch ( e ) {
        case QPaintDevice::PdmPhysicalDpiX:
            val = (int)m_resolution_y;
            break;
        case QPaintDevice::PdmPhysicalDpiY:
            val = (int)m_resolution_x;
            break;
        case QPaintDevice::PdmDpiX:
            val = (int)m_resolution_x;
            break;
        case QPaintDevice::PdmDpiY:
            val = (int)m_resolution_y;
            break;

        case QPaintDevice::PdmNumColors:
            val = 2; // black and white
            break;
        case QPaintDevice::PdmDepth:
            val = 1; // black and white
            break;

        case QPaintDevice::PdmWidth:
        case QPaintDevice::PdmHeight:
        case QPaintDevice::PdmWidthMM:
        case QPaintDevice::PdmHeightMM:
        default:
            break;
    }

    return val;
}

QMap<QString,QString> ZPLUtils::encodings;

QString ZPLUtils::footer()
{
    return "^XZ\n";
}

QString ZPLUtils::header()
{
    QString zpl = QString::null;

    zpl += "^FXLabel created by KBarcode www.kbarcode.net\n";
    zpl += "^XA\n"; // Label start
    zpl += "^JMA\n"; // set printer to 304dpi
    zpl += "^LH0,0\n"; // set label origin to 0, 0

    return zpl;
}

QString ZPLUtils::encoding( const QString & encoding )
{
    if( !encodings.count() )
        fillEncodings();

    return encodings[encoding];
}

QString ZPLUtils::fieldData( const QString & data )
{
    QString zpl = QString::null;

    zpl += "^FD" + data + "^FS\n"; // field data

    return zpl;
}

QString ZPLUtils::fieldOrigin( int x, int y )
{
    QString zpl = QString::null;

    zpl = QString("^FO%1,%2\n").arg( x ).arg( y ); // field origin

    return zpl;
}

QString ZPLUtils::font( const QFont & )
{
    QString zpl = QString::null;
    QString fontname = "D";
    // valid fonts:
    // A - H, GS
    /*
    if( font.isNull() )
        font = "D";
    else
        // Do some parsing
        ; // empty else!!!!
    */

    zpl += "^A" + fontname + "\n"; // select font

    return zpl;
}

void ZPLUtils::fillEncodings()
{
    encodings.insert( "b1", "1" ); // Code11
    encodings.insert( "b2", "2" ); // Interlieved 2 of 5
    encodings.insert( "i25", "2" ); // Interlieved 2 of 5
    encodings.insert( "i25 -c", "2" ); // Interlieved 2 of 5 no checksum
    encodings.insert( "code39", "3" ); // code39
    encodings.insert( "code39 -c", "3" ); // code39 no checksum
    encodings.insert( "b8", "3" ); // code39
    encodings.insert( "b9", "3" ); // code39
    encodings.insert( "pdf417", "7" ); // PDF417
    encodings.insert( "b55", "7" ); // PDF417
    encodings.insert( "b10", "8" ); // EAN8
    encodings.insert( "b11", "8" ); // EAN8
    encodings.insert( "b12", "8" ); // EAN8
    encodings.insert( "ean", "8" ); // EAN8
    encodings.insert( "b37", "9" ); // UPC-E
    encodings.insert( "b38", "9" ); // UPC-E
    encodings.insert( "b39", "9" ); // UPC-E
    encodings.insert( "upc", "9" ); // UPC-E
    encodings.insert( "code93", "A" ); // Code 93
    encodings.insert( "b25", "A" ); // Code 93
    encodings.insert( "code128", "C" ); // Code 128
    encodings.insert( "code128b", "C" ); // Code 128
    encodings.insert( "code128c", "C" ); // Code 128
    encodings.insert( "b20", "C" ); // Code 128
    encodings.insert( "b59", "C" ); // Code 128
    encodings.insert( "b60", "C" ); // Code 128
    encodings.insert( "b61", "C" ); // Code 128
    encodings.insert( "b57", "D" ); // Maxicode
    encodings.insert( "ean", "E" ); // EAN 13
    encodings.insert( "isbn", "E" ); // EAN 13
    encodings.insert( "b13", "E" ); // EAN 13
    encodings.insert( "b14", "E" ); // EAN 13
    encodings.insert( "b15", "E" ); // EAN 13
    encodings.insert( "b56", "F" ); // micro PDF417
    encodings.insert( "b2", "JF" ); // 2 of 5 Standard
    encodings.insert( "cbr", "K" ); // codabar
    encodings.insert( "b18", "K" ); // codabar
    encodings.insert( "b19", "K" ); // codabar
    encodings.insert( "b50", "L" ); // LOGMARS
    encodings.insert( "msi", "M" ); // MSI
    encodings.insert( "b47", "M" ); // MSI
    encodings.insert( "pls", "P" ); // Plessey
    encodings.insert( "b46", "P" ); // Plessey
    encodings.insert( "b58", "Q" ); // QR Code
    encodings.insert( "upc", "U" ); // UPC A
    encodings.insert( "b34", "U" ); // UPC A
    encodings.insert( "b35", "U" ); // UPC A
    encodings.insert( "b36", "U" ); // UPC A
    encodings.insert( "b71", "X" ); // Datamatrix
    encodings.insert( "b40", "Z" ); // Postnet
    encodings.insert( "b41", "Z" ); // Postnet
    encodings.insert( "b42", "Z" ); // Postnet
    encodings.insert( "b43", "Z" ); // Postnet
    encodings.insert( "b44", "Z" ); // Postnet
    encodings.insert( "b45", "Z" ); // Postnet
}

/***************************************************************************/

IPLUtils::IPLUtils()
{
    m_counter = 0;
}

QMap<QString,QString> IPLUtils::encodings;

void IPLUtils::addValue( const QString & v )
{
    m_values.append( v );
}

int IPLUtils::counter()
{
    return m_counter++;
}

QString IPLUtils::encoding( const QString & type )
{
    if( !encodings.count() )
        fillEncodings();

    return encodings[type];
}

QString IPLUtils::field( const QString & data )
{
    QString ipl = "<STX>";
    ipl += data;
    ipl += "<ETX>\n";
    return ipl;
}

QString IPLUtils::fieldOrigin( int x, int y )
{
    return QString("o%1,%2;f0;").arg( x ).arg( y ); // field origin
}

QString IPLUtils::footer()
{
    QString ipl = QString::null;

    // set the printer to print mode
    ipl += field( "R" );
    // start with actual data
    ipl += field( "<ESC>E3<CAN>" );  // choose format number 3

    for( unsigned int i = 0; i < m_values.count(); i++ )
        ipl += field( m_values[i] + ( i != m_values.count() - 1 ? "<CR>" : QString::null ) );

    // end actual data
    ipl += field( "<ETB>" );

    return ipl;
}

QString IPLUtils::header()
{
    QString ipl = QString::null;

    // start form definition:
    // ---
    // set the printer into propram mode
    ipl += field( "<ESC>P" );
    // set darkness to 0 (range -10 to 10)
    ipl += field( "<SI>d0" );
    // set print speed
    ipl += field( "<SI>S20" );
    // erase format 3, create format 3
    // a Intermec printer can store several "formats" (form definitions), we choose abitrarely number 3
    ipl += field( "E3;F3;" );

    return ipl;
}

void IPLUtils::fillEncodings()
{
    encodings.insert( "code39", "0" ); // code39
    encodings.insert( "code39 -c", "0" ); // code39 no checksum
    encodings.insert( "b8", "0" ); // code39
    encodings.insert( "b9", "0" ); // code39
    encodings.insert( "code93", "1" ); // Code 93
    encodings.insert( "b25", "1" ); // Code 93
    encodings.insert( "b2", "2" ); // Interlieved 2 of 5
    encodings.insert( "i25", "2" ); // Interlieved 2 of 5
    encodings.insert( "i25 -c", "2" ); // Interlieved 2 of 5 no checksum
    encodings.insert( "b2", "3" ); // 2 of 5 Standard
    encodings.insert( "cbr", "4" ); // codabar
    encodings.insert( "b18", "4" ); // codabar
    encodings.insert( "b19", "4" ); // codabar
    encodings.insert( "b1", "5" ); // Code11
    encodings.insert( "code128", "6" ); // Code 128
    encodings.insert( "code128b", "6" ); // Code 128
    encodings.insert( "code128c", "6" ); // Code 128
    encodings.insert( "b20", "6" ); // Code 128
    encodings.insert( "b59", "6" ); // Code 128
    encodings.insert( "b60", "6" ); // Code 128
    encodings.insert( "b61", "6" ); // Code 128
    encodings.insert( "b10", "7" ); // EAN8
    encodings.insert( "b11", "7" ); // EAN8
    encodings.insert( "b12", "7" ); // EAN8
    encodings.insert( "ean", "7" ); // EAN8
    encodings.insert( "b37", "7" ); // UPC-E
    encodings.insert( "b38", "7" ); // UPC-E
    encodings.insert( "b39", "7" ); // UPC-E
    encodings.insert( "upc", "7" ); // UPC-E
    encodings.insert( "ean", "7" ); // EAN 13
    encodings.insert( "isbn", "7" ); // EAN 13
    encodings.insert( "b13", "7" ); // EAN 13
    encodings.insert( "b14", "7" ); // EAN 13
    encodings.insert( "b15", "7" ); // EAN 13
    encodings.insert( "upc", "7" ); // UPC A
    encodings.insert( "b34", "7" ); // UPC A
    encodings.insert( "b35", "7" ); // UPC A
    encodings.insert( "b36", "7" ); // UPC A
    encodings.insert( "b40", "10" ); // Postnet
    encodings.insert( "b41", "10" ); // Postnet
    encodings.insert( "b42", "10" ); // Postnet
    encodings.insert( "b43", "10" ); // Postnet
    encodings.insert( "b44", "10" ); // Postnet
    encodings.insert( "b45", "10" ); // Postnet
    encodings.insert( "pdf417", "12" ); // PDF417
    encodings.insert( "b55", "12" ); // PDF417
    encodings.insert( "b57", "14" ); // Maxicode
    encodings.insert( "b71", "17" ); // Datamatrix
    encodings.insert( "b58", "18" ); // QR Code
    encodings.insert( "b56", "19" ); // micro PDF417
}

/***************************************************************************/

QMap<QString,QString> EPCLUtils::encodings;

QString EPCLUtils::footer()
{
    QString pcl = QString::null;

    // print color buffers
    pcl += EPCLUtils::field( "IS 0" ); // Yellow
    pcl += EPCLUtils::field( "IS 1" ); // Magenta
    pcl += EPCLUtils::field( "IS 2" ); // Cyan
    // print black resin buffer
    pcl += EPCLUtils::field( "I 10" );
    // print varnish and eject card
    pcl += EPCLUtils::field( "IV" );

    return pcl;
}

QString EPCLUtils::header()
{
    QString pcl = QString::null;

    // start form definition:
    // ---
    // Clear monochrome buffer
    pcl += EPCLUtils::field( "F" );
    // Clear varnish buffer
    pcl += EPCLUtils::field( "vF" );
    // Clear color buffers
    pcl += EPCLUtils::field( "$F" );
    // Make sure everything is positioned
    pcl += EPCLUtils::field( "+EC 0" );

    return pcl;
}

QString EPCLUtils::field( const QString & data )
{
    QString pcl = "\033" + data + "\r\n";
    return pcl;
}

QString EPCLUtils::encoding( const QString & type )
{
    if( !encodings.count() )
        fillEncodings();

    return encodings[type];
}

void EPCLUtils::fillEncodings()
{
    encodings.insert( "code39", "0" ); // code39
//    encodings.insert( "code39 -c", "0" ); // code39 no checksum
    encodings.insert( "b8", "0" ); // code39
    encodings.insert( "b9", "0" ); // code39
//    encodings.insert( "code93", "1" ); // Code 93
//    encodings.insert( "b25", "1" ); // Code 93
    encodings.insert( "b2", "1" ); // Interlieved 2 of 5
    encodings.insert( "i25", "1" ); // Interlieved 2 of 5
//    encodings.insert( "i25 -c", "2" ); // Interlieved 2 of 5 no checksum
    encodings.insert( "b2", "2" ); // 2 of 5 Standard
//    encodings.insert( "cbr", "4" ); // codabar
//    encodings.insert( "b18", "4" ); // codabar
//    encodings.insert( "b19", "4" ); // codabar
//    encodings.insert( "b1", "5" ); // Code11
    encodings.insert( "code128", "108" ); // Code 128
    encodings.insert( "code128b", "108" ); // Code 128
    encodings.insert( "code128c", "107" ); // Code 128
//    encodings.insert( "b20", "6" ); // Code 128
//    encodings.insert( "b59", "6" ); // Code 128
//    encodings.insert( "b60", "6" ); // Code 128
//    encodings.insert( "b61", "6" ); // Code 128
    encodings.insert( "b10", "3" ); // EAN8
    encodings.insert( "b11", "3" ); // EAN8
    encodings.insert( "b12", "3" ); // EAN8
    encodings.insert( "ean", "3" ); // EAN8
//    encodings.insert( "b37", "7" ); // UPC-E
//    encodings.insert( "b38", "7" ); // UPC-E
//    encodings.insert( "b39", "7" ); // UPC-E
//    encodings.insert( "upc", "7" ); // UPC-E
    encodings.insert( "ean", "4" ); // EAN 13
    encodings.insert( "isbn", "4" ); // EAN 13
    encodings.insert( "b13", "4" ); // EAN 13
    encodings.insert( "b14", "4" ); // EAN 13
    encodings.insert( "b15", "4" ); // EAN 13
    encodings.insert( "upc", "5" ); // UPC A
    encodings.insert( "b34", "5" ); // UPC A
    encodings.insert( "b35", "5" ); // UPC A
    encodings.insert( "b36", "5" ); // UPC A
//    encodings.insert( "b40", "10" ); // Postnet
//    encodings.insert( "b41", "10" ); // Postnet
//    encodings.insert( "b42", "10" ); // Postnet
//    encodings.insert( "b43", "10" ); // Postnet
//    encodings.insert( "b44", "10" ); // Postnet
//    encodings.insert( "b45", "10" ); // Postnet
//    encodings.insert( "pdf417", "12" ); // PDF417
//    encodings.insert( "b55", "12" ); // PDF417
//    encodings.insert( "b57", "14" ); // Maxicode
//    encodings.insert( "b71", "17" ); // Datamatrix
//    encodings.insert( "b58", "18" ); // QR Code
//    encodings.insert( "b56", "19" ); // micro PDF417
}

