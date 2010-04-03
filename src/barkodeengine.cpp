/***************************************************************************
                          barkodeengine.cpp  -  description
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

#include "barkodeengine.h"
#include "barkode.h"

BarkodeEngine::BarkodeEngine()
{
    m_valid = false;
    barkode = NULL;
}

BarkodeEngine::~BarkodeEngine()
{
}

BarkodeEngineOptions* BarkodeEngine::options()
{
    return NULL;
}

EmptyEngine::EmptyEngine()
    : BarkodeEngine()
{
}

void EmptyEngine::update( const QPaintDevice* )
{
    // Do nothing
}

const QSize EmptyEngine::size() const
{
    return QSize( 150, 80 );
}

void EmptyEngine::drawBarcode(  QPainter & painter, int x, int y )
{
    barkode->drawInvalid( painter, x, y );
}

