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

#ifndef CSVFILE_H
#define CSVFILE_H

#include <qfile.h>
#include <qstringlist.h>
#include <QTextStream>
#include <QList>

class QBuffer;

/** This class makes it very easy to parse a comma separated value file
 */
class CSVFile {
    public:
       CSVFile( const QString & filename );
       CSVFile( QBuffer & buf );
       ~CSVFile();

       /** reads the next line from the CSV file and returns 
	 * the line split into sections. Comments are ignored by this
	 * line. 
	 */
       QStringList readNextLine();

       /** returns true when the CSVFile object is valid.
	*  returns false incase of an error
	*/
       inline bool isValid() const;

       /** returns true when the end of file was reached
	*/
       inline bool isEof() const;

       /** set the encoding to use for the data
        */
       void setEncoding( const QString & enc );

       /** set the separator to use for the data
        */
       inline void setSeparator( const QString & sep );

       /** set the quoting character
        */
       inline void setQuote( const QString & quote );

       /** set the comment character
        */
       inline void setComment( const QString & comment );

       /** set the field widths for fixed field width files
        */
       inline void setFieldWidth( const QList<int> & width );

       /** sets wether this is a CSV file or 
        *  a file with fixed field width.
        *  \param b if true this is a CSV file (default)
        */
       inline void setCSVFile( bool b );

       /**
        * \returns true if this is a CSV file
        */
       inline bool isCSVFile() const;

    private:
       /** removes quoting characters as defined in lpdata from the 
	* string @p text
	*/
       QString removeQuote( const QString & text );

       QStringList readCsvLine( const QString & line );

       QStringList readFixedLine( const QString & line );

    private:
       /** the filehandle which is used to access the file
	*/
       QFile       m_file;
       QTextStream m_stream;

       QString m_quote;
       QString m_separator;
       QString m_comment;

       QList<int> m_width;

       bool m_csv;
       bool m_eof;
};

bool CSVFile::isValid() const
{
    if( !m_stream.device() )
	return false;

    return m_stream.device()->isOpen();
}

bool CSVFile::isEof() const
{
    return m_eof;
}

void CSVFile::setCSVFile( bool b ) 
{
    m_csv = b;
}

bool CSVFile::isCSVFile() const 
{
    return m_csv;
}

void CSVFile::setSeparator( const QString & sep )
{
    m_separator = sep;
}

void CSVFile::setQuote( const QString & quote )
{
    m_quote = quote;
}

void CSVFile::setComment( const QString & comment )
{
    m_comment = comment;
}

void CSVFile::setFieldWidth( const QList<int> & width )
{
    m_width = width;
}


#endif // CSVFILE_H
