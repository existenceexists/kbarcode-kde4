/* Copyright (C) 2004 Dominik Seichter <domseichter@web.de>

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

#include "barkode.h"
#include "barkodeengine.h"
#include "gnubarcode.h"
#include "pixmapbarcode.h"
#include "purepostscript.h"
#include "tbarcode2.h"
#include "tokenprovider.h"

#ifdef _ENABLE_NATIVE_GNU_BARCODE
#  include <barcode.h>
#else
#define BARCODE_DEFAULT_FLAGS 0x00000000

#define BARCODE_ENCODING_MASK 0x000000ff   /* 256 possibilites... */
#define BARCODE_NO_ASCII      0x00000100   /* avoid text in output */
#define BARCODE_NO_CHECKSUM   0x00000200   /* avoid checksum in output */

#define BARCODE_OUTPUT_MASK   0x000ff000   /* 256 output types */
#define BARCODE_OUT_EPS       0x00001000
#define BARCODE_OUT_PS        0x00002000
#define BARCODE_OUT_PCL       0x00004000   /* by Andrea Scopece */
/*                  PCL_III   0x00008000   */
#define BARCODE_OUT_PCL_III   0x0000C000
#define BARCODE_OUT_NOHEADERS 0x00100000   /* no header nor footer */

// include our own defines from barcode.h
enum {
    BARCODE_ANY = 0,     /* choose best-fit */
    BARCODE_EAN,
    BARCODE_UPC,        /* upc == 12-digit ean */
    BARCODE_ISBN,       /* isbn numbers (still EAN13) */
    BARCODE_39,         /* code 39 */
    BARCODE_128,        /* code 128 (a,b,c: autoselection) */
    BARCODE_128C,       /* code 128 (compact form for digits) */
    BARCODE_128B,       /* code 128, full printable ascii */
    BARCODE_I25,        /* interleaved 2 of 5 (only digits) */
    BARCODE_128RAW,     /* Raw code 128 (by Leonid A. Broukhis) */
    BARCODE_CBR,        /* Codabar (by Leonid A. Broukhis) */
    BARCODE_MSI,        /* MSI (by Leonid A. Broukhis) */
    BARCODE_PLS,        /* Plessey (by Leonid A. Broukhis) */
    BARCODE_93          /* code 93 (by Nathan D. Holmes) */
};
#endif // _ENABLE_NATIVE_GNU_BARCODE

#include <qdom.h>
#include <qfile.h>
#include <qpainter.h>
#include <QPicture>
#include <qpixmap.h>
#include <qsize.h>
#include <QDebug>
//Added by qt3to4:
#include <QList>

#include <klocale.h>
#include <kstandarddirs.h>

QList<tBarcodeInfo> Barkode::s_info;
QStringList* Barkode::s_encoding   = NULL;
bool Barkode::s_haveGnuBarcode     = false;
bool Barkode::s_havePdfEncode      = false;
bool Barkode::s_haveTBarcode       = false;
bool Barkode::s_haveTBarcode2      = false;
bool Barkode::s_havePurePostscript = false;

Barkode::Barkode()
{
    initInfo();
    m_engine = NULL;
    m_token = NULL;
    
    defaults();
}

Barkode::Barkode( const Barkode & barkode )
{
    initInfo();
    m_engine = NULL;
    m_token = NULL;
    
    *this = barkode;
}

Barkode::~Barkode()
{
    if( m_engine )
        delete m_engine;
}

void Barkode::setTokenProvider( TokenProvider* token )
{
    m_token = token;
}

void Barkode::defaults()
{
    m_valid = false;
    m_text_visible = true;
    
    m_background = Qt::white;
    m_foreground = Qt::black;
    m_textcolor  = Qt::black;

    m_quietzone = 0;
    m_bar_height = 80;
    m_fontsize = 12;

    m_rotation = 0;
    m_scaling = 1.0;
    m_cut = 1.0;
    m_datamatrix_size = 0;
    
    m_type = "code39";    
    m_value = "KBARCODE";
    m_database_mode = "static";
    
    m_index = 0;
    m_sequence = false;
    m_esequence = NUM;
    m_sequence_start = 1;
    m_sequence_step = 1;

    updateEngine();

    BarkodeEngineOptions* options = m_engine ? m_engine->options() : NULL;
    if( options )
        options->defaults();
}

void Barkode::setData( const Barkode & barkode )
{
    this->m_valid = barkode.m_valid;
    this->m_text_visible = barkode.m_text_visible;
    
    this->m_background = barkode.m_background;
    this->m_foreground = barkode.m_foreground;
    this->m_textcolor  = barkode.m_textcolor;

    this->m_quietzone = barkode.m_quietzone;
    this->m_bar_height = barkode.m_bar_height;
    this->m_fontsize = barkode.m_fontsize;
    this->m_rotation = barkode.m_rotation;
    this->m_scaling = barkode.m_scaling;
    this->m_cut = barkode.m_cut;
    
    this->m_type = barkode.m_type;
    this->m_value = barkode.m_value;
    this->m_database_mode = barkode.m_database_mode;
    
    this->m_index = barkode.m_index;
    this->m_sequence = barkode.m_sequence;
    this->m_esequence = barkode.m_esequence;
    this->m_sequence_start = barkode.m_sequence_start;
    this->m_sequence_step = barkode.m_sequence_step;

    updateEngine();

    *m_engine = *(barkode.engine());

    this->m_datamatrix_size = barkode.m_datamatrix_size;
}

