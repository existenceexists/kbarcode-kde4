/***************************************************************************
                          definition.cpp  -  description
                             -------------------
    begin                : Mit Nov 20 2002
    copyright            : (C) 2002 by Dominik Seichter
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

#include "definition.h"
#include "sqltables.h"

// Qt includes
#include <QTextStream>
#include <qsqlquery.h>
#include <qregexp.h>

// KDE includes
#include <kapplication.h>
#include <kconfiggroup.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <q3progressdialog.h>
#include <kglobal.h>

// a simple helper function
// that copies a file
bool filecopy( const char* src, const char* dest )
{
    FILE* s;
    FILE* d;
    int c;

    d = fopen(dest, "w");
    if( d == NULL )
        return false;

    s = fopen(src, "r");
    if( s == NULL ) {
        fclose( d );
        remove( dest );
        return false;
    }

    while(( c = getc( s )) != EOF )
        putc( c, d );

    fclose( s );
    fclose( d );
    return true;
}

class PrivateParser {
    public:
        PrivateParser( QString line, bool all = true );
        ~PrivateParser();

        const QString & getId() const { return m_label_def_id; }
        const QString & getProducer() const { return m_producer; }
        const QString & getType() const { return m_type; }
        const Measurements & getMeasurements() const { return m_measure; };

    private:
        QString removeQuote( const QString & q );

        QString m_label_def_id;
        QString m_producer;
        QString m_type;

        Measurements m_measure;
};

PrivateParser::PrivateParser( QString line, bool all )
{
    line = line.trimmed();
    int pos = line.find("(");
    line = line.mid( pos + 1, line.length() - pos - 1 );

    m_label_def_id = line.section( ",", 0, 0 ).trimmed();
    m_producer     = removeQuote( line.section( ",", 1, 1 ) );
    m_type         = removeQuote( line.section( ",", 2, 2 ) );

    if( all ) {
        m_measure.setGapTopMM( line.section( ",", 4, 4 ).toDouble() );
        m_measure.setGapLeftMM( line.section( ",", 5, 5 ).toDouble() );
        m_measure.setHeightMM( line.section( ",", 6, 6 ).toDouble() );
        m_measure.setWidthMM( line.section( ",", 7, 7 ).toDouble() );
        m_measure.setGapVMM( line.section( ",", 8, 8 ).toDouble() );
        m_measure.setGapHMM( line.section( ",", 9, 9 ).toDouble() );
        m_measure.setNumH( line.section( ",", 10, 10 ).toInt() );
        m_measure.setNumV( line.section( ",", 11, 11 ).toInt() );

        // fix broken label definitions
        // with numh and numv = 0
        if( !m_measure.numH() )
            m_measure.setNumH( 1 );

        if( !m_measure.numV() )
            m_measure.setNumV( 1 );
    }
}

PrivateParser::~PrivateParser()
{ }

QString PrivateParser::removeQuote( const QString & q )
{
    QString quote = q.trimmed();

    if( quote.startsWith("'") )
        quote = quote.mid( 1, quote.length() - 1 );

    if( quote.endsWith("'") )
        quote = quote.left( quote.length() - 1 );

    return quote;
}


/***************************************************************************/

Definition::Definition( QWidget* parent )
    : m_parent( parent )
{
    id = -1;
}

Definition::Definition( int label_def_id, QWidget* parent )
    : m_parent( parent )
{
    init( QString("%1").arg( label_def_id ) );
}

Definition::Definition( const QString & label_def_id, QWidget* parent )
    : m_parent( parent )
{
    init( label_def_id );
}

Definition::Definition( const QString & producer, const QString & type, QWidget* parent )
    : m_parent( parent )
{
    if( SqlTables::isConnected() ) {
        QSqlQuery query(
            "select label_no from " TABLE_LABEL_DEF " WHERE manufacture='" + producer + "' AND type='" + type + "'");
        while( query.next() )
            init( query.value( 0 ).toString() );
    } else {
        if(!openFile())
            return;

	initProgress();

        QString s;
        while( file->readLine( s, 1000 ) != -1 ) {
            if( s.isEmpty() || s.left( 1 ) == "#" )
                continue;

	    increaseProgress();

            PrivateParser p( s );
            if( p.getProducer() ==producer && p.getType() == type ) {
                init( p.getId() );
                break;
            }
        }

	destroyProgress();
    }

}

