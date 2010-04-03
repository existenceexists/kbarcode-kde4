/***************************************************************************
                          gnubarkodeengine.cpp  -  description
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
#include "gnubarkodeengine.h"
#include <barcode.h>

GnuBarkodeEngine::GnuBarkodeEngine()
    : BarkodeEngine( "libbarcode.so" )
{
}

void GnuBarkodeEngine::init()
{   
    if( !m_loaded )
    {
        (void*)Barcode_Create = dlsym(m_handle, "Barcode_Create");
        (void*)Barcode_Delete = dlsym(m_handle, "Barcode_Delete");
        (void*)Barcode_Encode = dlsym(m_handle, "Barcode_Encode");
        
        m_loaded = true;
    }
}

GnuBarkodeEngine gnuBarkodeEngine;
