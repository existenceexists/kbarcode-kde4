/***************************************************************************
                         pixmapbarcode.cpp  -  description
                             -------------------
    begin                : Mon Nov 22 2004
    copyright            : (C) 2004 by Dominik Seichter
    email                : domseichter@web.de
 ***************************************************************************/

/***************************************************************************
                                                                          
    This program is free software; you can redistribute it and/or modify  
    it under the terms of the GNU General Public License as published by  
    the Free Software Foundation; either version 2 of the License, or     
    (at your option) any later version.                                   
                                                                          
 ***************************************************************************/

#include "pixmapbarcode.h"
#include "barkode.h"

// patch from Ali Akcaagac says that this include is needed
#include <stdlib.h>

#include <kapplication.h>
#include <KProcess>
#include <kshell.h>
#include <ktemporaryfile.h>

#include <qbuffer.h>
#include <qdom.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qpainter.h>
#include <QPaintDevice>
#include <qpixmap.h>
#include <QDebug>
#include <QByteArray>
#include <QDesktopWidget>

/* Margin added by GNU Barcode to the barcodes */
#define BARCODE_MARGIN 10     

/* Use a 5KB buffer for pipes */
#define BUFFER_SIZE 1024 * 5 

PDF417Options::PDF417Options()
{
    defaults();
}

const PDF417Options& PDF417Options::operator=( const BarkodeEngineOptions& rhs )
{
    const PDF417Options* pdf = (dynamic_cast<const PDF417Options*>(&rhs));

    this->m_row = pdf->m_row;
    this->m_col = pdf->m_col;
    this->m_err = pdf->m_err;

    return *this;
}

void PDF417Options::defaults()
{
    m_row = 24;
    m_col = 8;
    m_err = 5;
}

void PDF417Options::load( const QDomElement* tag )
{
    m_row = tag->attribute( "pdf417.row", "24" ).toInt();
    m_col = tag->attribute( "pdf417.col", "8" ).toInt();
    m_err = tag->attribute( "pdf417.err", "5" ).toInt();
}

void PDF417Options::save( QDomElement* tag )
{
    tag->setAttribute( "pdf417.row", m_row );
    tag->setAttribute( "pdf417.col", m_col );
    tag->setAttribute( "pdf417.err", m_err );
}

PixmapBarcode::PixmapBarcode()
 : BarkodeEngine()
{
}


PixmapBarcode::~PixmapBarcode()
{
}

const PixmapBarcode & PixmapBarcode::operator=( const BarkodeEngine & rhs )
{
    const PixmapBarcode* pix = dynamic_cast<const PixmapBarcode*>(&rhs);

     if( pix ) 
     {
         m_pdf417_options = pix->m_pdf417_options;
         p                = pix->p;
     }

     return *this;
}

const QSize PixmapBarcode::size() const
{
    return ( p.size().isNull() ? QSize( 100, 80 ) : p.size() );
}

void PixmapBarcode::update( const QPaintDevice* device )
{
    p = p.copy( 0, 0, 0, 0 );
    createBarcode( &p, device );
}

void PixmapBarcode::drawBarcode( QPainter & painter, int x, int y )
{
    if( p.isNull() ) {
        createBarcode( &p, painter.device() );
    }
    
    if( p.isNull() ) // still no barcode....
    {
        barkode->drawInvalid( painter, x, y );
        return;
    }
        
    painter.drawPixmap( x, y, p );
}

bool PixmapBarcode::createPixmap( QPixmap* target, int resx, int resy )
{
    char* postscript = NULL;
    long postscript_size = 0;
    QString cmd;
    bool bMonocrome;

    bMonocrome = ( barkode->foreground() == Qt::black && 
                   barkode->background() == Qt::white &&
                   barkode->textColor() == Qt::black );

    KTemporaryFile* input = new KTemporaryFile();
    input->setSuffix(bMonocrome ? ".pbm" : ".ppm");
    input->open();

    if( Barkode::engineForType( barkode->type() ) == PDF417 ) {
        if(!createPdf417( input )) {
            cleanUp( input, target );
            return false;
        }

	target->load( input->fileName(), "GIF" );
    } else { 
        if( !createPostscript( &postscript, &postscript_size ) )
        {
            cleanUp( input, target );
            return false;
        }

	FILE* gs_pipe;

	if( !postscript_size )
	{
	    // GNU Barcode was not able to encode this barcode
	    cleanUp( input, target );
	    return false;
	}

        QRect size = bbox( postscript, postscript_size );
        double sw = (double)(size.x() + size.width())/72 * resx;
        double sh = (double)(size.y() + size.height())/72 * resy;

        if( Barkode::engineForType( barkode->type() ) == TBARCODE )
        {
            sw = (double)(size.x() + size.width());
            sh = (double)(size.y() + size.height());
        }
        
        pixmapBarcodeWidth = int(sw*(double)barkode->scaling());
        pixmapBarcodeHeight = int(sh*(double)barkode->scaling());

	cmd = QString("gs -g%1x%2").arg(pixmapBarcodeWidth).arg(pixmapBarcodeHeight);
	cmd += " -r" + QString::number( resx*(double)barkode->scaling()) + "x" + QString::number( resy*(double)barkode->scaling() );
	cmd += QString(" -sDEVICE=%1 -sOutputFile=").arg( bMonocrome ? "pbmraw" : "ppm" );
        cmd += input->fileName();
	cmd += " -sNOPAUSE -q - -c showpage quit";
	
	qDebug() << "cmd: " + cmd;
	gs_pipe = popen( cmd.toLatin1(), "w" );
        if( !gs_pipe )
	{
	    qDebug("ERROR: cannot open Ghostscript pipe!");
            cleanUp( input, target );
	    return false;
	}

	fwrite( postscript, sizeof(char), postscript_size, gs_pipe );
	pclose( gs_pipe );

        target->load( input->fileName(), "PBM" );
    }
        

    free( postscript );

    input->close();
    delete input;

    return true;
}

