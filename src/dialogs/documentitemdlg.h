/***************************************************************************
                         documentitemdlg.h  -  description
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

#ifndef DOCUMENTITEMDLG_H
#define DOCUMENTITEMDLG_H

#include <kdialogbase.h>
#include "documentitem.h"
#include "propertywidget.h"
#include <list> 

class KCommandHistory;

class DocumentItemDlg : public KDialogBase
{
 Q_OBJECT
 public:
    DocumentItemDlg( TokenProvider* token, DocumentItem* item, KCommandHistory* history, QWidget* parent );
    ~DocumentItemDlg();
    
    void addPage( PropertyWidget* widget );
 
 protected slots:
    void accept();
    
 private:
    DocumentItem* m_item;
    std::list<PropertyWidget*> m_list;
    
    KCommandHistory* m_history;
};

#endif
