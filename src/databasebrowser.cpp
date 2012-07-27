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
//#include <Q3SqlCursor>// -!F: delete
#include <QTableView>
#include <QSqlTableModel>
#include <QDataWidgetMapper>
#include <QAbstractItemDelegate>
#include <QDebug>
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

DatabaseBrowser::DatabaseBrowser( QString _database, QWidget *parent )
    : KXmlGuiWindow ( parent ) 
{
    /*m_direction = m_case = false;*/// -!F: original, delete
    m_findOptions = 0;

    /*table = new MyDataTable(this );*/// -!F: original
    table = new QTableView( this );
    //setCentralWidget( (QWidget*) table );
    setCentralWidget( table );
    
    model = new QSqlTableModel( this, *(SqlTables::getInstance()->database()) );
    model->setTable( _database );
    model->setEditStrategy( QSqlTableModel::OnFieldChange );
    table->setModel( model );
    //model->setFilter("");// do not filter the SELECT statement
    
    /*mapper = new QDataWidgetMapper;
    mapper->setModel( model );*/

    statusBar()->insertPermanentItem( i18n("Current Table: <b>" ) + _database, CUR_TABLE_ID, 0 );
    statusBar()->setSizeGripEnabled( true );
    statusBar()->show();

    database = _database;

    /*connect( (QObject*) table, SIGNAL( cursorChanged( QSql::Op ) ),
             SqlTables::getInstance(), SIGNAL( tablesChanged() ) );*/// -!F: original, what is the correct replacement?
    connect( model, SIGNAL( dataChanged(const QModelIndex &, const QModelIndex & ) ),
             SqlTables::getInstance(), SIGNAL( tablesChanged() ) );

    /*connect( this, SIGNAL( connectedSQL() ), this, SLOT( setupSql() ) );*/// -!F: original, keep, this line gives the warning: Object::connect: No such signal DatabaseBrowser::connectedSQL()
    connect( SqlTables::getInstance(), SIGNAL( connectedSQL() ), this, SLOT( setupSql() ) );// -!F: is this the right correction of the previous line ?

    findDlg = 0;
    m_find = 0;
    
    setupActions();
    
    setupSql();
    
    show();

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
        delete findDlg;
}

void DatabaseBrowser::setupActions()
{
    KXmlGuiWindow::setupGUI( Default, KStandardDirs::locate( "appdata", QString("databasebrowserui.rc") ) );
    
    KMenu* editMenu = new KMenu( this );
    editMenu->setTitle( i18n("&Edit") );

    KAction* acut = KStandardAction::cut( this, SLOT( cut() ), actionCollection() );
    KAction* acopy = KStandardAction::copy( this, SLOT( copy() ), actionCollection() );
    KAction* apaste = KStandardAction::paste( this, SLOT( paste() ), actionCollection() );
    KAction* afind = KStandardAction::find( this, SLOT( find2() ), actionCollection() );
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
    toolBar()->addAction( actionFindNext );

    /*MainWindow::loadConfig();*/// -!F: original, how to load databasebrowser window settings ?
}

void DatabaseBrowser::setupSql()
{
    /*Q3SqlCursor* cur = new Q3SqlCursor( database, true );// -!F: original, delete
    cur->select();*/
    /*QSqlQuery query( QString( "SELECT * FROM " ) + QString( database ) );*/// -!F: keep
    //pQuery = & query;// -!F: delete
    /*int i = 0;
    int c = 0;
    while ( cur->next() ) {
        for( c = 0; c < cur->count(); c++ ) {
            table->setText( i, c, cur->value( c ).toString() );
            table->horizontalHeader()->setLabel( c, cur->fieldName( c ) );
        }
        i++;
    }*/// -!F: original, delete
    /*while ( query.next() ) {// -!F: delete
        for( c = 0; c < query.record().count(); c++ ) {
            table->setText( i, c, query.value( c ).toString() );
            table->horizontalHeader()->setLabel( c, query.record().fieldName( c ) );
        }
        i++;
    }*/// -!F: delete

    /*table->setNumCols( c );
    table->setNumRows( i );

    table->setSqlCursor( cur, true, true );
    table->setSorting( true );
    table->setConfirmDelete( true );
    table->setAutoEdit( true );
    table->refresh( Q3DataTable::RefreshAll );*/// -!F: original, delete
    
    model->select();
    table->resizeColumnsToContents();
    
    /*for( int c = 0; c < model->columnCount(); c++ ) {
        //model->setHeaderData( c, Qt::Horizontal, ?);
        //mapper->addMapping( table->itemDelegateForColumn( c ), c );
        //model->setItemDelegate( itemDelegateForColumn( c ) );
    }*/// -!F: delete
}

