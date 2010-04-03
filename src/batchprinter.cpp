/***************************************************************************
                          batchprinter.cpp  -  description
                             -------------------
    begin                : Sat Jan 10 2004
    copyright            : (C) 2004 by Dominik Seichter
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
#include "batchprinter.h"

#include "definition.h"
#include "kbarcode.h"
#include "label.h"
#include "printersettings.h"
#include "zplutils.h"
#include "tec.h"

// Qt includes
#include <qfile.h>
#include <qpainter.h>
#include <QBuffer>
#include <QPaintDevice>
#include <QTextStream>
#include <QProgressDialog>
#include <QList>
#include <QPixmap>

// KDE includes
#include <kabc/addressee.h>
#include <kabc/addresseelist.h>
#include <kapplication.h>
#include <kconfiggroup.h>
#include <kimageio.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qprinter.h>

// Other includes
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <krun.h>
#define MAXDATASIZE 32550

BatchPrinter::BatchPrinter( QPrinter* p, QWidget* _parent )
    : printer( p ), parent( _parent )
{
    m_events = true;
    m_data = NULL;
    m_vardata = NULL;
    m_addrdata = NULL;

    m_cur_data_count = 0;

    m_paintDevice = p;
}

BatchPrinter::BatchPrinter( const QString & path, QWidget* p )
    : m_path( path ), parent( p )
{
    m_data            = NULL;
    m_vardata         = NULL;
    m_addrdata        = NULL;
    m_move            = 0;
    m_image_filename  = E_BARCODE;

    m_cur_data_count  = 0;

    m_paintDevice     = p;
}

BatchPrinter::BatchPrinter( const QString & path, int format, QWidget* _parent )
    : m_path( path ), m_bcp_format( format ), parent( _parent )
{
    m_data            = NULL;
    m_vardata         = NULL;
    m_addrdata        = NULL;
    m_move            = 0;
    m_image_filename  = E_BARCODE;

    m_cur_data_count  = 0;

    m_paintDevice     = _parent;
}

BatchPrinter::~BatchPrinter()
{
    delete m_data;
    delete m_vardata;
    delete m_addrdata;
}

void BatchPrinter::start()
{
    painter = new QPainter( printer );
    m_cur_data_count = 0;
    
    pageh = painter->device()->height();   // in pixel

    curw = 0;
    curh = 0;
    c_h = 0;
    c_w = 0;

    QProgressDialog* progress = createProgressDialog( i18n("Printing...") );

    m_measure = def->getMeasurements();

    moveLabels();
    if( m_data )
	startPrintData( progress );
    else if( m_vardata || m_addrdata )
	startPrintVarData( progress );

    painter->end();
    delete progress;
    delete painter;
}

void BatchPrinter::startPrintData( QProgressDialog* progress )
{
    labelprinterdata* lpdata = PrinterSettings::getInstance()->getData();

    unsigned int index = 0; // counter gets increased for every label, but not for
                            // labels like article_no etc. (those who are created
                            // by events.

    for( int i = 0; i < m_data->count(); i++ ) 
    {
        Label l( def, buffer, m_name, printer, m_customer,
                 (*m_data)[i].article_no, (*m_data)[i].group );
        l.setSerial( m_serial, m_increment );

        bool newgroup = false;
        if( i == 0 )
            newgroup = true;
        else {
            if( (*m_data)[i].group != (*m_data)[i-1].group )
                newgroup = true;
        }

        if( !curw )
            curw = m_measure.gapLeft( printer );

        if( !curh )
            curh = m_measure.gapTop( printer );

        if( m_events ) {
            if( newgroup ) {
                proccessEvents( lpdata->groupEvent1, i18n("Group : ") + (*m_data)[i].group, &l );
                proccessEvents( lpdata->groupEvent2, i18n("Group : ") + (*m_data)[i].group, &l );
                proccessEvents( lpdata->groupEvent3, i18n("Group : ") + (*m_data)[i].group, &l );
                proccessEvents( lpdata->groupEvent4, i18n("Group : ") + (*m_data)[i].group, &l );
            }
            proccessEvents( lpdata->articleEvent1, i18n("Article No. : ") + (*m_data)[i].article_no, &l );
            proccessEvents( lpdata->articleEvent2, i18n("Article No. : ") + (*m_data)[i].article_no, &l );
            proccessEvents( lpdata->articleEvent3, i18n("Article No. : ") + (*m_data)[i].article_no, &l );
            proccessEvents( lpdata->articleEvent4, i18n("Article No. : ") + (*m_data)[i].article_no, &l );
        }
        
        for( int z = 0; z < (*m_data)[i].number; z++ ) {
            changeLine();
            checkForNewPage( &l );

            painter->setClipRect((int)curw, (int)curh, (int)m_measure.width( printer ), (int)m_measure.height( printer ));
            l.setIndex( index );
            l.setRow( c_w );
            l.setCol( c_h );
            painter->save();
            l.draw( painter, (int)curw, (int)curh );
            painter->restore();
            
            //painter->drawPicture( (int)curw, (int)curh, *l.picture() );
            index++;
            painter->setClipping( false );

            drawBorders();

            curw += m_measure.gapH( printer );
            c_h++;

            if( !checkProgressDialog( progress) ) {
                printer->abort();
                painter->end();
                return;
            }
        }
    }
}

void BatchPrinter::startPrintVarData( QProgressDialog* progress )
{
    Label* l;
    while( ( l = initLabel() ) != NULL )
    {
        if( !curw )
            curw = m_measure.gapLeft( printer );

        if( !curh )
            curh = m_measure.gapTop( printer );

	changeLine();
	checkForNewPage( l );

	painter->setClipRect((int)curw, (int)curh, (int)m_measure.width( printer ), (int)m_measure.height( printer ));
        
	painter->save();
	l->draw( painter, (int)curw, (int)curh );
	delete l;
	painter->restore();
            
	painter->setClipping( false );

	drawBorders();

	curw += m_measure.gapH( printer );
	c_h++;

	if( !checkProgressDialog( progress) ) 
	{
	    printer->abort();
	    painter->end();
	    return;
	}
    }
}

Label* BatchPrinter::initLabel( int* number )
{
    Label* l = NULL;

    if( (m_vardata && m_cur_data_count >= m_vardata->count() ) ||
        (m_data && m_cur_data_count >= m_data->count() ) ||
        (m_addrdata && m_cur_data_count >= m_addrdata->count() ) )
        return NULL;

    if( number )
	*number = 1;

    if( m_vardata )
    {
	l = new Label( def, buffer, m_name, m_paintDevice );
	l->setUserVars( (*m_vardata)[m_cur_data_count] );
    }
    else if( m_data )
    {
	l = new Label( def, buffer, m_name, m_paintDevice, m_customer,
		       (*m_data)[m_cur_data_count].article_no, (*m_data)[m_cur_data_count].group );
	if( number )
	    *number = (*m_data)[m_cur_data_count].number;
    }
    else if( m_addrdata )
    {
	l = new Label( def, buffer, m_name, m_paintDevice );
        l->setAddressee( &((*m_addrdata)[m_cur_data_count]) );
    }

    l->setSerial( m_serial, m_increment );
    l->setIndex( m_cur_data_count );
    m_cur_data_count++;

    return l;
}

void BatchPrinter::startImages()
{
    QProgressDialog* progress = createProgressDialog( i18n("Creating Images...") );

    int number       = 0;
    m_cur_data_count = 0;

    Measurements measure = def->getMeasurements();
    Label* l = NULL;
    painter = new QPainter();
    QPixmap pixmap( (int)measure.width( parent ), (int)measure.height( parent ) );
    while( (l = initLabel( &number ) ) != NULL )
    {
        for( int i = 0; i < number; i++ ) 
	{
            pixmap.fill( Qt::white );
            painter->begin( &pixmap );
            l->setIndex( i );
            l->setRow( 0 );
            l->setCol( 0 );
            l->draw( painter, 0, 0 );

            painter->end();

            QString name = m_path + "/";
	    if( m_image_filename == E_ARTICLE )
		name += m_data ? (*m_data)[m_cur_data_count].article_no : QString::number( m_cur_data_count );
	    else if( m_image_filename == E_BARCODE )
		name += l->barcodeNo();
	    else
		name += m_image_custom_filename;

            QString filename = name + QString("_%1.").arg( i ) + m_image_format; //KImageIO::suffix( m_image_format );

            unsigned int c = 0;
            while( QFile::exists( filename ) ) {
                filename += "." + QString::number( c );
                c++;
            }

            pixmap.save( filename, m_image_format.toLatin1().data() );

            if( !checkProgressDialog( progress ) )
	    {
		delete l;
		delete progress;
		delete painter;
                return;
	    }
        }

	delete l;
    }

    delete progress;
    delete painter;

    new KRun( KUrl( m_path ), NULL );
}

void BatchPrinter::startBCP()
{
    int number = 0;
    QFile file( m_path );
    if( !file.open( QIODevice::WriteOnly ) ) // | IO_Raw ) )
    {
        KMessageBox::error( parent, QString( i18n("<qt>Can't open the file or device <b>%1</b></qt>.") ).arg( m_path ) );
        return;        
    }
        
    QProgressDialog* progress = createProgressDialog( i18n("Printing...") );
        
    if( m_bcp_format == PrinterSettings::ZEBRA )
        // Zebra printers are printed at 304dpi, this should
        // be user defined, though
        m_paintDevice = new BarcodePrinterDevice( 304.0, 304.0 );
    else if( m_bcp_format == PrinterSettings::INTERMEC )
        m_paintDevice = new BarcodePrinterDevice( 300.0, 300.0 );
    else if( m_bcp_format == PrinterSettings::TEC )
        // don't know which resolution is used for tec printers
        // so we use a factor to convert everything to mm
        // this is not accurate as QPaintDevice supports only integers
        m_paintDevice = new BarcodePrinterDevice( 25.4000508001016, 25.4000508001016 );
    else if( m_bcp_format == PrinterSettings::EPCL )
        m_paintDevice = new BarcodePrinterDevice( 304.0, 304.0 );
    
    
    QTextStream stream( &file );
    Label* l;
    while( ( l = initLabel( &number ) ) != NULL )
    {
        for( int i = 0; i < number; i++ ) 
        {
            switch( m_bcp_format )
            {
                case PrinterSettings::TEC:
                    // TODO: normally you should not care about the number or counter
                    // KBarcode will ensure that the label is printed often enough
		    if( m_data )			
			stream << tec452(m_name, (*m_data)[m_cur_data_count].number, (*m_data)[m_cur_data_count].article_no, (*m_data)[m_cur_data_count].group, m_customer);
		    else
			/* TODO */;
		    #warning "TEC requires Old KBarcode SQL tables for printing"
		    qDebug( "TEC requires Old KBarcode SQL tables for printing" );
                    break;
                case PrinterSettings::ZEBRA:
                    l->zpl( &stream );
                    break;
                case PrinterSettings::INTERMEC:
                    l->ipl( &stream );
                    break;
                case PrinterSettings::EPCL:
                    l->epcl( &stream );
                    break;
                default:
                    break;
            }
            
            if( !checkProgressDialog( progress ) )
            {
		delete l;
                delete progress;
                delete m_paintDevice;
		m_paintDevice = NULL;

                file.close();

                return;
            }
        }
	delete l;
    }
    
    delete progress;
    delete m_paintDevice;
    m_paintDevice = NULL;

    file.close();
}

