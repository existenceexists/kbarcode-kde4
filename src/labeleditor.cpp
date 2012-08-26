/***************************************************************************
                          labeleditor.cpp  -  description
                             -------------------
    begin                : Die Apr 23 2002
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

#include "labeleditor.h"

#include "barcodecombo.h"
#include "barcodegenerator.h"
#include "barcodeitem.h"
#include "barcodeprinterdlg.h"
#include "batchprinter.h"
#include "batchassistant.h"
#include "commands.h"
#include "configdialog.h"
#include "databasebrowser.h"
#include "documentitemdlg.h"
#include "kbarcode.h"
#include "kbarcodesettings.h"
#include "label.h"
#include "measurements.h"
#include "mimesources.h"
#include "multilineeditdlg.h"
#include "mycanvasitem.h"
#include "mycanvasview.h"
#include "newlabel.h"
#include "previewdialog.h"
#include "printersettings.h"
#include "printlabeldlg.h"
#include "rectitem.h"
#include "rectsettingsdlg.h"
#include "sqltables.h"
#include "tcanvasitem.h"
#include "tokendialog.h"
#include "tokenprovider.h"
#include "zplutils.h"
//NY34
#include "textlineitem.h"
//NY34

// QT includes
#include <qbuffer.h>
#include <qcheckbox.h>
#include <qclipboard.h>
#include <qdom.h>
#include <qimage.h>
#include <qinputdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmap.h>
#include <qmime.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qsqlquery.h>
#include <qtooltip.h>
#include <qvalidator.h>
#include <QtXml>
#include <qregexp.h>
#include <QList>
#include <QByteArray>
//#include <QCloseEvent>// -!F: original, delete
#include <qprinter.h>
#include <QPaintDevice>
#include <QDesktopWidget>
#include <QAction>
#include <QGraphicsItem>
#include <QUndoCommand>
#include <QTextDocument>

#include <QDebug>// -!F: delete

// KDE includes
#include <kabc/stdaddressbook.h>
#include <kaction.h>
#include <kapplication.h>
#include <kcolordialog.h>
#include <kundostack.h>
#include <kcombobox.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kimageio.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kmenu.h>
#include <kpushbutton.h>
#include <krun.h>
#include <kstatusbar.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kglobal.h>
#include <krecentfilesaction.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <KUrl>
#include <kstandardaction.h>
#include <ktoolbar.h>

#include "tcanvasitem.h"
#include "rectitem.h"
#include "textitem.h"
#include "imageitem.h"
#include "barcodeitem.h"
#include "lineitem.h"

#define STATUS_ID_SIZE 100
#define STATUS_ID_TEMPLATE 101
#define STATUS_ID_MOUSE 102

/*#define ID_LOCK_ITEM 8000*/// -!F: original, delete

#define CANVAS_UPDATE_PERIOD 50

#define KBARCODE_UNDO_LIMIT 25

using namespace KABC;

LabelEditor::LabelEditor( QWidget *parent, QString _filename, Qt::WindowFlags f, Qt::WidgetAttribute waf )
    : MainWindow( parent, f )
{
    setAttribute( waf );
    setWindowIcon( KIcon( KStandardDirs::locate( "appdata", QString( "hi16-app-kbarcode.png" ) ) ) );// -!F: 
    
    
    undoAct = redoAct = NULL;
    history = NULL;
    
    m_sonnetDialog = NULL;
    spellCheckedItems = NULL;
    correctedTexts = NULL;
    spellCheckedItemNumber = 0;
    positionInSpellCheckedText = -1;
    positionInCorrectedText = 0;
    wordWasReplaced = false;
    currentTextFragmentEndIndex = 0;
    findNextTextFragment = false;
    spellCheckedWordLength = 0;

    description = QString::null;
    d = new Definition();
    /*m_token = new TokenProvider( (QPaintDevice*) KApplication::desktop() );*/// -!F: keep
    m_token = new TokenProvider( KApplication::desktop() );

    statusBar()->insertPermanentItem( "", STATUS_ID_TEMPLATE, 0 );
    statusBar()->insertPermanentItem( "", STATUS_ID_SIZE, 0 );
    statusBar()->insertPermanentItem( "", STATUS_ID_MOUSE, 2 );
    statusBar()->setSizeGripEnabled( true );
    statusBar()->show();

    c = new MyCanvas( this );
    /*c->setDoubleBuffering( true );*/// -!F: original, Is it needed? What is the right replacement of this?
    /*c->setUpdatePeriod( CANVAS_UPDATE_PERIOD );*/// -!F: original, Is it needed? What is the right replacement of this?

    cv = new MyCanvasView( d, c, this );
    cv->setPosLabel( statusBar(), STATUS_ID_MOUSE );
    setCentralWidget( cv );

    //clearLabel();

    setupActions();
    setupContextMenu();
    setAutoSaveSettings( QString("Window") + QString(objectName()), true );

    clearLabel();

    loadConfig();
    show();

//    if( isFirstStart() )
//        moveDockWindow( tools, Qt::DockLeft );

    connect( cv, SIGNAL( doubleClickedItem(TCanvasItem*) ), this, SLOT( doubleClickedItem(TCanvasItem*) ) );
    connect( cv, SIGNAL( showContextMenu(QPoint) ), this, SLOT( showContextMenu(QPoint) ) );
    connect( cv, SIGNAL( movedSomething() ), this, SLOT( setEdited() ) );
    connect( cv, SIGNAL( movedSomething() ), c, SLOT( update() ) );
    connect( KBarcodeSettings::getInstance(), SIGNAL( updateGrid( int ) ), cv, SLOT( updateGUI() ) );
    connect( kapp, SIGNAL( aboutToQuit() ), this, SLOT( saveConfig() ) );
 
    /*connect( history, SIGNAL( commandExecuted( K3Command *) ), cv, SLOT( updateGUI() ) );
    connect( history, SIGNAL( commandExecuted( K3Command *) ), this, SLOT( setEdited() ) );*/

    if( !_filename.isEmpty() )
        openUrl( _filename );
}

LabelEditor::~LabelEditor()
{
    delete m_token;
    delete d;
    delete history;
    delete spellCheckedItems;
    delete correctedTexts;
}

void LabelEditor::loadConfig()
{
    KConfigGroup config = KGlobal::config()->group( "RecentFiles" );
    recentAct->loadEntries( config );

    config = KGlobal::config()->group( "LabelEditor" );

    gridAct->setChecked( config.readEntry("gridenabled", false ) );
    toggleGrid();
}

void LabelEditor::saveConfig()
{
    KConfigGroup config = KGlobal::config()->group( "RecentFiles" );

    recentAct->saveEntries( config );

    config = KGlobal::config()->group( "LabelEditor" );
    config.writeEntry("gridenabled", gridAct->isChecked() );

    config.sync();

    MainWindow::saveConfig();
}

void LabelEditor::createCommandHistory()
{
    if( undoAct && redoAct )
    {
	/*editMenu->removeAction( undoAct );
	toolBar()->removeAction( undoAct );
	editMenu->removeAction( redoAct );
	toolBar()->removeAction( redoAct );
	actionCollection()->removeAction( undoAct );
	actionCollection()->removeAction( redoAct );*/// -!F: original, delete
        /*undoAct->setEnabled( false );
        redoAct->setEnabled( false );*/// -!F: delete
        /*actionCollection()->removeAction( undoAct );
        actionCollection()->removeAction( redoAct );*/
    }

    history = new KUndoStack( this );
    cv->setHistory( history );

    // Don't set the undo limit anymore becouse in KDE4 it causes deletion ot items if the limit is reached:
    //history->setUndoLimit( KBARCODE_UNDO_LIMIT );
    
    /*connect( undoAct, SIGNAL(triggered(bool)), history, SLOT(undo()) );
    connect( redoAct, SIGNAL(triggered(bool)), history, SLOT(redo()) );
    
    connect( history, SIGNAL(commandHistoryChanged()), this, SLOT(setEdited()) );*/// -!F: original
    connect( history, SIGNAL(cleanChanged(bool)), this, SLOT(setEdited(bool)) );
}

