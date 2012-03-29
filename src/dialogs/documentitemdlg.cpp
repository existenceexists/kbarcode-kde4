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
#include <QWidget>

#include <k3command.h>
#include <klocale.h>
#include <kpagedialog.h>
#include <kpagewidgetmodel.h>
#include <kvbox.h>

DocumentItemDlg::DocumentItemDlg( TokenProvider* token, DocumentItem* item, K3CommandHistory* history, QWidget* parent )
    : KPageDialog( parent )
{
    setFaceType( KPageDialog::Tabbed );
    setCaption( i18n( "Properties" ) );
    /*setButtons( KDialog::Ok|KDialog::Cancel );*/// -!F: Is it needed? try to uncomment
    /*setDefaultButton( KDialog::Ok );*/// -!F: Is it needed? try to uncomment
    setModal( true );
    showButtonSeparator( true );
    
    m_item = item;
    m_history = history;
    //m_list.setAutoDelete( false );
    
     /*KVBox* boxBorder = addVBoxPage(i18n("&Position && Size"), QString::null, QPixmap() );
     addPage( new PropertySize( boxBorder ) );*/// -!F: original, delete
     KVBox* boxBorder1 = new KVBox();
     KPageWidgetItem *itemPage = addPage( boxBorder1, i18n( "&Position && Size" ) );
     addPagePropertyWidget( new PropertySize( itemPage->widget() ) );
    
     /*boxBorder = addVBoxPage(i18n("&Border"), QString::null, QPixmap() );
     addPage( new PropertyBorder( boxBorder ) );*/// -!F: original, delete
     KVBox* boxBorder2 = new KVBox();
     itemPage = addPage( boxBorder2, i18n( "&Border" ) );
     addPagePropertyWidget( new PropertyBorder( itemPage->widget() ) );
     
     if( m_item->rtti() == eRtti_Rect )
     {
        /*boxBorder = addVBoxPage(i18n("&Fill Color"), QString::null, QPixmap() );
        addPage( new PropertyFill( boxBorder) );*/// -!F: original, delete
        KVBox* boxBorder3 = new KVBox();
        itemPage = addPage( boxBorder3, i18n( "&Fill Color" ) );
        addPagePropertyWidget( new PropertyFill( itemPage->widget() ) );
     } 
     else if ( m_item->rtti() == eRtti_Barcode )
     {
        /*boxBorder = addVBoxPage(i18n("&Barcode"), QString::null, QPixmap() );
        addPage( new PropertyBarcode( token, boxBorder) );*/// -!F: original, delete
        KVBox* boxBorder3 = new KVBox();
        itemPage = addPage( boxBorder3, i18n( "&Barcode" ) );
        addPagePropertyWidget( new PropertyBarcode( token, itemPage->widget() ) );
     }
     else if ( m_item->rtti() == eRtti_Text )
     {
        /*boxBorder = addVBoxPage(i18n("&Rotation"), QString::null, QPixmap() );
        addPage( new PropertyRotation(  boxBorder ) );*/// -!F: original, delete
        KVBox* boxBorder3 = new KVBox();
        itemPage = addPage( boxBorder3, i18n( "&Rotation" ) );
        addPagePropertyWidget( new PropertyRotation( itemPage->widget() ) );

        /*boxBorder = addVBoxPage(i18n("&Text"), QString::null, QPixmap() );
        addPage( new PropertyText( token, boxBorder) );*/// -!F: original, delete
        KVBox* boxBorder4 = new KVBox();
        itemPage = addPage( boxBorder4, i18n( "&Text" ) );
        addPagePropertyWidget( new PropertyText( token, itemPage->widget() ) );
     }
     else if( m_item->rtti() == eRtti_Image )
     {
        /*boxBorder = addVBoxPage(i18n("&Image"), QString::null, QPixmap() );
        addPage( new PropertyImage( token, boxBorder) );*/// -!F: original, delete
        KVBox* boxBorder3 = new KVBox();
        itemPage = addPage( boxBorder3, i18n( "&Image" ) );
        addPagePropertyWidget( new PropertyImage( token, itemPage->widget() ) );
     }
//NY19
     else if ( m_item->rtti() == eRtti_TextLine )
     {
        /*boxBorder = addVBoxPage(i18n("&Text"), QString::null, QPixmap() );
        addPage( new PropertyTextLine( token, boxBorder) );*/// -!F: original, delete
        KVBox* boxBorder3 = new KVBox();
        itemPage = addPage( boxBorder3, i18n( "&Text" ) );
        addPagePropertyWidget( new PropertyTextLine( token, itemPage->widget() ) );
     }
//NY19

     /*showPage( pageIndex( boxBorder ) );*/// -!F: original, delete
    setCurrentPage( itemPage );

     // Add it after the call to showPage
     // so that this page is not always shown
     // as default page
     if( TokenProvider::hasJavaScript() )
     {
	 /*boxBorder = addVBoxPage(i18n("&Visibility"), QString::null, QPixmap() );
	 addPage( new PropertyVisible( boxBorder ) );*/// -!F: original, delete
        KVBox* boxBorder5 = new KVBox();
        itemPage = addPage( boxBorder5, i18n( "&Visibility" ) );
        addPagePropertyWidget( new PropertyVisible( itemPage->widget() ) );
     }
     
     std::list<PropertyWidget*>::iterator it;
     for( it=m_list.begin();it!=m_list.end();it++)
     {
        (*it)->initSettings( m_item );
     }

     /*resize( configDialogSize("DocumentItemDlg") );*/// -!F: original, delete
     /*KConfigGroup* config = KConfigGroup();
     restoreDialogSize( config );*/// -!F: added, adjust
}

DocumentItemDlg::~DocumentItemDlg()
{
    /*saveDialogSize("DocumentItemDlg");*/// -!F: original, delete
    /*saveDialogSize("DocumentItemDlg");*/// -!F: added, adjust

    std::list<PropertyWidget*>::iterator it;
    for( it=m_list.begin();it!=m_list.end();it++)
    {
        delete (*it);
    }
}

void DocumentItemDlg::addPagePropertyWidget( PropertyWidget* widget )
{
    m_list.push_back( widget );
}

void DocumentItemDlg::accept()
{
    K3MacroCommand* mc = new K3MacroCommand( i18n("Property changed") );
    std::list<PropertyWidget*>::iterator it;
    for( it=m_list.begin();it!=m_list.end();it++)
    {
       (*it)->applySettings( m_item, mc );
    }
    
    m_history->addCommand( mc, false );
    
    KPageDialog::accept();
}

#include "documentitemdlg.moc"