void Barkode::operator=( const Barkode & barkode )
{
    setData( barkode );
}

bool Barkode::operator==( const Barkode & barkode ) const
{
    bool b = true;
#warning "operator== is incomplete"

    // colors
    b = ( this->m_background == barkode.m_background && this->m_foreground == barkode.m_foreground ) && b;

    // sizes 
    b = ( this->m_quietzone == barkode.m_quietzone && this->m_bar_height == barkode.m_bar_height && this->m_fontsize == barkode.m_fontsize ) && b;

    // values
    b = ( this->m_type == barkode.m_type && this->m_value == barkode.m_value ) && b;
    
    return b;
}

const QPicture Barkode::picture()
{
    QPicture pic;
    QPainter painter( &pic );

    painter.fillRect( 0, 0, size().width(), size().height(), m_background );

    // sub classes do the real drawing
    drawBarcode( painter );

    return pic;
}

void Barkode::drawInvalid( QPainter & painter, int x, int y )
{
    QRect rect( x, y, size().width(), size().height() );
    painter.save();
    painter.fillRect( rect, Qt::white );
    painter.setPen( QPen( Qt::red, 2 ) );
    painter.drawRect( rect );
    painter.drawLine( x, y, x + size().width(), y + size().height() );
    painter.drawLine( x, y + size().height(), x + size().width(), y );
    painter.restore();
}

const QSize Barkode::size() const
{
    return m_engine ? m_engine->size() : QSize( 0, 0 );
}

const QPixmap Barkode::pixmap( double scalex, double scaley )
{
    QPixmap pixmap( (int)(size().width() * scalex), (int)(size().height() * scaley) );
    if( !pixmap.isNull() )
    {
        QPainter painter( &pixmap );
        painter.scale( scalex, scaley );
        painter.fillRect( 0, 0, size().width(), size().height(), m_background );
        drawBarcode( painter );
    }
    
    return pixmap;
}

const QString Barkode::parsedValue()
{
    QString text = m_token ? m_token->parse( m_value ) : m_value;

    if( !sequenceEnabled() )
        return text;
        
    if( m_value.contains( '#' ) <= 0 )
        return text;

    int pos = 0, counter = 1;

    pos = text.indexOf("#", pos);
    pos++;
    while( text[pos] == '#' ) {
        text.remove(pos, 1);
        counter++;
    }

    pos = text.indexOf("#", 0);
    QString temp;

    if( sequenceMode() == NUM ) {
        int v = sequenceStart() + m_index*sequenceStep();
        temp.sprintf("%0*i", counter, v );
    } else {
        for( int i = 0; i < counter; i++ )
            temp.append( "A" );

        unsigned int z = 0;
        for( int p = temp.length(); p >= 0; p--, z++ ) {
            if( sequenceMode() == ALPHA ) {
                int v = 'A' + m_index*sequenceStep();
                v -= z*('Z'-'A');
            
                if( v <= 'Z' ) {
                    temp[p] = QChar(v);                        
                    break;
                } else if( v > 'Z' )
                    v = 'Z';
                temp[p] = QChar(v);            
            } else if( sequenceMode() == ALPHANUM ) {
                qDebug("NOT IMPLEMENTED");
/*                char array[36];
                for( unsigned int i = 'A'; i <= 'Z'; i++ )
                    array[i-'A'] = i;
                for( unsigned int i = '0'; i <= '9'; i++ )
                    array['Z'-'A'+i-'0'] = i;

                int z = m_index*barcode.sequence.step;
                if( z < sizeof(array) )
                    temp[]
                int v = array[ ]*/
            }
        }
    }
    
    text.replace( pos, 1, temp);
    return text;
}

EEngine Barkode::engineForType( const QString & type )
{
    EEngine engine = NONE;
    
    for( int i=0;i<s_info.count();i++)
        if( s_info[i].xml == type )
        {
            engine = s_info[i].engine;
            break;
        }

    return engine;
}

void Barkode::updateEngine()
{
    EEngine engine = engineForType( type() );
        
    if( m_engine && engine != m_engine->engine() )
    {
        // switch to another engine
        delete m_engine;
        m_engine = NULL;
    }

    if( !m_engine )
    {
        if( engine == GNU_BARCODE )
#ifdef _ENABLE_NATIVE_GNU_BARCODE
            m_engine = new GnuBarcode();
#else
            m_engine = new PixmapBarcode();
#endif // _ENABLE_NATIVE_GNU_BARCODE
        else if( engine == TBARCODE )
            m_engine = new PixmapBarcode();
        else if( engine == TBARCODE2 )
            m_engine = new TBarcode2();
        else if( engine == PDF417 )
            m_engine = new PixmapBarcode();
        else if( engine == PURE_POSTSCRIPT )
            m_engine = new PurePostscriptBarcode();
        else
            m_engine = new EmptyEngine();
            
        m_engine->setBarkode( this );
    }
}

void Barkode::drawBarcode(  QPainter & painter, int x, int y ) 
{ 
    if( m_engine ) {
        m_engine->drawBarcode( painter, x, y );
    }
}

void Barkode::update( const QPaintDevice* device )
{
    if( m_engine ) {
        m_engine->update( device );
    }
}

////////////////////////////////////////////////////////////
// Get attributes                                         //
////////////////////////////////////////////////////////////

bool Barkode::isValid() const
{
    return m_valid;
}