void LabelEditor::createCommandHistoryActions()
{
    /*undoAct = (KAction*)actionCollection()->action("edit_undo");
    redoAct = (KAction*)actionCollection()->action("edit_redo");

    editMenu->insertAction( editMenu->actions()[0], undoAct );
    editMenu->insertAction( editMenu->actions()[1], redoAct );

    toolBar()->insertAction( toolBar()->actions()[5], undoAct );
    toolBar()->insertAction( toolBar()->actions()[6], redoAct );*/// -!F: original, delete
    
    if( undoAct && redoAct ) {
        toolBar()->removeAction(undoAct);
        toolBar()->removeAction(redoAct);
        // Remove the actions from the "Edit" menu:
        menuBar()->actions()[1]->menu()->removeAction( undoAct );
        menuBar()->actions()[1]->menu()->removeAction( redoAct );
        actionCollection()->removeAction( undoAct );
        actionCollection()->removeAction( redoAct );
        undoAct = 0;
        redoAct = 0;
    }
    
    undoAct = history->createUndoAction( actionCollection() );
    redoAct = history->createRedoAction( actionCollection() );
    
    toolBar()->addAction( undoAct );
    toolBar()->addAction( redoAct );
    // Insert the actions into the "Edit" menu:
    menuBar()->actions()[1]->menu()->insertAction( menuBar()->actions()[1]->menu()->actions()[0], undoAct );
    menuBar()->actions()[1]->menu()->insertAction( menuBar()->actions()[1]->menu()->actions()[1], redoAct );
}

void LabelEditor::clearLabel()
{
    TCanvasItem* citem;
    QList<QGraphicsItem *>::iterator it;

    description = QString::null;

    if( history ) {
        history->clear();
        delete history;
        history = NULL;
    }
    createCommandHistory();
    createCommandHistoryActions();

    /*connect( history, SIGNAL( commandExecuted( K3Command *) ), cv, SLOT( updateGUI() ) );
    connect( history, SIGNAL( commandExecuted( K3Command *) ), this, SLOT( setEdited() ) );*/
    connect( history, SIGNAL( indexChanged( int ) ), cv, SLOT( updateGUI() ) );

    m_edited = false;

    QList<QGraphicsItem *> list = c->items();
    for (it = list.begin(); it != list.end(); ++it)
    {
	citem = static_cast<TCanvasItem*>(*it);
	citem->remRef();
    }

    updateInfo();
    c->update();
    cv->repaint();
}

bool LabelEditor::save()
{
    bool ret;
    if( filename.isEmpty() )
        ret = saveas();
    else
        ret = save( filename );

    KUrl url;
    url.setPath( filename );
    recentAct->addUrl( url );

    updateInfo();

    return ret;
}

bool LabelEditor::saveas()
{
    QString name = KFileDialog::getSaveFileName ( KUrl(), "*.kbarcode", this );
    if(name.isEmpty())
        return false;

    if( name.right(9).toLower() != ".kbarcode" )
        name += ".kbarcode";

    return save( name );
}

bool LabelEditor::save( QString name )
{
    if( QFile::exists( name ) )
        QFile::remove( name );

    QFile f( name );
    if ( !f.open( QIODevice::WriteOnly ) )
        return false;

    save( &f );

    m_token->setLabelName( filename.right( filename.length() - filename.lastIndexOf( "/" ) - 1 ) );
    // maybe we should redraw all items on the canvas now.
    // if there is a label with [filename], the filename might not
    // get updated if the label gets saved with another filename.

    filename = name;
    /*history->documentSaved();*/
    history->setClean();
    m_edited = false;

    enableActions();
    setCaption( filename, false );

    return true;
}

void LabelEditor::save( QIODevice* device )
{

    QDomDocument doc( "KBarcodeLabel" );
    QDomElement root = doc.createElement( "kbarcode" );
    doc.appendChild( root );

    writeXMLHeader( &root, description, d );

    QList<QGraphicsItem *> list = c->items();
    for( int i = 0; i < list.count(); i++ )
    {
        TCanvasItem* item = static_cast<TCanvasItem*>(list[i]);
        DocumentItem* ditem = item->item();

        writeXMLDocumentItem( &root, &ditem );
    }

    QByteArray xml = doc.toByteArray();
    device->write( xml, xml.length() );
    device->close();
}

bool LabelEditor::open()
{
    QString name = KFileDialog::getOpenFileName ( KUrl(), "*.kbarcode", this, i18n("Select Label") );
    if(name.isEmpty()) return false;

    return openUrl( name );
}

bool LabelEditor::openUrl( const QString & url )
{
    if( url.isEmpty() ) {
        return open();
    }

    filename = url;
    setCaption( filename, false );
    m_token->setLabelName( filename.right( filename.length() - filename.lastIndexOf( "/" ) - 1 ) );

    QFile f( filename );
    if ( !f.open( QIODevice::ReadOnly ) )
        return false;

    clearLabel();

    QDomDocument doc( "KBarcodeLabel" );
    if ( !doc.setContent( &f ) ) {
        f.close();
        return false;
    }
    f.close();

    bool kbarcode18 = false;
    delete d;
    d = NULL;

    readXMLHeader( &doc, description, kbarcode18, &d );

    if( !d || d->getId() == -1 )
    {
        KMessageBox::error( this, QString( i18n("<qt>The file <b>%1</b> cannot be loaded as the label definition is missing.</qt>") ).arg( filename ) );
        return false;
    }

    cv->setDefinition( d );

    DocumentItemList list;
    readDocumentItems( &list, &doc, m_token, kbarcode18 );
    for( int i=0;i<list.count();i++ )
    {
        TCanvasItem* citem = new TCanvasItem( cv );
        citem->setItem( list.at( i ) );
	citem->addRef();
        c->addItem( citem );
        citem->show();
        citem->update();
        // -!F: is the code in this for loop all the code necessary to load an item?
    }
    list.clear();

    KUrl murl;
    murl.setPath( filename );
    recentAct->addUrl( murl );

    enableActions();
    cv->repaint();

    return true;
}

bool LabelEditor::newLabel()
{
    NewLabel* nl = new NewLabel( this );
    if( nl->exec() != QDialog::Accepted ) {
        delete nl;
        return false;
    }

    closeLabel();

    if( !nl->empty() )
    {
        d->setId( nl->labelId() );
        clearLabel();
        cv->setDefinition( d );
    }

    delete nl;

    filename = QString::null;
    m_token->setLabelName( filename.right( filename.length() - filename.lastIndexOf( "/" ) - 1 ) );
    setCaption( filename, false );
    enableActions();

    return true;
}