void DatabaseBrowser::find()
{
    if( m_find ) {
        /*m_findPattern = m_find->pattern();
        m_findOptions = m_find->options();*/
        delete m_find;
        m_find = 0L;
    }
    
    findDlg = new KFindDialog( this );
        
    findDlg->setPattern( m_findPattern );
    findDlg->setOptions( m_findOptions );
    /*long findOptions = findDlg->options();*/
    /*if ( m_direction && !( ( findOptions & KFind::FindBackwards ) == KFind::FindBackwards ) ) {
        findOptions = findOptions | KFind::FindBackwards;
    }
    if ( m_case && !( ( findOptions & KFind::CaseSensitive ) == KFind::CaseSensitive ) ) {
        findOptions = findOptions | KFind::CaseSensitive;
    }*/// -!F: delete
    /*if ( m_direction ) {
        findOptions = findOptions | KFind::FindBackwards;
    }
    if ( m_case ) {
        findOptions = findOptions | KFind::CaseSensitive;
    }
    findDlg->setOptions( findOptions );*/
    
    /*findObject = new KFind( m_find, findDlg->options(), table, findDlg );
    connect( findObject, SIGNAL( findNext() ), this, SLOT( slotFindNext() ) );*/
    connect( findDlg, SIGNAL( okClicked() ), this, SLOT( slotFindNext() ) );
    
    m_findCurrentRow = 0;
    
    QDialog::DialogCode res = (QDialog::DialogCode) findDlg->exec();
    delete findDlg;
    findDlg = 0;
    if( res == QDialog::Accepted ) {
        find();
    }
}

void DatabaseBrowser::find2()
{
    if( m_find ) {
        /*m_findPattern = m_find->pattern();
        m_findOptions = m_find->options();*/
        delete m_find;
        m_find = 0L;
    }
    
    findDlg = new KFindDialog( this );
        
    findDlg->setPattern( m_findPattern );
    findDlg->setOptions( m_findOptions );
    
    connect( findDlg, SIGNAL( okClicked() ), this, SLOT( slotFindNext() ) );
    
    m_findCurrentRow = 0;
    
    findDlg->exec();
    delete findDlg;
    findDlg = 0;
}

#if 0
void DatabaseBrowser::slotFindNext()
{
    if( m_find ) {
        KFind::Result res = KFind::NoMatch;
        
        /*while ( res == KFind::NoMatch && ( table->rowAt( m_findCurrentRow ) != -1 ) ) {*/// -!F: delete
        while ( ( res == KFind::NoMatch ) && ( m_findCurrentRow < model->rowCount() ) ) {
            for( int column = 0; column < model->columnCount(); column++ ) {
                if( m_find->needData() && model->hasIndex( m_findCurrentRow, column ) ) {
                    m_find->setData( model->data( model->index( m_findCurrentRow, column ), Qt::DisplayRole ).toString() );
                    //qDebug() << "if( m_find->needData() m_findCurrentRow == " << m_findCurrentRow;
                }
                

                // Let KFind inspect the text fragment, and display a dialog if a match is found
                res = m_find->find();
            }

            /*if ( res == KFind::NoMatch ) {*/
            if( ( m_find->options() & KFind::FindBackwards ) == KFind::FindBackwards ) {// find backwards
                m_findCurrentRow--;
            } else {// find forwards
                m_findCurrentRow++;
            }
        }

        if ( res == KFind::NoMatch ) {// i.e. at end
            /*<Call either  m_find->displayFinalDialog(); m_find->deleteLater(); m_find = 0L;
           or           if ( m_find->shouldRestart() ) { reinit (w/o FromCursor) and call slotFindNext(); }
                        else { m_find->closeFindNextDialog(); }>*/
            m_findCurrentRow = 0;
        }
    } else {
        
        if( findDlg ) {
            m_findOptions = findDlg->options();
            m_findPattern = findDlg->pattern();
        }
        // This creates a find-next-prompt dialog if needed.
        /*m_find = new KFind( m_findPattern, m_findOptions, this, findDlg );*/// -!F: Use this with non-modal dialog.
        m_find = new KFind( m_findPattern, m_findOptions, this );

        // Connect highlight signal to code which handles highlighting
        // of found text.
        connect( m_find, SIGNAL( highlight( const QString &, int, int ) ),
            this, SLOT( slotHighlight( const QString &, int, int ) ) );
        // Connect findNext signal - called when pressing the button in the dialog
        connect( m_find, SIGNAL( findNext() ),
            this, SLOT( slotFindNext() ) );
        
        slotFindNext();
    }
}
#endif

