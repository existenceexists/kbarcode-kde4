/***************************************************************************
                          barkodeengine.h  -  description
                             -------------------
    begin                : Fri Nov 05 2004
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

#ifndef BARKODEENGINE_H
#define BARKODEENGINE_H

/**
 * The different generator backends / engines
 */
typedef enum { GNU_BARCODE, PDF417, TBARCODE, TBARCODE2, PIXMAP, PURE_POSTSCRIPT, NONE } EEngine;

class Barkode;
class QDomElement;
class QPainter;

/** An interface for additional special options
 *  supported by a BarkodeEngine
 */
class BarkodeEngineOptions {
 public:
    BarkodeEngineOptions() {};
    virtual ~BarkodeEngineOptions() {};

    virtual void defaults() = 0;

    virtual void load( const QDomElement* tag ) = 0;
    virtual void save( QDomElement* tag ) = 0;
};

#include <qsize.h>

class QPaintDevice;

/**
  * Inherit from this class if you want to create a
  * (barkode engine) for use with KBarcode.
  * @author Dominik Seichter
  */
class BarkodeEngine{
    public:
        BarkodeEngine();
        virtual ~BarkodeEngine();

        virtual const BarkodeEngine & operator=( const BarkodeEngine & rhs ) = 0;

        virtual EEngine engine() const = 0;
        virtual void update( const QPaintDevice* device ) = 0;
        virtual const QSize size() const = 0;

        void setBarkode( Barkode* b ) { barkode = b; }
        virtual void drawBarcode(  QPainter & painter, int x, int y ) = 0;

        /** @returns a pointer to a BarkodeEngineOptions object
         *  if this BarkodeEngine has one. Otherwise null is returned.
         *  The returned object has to be casted down to the concrete object
         *  you want to use, before setting any of the options.
         */
        virtual BarkodeEngineOptions* options();

    protected:
        Barkode* barkode;
        QSize m_size;
        
        bool m_valid;
};

/** A barcode engine with implemented empty and does
  * nothing
  */
class EmptyEngine : public BarkodeEngine {
    public:
        EmptyEngine();

        virtual inline const BarkodeEngine & operator=( const BarkodeEngine & rhs );
        virtual inline EEngine engine() const;
        virtual void update( const QPaintDevice* device );
        virtual const QSize size() const;
        virtual void drawBarcode(  QPainter & painter, int x, int y );
};

const BarkodeEngine & EmptyEngine::operator=( const BarkodeEngine & )
{
    return *this;
}

EEngine EmptyEngine::engine() const
{
    return NONE;
}

#endif
