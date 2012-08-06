/***************************************************************************
                         documentitem.h  -  description
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

#ifndef DOCUMENTITEM_H
#define DOCUMENTITEM_H

#include <xmlutils.h>
#include <qobject.h>
#include <qstring.h>
#include <QTextStream>
//#include <QDesktopWidget>// -!F: ?

#include <qcolor.h>
#include <qpen.h>

#include "zplutils.h"

enum ERtti {
    eRtti_Barcode,
    eRtti_Image,
    eRtti_Line,
    eRtti_Rect,
    eRtti_Text,
    //NY33
    eRtti_TextLine
    //NY33
};

class QDomElement;
class QPainter;
class QPaintDevice;
class QTextStream;
class TCanvasItem;
class TokenProvider;

/**
 * Class DocumentItem
 * Every object which can be placed on a document has to be a subclass of this class.
 */
class DocumentItem : public QObject, protected XMLUtils {
public:
    DocumentItem ( );
    virtual ~DocumentItem() { };
    
    /**
     * 
     * @param element 
     */
    virtual void loadXML (QDomElement* element);
        
    
    /**
     * save this item to XML
     */
    virtual void saveXML (QDomElement* element);
        
    
    /**
     * Draws the item
     */
    virtual void  draw (QPainter* painter) = 0;
    
    /**
     * Draws the item on the preview. Added by Frank. Differs from draw() by coordinates that are used to paint at.
     */
    virtual void drawPreview (QPainter* painter) = 0;
    /*virtual void drawPreview (QPainter* painter) {};*/// -!F: an alternative to the pure virtual function
        
    
    /**
     * Draw a border around the item, has to be reimplemented for round items or barcodes which do not allow borders.
     */
    virtual void drawBorder (QPainter* painter);
    
    /**
     * Draw a border around the item on the label of the preview dialog, has to be reimplemented for round items or barcodes which do not allow borders.
     * Added by Frank. 
     */
    virtual void drawBorderPreview( QPainter* painter );
        
    virtual int  rtti () const = 0;

    /** 
     * Write a ZPL string for drawing this item onto a zebra printer to a QTextStream
     */
    virtual void drawZpl( QTextStream* stream ) = 0;

    /** 
     * Write a IPL string for drawing this item onto a zebra printer to a QTextStream
     */
    virtual void drawIpl( QTextStream* stream, IPLUtils* utils ) = 0;
        
    /** 
     * Write a EPCL string for drawing this item onto a zebra printer to a QTextStream
     */
    virtual void drawEPcl( QTextStream* stream ) = 0;
    
    void setCanvasItem( TCanvasItem* item );
    TCanvasItem* canvasItem() const;
    
    void setPaintDevice( QPaintDevice* device );// -!F: original
    /*void setPaintDevice( QDesktopWidget* device );*/// -!F: delete, 
    QPaintDevice* paintDevice() const;// -!F: original
    /*QDesktopWidget* paintDevice() const;*/// -!F: delete, 
    
    void setTokenProvider( TokenProvider* token );
    TokenProvider* tokenProvider() const;
    
    /**
     * 
     * @param b 
     */
    void  setBorder (bool b);
    bool  border ()  const;
        
    void setPen( const QPen& pen );
    QPen pen() const;

    /** move the documentitem using pixel coordinates on the current 
      * paint device
      */
    void move( int x, int y );
    /** move the documentitem using 1/1000th of a millimeter coordinates 
      */
    void moveMM( int x, int y );

    /** set the width and height of this item using pixel coordinates
      * of the current paint device
      */
    void setSize( int w, int h );
    /** set the width and height of this item using 1/1000th of a
      * millimeter coordinates
      */
    void setSizeMM( int w, int h );
    
    /** The bounding rectangle. I.e. the rectangle including
      * the border width 
      */
    void setBoundingRect( const QRect & rect );
    /** The bounding rectangle. I.e. the rectangle including
      * the border width 
      */
    QRect boundingRect() const;

    /** The drawing rectangle. I.e. the rectangle with the
      * border width substracted
      */
    void setRect( const QRect & rect );
    /** The drawing rectangle. I.e. the rectangle with the
      * border width substracted
      */
    QRect rect() const;

    /** the bounding rectangle including the border in 1/1000mm
      */
    void setRectMM( const QRect & rect );
    /** the bounding rectangle including the border in 1/1000mm
      */
    QRect rectMM() const;
    
    /** set the item to be locked in the label editor
      * i.e protect it from being move or resized by the user
      */
    void setLocked( bool locked );
    const bool locked() const;

    /** set a javascript script for this item which is evaluate 
     *  to determine wether this item should be printed (only in Label
     *  as items are always shown in the labeleditor.
     *  The script shall return true if the item should be visible.
     *  If script returns false the item will not be printed.
     */
    inline void setVisibilityScript( const QString & script );
    inline const QString visibilityScript() const;
    
    int z() const;
    void setZ( int z );
    
    int additionOrder() const;
    void setAdditionOrder( int additionOrder );

    /** Only the z index is compared
      */
    bool operator<( const DocumentItem & docitem )
    {
        return m_z < docitem.m_z;
    }
    
    /** Only the z index is compared
      */
    bool operator==( const DocumentItem & docitem )
    {
        return m_z == docitem.m_z;
    }
private:
    void init();
            
private:
    /**
     * Defines wether this item has a border or not
     */
     bool m_border;
     QPen m_pen;
     QRect m_rect;
     int m_z;
     int m_additionOrder;
     bool m_locked;

    /**
     * Background color of this document item
     */
     QColor m_background;

     QString m_visibilityScript;

     QPaintDevice* m_device;// -!F: original, keep
     /*QDesktopWidget* m_device;*/// -!F: delete, 
     TCanvasItem* m_canvasitem;
     TokenProvider* m_token;
};

void DocumentItem::setVisibilityScript( const QString & script )
{
    m_visibilityScript = script;
}

const QString DocumentItem::visibilityScript() const
{
    return m_visibilityScript;
}

typedef QList<DocumentItem*> DocumentItemList;

#endif //DOCUMENTITEM_H

