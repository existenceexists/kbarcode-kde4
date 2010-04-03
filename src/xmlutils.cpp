/***************************************************************************
                          xmlutils.cpp  -  description
                             -------------------
    begin                : Mit Mai 7 2003
    copyright            : (C) 2003 by Dominik Seichter
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

#include "xmlutils.h"
#include "definition.h"
#include "mycanvasview.h"
#include "mycanvasitem.h"
#include "labelutils.h"
#include "measurements.h"
#include "barcodeitem.h"
#include "rectitem.h"
#include "imageitem.h"
#include "lineitem.h"
#include "textitem.h"
#include "textlineitem.h"

// Qt includes
#include <qdom.h>
//Added by qt3to4:
#include <QPixmap>

// KDE includes
#include <kapplication.h>
#include <klocale.h>
#include <kmessagebox.h>

XMLUtils::XMLUtils()
{
    // read barcodes saved by kbarcode < 1.3.0
    if( !legacy.count() ) 
    {
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
}

XMLUtils::~XMLUtils()
{
}

QMap<QString,QString> XMLUtils::legacy;

void XMLUtils::readXMLHeader( QDomDocument* doc, QString & description, bool & kbarcode18, Definition** def )
{
    QDomNode n = doc->documentElement().firstChild();
    // this should not be neccessary,
    // but <label><id> needs to be processed first
    while( !n.isNull() ) {
        QDomElement e = n.toElement(); // try to convert the node to an element.
        if( !e.isNull() )
            // label has to be the first xml element!
            if( e.tagName() == "label" ) {
                kbarcode18 = (e.attribute( "fileformat-version", "1" ).toInt() == 1);
                QDomNode n = e.firstChild();
                while( !n.isNull() ) {
                    QDomElement e = n.toElement(); // try to convert the node to an element.
                    if( !e.isNull() )                                                  
                        if( e.tagName() == "description" )
                            description = e.text();
                        else if( e.tagName() == "id" ) {
                            if( *def )
                                delete *def;
                            *def = readDefinition( &e );
                        }

                    n = n.nextSibling();
                }
            }
        n = n.nextSibling();
    }
}

void XMLUtils::writeXMLHeader( QDomNode* root, const QString & description, Definition* def )
{
    QDomElement data = root->ownerDocument().createElement( "label" );
    /** kbarcode <= 1.8.x = fileformat-version = 1 */
    /** kbarcode >= 1.9.0 = fileformat-version = 2 */
    data.setAttribute( "fileformat-version", "2" );
    
    if( !description.isEmpty() ) {
        QDomElement labeldescription = root->ownerDocument().createElement( "description" );
        labeldescription.appendChild( root->ownerDocument().createTextNode( description ) );
        data.appendChild( labeldescription );
    }

    QDomElement labelid = root->ownerDocument().createElement( "id" );
    writeDefinition( &labelid, def );
    labelid.appendChild( root->ownerDocument().createTextNode( QString( "%1" ).arg(def->getId()) ) );

    data.appendChild( labelid );
    root->appendChild( data );
}

