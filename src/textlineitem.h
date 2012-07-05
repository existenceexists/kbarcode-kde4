/***************************************************************************
                         textilinetem.h  -  description
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

#ifndef TEXTLINEITEM_H
#define TEXTLINEITEM_H
#include <qstring.h>
#include <QTextStream>

#include "documentitem.h"

/**
 * Class TextLineItem
 * plain text (drawn using either DSRichText or QSimpleRichText - once it is bug free) on screen or printer.
 */
class TextLineItem : public DocumentItem {
public:
    TextLineItem ();
        
    void  draw (QPainter* painter);
    void drawPreview(QPainter* painter);
    void drawZpl( QTextStream* stream );
    void drawIpl( QTextStream* stream, IPLUtils* utils );
    void drawEPcl( QTextStream* stream );
    int  rtti () const { return eRtti_TextLine; }

    void  loadXML (QDomElement* element);
    void  saveXML (QDomElement* element);
    
    void  setText (const QString & text);
    void  setFont (int index);
    int   getFont () const; 
    void setMagHor (int count);
    int getMagHor () const;
    int getMagVert () const;
    void setMagVert (int count);
    QString  text () const;
    
    static bool IsQtTextRenderingBroken();
private:
    void init();
    
private:
    static bool s_qt_broken;
    static bool s_qt_broken_init;
    
    QString m_text;
    int m_font;
    int m_rotate;
    int m_mag_vert;
    int m_mag_hor;
};
#endif //TEXTITEM_H