void DatabaseBrowser::slotFindNext()
{
    if( m_find ) {
        KFind::Result res = KFind::NoMatch;
        int column = 0;
        
        while ( ( res == KFind::NoMatch ) && ( m_findCurrentRow < model->rowCount() ) ) {
            if( m_find->needData() ) {
                m_find->setData( model->data( model->index( m_findCurrentRow, column ), Qt::DisplayRole ).toString() );
                //qDebug() << "if( m_find->needData() m_findCurrentRow == " << m_findCurrentRow;// -!F: delete
            }
            

            // Let KFind inspect the text fragment, and display a dialog if a match is found
            res = m_find->find();

            if ( res == KFind::NoMatch ) {//Move to the next text fragment, honoring the FindBackwards setting for the direction
                if( column < (model->columnCount() - 1) ) {
                    column++;
                } else {
                    column = 0;
                    if( ( m_find->options() & KFind::FindBackwards ) == KFind::FindBackwards ) {// find backwards
                        m_findCurrentRow--;
                        if( m_findCurrentRow < 0 ) {
                            m_findCurrentRow = model->rowCount() - 1;
                        }
                    } else {// find forwards
                        m_findCurrentRow++;
                    }
                }
            }
        }

        if ( res == KFind::NoMatch ) {// i.e. at end and there was no match
            /*<Call either  m_find->displayFinalDialog(); m_find->deleteLater(); m_find = 0L;
           or           if ( m_find->shouldRestart() ) { reinit (w/o FromCursor) and call slotFindNext(); }
                        else { m_find->closeFindNextDialog(); }>*/
            if( ( m_find->options() & KFind::FindBackwards ) == KFind::FindBackwards ) {
                m_findCurrentRow = model->columnCount() - 1;
            } else {
                m_findCurrentRow = 0;
            }
        } else {// There was a match so the matching row is selected and a dialog "Find next" is displayed.
            if( ( m_find->options() & KFind::FindBackwards ) == KFind::FindBackwards ) {
                m_findCurrentRow--;// continue the find in the next row.
            } else {
                m_findCurrentRow++;// continue the find in the next row.
            }
        }
    } else {
        
        if( findDlg ) {
            m_findOptions = findDlg->options();
            m_findPattern = findDlg->pattern();
        }
        // This creates a find-next-prompt dialog if needed.
        /*m_find = new KFind( m_findPattern, m_findOptions, this, findDlg );*/// -!F: Use this with non-modal dialog.
        m_find = new KFind( m_findPattern, m_findOptions, this );

        // Connect highlight signal to code which handles highlighting
        // of found text.
        connect( m_find, SIGNAL( highlight( const QString &, int, int ) ),
            this, SLOT( slotHighlight( const QString &, int, int ) ) );
        // Connect findNext signal - called when pressing the button in the dialog
        connect( m_find, SIGNAL( findNext() ),
            this, SLOT( slotFindNext() ) );
        
        slotFindNext();
    }
}

void DatabaseBrowser::slotHighlight( const QString & text, int matchingIndex, int matchedLength )
{
    table->selectRow( m_findCurrentRow );
    //table->scrollTo( model->index( m_findCurrentRow, m_findCurrentColumn ) );
}

void DatabaseBrowser::cut()
{
    /*QString text = table->value( table->currentRow(), table->currentColumn() ).toString();
    if( !text.isEmpty() ) {
        kapp->clipboard()->setText( text );

        QSqlRecord* buffer = table->sqlCursor()->primeUpdate();
        if( buffer ) {
            buffer->setValue( table->horizontalHeader()->label( table->currentColumn() ), "" );
            table->sqlCursor()->update();
            table->refresh();
        }

    }*/
}

void DatabaseBrowser::copy()
{
    /*QString text = table->value( table->currentRow(), table->currentColumn() ).toString();
    if( !text.isEmpty() )
        kapp->clipboard()->setText( text );*/
}

void DatabaseBrowser::paste()
{
    /*QString text = kapp->clipboard()->text();
    if( !text.isEmpty() ) {
        QSqlRecord* buffer = table->sqlCursor()->primeUpdate();
        if( buffer ) {
            buffer->setValue( table->horizontalHeader()->label( table->currentColumn() ), text );
            table->sqlCursor()->update();
            table->refresh();
        }
    }*/

}

void DatabaseBrowser::import()
{
    new CSVImportDlg( this );
}

#include "databasebrowser.moc"
