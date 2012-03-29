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

#include "documentitem.h"
#include "propertywidget.h"
#include <list> 
#include <kpagedialog.h>

class K3CommandHistory;
class KPageDialog;

class DocumentItemDlg : public KPageDialog
{
 Q_OBJECT
 public:
    DocumentItemDlg( TokenProvider* token, DocumentItem* item, K3CommandHistory* history, QWidget* parent );
    ~DocumentItemDlg();
    
    void addPagePropertyWidget( PropertyWidget* widget );
 
 protected slots:
    void accept();
    
 private:
    DocumentItem* m_item;
    std::list<PropertyWidget*> m_list;
    
    K3CommandHistory* m_history;
};

#endif
