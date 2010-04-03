/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef BARKODE_H
#define BARKODE_H


#include <qcolor.h>
#include <qstring.h>
//Added by qt3to4:
#include <QList>
#include <QPixmap>

#include "barkodeengine.h"

/**
  * Enumeration values for sequence modes
  */
typedef enum { NUM, ALPHA, ALPHANUM } ESequence;

/** This values are used to indicate the feature supported by a 
  * certain barcode typedef
  */
enum { NOCUT = 0x00001, MULTILINE = 0x00002, NOTEXT = 0x00004, NOSCALE = 0x00008,
       /**
         * Different advanced option modes.
         */
       PDF417BARCODE = 0x00100, TBARCODEADV = 0x00200, PUREADV = 0x00400, DATAMATRIX = 0x00800, 
       COLORED = 0x00010,

       /**
         * Checksum ID's
         */
       POSTNETCHECK = 0x01000, CODE128CHECK = 0x02000, EAN8CHECK = 0x04000, EAN13CHECK = 0x08000,
       UPCACHECK = 0x10000, UPCECHECK = 0x20000, MODULOALLCHECK = 0x40000, MODULO10CHECK = 0x80000,

       /**
         * Short cut's
         */
       BARCODE2D =  NOCUT | MULTILINE | NOTEXT };

/** This structure holds information about a supported barcode encoding typedef
  */
typedef struct _tBarcodeInfo 
{
    /** the name which is stored for identification in the xml file
      * and used as commandline parameter for external tools
      */
    QString xml;
    /** the user visible name of this typedef
      */
    QString name;
    /** the barcode engine to use
      */
    EEngine engine;
    /** features supported by this type orred together
      */
    unsigned int features;

    /** internal value (e.g. identifier of GNU Barcode)
      */
    int internal;

    /** A regular expression that validates
     *  a value for this barcode.
     */
    QString validator;
    QString validatorNot;
} tBarcodeInfo;

typedef QList<tBarcodeInfo> TBarcodeInfoList;

class BarkodeEngine;
class QPainter;
class QPicture;
class QSize;
class QPaintDevice;
class TokenProvider;

class Barkode {
 public:
    Barkode();
    Barkode( const Barkode & barkode );
    virtual ~Barkode();

    /** set a TokenProvider for data field parsing
      */
    void setTokenProvider( TokenProvider* token );

    /** restore default values 
      */
    void defaults();

    /** call this method after you changed a property
      * to update the internal structures
      */
    void update( const QPaintDevice* device );
    
    /** get the barcode value after all sequences and datafields have
      * been parsed
      */
    const QString parsedValue();
 
    const QPixmap pixmap( double scalex = 1.0, double scaley = 1.0 );
    const QPicture picture();
    const QSize size() const;
    bool isValid() const;

    const QColor & background() const;
    const QColor & foreground() const;
    const QColor & textColor() const;
    unsigned int fontsize() const;
    unsigned int quietZone() const;
    unsigned int barHeight() const;
    const QString & type() const;
    const QString & value() const;
    bool sequenceEnabled() const;
    ESequence sequenceMode() const;
    int sequenceStart() const;
    int sequenceStep() const;
    const QString & databaseMode() const;
    bool textVisible() const;
    int rotation() const;
    double scaling() const;
    unsigned int index() const;
    double cut() const;
    int datamatrixSize() const;
    
    void setBackground( const QColor & c );
    void setForeground( const QColor & c );
    void setTextColor( const QColor & c );
    void setFontsize ( unsigned int f );
    void setQuietZone( unsigned int q );
    void setBarHeight( unsigned int h );
    void setType( const QString & type );
    void setValue( const QString & value );
    void setTextVisible( const bool b );
    /** Set the index of the barcodes for barcode sequences.
      */
    void setIndex( unsigned int i );
    void setSequenceEnabled( bool b );
    void setSequenceMode( ESequence e );
    void setSequenceStart( int s );
    void setSequenceStep( int s );
    void setDatabaseMode( const QString & mode );
    void setRotation( const int r );
    void setScaling( const double d );
    void setCut( const double c );
    void setDatamatrixSize( int s );
    
