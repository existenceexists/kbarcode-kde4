/***************************************************************************
                          xmlutils.h  -  description
                             -------------------
    begin                : Mit Mai 7 2003
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

#ifndef XMLUTILS_H
#define XMLUTILS_H

#include <qmap.h>

class DocumentItem;
class BarcodeItem;
class Barkode;
class Definition;
class TokenProvider;
class MyCanvasView;
class QColor;
class QDomDocument;
class QDomElement;
class QDomNode;
class QRect;
class QString;
class QWidget;
typedef QList<DocumentItem*> DocumentItemList;

/** This class provides helper function for saving and reading to XML files.
  *
  * @author Dominik Seichter
  */
class XMLUtils {
    public: 
        XMLUtils();
        ~XMLUtils();

        void readXMLHeader( QDomDocument* doc, QString & description, bool & kbarcode18, Definition** def );
        void writeXMLHeader( QDomNode* root, const QString & description, Definition* def );
                
        void readDocumentItems( DocumentItemList* list, QDomDocument* doc, TokenProvider* token, bool kbarcode18 );

        void writeXMLDocumentItem( QDomElement* root, DocumentItem** item );
        bool readXMLDocumentItem( QDomElement* tag, DocumentItem** item, TokenProvider* token );
        
        void writeXMLColor( QDomElement* tag, const QString & prefix, QColor c ) const;
        QColor readXMLColor( QDomElement* tag, const QString & prefix, QColor c );

        //void writeXMLRect( QDomElement* tag, QRect r, QPoint ptOrigin ) const;
        QRect readXMLRect( QDomElement* tag );

        void writeDefinition( QDomElement* tag, Definition* d ) const;
        Definition* readDefinition( QDomElement* tag );

        void writeBarcode( QDomElement* tag, const Barkode* data, bool cache = false ) const; 
        void readBarcode( QDomElement* tag, Barkode* bcode );

    private:
        static QMap<QString,QString> legacy;
};

#endif
