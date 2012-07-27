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
//#include "mydatatable.h"// currently not used by Kbarcode
#include "definition.h"
#include "sqltables.h"
#include "csvimportdlg.h"

// Qt includes
#include <qclipboard.h>
#include <QTableView>
#include <QSqlTableModel>
//Added by qt3to4:
#include <QWidget>

// KDE includes
#include <kaction.h>
#include <kapplication.h>
#include <kfinddialog.h>
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
    m_findOptions = 0;

    table = new QTableView( this );
    setCentralWidget( table );
    
    model = new QSqlTableModel( this, *(SqlTables::getInstance()->database()) );
    model->setTable( _database );
    model->setEditStrategy( QSqlTableModel::OnFieldChange );
    table->setModel( model );

    statusBar()->insertPermanentItem( i18n("Current Table: <b>" ) + _database, CUR_TABLE_ID, 0 );
    statusBar()->setSizeGripEnabled( true );
    statusBar()->show();

    database = _database;

    /*connect( (QObject*) table, SIGNAL( cursorChanged( QSql::Op ) ),
             SqlTables::getInstance(), SIGNAL( tablesChanged() ) );*/// -!F: original, what is the correct replacement?
    connect( model, SIGNAL( dataChanged(const QModelIndex &, const QModelIndex & ) ),
             SqlTables::getInstance(), SIGNAL( tablesChanged() ) );

    /*connect( this, SIGNAL( connectedSQL() ), this, SLOT( setupSql() ) );*/// -!F: original, keep, this line gives the warning: Object::connect: No such signal DatabaseBrowser::connectedSQL()
    connect( SqlTables::getInstance(), SIGNAL( connectedSQL() ), this, SLOT( setupSql() ) );// -!F: is this the right replacement of the previous line ?

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
    KAction* afind = KStandardAction::find( this, SLOT( find() ), actionCollection() );
    
    editMenu->addAction( acut );
    editMenu->addAction( acopy );
    editMenu->addAction( apaste );
    
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
    
    actionCollection()->action( "help_about_app" )->setIcon( KIcon ( KStandardDirs::locate(
        "appdata", QString( "hi16-app-kbarcode.png" ) ) ) );;
    setWindowIcon( KIcon( KStandardDirs::locate( "appdata", QString("hi16-app-kbarcode.png") ) ) );
    
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
    model->select();
    table->resizeColumnsToContents();
}

void DatabaseBrowser::find()
{
    if( m_find ) {
        delete m_find;
        m_find = 0L;
    }
    
    findDlg = new KFindDialog( this );
        
    findDlg->setPattern( m_findPattern );
    findDlg->setOptions( m_findOptions );
    
    connect( findDlg, SIGNAL( okClicked() ), this, SLOT( slotFindNext() ) );
    
    findDlg->exec();
    delete findDlg;
    findDlg = 0;
}

void DatabaseBrowser::slotFindNext()
{
    if( ( m_findOptions & KFind::FindBackwards ) == KFind::FindBackwards ) {
        findNextBackwards();
    } else {
        findNextForwards();
    }
}

void DatabaseBrowser::findNextForwards()
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
                    m_findCurrentRow++;
                }
            }
        }

        if ( res == KFind::NoMatch ) {// i.e. at end and there was no match
            /*<Call either  m_find->displayFinalDialog(); m_find->deleteLater(); m_find = 0L;
            or           if ( m_find->shouldRestart() ) { reinit (w/o FromCursor) and call slotFindNext(); }
                         else { m_find->closeFindNextDialog(); }>*/
            m_findCurrentRow = 0;
        } else {// There was a match so the matching row is selected and a dialog "Find next" is displayed.
            m_findCurrentRow++;// continue the find in the next row if a user clicks on the "Find next" button.
        }
    } else {// Create the KFind instance:
        createKFindInstance();
    }
}

void DatabaseBrowser::findNextBackwards()
{
    if( m_find ) {
        KFind::Result res = KFind::NoMatch;
        int column = 0;
        
        while ( ( res == KFind::NoMatch ) && ( m_findCurrentRow >= 0 ) ) {
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
                    m_findCurrentRow--;// find backwards
                }
            }
        }

        if ( res == KFind::NoMatch ) {// i.e. at end and there was no match
            /*<Call either  m_find->displayFinalDialog(); m_find->deleteLater(); m_find = 0L;
            or           if ( m_find->shouldRestart() ) { reinit (w/o FromCursor) and call slotFindNext(); }
                        else { m_find->closeFindNextDialog(); }>*/
            m_findCurrentRow = model->rowCount() - 1;
        } else {// There was a match so the matching row is selected and a dialog "Find next" is displayed.
            m_findCurrentRow--;// continue the find in the next row if a user clicks on the "Find next" button.
        }
    } else {// Create the KFind instance:
        createKFindInstance();
    }
}

void DatabaseBrowser::createKFindInstance()
{
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
    
    // Set a row that we will start searching from:
    if( ( m_findOptions & KFind::FromCursor ) == KFind::FromCursor ) {
        m_findCurrentRow = table->currentIndex().row();
    } else {
        if( ( m_findOptions & KFind::FindBackwards ) == KFind::FindBackwards ) {
            m_findCurrentRow = model->rowCount() - 1;
        } else {
            m_findCurrentRow = 0;
        }
    }
    
    slotFindNext();// Begin the search
}

void DatabaseBrowser::slotHighlight( const QString & text, int matchingIndex, int matchedLength )
{
    table->selectRow( m_findCurrentRow );
    //table->scrollTo( model->index( m_findCurrentRow, m_findCurrentColumn ) );
}

void DatabaseBrowser::cut()
{
    QString text = table->currentIndex().data().toString();
    if( !text.isEmpty() ) {
        kapp->clipboard()->setText( text );
        model->setData( table->currentIndex(), "" );
    }
}

void DatabaseBrowser::copy()
{
    QString text = table->currentIndex().data().toString();
    if( !text.isEmpty() ) {
        kapp->clipboard()->setText( text );
    }
}

void DatabaseBrowser::paste()
{
    QString text = kapp->clipboard()->text();
    if( !text.isEmpty() ) {
        model->setData( table->currentIndex(), text );
    }
}

void DatabaseBrowser::import()
{
    new CSVImportDlg( this );
}

#include "databasebrowser.moc"
