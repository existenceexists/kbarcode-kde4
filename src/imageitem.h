/***************************************************************************
                         imageitem.h  -  description
                             -------------------
    begin                : Do Sep 9 2004
    copyright            : (C) 2004 by Dominik Seichter
    email                : domseichter@web.de
 ***************************************************************************/

/***************************************************************************
                                                                          
    This program is free software; you can redistribute it and/or modify  
    it under the terms of the GNU General Public License as published by  
    the Free Software Foundation; either version 2 of the License, or     
    (at your option) any later version.                                   
                                                                          
 ***************************************************************************/
#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include "documentitem.h"

#include <qpixmap.h>
#include <QTextStream>

enum EImageScaling { eImage_Original = 0, eImage_Scaled, eImage_Zoomed };

/**
@author Dominik Seichter
*/
class ImageItem : public DocumentItem
{
 public:
    ImageItem();
    ~ImageItem();

    void setExpression( const QString & ex );
    inline QString expression() const { return m_expression; };

    void setPixmap( const QPixmap & pix );
    const QPixmap & pixmap();
    
    void setScaling( EImageScaling scaling );
    EImageScaling scaling() const;
    
    void setRotation( double rot );
    double rotation() const;

    void setMirrorHorizontal( bool b );
    bool mirrorHorizontal() const;
    
    void setMirrorVertical( bool b );
    bool mirrorVertical() const;
    
    virtual int rtti() const { return eRtti_Image;}
    virtual void draw(QPainter* painter);
    virtual void drawZpl( QTextStream* stream );
    virtual void drawIpl( QTextStream* stream, IPLUtils* utils );
    virtual void drawEPcl( QTextStream* stream );
    virtual void loadXML(QDomElement* element);
    virtual void saveXML(QDomElement* element);
 
    void updateImage();
    void createImage();
    
 private:
    void init();
    
 private:
    QString m_expression;
    QPixmap m_pixmap;
    QPixmap m_tmp;
    
    double m_rotation;
    bool m_mirror_v;
    bool m_mirror_h;
    
    bool createNewImage;
       
    EImageScaling m_scaling;
};

#endif