Definition::~Definition()
{
}

QFile* Definition::file = 0;
QByteArray* Definition::array = 0;
QStringList* Definition::listProducers = 0;
QMap<QString,QStringList> Definition::mapTypes;
QProgressDialog* Definition::m_progress = 0;

void Definition::initProgress()
{
/*
    if(!m_progress)
	m_progress = new QProgressDialog( i18n("Loading Label Definitions..."), QString::null, 0, NULL, "m_progress", true );

    m_progress->setTotalSteps( 1000 );
    m_progress->show();
*/
//    m_progress->reparent( m_parent, m_progress->pos(), true );
}

void Definition::increaseProgress()
{
/*
    if( m_progress )
	m_progress->setProgress( m_progress->progress() + 1 );
*/
}

void Definition::destroyProgress()
{
/*
    if( m_progress )
	delete m_progress;
    m_progress = NULL;
*/
}

void Definition::setId( const QString & label_def_id )
{
    init( label_def_id );
}

void Definition::setId( int label_def_id )
{
    init( QString("%1").arg( label_def_id ) );
}

void Definition::init( const QString & label_def_id )
{
    if( SqlTables::isConnected() ) {
        QSqlQuery* query = new QSqlQuery(
            "select number_h, number_v, gap_left, gap_top, gap_v, gap_h, width, height, manufacture, type from " TABLE_LABEL_DEF
            " WHERE label_no = " + label_def_id );

        while( query->next() ) {
            m_measure.setNumH( query->value( 0 ).toInt() );
            m_measure.setNumV( query->value( 1 ).toInt() );
            m_measure.setGapLeftMM( query->value( 2 ).toDouble() );
            m_measure.setGapTopMM( query->value( 3 ).toDouble() );
            m_measure.setGapVMM( query->value( 4 ).toDouble() );
            m_measure.setGapHMM( query->value( 5 ).toDouble() );
            m_measure.setWidthMM( query->value( 6 ).toDouble() );
            m_measure.setHeightMM( query->value( 7 ).toDouble() );
            producer = query->value( 8 ).toString();
            type = query->value( 9 ).toString();
        }
    } else {
        getFileMeasurements( label_def_id );
    }

    id = label_def_id.toInt();

}

const Measurements & Definition::getMeasurements() const
{
    return m_measure;
}

void Definition::getFileMeasurements( const QString & label_def_id )
{
    if(!openFile()) {
        m_measure = Measurements();
        return;
    }

    initProgress();

    QTextStream stream(*array, QIODevice::ReadOnly );
    while( !stream.atEnd() ) {
        QString s = stream.readLine();
        if( s.isEmpty() || s.startsWith( "#" ) )
            continue;

	increaseProgress();

        PrivateParser p( s );
        if( p.getId() != label_def_id )
            continue;

        producer  = p.getProducer();
        type      = p.getType();
        m_measure = p.getMeasurements();
        break;
    }

    destroyProgress();
}

bool Definition::openFile()
{
    if( file ) {
        file->at( 0 );
        return true;        
    }

    QString f = KStandardDirs::locateLocal( "data", "kbarcode/labeldefinitions.sql" );
    if( !QFile::exists( f ) ) {
        KConfigGroup config = KGlobal::config()->group( "Definitions" );
        
        // copy file to new location
        QString fname = config.readEntry( "defpath", locate( "data", "kbarcode/labeldefinitions.sql" ) );
        if( !QFile::exists( fname ) || fname.isEmpty() ) 
            return ( showFileError() ? openFile() : false );

        if(!filecopy( (const char*)fname, (const char*)f ))
            return ( showFileError() ? openFile() : false );
    }

    file = new QFile( f );
    if( !file->open( QIODevice::ReadOnly ) ) {
        delete file;
        file = 0;
        return ( showFileError() ? openFile() : false );
    }

    // keeping this array around
    // increases speed quite a lot
    // but does also cost lot's of memory
    array = new QByteArray();
    *array = file->readAll();
    file->at( 0 );

    return true;
}

