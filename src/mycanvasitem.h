/***************************************************************************
                          mycanvasitem.h  -  description
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

#ifndef MYCANVASITEM_H
#define MYCANVASITEM_H

#include <q3canvas.h>
#include <qpoint.h>
#include <qimage.h>
#include <q3picture.h>
//Added by qt3to4:
#include <QPixmap>
#include "tokenprovider.h"

#define SPOTSIZE 7

class QPixmap;
/** A very small singleton, which creates a pixmap
  * for the accesor points in the label editor.
  *
  * @author Dominik Seichter
  */
class SpotProvider {
    public:
        static SpotProvider* getInstance();

        const QPixmap* spot() const {
            return m_spot;
        }

    private:
        SpotProvider();
        ~SpotProvider();

        static SpotProvider* instance;
        static QPixmap* m_spot;
};


#endif
