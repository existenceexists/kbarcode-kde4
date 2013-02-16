/***************************************************************************
                       tbarcode2.cpp  -  description
                             -------------------
    begin                : Mon Feb 20 2006
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

#include "tbarcode2.h"
#include "barkode.h"

#include <stdlib.h>

#include <qdom.h>
#include <qfile.h>
#include <QDebug>

#include <ktemporaryfile.h>
#include <kshell.h>

TBarcodeOptions::TBarcodeOptions()
{
    defaults();
}

void TBarcodeOptions::defaults()
{
    m_escape = false;
    m_above = false;
    m_autocorrect = false;
    m_checksum = 0;
    m_modulewidth = 0.353;
    m_height = 20;
}

void TBarcodeOptions::load( const QDomElement* tag )
{
    this->setModuleWidth( tag->attribute( "tbarcode.modulewidth", "0.353" ).toDouble() );
    this->setEscape( tag->attribute( "tbarcode.escape", "0" ).toInt() );
    this->setAbove( tag->attribute( "tbarcode.above", "0" ).toInt() );
    this->setAutocorrect( tag->attribute ( "tbarcode.autocorrect", "0" ).toInt() );
    this->setCheckSum( tag->attribute( "tbarcode.checksum", "0" ).toInt() );
    this->setHeight( tag->attribute( "tbarcode.height", "0" ).toInt() );
}

void TBarcodeOptions::save( QDomElement* tag )
{
    tag->setAttribute( "tbarcode.modulewidth", this->moduleWidth() );
    tag->setAttribute( "tbarcode.escape", this->escape() );
    tag->setAttribute( "tbarcode.above", this->above() );
    tag->setAttribute( "tbarcode.autocorrect", this->autocorrect() );
    tag->setAttribute( "tbarcode.checksum", this->checksum() );
    tag->setAttribute( "tbarcode.height", this->height() );
}

const TBarcodeOptions& TBarcodeOptions::operator=( const BarkodeEngineOptions& rhs )
{
    const TBarcodeOptions* tbarcode = (dynamic_cast<const TBarcodeOptions*>(&rhs));

    this->m_escape      = tbarcode->m_escape;
    this->m_above       = tbarcode->m_above;
    this->m_autocorrect = tbarcode->m_autocorrect;
    this->m_checksum    = tbarcode->m_checksum;
    this->m_modulewidth = tbarcode->m_modulewidth;
    this->m_height      = tbarcode->m_height;

    return *this;
}

TBarcode2::TBarcode2()
    : PixmapBarcode()
{

}

TBarcode2::~TBarcode2()
{

}

const TBarcode2 & TBarcode2::operator=( const BarkodeEngine & rhs )
{
    const TBarcode2* barcode = dynamic_cast<const TBarcode2*>(&rhs);

    if( barcode ) 
    {
        m_options = barcode->m_options;
    }

    return *this;
}

bool TBarcode2::createPostscript( char** postscript, long* postscript_size )
{
    QString cmd;
    const char* text;

    const TBarcodeOptions* tbarcode = (dynamic_cast<TBarcodeOptions*>(barkode->engine()->options()));
    if( !tbarcode )
        return false;

    if( !barkode->textVisible() )
        text = "hide";
    else if( tbarcode->above() )
        text = "above";
    else
        text = "below";

    QByteArray barkodeTypeByteArray = barkode->type().toLatin1();
    const char* barkodeTypeChar = barkodeTypeByteArray.constData();
    QByteArray barkodeParsedValueByteArray = KShell::quoteArg(barkode->parsedValue()).toLatin1();
    const char* barkodeParsedValueChar = barkodeParsedValueByteArray.constData();
    cmd = cmd.sprintf( "tbarcode -f PS -b %s -d %s  -t %s --translation=%s --autocorrect=%s --modulewidth=%.3f -h %i --checkdigit=%i --72dpiraster\n", 
		       barkodeTypeChar,
		       barkodeParsedValueChar,
                       text, tbarcode->escape() ? "on" : "off", 
                       tbarcode->autocorrect() ? "on" : "off", 
                       tbarcode->moduleWidth(), 
                       tbarcode->height(),
                       tbarcode->checksum() );

    qDebug() << "Cmd = " + cmd;
    if( !readFromPipe( cmd.toLatin1(), postscript, postscript_size ) )
        return false;

    return true;
}

QRect TBarcode2::bbox( const char* postscript, long postscript_size )
{
    const char* gs_bbox = "gs -sDEVICE=bbox -sNOPAUSE -q %1 -c showpage quit 2>&1";

    char*   buffer = NULL;
    long    len    = 0;
    QRect   size;

    KTemporaryFile psfileF;
    KTemporaryFile * psfile = & psfileF;
    psfile->setSuffix(".ps");
    psfile->open();
    psfile->write( postscript, postscript_size );
    psfile->flush();
    
    QString gs_bbox_string = QString( gs_bbox ).arg( psfile->fileName() );
    QByteArray ba = gs_bbox_string.toLatin1();
    const char* gs_bbox_cmd = ba.constData();

    if( !readFromPipe( gs_bbox_cmd, &buffer, &len ) || !len )
    {
	psfile->close();
        return QRect( 0, 0, 0, 0 );
    }
    else
	psfile->close();

    size = PixmapBarcode::bbox( buffer, len );
    free( buffer );

    return size;

}