const QStringList Definition::getProducers()
{
    if( listProducers )
        return *listProducers;

    listProducers = new QStringList();

    if( SqlTables::isConnected() ) {
        QSqlQuery query("SELECT manufacture FROM " TABLE_LABEL_DEF " GROUP by manufacture;");
        while( query.next() )
            listProducers->append( query.value( 0 ).toString() );
    } else {
        if(!openFile() )
            return *listProducers;

	initProgress();

        QTextStream stream(*array, QIODevice::ReadOnly );
        while( !stream.atEnd() ) {
            QString s = stream.readLine();
            if( s.isEmpty() || s.startsWith( "#" ) )
                continue;

	    increaseProgress();

            PrivateParser p( s, false );
            if( !listProducers->contains( p.getProducer() ) )
                listProducers->append( p.getProducer() );
        }

	destroyProgress();
    }

    return *listProducers;
}

const QStringList Definition::getTypes( QString producer )
{
    if( mapTypes.contains( producer ) ) {
        return mapTypes[producer];
    }

    QStringList list;

    if( SqlTables::isConnected() ) {
        QSqlQuery query("SELECT type FROM " TABLE_LABEL_DEF " WHERE manufacture='" + producer + "'" );
        while( query.next() )
            if( !list.contains( query.value( 0 ).toString() ) )
                    list.append( query.value( 0 ).toString() );
    } else {
        if(!openFile())
            return list;

	initProgress();

        QTextStream stream(*array, QIODevice::ReadOnly );
        while( !stream.atEnd() ) {
            QString s = stream.readLine();
            if( s.isEmpty() || s.startsWith( "#" ) )
                continue;

	    increaseProgress();

            PrivateParser p( s, false );
            if( p.getProducer() == producer )
                if( !list.contains( p.getType() ) )
                    list.append( p.getType() );
        }

	destroyProgress();
    }

    mapTypes.insert( producer, list );

    return list;
}

void Definition::updateProducer()
{
    // TODO: check wether this function is
    // correct! w/ SQL and without
    if( listProducers ) {
        delete listProducers;
        listProducers = 0;

        if( array ) {
            delete array;
            array = 0;
        }

        if( file ) {
            file->close();
            delete file;
            file = 0;
        }
        mapTypes.clear();
    }
}

int Definition::write( const Measurements & c, QString type, QString producer )
{
    int r = -1;
    if( SqlTables::isConnected() )
        r = Definition::writeSQL( c, type, producer );
    else
        r = Definition::writeFile( c, type, producer ); 

    Definition::updateProducer();
    return r;
}

int Definition::writeFile( const Measurements & c, QString type, QString producer )
{
    if( !openFile() )
        return -1;
        
    QStringList data;
    bool datawritten = false;
    int index = 0;
    QString entry = ", '" +
                  producer + "', '" + type + "', 'C',"+ I2S(c.gapTopMM()) +
                  ", " + I2S(c.gapLeftMM()) + ", " +
                  I2S(c.heightMM()) + ", " + I2S(c.widthMM()) + ", " +
                  I2S(c.gapVMM()) + ", " + I2S(c.gapHMM()) + ", " +
                  I2S(c.numH()) + ", " + I2S(c.numV()) + ", NULL, NULL )";
        
    QString s;
    while( file->readLine( s, 1000 ) != -1 ) {
        if( s.isEmpty() || s.left( 1 ) == "#" ) {
            data.append( s );
            continue;
        }
            
        PrivateParser p( s );
        if( p.getId().toInt() > index )
            index = p.getId().toInt();

        if( p.getType() == type && p.getProducer() == producer ) {
            // update an item already present in the list
            entry = entry.prepend( "INSERT INTO " TABLE_LABEL_DEF " VALUES (" + I2S(p.getId().toInt()) );
            data.append( entry );
            datawritten = true;
        } else
            data.append( s );
    }

    if( !datawritten ) {
        entry = entry.prepend( "INSERT INTO " TABLE_LABEL_DEF " VALUES (" + I2S(index+1) );
        data.append( entry );
    }
    
    file->close();
    if( !file->open( QIODevice::WriteOnly ) ) {
        file->open( QIODevice::ReadOnly );
        return -1;
    }

    QTextStream t( file );
    for( unsigned int i = 0; i < data.count(); i++ )
        t << data[i].replace( QRegExp("\\n"), "" ) << "\n";

    // get the file back to the normal stage
    file->close();
    file->open( QIODevice::ReadOnly );

    return index + 1;
}