void XMLUtils::readDocumentItems( DocumentItemList* list, QDomDocument* doc, TokenProvider* token, bool kbarcode18 )
{
    QDomNode n = doc->documentElement().firstChild();
    
    if( kbarcode18 )
    {
        // legacy loading
        while( !n.isNull() ) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if( !e.isNull() )
                if( e.tagName() == "barcode" ) {
                    BarcodeItem* bcode = new BarcodeItem();
		    ((DocumentItem*)bcode)->setTokenProvider( token );
                    readBarcode( &e, bcode );
                    bcode->updateBarcode();
                    bcode->setZ( e.attribute( "z", "0" ).toInt() );
                
                    list->append( bcode );
                } else if( e.tagName() == "textfield" ) {
                    TextItem* t = new TextItem();
		    t->setTokenProvider( token );
                    
                    QRect r = readXMLRect( &e );
                    t->setBoundingRect( r );
                    t->setZ( e.attribute( "z", "0" ).toInt() );
                       
                    QDomNode n = e.firstChild();
                    while( !n.isNull() ) {
                        QDomElement e = n.toElement(); // try to convert the node to an element.
                        if( !e.isNull() )
                            if( e.tagName() == "text" )
                            {
                                t->setText(  e.text() );
                                break;
                            }
                        n = n.nextSibling();
                    }
        
                    list->append( t );
                } else if( e.tagName() == "picture" ) {
                    ImageItem* r = new ImageItem();
        
                    QRect rect = readXMLRect( &e );
                    r->setBoundingRect( rect );
                    
                    QPixmap pix;
                    pix.loadFromData( e.text().toUtf8(), "XPM" );
                    r->setPixmap( pix );
                    r->setRotation( e.attribute("rotation", "0.0" ).toDouble() );
                    r->setZ( e.attribute( "z", "0" ).toInt() );
                    list->append( r );
                } else if( e.tagName() == "rect" ) {
                    RectItem* r = new RectItem();
                    
                    QRect rect = readXMLRect( &e );
                    r->setBoundingRect( rect );
                    r->setZ( e.attribute( "z", "0" ).toInt() );
                    
                    r->setCircle( e.attribute( "circle", "0" ).toInt() );
                    
                    r->setPen( QPen(readXMLColor( &e, "bordercolor", Qt::black ),e.attribute( "borderwidth", "1" ).toInt(),(Qt::PenStyle)e.attribute( "borderstyle", "1" ).toInt() ));
                    r->setColor( readXMLColor( &e, "color", Qt::black ) );
                    list->append( r );
                } else if( e.tagName() == "line" ) {
                    //TODO: test legacy loading of lines
                    LineItem* cl = new LineItem();
                    int x = e.attribute( "x1", "0" ).toInt();
                    int y = e.attribute( "y1", "0" ).toInt();
                    cl->setBoundingRect( QRect( x, y, e.attribute( "x2", "0" ).toInt(), e.attribute( "y2", "0" ).toInt() ) );
                    
                    cl->setZ( e.attribute( "z", "0" ).toInt() );
                    cl->setPen( QPen( readXMLColor( &e, "color", Qt::black ),
                                e.attribute( "width", "0" ).toInt(), (Qt::PenStyle)e.attribute( "style", "0" ).toInt() ) );
                    list->append( cl );
                }
            n = n.nextSibling();
        }
    }
    else
    {
        while( !n.isNull() ) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if( !e.isNull() )
            {
                DocumentItem* item = NULL;
                if( readXMLDocumentItem( &e, &item, token ) )
                    list->append( item );
            }
            n = n.nextSibling();
        }
    }

    qSort( list->begin(), list->end() );
}

void XMLUtils::writeXMLDocumentItem( QDomElement* root, DocumentItem** item )
{
    if( !item )
        return;
        
    QDomElement tag;
    if( (*item)->rtti() == eRtti_Barcode )
        tag = root->ownerDocument().createElement( "barcode" );
    else if( (*item)->rtti() == eRtti_Image )
        tag = root->ownerDocument().createElement( "picture" );
    else if( (*item)->rtti() == eRtti_Text )
        tag = root->ownerDocument().createElement( "textfield" );
    else if( (*item)->rtti() == eRtti_Rect )
        tag = root->ownerDocument().createElement( "rect" );
    else if( (*item)->rtti() == eRtti_Line )
        tag = root->ownerDocument().createElement( "line" );
//NY32    
    else if( (*item)->rtti() == eRtti_TextLine )
        tag = root->ownerDocument().createElement( "textlinefield" );
//NY32
    
    if( !tag.isNull() )
    {
        (*item)->saveXML( &tag );
        root->appendChild( tag );
    }
}

bool XMLUtils::readXMLDocumentItem( QDomElement* tag, DocumentItem** item, TokenProvider* token )
{
    if( !item )        
        return false;
        
    if( tag->tagName() == "barcode" )
        *item = new BarcodeItem();
    else if( tag->tagName() == "textfield" )
        *item = new TextItem();
    else if( tag->tagName() == "picture" )
        *item = new ImageItem();
    else if( tag->tagName() == "rect" )
        *item = new RectItem();
    else if( tag->tagName() == "line" )
        *item = new LineItem();
    else if( tag->tagName() == "textlinefield" )
        *item = new TextLineItem();
    else
        return false;

    (*item)->setTokenProvider( token );
    (*item)->loadXML( tag );
    return true;
}

void XMLUtils::writeXMLColor( QDomElement* tag, const QString & prefix, QColor c ) const
{
    tag->setAttribute( prefix + "r", c.red() );
    tag->setAttribute( prefix + "g", c.green() );
    tag->setAttribute( prefix + "b", c.blue() );
}

QColor XMLUtils::readXMLColor( QDomElement* tag, const QString & prefix, QColor c )
{
    int r = c.red();
    int g = c.green();
    int b = c.blue();

    r = tag->attribute( prefix + "r", QString("%1").arg( r ) ).toInt();
    g = tag->attribute( prefix + "g", QString("%1").arg( g ) ).toInt();
    b = tag->attribute( prefix + "b", QString("%1").arg( b ) ).toInt();

    return QColor( r, g, b);
}

