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

#include "tokendialog.h"
#include "tokenprovider.h"
#include "sqltables.h"
#include "dstextedit.h"

#include <kassistantdialog.h>
#include <klineedit.h>
#include <klistwidget.h>
#include <k3listview.h>
#include <klocale.h>

#include <q3hbox.h> 
#include <qlabel.h>
#include <qlayout.h>
#include <qsplitter.h>
#include <qtooltip.h>
#include <QList>
#include <q3vbox.h>
//#include <qvbuttongroup.h>
#include <QGroupBox>
#include <QStackedWidget>
#include <qradiobutton.h>
#include <QWidget>
#include <QObject>
#include <Q3ListBoxItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QStringList>
//Added by qt3to4:
#include <QVBoxLayout>
#include <kcombobox.h>
#include <q3textbrowser.h>
#include <kpushbutton.h>

TokenDialog::TokenDialog(TokenProvider* token ,QWidget *parent)
    : KAssistantDialog( parent ), m_token( token )
{
    m_custom_tokens = m_token->listUserVars();

    setupPage1();
    setupPage2();
    setupPage3();

    setupStackPage1();
    setupStackPage2();
 
    setupStack2Page1();
    setupStack2Page2();
    setupStack2Page3();
    setupStack2Page4();
    setupStack2Page5();
    
    enableFinishButtonStack2Page1 = false;
    enableFinishButtonStack2Page2 = false;
    enableFinishButtonStack2Page3 = false;
    enableFinishButtonStack2Page4 = false;
    enableFinishButtonStack2Page5 = false;

    enableControls();
}

void TokenDialog::setupPage1()
{
    QGroupBox* page = new QGroupBox( i18n("What do you want to insert?") );
    QVBoxLayout* page_layout = new QVBoxLayout;

    radioFixed = new QRadioButton( i18n("Insert a &fixed data field") );
    page_layout->addWidget(radioFixed);
    radioCustom = new QRadioButton( i18n("Insert a &custom SQL query, variable or JavaScript function") );
    page_layout->addWidget(radioCustom);
    page->setLayout(page_layout);
    
    radioFixed->setChecked( true );

    KPageWidgetItem * page1Item = addPage( page, i18n("Step 1 of 3") );
    page1Item->setObjectName("page1");
}

void TokenDialog::setupPage2()
{
    page2 = new QStackedWidget( this );

    KPageWidgetItem * page2Item = addPage( page2, i18n("Step 2 of 3") );
    page2Item->setObjectName("page2");
}

void TokenDialog::setupPage3()
{
    page3 = new QStackedWidget( this );

    KPageWidgetItem * page3Item = addPage( page3, i18n("Step 3 of 3") );
    page3Item->setObjectName("page3");
}

void TokenDialog::setupStackPage1()
{
    stackPage1 = new QWidget;
    QVBoxLayout* stackPage1_layout = new QVBoxLayout;
    stackPage1->setLayout(stackPage1_layout);

    QGroupBox* group = new QGroupBox( i18n("What do you want to insert?"));
    stackPage1_layout->addWidget(group);
    QVBoxLayout* group_layout = new QVBoxLayout;
    radioAll = new QRadioButton( i18n("&Select from a list of all tokens") );
    group_layout->addWidget(radioAll);
    radioLabel = new QRadioButton( i18n("Insert printing &informations") );
    group_layout->addWidget(radioLabel);
    radioSQL = new QRadioButton( i18n("&Insert a database field") );
    group_layout->addWidget(radioSQL);
    radioDate = new QRadioButton( i18n("Insert a &date/time function") );
    group_layout->addWidget(radioDate);
    radioAddress = new QRadioButton( i18n("Insert an &addressbook field") );
    radioAll->setChecked( true );
    group_layout->addWidget(radioAddress);
    group->setLayout(group_layout);

    page2->addWidget( stackPage1 );
}