void BatchPrinter::checkForNewPage( Label* label )
{
    if( curh + m_measure.height( printer ) > pageh || c_w == m_measure.numV() ) {
        printer->newPage();
        label->setPage( label->page() + 1 );
        curh = m_measure.gapTop( printer );
        curw = m_measure.gapLeft( printer );
        c_w = 0;
    }
}

void BatchPrinter::drawBorders()
{
    if( PrinterSettings::getInstance()->getData()->border ) {
        painter->setPen( QPen( Qt::black, 1 ) );
        painter->drawRect( (int)curw, (int)curh,
                           (int)m_measure.width( painter->device() ),
                           (int)m_measure.height( painter->device() ) );
    }
}

void BatchPrinter::changeLine()
{
    if( c_h >= m_measure.numH() ) {
        c_h = 0;
        curw = m_measure.gapLeft( printer );
        curh += m_measure.gapV( printer );
        c_w++;
    }
}

void BatchPrinter::proccessEvents( int lb, QString value, Label* label )
{
    if( lb == NEW_PAGE ) {
        if( curh > m_measure.gapTop( printer ) || curw > m_measure.gapLeft( printer ) ) {
            printer->newPage();
            label->setPage( label->page() + 1 );
            curh = m_measure.gapTop( printer );
            curw = m_measure.gapLeft( printer );
            c_h = 0;
            c_w = 0;
        }
    }

    if( lb == LINE_BREAK )
        changeLine();

    if( lb == ARTICLE_GROUP_NO )
        printXLabel( lb, label, value );


    if( lb == LABEL_X )
        printXLabel( lb, label, QString::null );
}

