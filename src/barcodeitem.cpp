/***************************************************************************
                         barcodeitem.cpp  -  description
                             -------------------
    begin                : Do Sep 2 2004
    copyright            : (C) 2004 by Dominik Seichter
    email                : domseichter@web.de
 ***************************************************************************/

/***************************************************************************
                                                                          
    This program is free software; you can redistribute it and/or modify  
    it under the terms of the GNU General Public License as published by  
    the Free Software Foundation; either version 2 of the License, or     
    (at your option) any later version.                                   
                                                                          
 ***************************************************************************/

#include "barcodeitem.h"
#include "tcanvasitem.h"

#include <qdom.h>
#include <qpainter.h>
#include <QPaintDevice>
#include <QTextStream>
#include <QDebug>

BarcodeItem::BarcodeItem()
    : Barkode(), DocumentItem()
{
    init();
}

BarcodeItem::BarcodeItem( const Barkode & bcode )
    : Barkode( bcode ), DocumentItem()
{
    init();
}

QMap<QString,QString> BarcodeItem::legacy;

void BarcodeItem::init()
{
 // read barcodes saved by kbarcode < 1.3.0
    /*
      // disable legacy code as it is really old
      // and conflicts with tbarcode2 support

    if( !legacy.count() ) {
        legacy.insert( "0", "any" );
        legacy.insert( "1", "ean" );
        legacy.insert( "2", "upc" );
        legacy.insert( "3", "isbn" );
        legacy.insert( "4", "code39" );
        legacy.insert( "5", "code128" );
        legacy.insert( "6", "code128c" );
        legacy.insert( "7", "code128b" );
        legacy.insert( "8", "i25" );
        legacy.insert( "9", "i28raw" );
        legacy.insert( "10", "cbr" );
        legacy.insert( "11", "msi" );
        legacy.insert( "12", "pls" );
        legacy.insert( "13", "code93" );
        legacy.insert( "14", "msi" );
        legacy.insert( "15", "code39 -c" );
        legacy.insert( "16", "i25 -c" );
    }
    */
    setBorder( false );
    setRect( QRect( 0, 0, 100, 100 ) );
   
    updateBarcode();
}

void BarcodeItem::loadXML (QDomElement* element)
{
    // TODO: default() should be called first and the current values should be used
    // instead of the now hardcoded values
    // i.e: setQuietZone( element->attribute("margin", QString::number( quietZone() ) ).toInt() );
    
    setQuietZone( element->attribute("margin", "10" ).toInt() );
    setRotation( element->attribute("rotation", "0" ).toInt() );
    setScaling( element->attribute("scale", "1000" ).toDouble() / 1000 );
    setCut( element->attribute("cut", "1.0" ).toDouble() );
    setType( element->attribute("type", "code39" ) );
    
    /*
    * check for encoding types saved by kbarcode <= 1.2.0
    */
    if( legacy.contains( type() ) )
        setType( legacy[type()] );

    setTextVisible( element->attribute("text", "0" ).toInt() );
    setDatabaseMode( element->attribute("caption", "Static" ) );

    if( engine()->options() )
        engine()->options()->load( element );

    setDatamatrixSize( element->attribute( "datamatrix.size", "0" ).toInt() );

    setSequenceEnabled( element->attribute( "sequenceenabled", "0" ).toInt() );
    if( sequenceEnabled() ) 
    {
        setSequenceMode( (ESequence)element->attribute( "sequencemode", "0" ).toInt() );
        setSequenceStep( element->attribute( "sequencestep", "1" ).toInt() );
        setSequenceStart( element->attribute( "sequencestart", "0" ).toInt() );
    }

    QDomNode n = element->firstChild();
    while( !n.isNull() ) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if( !e.isNull() )
            if( e.tagName() == "value" )
                setValue( e.text() );

        n = n.nextSibling();
    }

    DocumentItem::loadXML( element );
    
    updateBarcode();
}