/*
void XMLUtils::writeXMLRect( QDomElement* tag, QRect r, QPoint ptOrigin ) const
{
    r.moveBy( -ptOrigin.x(), -ptOrigin.y() );
    
    tag->setAttribute( "x", r.x() );
    tag->setAttribute( "y", r.y() );
    tag->setAttribute( "width", r.width() );
    tag->setAttribute( "height", r.height() );

    LabelUtils l;
    tag->setAttribute( "x_mm", l.pixelToMm( r.x(), KApplication::desktop(), LabelUtils::DpiX ) );
    tag->setAttribute( "y_mm", l.pixelToMm( r.y(), KApplication::desktop(), LabelUtils::DpiY ) );
    tag->setAttribute( "width_mm", l.pixelToMm( r.width(), KApplication::desktop(), LabelUtils::DpiX ) );
    tag->setAttribute( "height_mm", l.pixelToMm( r.height(), KApplication::desktop(), LabelUtils::DpiY ) );
}*/

QRect XMLUtils::readXMLRect( QDomElement* tag )
{
    QRect r;
    int x = tag->attribute( "x", "0" ).toInt();
    int y = tag->attribute( "y", "0" ).toInt();

    r.setX( x );
    r.setY( y );    
    r.setWidth( tag->attribute( "width", "-1" ).toInt() );
    r.setHeight( tag->attribute( "height", "-1" ).toInt() );
    return r;
}

void XMLUtils::writeDefinition( QDomElement* tag, Definition* d ) const
{
    tag->setAttribute("producer", d->getProducer());
    tag->setAttribute("type", d->getType());

    tag->setAttribute("width", d->getMeasurements().widthMM());
    tag->setAttribute("height", d->getMeasurements().heightMM());
    tag->setAttribute("gap_left", d->getMeasurements().gapLeftMM());
    tag->setAttribute("gap_top", d->getMeasurements().gapTopMM());
    tag->setAttribute("gap_v", d->getMeasurements().gapVMM());
    tag->setAttribute("gap_h", d->getMeasurements().gapHMM());
    tag->setAttribute("num_v", d->getMeasurements().numV());
    tag->setAttribute("num_h", d->getMeasurements().numH());
}

Definition* XMLUtils::readDefinition( QDomElement* tag )
{
    int label_def_id = tag->text().toInt();
    QString producer = tag->attribute("producer", "" );
    QString type = tag->attribute("type", "" );

    // The label was written by KBarcode <= 1.2.0
    Definition* d = new Definition( label_def_id );
    if( type.isEmpty() && producer.isEmpty() ) {
        KMessageBox::information( NULL,
            i18n("<qt>This appears file appears to be created by an older version of KBarcode.<br>"
                 "Please check if the used label definition is correct:<br>"
                 "<b>") + d->getProducer() + " " + d->getType() + "</b><br>"
                 "You can change the label definition at <i>Edit->Change Label...</i></qt>");

        return d;
    }
    
    if( d->getType() == type && d->getProducer() == producer && !type.isEmpty() && !producer.isEmpty() )
        return d;
        
    // The id does not match the correct type and producer,
    // try to find the correct one
    if( (d->getType() != type || d->getProducer() != producer) &&
        !(type.isEmpty() && producer.isEmpty() ) ) {

        int id = label_def_id;
        QStringList p = Definition::getProducers();
        if( !p.contains( producer ) ) {
            for( int i = 0; i < p.count(); i++ ) {
                id = Definition::getClosest( p[i], type );
                if( id >= 0 )
                    break;
            }
        } else
            id = Definition::getClosest( producer, type );

        if( id != label_def_id && id >= 0 ) {
            d->setId( id );
            return d;
        }
    }

    // Everything failed
    // Read the measurements from the file
    Measurements m;
    m.setWidthMM( tag->attribute("width", I2S( m.widthMM() ) ).toDouble() );
    m.setHeightMM( tag->attribute("height", I2S( m.heightMM() )).toDouble() );
    m.setGapLeftMM( tag->attribute("gap_left", I2S( m.gapLeftMM() )).toDouble() );
    m.setGapTopMM( tag->attribute("gap_top", I2S( m.gapTopMM() )).toDouble() );
    m.setGapVMM( tag->attribute("gap_v", I2S( m.gapVMM() )).toDouble() );
    m.setGapHMM( tag->attribute("gap_h", I2S( m.gapHMM() )).toDouble() );
    m.setNumV( tag->attribute("num_v", I2S( m.numH() )).toInt() );
    m.setNumH( tag->attribute("num_h", I2S( m.numV() )).toInt() );

    qDebug("Definition not found: writing to file!");
    d->setId( Definition::write( m, type, producer ) );
        
    return d;
}

