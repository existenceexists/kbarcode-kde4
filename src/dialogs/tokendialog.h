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

#include "documentitem.h"
#include "tokenprovider.h"
#include <kassistantdialog.h>
//Added by qt3to4:
#include <QList>

class KListWidget;
class Q3ListView;
class QListWidgetItem;
class Q3ListViewItem;
class KLineEdit;
class TokenProvider;
class KAssistantDialog;
class QTreeWidget;
class QTreeWidgetItem;
class KPageWidgetItem;

class QRadioButton;
class QStackedWidget;
class QWidget;
class KComboBox;
class Q3TextBrowser;
class KPushButton;
class DSTextEdit;

class TokenDialog : public KAssistantDialog {
    
 Q_OBJECT

 public:
    TokenDialog( TokenProvider* token, QWidget *parent = 0 );
   
    inline const QString & token() const { return m_result; }

 private slots:
    void categoryChanged( QListWidgetItem* item );
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
     void next();
     void back();

 protected:
    void accept();
    void configureCurrentPage( KPageWidgetItem* w );

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
    KListWidget* listVariable;

    KLineEdit* editQuery;
    Q3TextBrowser* textQueryResults;
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
    
    bool enableFinishButtonStack2Page1;
    bool enableFinishButtonStack2Page2;
    bool enableFinishButtonStack2Page3;
    bool enableFinishButtonStack2Page4;
    bool enableFinishButtonStack2Page5;

    QList<tToken> m_tokens;
    TokenProvider* m_token;

    KListWidget* category;
    QTreeWidget* allList;
    KLineEdit* lineEdit;

    QTreeWidget* labelList;
};

#endif
