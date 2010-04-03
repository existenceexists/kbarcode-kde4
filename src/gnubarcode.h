/***************************************************************************
                          gnubarcode.h  -  description
                             -------------------
    begin                : 
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

#ifndef GNU_BARCODE_H
#define GNU_BARCODE_H

// #define _ENABLE_NATIVE_GNU_BARCODE

#ifdef _ENABLE_NATIVE_GNU_BARCODE

#include "barkodeengine.h"

struct Barcode_Item;

class GnuBarcode : public BarkodeEngine {
 public:
    GnuBarcode();
    ~GnuBarcode();

    inline EEngine engine() const;

    const QSize size() const;
    void update( const QPaintDevice* device );
    void drawBarcode( QPainter & painter, int x, int y );

 private:
    /** Draws the barcodes bars if @p painter is not 0.
      * Otherwise only the required width is returned.
      */
    int drawBars( QPainter* painter, int x, int y );

    /** Draws the text if @p painter is not 0.
      * Otherwise only the required width is returned.
      */
    int drawText( QPainter* painter, int x, int y );
    
    /** Fill m_size with the needed space
      */
    void setupSize();
 
 private:
    int m_bar_height;
    int m_barcode_height;
    int m_font_size;
    
    double m_scalex;
    double m_scaley;
    double m_scale_ps_x;
    double m_scale_ps_y;
        
    Barcode_Item* m_item;
    QSize m_size;
};

EEngine GnuBarcode::engine() const
{
    return GNU_BARCODE;
}

#endif // _ENABLE_NATIVE_GNU_BARCODE
#endif /* GNU_BARCODE_H */
