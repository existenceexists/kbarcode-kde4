/***************************************************************************
                          measurements.cpp  -  description
                             -------------------
    begin                : Mit Dec 24 2003
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

#include "measurements.h"

// Qt includes
#include <QPaintDevice>

// KDE includes
#include <kglobal.h>

Measurements::Measurements()
{
    Measurements::init();

    defaultMeasurements();
}

Measurements::Measurements( const Measurements & rhs )
{
    Measurements::init();

    this->operator=( rhs );
}


Measurements::~Measurements() {}

int Measurements::m_system = None;
QString Measurements::m_string = QString::null;

void Measurements::init()
{
    if( m_system == None ) {
        m_system = KGlobal::locale()->measureSystem();
        m_string = (m_system == Metric) ?
                   KGlobal::staticQString( i18n("mm" ) ) :
                   KGlobal::staticQString( i18n("in" ) );
    }
}

void Measurements::defaultMeasurements() {
    // init everything in case of
    // database connection fails.
    gap_h = 40.0;
    gap_left = 0.0;
    gap_top = 0.0;
    gap_v = 40.0;
    m_height = 40.0;
    num_h = 1;
    num_v = 1;
    m_width = 40.0;
}

void Measurements::operator=( const Measurements & m ) {
    num_h    = m.num_h;
    num_v    = m.num_v;
    gap_left = m.gap_left;
    gap_top  = m.gap_top;
    gap_v    = m.gap_v;
    gap_h    = m.gap_h;
    m_width  = m.m_width;
    m_height = m.m_height;
}

double Measurements::mmToPixel( double mm, const QPaintDevice* device, int mode ) const {
    if( mode == DpiX )
        return (mm / 25.4) * device->logicalDpiX();
    else
        return (mm / 25.4 ) * device->logicalDpiY();
}

double Measurements::gapLeft( const QPaintDevice* device ) const {
    return mmToPixel( gap_left, device, DpiX );
}

double Measurements::gapTop( const QPaintDevice* device ) const {
    return mmToPixel( gap_top, device, DpiY );
}

double Measurements::gapV( const QPaintDevice* device ) const {
    return mmToPixel( gap_v, device, DpiX );
}

double Measurements::gapH( const QPaintDevice* device ) const {
    return mmToPixel( gap_h, device, DpiY );
}

double Measurements::width( const QPaintDevice* device ) const {
    return mmToPixel( m_width, device, DpiX );
}

double Measurements::height( const QPaintDevice* device ) const {
    return mmToPixel( m_height, device, DpiY );
}

double Measurements::gapLeft() const {
    return (gapLeftMM() / (m_system == Imperial ? 25.4 : 1 ));
}

double Measurements::gapTop() const {
    return (gapTopMM() / (m_system == Imperial ? 25.4 : 1 ));
}

double Measurements::gapV() const {
    return (gapVMM() / (m_system == Imperial ? 25.4 : 1 ));
}

double Measurements::gapH() const {
    return (gapHMM() / (m_system == Imperial ? 25.4 : 1 ));
}

double Measurements::width() const {
    return (widthMM() / (m_system == Imperial ? 25.4 : 1 ));
}

double Measurements::height() const {
    return (heightMM() / (m_system == Imperial ? 25.4 : 1 ));
}

void Measurements::setGapLeft( double d )
{
    gap_left = (d * (m_system == Imperial ? 25.4 : 1 ));
}

void Measurements::setGapTop( double d )
{
    gap_top = (d * (m_system == Imperial ? 25.4 : 1 ));
}

void Measurements::setGapV( double d )
{
    gap_v = (d * (m_system == Imperial ? 25.4 : 1 ));
}

void Measurements::setGapH( double d )
{
    gap_h = (d * (m_system == Imperial ? 25.4 : 1 ));
}

void Measurements::setWidth( double d )
{
    m_width = (d * (m_system == Imperial ? 25.4 : 1 ));
}

void Measurements::setHeight( double d )
{
    m_height = (d * (m_system == Imperial ? 25.4 : 1 ));
}
