/***************************************************************************
                          tbarcode2.h  -  description
                             -------------------
    begin                : Mon Feb 20 2006
    copyright            : (C) 2006 by Dominik Seichter
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

#ifndef TBARCODE2_H
#define TBARCODE2_H

#include "pixmapbarcode.h"

class QDomElement;

/** A small class which contains all the options 
  * which can be configured for tbarcode
  */
class TBarcodeOptions : public BarkodeEngineOptions {
    public:
        TBarcodeOptions();
        
        const TBarcodeOptions& operator=( const BarkodeEngineOptions& rhs );
        void defaults();

        void load( const QDomElement* tag );
        void save( QDomElement* tag );
        
        inline bool escape() const { return m_escape; }
        inline bool above() const { return m_above; }
        inline bool autocorrect() const { return m_autocorrect; }
        inline int checksum() const { return m_checksum; }
        inline double moduleWidth() const { return m_modulewidth; }
        inline int height() const { return m_height; }

        inline void setEscape( bool b ) { m_escape = b; }
        inline void setAbove( bool b ) { m_above = b; }
        inline void setAutocorrect( bool b ) { m_autocorrect = b; }
        inline void setModuleWidth( double d ) { m_modulewidth = d; }
        inline void setCheckSum( int i ) { m_checksum = i; }
        inline void setHeight( int i ) { m_height = i; }
    private:
        bool m_escape;
        bool m_above;
        bool m_autocorrect;
        double m_modulewidth;
        int m_checksum;        
        int m_height;
};

class TBarcode2 : public PixmapBarcode {
 public:
    TBarcode2();
    ~TBarcode2();

    const TBarcode2 & operator=( const BarkodeEngine & rhs );

    inline EEngine engine() const;

    inline BarkodeEngineOptions* options();

 private:
    virtual bool createPostscript( char** postscript, long* postscript_size );
    virtual QRect bbox( const char* postscript, long postscript_size );

 private:
    TBarcodeOptions m_options;
};

EEngine TBarcode2::engine() const
{
    return TBARCODE2;
}

BarkodeEngineOptions* TBarcode2::options()
{
    return &m_options;
}

#endif /* TBARCODE2_H */