const QColor & Barkode::background() const
{
    return m_background;
}

const QColor & Barkode::foreground() const
{
    return m_foreground;
}

const QColor & Barkode::textColor() const
{
    return m_textcolor;
}

unsigned int Barkode::fontsize() const
{
    return m_fontsize;
}

unsigned int Barkode::quietZone() const
{
    return m_quietzone;
}

unsigned int Barkode::barHeight() const
{
    return m_bar_height;
}

const QString & Barkode::type() const
{
    return m_type;
}

const QString & Barkode::value() const
{
    return m_value;
}

unsigned int Barkode::index() const
{
    return m_index;
}

bool Barkode::sequenceEnabled() const
{
    return m_sequence;
}

ESequence Barkode::sequenceMode() const
{
    return m_esequence;
}

int Barkode::sequenceStart() const
{
    return m_sequence_start;
}

int Barkode::sequenceStep() const
{
    return m_sequence_step;
}

const QString & Barkode::databaseMode() const
{
    return m_database_mode;
}

bool Barkode::textVisible() const
{
    return m_text_visible;
}

int Barkode::rotation() const
{
    return m_rotation;
}

double Barkode::scaling() const
{
    return m_scaling;
}

double Barkode::cut() const
{
    return m_cut;
}

int Barkode::datamatrixSize() const
{
    return m_datamatrix_size;
}

////////////////////////////////////////////////////////////
// Set attributes                                         //
////////////////////////////////////////////////////////////

void Barkode::setBackground( const QColor & c )
{
    m_background = c;
}

void Barkode::setForeground( const QColor & c )
{
    m_foreground = c;
}

void Barkode::setTextColor( const QColor & c )
{
    m_textcolor = c;
}

void Barkode::setFontsize( unsigned int f )
{
    m_fontsize = f;
}

void Barkode::setQuietZone( const unsigned int q )
{
    m_quietzone = q;
}

void Barkode::setBarHeight( unsigned int h )
{
    m_bar_height = h;
}

void Barkode::setType( const QString & type )
{
    m_type = type;
    updateEngine();
}

void Barkode::setValue( const QString & value )
{
    m_value = value;
}

void Barkode::setIndex( const unsigned int i )
{
    m_index = i;
}

void Barkode::setSequenceEnabled( bool b )
{
    m_sequence = b;
}

void Barkode::setSequenceMode( ESequence e )
{
    m_esequence = e;
}

void Barkode::setSequenceStart( int s )
{
    m_sequence_start = s;
}

void Barkode::setSequenceStep( int s )
{
    m_sequence_step = s;
}

void Barkode::setDatabaseMode( const QString & mode )
{
    m_database_mode = mode;
}

void Barkode::setTextVisible( const bool b )
{
    m_text_visible = b;
}

void Barkode::setRotation( const int r )
{
    m_rotation = r;
}

void Barkode::setScaling( const double d )
{
    m_scaling = d;
}

void Barkode::setCut( const double c )
{
    m_cut = c;
}

void Barkode::setDatamatrixSize( int s )
{
    m_datamatrix_size = s;
}

////////////////////////////////////////////////////////////
// Fill the s_info structure
////////////////////////////////////////////////////////////

bool Barkode::hasFeature( const QString & type, unsigned int feature )
{
    for( int i = 0; i < s_info.count(); i++ )
        if( s_info[i].xml == type )
            return (s_info[i].features & feature) == feature;
            
    return false;
}

int Barkode::internalType( const QString & type )
{
    for( int i = 0; i < s_info.count(); i++ )
        if( s_info[i].xml == type )
            return s_info[i].internal;
            
    return false;
}

const QStringList* Barkode::encodingTypes()
{
    if( !s_encoding )
    {
        s_encoding = new QStringList;
        for( int i = 0; i < s_info.count(); i++ )
            s_encoding->append( s_info[i].name );
        //s_encoding->sort();
    }
    
    return s_encoding;
}

const QString Barkode::typeFromName( const QString & name )
{
    for( int i = 0; i < s_info.count(); i++ )
        if( s_info[i].name == name )
            return s_info[i].xml;
            
    return QString();
}

const QString Barkode::nameFromType( const QString & type )
{
    for( int i = 0; i < s_info.count(); i++ )
        if( s_info[i].xml == type )
            return s_info[i].name;
            
    return QString();
}

QString* Barkode::validatorFromType( const QString & type )
{
    for( int i = 0; i < s_info.count(); i++ )
        if( s_info[i].xml == type )
            return &(s_info[i].validator);

    return NULL;
}

QString* Barkode::validatorNotFromType( const QString & type )
{
    for( int i = 0; i < s_info.count(); i++ )
        if( s_info[i].xml == type )
            return &(s_info[i].validatorNot);

    return NULL;
}

tBarcodeInfo Barkode::createInfo( const char* xml, const QString & name, const EEngine engine, 
                                const unsigned int features, const int internal )
{
    tBarcodeInfo info;
    QString strengine;

    switch( engine ) 
    {
        case GNU_BARCODE:
            strengine = i18n(" [GNU Barcode]");
            break;
        case PDF417:
            strengine = i18n(" [PDF 417]");
            break;
        case TBARCODE:
            strengine = i18n(" [TBarcode]");
            break;
        case TBARCODE2:
            strengine = i18n(" [TBarcode2]");
            break;
        case PURE_POSTSCRIPT:
            strengine = i18n(" [Barcode Writer in Pure Postscript]");
            break;
        case NONE:
        default: 
            // does not need to be translated as it should not visible anytime ever
            strengine = " [NONE]";
            break;
    }

    info.xml = xml;
    info.name = name + strengine;
    info.engine = engine;
    info.features = features;
    info.internal = internal;
    
    return info;
}

