/***************************************************************************
                         imageitem.h  -  description
                             -------------------
    begin                : Do Sep 9 2004
    copyright            : (C) 2004 by Dominik Seichter
    email                : domseichter@web.de
 ***************************************************************************/

/***************************************************************************
                                                                          
    This program is free software; you can redistribute it and/or modify  
    it under the terms of the GNU General Public License as published by  
    the Free Software Foundation; either version 2 of the License, or     
    (at your option) any later version.                                   
                                                                          
 ***************************************************************************/

#include "imageitem.h"
#include "tokenprovider.h"

#include <qbuffer.h>
#include <qdom.h>
#include <qimage.h>
#include <QPaintDevice>
#include <QX11Info>
#include <QTextStream>

#include <qpainter.h>
#include <QPixmap>
#include <QImageWriter>

#include <klocale.h>
#include <kcodecs.h> 

ImageItem::ImageItem()
    : DocumentItem()
{
    init();
}

ImageItem::~ImageItem()
{
}

void ImageItem::init()
{
    m_rotation = 0.0;
    m_mirror_h = false;
    m_mirror_v = false;
    m_scaling = eImage_Original;
    setBorder( false );
    
    setRect( QRect( 0, 0, 20, 20 ) );
}

void ImageItem::draw(QPainter* painter)
{
    createImage();
    
    painter->save();
    painter->drawPixmap( rect().x(), rect().y(), m_tmp );
    painter->restore();
    DocumentItem::drawBorder(painter);
}

void ImageItem::drawZpl( QTextStream* stream )
{
    createImage();
    
    QBuffer buffer;
    /*if( buffer.open( QIODevice::WriteOnly ) ) // -!F: original, delete
    {
        // TODO: bmp????-
        QImageIO io( &buffer, "PNG" ); 
        QImage image = m_tmp.convertToImage();
        // create a black and white image
        io.setImage( image.convertDepth( 1 ) );
        io.write();
        buffer.close();

        QByteArray data = buffer.buffer();
        *stream << ZPLUtils::fieldOrigin( rect().x(), rect().y() );
        *stream << "~DYD,p,P," << QString::number( data.size() ) + ",0,";
        for( unsigned int i=0;i<data.size();i++)
            *stream << data[i];
    }*/
    if( buffer.open( QIODevice::WriteOnly ) )
    {
        // TODO: bmp????-
        QImageWriter io( &buffer, QByteArray("PNG") ); 
        QImage image = m_tmp.toImage();
        // create a black and white image
        io.write(image.convertToFormat(QImage::Format_Mono));
        buffer.close();

        QByteArray data = buffer.buffer();
        *stream << ZPLUtils::fieldOrigin( rect().x(), rect().y() );
        *stream << "~DYD,p,P," << QString::number( data.size() ) + ",0,";
        for( int i=0;i<data.size();i++)
            *stream << data[i];
    }
    stream -> flush();
}

void ImageItem::drawIpl( QTextStream*, IPLUtils* )
{
    qDebug("ImageItem is not implemented for IPL!");
}

void ImageItem::drawEPcl( QTextStream* stream )
{
    createImage();
    
    // you can access m_tmp (which is a QPixmap now)
    // m_tmp is an image which has all transformations
    // (i.e. rotation, scaling, flipping) already apllied
            
	QImage si = m_tmp.toImage();
	int width = si.width();
	int height = si.height();
	int c;
	char out;

	for( int buf=0; buf < 3; buf++ ){
		*stream << QString("GS %1 32 %1 %1 %1 %1 ")
			.arg(buf).arg(rect().x()).arg(rect().y()).arg(width).arg(height);

		for( int i=width-1; i >= 0; i-- ){
			for( int j=0; j < height; j++ ){
				if( buf == 0 )
					c = qBlue(si.pixel(i,j)); // Yellow
				else if( buf == 1 )
					c = qGreen(si.pixel(i,j)); // Magenta
				else if( buf == 2 )
					c = qRed(si.pixel(i,j)); // Cyan

				// printer has 5-bits per color - drop 3
				// we also need to subtract from 255 to convert from RGB to CMY
				out = (0xff - c) >> 3;
				*stream << out;
			}
		}

		*stream << "\r\n";
	}
        
        stream -> flush();
}

void ImageItem::loadXML(QDomElement* element)
{
    QByteArray out;
    if( !element->text().isEmpty() )
    {
	KCodecs::base64Decode( element->text().toUtf8(), out );
	m_pixmap.loadFromData( out, "PNG" );
    }

    m_expression = element->attribute( "expression", QString::null );
    m_scaling = (EImageScaling)element->attribute( "scaling", "0" ).toInt();
    m_mirror_h = (bool)element->attribute("mirror_horizontal", "0" ).toInt();
    m_mirror_v = (bool)element->attribute("mirror_vertical", "0" ).toInt();
    m_rotation = element->attribute("rotation", "0.0").toDouble();
    DocumentItem::loadXML(element);
    
    updateImage();
}