void LabelEditor::setupActions()
{
    /*KAction* newAct = KStandardAction::openNew( this, SLOT(startEditor()), actionCollection() );
    KAction* loadAct = KStandardAction::open( this, SLOT(startLoadEditor()), actionCollection() );
    KAction* quitAct = KStandardAction::quit(kapp, SLOT(quit()), actionCollection());*/// -!F: original, delete
    KStandardAction::openNew( this, SLOT(startEditor()), actionCollection() );
    KStandardAction::open( this, SLOT(startLoadEditor()), actionCollection() );
    KStandardAction::quit(kapp, SLOT(quit()), actionCollection());
    /*KAction* closeAct = KStandardAction::close( this, SLOT( close() ), actionCollection(), "close" );*/// -!F: original, delete
    KStandardAction::close( this, SLOT( close() ), actionCollection() );
    /*closeLabelAct = new KAction( i18n("Close &Label" ), 0, 0, this, SLOT( closeLabel() ), actionCollection() );*/// -!F: original, delete
    closeLabelAct = new KAction( this );
    closeLabelAct->setText( i18n("Close &Label") );
    actionCollection()->addAction( "closeLabelAct", closeLabelAct );
    connect( closeLabelAct, SIGNAL(triggered(bool)), this, SLOT(closeLabel()) );

    /*recentAct = new KRecentFilesAction( i18n("&Recent Files"), 0, this, SLOT( loadRecentEditor( const KUrl& ) ) );*/// -!F: original, keep
    recentAct = new KRecentFilesAction( this );
    recentAct->setText( i18n("&Recent Files") );
    actionCollection()->addAction( "recentAct", recentAct );
    /*connect( recentAct, SIGNAL(triggered(bool)), this, SLOT(loadRecentEditor( const KUrl& )) );*/// -!F: There is no slot loadRecentEditor(). See the previous line that is commented out.

    /*KAction* importPrintFileAct = new KAction( i18n("&Import and Print Batch File..."), BarIconSet( "fileprint" ), 0, this, SLOT( batchPrint() ), actionCollection() );*/// -!F: original, delete
    KAction* importPrintFileAct = new KAction( this );
    importPrintFileAct->setText( i18n("&Import and Print Batch File...") );
    importPrintFileAct->setIcon( KIcon( "document-print" ) );
    actionCollection()->addAction( "importPrintFileAct", importPrintFileAct );
    connect( importPrintFileAct, SIGNAL(triggered(bool)), this, SLOT(batchPrint()) );

    /*saveAct = KStandardAction::save( this, SLOT( save() ), actionCollection(), "save" );*/// -!F: original, delete
    saveAct = KStandardAction::save( this, SLOT( save() ), actionCollection() );
    /*saveAsAct = KStandardAction::saveAs( this, SLOT( saveas() ), actionCollection(), "saveas" );*/// -!F: original, delete
    saveAsAct = KStandardAction::saveAs( this, SLOT( saveas() ), actionCollection() );
    /*descriptionAct = new KAction( i18n("&Change description..."), 0, 0, this, SLOT(changeDes()), actionCollection() );*/// -!F: original, delete
    descriptionAct = new KAction( this );
    descriptionAct->setText( i18n("&Change description...") );
    actionCollection()->addAction( "descriptionAct", descriptionAct );
    connect( descriptionAct, SIGNAL(triggered(bool)), this, SLOT(changeDes()) );
    /*deleteAct = new KAction( i18n("&Delete Object"), QIcon( BarIcon("editdelete") ), Qt::Key_Delete, cv, SLOT( deleteCurrent() ), actionCollection() );*/// -!F: original, delete
    deleteAct = new KAction( this );
    deleteAct->setText( i18n("&Delete Object") );
    deleteAct->setIcon( KIcon( "edit-delete" ) );
    deleteAct->setShortcut( Qt::Key_Delete );
    actionCollection()->addAction( "deleteAct", deleteAct );
    connect( deleteAct, SIGNAL(triggered(bool)), cv, SLOT(deleteCurrent()) );
    /*editPropAct = new KAction( i18n("&Properties..."), 0, 0, this, SLOT( doubleClickedCurrent() ), actionCollection() );*/// -!F: original, delete
    editPropAct = new KAction( this );
    editPropAct->setText( i18n("&Properties...") );
    actionCollection()->addAction( "editPropAct", editPropAct );
    connect( editPropAct, SIGNAL(triggered(bool)), this, SLOT(doubleClickedCurrent()) );
    /*printAct = KStandardAction::print( this, SLOT( print() ), actionCollection(), "print" );*/// -!F: original, delete
    printAct = KStandardAction::print( this, SLOT( print() ), actionCollection() );
    /*bcpAct = new KAction( i18n("Print to &Barcode Printer..."), 0, 0, this, SLOT( printBCP() ), actionCollection() );*/// -!F: original, delete
    bcpAct = new KAction( this );
    bcpAct->setText( i18n("Print to &Barcode Printer...") );
    actionCollection()->addAction( "bcpAct", bcpAct );
    connect( bcpAct, SIGNAL(triggered(bool)), this, SLOT(printBCP()) );
    /*imgAct = new KAction( i18n("Print to &Image..."), 0, 0, this, SLOT(printImage() ), actionCollection() );*/// -!F: original, delete
    imgAct = new KAction( this );
    imgAct->setText( i18n("Print to &Image...") );
    actionCollection()->addAction( "imgAct", imgAct );
    connect( imgAct, SIGNAL(triggered(bool)), this, SLOT(printImage()) );
    /*changeSizeAct = new KAction( i18n("&Change Label..."), 0, 0, this, SLOT( changeSize() ), actionCollection() );*/// -!F: original, delete
    changeSizeAct = new KAction( this );
    changeSizeAct->setText( i18n("&Change Label...") );
    actionCollection()->addAction( "changeSizeAct", changeSizeAct );
    connect( changeSizeAct, SIGNAL(triggered(bool)), this, SLOT(changeSize()) );
    /*barcodeAct = new KAction( i18n("Insert &Barcode"), QIcon( BarIcon("barcode") ), 0, this, SLOT( insertBarcode() ), actionCollection() );*/// -!F: original, delete
    barcodeAct = new KAction( this );
    barcodeAct->setText( i18n("Insert &Barcode") );
    barcodeAct->setIcon( KIcon( "view-barcode-add" ) );
    actionCollection()->addAction( "barcodeAct", barcodeAct );
    connect( barcodeAct, SIGNAL(triggered(bool)), this, SLOT(insertBarcode()) );
    barcodeAct->setEnabled( Barkode::haveBarcode() );

    /*pictureAct = new KAction( i18n("Insert &Picture"), QIcon( BarIcon("inline_image") ), 0, this, SLOT( insertPicture() ), actionCollection() );*/// -!F: original, delete
    pictureAct = new KAction( this );
    pictureAct->setText( i18n("Insert &Picture") );
    pictureAct->setIcon( KIcon( "insert-image" ) );
    actionCollection()->addAction( "pictureAct", pictureAct );
    connect( pictureAct, SIGNAL(triggered(bool)), this, SLOT(insertPicture()) );
    /*textAct = new KAction( i18n("Insert &Text"), QIcon( BarIcon("text") ), 0, this, SLOT( insertText() ), actionCollection() );*/// -!F: original, delete
    textAct = new KAction( this );
    textAct->setText( i18n("Insert &Text") );
    textAct->setIcon( KIcon( "text-field" ) );
    actionCollection()->addAction( "textAct", textAct );
    connect( textAct, SIGNAL(triggered(bool)), this, SLOT(insertText()) );
    /*textDataAct = new KAction( i18n("Insert &Data Field"), QIcon( BarIcon("contents") ), 0, this, SLOT( insertDataText() ), actionCollection() );*/// -!F: original, delete
    textDataAct = new KAction( this );
    textDataAct->setText( i18n("Insert &Data Field") );
    textDataAct->setIcon( KIcon( "view-table-of-contents-ltr" ) );
    /*textDataAct->setIcon( KIcon( "code-context" ) );*/// -!F: added, keep
    actionCollection()->addAction( "textDataAct", textDataAct );
    connect( textDataAct, SIGNAL(triggered(bool)), this, SLOT(insertDataText()) );
    /*textLineAct = new KAction( i18n("Insert &Text Line"), QIcon( BarIcon("text") ), 0, this, SLOT( insertTextLine() ), actionCollection() );*/// -!F: original, delete
    textLineAct = new KAction( this );
    textLineAct->setText( i18n("Insert &Text Line") );
    /*textLineAct->setIcon( KIcon( "text-plain" ) );*/// -!F: added, keep
    textLineAct->setIcon( KIcon( "insert-text" ) );
    actionCollection()->addAction( "textLineAct", textLineAct );
    connect( textLineAct, SIGNAL(triggered(bool)), this, SLOT(insertTextLine()) );
    /*lineAct = new KAction( i18n("Insert &Line"), QIcon( BarIcon("kbarcodelinetool") ), 0, this, SLOT( insertLine() ), actionCollection() );*/// -!F: original, delete
    lineAct = new KAction( this );
    lineAct->setText( i18n("Insert &Line") );
    lineAct->setIcon( KIcon( KStandardDirs::locate( "appdata", "hi16-action-kbarcodelinetool.png" ) ) );
    actionCollection()->addAction( "lineAct", lineAct );
    connect( lineAct, SIGNAL(triggered(bool)), this, SLOT(insertLine()) );
    /*rectAct = new KAction( i18n("Insert &Rectangle"), QIcon( BarIcon("kbarcoderect") ), 0, this, SLOT( insertRect() ), actionCollection() );*/// -!F: original, delete
    rectAct = new KAction( this );
    rectAct->setText( i18n("Insert &Rectangle") );
    rectAct->setIcon( KIcon( KStandardDirs::locate( "appdata", "hi32-action-kbarcoderect.png" ) ) );
    actionCollection()->addAction( "rectAct", rectAct );
    connect( rectAct, SIGNAL(triggered(bool)), this, SLOT(insertRect()) );
    /*circleAct = new KAction( i18n("Insert &Ellipse"), QIcon( BarIcon("kbarcodeellipse") ), 0, this, SLOT( insertCircle() ), actionCollection() );*/// -!F: original, delete
    circleAct = new KAction( this );
    circleAct->setText( i18n("Insert &Ellipse") );
    circleAct->setIcon( KIcon( KStandardDirs::locate( "appdata", "hi16-action-kbarcodeellipse.png" ) ) );
    actionCollection()->addAction( "circleAct", circleAct );
    connect( circleAct, SIGNAL(triggered(bool)), this, SLOT(insertCircle()) );
    /*spellAct = KStandardAction::spelling( this, SLOT(spellCheck()), actionCollection(), "spell" );*/// -!F: original, delete
    spellAct = KStandardAction::spelling( this, SLOT(spellCheck()), actionCollection() );
    /*gridAct = new KToggleAction( i18n("&Grid"), QIcon( BarIcon("kbarcodegrid") ), 0, this, SLOT( toggleGrid() ), actionCollection() );*/// -!F: original, delete
    gridAct = new KToggleAction( KIcon( KStandardDirs::locate( "appdata", "hi16-action-kbarcodegrid.png" ) ), i18n("&Grid"), this );
    actionCollection()->addAction( "gridAct", gridAct );
    connect( gridAct, SIGNAL(triggered(bool)), this, SLOT(toggleGrid()) );
    /*previewAct = new KAction( i18n("&Preview..."), 0, 0, this, SLOT( preview() ), actionCollection() );*/// -!F: original, delete
    previewAct = new KAction( this );
    previewAct->setText( i18n("&Preview...") );
    actionCollection()->addAction( "previewAct", previewAct );
    connect( previewAct, SIGNAL(triggered(bool)), this, SLOT(preview()) );
    /*sep = new KActionSeparator( this );*/// -!F: original, delete
    /*sep = new QAction( this );
    sep->setSeparator( true );*/// -!F: delete
    /*cutAct = KStandardAction::cut( this, SLOT( cut() ), actionCollection(), "cut" );*/// -!F: original, delete
    cutAct = KStandardAction::cut( this, SLOT( cut() ), actionCollection() );
    /*copyAct = KStandardAction::copy( this, SLOT( copy() ), actionCollection(), "copy" );*/// -!F: original, delete
    copyAct = KStandardAction::copy( this, SLOT( copy() ), actionCollection() );
    /*pasteAct = KStandardAction::paste( this, SLOT( paste() ), actionCollection(), "paste" );*/// -!F: original, delete
    pasteAct = KStandardAction::paste( this, SLOT( paste() ), actionCollection() );
    /*selectAllAct = KStandardAction::selectAll( cv, SLOT( selectAll() ), actionCollection(), "select_all" );*/// -!F: original, delete
    selectAllAct = KStandardAction::selectAll( cv, SLOT( selectAll() ), actionCollection() );
    /*deSelectAllAct = KStandardAction::deselect( cv, SLOT( deSelectAll() ), actionCollection(), "de_select_all" );*/// -!F: original, delete
    deSelectAllAct = KStandardAction::deselect( cv, SLOT( deSelectAll() ), actionCollection() );
    /*addressBookAct = new KAction( i18n("Address&book"), QIcon( BarIcon("kaddressbook") ), 0, this, SLOT( launchAddressBook() ), actionCollection() );*/// -!F: original, delete
    addressBookAct = new KAction( this );
    addressBookAct->setText( i18n("Address&book") );
    addressBookAct->setIcon( KIcon( "kaddressbook" ) );
    actionCollection()->addAction( "addressBookAct", addressBookAct );
    connect( addressBookAct, SIGNAL(triggered(bool)), this, SLOT(launchAddressBook()) );
    /*KAction* singleBarcodeAct = new KAction(i18n("&Create Single Barcode..."), "",
                                0, this, SLOT(startBarcodeGen()),
                                actionCollection(), "create" );*/// -!F: original, delete
    KAction* singleBarcodeAct = new KAction( this );
    singleBarcodeAct->setText( i18n("&Create Single Barcode...") );
    actionCollection()->addAction( "create", singleBarcodeAct );
    connect( singleBarcodeAct, SIGNAL(triggered(bool)), this, SLOT(startBarcodeGen()) );
    singleBarcodeAct->setEnabled( Barkode::haveBarcode() );
    
    /*undoAct = KStandardAction::undo( history, SLOT( undo() ), actionCollection() );
    redoAct = KStandardAction::redo( history, SLOT( redo() ), actionCollection() );
    undoAct->setEnabled( false );
    redoAct->setEnabled( false );*/// -!F: delete

    /*toolBar()->addAction( newAct );
    toolBar()->addAction( loadAct );
    toolBar()->addAction( saveAct );
    toolBar()->addAction( printAct );
    toolBar()->addSeparator();
    toolBar()->addAction( cutAct );
    toolBar()->addAction( copyAct );
    toolBar()->addAction( pasteAct );*/

    /*tools = new KToolBar( this, this->leftDock(), true);*/// -!F: original, delete
    /*tools = toolBar();*/// -!F: delete
    /*tools->setAllowedAreas( Qt::LeftToolBarArea );*/// -!F: added, try to uncomment this to find out if is needed
    
    /*QAction * separatorTools1 = new QAction( this );
    separatorTools1->setSeparator(true);*/// -!F: delete

    /*tools->addAction( barcodeAct );
    tools->addAction( pictureAct );
    tools->addAction( textAct );
    tools->addAction( textDataAct );
    tools->addAction( textLineAct );
    tools->addAction( lineAct );
    tools->addAction( rectAct );
    tools->addAction( circleAct );
    tools->addSeparator();
//    spellAct->plug( tools );  // KDE 3.2
    tools->addAction( gridAct );*/

    /*MainWindow::setupActions();*/// -!F: original, delete
    setupGUI( Default, KStandardDirs::locate( "appdata", QString("labeleditorui.rc") ) );
    connect( recentAct, SIGNAL( urlSelected( const KUrl& ) ), this, SLOT( startLoadRecentEditor( const KUrl& ) ) );

    /*KMenu* fileMenu = new KMenu( this );
    editMenu = new KMenu( this );
    KMenu* viewMenu = new KMenu( this );
    KMenu* insMenu = new KMenu( this );
    KMenu* toolMenu = new KMenu( this );
    KMenu* barMenu = new KMenu( this );*/// -!F: original, delete

    /*menuBar()->removeItemAt( 0 );
    menuBar()->insertItem( i18n("&File"), fileMenu, -1, 0 );
    menuBar()->insertItem( i18n("&Edit"), editMenu, -1, 1 );
    menuBar()->insertItem( i18n("&Insert"), insMenu, -1, 2 );
    menuBar()->insertItem( i18n("&View"), viewMenu, -1, 3 );
    menuBar()->insertItem( i18n("T&ools"), toolMenu, -1, 4 );
    menuBar()->insertItem( i18n("&Barcode"), barMenu, -1, 5 );

    // Menubar
    fileMenu->addAction( newAct );
    fileMenu->addAction( loadAct );
    fileMenu->addAction( recentAct );
    fileMenu->addAction( saveAct );
    fileMenu->addAction( saveAsAct );
    fileMenu->addAction( sep );
    fileMenu->addAction( printAct );
    fileMenu->addAction( bcpAct );
    fileMenu->addAction( imgAct );
    fileMenu->addAction( sep );
    fileMenu->addAction( closeLabelAct );
    fileMenu->addAction( closeAct );
    fileMenu->addAction( quitAct );

    editMenu->addAction( sep );
    editMenu->addAction( cutAct );
    editMenu->addAction( copyAct );
    editMenu->addAction( pasteAct );
    editMenu->addAction( sep );
    editMenu->addAction( selectAllAct );
    editMenu->addAction( deSelectAllAct );
    editMenu->addAction( sep );
    editMenu->addAction( descriptionAct );
    editMenu->addAction( changeSizeAct );
    editMenu->addAction( sep );
    editMenu->addAction( deleteAct );
    editMenu->addAction( editPropAct );

    insMenu->addAction( barcodeAct );
    insMenu->addAction( pictureAct );
    insMenu->addAction( textAct );
    insMenu->addAction( textDataAct );
    insMenu->addAction( textLineAct );
    insMenu->addAction( lineAct );
    insMenu->addAction( rectAct );
    insMenu->addAction( circleAct );

//    spellAct->plug( toolMenu ); // KDE 3.2
    toolMenu->insertSeparator();
    toolMenu->addAction( addressBookAct );

    viewMenu->addAction( gridAct );
    viewMenu->addAction( previewAct );

    barMenu->addAction( singleBarcodeAct );
    barMenu->addAction( importPrintFileAct );*/// -!F: original, delete

    enableActions();
}