void Barkode::initInfo()
{
    if( s_info.count() )
        return;

    PurePostscriptBarcode::init();

    s_haveGnuBarcode = !KStandardDirs::findExe( "barcode" ).isNull();
    s_havePdfEncode  = !KStandardDirs::findExe( "pdf417_enc" ).isNull();
    s_haveTBarcode   = false; //!KStandardDirs::findExe( "tbarcodeclient" ).isNull();
// TODO: do not yet enable TBarcode2 support
    s_haveTBarcode2  = !KStandardDirs::findExe( "tbarcode" ).isNull();
    s_havePurePostscript = !KStandardDirs::findExe( "gs" ).isNull() 
                             && PurePostscriptBarcode::hasPurePostscriptBarcode();

// TODO: I think this functionality should be moved to the engines. - VZsolt
    if( s_haveGnuBarcode )
    {
        s_info.append( createInfo( "128raw", i18n("Raw code 128"), GNU_BARCODE, 0, BARCODE_128RAW ) );
        s_info.append( createInfo( "cbr", i18n("Codabar"), GNU_BARCODE, 0, BARCODE_CBR ) );
        s_info.append( createInfo( "cbr -c", i18n("Codabar (no checksum)"), GNU_BARCODE, 0, BARCODE_CBR ) );
        s_info.append( createInfo( "code128", i18n("Code 128 (a,b,c: autoselection)"), GNU_BARCODE, 0, BARCODE_128 ) );
        s_info.append( createInfo( "code128b", i18n("Code 128B, full printable ascii"), GNU_BARCODE, 0, BARCODE_128B ) );
        s_info.append( createInfo( "code128c", i18n("Code 128C (compact form digits)"), GNU_BARCODE, 0, BARCODE_128C ) );
        s_info.append( createInfo( "code39 -c", i18n("Code 39 (no checksum)"), GNU_BARCODE, 0, BARCODE_39 | BARCODE_NO_CHECKSUM ) );
        s_info.append( createInfo( "code39", i18n("Code 39"), GNU_BARCODE, 0, BARCODE_39 ) );
        s_info.append( createInfo( "code93", i18n("Code 93"), GNU_BARCODE, 0, BARCODE_93 ) );
        s_info.append( createInfo( "ean", i18n("EAN (EAN 8 or EAN 13)"), GNU_BARCODE, 0, BARCODE_EAN ) );
        s_info.append( createInfo( "i25 -c", i18n("interleaved 2 of 5 (only digits, no checksum)"), GNU_BARCODE, 0, BARCODE_I25 | BARCODE_NO_CHECKSUM ) );
        s_info.append( createInfo( "i25", i18n("interleaved 2 of 5 (only digits)"), GNU_BARCODE, 0, BARCODE_I25 ) );
        s_info.append( createInfo( "isbn", i18n("ISBN (still EAN13)"), GNU_BARCODE, NOCUT, BARCODE_ISBN ) );
        s_info.append( createInfo( "msi", i18n("MSI"), GNU_BARCODE, 0, BARCODE_MSI ) );
        s_info.append( createInfo( "pls", i18n("Plessey"), GNU_BARCODE, 0, BARCODE_PLS ) );
        s_info.append( createInfo( "upc", i18n("UPC (12-digit EAN; UPCA and UPCB)"), GNU_BARCODE, 0, BARCODE_UPC ) );
    }
    
    if( s_havePdfEncode ) 
    {
        s_info.append( createInfo( "pdf417", i18n("pdf 417 2D Barcode"), PDF417, BARCODE2D | PDF417BARCODE ) );
    }

    if( s_havePurePostscript ) 
    {
        // add a leading "ps_" so that they do not conflict with GNU Barcode types
        PurePostscriptBarcode::initInfo( &s_info );

        /*
        s_info.append( createInfo( "ps_auspost", i18n("Australian Post"), PURE_POSTSCRIPT, PUREADV | COLORED | NOCUT ) );
        s_info.append( createInfo( "ps_code11", i18n("Code 11"), PURE_POSTSCRIPT, PUREADV | COLORED ) );
        s_info.append( createInfo( "ps_code128", i18n("Code 128"), PURE_POSTSCRIPT, PUREADV | COLORED ) );
        s_info.append( createInfo( "ps_code2of5", i18n("Code 2 of 5"), PURE_POSTSCRIPT, PUREADV | COLORED ) );
        s_info.append( createInfo( "ps_code39", i18n("Code 39"), PURE_POSTSCRIPT, PUREADV | COLORED | MODULOALLCHECK ) );
        s_info.append( createInfo( "ps_code93", i18n("Code 93"), PURE_POSTSCRIPT, PUREADV | COLORED ) );
        s_info.append( createInfo( "ps_ean13", i18n("EAN 13"), PURE_POSTSCRIPT, PUREADV | COLORED ) );
        s_info.append( createInfo( "ps_ean2", i18n("EAN 2"), PURE_POSTSCRIPT, PUREADV | COLORED | NOCUT ) );
        s_info.append( createInfo( "ps_ean5", i18n("EAN 5"), PURE_POSTSCRIPT, PUREADV | COLORED | NOCUT ) );
        s_info.append( createInfo( "ps_ean8", i18n("EAN 8"), PURE_POSTSCRIPT, PUREADV | COLORED ) );
        s_info.append( createInfo( "ps_interleaved2of5", i18n("Interleaved 2 of 5"), PURE_POSTSCRIPT, PUREADV | COLORED | MODULOALLCHECK ) );
        s_info.append( createInfo( "ps_isbn", i18n("ISBN"), PURE_POSTSCRIPT, PUREADV | COLORED ) );
        s_info.append( createInfo( "ps_kix", i18n("Kix (Dutch Postal)"), PURE_POSTSCRIPT, PUREADV | COLORED | NOCUT ) );
        s_info.append( createInfo( "ps_msi", i18n("MSI"), PURE_POSTSCRIPT, PUREADV | COLORED | MODULOALLCHECK ) );
        s_info.append( createInfo( "ps_plessey", i18n("Plessey"), PURE_POSTSCRIPT, PUREADV | COLORED ) );
        s_info.append( createInfo( "ps_postnet", i18n("Postnet"), PURE_POSTSCRIPT, PUREADV | COLORED | NOCUT ) );
        s_info.append( createInfo( "ps_rationalizedCodabar", i18n("Rationalized Codabar"), PURE_POSTSCRIPT, PUREADV | COLORED | MODULOALLCHECK ) );
        s_info.append( createInfo( "ps_royalmail", i18n("Royal Mail"), PURE_POSTSCRIPT, PUREADV | COLORED | NOCUT ) );
        s_info.append( createInfo( "ps_symbol", i18n("Symbol"), PURE_POSTSCRIPT, PUREADV | COLORED ) );
        s_info.append( createInfo( "ps_upca", i18n("UPCA"), PURE_POSTSCRIPT, PUREADV | COLORED ) );
        s_info.append( createInfo( "ps_upce", i18n("UPCE"), PURE_POSTSCRIPT, PUREADV | COLORED ) );
        */
    }

    if( s_haveTBarcode ) 
    {
        s_info.append( createInfo( "b1", "Code 11", TBARCODE, TBARCODEADV | MODULO10CHECK ) );
        s_info.append( createInfo( "b2", "Code 2 of 5 (Standard)", TBARCODE, TBARCODEADV | MODULO10CHECK ) );
        s_info.append( createInfo( "b3", "Interleaved 2 of 5 Standard", TBARCODE, TBARCODEADV ) );
        s_info.append( createInfo( "b4", "Code 2 of 5 IATA", TBARCODE, TBARCODEADV | MODULO10CHECK ) );
        s_info.append( createInfo( "b5", "Code 2 of 5 Matrix", TBARCODE, TBARCODEADV | MODULO10CHECK ) );
        s_info.append( createInfo( "b6", "Code 2 of 5 Data Logic", TBARCODE, TBARCODEADV | MODULO10CHECK ) );
        s_info.append( createInfo( "b7", "Code 2 of 5 Industrial", TBARCODE, TBARCODEADV | MODULO10CHECK ) );
        s_info.append( createInfo( "b8", "Code 3 of 9 (Code 39)", TBARCODE, TBARCODEADV | MODULOALLCHECK ) );
        s_info.append( createInfo( "b9", "Code 3 of 9 (Code 39) ASCII", TBARCODE, TBARCODEADV | MODULOALLCHECK ) );
        s_info.append( createInfo( "b10", "EAN8", TBARCODE, TBARCODEADV | EAN8CHECK | MODULO10CHECK ) );
        s_info.append( createInfo( "b11", "EAN8 - 2 digits add on", TBARCODE, NOCUT | TBARCODEADV | EAN8CHECK | MODULO10CHECK ) );
        s_info.append( createInfo( "b12", "EAN8 - 5 digits add on", TBARCODE, NOCUT | TBARCODEADV | EAN8CHECK | MODULO10CHECK ) );
        s_info.append( createInfo( "b13", "EAN13", TBARCODE, TBARCODEADV | EAN13CHECK | MODULO10CHECK ) );
        s_info.append( createInfo( "b14", "EAN13 - 2 digits add on", TBARCODE, NOCUT | TBARCODEADV | EAN13CHECK | MODULO10CHECK ) );
        s_info.append( createInfo( "b15", "EAN13 - 5 digits add on", TBARCODE, NOCUT | TBARCODEADV | EAN13CHECK | MODULO10CHECK ) );
        s_info.append( createInfo( "b16", "EAN128 (supports AIS)", TBARCODE, TBARCODEADV | MODULOALLCHECK ) );
        s_info.append( createInfo( "b17", "UPC 12 Digits", TBARCODE, TBARCODEADV | MODULOALLCHECK ) );
        s_info.append( createInfo( "b18", "CodaBar (2 width)", TBARCODE, TBARCODEADV | MODULO10CHECK ) );
        s_info.append( createInfo( "b19", "CodaBar (18 widths)", TBARCODE, TBARCODEADV ) );
        s_info.append( createInfo( "b20", "Code128", TBARCODE, TBARCODEADV ) );
        s_info.append( createInfo( "b21", "Deutsche Post Leitcode", TBARCODE, TBARCODEADV ) );
        s_info.append( createInfo( "b22", "Deutsche Post Identcode", TBARCODE, TBARCODEADV ) );
        s_info.append( createInfo( "b25", "Code 93", TBARCODE, TBARCODEADV ) );
        s_info.append( createInfo( "b26", "Identical to eBC_UPCA", TBARCODE, TBARCODEADV ) );
        s_info.append( createInfo( "b33", "UCC128 (= EAN128)", TBARCODE, TBARCODEADV ) );
        s_info.append( createInfo( "b34", "UPC A", TBARCODE, TBARCODEADV | TBARCODEADV | UPCACHECK ) );
        s_info.append( createInfo( "b35", "UPC A - 2 digit add on", TBARCODE, TBARCODEADV | UPCACHECK ) );
        s_info.append( createInfo( "b36", "UPC A - 5 digit add on", TBARCODE, TBARCODEADV | UPCACHECK ) );
        s_info.append( createInfo( "b37", "UPC E", TBARCODE, TBARCODEADV | UPCECHECK ) );
        s_info.append( createInfo( "b38", "UPC E - 2 digit add on", TBARCODE, TBARCODEADV | UPCECHECK ) );
        s_info.append( createInfo( "b39", "UPC E - 5 digit add on", TBARCODE, TBARCODEADV | UPCECHECK ) );
        s_info.append( createInfo( "b40", "PostNet ZIP (5d.)", TBARCODE, NOCUT | TBARCODEADV | POSTNETCHECK ) );
        s_info.append( createInfo( "b41", "PostNet ZIP (5d.+CD)", TBARCODE, NOCUT | TBARCODEADV | POSTNETCHECK ) );
        s_info.append( createInfo( "b42", "PostNet ZIP (8d.)", TBARCODE, NOCUT | TBARCODEADV | POSTNETCHECK ) );
        s_info.append( createInfo( "b43", "PostNet ZIP+4 (5d.+4d.+CD)", TBARCODE, NOCUT | TBARCODEADV | POSTNETCHECK ) );
        s_info.append( createInfo( "b44", "PostNet DPBC (5d.+4d.+2d.)", TBARCODE, NOCUT | TBARCODEADV | POSTNETCHECK ) );
        s_info.append( createInfo( "b45", "PostNet DPBC (5d.+4d.+2d.+CD)", TBARCODE, NOCUT | TBARCODEADV | POSTNETCHECK ) );
        s_info.append( createInfo( "b46", "Plessey Code", TBARCODE, TBARCODEADV ) );
        s_info.append( createInfo( "b47", "MSI Code", TBARCODE, TBARCODEADV | MODULO10CHECK ) );
        s_info.append( createInfo( "b50", "LOGMARS", TBARCODE, TBARCODEADV | MODULOALLCHECK ) );
        s_info.append( createInfo( "b55", "PDF417 - 2D bar code", TBARCODE, BARCODE2D | PDF417BARCODE ) );
        s_info.append( createInfo( "b56", "PDF417 Truncated - 2D bar code", TBARCODE, BARCODE2D | PDF417BARCODE ) );
        s_info.append( createInfo( "b57", "MaxiCode - 2D-bar code (Postscript only)", TBARCODE, BARCODE2D ) );
        s_info.append( createInfo( "b58", "QR-Code", TBARCODE, BARCODE2D ) );
        s_info.append( createInfo( "b59", "Code128 (CharSet A)", TBARCODE, TBARCODEADV | CODE128CHECK | MODULOALLCHECK ) );
        s_info.append( createInfo( "b60", "Code128 (CharSet B)", TBARCODE, TBARCODEADV | CODE128CHECK | MODULOALLCHECK ) );
        s_info.append( createInfo( "b61", "Code128 (CharSet C)", TBARCODE, TBARCODEADV | CODE128CHECK | MODULOALLCHECK ) );
        s_info.append( createInfo( "b62", "Code 93 Ascii", TBARCODE, TBARCODEADV | MODULOALLCHECK ) );
        s_info.append( createInfo( "b63", "Australian Post Standard Customer", TBARCODE, NOCUT | TBARCODEADV ) );
        s_info.append( createInfo( "b64", "Australian Post Customer 2", TBARCODE, NOCUT | TBARCODEADV ) );
        s_info.append( createInfo( "b65", "Australian Post Customer 3", TBARCODE, NOCUT | TBARCODEADV ) );
        s_info.append( createInfo( "b66", "Australian Post Reply Paid", TBARCODE, NOCUT | TBARCODEADV ) );
        s_info.append( createInfo( "b67", "Australian Post Routing", TBARCODE, NOCUT | TBARCODEADV ) );
        s_info.append( createInfo( "b68", "Australian Post Redirection", TBARCODE, NOCUT | TBARCODEADV ) );
        s_info.append( createInfo( "b69", "ISBN Code (=EAN13P5)", TBARCODE, TBARCODEADV ) );
        s_info.append( createInfo( "b70", "Royal Mail 4 State (RM4SCC)", TBARCODE, NOCUT | TBARCODEADV ) );
        s_info.append( createInfo( "b71", "Data Matrix", TBARCODE, DATAMATRIX | BARCODE2D | NOSCALE ) );
    }    
 
    if( s_haveTBarcode2 ) 
    {
        s_info.append( createInfo( "1", "Code 11", TBARCODE2, TBARCODEADV | MODULO10CHECK ) );
        s_info.append( createInfo( "2", "Code 2 of 5 (Standard)", TBARCODE2, TBARCODEADV | MODULO10CHECK ) );
        s_info.append( createInfo( "3", "Interleaved 2 of 5 Standard", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "4", "Code 2 of 5 IATA", TBARCODE2, TBARCODEADV | MODULO10CHECK ) );
        s_info.append( createInfo( "5", "Code 2 of 5 Matrix", TBARCODE2, TBARCODEADV | MODULO10CHECK ) );
        s_info.append( createInfo( "6", "Code 2 of 5 Data Logic", TBARCODE2, TBARCODEADV | MODULO10CHECK ) );
        s_info.append( createInfo( "7", "Code 2 of 5 Industrial", TBARCODE2, TBARCODEADV | MODULO10CHECK ) );
        s_info.append( createInfo( "8", "Code 3 of 9 (Code 39)", TBARCODE2, TBARCODEADV | MODULOALLCHECK ) );
        s_info.append( createInfo( "9", "Code 3 of 9 (Code 39) ASCII", TBARCODE2, TBARCODEADV | MODULOALLCHECK ) );
        s_info.append( createInfo( "10", "EAN8", TBARCODE2, TBARCODEADV | EAN8CHECK | MODULO10CHECK ) );
        s_info.append( createInfo( "11", "EAN8 - 2 digits add on", TBARCODE2, NOCUT | TBARCODEADV | EAN8CHECK | MODULO10CHECK ) );
        s_info.append( createInfo( "12", "EAN8 - 5 digits add on", TBARCODE2, NOCUT | TBARCODEADV | EAN8CHECK | MODULO10CHECK ) );
        s_info.append( createInfo( "13", "EAN13", TBARCODE2, TBARCODEADV | EAN13CHECK | MODULO10CHECK ) );
        s_info.append( createInfo( "14", "EAN13 - 2 digits add on", TBARCODE2, NOCUT | TBARCODEADV | EAN13CHECK | MODULO10CHECK ) );
        s_info.append( createInfo( "15", "EAN13 - 5 digits add on", TBARCODE2, NOCUT | TBARCODEADV | EAN13CHECK | MODULO10CHECK ) );
        s_info.append( createInfo( "16", "EAN128", TBARCODE2, TBARCODEADV | MODULOALLCHECK ) );
        s_info.append( createInfo( "17", "UPC 12 Digits", TBARCODE2, TBARCODEADV | MODULOALLCHECK ) );
        s_info.append( createInfo( "18", "CodaBar (2 width)", TBARCODE2, TBARCODEADV | MODULO10CHECK ) );
        //s_info.append( createInfo( "19", "CodaBar (18 widths)", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "20", "Code128", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "21", "Deutsche Post Leitcode", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "22", "Deutsche Post Identcode", TBARCODE2, TBARCODEADV ) );
        //s_info.append( createInfo( "23", "Code 16K", TBARCODE2, TBARCODEADV ) );
        //s_info.append( createInfo( "24", "Code 49", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "25", "Code 93", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "26", "UPC 25 (Identical to UPCA)", TBARCODE2, TBARCODEADV ) );
        //s_info.append( createInfo( "27", "UPCD1", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "28", "Flattermarken", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "29", "RSS-14", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "30", "RSS Limited", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "31", "RSS Expanded", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "32", "Telepen Alpha", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "33", "UCC128 (= EAN128)", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "34", "UPC A", TBARCODE2, TBARCODEADV | TBARCODEADV | UPCACHECK ) );
        s_info.append( createInfo( "35", "UPC A - 2 digit add on", TBARCODE2, TBARCODEADV | UPCACHECK ) );
        s_info.append( createInfo( "36", "UPC A - 5 digit add on", TBARCODE2, TBARCODEADV | UPCACHECK ) );
        s_info.append( createInfo( "37", "UPC E", TBARCODE2, TBARCODEADV | UPCECHECK ) );
        s_info.append( createInfo( "38", "UPC E - 2 digit add on", TBARCODE2, TBARCODEADV | UPCECHECK ) );
        s_info.append( createInfo( "39", "UPC E - 5 digit add on", TBARCODE2, TBARCODEADV | UPCECHECK ) );
        s_info.append( createInfo( "40", "PostNet5", TBARCODE2, NOCUT | TBARCODEADV | POSTNETCHECK ) );
        s_info.append( createInfo( "41", "PostNet6 (5+CD)", TBARCODE2, NOCUT | TBARCODEADV | POSTNETCHECK ) );
        s_info.append( createInfo( "42", "PostNet9 (5+4)", TBARCODE2, NOCUT | TBARCODEADV | POSTNETCHECK ) );
        s_info.append( createInfo( "43", "PostNet10 (5+4+CD)", TBARCODE2, NOCUT | TBARCODEADV | POSTNETCHECK ) );
        s_info.append( createInfo( "44", "PostNet11 (5+4+2)", TBARCODE2, NOCUT | TBARCODEADV | POSTNETCHECK ) );
        s_info.append( createInfo( "45", "PostNet12 (5+4+2+CD)", TBARCODE2, NOCUT | TBARCODEADV | POSTNETCHECK ) );
        s_info.append( createInfo( "46", "Plessey Code", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "47", "MSI Code", TBARCODE2, TBARCODEADV | MODULO10CHECK ) );
        s_info.append( createInfo( "48", "SSCC18", TBARCODE2, TBARCODEADV ) );
        //s_info.append( createInfo( "49", "FIM", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "50", "LOGMARS", TBARCODE2, TBARCODEADV | MODULOALLCHECK ) );
        s_info.append( createInfo( "51", "Pharmacode One-Track", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "52", "Pharmacentralnumber", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "53", "Pharmacode Two-Track", TBARCODE2, TBARCODEADV ) );
        //s_info.append( createInfo( "54", "General Parcel", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "55", "PDF417 - 2D bar code", TBARCODE2, BARCODE2D | PDF417BARCODE ) );
        s_info.append( createInfo( "56", "PDF417 Truncated - 2D bar code", TBARCODE2, BARCODE2D | PDF417BARCODE ) );
        s_info.append( createInfo( "57", "MaxiCode - 2D-bar code (Postscript only)", TBARCODE2, BARCODE2D ) );
        s_info.append( createInfo( "58", "QR-Code", TBARCODE2, BARCODE2D ) );
        s_info.append( createInfo( "59", "Code128 (CharSet A)", TBARCODE2, TBARCODEADV | CODE128CHECK | MODULOALLCHECK ) );
        s_info.append( createInfo( "60", "Code128 (CharSet B)", TBARCODE2, TBARCODEADV | CODE128CHECK | MODULOALLCHECK ) );
        s_info.append( createInfo( "61", "Code128 (CharSet C)", TBARCODE2, TBARCODEADV | CODE128CHECK | MODULOALLCHECK ) );
        s_info.append( createInfo( "62", "Code 93 Ascii", TBARCODE2, TBARCODEADV | MODULOALLCHECK ) );
        s_info.append( createInfo( "63", "Australian Post Standard Customer", TBARCODE2, NOCUT | TBARCODEADV ) );
        s_info.append( createInfo( "64", "Australian Post Customer 2", TBARCODE2, NOCUT | TBARCODEADV ) );
        s_info.append( createInfo( "65", "Australian Post Customer 3", TBARCODE2, NOCUT | TBARCODEADV ) );
        s_info.append( createInfo( "66", "Australian Post Reply Paid", TBARCODE2, NOCUT | TBARCODEADV ) );
        s_info.append( createInfo( "67", "Australian Post Routing", TBARCODE2, NOCUT | TBARCODEADV ) );
        s_info.append( createInfo( "68", "Australian Post Redirection", TBARCODE2, NOCUT | TBARCODEADV ) );
        s_info.append( createInfo( "69", "ISBN Code (=EAN13P5)", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "70", "Royal Mail 4 State (RM4SCC)", TBARCODE2, NOCUT | TBARCODEADV ) );
        s_info.append( createInfo( "71", "Data Matrix", TBARCODE2, DATAMATRIX | BARCODE2D | NOSCALE ) );
        s_info.append( createInfo( "72", "EAN-14", TBARCODE2, TBARCODEADV ) );
        //s_info.append( createInfo( "73", "Codablock-E", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "74", "Codablock-F", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "75", "NVE-18", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "76", "Japanese Postal Code", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "77", "Korean Postal", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "78", "RSS-14 Truncated", TBARCODE2, BARCODE2D ) );
        s_info.append( createInfo( "79", "RSS-14 Stacked", TBARCODE2, BARCODE2D ) );
        s_info.append( createInfo( "80", "RSS-14 Stacked Omnidirektional", TBARCODE2, BARCODE2D ) );
        s_info.append( createInfo( "81", "RSS Expanded Stacked", TBARCODE2, BARCODE2D ) );
        s_info.append( createInfo( "82", "Planet 12 digits", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "83", "Planet 14 digits", TBARCODE2, TBARCODEADV ) );
        s_info.append( createInfo( "84", "MicroPDF417", TBARCODE2, BARCODE2D ) );
    }    

    Barkode::initValidators();
}