void BatchPrinter::printXLabel( int lb, Label* label, const QString & value )
{
    changeLine();
    checkForNewPage( label );

    Label::getXLabel( curw, curh, m_measure.width( printer ), m_measure.height( printer ), painter, lb, value );

    drawBorders();

    curw += + m_measure.gapH( printer );
    c_h++;
}

void BatchPrinter::moveLabels()
{
    for( unsigned int i = 0; i < m_move; i++ ) 
    {
        if( !curw )
            curw = m_measure.gapLeft( printer );

        if( !curh )
            curh = m_measure.gapTop( printer );

        changeLine();

        if( curh + m_measure.height( printer ) > pageh || c_w == m_measure.numV() ) 
        {
            c_w = 0;
            curh = m_measure.gapTop( printer );
            curw = m_measure.gapLeft( printer );
        }

        curw += m_measure.gapH( printer );
        c_h++;
    }
}

QProgressDialog* BatchPrinter::createProgressDialog( const QString & caption )
{
    QProgressDialog* progress = new QProgressDialog( caption, i18n("&Cancel"), 0, m_labels+1, parent );
    progress->setValue( 0 );
    progress->show();
    return progress;
}

bool BatchPrinter::checkProgressDialog( QProgressDialog* progress )
{
    kapp->processEvents( 0 );
    progress->setValue( progress->value() + 1 );
    if( progress->wasCanceled() ) {
        delete progress;
        return false;
    }
    return true;
}

void BatchPrinter::setData( QList<data>* list ) 
{
    if( m_data )
	delete m_data;
    m_data = list; 
}

void BatchPrinter::setData( TVariableList* list )
{
    if( m_vardata )
	delete m_vardata;
    m_vardata = list;
}

void BatchPrinter::setData( KABC::AddresseeList* list )
{
    if( m_addrdata )
	delete m_addrdata;
    m_addrdata = list;
}