void LabelEditor::setupContextMenu()
{
    m_mnuContext = new KMenu( this );
    
    QMenu* orderMenu = m_mnuContext->addMenu( i18n("&Order") );
    orderMenu->addAction( i18n("&On Top"), this, SLOT( onTopCurrent() ) );
    orderMenu->addAction( i18n("&Raise"), this, SLOT( raiseCurrent() ) );
    orderMenu->addAction( i18n("&Lower"), this, SLOT( lowerCurrent() ) );
    orderMenu->addAction( i18n("&To Background"), this, SLOT( backCurrent() ) );

    QMenu* centerMenu = m_mnuContext->addMenu( i18n("&Center") );
    centerMenu->addAction( i18n("Center &Horizontally"), this, SLOT( centerHorizontal() ) );
    centerMenu->addAction( i18n("Center &Vertically"), this, SLOT( centerVertical() ) );

    m_mnuContext->addSeparator();
    m_mnuContext->addAction( KIcon("edit-delete"), i18n("&Delete"), cv, SLOT( deleteCurrent() ) );
    /*m_mnuContext->insertItem( i18n("&Protect Position and Size"), this, SLOT( lockItem() ), 0, ID_LOCK_ITEM );*/// -!F: original, delete
    protectAction = m_mnuContext->addAction( i18n("&Protect Position and Size"), this, SLOT( lockItem() ) );
    protectAction->setCheckable( true );
    m_mnuContext->addSeparator();
    m_mnuContext->addAction( i18n("&Properties"), this, SLOT( doubleClickedCurrent() ) );
}