bool PixmapBarcode::createPostscript( char** postscript, long* postscript_size )
{
    QString cmd;

    /*
    if( Barkode::engineForType( barkode->type() ) == TBARCODE ) 
    {
        cmd = createTBarcodeCmd();
        qDebug("tbarcodeclient commandline: %s", cmd.toLatin1() );
    }
    else // GNU_BARCODE
    */
    {
        cmd = "barcode -E -b ";
	cmd += KShell::quoteArg( barkode->parsedValue() ) + (barkode->textVisible() ? "" : " -n");
        cmd += " -e " + barkode->type();
    }
    
    if( !readFromPipe( cmd.toLatin1(), postscript, postscript_size ) )
        return false;

    return true;
}

QRect PixmapBarcode::bbox( const char* data, long size )  
{
    int x = 0, y = 0, w = 0, h = 0;
    const char* bbox = "%%BoundingBox:";
    int len = strlen( bbox );

    QRect s(0,0,0,0);
    QByteArray array;
    array.setRawData( data, size );
    
    QBuffer b( & array );
    if( !b.open( QIODevice::ReadOnly ) )
        return s;
    
    QTextStream t( &b );
        
    QString text = t.readLine();
    while( !text.isNull() )
    {
        if( text.startsWith( bbox ) )
        {
            text = text.right( text.length() - len );
            QByteArray textByteArray = text.toUtf8();
            const char* cString = textByteArray.constData();
            sscanf( cString, "%d %d %d %d", &x, &y, &w, &h );
            s = QRect( x, y, w, h );
            break;
        }

	text = t.readLine();
    }

    b.close();
    array.clear();
    array.setRawData( data, size );

    return s;
}

bool PixmapBarcode::readFromPipe( const char* command, char** buffer, long* buffer_size )
{
    FILE* pipe = popen( command, "r" );
    if( !pipe )
    {
        qDebug("ERROR: cannot open pipe %s!", command );
        return false;
    }

    char* buf = (char*)malloc( BUFFER_SIZE );
    char* tmp = NULL;
    int s     = 0;

    *buffer_size = 0;
    do {
        s = fread( buf, sizeof(char), BUFFER_SIZE, pipe );

        // Special case:
        // GNU Barcode Error
        if( !s ) 
            break;

        // there won't be more data to read
        tmp = (char*)malloc( *buffer_size + s );
        
        if( *buffer ) 
        {
            memcpy( tmp, *buffer, *buffer_size );
            free( *buffer );
        }
        
        memcpy( tmp+ *buffer_size, buf, s );
        *buffer = tmp;
        *buffer_size += s;
    } while( s == BUFFER_SIZE );
    
    pclose( pipe );
    free( buf );

    return true;
}

void PixmapBarcode::createBarcode( QPixmap* target, const QPaintDevice* device )
{
    int resx = device->logicalDpiX();
    int resy = device->logicalDpiY();
    
    pixmapBarcodeWidth = 0;
    pixmapBarcodeHeight = 0;

    QPixmap* cached = 0;//BarcodeCache::instance()->read( barcode, resx, resy, value );

    // no matching barcode found in cache
    if( !cached ) {
        if( !createPixmap( target, resx, resy ) ) {
            return;
        }
    } else {
        *target = *cached;
        delete cached;
    }
    
    if( Barkode::hasFeature( barkode->type(), PDF417 ) ) {
        // we have to scale to the correct resolution.
        // we scale already here and not at the end,
        // so that the addMargin function does not get a scaled margin.
        int screenresx = KApplication::desktop()->logicalDpiX();
        int screenresy = KApplication::desktop()->logicalDpiY();
    
        QMatrix m;
        double scalex = (resx/screenresx)*barkode->scaling();
        double scaley = (resy/screenresy)*barkode->scaling();
        m.scale( scalex, scaley );
        *target = target->transformed( m );
    }
    *target = cut( target, barkode->cut() );
    *target = addMargin( target );

    // Rotate
    QMatrix m;
    m.rotate( (double)barkode->rotation() );
    *target = target->transformed( m );

    //barcode.valid = true;
}

