/***************************************************************************
                         barcodeitem.h  -  description
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

#ifndef BARCODEITEM_H
#define BARCODEITEM_H

#include <qmap.h>
#include <qpixmap.h>
#include <QTextStream>

#include "barkode.h"
#include "gnubarcode.h"
#include "documentitem.h"

/**
 * Class BarcodeItem
 * A barcode on the screen or printer.
 */
class BarcodeItem : public Barkode, public DocumentItem {
public:
    BarcodeItem();
    BarcodeItem( const Barkode & bcode );
    
    void updateBarcode();
    
    int rtti() const { return eRtti_Barcode;} 
        
    void loadXML (QDomElement* element);
    void saveXML (QDomElement* element);
    void draw (QPainter* painter); 
    void drawZpl( QTextStream* stream );    
    void drawIpl( QTextStream* stream, IPLUtils* utils );
    void drawEPcl( QTextStream* stream );
    
private:
    void init();
    
private:
    static QMap<QString,QString> legacy;

    QPixmap m_pixmap;
};
#endif //BARCODEITEM_H