void LabelEditor::insertBarcode()
{
    NewBarcodeCommand* bc = new NewBarcodeCommand( cv, m_token );
    /*bc->execute();

    BarcodeItem* bcode = static_cast<BarcodeItem*>((static_cast<TCanvasItem*>(bc->createdItem()))->item());
    if( !bcode )
        return;

    history->push( bc, false );*/// -!F how to solve this?
    
    history->push( bc );

    /*BarcodeItem* bcode = static_cast<BarcodeItem*>((static_cast<TCanvasItem*>(bc->createdItem()))->item());
    if( !bcode )
        history->undo();*/// -!F how to solve this?
}

void LabelEditor::insertPicture()
{
    NewPictureCommand* pc = new NewPictureCommand( cv );
    history->push( pc );

    TCanvasItem* item = pc->createdItem();
    doubleClickedItem( item );
}

void LabelEditor::insertText()
{
    insertText( "<nobr>Some Text</nobr>" );
}

void LabelEditor::insertDataText()
{
//    DocumentItemList list = cv->getAllItems();
//    QStringList vars = m_token->listUserVars( &list );

    TokenDialog dlg( m_token, this );
    if( dlg.exec() == QDialog::Accepted )
        insertText( dlg.token() );
}

void LabelEditor::insertText( QString caption )
{
    NewTextCommand* tc = new NewTextCommand( caption, cv, m_token );
    history->push( tc );
}

//NY30
void LabelEditor::insertTextLine()
{
    insertTextLine( "Some Plain Text" );
}

void LabelEditor::insertTextLine( QString caption )
{
    NewTextLineCommand* tc = new NewTextLineCommand( caption, cv, m_token );
    history->push( tc );
}
//NY30

void LabelEditor::insertRect()
{
    NewRectCommand* rc = new NewRectCommand( cv );
    history->push( rc );
}

void LabelEditor::insertCircle()
{
    NewRectCommand* rc = new NewRectCommand( cv, true );
    history->push( rc );
}

void LabelEditor::insertLine()
{
    NewLineCommand* lc = new NewLineCommand( cv );
    history->push( lc );
}

void LabelEditor::changeDes()
{
    /*QString tmp = QInputDialog::getText( i18n("Label Description"),
            i18n("Please enter a description:"), QLineEdit::Normal, description );*/// -!F: original, delete
    QString tmp = QInputDialog::getText( this, i18n("Label Description"),
            i18n("Please enter a description:"), QLineEdit::Normal, description );
    if( !tmp.isEmpty() )
        description = tmp;
}

void LabelEditor::changeSize()
{
    /*NewLabel* nl = new NewLabel( this, true, true );*/// -!F: original, delete
    NewLabel* nl = new NewLabel( this, true, Qt::Window );// -!F: Is this the right replacement?
    nl->setLabelId( d->getId() );
    if( nl->exec() == QDialog::Rejected )
    {
        delete nl;
        return;
    }
    
    d->setId( nl->labelId() );
    cv->setDefinition( d );
    
    updateInfo();
    enableActions();
    // TODO: make sure that all items are redrawn.
    // Otherwise barcodes might become invisible when changing the label
    c->update();
    cv->repaint();  
    delete nl;
}

void LabelEditor::updateInfo()
{
    statusBar()->changeItem( i18n("Size: ") + QString("%1%2 x %3%4").arg(
                 d->getMeasurements().width() ).arg( Measurements::system()
                 ).arg( d->getMeasurements().height()  ).arg( Measurements::system() ), STATUS_ID_SIZE );
    statusBar()->changeItem( i18n("Label Template: ") + d->getProducer() + " - " + d->getType(), STATUS_ID_TEMPLATE );
}

void LabelEditor::doubleClickedItem( TCanvasItem* item )
{
    m_token->setCurrentDocumentItems( cv->getAllItems() );
    DocumentItemDlg dlg( m_token, item->item(), history, this );
    if( dlg.exec() == QDialog::Accepted )
    {
        c->update();
        cv->repaint();
    }
}

void LabelEditor::doubleClickedCurrent()
{
    if( cv->getActive() )
        doubleClickedItem( cv->getActive() );
}

