/***************************************************************************
                          databasebrowser.h  -  description
                             -------------------
    begin                : Mit Mai 15 2002
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

#ifndef DATABASEBROWSER_H
#define DATABASEBROWSER_H

#include "mainwindow.h"

/*#include <kfinddialog.h>// -!F: keep, 
#include <kfind.h>*/

class KAction;
//class KEdFind;// -!F: original, delete
class KFindDialog;// -!F: keep
class KFind;// -!F: keep
class KXmlGuiWindow;
class KMenuBar;
class KToolBar;
class KPushButton;
class KMenu;
//class MyDataTable;// -!F: original, delete
class QTableView;
class QSqlTableModel;
class QDataWidgetMapper;
/** A database browser widget. Allows small changes to SQL tables
  * and is mostly used for having a quick look on the tables.
  */
class DatabaseBrowser : public KXmlGuiWindow{
    Q_OBJECT
    public:
        DatabaseBrowser( QString _database, QWidget *parent=0);
        ~DatabaseBrowser();

    private:
        void setupActions();

    private slots:
        void setupSql();

        void cut();
        void copy();
        void paste();

        void find();
        void slotFindNext();

        void import();

    protected:
        QString database;
        /*MyDataTable* table;*/// -!F: original
        QTableView * table;
        QSqlTableModel * model;
        QDataWidgetMapper * mapper;

        KAction* undoAct;
        KAction* deleteAct;
        KAction* newAct;

        KFindDialog * findDlg;
        KFind * findObject;

        QString m_find;
        bool m_direction;
        bool m_case;
};

#endif
