/***************************************************************************
                         lineitem.h  -  description
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

#ifndef LINEITEM_H
#define LINEITEM_H
#include <qstring.h>
#include "documentitem.h"
#include "qcolor.h"

/**
 * Class LineItem
 * Implements a line on screen or printer
 */
class LineItem : public DocumentItem {
public:
    LineItem ();
    
    virtual int rtti() const { return eRtti_Line; }
    virtual void draw(QPainter* painter);
    virtual void drawZpl( QTextStream* stream );
    virtual void drawIpl( QTextStream* stream, IPLUtils* utils );
    virtual void drawEPcl( QTextStream* stream );
    virtual void loadXML(QDomElement* element);
    virtual void saveXML(QDomElement* element);
    
private:
    void init();
};
#endif //LINEITEM_H