void LabelEditor::showContextMenu( QPoint pos )
{
    TCanvasItemList list = cv->getSelected();
    
    /*m_mnuContext->setItemChecked( ID_LOCK_ITEM, (list[0])->item()->locked() );*/// -!F: original, delete
    protectAction->setChecked( (list[0])->item()->locked() );
    m_mnuContext->popup( pos );
}

void LabelEditor::lockItem()
{
    TCanvasItemList list = cv->getSelected();
    QUndoCommand* mc = new QUndoCommand( i18n("Protected Item") );
    
    DocumentItem* item = NULL;
    for( int i=0;i<list.count();i++)
    {
        item = list[i]->item();
        new LockCommand( !item->locked(), list[i], mc );
    }
    
    history->push( mc );
}

void LabelEditor::print()
{
    PrintLabelDlg pld( this );
    if( pld.exec() != QDialog::Accepted )
        return;

    PrinterSettings::getInstance()->getData()->border = pld.border();

    QPrinter* printer = PrinterSettings::getInstance()->setupPrinter( KUrl( filename ), this );
    if( !printer )
        return;

    BatchPrinter batch( printer, this );
    batch.setMove( pld.position() );

    batchPrint( &batch, pld.labels(), BatchPrinter::POSTSCRIPT );

    delete printer;
}

void LabelEditor::printBCP()
{
    BarcodePrinterDlg dlg(this);
    if( dlg.exec() == QDialog::Accepted )
    {
        QString name( dlg.printToFile() ? dlg.fileName() : dlg.deviceName() );

        BatchPrinter batch( name, dlg.outputFormat(), this );
        batchPrint( &batch, 1, BatchPrinter::BCP );
    }
}

void LabelEditor::printImage()
{
    KFileDialog fd( KUrl( ":save_image" ), KImageIO::pattern( KImageIO::Writing ), this );
    fd.setModal( true );
    fd.setMode( KFile::File );
    fd.setOperationMode( KFileDialog::Saving );
    if( fd.exec() == QDialog::Accepted ) {
        QString path = fd.selectedUrl().path();
        BatchPrinter batch( path, this );
        batchPrint( &batch, 1, BatchPrinter::IMAGE );
    }
}

void LabelEditor::batchPrint( BatchPrinter* batch, int copies, int mode )
{
    QBuffer buffer;
    if( !buffer.open( QIODevice::WriteOnly ) )
        return;

    save( &buffer );

    batch->setBuffer( &buffer );
    batch->setSerial( QString::null, 1 );
    batch->setName( filename );
    batch->setDefinition( d );
    batch->setCustomer( QString::null );
    batch->setEvents( false );

    QList<BatchPrinter::data>* list = new QList<BatchPrinter::data>;
    BatchPrinter::data m_data;
    m_data.number = copies;
    m_data.article_no = QString::null;
    m_data.group = QString::null;
    list->append( m_data );

    batch->setData( list );
    switch( mode )
    {
        default:
        case BatchPrinter::POSTSCRIPT:
            batch->start();
            break;
        case BatchPrinter::IMAGE:
            batch->startImages();
            break;
        case BatchPrinter::BCP:
            batch->startBCP();
            break;
    }
}

void LabelEditor::spellCheck()
{
    /*QUndoCommand* sc = new QUndoCommand( i18n("Spellchecking") );
    bool executeTextChangeCommand = false;
    QList<QGraphicsItem *> list = c->items();
    for( int i = 0; i < list.count(); i++ ) {
        if( ((TCanvasItem*)list[i])->rtti() == eRtti_Text ) {
            TCanvasItem* item = (TCanvasItem*)list[i];
            TextItem* mytext = (TextItem*)item->item();
            QString text = mytext->text();
            bool nocheck = false;
//            for( int z = 0; z < comboText->count(); z++ )
//                if( text == "[" + comboText->text(z) + "]" ) {
//                    nocheck = true;
//                    break;
//                }

            if( !nocheck ) {
                QString textbefore = text;
                Sonnet::Speller speller( "en_US" );
                Sonnet::BackgroundChecker spellChecker( speller, this );
                spellChecker.setText( text );
                //spellChecker.start();
                if( spellChecker.text() != textbefore ) {
                    new TextChangeCommand( mytext, text, sc );
                    executeTextChangeCommand = true;
                }
            }
        }
    }

    if( executeTextChangeCommand ) {
        history->push( sc );
    } else {
        delete sc;
    }*/
    
    qDebug() << "LabelEditor::spellCheck 1";
    //if ( !sonnetDialogExists )
    /*if ( !m_sonnetDialog )
    {
        qDebug() << "LabelEditor::spellCheck !m_sonnetDialog";
        sonnetDialogExists = true;
        showSonnetDialog = true;
        m_sonnetDialog = new Sonnet::Dialog( new Sonnet::BackgroundChecker( this ), this );
        //connect signals to slots:
        //connect( m_sonnetDialog, SIGNAL(misspelling(const QString&,int)), this, SLOT(spellCheckShow(const QString&,int)) );
        connect( m_sonnetDialog, SIGNAL(done(const QString&)), this, SLOT(spellcheckDone(const QString&)) );
        connect( m_sonnetDialog, SIGNAL(replace(const QString&,int,const QString &)), this, SLOT(replaceWord(const QString&,int,const QString&)) );;
        m_sonnetDialog->setSpellCheckContinuedAfterReplacement( true );
    }*/
    if( m_sonnetDialog )
    {
        qDebug() << "LabelEditor::spellCheck if( m_sonnetDialog )";
        delete m_sonnetDialog;
        m_sonnetDialog = NULL;
    }
    qDebug() << "LabelEditor::spellCheck 1 b";
    //sonnetDialogExists = true;
    //showSonnetDialog = true;
    wordWasReplaced = false;
    m_sonnetDialog = new Sonnet::Dialog( new Sonnet::BackgroundChecker( this ), this );
    //connect signals to slots:
    //connect( m_sonnetDialog, SIGNAL(misspelling(const QString&,int)), this, SLOT(spellCheckShow(const QString&,int)) );
    connect( m_sonnetDialog, SIGNAL(done(const QString&)), this, SLOT(spellcheckDone(const QString&)) );
    connect( m_sonnetDialog, SIGNAL(replace(const QString&,int,const QString &)), this, SLOT(replaceWord(const QString&,int,const QString&)) );
    connect( m_sonnetDialog, SIGNAL(buttonClicked(KDialog::ButtonCode)), this, SLOT(spellCheckButtonPressed(KDialog::ButtonCode)) );
    m_sonnetDialog->setSpellCheckContinuedAfterReplacement( true );
    qDebug() << "LabelEditor::spellCheck 2";
    
    if( spellCheckedItems ) {
        qDebug() << "LabelEditor::spellCheck if( spellCheckedItems )";
        delete spellCheckedItems;
        spellCheckedItems = NULL;
    }
    qDebug() << "LabelEditor::spellCheck 3";
    //QList<QGraphicsItem *> list = c->items();
    TCanvasItemList list = cv->getSelected();
    spellCheckedItems = new TCanvasItemList;
    for( int i = 0; i < list.count(); i++ ) {
        if( list[i]->rtti() == eRtti_Text ) {
            spellCheckedItems->append( list[i] );
        }
    }
    qDebug() << "LabelEditor::spellCheck 3 b";
    spellCheckedItemNumber = 0;
    positionInSpellCheckedText = -1;
    //doFindNextWord = false;
    if( correctedTexts ) {
        qDebug() << "LabelEditor::spellCheck if( correctedTexts )";
        delete correctedTexts;
        correctedTexts = NULL;
    }
    correctedTexts = new QList<QString>;
    qDebug() << "LabelEditor::spellCheck 4";
    
    m_sonnetDialog->setBuffer( QString() );
    qDebug() << "LabelEditor::spellCheck 5";
    //spellcheckDone( QString() );
    
    //if (sonnetDialogExists) {
    /*if( showSonnetDialog ) {
        m_sonnetDialog->show();
    }*/
    m_sonnetDialog->show();
    /*if( m_sonnetDialog && m_sonnetDialog->isHidden() ) {
        qDebug() << "LabelEditor::spellCheck m_sonnetDialog->isHidden()";
        m_sonnetDialog->show();
    }*/
    qDebug() << "LabelEditor::spellCheck 6";
}

