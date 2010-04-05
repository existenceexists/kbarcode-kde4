/***************************************************************************
                          tokendialog.h  -  description
                             -------------------
    begin                : Sat Oct 23 2004
    copyright            : (C) 2004 by Dominik Seichter
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

#ifndef TOKENDIALOG_H
#define TOKENDIALOG_H

#include <QWizard>
#include "documentitem.h"
#include "tokenprovider.h"
//Added by qt3to4:
#include <QList>

class KListBox;
class QTreeWidget;
class QListBoxItem;
class QTreeWidgetItem;
class KLineEdit;
class TokenProvider;

class QRadioButton;
class QStackedWidget;
class QWidget;
class KComboBox;
class QTextBrowser;
class KPushButton;
class DSTextEdit;

class TokenDialog : public QWizard {
    
 Q_OBJECT

 public:
    TokenDialog( TokenProvider* token, QWidget *parent = 0 );
   
    inline const QString & token() const { return m_result; }

 private slots:
    void categoryChanged( QListBoxItem* item );
    void itemChanged( QTreeWidgetItem* item );

 private:
    void initAll();
    void initStackPage2();

    void setupPage1();
    void setupPage2();
    void setupPage3();

    void setupStackPage1();
    void setupStackPage2();

    void setupStack2Page1();
    void setupStack2Page2();
    void setupStack2Page3();
    void setupStack2Page4();
    void setupStack2Page5();

 private slots:
     void enableControls();
     void testQuery();

 protected:
    void accept();
    void showPage( QWidget* w );

 private:
    QStringList m_custom_tokens;

    QRadioButton* radioAll;
    QRadioButton* radioLabel;
    QRadioButton* radioSQL;
    QRadioButton* radioDate;
    QRadioButton* radioFixed;
    QRadioButton* radioCustom;
    QRadioButton* radioAddress;

    QRadioButton* radioSQLQuery;
    QRadioButton* radioVariable;
    QRadioButton* radioJavaScript;

    QRadioButton* radioVariableNew;
    QRadioButton* radioVariableExisting;
    
    KLineEdit* editVariable;
    KListBox* listVariable;

    KLineEdit* editQuery;
    QTextBrowser* textQueryResults;
    KPushButton* buttonQuery;

    DSTextEdit* editJavaScript;

    QString m_result;

    QStackedWidget* page2;
    QStackedWidget* page3;

    QWidget* stackPage1;
    QWidget* stackPage2;

    QWidget* stack2Page1;
    QWidget* stack2Page2;
    QWidget* stack2Page3;
    QWidget* stack2Page4;
    QWidget* stack2Page5;

    QList<tToken> m_tokens;
    TokenProvider* m_token;

    KListBox* category;
    QTreeWidget* allList;
    KLineEdit* lineEdit;

    QTreeWidget* labelList;
};

#endif
