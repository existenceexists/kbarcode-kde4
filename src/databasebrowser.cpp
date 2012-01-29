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
/*#include <QSqlQuery>*/// -!F: keep
#include <Q3SqlCursor>
//Added by qt3to4:
//#include <QSqlCursor>// -!F: original, delete
#include <QWidget>

// KDE includes
#include <kaction.h>
#include <kapplication.h>
//#include <keditcl.h>// -!F: original, delete
#include <kfinddialog.h>// -!F: keep, replacement of keditcl.h
#include <kfind.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmenu.h>
#include <kstatusbar.h>
#include <KActionCollection>
#include <KStandardAction>
#include <kstandarddirs.h>
#include <kxmlguiwindow.h>
#include <ktoolbar.h>

#define CUR_TABLE_ID 6666

DatabaseBrowser::DatabaseBrowser( QString _database, QWidget *parent)
    : KXmlGuiWindow ( parent ) 
{
    m_direction = m_case = false;

    table = new MyDataTable(this );
    //setCentralWidget( (QWidget*) table );
    setCentralWidget( table );

    statusBar()->insertPermanentItem( i18n("Current Table: <b>" ) + _database, CUR_TABLE_ID, 0 );
    statusBar()->setSizeGripEnabled( true );
    statusBar()->show();

    database = _database;

    connect( (QObject*) table, SIGNAL( cursorChanged( QSql::Op ) ),
             SqlTables::getInstance(), SIGNAL( tablesChanged() ) );

    /*connect( this, SIGNAL( connectedSQL() ), this, SLOT( setupSql() ) );*/// -!F: original, keep, this line gives the warning: Object::connect: No such signal DatabaseBrowser::connectedSQL()
    connect( SqlTables::getInstance(), SIGNAL( connectedSQL() ), this, SLOT( setupSql() ) );// -!F: is this the right correction of the previous line ?

    /*findDlg = 0;*/// -!F: original, delete
    findDialogExists = false;
    
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
    KXmlGuiWindow::setAutoSaveSettings( QString("DatabaseBrowser") );

    if( findDlg )
        delete findDlg;// -!F: original, delete, now need to delete a pointer to findDlg
}

void DatabaseBrowser::setupActions()
{
    KXmlGuiWindow::setupGUI( Default, KStandardDirs::locate( "appdata", QString("databasebrowserui.rc") ) );
    
    KMenu* editMenu = new KMenu( this );
    editMenu->setTitle( i18n("&Edit") );

    KAction* acut = KStandardAction::cut( this, SLOT( cut() ), actionCollection() );
    KAction* acopy = KStandardAction::copy( this, SLOT( copy() ), actionCollection() );
    KAction* apaste = KStandardAction::paste( this, SLOT( paste() ), actionCollection() );
    KAction* afind = KStandardAction::find( this, SLOT( find() ), actionCollection() );
    /*menuBar()->insertItem( i18n("&Edit"), editMenu, -1, 1 );*/// -!F: original, delete

    /*acut->plug( editMenu );
    acopy->plug( editMenu );
    apaste->plug( editMenu );*/// -!F: original, delete
    editMenu->addAction( acut );
    editMenu->addAction( acopy );
    editMenu->addAction( apaste );
    
    /*editMenu->insertSeparator(  );
    afind->plug( editMenu );
    KStandardAction::findNext( this, SLOT( slotFindNext() ), actionCollection() )->plug( editMenu );
    editMenu->insertSeparator();
    KAction* aimport = new KAction( i18n("&Import CSV File..."), "",
                                0, this, SLOT(import()), actionCollection(), "import" );
    aimport->plug( editMenu );*/// -!F: modified original, delete
    editMenu->addSeparator();
    editMenu->addAction( afind );
    KAction* actionFindNext = KStandardAction::findNext( this, SLOT( slotFindNext() ), actionCollection() );
    editMenu->addAction( actionFindNext );
    editMenu->addSeparator();
    KAction* aimport = new KAction( this );
    aimport->setText( i18n("&Import CSV File...") );
    actionCollection()->addAction( "import", aimport );
    connect(aimport, SIGNAL(triggered(bool)), this, SLOT(import()));
    editMenu->addAction( aimport );
    
    menuBar()->insertMenu( menuBar()->actions()[1], editMenu );
    
    /*menuBar()->actions()[ -1 ]->actions()[ 8 ]->setIcon( KIcon ( KStandardDirs::locate(
            "appdata", QString( "hi16-app-kbarcode.png" ) ) ) );*/// -!F: delete
    /*helpMenu()->actions()[ 4 ]->setIcon( KIcon ( KStandardDirs::locate(
            "appdata", QString( "hi16-app-kbarcode.png" ) ) ) );*/// -!F: delete
    actionCollection()->action( "help_about_app" )->setIcon( KIcon ( KStandardDirs::locate(
        "appdata", QString( "hi16-app-kbarcode.png" ) ) ) );;
    setWindowIcon( KIcon( KStandardDirs::locate( "appdata", QString("hi16-app-kbarcode.png") ) ) );
        
    /*acut->plug( toolBar() );
    acopy->plug( toolBar() );
    apaste->plug( toolBar() );

    toolBar()->insertSeparator();
    afind->plug( toolBar() );*/// -!F: original, delete
    toolBar()->addAction( acut );
    toolBar()->addAction( acopy );
    toolBar()->addAction( apaste );
    toolBar()->addSeparator();
    toolBar()->addAction( afind );

    /*MainWindow::loadConfig();*/// -!F: original, how to load databasebrowser window settings ?
}

