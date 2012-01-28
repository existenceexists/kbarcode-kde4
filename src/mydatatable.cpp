/***************************************************************************
                          mydatatable.cpp  -  description
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

#include "mydatatable.h"

#include <Q3DataTable>

MyDataTable::MyDataTable(QWidget *parent )
    : Q3DataTable(parent)
{ }

MyDataTable::~MyDataTable()
{
}

bool MyDataTable::update()
{
    return updateCurrent();
}


bool MyDataTable::deleteItem()
{
    return deleteCurrent();
}

bool MyDataTable::newItem()
{
    return insertCurrent();
}

#include "mydatatable.moc"
