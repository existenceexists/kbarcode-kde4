/***************************************************************************
                         documentitemdlg.cpp  -  description
                             -------------------
    begin                : Do Sep 10 2004
    copyright            : (C) 2004 by Dominik Seichter
    email                : domseichter@web.de
 ***************************************************************************/

/***************************************************************************
                                                                          
    This program is free software; you can redistribute it and/or modify  
    it under the terms of the GNU General Public License as published by  
    the Free Software Foundation; either version 2 of the License, or     
    (at your option) any later version.                                   
                                                                          
 ***************************************************************************/

#include "documentitemdlg.h"
#include "tokenprovider.h"

#include <q3vbox.h>
//Added by qt3to4:
#include <QPixmap>

#include <k3command.h>
#include <klocale.h>

DocumentItemDlg::DocumentItemDlg( TokenProvider* token, DocumentItem* item, KCommandHistory* history, QWidget* parent )
    : KDialogBase( Tabbed, i18n("Properties"), KDialogBase::Ok|KDialogBase::Cancel,
      KDialogBase::Ok, parent, 0, true, true )
{
    m_item = item;
    m_history = history;
    //m_list.setAutoDelete( false );
    
     KVBox* boxBorder = addVBoxPage(i18n("&Position && Size"), QString::null, QPixmap() );
     addPage( new PropertySize( boxBorder ) );
    
     boxBorder = addVBoxPage(i18n("&Border"), QString::null, QPixmap() );
     addPage( new PropertyBorder( boxBorder ) );
     
     if( m_item->rtti() == eRtti_Rect )
     {
        boxBorder = addVBoxPage(i18n("&Fill Color"), QString::null, QPixmap() );
        addPage( new PropertyFill( boxBorder) );
     } 
     else if ( m_item->rtti() == eRtti_Barcode )
     {
        boxBorder = addVBoxPage(i18n("&Barcode"), QString::null, QPixmap() );
        addPage( new PropertyBarcode( token, boxBorder) );
     }
     else if ( m_item->rtti() == eRtti_Text )
     {
        boxBorder = addVBoxPage(i18n("&Rotation"), QString::null, QPixmap() );
        addPage( new PropertyRotation(  boxBorder ) );

        boxBorder = addVBoxPage(i18n("&Text"), QString::null, QPixmap() );
        addPage( new PropertyText( token, boxBorder) );
     }
     else if( m_item->rtti() == eRtti_Image )
     {
        boxBorder = addVBoxPage(i18n("&Image"), QString::null, QPixmap() );
        addPage( new PropertyImage( token, boxBorder) );
     }
//NY19
     else if ( m_item->rtti() == eRtti_TextLine )
     {
        boxBorder = addVBoxPage(i18n("&Text"), QString::null, QPixmap() );
        addPage( new PropertyTextLine( token, boxBorder) );
     }
//NY19

     showPage( pageIndex( boxBorder ) );

     // Add it after the call to showPage
     // so that this page is not always shown
     // as default page
     if( TokenProvider::hasJavaScript() )
     {
	 boxBorder = addVBoxPage(i18n("&Visibility"), QString::null, QPixmap() );
	 addPage( new PropertyVisible( boxBorder ) );
     }
     
     std::list<PropertyWidget*>::iterator it;
     for( it=m_list.begin();it!=m_list.end();it++)
     {
        (*it)->initSettings( m_item );
     }

     resize( configDialogSize("DocumentItemDlg") );
}

DocumentItemDlg::~DocumentItemDlg()
{
    saveDialogSize("DocumentItemDlg");

    std::list<PropertyWidget*>::iterator it;
    for( it=m_list.begin();it!=m_list.end();it++)
    {
        delete (*it);
    }
}

void DocumentItemDlg::addPage( PropertyWidget* widget )
{
    m_list.push_back( widget );
}

void DocumentItemDlg::accept()
{
    KMacroCommand* mc = new KMacroCommand( i18n("Property changed") );
    std::list<PropertyWidget*>::iterator it;
    for( it=m_list.begin();it!=m_list.end();it++)
    {
       (*it)->applySettings( m_item, mc );
    }
    
    m_history->addCommand( mc, false );
    
    KDialogBase::accept();
}

#include "documentitemdlg.moc"
