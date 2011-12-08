/***************************************************************************
                          purepostscript.cpp  -  description
                             -------------------
    begin                : Mon Jan 2 2006
    copyright            : (C) 2006 by Dominik Seichter
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

#include "purepostscript.h"
#include "barkode.h"

#include <stdlib.h>

#include <qdom.h>
#include <qfile.h>
#include <QTextStream>

#include <kstandarddirs.h>
#include <ktemporaryfile.h>

#define MAX_LINE_LENGTH 256
#define BEGIN_TEMPLATE "--BEGIN TEMPLATE--"
#define END_TEMPLATE "--END TEMPLATE--"

QString PurePostscriptBarcode::s_path = QString::null;

PurePostscriptOptions::PurePostscriptOptions()
    : BarkodeEngineOptions()
{
    defaults();
}

const BarkodeEngineOptions& PurePostscriptOptions::operator=( const BarkodeEngineOptions& rhs )
{
    const PurePostscriptOptions* ps = (dynamic_cast<const PurePostscriptOptions*>(&rhs));

    m_check = ps->m_check;

    return *this;
}

void PurePostscriptOptions::defaults()
{
    m_check = false;
}

void PurePostscriptOptions::load( const QDomElement* tag )
{
    m_check = (bool)tag->attribute( "pure.check", "0" ).toInt();
}

void PurePostscriptOptions::save( QDomElement* tag )
{
    tag->setAttribute( "pure.check", (int)m_check );
}

PurePostscriptBarcode::PurePostscriptBarcode()
    : PixmapBarcode()
{
    bool append = false;
    QString line;

    if( s_path.isNull() )
    {
        qDebug( "Cannot locate barcode writer in pure postscript." );
        return;
    }

    /*QFile pureFile( s_path );// -!F: delete block
    if( pureFile.open( QIODevice::ReadOnly ) )
    {
        while( pureFile.readLine( line, MAX_LINE_LENGTH ) != -1 )
        {
            if( append ) 
            {
                if( line.contains( END_TEMPLATE ) )
                    break;
                    
                m_program.append( line );
            }

            if( !append && line.contains( BEGIN_TEMPLATE ) )
                append = true;
        }
        pureFile.close();
    }*/
    QFile pureFile( s_path );
    if( pureFile.open( QIODevice::ReadOnly ) )
    {
        QTextStream s( & pureFile );
	QString line = s.readLine(MAX_LINE_LENGTH);
        while (!line.isNull())
        {
            if( append ) 
            {
                if( line.contains( END_TEMPLATE ) )
                    break;
                    
                m_program.append( line );
            }

            if( !append && line.contains( BEGIN_TEMPLATE ) )
            {
                append = true;
            }
            line = s.readLine(MAX_LINE_LENGTH);
        }
        pureFile.close();
    }
}

PurePostscriptBarcode::~PurePostscriptBarcode()
{
}

void PurePostscriptBarcode::init()
{
    if( s_path.isNull() )
    {
        // first look at the default location
        const char* default_barcode = "/usr/share/libpostscriptbarcode/barcode.ps";
        if( QFile::exists( default_barcode ) )
            s_path = default_barcode;
        else
            s_path = KStandardDirs::locate( "data", "kbarcode/barcode.ps" );
    }

    if( !QFile::exists( s_path ) )
        s_path = QString::null;
}

#define START_TOKEN "% --"
#define BEGIN_ENCODER "BEGIN ENCODER "
#define DESCRIPTION "DESC: "
#define EXAMPLE "EXAM: "

