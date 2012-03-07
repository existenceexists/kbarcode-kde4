/***************************************************************************
                          mimesources.cpp  -  description
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

#include "mimesources.h"
#include "mycanvasview.h"
#include "documentitem.h"
#include "commands.h"
#include "xmlutils.h"

#include <k3command.h>

#include <qbuffer.h>
#include <qdom.h>
#include <QString>
#include <QMimeData>
#include <QMimeSource>

class DragCommand : public NewItemCommand {
    public:
        DragCommand( MyCanvasView* view, DocumentItem* doc_item )
            : NewItemCommand( view, i18n("Pasted Object") )
        {
            m_doc_item = doc_item;
        }
        
        void create()
        {
            m_object = m_doc_item;
        }
        
    private:
        DocumentItem* m_doc_item;
};



DocumentItemDrag::DocumentItemDrag()
    : QMimeData()
{
    /*setData( DocumentItemDrag::mimeType() );
    this->dragSource = dragSource;*/// -!F: delete
}

const char * DocumentItemDrag::mimeType()// -!F: Replace const char* with QString.
{
    return "application/x-kbarcode-document-item";
}

void DocumentItemDrag::setDocumentItem( DocumentItemList* list )
{
    QByteArray data;
    QBuffer buffer( & data );
    if( buffer.open( QIODevice::WriteOnly ) )
    {
        QDomDocument doc("KBarcodeClipboard");
        QDomElement root = doc.createElement( "root" );
        doc.appendChild( root );
        
        XMLUtils xml;
        for( int i=0;i<list->count();i++)
        {
            DocumentItem* item = list->at( i );
            xml.writeXMLDocumentItem( &root, &item );
        }
                
        QTextStream t( &buffer );
        doc.save( t, 0 );
        
        buffer.close();
        setData( DocumentItemDrag::mimeType(), data );    
    }
}

bool DocumentItemDrag::canDecode( QMimeSource* e )
{
    return e->provides( DocumentItemDrag::mimeType() );
}

bool DocumentItemDrag::decode( QMimeSource* mime, MyCanvasView* cv, TokenProvider* token, K3CommandHistory* history )
{
    QByteArray data = mime->encodedData( DocumentItemDrag::mimeType() );
    QDomDocument doc( "KBarcodeClipboard" );
    if( !doc.setContent( data ) )
        return false;
    
    QDomNode n = doc.documentElement();
    QDomNodeList list = n.childNodes();
    K3MacroCommand* commands = new K3MacroCommand( i18n("Paste") );
    
    for( unsigned int i=0;i<list.length();i++)
    {
        QDomNode n = list.item(i);
        
        QDomElement e = n.toElement();
        if( !e.isNull() )
        {
            XMLUtils xml;
            DocumentItem* item = NULL;
            if( xml.readXMLDocumentItem( &e, &item, token ) )
            {            
                DragCommand* dc = new DragCommand( cv, item );
                dc->execute();
                commands->addCommand( dc );
            }
            else
            {
                delete commands;
                return false;
            }
        }
    }
    
    history->addCommand( commands, false );
        
    return true;
}


#include "mimesources.moc"
