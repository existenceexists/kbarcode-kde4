/***************************************************************************
                          label.cpp  -  description
                             -------------------
    begin                : Mon Apr 29 2002
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

#include "label.h"
#include "measurements.h"
#include "kbarcode.h"
#include "printersettings.h"
#include "sqltables.h"
#include "documentitem.h"
#include "barcodeitem.h"
#include "rectitem.h"
#include "lineitem.h"
#include "imageitem.h"
#include "textitem.h"

// Qt includes
#include <qimage.h>
#include <qiodevice.h>
#include <qpainter.h>
#include <qpen.h>
#include <q3sqlcursor.h>
#include <qxml.h>
#include <QDomDocument>
//Added by qt3to4:
#include <QSqlQuery>

// KDE includes
#include <kapplication.h>
#include <klocale.h>
#include <qprinter.h>

Label::Label( Definition* _def, QIODevice* device, QString labelname, QPaintDevice* _printer, QString customer_id, QString _article_no, QString _group )
    : TokenProvider( _printer )
{
    m_sequence = false;
    m_printer = _printer;
   
    setCustomerNo( customer_id );
    setArticleNo( _article_no );
    setGroup( _group );
    setLabelName( labelname.right( labelname.length() - labelname.findRev( "/" ) - 1 ) );

    d = _def;

    load( device );
}

Label::Label( Definition* _def, QIODevice* device, QString labelname, QPaintDevice* _printer )
    : TokenProvider( _printer )
{
    m_sequence = false;
    m_printer = _printer;
   
    setLabelName( labelname.right( labelname.length() - labelname.findRev( "/" ) - 1 ) );

    d = _def;

    load( device );
}

Label::~Label()
{
}

void Label::epcl( QTextStream* stream )
{
    *stream << EPCLUtils::header();
    
    /*DocumentItem* item;
    for( item = m_list.first();item;item=m_list.next())
	if( drawThisItem( item ) )
	    item->drawEPcl( stream );*/// -!F: original, delete
    QList<DocumentItem*>::iterator i;
    for( i = m_list.begin(); i != m_list.end(); ++i ) {
        if( drawThisItem( *i ) ) {
            (*i)->drawEPcl( stream );
        }
    }
    
    *stream << EPCLUtils::footer();
    
    stream -> flush();
}

void Label::ipl( QTextStream* stream )
{
    IPLUtils utils;
    *stream << utils.header();
    
    /*DocumentItem* item;
    for( item = m_list.first();item;item=m_list.next())
	if( drawThisItem( item ) )
	    item->drawIpl( stream, &utils );*/// -!F: original, delete
    QList<DocumentItem*>::iterator i;
    for( i = m_list.begin(); i != m_list.end(); ++i ) {
        if( drawThisItem( *i ) ) {
            (*i)->drawIpl( stream, &utils );
        }
    }
    
    *stream << utils.footer();
    
    stream -> flush();
}


void Label::zpl( QTextStream* stream )
{
    *stream << ZPLUtils::header();
    
    /*DocumentItem* item;
    for( item = m_list.first();item;item=m_list.next())
	if( drawThisItem( item ) )
	    item->drawZpl( stream );*/// -!F: original, delete
    QList<DocumentItem*>::iterator i;
    for( i = m_list.begin(); i != m_list.end(); ++i ) {
        if( drawThisItem( *i ) ) {
            (*i)->drawZpl( stream );
        }
    }
    
    *stream << ZPLUtils::footer();
    
    stream -> flush();
}

void Label::InitBarcodes()
{
    bool firstbarcode = true;
    
    /*DocumentItem* item;
    for( item = m_list.first();item;item=m_list.next())*/// -!F: original, delete
    QList<DocumentItem*>::iterator i;
    for( i = m_list.begin(); i != m_list.end(); ++i ) 
    {
        DocumentItem* item = *i;
        if( item->rtti() == eRtti_Barcode )
        {
	    /*
	      TODO: I am not sure wether this should be done
	    if( !drawThisItem( item ) )
		continue;
	    */

            BarcodeItem* bc = static_cast<BarcodeItem*>(item);
            bc->setIndex( index() );
            setBarcodeValue( bc );
            bc->updateBarcode();
            
            if( firstbarcode )
            {
                // set some values of the first barcode
                firstbarcode = false;
        
                m_sequence = bc->sequenceEnabled() || m_sequence;
                setBarcodeNo( bc->parsedValue() );
                setEncodingTypeName( bc->type() ); 
            }
        }
    }
    
    updateDone();    
}