void DatabaseBrowser::setupSql()
{
    Q3SqlCursor* cur = new Q3SqlCursor( database, true );// -!F: original, delete
    cur->select();
    /*QSqlQuery query( QString( "SELECT * FROM " ) + QString( database ) );*/// -!F: keep
    //pQuery = & query;// -!F: delete
    int i = 0;
    int c = 0;
    while ( cur->next() ) {
        for( c = 0; c < cur->count(); c++ ) {
            table->setText( i, c, cur->value( c ).toString() );
            table->horizontalHeader()->setLabel( c, cur->fieldName( c ) );
        }
        i++;
    }
    /*while ( query.next() ) {// -!F: keep
        for( c = 0; c < query.record().count(); c++ ) {
            table->setText( i, c, query.value( c ).toString() );
            table->horizontalHeader()->setLabel( c, query.record().fieldName( c ) );
        }
        i++;
    }*/

    table->setNumCols( c );
    table->setNumRows( i );

    table->setSqlCursor( cur, true, true );
    table->setSorting( true );
    table->setConfirmDelete( true );
    table->setAutoEdit( true );
    table->refresh( Q3DataTable::RefreshAll );
}

void DatabaseBrowser::find()
{
    /*if( !findDlg )
        findDlg = new KEdFind( this, false );
        
    findDlg->setText( m_find );
    findDlg->setDirection( m_direction );
    findDlg->setCaseSensitive( m_case );
    connect( findDlg, SIGNAL( search() ), this, SLOT( findNext() ) );
    
    findDlg->exec();*/// -!F: original, delete
    if( !findDialogExists ) {
        findDlg = new KFindDialog( this );
        findDialogExists = true;
    }
        
    findDlg->setPattern( m_find );
    /*KFind::Options findOptions = findDlg->options();*/
    long findOptions = findDlg->options();
    if ( m_direction && !( ( findOptions & KFind::FindBackwards ) == KFind::FindBackwards ) ) {
        findOptions = findOptions | KFind::FindBackwards;
    }
    if ( m_case && !( ( findOptions & KFind::CaseSensitive ) == KFind::CaseSensitive ) ) {
        findOptions = findOptions | KFind::CaseSensitive;
    }
    findDlg->setOptions( findOptions );
    
    findObject = new KFind( m_find, findDlg->options(), table, findDlg );
    connect( findObject, SIGNAL( findNext() ), this, SLOT( slotFindNext() ) );
    
    findDlg->exec();
    findDialogExists = false;
}

void DatabaseBrowser::slotFindNext()
{
    if( findDialogExists ) {
        m_find = findDlg->pattern();
        m_direction = ( findDlg->options() & KFind::FindBackwards ) == KFind::FindBackwards;
        m_case = ( findDlg->options() & KFind::CaseSensitive ) == KFind::CaseSensitive;
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