void BarcodeItem::saveXML (QDomElement* element)
{
    element->setAttribute( "margin", quietZone() );
    element->setAttribute( "rotation", rotation() );
    element->setAttribute( "cut", cut() );
    element->setAttribute( "caption", databaseMode() );

    /*
     * This values are only needed for !cache and for sequences
     */
    element->setAttribute( "sequenceenabled", sequenceEnabled() );
    if( sequenceEnabled() ) 
    {
        element->setAttribute( "sequencemode", (int)sequenceMode() );
        element->setAttribute( "sequencestep", sequenceStep() );
        element->setAttribute( "sequencestart", sequenceStart() );
    }

    element->setAttribute( "type", type() );
    element->setAttribute( "text", textVisible() );
    element->setAttribute( "scale", scaling()*1000 );

    if( engine()->options() )
        engine()->options()->save( element );

    element->setAttribute( "datamatrix.size", datamatrixSize() );

    QDomElement texttag = element->ownerDocument().createElement( "value" );
    texttag.appendChild( element->ownerDocument().createTextNode( value() ) );

    element->appendChild( texttag );

    DocumentItem::saveXML( element );
}

void BarcodeItem::draw (QPainter* painter)
{
    painter->save();
    /*drawBarcode( *painter, rect().x(), rect().y() );*/// -!F: original
    drawBarcode( *painter, 0, 0 );// -!F: added
    painter->restore();
    
    /*TCanvasItem* citem = canvasItem();
    if( citem ) 
    {
        citem->setSize( Barkode::size().width(), Barkode::size().height() );
    }*/// -!F: original, why was setSize() called ? It couses bad positioning of an item as well as too much CPU usage.
    // TODO: do a bitBlt when device is screen
    //painter->drawPixmap( rect().x(), rect().y(), m_pixmap );
    //bitBlt( painter->device(), rect().x(), rect().y(), &m_pixmap, 0, 0, rect().width(), rect().height(), Qt::CopyROP );

    DocumentItem::drawBorder( painter );
}

void BarcodeItem::drawPreview (QPainter* painter)// -!F: added
{
    painter->save();
    drawBarcode( *painter, rect().x(), rect().y() );
    painter->restore();

    DocumentItem::drawBorderPreview( painter );
}

void BarcodeItem::drawZpl( QTextStream* stream )
{
    QString encoding = ZPLUtils::encoding( type() );
    if( encoding.isNull() )
    {
        qDebug( "ERROR: No ZPL barcode found");
        return;
    }
    
    *stream << ZPLUtils::fieldOrigin( rect().x(), rect().y() );
    *stream << "^B" << encoding;
    *stream << ZPLUtils::fieldData( value() );
    stream -> flush();
}

void BarcodeItem::drawIpl(  QTextStream* stream, IPLUtils* utils )
{
    QString encoding = utils->encoding( type() );

    if( encoding.isEmpty() )
    {
        qDebug( "ERROR: No IPL barcode found");
        return;
    }
    
    int counter = utils->counter();
    QString s = QString("B%1;").arg( counter ); // field number

    s += utils->fieldOrigin( rect().x(), rect().y() );    
    s += QString("c%1;").arg( encoding ); // encoding type
    s += QString("h%1;").arg( rect().height() ); // height of barcode
    s += QString("w%1;").arg( 3 ); // width of barcode (per line)
    s += QString("d0,%1;").arg( value().length() ); // max length of data
    
    *stream << utils->field( s );
    utils->addValue( value() );
    stream -> flush();
}

void BarcodeItem::drawEPcl( QTextStream* stream )
{
    QString encoding = EPCLUtils::encoding( type() );
    if( encoding.isEmpty() )
    {
        qDebug( "ERROR: No EPCL barcode found");
        return;
    }

    // Coordinates cannot start at zero
    QString s = QString("B %1").arg( rect().x()+1 );
    s += QString(" %1 0").arg( rect().y() + rect().height() );
    s += QString(" %1 1 4").arg( encoding );
    s += QString(" %1 1").arg( rect().height() );
    s += QString(" %1").arg( value() );

    *stream << EPCLUtils::field( s );
    stream -> flush();
}

void BarcodeItem::updateBarcode()
{
    Barkode::setTokenProvider( tokenProvider() );
    Barkode::update( DocumentItem::paintDevice() );
    setSize( Barkode::size().width(), Barkode::size().height() );
    if( canvasItem() ) {
        canvasItem()->setRect( 0, 0, Barkode::size().width(), Barkode::size().height() );
    }
}

