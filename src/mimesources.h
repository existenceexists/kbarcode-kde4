/***************************************************************************
                          mimesources.h  -  description
                             -------------------
    begin                : Son Sep 14 2003
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

#ifndef MIMESOURCES_H
#define MIMESOURCES_H

#include <QMimeData>
#include "documentitem.h"

class KUndoStack;
class TokenProvider;
class MyCanvasView;

class DocumentItemDrag : public QMimeData {
    Q_OBJECT
    public:
        DocumentItemDrag();
        
        static QString mimeType();
        
        void setDocumentItem( DocumentItemList* list );

        static bool canDecode( QMimeData * );
        static bool decode( QMimeData *, MyCanvasView* cv, TokenProvider* token, KUndoStack* history );
};

#endif
