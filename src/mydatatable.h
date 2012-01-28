/***************************************************************************
                          mydatatable.h  -  description
                             -------------------
    begin                : Son Jun 16 2002
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

#ifndef MYDATATABLE_H
#define MYDATATABLE_H

#include <qwidget.h>
#include <q3datatable.h>

/** A table to modify SQL tables.
  */
class MyDataTable : public Q3DataTable  {
   Q_OBJECT
   public:
        MyDataTable(QWidget *parent=0);
        ~MyDataTable();

        bool update();
        bool deleteItem();
        bool newItem();
};

#endif