void ImageItem::saveXML(QDomElement* element)
{
    QBuffer buf;
    if( !m_pixmap.isNull() )
    {
	if(!buf.open( QIODevice::WriteOnly ))
	    return;
        
	m_pixmap.save( &buf, "PNG" );
	buf.close();
    
	element->appendChild( element->ownerDocument().createCDATASection( KCodecs::base64Encode( buf.buffer(), true ) ) );
    }

    element->setAttribute( "expression", m_expression );
    element->setAttribute( "scaling", (int)m_scaling );
    element->setAttribute( "mirror_horizontal", m_mirror_h );
    element->setAttribute( "mirror_vertical", m_mirror_v );
    element->setAttribute( "rotation", m_rotation );
    
    DocumentItem::saveXML(element);
}

void ImageItem::setExpression( const QString & ex )
{
    m_expression = ex;
    updateImage();
}

void ImageItem::setPixmap( const QPixmap & pix )
{
    m_pixmap = pix;
    updateImage();
}

const QPixmap & ImageItem::pixmap()
{
    return m_pixmap;
}

void ImageItem::setScaling( EImageScaling scaling )
{
    m_scaling = scaling;
    updateImage();
}

EImageScaling ImageItem::scaling() const
{
    return m_scaling;
}

void ImageItem::updateImage()
{
    /*m_tmp.resize( QSize(0,0) );*/// -!F: original, delete
    m_tmp = m_tmp.copy(QRect(QPoint(0, 0), QSize(0,0)));
}

void ImageItem::createImage()
{
    if( m_tmp.isNull() )
    {
        QImage img;

	if( m_pixmap.isNull() )
	    img.load( tokenProvider() ? tokenProvider()->parse( m_expression ) : m_expression );
	else
	    img = m_pixmap.toImage();

	if( !img.isNull() )
	{
	    if( m_rotation != 0.0 )
	    {
		QMatrix matrix;
		matrix.rotate( m_rotation );
		img = img.transformed( matrix );
	    }
        
	    // scale with high quality on the printer
	    // but use faster scaling for onscreen operations
	    if( m_scaling != eImage_Original )
	    {
		/*if( DocumentItem::paintDevice()->isExtDev() )// -!F: original, delete, What is the right replacement of isExtDev() ? They say no replacement is available.
		    img = img.smoothScale( rect().width(), rect().height(), 
					   (m_scaling == eImage_Scaled ? Qt::ScaleMin : QImage::ScaleFree) );
		else
		    img = img.scale( rect().width(), rect().height(), 
				     (m_scaling == eImage_Scaled ? Qt::ScaleMin : QImage::ScaleFree) );*/
		img = img.scaled( rect().width(), rect().height(), 
			(m_scaling == eImage_Scaled ? Qt::KeepAspectRatio : Qt::IgnoreAspectRatio),
			Qt::SmoothTransformation );
	    }
	    else
	    {
		// we have to scale because of the bigger printer resolution
		/*if( DocumentItem::paintDevice()->isExtDev() )// -!F: original, delete,  What is the right replacement of isExtDev() ? They say no replacement is available.
		{
		    QPaintDevice* device = DocumentItem::paintDevice();
		    
		    img = img.smoothScale(
                                          (int)(img.width() * ((double)device->logicalDpiX()/QX11Info::appDpiX())),
                                          (int)(img.height() * ((double)device->logicalDpiY()/QX11Info::appDpiY())),
                                          Qt::ScaleMin );
		}*/
		QPaintDevice* device = DocumentItem::paintDevice();
		
		img = img.scaled(
                    (int)(img.width() * ((double)device->logicalDpiX()/QX11Info::appDpiX())),
                    (int)(img.height() * ((double)device->logicalDpiY()/QX11Info::appDpiY())),
                    Qt::KeepAspectRatio );
	    }

	    if( m_mirror_h || m_mirror_v )
		img = img.mirrored( m_mirror_h, m_mirror_v );
	    
	    m_tmp.convertFromImage( img );
	}
	else
	{
	    // The expression does not return a valid image path
	    // or the user did not specify a image to load
	    
	    // make sure that we do not get a 0 0 0 0 rectangle
	    QRect myrect( 0, 0, rect().width() ? rect().width() : 100 , rect().height() ? rect().height() : 80 );
	    m_tmp = m_tmp.copy(0, 0, rect().width(), rect().height() );
	    
	    QPainter painter( &m_tmp );
	    painter.fillRect( myrect, Qt::white );
	    painter.setPen( QPen( Qt::red, 2 ) );
	    painter.drawRect( myrect );
	    painter.drawLine( 0, 0, myrect.width(), myrect.height() );
	    painter.drawLine( 0, myrect.height(), myrect.width(), 0 );
	    painter.setPen( Qt::black );
	    painter.drawText( 0, painter.fontMetrics().lineSpacing(), i18n("Expression: ") + m_expression );
	    painter.end();
	}
    }
}

void ImageItem::setRotation( double rot )
{
    if( m_rotation <= 360.0 && m_rotation >= -360.0 )
    {
        m_rotation = rot;
        updateImage();
    }
}

double ImageItem::rotation() const
{
    return m_rotation;
}

void ImageItem::setMirrorHorizontal( bool b )
{
    m_mirror_h = b;
    updateImage();
}

bool ImageItem::mirrorHorizontal() const
{
    return m_mirror_h;
}

void ImageItem::setMirrorVertical( bool b )
{
    m_mirror_v = b;
    updateImage();
}

bool ImageItem::mirrorVertical() const
{
    return m_mirror_v;
}
