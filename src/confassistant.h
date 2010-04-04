/***************************************************************************
                          confassistant.h  -  description
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

#ifndef CONFASSISTANT_H
#define CONFASSISTANT_H

#include <kassistantdialog.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

class QGroupBox;
class QCheckBox;
class QVBoxLayout;
class QHBoxLayout; 
class QGridLayout;
class QRadioButton;
class KComboBox;
class KLineEdit;
class KPushButton;
class KUrlLabel;
class QLabel;
class QWidget;
class SqlWidget;
/** KBarcodes configuration assistant.
  */
class ConfAssistant : public KAssistantDialog
{ 
    Q_OBJECT

    public:
        ConfAssistant( QWidget* parent = 0, bool modal = true );
        ~ConfAssistant();

        void showPage( QWidget* page );

        // used in mainwindow.cpp
        // not a clean API, but I am lazy :-(
        
        // TODO: rewrite it cleanly
        QCheckBox* checkDatabase;

    private slots:
        void testSettings( bool b );
        void create();
        void example();
        void useDatabase();

    protected slots:
        void accept();
        
    private:
        void setupPage1();
        void setupPage0();
        void setupPage2();
        void setupPage3();

        SqlWidget* sqlwidget;
        
        QWidget* page;
        QLabel* logo;
        QLabel* TextLabel2_2;
        KUrlLabel* KUrlLabel1;
        QWidget* page_2;
        QLabel* TextLabel1;
        QLabel* TextLabel2;
        QLabel* TextLabel3;
        QLabel* TextLabel4;
        QLabel* TextLabel5;
        QLabel* TextLabel6;
        KPushButton* buttonTest;
        QWidget* page_3;
        QLabel* TextLabel1_2;
        KPushButton* buttonCreate;
        KPushButton* buttonExample;
      
    protected:
        QVBoxLayout* pageLayout;
        QHBoxLayout* Layout8;
        QVBoxLayout* Layout7;
        QVBoxLayout* pageLayout_2;
        QVBoxLayout* pageLayout_4;
        QHBoxLayout* Layout5;
        QVBoxLayout* Layout3;
        QVBoxLayout* Layout4;
        QVBoxLayout* Layout6;
        QVBoxLayout* pageLayout_3;
        QVBoxLayout* Layout5_2;
};

#endif // CONFASSISTANT_H