void Barkode::initValidators()
{
    QString path;
    const char* rules = "/usr/share/libpostscriptbarcode/rules.xml";
    if( QFile::exists( rules ) )
        path = rules;
    else
        path =  KStandardDirs::locate( "appdata", "rules.xml" );

    QFile xml( path );
    QDomDocument doc;
    QDomElement  root;
    QDomNode     n;
    QString      id;
    QString*     regular;
    QString*     regularNot;

    if( !xml.open( QIODevice::ReadOnly ) )
    {
        qDebug( "Cannot read validation rules from %s\n", qPrintable( path ) );
        return;
    }

    doc.setContent( &xml );
    root = doc.documentElement();
    n    = root.firstChild();

    while( !n.isNull() ) 
    {
        QDomElement e = n.toElement();
        if( !e.isNull() && e.tagName() == "encoder" )
        {
            id = e.attribute( "id" );
            id.prepend( "ps_" );
            regular    = Barkode::validatorFromType( id );
            regularNot = Barkode::validatorNotFromType( id );
            QString nullString1 = QString();
            if( !regular ) {
                regular = & nullString1;
            }
            QString nullString2 = QString();
            if( !regularNot ) {
                regularNot = & nullString2;
            }

            QDomNode child = e.firstChild();
            while( !child.isNull() )
            {
                QDomElement e = child.toElement();
                QString pattern = QString::null;

                if( child.firstChild().isCDATASection() )
                {
                    pattern = child.firstChild().toCDATASection().data().trimmed();
                }

                
                if( !e.isNull() && e.tagName() == "pattern" ) 
                {
                    QString sense = e.attribute( "sense", "true" );
                    if(  sense == "true" )
                        *regular = pattern;
                    else
                        *regularNot = pattern;
                }

                child = child.nextSibling();
            }
        }

        n = n.nextSibling();
    }

    xml.close();
}

