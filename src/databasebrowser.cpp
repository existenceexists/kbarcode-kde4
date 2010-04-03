/***************************************************************************
                          databasebrowser.cpp  -  description
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

#include "databasebrowser.h"
#include "mydatatable.h"
#include "definition.h"
#include "sqltables.h"
#include "csvimportdlg.h"

// Qt includes
#include <qclipboard.h>
//Added by qt3to4:
#include <QSqlCursor>

// KDE includes
#include <kaction.h>
#include <kapplication.h>
#include <keditcl.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmenu.h>
#include <kstatusbar.h>

#define CUR_TABLE_ID 6666

DatabaseBrowser::DatabaseBrowser( QString _database, QWidget *parent)
    : MainWindow(parent)
{
    m_direction = m_case = false;

    table = new MyDataTable(this );
    setCentralWidget( table );

    statusBar()->insertItem( i18n("Current Table: <b>" ) + _database, CUR_TABLE_ID, 0, true );
    statusBar()->setSizeGripEnabled( true );
    statusBar()->show();

    database = _database;

    connect( table, SIGNAL( cursorChanged( QSql::Op ) ),
             SqlTables::getInstance(), SIGNAL( tablesChanged() ) );

    connect( this, SIGNAL( connectedSQL() ), this, SLOT( setupSql() ) );

    findDlg = 0;
    
    setupActions();
    show();

    setupSql();
}

DatabaseBrowser::~DatabaseBrowser()
{
    // update sql label definitions
    // because they may have changed
    // TODO:
    // add selction here to only update
    // if neccessary!
    Definition::updateProducer();
    MainWindow::saveConfig();    

    if( findDlg )
        delete findDlg;
}

void DatabaseBrowser::setupActions()
{
    MainWindow::setupActions();
    KMenu* editMenu = new KMenu( this );

    KAction* acut = KStandardAction::cut( this, SLOT( cut() ), actionCollection() );
    KAction* acopy = KStandardAction::copy( this, SLOT( copy() ), actionCollection() );
    KAction* apaste = KStandardAction::paste( this, SLOT( paste() ), actionCollection() );
    KAction* afind = KStandardAction::find( this, SLOT( find() ), actionCollection() );
    menuBar()->insertItem( i18n("&Edit"), editMenu, -1, 1 );

    acut->plug( editMenu );
    acopy->plug( editMenu );
    apaste->plug( editMenu );
    
    editMenu->insertSeparator();
    afind->plug( editMenu );
    KStandardAction::findNext( this, SLOT( findNext() ), actionCollection() )->plug( editMenu );
    editMenu->insertSeparator();
    KAction* aimport = new KAction( i18n("&Import CSV File..."), "",
                                0, this, SLOT(import()), actionCollection(), "import" );
    aimport->plug( editMenu );
        
    acut->plug( toolBar() );
    acopy->plug( toolBar() );
    apaste->plug( toolBar() );

    toolBar()->insertSeparator();
    afind->plug( toolBar() );

    MainWindow::loadConfig();
}

void DatabaseBrowser::setupSql()
{
    QSqlCursor* cur = new QSqlCursor( database, true );
    cur->select();
    unsigned int i = 0;
    unsigned int c = 0;
    while ( cur->next() ) {
        for( c = 0; c < cur->count(); c++ ) {
            table->setText( i, c, cur->value( c ).toString() );
            table->horizontalHeader()->setLabel( c, cur->fieldName( c ) );
        }
        i++;
    }

    table->setNumCols( c );
    table->setNumRows( i );

    table->setSqlCursor( cur, true, true );
    table->setSorting( true );
    table->setConfirmDelete( true );
    table->setAutoEdit( true );
    table->refresh( QDataTable::RefreshAll );
}

void DatabaseBrowser::find()
{
    if( !findDlg )
        findDlg = new KEdFind( this, false );
        
    findDlg->setText( m_find );
    findDlg->setDirection( m_direction );
    findDlg->setCaseSensitive( m_case );
    connect( findDlg, SIGNAL( search() ), this, SLOT( findNext() ) );
    
    findDlg->exec();
    
}

void DatabaseBrowser::findNext()
{
    if( findDlg ) {
        m_find = findDlg->getText();
        m_direction = findDlg->get_direction();
        m_case = findDlg->case_sensitive();
    } else
        find();

    table->find( m_find, m_case, m_direction );
}

void DatabaseBrowser::cut()
{
    QString text = table->value( table->currentRow(), table->currentColumn() ).toString();
    if( !text.isEmpty() ) {
        kapp->clipboard()->setText( text );

        QSqlRecord* buffer = table->sqlCursor()->primeUpdate();
        if( buffer ) {
            buffer->setValue( table->horizontalHeader()->label( table->currentColumn() ), "" );
            table->sqlCursor()->update();
            table->refresh();
        }

    }
}

void DatabaseBrowser::copy()
{
    QString text = table->value( table->currentRow(), table->currentColumn() ).toString();
    if( !text.isEmpty() )
        kapp->clipboard()->setText( text );
}

void DatabaseBrowser::paste()
{
    QString text = kapp->clipboard()->text();
    if( !text.isEmpty() ) {
        QSqlRecord* buffer = table->sqlCursor()->primeUpdate();
        if( buffer ) {
            buffer->setValue( table->horizontalHeader()->label( table->currentColumn() ), text );
            table->sqlCursor()->update();
            table->refresh();
        }
    }

}

void DatabaseBrowser::import()
{
    new CSVImportDlg( this );
}

#include "databasebrowser.moc"