void Label::draw( QPainter* painter, int x, int y )
{
    QSize label( (int)d->getMeasurements().width( painter->device() ),
		 (int)d->getMeasurements().height( painter->device() ) );

    InitBarcodes();

    /*DocumentItem* item;
    for( item = m_list.first();item;item=m_list.next())*/// -!F: original, delete
    QList<DocumentItem*>::iterator i;
    for( i = m_list.begin(); i != m_list.end(); ++i ) {
        DocumentItem* item = *i;
	if( drawThisItem( item ) )
	{
	    // add x and y to clip coordinates
	    // as clip has its top left corner
	    // at (0,0)
	    QRect clip( item->boundingRect() );
	    if( x + clip.x() < x )
		clip.setX( 0 );

	    if( y + clip.y() < y )
		clip.setY( 0 );

	    if( clip.x() + clip.width() > label.width() )
		clip.setWidth( label.width() - clip.x() );

	    if( clip.y() + clip.height() > label.height() )
		clip.setHeight( label.height() - clip.y() );
	    
	    painter->save();
	    painter->translate( x,y );
	    /*painter->setClipRect( clip, QPainter::CoordPainter );*/// -!F: original, what is the correct replacement of this ?
	    painter->setClipRect( clip );
	    item->draw( painter );
	    painter->restore();
	}
    }
}

void Label::setBarcodeValue( Barkode* barcode )
{
    // use the same i18n() for static as in BarcodeSettingsDlg
    if( barcode->databaseMode().toLower() != "static" && barcode->databaseMode().toLower() != i18n("Static") ) 
    {
        QString encoding_type = getTypeFromCaption( barcode->databaseMode() );
        QString mode = getModeFromCaption( barcode->databaseMode() );

        QSqlQuery query( "select barcode_no, encoding_type from " TABLE_BASIC 
                         " where article_no = '" + articleNo() + "'" ); 
        while ( query.next() ) 
        {
            barcode->setValue( query.value(0).toString() );
            barcode->setType( query.value(1).toString() );
        }

        if( mode.toLower() != "main" ) 
        {
            QSqlQuery query1( "select barcode_no from " TABLE_CUSTOMER_TEXT " where customer_no ='"+ mode +
                             "' and article_no='" + articleNo() + "'" );
            while ( query1.next() ) 
            {
                if( !query1.value(0).toString().isEmpty() ) 
                {
                    barcode->setValue( query1.value(0).toString() );
                    barcode->setType( encoding_type );
                }
            }
        }
    }
}

void Label::load( QIODevice* device )
{
    if( !device ) return;

    if( !device->isOpen() )
        device->open( QIODevice::ReadOnly );

    QDomDocument doc( "KBarcodeLabel" );        
    doc.setContent( device );
    
    bool kbarcode18;
    QString description;
    Definition* definition = NULL;
    readXMLHeader( &doc, description, kbarcode18, &definition );
    delete definition;
    
    readDocumentItems( &m_list, &doc, NULL, kbarcode18 );
    /*m_list.setAutoDelete( true );*/// -!F: DocumentItemList has no member named setAutoDelete, what is the correct replacement of this?
                
    // sort the list by z index
    /*m_list.sort();*/// -!F: original, is qSort() the right replacement ?
    qSort( m_list );
    
    /*DocumentItem* item;
    for( item = m_list.first();item;item=m_list.next())*/// -!F: original, delete
    QList<DocumentItem*>::iterator i;
    for( i = m_list.begin(); i != m_list.end(); ++i ) {
        DocumentItem* item = *i;
        // set the paint device for all items
        item->setPaintDevice( m_printer );
        item->setTokenProvider( this );
    }
    
    device->close();
}

void Label::getXLabel( double x, double y, double width, double height, QPainter* painter, int mode, QString value )
{
    painter->save();
    if( mode == LABEL_X ) {
        painter->setPen( QPen( Qt::black, 5) );
        painter->drawLine( (int)x, (int)y, int(x+width), int(y+height) );
        painter->drawLine( (int)x, int(y+height), int(x+width), (int)y );
    } else if( mode == ARTICLE_GROUP_NO ) {
        painter->setPen( QPen( Qt::black, 1 ) );
        QFont f( "helvetica", 15 );
        int w = 0;
        do {
            f.setPointSize( f.pointSize() - 1 );
            painter->setFont( f );
            w = painter->fontMetrics().width( value );
        } while( w > width && f.pointSize() > 0 );

        painter->drawText( int(x + (width-w)/2), int(y + (height-f.pointSize())/2), value);
    }

    painter->restore();
    return;
}

int Label::getId() const
{
    if( d )
        return d->getId();

    return -1;
};

bool Label::drawThisItem( const DocumentItem* item )
{
    QString script = item->visibilityScript();

    // make things go faster if the script is just "true"
    if( script.isEmpty() || script == "true" )
	return true;

    script = parse( script );
    
    return jsParseToBool( script );
}

bool Label::update() 
{
    /*DocumentItem* item;
    for( item = m_list.first();item;item=m_list.next())
	if( !item->visibilityScript().isEmpty() && item->visibilityScript() != "true" )
	    return true;*/// -!F: original, delete
    QList<DocumentItem*>::iterator i;
    for( i = m_list.begin(); i != m_list.end(); ++i ) {
        if( !(*i)->visibilityScript().isEmpty() && (*i)->visibilityScript() != "true" ) {
            return true;
        }
    }
	    
    return TokenProvider::update();
}
