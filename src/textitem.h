/***************************************************************************
                         textitem.h  -  description
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

#ifndef TEXTITEM_H
#define TEXTITEM_H
#include <qstring.h>
#include "documentitem.h"
#include <QTextStream>

class LabelEditor;

/**
 * Class TextItem
 * Formated rich text (drawn using either DSRichText or QSimpleRichText - once it is bug free) on screen or printer.
 */
class TextItem : public DocumentItem {
public:
    TextItem ();
        
    void draw (QPainter* painter);
    void drawPreview( QPainter* painter );
    void drawZpl( QTextStream* stream );
    void drawIpl( QTextStream* stream, IPLUtils* utils );
    void drawEPcl( QTextStream* stream );
    
    int  rtti () const { return eRtti_Text; }

    void  loadXML (QDomElement* element);
    void  saveXML (QDomElement* element);
    
    void  setText (const QString & text);
    QString  text () const;

    inline void setRotation( double rot ) { m_rotation = rot; }
    inline double rotation() const { return m_rotation; }

    static bool IsQtTextRenderingBroken();
private:
    void init();
    
private:
    static bool s_qt_broken;
    static bool s_qt_broken_init;

    double m_rotation;
    QString m_text;
};
#endif //TEXTITEM_H

