/***************************************************************************
                          gnubarkodeengine.h  -  description
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

#ifndef GNUBARKODEENGINE_H
#define GNUBARKODEENGINE_H

#include <barkodeengine.h>

struct Barcode_Item;

/**
  * Loads GNU Barcode lib dynamically when needed, 
  * so that KBarcode does not have to link against 
  * it and works even without lib gnu barcode.
  * @author Dominik Seichter
  */
class GnuBarkodeEngine : public BarkodeEngine
{
    public:
        GnuBarkodeEngine();

        struct Barcode_Item* (*Barcode_Create)(char* text);
        int (*Barcode_Delete)(struct Barcode_Item* bc);
        int (*Barcode_Encode)(struct Barcode_Item *bc, int flags);
        
    
    protected:
        void init();
};

extern GnuBarkodeEngine gnuBarkodeEngine;
#endif