    /** Sets the barcodes data structures
      * the same as operator= 
      */
    void setData( const Barkode & barkode );
    void operator=( const Barkode & barkode );
    bool operator==( const Barkode & barkode ) const;

    inline BarkodeEngine* engine();
    inline const BarkodeEngine* engine() const;

    /** Test if the encodingType @p type has a certain 
      * @p feature, as defined in the codes QList.
      */
    static bool hasFeature( const QString & type, unsigned int feature );
    /** Returns the internal value for the encoding type @p typedef
      */
    static int internalType( const QString & type );
    /** Returns all supported encoding types
      * ready for displaying them to the user
      */
    static const QStringList* encodingTypes();
    /** Convert the uservisible encoding type @p name
      * to the internal identifier
      */
    static const QString & typeFromName( const QString & name );
    /** Convert the internal identifier @p type 
      * to the user visible encoding name
      */
    static const QString & nameFromType( const QString & type );

    static QString* validatorFromType( const QString & type );
    static QString* validatorNotFromType( const QString & type );

    /** return the barcode generation engine for type @p typedef
      */

    static EEngine engineForType( const QString & type );
    /** Draw an invalid barcode sign to QPainter @p painter
      */    
    void drawInvalid( QPainter & painter, int x = 0, int y = 0 );
    
    /** returns wether barcode functionallity
      * is available
      */
    static bool haveBarcode() {
        Barkode::initInfo();
        return ( s_haveGnuBarcode || s_havePdfEncode || s_haveTBarcode || s_havePurePostscript );
    }
    static bool haveGNUBarcode() {
        Barkode::initInfo();
        return s_haveGnuBarcode;
    }
    static bool haveTBarcode() {
        Barkode::initInfo();
        return s_haveTBarcode;
    }
    static bool haveTBarcode2() {
        Barkode::initInfo();
        return s_haveTBarcode2;
    }
    static bool havePDFBarcode() {
        Barkode::initInfo();
        return s_havePdfEncode;
    }
    static bool havePurePostscriptBarcode() {
        Barkode::initInfo();
        return s_havePurePostscript;
    }

    static tBarcodeInfo createInfo( const char* xml, const QString & name, const EEngine engine, 
                                    const unsigned int features = 0, const int internal = 0 );

 protected:
    void drawBarcode(  QPainter & painter, int x = 0, int y = 0 );

 private:
    static void initInfo();
    static void initValidators();

    void updateEngine();

 protected:
    BarkodeEngine* m_engine;
    TokenProvider* m_token;

    bool m_valid;
    bool m_sequence;
    ESequence m_esequence;
    int m_sequence_start;
    int m_sequence_step;
    bool m_text_visible;
    
    QColor m_background;
    QColor m_foreground;
    QColor m_textcolor;

    unsigned int m_quietzone;
    unsigned int m_bar_height;
    unsigned int m_fontsize;
    unsigned int m_index;
    int m_datamatrix_size;
    int m_rotation;
    
    double m_scaling;
    double m_cut; // temporarly used for the pixmap engines
                  // won't be needed for the Barkode engine
        
    QString m_type;
    QString m_value;
    QString m_database_mode;
    
    static TBarcodeInfoList s_info;
    static QStringList* s_encoding;

    static bool s_haveGnuBarcode;
    static bool s_haveTBarcode;
    static bool s_haveTBarcode2;
    static bool s_havePdfEncode;
    static bool s_havePurePostscript;
};

BarkodeEngine* Barkode::engine()
{
    return m_engine;
}

const BarkodeEngine* Barkode::engine() const
{
    return m_engine;
}

#endif // BARKODE_H


