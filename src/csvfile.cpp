/***************************************************************************
                          csvfile.h  -  description
                             -------------------
    begin                : Mon Mar 28 2005
    copyright            : (C) 2005 by Dominik Seichter
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

#include "csvfile.h"

#include "printersettings.h"

#include <qbuffer.h>
#include <qtextcodec.h>
#include <QDebug>
#include <QFile>
#include <QTextStream>

CSVFile::CSVFile( const QString & filename )
{
    m_eof = false;
    m_csv = true;

    m_quote = PrinterSettings::getInstance()->getData()->quote;
    m_separator = PrinterSettings::getInstance()->getData()->separator;
    m_comment = PrinterSettings::getInstance()->getData()->comment;
    
    m_file.setFileName( filename );
    m_file.open( QIODevice::ReadOnly );

    if( m_file.isOpen() ) {
        m_stream.setDevice( &m_file );
    }
}

CSVFile::CSVFile( QBuffer & buf )
{
    m_eof = false;
    m_csv = true;

    m_quote = PrinterSettings::getInstance()->getData()->quote;
    m_separator = PrinterSettings::getInstance()->getData()->separator;
    m_comment = PrinterSettings::getInstance()->getData()->comment;

    buf.open( QIODevice::ReadOnly );

    if( buf.isOpen() )
	m_stream.setDevice( &buf );
}

CSVFile::~CSVFile()
{
    if( m_stream.device() && m_stream.device()->isOpen() )
	m_stream.device()->close();
}

QStringList CSVFile::readNextLine()
{
    QString line;

    if( !m_stream.device() )
	return QStringList();

    if( !m_stream.device()->isOpen() )
	return QStringList();

    // walk through the lines until a line containing valid data
    for( ;; )
    {
	line = m_stream.readLine();
	line = line.trimmed();

	// check for eof
	if( line.isNull() )
	{
	    m_eof = true;
	    return QStringList();
	}
	
	// ignore comments and empty lines
	if( (!m_comment.isEmpty() && line.startsWith( m_comment )) || line.isEmpty() ) 
	    continue;

	break;
    }

    return m_csv ? readCsvLine( line ) : readFixedLine( line );
}

QStringList CSVFile::readCsvLine( const QString & l )
{
    QString     line( l );
    QStringList sections;
    unsigned int start   = 0;
    unsigned int end     = 0;
    unsigned int len;
    bool quoted          = false;
    bool quote_empty;

    // if line does not end with separator, add one
    if( !line.endsWith( m_separator ) )
	line.append( m_separator );

    // we have to handle here the case, 
    // that the separator is included 
    // in a quoted field
    len = line.length(); // cache for better speed
    quote_empty = m_quote.isEmpty();
    while( end < len )
    {
        if( !quote_empty && line.right( len - end ).startsWith( m_quote ) ) 
            quoted = !quoted;
        else if( !quoted && line.right( len - end ).startsWith( m_separator ) ) 
        {
            sections << removeQuote( line.mid( start, end-start ) );

            start = end;
            ++start;
        }
        else if( line[end] == '\n' )
            break;
        
        ++end;
    }

    return sections;
}

QStringList CSVFile::readFixedLine( const QString & line )
{
    QString     data( line );
    QStringList list;

    for( int i=0;i<m_width.count();i++ )
    {
        list << data.left( m_width[i] );
        data = data.right( data.length() - m_width[i] );
    }

    return list;
}

QString CSVFile::removeQuote( const QString & text ) 
{
    QString line = text.trimmed();

    if( m_quote.isEmpty() )
        return text;

    if( line.startsWith( m_quote ) )
	line = line.right( line.length() - m_quote.length() );

    if( line.endsWith( m_quote ) )
	line = line.left( line.length() - m_quote.length() );

    return line;
}

void CSVFile::setEncoding( const QString & enc )
{
    m_stream.setCodec( QTextCodec::codecForName( enc.toLatin1() ) );
}