void XMLUtils::writeBarcode( QDomElement* tag, const Barkode* data, bool cache ) const
{
    if( !cache ) {
        /*
         * This values are not needed for the barcode cache.
         */
        tag->setAttribute( "margin", data->quietZone() );
        tag->setAttribute( "rotation", data->rotation() );
        tag->setAttribute( "cut", data->cut() );
        tag->setAttribute( "caption", data->databaseMode() );

        /*
         * This values are only needed for !cache and for sequences
         */
        tag->setAttribute( "sequenceenabled", data->sequenceEnabled() );
        /*
        if( data->sequenceEnabled() ) {
            tag->setAttribute( "sequencemode", data->sequence.mode );
            tag->setAttribute( "sequencestep", data->sequence.step );
            tag->setAttribute( "sequencestart", data->sequence.start );
        }
        */
    }

    tag->setAttribute( "type", data->type() );
    tag->setAttribute( "text", data->textVisible() );
    tag->setAttribute( "scale", data->scaling()*1000 );

    /*
    if( BarCode::hasFeature( data->type, PDF417BARCODE ) ) {
        tag->setAttribute( "pdf417.row", data->pdf417.row );
        tag->setAttribute( "pdf417.col", data->pdf417.col );
        tag->setAttribute( "pdf417.err", data->pdf417.err );
    }

    if( BarCode::hasFeature( data->type, DATAMATRIX ) )
        tag->setAttribute( "datamatrix.size", data->datamatrix.size );

    if( BarCode::hasFeature( data->type, TBARCODE ) ) {
        tag->setAttribute( "tbarcode.modulewidth", data->tbarcode.modulewidth );
        tag->setAttribute( "tbarcode.escape", data->tbarcode.escape );
        tag->setAttribute( "tbarcode.above", data->tbarcode.above );
        tag->setAttribute( "tbarcode.autocorrect", data->tbarcode.autocorrect );
        tag->setAttribute( "tbarcode.checksum", data->tbarcode.checksum );
    }
    */
        
    QDomElement texttag = tag->ownerDocument().createElement( "value" );
    texttag.appendChild( tag->ownerDocument().createTextNode( data->value() ) );

    tag->appendChild( texttag );
}

void XMLUtils::readBarcode( QDomElement* tag, Barkode* bcode )
{
    if( tag->tagName() == "barcode" ) {
        bcode->setQuietZone( tag->attribute("margin", "10" ).toInt() );
        bcode->setRotation( tag->attribute("rotation", "0" ).toInt() );
        bcode->setScaling( tag->attribute("scale", "1000" ).toDouble() / 1000 );
        bcode->setCut( tag->attribute("cut", "1.0" ).toDouble() );
        bcode->setType( tag->attribute("type", "code39" ) );

        /*
         * check for encoding types saved by kbarcode <= 1.2.0
         */
         if( legacy.contains( bcode->type() ) )
            bcode->setType( legacy[bcode->type()] );

        bcode->setTextVisible( tag->attribute("text", "0" ).toInt() ); 
        bcode->setDatabaseMode( tag->attribute("caption", "static" ) );

        if( bcode->engine()->options() )
            bcode->engine()->options()->load( tag );

        bcode->setDatamatrixSize( tag->attribute( "datamatrix.size", "0" ).toInt() );
    
        bcode->setSequenceEnabled( tag->attribute( "sequenceenabled", "0" ).toInt() );
        if( bcode->sequenceEnabled() ) 
        {
            bcode->setSequenceMode( (ESequence)tag->attribute( "sequencemode", "0" ).toInt() );
            bcode->setSequenceStep( tag->attribute( "sequencestep", "1" ).toInt() );
            bcode->setSequenceStart( tag->attribute( "sequencestart", "0" ).toInt() );
        }

        QDomNode n = tag->firstChild();
        while( !n.isNull() ) {
            QDomElement e = n.toElement(); // try to convert the node to an element.
            if( !e.isNull() )
                if( e.tagName() == "value" )
                    bcode->setValue( e.text() );

            n = n.nextSibling();
        }
    }
}