bool PixmapBarcode::createPdf417( KTemporaryFile* output )
{
    const PDF417Options* options = (dynamic_cast<const PDF417Options*>(barkode->engine()->options()));

    if( !options ) 
    {
        qDebug("No PDF417Options available!");
        return false;
    }

    KTemporaryFile textFile;
    KTemporaryFile * text = & textFile;
    text->setSuffix(".txt");
    QTextStream t( text );
    t << barkode->parsedValue();
    text->close();

    // ps does not work because bounding box information is missing
    // pbm cannot be loaded by KImgIO (works fine in GIMP)
    // gif is the only other option
    KProcess proc;
    QString cmd;
    cmd += "pdf417_enc";
    cmd += " ";
    cmd += "-tgif";
    cmd += " ";
    cmd += text->fileName();
    cmd += " ";
    cmd += output->fileName();
    cmd += " ";
    cmd += options->row();
    cmd += " ";
    cmd += options->col();
    cmd += " ";
    cmd += options->err();
    proc.setShellCommand(cmd);
         
    proc.start();

    if( proc.exitStatus() ) {
	text->close();
        return false;
    }

    text->close();
    return true;    
}

#if 0
QString PixmapBarcode::createTBarcodeCmd()
{
    QString cmd;

    // print text
    QString flag = barkode->textVisible() ? " Ton" : " n Toff"; // we pass the old parameter Ton and the new one: n
    // escape text
    flag.append( barkode->tbarcodeOptions()->escape() ? " son" : " soff" );
    // autocorrection
    flag.append( barkode->tbarcodeOptions()->autocorrect() ? " Aon" : " Aoff" );
    // barcode height
    flag.append( QString( " h%1" ).arg( barkode->tbarcodeOptions()->height() ) );
    // text above
    if( barkode->tbarcodeOptions()->above() )
        flag.append( " a" );
    
    cmd = "tbarcodeclient "; 
    if( !Barkode::hasFeature( barkode->type(), BARCODE2D ) )
        cmd += QString( " m%1" ).arg( barkode->tbarcodeOptions()->moduleWidth() * 1000 );

    if( Barkode::hasFeature( barkode->type(), DATAMATRIX ) ) 
        cmd += QString( " Ds%1" ).arg( barkode->datamatrixSize() );

    if( Barkode::hasFeature( barkode->type(), PDF417BARCODE ) )
        cmd += QString( " Pr%1 Pc%2 Pe%3" ).arg( barkode->pdf417Options()->row() )
                                           .arg( barkode->pdf417Options()->col() )
                                           .arg( barkode->pdf417Options()->err() );
        
    cmd += " " + barkode->type() + QString(" tPS c%1").arg( barkode->tbarcodeOptions()->checksum() );
    cmd += flag + " d" + KShellProcess::quote(  barkode->parsedValue() );

    return cmd;
}
#endif // 0

void PixmapBarcode::cleanUp( KTemporaryFile* file, QPixmap* target )
{
    QPixmap targetCopy = target->copy(0, 0, 0, 0);
    target = & targetCopy;

    file->close();
    delete file;
}

QPixmap PixmapBarcode::cut( QPixmap* pic, double cut)
{
    if( cut == 1.0 )
        return (*pic);

    QPixmap pcut( pic->width(), int((double)pic->height() * cut) );
    pcut.fill( Qt::white ); // barcode.bg

    QMatrix m;
    /*
     * if text is above the barcode cut from
     * below the barcode.
     */

    // TODO: put this into one if, I am to stupid today.....
    if( Barkode::hasFeature( barkode->type(), TBARCODEADV ) ) {
        //if( !barcode.tbarcode.above )
            m.rotate( 180 );
    } else
        m.rotate( 180 );

    QPainter painter( &pcut );
    painter.drawPixmap( 0, 0, pic->transformed( m ) );

    return pcut.transformed( m );
}

QPixmap PixmapBarcode::addMargin( QPixmap* pic )
{
    /* We have to handle UPC special because of the checksum character
     * which is printed on the right margin.
     * The samve goes for ISBN codes.
     * Any other formats??
     */

    bool gnubarcode = (Barkode::engineForType( barkode->type() ) == GNU_BARCODE) ||
        (Barkode::engineForType( barkode->type() ) == PURE_POSTSCRIPT);
    double barm = gnubarcode ? BARCODE_MARGIN * barkode->scaling() : 0.0;

    // Add margin
    double sx = barm;
    double sy = barm;
    double sw = pic->width()  - (barm * 2);
    double sh = pic->height() - (barm * 2);
    int margin = (int)(barkode->quietZone()*2 - barm*2);

    int w, h = 0;
    if( gnubarcode && (barkode->type() == "upc" || barkode->type() == "isbn") ) 
    {
        sw = pic->width() - barm;

        w = pic->width() + int(barkode->quietZone()*2 - barm);
        h = pic->height() + margin;
    } 
    else {
        w = pic->width() + margin;
        h = pic->height() + margin;
    }
    QPixmap p(w, h);

    p.fill( barkode->background() );
    QPainter painter( &p );
    painter.drawPixmap( barkode->quietZone(), barkode->quietZone(), *pic, (int)sx, (int)sy, (int)sw, (int)sh );
    painter.end();

    return p;
}