int Definition::writeSQL( const Measurements & c, QString type, QString producer )
{
    bool newitem = true;
    QSqlQuery q( "SELECT manufacture, type FROM " TABLE_LABEL_DEF );
    // TODO: use a more inteligent query using where=
    while( q.next() )
        if( q.value( 0 ) == producer &&
            q.value( 1 ) == type )
                newitem = false;

    if( newitem ) {
        QSqlQuery query(
              "INSERT INTO " TABLE_LABEL_DEF " (manufacture, type, gap_top, gap_left, "
              "width, height, gap_v, gap_h, number_h, number_v) VALUES ('" +
              producer + "', '" + type + "', '"+ I2S( c.gapTopMM() ) +
              "', '" + I2S( c.gapLeftMM() ) + "', '" +
              I2S( c.widthMM() ) + "', '" + I2S( c.heightMM() ) + "', '" +
              I2S( c.gapVMM() ) + "', '" + I2S( c.gapHMM() ) + "', '" +
              I2S( c.numH() ) + "', '" + I2S( c.numV() ) + "')"
              );

        if(!query.isValid())
            qDebug("Query to insert values not valid!");
    } else {
        QSqlQuery query( "UPDATE " TABLE_LABEL_DEF " SET "
            "gap_top = " + I2S( c.gapTopMM() ) + " ,gap_left = " + I2S( c.gapLeftMM() ) +
            " ,width = " + I2S( c.widthMM() ) + " ,height = " + I2S( c.heightMM() ) +
            " ,gap_v = " + I2S( c.gapVMM() ) + " ,gap_h = " + I2S( c.gapHMM() ) +
            " ,number_h = " + I2S( c.numH() ) + " ,number_v = " + I2S( c.numV() ) +
            " WHERE manufacture = '" + producer + "' AND"
            " type = '" + type + "'" );

        if(!query.isValid())
            qDebug("Query to update values not valid!\n%s\n", query.lastQuery().toLatin1() );
    }

    QSqlQuery qi("SELECT label_no FROM " TABLE_LABEL_DEF " WHERE manufacture='" + producer + "' AND type='" + type + "'" );
    while( qi.next() )
        return qi.value( 0 ).toInt();

    return -1;
}


bool Definition::nodefmsg = true;
bool Definition::showFileError()
{
    if( nodefmsg ) {
        KMessageBox::information( 0,
            i18n("KBarcode is unable to find its label definitions."
                 "Please make sure that the file $KDEDIR/share/apps/kbarcode/labeldefinitions.sql "
                 "does exist. This file is part of the KBarcode distribution. "
                 "You will be prompted now to select the file containing the labeldefinitions."),
                 "", "NoDefinitionsFound" );

        QString f = KFileDialog::getOpenFileName( QString::null, QString::null, 0 );
        if( !f.isEmpty() && QFile::exists( f ) ) {
            KConfigGroup config = KGlobal::config()->group( "Definitions" );
            config.writeEntry( "defpath", f );
            config.sync();
        }
        nodefmsg = false;
        return openFile();
    } else
        qDebug("No label definitions found. Please install them.");

    return false;
}

int Definition::getClosest( const QString & producer, const QString & type )
{
    QStringList t = Definition::getTypes(producer); 
    for( unsigned int z = 0; z < t.count(); z++ )  {
        if( t[z] == type ) {
            Definition d( producer, type );
            return d.getId();
        }
    }

    return -1;
}