void LabelEditor::spellcheckDone( const QString & newText )
{
    qDebug() << "LabelEditor::spellcheckDone 1";
    if( positionInSpellCheckedText != -1 ) {
        QString word = findNextWord();
        if( !word.isEmpty() ) {
            //showSonnetDialog = true;
            m_sonnetDialog->setBuffer( word );
            return;
        }
    }
    qDebug() << "LabelEditor::spellcheckDone 2";
    
    if( spellCheckedItems ) {
        qDebug() << "LabelEditor::spellcheckDone 4";
        for( ; spellCheckedItemNumber < spellCheckedItems->count(); spellCheckedItemNumber++ ) {
            setupSpellCheckedText( (*spellCheckedItems)[spellCheckedItemNumber] );
            return;
        }
    }
    applySpellCheckCorrection();
    spellCheckFinished();
}

void LabelEditor::setupSpellCheckedText( const TCanvasItem* item )
{
    qDebug() << "LabelEditor::setupSpellCheckedText -----";
    spellCheckedItemNumber++;
    TextItem* myTextItem = (TextItem*)item->item();
    spellCheckedText = myTextItem->text();
    correctedTexts->append( spellCheckedText );
    positionInSpellCheckedText = 0;
    positionInCorrectedText = 0;
    currentTextFragmentEndIndex = 0;
    findNextTextFragment = true;
    
    QRegExp regBody("<body[^>]*>");
    int indexBody = regBody.indexIn( spellCheckedText );
    if( indexBody != -1 ) {
        positionInSpellCheckedText = indexBody + regBody.matchedLength();
        positionInCorrectedText = indexBody + regBody.matchedLength();
        currentTextFragmentEndIndex = indexBody + regBody.matchedLength();
    }
    spellcheckDone( QString() );
}

void LabelEditor::replaceWord(const QString & oldWord, int start, const QString & newWord)
{
    (*correctedTexts)[correctedTexts->count() - 1].replace( 
        positionInCorrectedText - spellCheckedWordLength, oldWord.length(), newWord );
    positionInCorrectedText += newWord.length() - oldWord.length();
    wordWasReplaced = true;
}

QString LabelEditor::findNextWord()
{
    int positionInSpellCheckedTextTmp = 0;
    if( findNextTextFragment ) {
        findNextTextFragment = false;
        QRegExp reg(">[^<]+<");
        reg.setMinimal( true );
        positionInSpellCheckedTextTmp = reg.indexIn( spellCheckedText, positionInSpellCheckedText );
        positionInSpellCheckedText = positionInSpellCheckedTextTmp + 1;
        positionInCorrectedText += positionInSpellCheckedText - currentTextFragmentEndIndex;
        currentTextFragmentEndIndex = positionInSpellCheckedText + reg.matchedLength() - 2;
    }
    if( positionInSpellCheckedTextTmp == -1 ) {
        return QString();
    }
    QRegExp regText("\\w+");
    int indexText = regText.indexIn( spellCheckedText.mid( positionInSpellCheckedText, currentTextFragmentEndIndex - positionInSpellCheckedText ) );
    QString word;
    if( indexText == -1 ) {
        positionInCorrectedText += currentTextFragmentEndIndex - positionInSpellCheckedText;
        findNextTextFragment = true;
        word = findNextWord();
    } else {
        positionInCorrectedText += indexText;
        positionInSpellCheckedText += indexText;
        word = spellCheckedText.mid( positionInSpellCheckedText, regText.matchedLength() );
        positionInSpellCheckedText += regText.matchedLength();
        positionInCorrectedText += regText.matchedLength();
        spellCheckedWordLength = regText.matchedLength();
    }
    return word;
}

void LabelEditor::applySpellCheckCorrection()
{
    qDebug() << "LabelEditor::applySpellCheckCorrection 1";
    QUndoCommand* sc = new QUndoCommand( i18n("Spellchecking") );
    bool executeTextChangeCommand = false;
    if( wordWasReplaced ) {
        qDebug() << "LabelEditor::applySpellCheckCorrection if( wordWasReplaced )";
        wordWasReplaced = false;
        for( int i = 0; i < spellCheckedItems->count(); i++ ) {
            if( ((TextItem*)(*spellCheckedItems)[i]->item())->text() != (*correctedTexts)[i] ) {
                new TextChangeCommand( 
                    (TextItem*)(*spellCheckedItems)[i]->item(), 
                    (*correctedTexts)[i], sc );
                executeTextChangeCommand = true;
            }
        }
    }
    //if( sc->childCount() > 0 ) {
    if( executeTextChangeCommand ) {// Push the macro command only if there are child TextChangeCommand commands
        history->push( sc );
    } else {
        delete sc;
    }
}

void LabelEditor::spellCheckFinished()
{
    qDebug() << "LabelEditor::spellCheckFinished 1";
    if( spellCheckedItems ) {
        qDebug() << "LabelEditor::spellCheckFinished 2";
        //spellCheckedItems->clear();
        delete spellCheckedItems;
        spellCheckedItems = NULL;
    }
    if( correctedTexts ) {
        qDebug() << "LabelEditor::spellCheckFinished 2 b";
        //correctedTexts->clear();
        delete correctedTexts;
        correctedTexts = NULL;
    }
    qDebug() << "LabelEditor::spellCheckFinished 3";
    //sonnetDialogExists = false;
    //showSonnetDialog = false;
    spellCheckedItemNumber = 0;
    positionInSpellCheckedText = -1;
    qDebug() << "LabelEditor::spellCheckFinished 4";
}

void LabelEditor::spellCheckButtonPressed( KDialog::ButtonCode buttonNumber )
{
    qDebug() << "LabelEditor::spellCheckButtonPressed 1";
    if( buttonNumber == KDialog::User1 ) {
        qDebug() << "LabelEditor::spellCheckButtonPressed 2";
        applySpellCheckCorrection();
        spellCheckFinished();
    } else if( buttonNumber == KDialog::Cancel ) {
        qDebug() << "LabelEditor::spellCheckButtonPressed 3";
        spellCheckFinished();
    }
}

void LabelEditor::centerHorizontal()
{
    if( !cv->getActive() )
        return;

    TCanvasItem* item = cv->getActive();
    
    MoveCommand* mv = new MoveCommand( int( ((d->getMeasurements().widthMM() * 1000.0 - item->item()->rectMM().width())/2 )) - item->item()->rectMM().x(), 0, item );
    history->push( mv );
}

void LabelEditor::centerVertical()
{
    if( !cv->getActive() )
        return;

    TCanvasItem* item = cv->getActive();

    MoveCommand* mv = new MoveCommand( 0, int( ((d->getMeasurements().heightMM() * 1000.0 - item->item()->rectMM().height())/2 ) - item->item()->rectMM().y() ), item );
    history->push( mv );
}

void LabelEditor::raiseCurrent()
{
    if( !cv->getActive() )
        return;

    ChangeZCommand* czc = new ChangeZCommand( (int)cv->getActive()->zValue() + 1, cv->getActive() );
    history->push( czc );
}

void LabelEditor::lowerCurrent()
{
    if( !cv->getActive() )
        return;

    ChangeZCommand* czc = new ChangeZCommand( (int)cv->getActive()->zValue() - 1, cv->getActive() );
    history->push( czc );
}

void LabelEditor::onTopCurrent()
{
    if( !cv->getActive() )
        return;

    int z = 0;

    QList<QGraphicsItem *> list = c->items();
    for( int i = 0; i < list.count(); i++ )
        if( list[i]->zValue() > z )
            z = (int)list[i]->zValue();


    ChangeZCommand* czc = new ChangeZCommand( z + 1, cv->getActive() );
    history->push( czc );
}

