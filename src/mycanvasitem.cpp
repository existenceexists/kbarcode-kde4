/***************************************************************************
                          mycanvasitem.cpp  -  description
                             -------------------
    begin                : Die Apr 23 2002
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

#include "mycanvasitem.h"
#include "labelutils.h"
#include "labeleditor.h"

// KDE includes
#include <kapplication.h>

// QT includes
#include <qbitmap.h>
#include <qpainter.h>
//Added by qt3to4:
#include <QPixmap>

SpotProvider::SpotProvider()
{
    m_spot = new QPixmap( SPOTSIZE, SPOTSIZE );
    m_spot->fill( Qt::green );

    QPainter p( m_spot );
    p.drawRect( 0, 0, SPOTSIZE, SPOTSIZE );
    p.end();
}

SpotProvider::~SpotProvider()
{
    delete m_spot;
}

SpotProvider* SpotProvider::instance = 0;
QPixmap* SpotProvider::m_spot = 0;

SpotProvider* SpotProvider::getInstance()
{
    if( !instance )
        instance = new SpotProvider();

    return instance;
}