void TokenDialog::setupStackPage2()
{
    stackPage2 = new QWidget();
    QVBoxLayout* stackPage2_layout = new QVBoxLayout;
    stackPage2->setLayout(stackPage2_layout);

    QGroupBox* group = new QGroupBox( i18n("What do you want to insert?") );
    stackPage2_layout->addWidget(group);
    QVBoxLayout* group_layout = new QVBoxLayout;
    radioVariable = new QRadioButton( i18n("Insert a custom &variable") );
    group_layout->addWidget(radioVariable);
    radioSQLQuery = new QRadioButton( i18n("Insert a &SQL query") );
    group_layout->addWidget(radioSQLQuery);
    radioJavaScript = new QRadioButton( i18n("Insert a &JavaScript function") );
    radioVariable->setChecked( true );
    group_layout->addWidget(radioJavaScript);
    group->setLayout(group_layout);
    connect( radioVariable, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( radioSQLQuery, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( radioJavaScript, SIGNAL( clicked() ), this, SLOT( enableControls() ) );

    page2->addWidget( stackPage2 );
}

void TokenDialog::setupStack2Page1()
{
    stack2Page1 = new QWidget;
    stack2Page1->setObjectName( "stack2Page1" );

    QVBoxLayout* layout = new QVBoxLayout( stack2Page1 );
    QSplitter* splitter = new QSplitter( stack2Page1 );
    layout->addWidget( splitter );
    
    QWidget* left = new QWidget( splitter );
    QVBoxLayout* left_layout = new QVBoxLayout;
    left->setLayout(left_layout);
    QWidget* right = new QWidget( splitter );
    QVBoxLayout* right_layout = new QVBoxLayout;
    right->setLayout(right_layout);
    
    QLabel* label = new QLabel( i18n("&Category:") );
    left_layout->addWidget(label);
    category = new KListWidget;
    category->setMinimumWidth( 140 );
    left_layout->addWidget(category);
    label->setBuddy( category );

    label = new QLabel( i18n("&Token:") );
    right_layout->addWidget(label);
    allList = new QTreeWidget;
    right_layout->addWidget(allList);
    allList->setHeaderLabels( QStringList() << i18n("Token") << i18n("Description") );
    /*allList->setColumnWidthMode( 0, Q3ListView::Maximum );
    allList->setColumnWidthMode( 1, Q3ListView::Maximum );*/// -!F: original, delete
    allList->setColumnWidth( 0, 270 );
    allList->setColumnWidth( 1, 350 );
    allList->setMinimumWidth( 600 );
    allList->setMinimumHeight( 200 );
    label->setBuddy( allList );
    label = new QLabel( i18n("&Custom Expression to be inserted in the token.") );
    right_layout->addWidget(label);
    lineEdit = new KLineEdit;
    right_layout->addWidget(lineEdit);
    lineEdit->setEnabled( false );
    label->setBuddy( lineEdit );

    lineEdit->setToolTip( i18n("<qt>Certain tokens, like for exaple the sqlquery token need arguments. "
				  "In the case of the sqlquery token, the sure has to enter a sql query in "
				  "this text field.</qt>" ) );

    QList<int> sizes;
    int w = (width() / 4);
    sizes << w << w * 3;
    
    left_layout->setStretchFactor( category, 2 );
    right_layout->setStretchFactor( allList, 2 );
    splitter->setSizes( sizes );

    connect( category, SIGNAL( executed( QListWidgetItem* ) ), this, SLOT( categoryChanged( QListWidgetItem* ) ) );
    connect( allList, SIGNAL( itemClicked( QTreeWidgetItem*, int ) ), this, SLOT( itemChanged( QTreeWidgetItem* ) ) );

    initAll();

    page3->addWidget( stack2Page1 );
}

void TokenDialog::setupStack2Page2()
{
    stack2Page2 = new QWidget;
    stack2Page2->setObjectName( "stack2Page2" );
	QVBoxLayout* stack2Page2_layout = new QVBoxLayout;
	stack2Page2->setLayout(stack2Page2_layout);

    labelList = new QTreeWidget;
	stack2Page2_layout->addWidget(labelList);
    /*labelList->addColumn( i18n("Token"), 0 );
    labelList->addColumn( i18n("Description"), 1 );
    labelList->setColumnWidthMode( 0, Q3ListView::Maximum );
    labelList->setColumnWidthMode( 1, Q3ListView::Maximum );*/// -!F: original, delete
    labelList->setHeaderLabels( QStringList() << i18n("Token") << i18n("Description") );
    labelList->setColumnWidth( 0, 270 );

    connect( labelList, SIGNAL( itemSelectionChanged() ), this, SLOT( enableControls() ) );
    connect( labelList, SIGNAL( itemDoubleClicked( QTreeWidgetItem *, int ) ), this, SLOT( accept() ) );

    page3->addWidget( stack2Page2 );
}

void TokenDialog::setupStack2Page3() 
{
    stack2Page3 = new QGroupBox();
    stack2Page3->setObjectName( "stack2Page3" );
    QVBoxLayout* group_layout = new QVBoxLayout;

    radioVariableNew = new QRadioButton( i18n("&Create a new custom variable") );
    group_layout->addWidget(radioVariableNew);
    editVariable = new KLineEdit;
    group_layout->addWidget(editVariable);

    radioVariableExisting = new QRadioButton( i18n("&Insert an existing custom variable") );
    group_layout->addWidget(radioVariableExisting);
    listVariable = new KListWidget;
    radioVariableNew->setChecked( true );
    group_layout->addWidget(listVariable);
    stack2Page3->setLayout(group_layout);
    if( m_token )
        listVariable->addItems( m_token->listUserVars() );

    if( !listVariable->count() )
        radioVariableExisting->setEnabled( false );

    connect( radioVariableNew, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( radioVariableExisting, SIGNAL( clicked() ), this, SLOT( enableControls() ) );
    connect( editVariable, SIGNAL( textChanged( const QString & ) ), this, SLOT( enableControls() ) );
    connect( listVariable, SIGNAL( currentItemChanged( QListWidgetItem*, QListWidgetItem* ) ), this, SLOT( enableControls() ) );
    connect( listVariable, SIGNAL( doubleClicked( QListWidgetItem*, const QPoint & ) ), this, SLOT( accept() ) );

    page3->addWidget( stack2Page3 );
}

void TokenDialog::setupStack2Page4() 
{
    stack2Page4 = new QWidget();
    stack2Page4->setObjectName( "stack2Page4" );
    QVBoxLayout* stack2Page4_layout = new QVBoxLayout;
    stack2Page4->setLayout(stack2Page4_layout);

    if( !SqlTables::isConnected() )
        stack2Page4_layout->addWidget(new QLabel( i18n("<qt><b>No SQL connection found!</b><br>You can build a query, "
						       "but you will not be able to execute or test it right now.<br></qt>")));
    
    QWidget* hbox = new QWidget;
    QHBoxLayout* hbox_layout = new QHBoxLayout;
    hbox->setLayout(hbox_layout);
    stack2Page4_layout->addWidget(hbox);

    QLabel* label = new QLabel( i18n("&SQL Query:"));
    hbox_layout->addWidget(label);
    editQuery = new KLineEdit;
    hbox_layout->addWidget(editQuery);
    buttonQuery = new KPushButton( i18n("&Test") );
    hbox_layout->addWidget(buttonQuery);
    label->setBuddy( editQuery );

    hbox_layout->setStretchFactor( editQuery, 2 );

    stack2Page4_layout->addWidget(new QLabel( i18n("Query test results:")));
    textQueryResults = new Q3TextBrowser;
    stack2Page4_layout->addWidget(textQueryResults);
    textQueryResults->setReadOnly( true );

    connect( buttonQuery, SIGNAL( clicked() ), this, SLOT( testQuery() ) );
    connect( editQuery, SIGNAL( textChanged( const QString & ) ), this, SLOT( enableControls() ) );
        
    page3->addWidget( stack2Page4 );
}

void TokenDialog::setupStack2Page5() 
{
    stack2Page5 = new QWidget();
    stack2Page5->setObjectName( "stack2Page5" );
    QVBoxLayout* stack2Page5_layout = new QVBoxLayout;
    stack2Page5->setLayout(stack2Page5_layout);

    editJavaScript = new DSTextEdit;
    stack2Page5_layout->addWidget(editJavaScript);
    editJavaScript->setText( i18n("/* Place your JavaScript code into this text field. */\n") );

    connect( editJavaScript, SIGNAL( textChanged() ), SLOT( enableControls() ) );

    page3->addWidget( stack2Page5 );
}

void TokenDialog::accept()
{
    if( radioCustom->isChecked() )
    {
        if( radioVariable->isChecked() ) 
        {
            if( radioVariableNew->isChecked() ) 
            {
                m_result = editVariable->text();
                if( !m_result.startsWith( "$" ) )
                    m_result.prepend( '$' );
            }
            else if( radioVariableExisting->isChecked() )
                m_result = listVariable->currentItem()->text();
        }
        else if( radioSQLQuery->isChecked() )
            m_result = QString( "sqlquery:%2").arg( editQuery->text() );
        else if( radioJavaScript->isChecked() )
            m_result = QString( "js:%2").arg( editJavaScript->text() );

        m_result = "[" + m_result + "]";
    }
    else
    {
        /*QTreeWidgetItem* item = ( radioAll->isChecked() ? allList->selectedItem() : labelList->selectedItem() );*/// -!F: original, delete
        QTreeWidgetItem* item = NULL;
        if( radioAll->isChecked() ) {
            if( !allList->selectedItems().isEmpty() ) {
                item = allList->selectedItems().first();
            }
        } else {
            if( !labelList->selectedItems().isEmpty() ) {
                item = labelList->selectedItems().first();
            }
        }

        if( item )
        {
            for( int i = 0; i < m_tokens.count(); i++ ) {
                if( QString( "[%1]").arg( m_tokens[i].token ) == item->text( 0 ) )
                {
                    if( m_tokens[i].appendix )
                        m_result =  QString( "[%1%2]").arg( m_tokens[i].token ).arg( lineEdit->text() );
                    else
                        m_result = item->text( 0 );
                    break;
                }
            }
        }
    }

    KAssistantDialog::accept();
}

void TokenDialog::next()
{
    KAssistantDialog::next();
    
    configureCurrentPage(currentPage());
}

void TokenDialog::back()
{
    KAssistantDialog::back();
    
    configureCurrentPage(currentPage());
}

void TokenDialog::configureCurrentPage( KPageWidgetItem* w )
{
    if( w->objectName() == QString("page3") )
    {
        if( radioCustom->isChecked() ) 
        {
            if( radioVariable->isChecked() ) {
                page3->setCurrentWidget( stack2Page3 );
                enableButton( KDialog::User1, enableFinishButtonStack2Page3 );
            }
            else if( radioSQLQuery->isChecked() ) {
                page3->setCurrentWidget( stack2Page4 );
                enableButton( KDialog::User1, enableFinishButtonStack2Page4 );
            }
            else if( radioJavaScript->isChecked() ) 
            {
                page3->setCurrentWidget( stack2Page5 );
                enableButton( KDialog::User1, enableFinishButtonStack2Page5 );
                editJavaScript->setFocus();
            }
        }
        else
        {
            if( radioAll->isChecked() ) {
                page3->setCurrentWidget( stack2Page1 );
                enableButton( KDialog::User1, enableFinishButtonStack2Page1 );
            }
            else 
            {
                initStackPage2();
                page3->setCurrentWidget( stack2Page2 );
                /*enableButton( KDialog::User1, enableFinishButtonStack2Page2 );*/// -!F: added, delete
                enableButton( KDialog::User1, false );
            }
        }
    }
    else if( w->objectName() == QString("page2") )
    {
        if( radioFixed->isChecked() ) {
            page2->setCurrentWidget( stackPage1 );
        }
        else if( radioCustom->isChecked() ) {
            page2->setCurrentWidget( stackPage2 );
        }
    }
}

void TokenDialog::initAll()
{
    int i, z;
    QList<tCategories>* categories = TokenProvider::getTokens();

    category->addItem( i18n("All") );
    
    for( i = 0; i < categories->count(); i++ )
        category->addItem( TokenProvider::captionForCategory( (TokenProvider::ECategories)(*categories)[i].category ) );

    for( i = 0; i < categories->count(); i++ )
	for( z = 0; z < (*categories)[i].tokens.count(); z++ )
	    m_tokens.append( (*categories)[i].tokens[z] );

    if( m_token )
    {
        QStringList custom_tokens = m_token->listUserVars();
        for( i = 0; i < custom_tokens.count(); i++ )
            m_tokens.append( tToken(  custom_tokens[i], i18n("Variable defined by the user for this label.") ) );
    }

    category->setCurrentItem( category->item( 0 ) );
    categoryChanged( category->item( 0 ) );
}

void TokenDialog::initStackPage2()
{
    TokenProvider::ECategories cat;
    labelList->clear();

    if( radioLabel->isChecked() )
        cat = TokenProvider::CAT_LABEL;
    else if( radioSQL->isChecked() )
        cat = TokenProvider::CAT_DATABASE;
    else if( radioDate->isChecked() )
        cat = TokenProvider::CAT_DATE;
    else if( radioAddress->isChecked() )
        cat = TokenProvider::CAT_ADDRESS;
    else
        return;

    QList<tCategories>* categories = TokenProvider::getTokens();
    for( int i = 0; i < (int)categories->count(); i++ )
    {
        if( (*categories)[i].category == cat )
        {
            for( int z = 0; z < (*categories)[i].tokens.count(); z++ )
                labelList->addTopLevelItem( new QTreeWidgetItem( labelList, QStringList() << QString( "[%1]").arg( (*categories)[i].tokens[z].token ) << 
                                                     (*categories)[i].tokens[z].description ) );
            
            break;
        }
    }
}

void TokenDialog::categoryChanged( QListWidgetItem* item )
{
    int i;
    QList<tCategories>* categories = TokenProvider::getTokens();
    allList->clear();
    lineEdit->setEnabled( false );

    if( category->row( item ) == 0 )
    {
        for( i = 0; i < m_tokens.count(); i++ )
	    allList->addTopLevelItem( new QTreeWidgetItem( allList, QStringList() << QString( "[%1]").arg( m_tokens[i].token ) <<
						 m_tokens[i].description ) );
    } 
    else
    {
        for( i = 0; i < categories->count(); i++ )
        {
            if( TokenProvider::captionForCategory( (TokenProvider::ECategories)(*categories)[i].category ) == item->text() )
            {
                for( int z = 0; z < (*categories)[i].tokens.count(); z++ )
                    allList->addTopLevelItem( new QTreeWidgetItem( allList, QStringList() << QString( "[%1]").arg( (*categories)[i].tokens[z].token ) <<
                                      (*categories)[i].tokens[z].description ) );
                
                break;
            }
        }

	// TODO: comparing by a user visible string cries for bugs!!!
	if( item->text() == i18n("Custom Values") )
	    for( i=0;i<m_custom_tokens.count();i++ )
		allList->addTopLevelItem( new QTreeWidgetItem( allList, QStringList() << QString( "[%1]").arg( m_custom_tokens[i] ) << 
						     i18n("Variable defined by the user for this label.") ) );
    }
}

void TokenDialog::itemChanged( QTreeWidgetItem* item )
{
    for( int i = 0; i < m_tokens.count(); i++ )
    {
	if( QString( "[%1]").arg( m_tokens[i].token ) == item->text( 0 ) )
	{
	    lineEdit->setEnabled( m_tokens[i].appendix );
	    if(  m_tokens[i].appendix )
		lineEdit->setFocus();
	    break;
	}
    }

    enableControls();
}

void TokenDialog::enableControls()
{
    enableFinishButtonStack2Page1 = false;
    enableFinishButtonStack2Page2 = false;
    enableFinishButtonStack2Page3 = false;
    enableFinishButtonStack2Page4 = false;
    enableFinishButtonStack2Page5 = false;

    listVariable->setEnabled( radioVariableExisting->isChecked() );
    editVariable->setEnabled( radioVariableNew->isChecked() );

    if( ( editVariable->isEnabled() && !editVariable->text().isEmpty() ) ||
        ( listVariable->isEnabled() && ( listVariable->currentItem() != 0 ) ) ) {
        enableFinishButtonStack2Page3 = true;
    }

    buttonQuery->setEnabled( radioSQLQuery->isChecked() && !editQuery->text().isEmpty() && SqlTables::isConnected() );
    if( radioSQLQuery->isChecked() && !editQuery->text().isEmpty() ) {
        enableFinishButtonStack2Page4 = true;
    }

    if( radioJavaScript->isChecked() && !editJavaScript->text().isEmpty() ) {
        enableFinishButtonStack2Page5 = true;
    }

    if( !radioCustom->isChecked() )
    {
        if( !radioAll->isChecked() && !labelList->selectedItems().isEmpty() ) {
            enableFinishButtonStack2Page2 = true;
        }
        
        if( radioAll->isChecked() && !allList->selectedItems().isEmpty() ) {
            enableFinishButtonStack2Page1 = true;
        }
    }
    
    QString widgetName = page3->currentWidget()->objectName();
    if( widgetName == QString( "stack2Page1" ) ) {
        enableButton( KDialog::User1, enableFinishButtonStack2Page1 );
    } else if( widgetName == QString( "stack2Page2" ) ) {
        enableButton( KDialog::User1, enableFinishButtonStack2Page2 );
    } else if( widgetName == QString( "stack2Page3" ) ) {
        enableButton( KDialog::User1, enableFinishButtonStack2Page3 );
    } else if( widgetName == QString( "stack2Page4" ) ) {
        enableButton( KDialog::User1, enableFinishButtonStack2Page4 );
    } else if( widgetName == QString( "stack2Page5" ) ) {
        enableButton( KDialog::User1, enableFinishButtonStack2Page5 );
    }
}

void TokenDialog::testQuery()
{
    QString ret = "[sqlquery:" + editQuery->text() + "]";
    if( m_token )
        ret = m_token->parse( ret );
    textQueryResults->setText( ret );
}

#include "tokendialog.moc"