void LabelEditor::backCurrent()
{
    if( !cv->getActive() )
        return;

    int z = 0;

    QList<QGraphicsItem *> list = c->items();
    for( int i = 0; i < list.count(); i++ )
        if( list[i]->zValue() < z )
            z = (int)list[i]->zValue();

    ChangeZCommand* czc = new ChangeZCommand( z - 1, cv->getActive() );
    history->push( czc );
}

const QString LabelEditor::fileName() const
{
    return filename.right( filename.length() - filename.lastIndexOf( "/" ) - 1 );
}

void LabelEditor::preview()
{
    QBuffer buffer;
    if( !buffer.open( QIODevice::WriteOnly ) )
        return;

    save( &buffer );

    // No need to delete pd as it has Qt::WA_DeleteOnClose (earlier known as Qt::WDestructiveClose) set!
    PreviewDialog* pd = new PreviewDialog( &buffer, d, fileName(), this );
    pd->exec();
}

void LabelEditor::toggleGrid()
{
    c->setGrid( gridAct->isChecked() );
    c->update();
    cv->repaint();
}

void LabelEditor::cut()
{
    copy();
    cv->deleteCurrent();
}

void LabelEditor::copy()
{
    TCanvasItemList list = cv->getSelected();
    if( list.isEmpty() )
        return;

    DocumentItemList items;
    for( int i=0;i<list.count();i++)
        items.append( (list[i])->item() );

    DocumentItemDrag* drag = new DocumentItemDrag();
    drag->setDocumentItem( &items );
#if QT_VERSION >= 0x040000
    kapp->clipboard()->setMimeData( drag, QClipboard::Clipboard );
#elif QT_VERSION >= 0x030100
    kapp->clipboard()->setData( drag, QClipboard::Clipboard );
#else
    kapp->clipboard()->setData( drag );
#endif
}

void LabelEditor::paste()
{
    const QMimeData* data = QApplication::clipboard()->mimeData();
    QMimeData* dataChangeable = (QMimeData*) data;
    if ( DocumentItemDrag::canDecode( dataChangeable ) )
        DocumentItemDrag::decode( dataChangeable, cv, m_token, history );
}

void LabelEditor::startEditor()
{
    if( isChanged() ) {
        LabelEditor* lb = new LabelEditor( NULL, QString::null );
        lb->startupDlg( eCreateNewLabel, QString::null );
    } else
        newLabel();
}

void LabelEditor::startBarcodeGen()
{
    new BarcodeGenerator();
}

void LabelEditor::startLoadRecentEditor( const KUrl& url )
{
    if( !QFile::exists( url.path() ) ) {
        KMessageBox::information( this, i18n("The file %1 does not exist.").arg( url.path() ) );
        recentAct->removeUrl( url );
        return;
    }

    if( isChanged() )
        new LabelEditor( 0, url.path() );
    else
        openUrl( url.path() );
}

void LabelEditor::startLoadEditor()
{
    if( isChanged() ) {
        LabelEditor* lb = new LabelEditor( 0, QString::null );
        lb->startupDlg( eLoadLabel, QString::null );
    } else
        open();
}

void LabelEditor::batchPrint()
{
    new BatchAssistant( NULL );
}

bool LabelEditor::queryClose()
{
    /* This method returns true if the label editor window can be closed safely false otherwise.
     * It is called by KDE automatically if a user or the program wants to close the label editor window.
     */
    if( !isChanged() ) {
        saveConfig();
        return true;
    }

    int m = KMessageBox::warningYesNoCancel( this,
        i18n("<qt>The document has been modified.<br><br>Do you want to save it ?</qt>") );

    if( m == KMessageBox::Cancel ) {
        return false;
    } else if( m == KMessageBox::No ) {
        saveConfig();
        return true;
    } else if( m == KMessageBox::Yes ) {
        if( save() ) {
            saveConfig();
            return true;
        } else {
            return false;
        }
    }
    
    saveConfig();
    return true;
}

bool LabelEditor::isChanged()
{
    if( !c->width() && !c->height() )
        return false;

    if( m_edited )
        return true;

    return false;
}

bool LabelEditor::startupDlg( ELabelEditorMode mode, QString f )
{
    if( mode == eCreateNewLabel && KBarcodeSettings::getInstance()->newDialog() ) 
    {
        if(!newLabel()) {
            close();
            return false;
        }
    } 
    else if( mode == eLoadLabel ) 
    {
        if(!openUrl(f)) {
            close();
            return false;
        }
    }

    return true;
}

void LabelEditor::closeLabel()
{
    delete d;
    d = new Definition();

    m_edited = false;

    clearLabel();
    enableActions();

    cv->setDefinition( d );

    filename = QString::null;
    setCaption( filename, false );
}

void LabelEditor::setEdited( bool isInCleanState )
{
    /*setCaption( filename, true );
    m_edited = true;*/
    setCaption( filename, !isInCleanState );
    m_edited = !isInCleanState;
    
    /*QAction * undoAct2 = actionCollection()->action("edit_undo");
    QAction * redoAct2 = actionCollection()->action("edit_redo");*/// -!F: delete
    /*if( history->isRedoAvailable() ) {
        redoAct->setEnabled( true );
    } else {
        redoAct->setEnabled( false );
    }
    if( history->isUndoAvailable() ) {
        undoAct->setEnabled( true );
    } else {
        undoAct->setEnabled( false );
    }*/
    
    enableActions();
}

void LabelEditor::enableActions()
{
    editPropAct->setEnabled( cv->getActive() );
    deleteAct->setEnabled( cv->getActive() );

    if( d->getId() == -1 ){
        // label closed
        deleteAct->setEnabled( false );
        barcodeAct->setEnabled( false );
        pictureAct->setEnabled( false );
        textAct->setEnabled( false );
        textDataAct->setEnabled( false );
        textLineAct->setEnabled( false );
        rectAct->setEnabled( false );
        circleAct->setEnabled( false );
        lineAct->setEnabled( false );
        spellAct->setEnabled( false );
        gridAct->setEnabled( false );

        saveAct->setEnabled( false );
        saveAsAct->setEnabled( false );
        printAct->setEnabled( false );
        bcpAct->setEnabled( false );
        imgAct->setEnabled( false );

        previewAct->setEnabled( false );
        closeLabelAct->setEnabled( false );
        descriptionAct->setEnabled( false );

        cutAct->setEnabled( false );
        copyAct->setEnabled( false );
        pasteAct->setEnabled( false );
        
        selectAllAct->setEnabled( false );
        deSelectAllAct->setEnabled( false );
    } else {
        deleteAct->setEnabled( true );
        barcodeAct->setEnabled( Barkode::haveBarcode() );
        pictureAct->setEnabled( true );
        textAct->setEnabled( true );
        textDataAct->setEnabled( true );
        textLineAct->setEnabled( true );
        rectAct->setEnabled( true );
        circleAct->setEnabled( true );
        lineAct->setEnabled( true );
        spellAct->setEnabled( true );
        gridAct->setEnabled( true );

        saveAct->setEnabled( true );
        saveAsAct->setEnabled( true );
        printAct->setEnabled( true );
        bcpAct->setEnabled( true );
        imgAct->setEnabled( true );
        descriptionAct->setEnabled( true );

        previewAct->setEnabled( true );
        closeLabelAct->setEnabled( true );

        cutAct->setEnabled( true );
        copyAct->setEnabled( true );
        pasteAct->setEnabled( true );
        
        selectAllAct->setEnabled( true );
        deSelectAllAct->setEnabled( true );
    }
}

void LabelEditor::launchAddressBook()
{
    KRun::runCommand( "kaddressbook", this );
}

#include "labeleditor.moc"