void PurePostscriptBarcode::initInfo( TBarcodeInfoList* info )
{
    PurePostscriptBarcode::init();

    /*QFile pureFile( s_path );
    if( pureFile.open( QIODevice::ReadOnly ) )
    {
        QString encoder;
        QString description;
        QString example;
        QString line;
        
        while( pureFile.readLine( line, MAX_LINE_LENGTH ) != -1 )
        {
            *//*
              % --BEGIN ENCODER ean13--
              % --DESC: EAN-13
              % --EXAM: 977147396801
            *//*
            
            if( line.startsWith( START_TOKEN ) ) 
            {
                // remove all whitespaces on the line ending (and '-')
                line = line.trimmed();

                line = line.right( line.length() - QString( START_TOKEN ).length() );
                if( line.startsWith( BEGIN_ENCODER ) ) 
                {
                    encoder = line.right( line.length() - QString( BEGIN_ENCODER ).length() );

                    if( encoder.endsWith( "--" ) )
                        encoder = encoder.left( encoder.length() - 2 );
                }
                else if( line.startsWith( DESCRIPTION ) )
                    description = line.right( line.length() - QString( DESCRIPTION ).length() );
                else if( line.startsWith( EXAMPLE ) )
                {
                    example = line.right( line.length() - QString( EXAMPLE ).length() );

                    // we should have a complete encoder now.
                    info->append( Barkode::createInfo( QString("ps_") + encoder, description, PURE_POSTSCRIPT, PUREADV | COLORED ) );
                }
            }
        }
        pureFile.close();
    }*/
    QFile pureFile( s_path );
    if( pureFile.open( QIODevice::ReadOnly ) )
    {
        QString encoder;
        QString description;
        QString example;
        QTextStream s( & pureFile );
	QString line = s.readLine(MAX_LINE_LENGTH);
        
        while( !line.isNull() )
        {
            /*
              % --BEGIN ENCODER ean13--
              % --DESC: EAN-13
              % --EXAM: 977147396801
            */
            
            if( line.startsWith( START_TOKEN ) ) 
            {
                // remove all whitespaces on the line ending (and '-')
                line = line.trimmed();

                line = line.right( line.length() - QString( START_TOKEN ).length() );
                if( line.startsWith( BEGIN_ENCODER ) ) 
                {
                    encoder = line.right( line.length() - QString( BEGIN_ENCODER ).length() );

                    if( encoder.endsWith( "--" ) )
                        encoder = encoder.left( encoder.length() - 2 );
                }
                else if( line.startsWith( DESCRIPTION ) )
                    description = line.right( line.length() - QString( DESCRIPTION ).length() );
                else if( line.startsWith( EXAMPLE ) )
                {
                    example = line.right( line.length() - QString( EXAMPLE ).length() );

                    // we should have a complete encoder now.
                    /*info->append( Barkode::createInfo( QString("ps_") + encoder, description, PURE_POSTSCRIPT, PUREADV | COLORED ) );*/
                    QString encoderWithPrefix = QString("ps_") + encoder;
                    QByteArray encoderByteArray = encoderWithPrefix.toUtf8();
                    const char* encoderCChar = encoderByteArray.constData();
		    info->append( Barkode::createInfo( encoderCChar, description, PURE_POSTSCRIPT, PUREADV | COLORED, Barkode::internalType(encoderWithPrefix) ) );
                }
            }
            line = s.readLine(MAX_LINE_LENGTH);
        }
        pureFile.close();
    }
    
}

bool PurePostscriptBarcode::hasPurePostscriptBarcode()
{
    return !s_path.isNull();
}

void PurePostscriptBarcode::createProgram( QString & prg )
{
    const PurePostscriptOptions* options = (dynamic_cast<const PurePostscriptOptions*>(barkode->engine()->options()));
    QString type = barkode->type().right( barkode->type().length() - 3 );
    QString opt;
    
    opt.sprintf( "%s %s barcolor=%02X%02X%02X showbackground backgroundcolor=%02X%02X%02X textcolor=%02X%02X%02X", 
                 barkode->textVisible() ? "includetext" : "",
                 options && options->checksum() ? "includecheck" : "",
                 barkode->foreground().red(), barkode->foreground().green(), barkode->foreground().blue(),
                 barkode->background().red(), barkode->background().green(), barkode->background().blue(), 
                 barkode->textColor().red(), barkode->textColor().green(), barkode->textColor().blue()
                 );

    prg = "%!PS-Adobe-2.0 EPSF-2.0\n%%EndComments\n%%EndProlog\n";
    prg += m_program;
    prg += QString("20 20 moveto\n(%1) (%2) %3 barcode\n")
        .arg( barkode->parsedValue() )
        .arg( opt ).arg( type );
}

QRect PurePostscriptBarcode::bbox( const char* postscript, long postscript_size ) 
{
    const char* gs_bbox = "gs -sDEVICE=bbox -sNOPAUSE -q %1 -c showpage quit 2>&1";

    char*   buffer = NULL;
    long    len    = 0;
    QRect   size;

    /*KTemporaryFile psfile( QString::null, ".ps" );*/
    KTemporaryFile * psfile = new KTemporaryFile();
    psfile->setSuffix(".ps");
    /*psfile.file()->write( postscript, postscript_size );
    psfile.file()->close();*/
    psfile->write( postscript, postscript_size );
    psfile->close();

    /*if( !readFromPipe( QString( gs_bbox ).arg( psfile.file()->name() ).toLatin1(), &buffer, &len ) || !len )*/
    if( !readFromPipe( QString( gs_bbox ).arg( psfile->name() ).toLatin1(), &buffer, &len ) || !len )
    {
        /*psfile.unlink();*/
	psfile->close();
        return QRect( 0, 0, 0, 0 );
    }
    else
        /*psfile.unlink();*/
	psfile->close();

    size = PixmapBarcode::bbox( buffer, len );
    free( buffer );

    return size;
}

bool PurePostscriptBarcode::createPostscript( char** postscript, long* postscript_size )
{
    QString cmd;

    if( m_program.isEmpty() )
        return false;

    createProgram( cmd );

    *postscript_size = cmd.length();
    *postscript = (char*)malloc( sizeof(char) * *postscript_size );
    if( !*postscript ) 
        return false;

    memcpy( *postscript, cmd.toLatin1(), *postscript_size );

    return true;
}
