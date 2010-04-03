/***************************************************************************
                         pixmapbarcode.cpp  -  description
                             -------------------
    begin                : Mon Nov 22 2004
    copyright            : (C) 2004 by Dominik Seichter
    email                : domseichter@web.de
 ***************************************************************************/

/***************************************************************************
                                                                          
    This program is free software; you can redistribute it and/or modify  
    it under the terms of the GNU General Public License as published by  
    the Free Software Foundation; either version 2 of the License, or     
    (at your option) any later version.                                   
                                                                          
 ***************************************************************************/

#ifndef PIXMAPBARCODE_H
#define PIXMAPBARCODE_H

#include "barkodeengine.h"
#include <qpixmap.h>

class KTemporaryFile;
class QPainter;

class PDF417Options : public BarkodeEngineOptions {
    public:
        PDF417Options();

        const PDF417Options& operator=( const BarkodeEngineOptions& rhs );
        void defaults();

        void load( const QDomElement* tag );
        void save( QDomElement* tag );

        inline int row() const { return m_row; }
        inline int col() const { return m_col; }
        inline int err() const { return m_err; }
        
        inline void setRow( int r ) { m_row = r; }
        inline void setCol( int r ) { m_col = r; }
        inline void setErr( int r ) { m_err = r; }
        
    private:
        int m_row;
        int m_col;
        int m_err;
};

/**
  * A pixmap based barcode engine which 
  * uses different commandline tools to 
  * generate barcodes.
  * This class is a port of the old BarCode
  * class to the new Barkode interface.
  *
  * @author Dominik Seichter
  */
class PixmapBarcode : public BarkodeEngine
{
    public:
        PixmapBarcode();
        ~PixmapBarcode();

        const PixmapBarcode & operator=( const BarkodeEngine & rhs );

        inline EEngine engine() const;
        const QSize size() const;
        void update( const QPaintDevice* device );
        void drawBarcode( QPainter & painter, int x, int y );

        inline BarkodeEngineOptions* options();
        
    private:
        bool createPixmap( QPixmap* target, int resx, int resy );
        void createBarcode( QPixmap* target, const QPaintDevice* device );
        bool createPdf417( KTemporaryFile* output );
        //QString createTBarcodeCmd();

        virtual bool createPostscript( char** postscript, long* postscript_size );

        /** Mage sure the temporaray file @p file 
          * gets closed and deleted properly
          */
        void cleanUp( KTemporaryFile* file, QPixmap* target );
        /**
          * Cut the barcode either on the top or
          * on the bottom, depending on the text position.
          *g
          * @param pic the QPixmap that will be cutted
          * @param cut a value between 0.0 and 1.0. If cut = 1.0
          *        then no cut will happen, if cut = 0.5, half of
          *        the barcode is going to be cutted away.
          */
        QPixmap cut( QPixmap* pic, double cut );
        
        /**
          * Add a margin to the QPixmap pic.
          * @param pic the QPixmap pic which should get a margin
          */
        QPixmap addMargin( QPixmap* pic );

    protected:
        /** Creates a pipe and read the data returned by the command into a buffer.
         *  \param command the command to execute
         *  \param buffer pointer to a buffer, will be malloc'ed and has to be free'd
         *  \param buffer_size pointer to a long where the size of the buffer will be stored
         *  \returns true if all data could be read sucessfully otherwise false.
         */
        bool readFromPipe( const char* command, char** buffer, long* buffer_size );

        /**
         *  \param postscript a postscript program
         *  \param postscript_size length of the postscript program
         *  \returns the BoundingBox of the postscript or EPS file
         *           so that it can be cropped correctly
         */
        virtual QRect bbox( const char* postscript, long postscript_size );

    private:
        QPixmap p;
        PDF417Options m_pdf417_options;
};

EEngine PixmapBarcode::engine() const
{
    return PIXMAP;
}

BarkodeEngineOptions* PixmapBarcode::options()
{
    return &m_pdf417_options;
}

#endif
