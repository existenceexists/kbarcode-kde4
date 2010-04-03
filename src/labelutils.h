//Added by qt3to4:
#include <QSqlQuery>
#include <QPixmap>
/***************************************************************************
                          labelutils.h  -  description
                             -------------------
    begin                : Sam Okt 26 2002
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

#ifndef LABELUTILS_H
#define LABELUTILS_H

class Definition;
class QPrinter;
class QDomElement;
class QFont;
class QImage;
class QPainter;
class QPixmap;
class QPaintDevice;
class QPoint;
class QRect;
class QSize;
class QString;
class QStringList;
class QSqlQuery;
class LabelUtils {
    public:
        LabelUtils();
        ~LabelUtils();

        enum _dpixy {
            DpiX,
            DpiY
        };

        double pixelToMm( double pixel, const QPaintDevice* device, int mode = DpiX );
        double mmToPixel( double mm, const QPaintDevice* device, int mode = DpiX );

        double pixelToPixelX( double unit, const QPaintDevice* src, const QPaintDevice* dest );
        double pixelToPixelY( double unit, const QPaintDevice* src, const QPaintDevice* dest );
        static QSize stringSize( const QString & t );
        static QPixmap* drawString( const QString & t, int w, int h, double rot = 0 );
        static QPixmap* drawString( const QString & t ) {
            return LabelUtils::drawString( t, 0, 0, 0 );
        }
        static void renderString( QPainter* painter, const QString & t, const QRect & r, double scalex, double scaley );

        const QString getTypeFromCaption( const QString & cap );
        const QString getModeFromCaption( const QString & cap );
};

#endif
