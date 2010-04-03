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
#include <q3canvas.h>
#include <qcheckbox.h>
#include <qclipboard.h>
#include <q3dockarea.h>
#include <qdom.h>
#include <q3dragobject.h>
#include <q3groupbox.h>
#include <qimage.h>
#include <qinputdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmap.h>
#include <qmime.h>
#include <qpainter.h>
#include <q3picture.h>
#include <qpoint.h>
#include <q3progressdialog.h>
#include <qsqlquery.h>
#include <q3textbrowser.h>
#include <qtooltip.h>
#include <qvalidator.h>
#include <QtXml>
#include <qregexp.h>
#include <QList>
#include <QByteArray>
#include <QCloseEvent>
#include <qprinter.h>

// KDE includes
#include <kabc/stdaddressbook.h>
#include <kaction.h>
#include <kapplication.h>
#include <kcolordialog.h>
#include <k3command.h>
#include <kcombobox.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kimageio.h>
#include <klineedit.h>
#include <k3listbox.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <knuminput.h>
#include <kmenu.h>
#include <kpushbutton.h>
#include <krun.h>
#include <kspell.h>
#include <kstatusbar.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kglobal.h>

#include "tcanvasitem.h"
#include "rectitem.h"
#include "textitem.h"
#include "imageitem.h"
#include "barcodeitem.h"
#include "lineitem.h"

#define STATUS_ID_SIZE 100
#define STATUS_ID_TEMPLATE 101
#define STATUS_ID_MOUSE 102

#define ID_LOCK_ITEM 8000

#define CANVAS_UPDATE_PERIOD 50

#define KBARCODE_UNDO_LIMIT 25

using namespace KABC;

LabelEditor::LabelEditor( QWidget *parent, QString _filename, Qt::WFlags f )
    : MainWindow( parent, f )
{
    undoAct = 
	redoAct = NULL; 
    history = NULL;

    description = QString::null;
    d = new Definition();
    m_token = new TokenProvider( KApplication::desktop() );

    statusBar()->insertItem( "", STATUS_ID_TEMPLATE, 0, true );
    statusBar()->insertItem( "", STATUS_ID_SIZE, 0, true );
    statusBar()->insertItem( "", STATUS_ID_MOUSE, 2, true );
    statusBar()->setSizeGripEnabled( true );
    statusBar()->show();

    c = new MyCanvas( this );
    c->setDoubleBuffering( true );
    c->setUpdatePeriod( CANVAS_UPDATE_PERIOD );

    cv = new MyCanvasView( d, c, this );
    cv->setPosLabel( statusBar(), STATUS_ID_MOUSE );
    setCentralWidget( cv );

    setupActions();
    setupContextMenu();
    setAutoSaveSettings( QString("Window") + name, true );

    clearLabel();

    loadConfig();
    show();

//    if( isFirstStart() )
//        moveDockWindow( tools, Qt::DockLeft );

    connect( cv, SIGNAL( doubleClickedItem(TCanvasItem*) ), this, SLOT( doubleClickedItem(TCanvasItem*) ) );
    connect( cv, SIGNAL( showContextMenu(QPoint) ), this, SLOT( showContextMenu(QPoint) ) );
    connect( cv, SIGNAL( movedSomething() ), this, SLOT( setEdited() ) );
    connect( KBarcodeSettings::getInstance(), SIGNAL( updateGrid( int ) ), cv, SLOT( updateGUI() ) );
    connect( kapp, SIGNAL( aboutToQuit() ), this, SLOT( saveConfig() ) );
 
    connect( history, SIGNAL( commandExecuted() ), cv, SLOT( updateGUI() ) );
    connect( history, SIGNAL( commandExecuted() ), this, SLOT( setEdited() ) );

    if( !_filename.isEmpty() )
        openUrl( _filename );
}

LabelEditor::~LabelEditor()
{
    delete m_token;
    delete d;
    delete history;
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
	undoAct->unplug( editMenu );
	undoAct->unplug( toolBar() );
	redoAct->unplug( editMenu );
	redoAct->unplug( toolBar() );
	actionCollection()->remove( undoAct );
	actionCollection()->remove( redoAct );
    }

    history = new KCommandHistory( actionCollection(), false );
    cv->setHistory( history );

    history->setUndoLimit( KBARCODE_UNDO_LIMIT );
    history->setRedoLimit( KBARCODE_UNDO_LIMIT );
}

void LabelEditor::createCommandHistoryActions()
{
    undoAct = (KAction*)actionCollection()->action("edit_undo");
    redoAct = (KAction*)actionCollection()->action("edit_redo");

    undoAct->plug( editMenu, 0 );
    redoAct->plug( editMenu, 1 );

    undoAct->plug( toolBar(), 5 );
    redoAct->plug( toolBar(), 6 );
}

void LabelEditor::clearLabel()
{
    TCanvasItem* citem;
    QCanvasItemList::Iterator it;

    description = QString::null;

    delete history;
    createCommandHistory();
    createCommandHistoryActions();

    connect( history, SIGNAL( commandExecuted() ), cv, SLOT( updateGUI() ) );
    connect( history, SIGNAL( commandExecuted() ), this, SLOT( setEdited() ) );

    m_edited = false;

    QCanvasItemList list = c->allItems();
    it = list.begin();
    for (; it != list.end(); ++it)
    {
	citem = static_cast<TCanvasItem*>(*it);
	citem->remRef();
    }

    updateInfo();
    c->update();
    cv->repaintContents();
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
    QString name = KFileDialog::getSaveFileName ( NULL, "*.kbarcode", this );
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

    m_token->setLabelName( filename.right( filename.length() - filename.findRev( "/" ) - 1 ) );
    // maybe we should redraw all items on the canvas now.
    // if there is a label with [filename], the filename might not
    // get updated if the label gets saved with another filename.

    filename = name;
    history->documentSaved();
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

    QCanvasItemList list = c->allItems();
    for( unsigned int i = 0; i < list.count(); i++ )
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
    QString name = KFileDialog::getOpenFileName ( NULL, "*.kbarcode", this, i18n("Select Label") );
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
    m_token->setLabelName( filename.right( filename.length() - filename.findRev( "/" ) - 1 ) );

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
    for( unsigned int i=0;i<list.count();i++ )
    {
        TCanvasItem* citem = new TCanvasItem( cv );
        citem->setItem( list.at( i ) );
	citem->addRef();
    }
    list.clear();

    KUrl murl;
    murl.setPath( filename );
    recentAct->addUrl( murl );

    enableActions();
    cv->repaintContents( true );

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
    m_token->setLabelName( filename.right( filename.length() - filename.findRev( "/" ) - 1 ) );
    setCaption( filename, false );
    enableActions();

    return true;
}

void LabelEditor::setupActions()
{
    KAction* newAct = KStandardAction::openNew( this, SLOT(startEditor()), actionCollection() );
    KAction* loadAct = KStandardAction::open( this, SLOT(startLoadEditor()), actionCollection() );
    KAction* quitAct = KStandardAction::quit(kapp, SLOT(quit()), actionCollection());
    KAction* closeAct = KStandardAction::close( this, SLOT( close() ), actionCollection(), "close" );
    closeLabelAct = new KAction( i18n("Close &Label" ), 0, 0, this, SLOT( closeLabel() ), actionCollection() );

    recentAct = new KRecentFilesAction( i18n("&Recent Files"), 0, this, SLOT( loadRecentEditor( const KUrl& ) ) );

    KAction* importPrintFileAct = new KAction( i18n("&Import and Print Batch File..."), BarIconSet( "fileprint" ), 0, this, SLOT( batchPrint() ), actionCollection() );

    saveAct = KStandardAction::save( this, SLOT( save() ), actionCollection(), "save" );
    saveAsAct = KStandardAction::saveAs( this, SLOT( saveas() ), actionCollection(), "saveas" );
    descriptionAct = new KAction( i18n("&Change description..."), 0, 0, this, SLOT(changeDes()), actionCollection() );
    deleteAct = new KAction( i18n("&Delete Object"), QIcon( BarIcon("editdelete") ), Qt::Key_Delete, cv, SLOT( deleteCurrent() ), actionCollection() );
    editPropAct = new KAction( i18n("&Properties..."), 0, 0, this, SLOT( doubleClickedCurrent() ), actionCollection() );
    printAct = KStandardAction::print( this, SLOT( print() ), actionCollection(), "print" );
    bcpAct = new KAction( i18n("Print to &Barcode Printer..."), 0, 0, this, SLOT( printBCP() ), actionCollection() );
    imgAct = new KAction( i18n("Print to &Image..."), 0, 0, this, SLOT(printImage() ), actionCollection() );
    changeSizeAct = new KAction( i18n("&Change Label..."), 0, 0, this, SLOT( changeSize() ), actionCollection() );
    barcodeAct = new KAction( i18n("Insert &Barcode"), QIcon( BarIcon("barcode") ), 0, this, SLOT( insertBarcode() ), actionCollection() );
    barcodeAct->setEnabled( Barkode::haveBarcode() );

    pictureAct = new KAction( i18n("Insert &Picture"), QIcon( BarIcon("inline_image") ), 0, this, SLOT( insertPicture() ), actionCollection() );
    textAct = new KAction( i18n("Insert &Text"), QIcon( BarIcon("text") ), 0, this, SLOT( insertText() ), actionCollection() );
    textDataAct = new KAction( i18n("Insert &Data Field"), QIcon( BarIcon("contents") ), 0, this, SLOT( insertDataText() ), actionCollection() );
    textLineAct = new KAction( i18n("Insert &Text Line"), QIcon( BarIcon("text") ), 0, this, SLOT( insertTextLine() ), actionCollection() );
    lineAct = new KAction( i18n("Insert &Line"), QIcon( BarIcon("kbarcodelinetool") ), 0, this, SLOT( insertLine() ), actionCollection() );
    rectAct = new KAction( i18n("Insert &Rectangle"), QIcon( BarIcon("kbarcoderect") ), 0, this, SLOT( insertRect() ), actionCollection() );
    circleAct = new KAction( i18n("Insert &Ellipse"), QIcon( BarIcon("kbarcodeellipse") ), 0, this, SLOT( insertCircle() ), actionCollection() );
    spellAct = KStandardAction::spelling( this, SLOT(spellCheck()), actionCollection(), "spell" );
    gridAct = new KToggleAction( i18n("&Grid"), QIcon( BarIcon("kbarcodegrid") ), 0, this, SLOT( toggleGrid() ), actionCollection() );
    previewAct = new KAction( i18n("&Preview..."), 0, 0, this, SLOT( preview() ), actionCollection() );
    sep = new KActionSeparator( this );
    cutAct = KStandardAction::cut( this, SLOT( cut() ), actionCollection(), "cut" );
    copyAct = KStandardAction::copy( this, SLOT( copy() ), actionCollection(), "copy" );
    pasteAct = KStandardAction::paste( this, SLOT( paste() ), actionCollection(), "paste" );
    selectAllAct = KStandardAction::selectAll( cv, SLOT( selectAll() ), actionCollection(), "select_all" );
    deSelectAllAct = KStandardAction::deselect( cv, SLOT( deSelectAll() ), actionCollection(), "de_select_all" );
    addressBookAct = new KAction( i18n("Address&book"), QIcon( BarIcon("kaddressbook") ), 0, this, SLOT( launchAddressBook() ), actionCollection() );
    KAction* singleBarcodeAct = new KAction(i18n("&Create Single Barcode..."), "",
                                0, this, SLOT(startBarcodeGen()),
                                actionCollection(), "create" );
    singleBarcodeAct->setEnabled( Barkode::haveBarcode() );

    newAct->plug( toolBar() );
    loadAct->plug( toolBar() );
    saveAct->plug( toolBar() );
    printAct->plug( toolBar() );
    sep->plug( toolBar() );
    cutAct->plug( toolBar() );
    copyAct->plug( toolBar() );
    pasteAct->plug( toolBar() );

    tools = new KToolBar( this, this->leftDock(), true);

    barcodeAct->plug( tools );
    pictureAct->plug( tools );
    textAct->plug( tools );
    textDataAct->plug( tools );
    textLineAct->plug( tools );
    lineAct->plug( tools );
    rectAct->plug( tools );
    circleAct->plug( tools );
    (new KActionSeparator( this ))->plug( tools );
//    spellAct->plug( tools );  // KDE 3.2
    gridAct->plug( tools );

    MainWindow::setupActions();
    connect( recentAct, SIGNAL( urlSelected( const KUrl& ) ), this, SLOT( startLoadRecentEditor( const KUrl& ) ) );

    KMenu* fileMenu = new KMenu( this );
    editMenu = new KMenu( this );
    KMenu* viewMenu = new KMenu( this );
    KMenu* insMenu = new KMenu( this );
    KMenu* toolMenu = new KMenu( this );
    KMenu* barMenu = new KMenu( this );

    menuBar()->removeItemAt( 0 );
    menuBar()->insertItem( i18n("&File"), fileMenu, -1, 0 );
    menuBar()->insertItem( i18n("&Edit"), editMenu, -1, 1 );
    menuBar()->insertItem( i18n("&Insert"), insMenu, -1, 2 );
    menuBar()->insertItem( i18n("&View"), viewMenu, -1, 3 );
    menuBar()->insertItem( i18n("T&ools"), toolMenu, -1, 4 );
    menuBar()->insertItem( i18n("&Barcode"), barMenu, -1, 5 );

    // Menubar
    newAct->plug( fileMenu );
    loadAct->plug( fileMenu );
    recentAct->plug( fileMenu );
    saveAct->plug( fileMenu );
    saveAsAct->plug( fileMenu );
    sep->plug( fileMenu );
    printAct->plug( fileMenu );
    bcpAct->plug( fileMenu );
    imgAct->plug( fileMenu );
    sep->plug( fileMenu );
    closeLabelAct->plug( fileMenu );
    closeAct->plug( fileMenu );
    quitAct->plug( fileMenu );

    sep->plug( editMenu );
    cutAct->plug( editMenu );
    copyAct->plug( editMenu );
    pasteAct->plug( editMenu );
    sep->plug( editMenu );
    selectAllAct->plug( editMenu );
    deSelectAllAct->plug( editMenu );
    sep->plug( editMenu );
    descriptionAct->plug( editMenu );
    changeSizeAct->plug( editMenu );
    sep->plug( editMenu );
    deleteAct->plug( editMenu );
    editPropAct->plug( editMenu );

    barcodeAct->plug( insMenu );
    pictureAct->plug( insMenu );
    textAct->plug( insMenu );
    textDataAct->plug( insMenu );
    textLineAct->plug( insMenu );
    lineAct->plug( insMenu );
    rectAct->plug( insMenu );
    circleAct->plug( insMenu );

//    spellAct->plug( toolMenu ); // KDE 3.2
    toolMenu->insertSeparator();
    addressBookAct->plug( toolMenu );

    gridAct->plug( viewMenu );
    previewAct->plug( viewMenu );

    singleBarcodeAct->plug( barMenu );
    importPrintFileAct->plug( barMenu );

    enableActions();
}

void LabelEditor::setupContextMenu()
{
    m_mnuContext = new KMenu( this );
    m_mnuContext->setCheckable( true );
    
    KMenu* orderMenu = new KMenu( m_mnuContext );
    orderMenu->insertItem( i18n("&On Top"), this, SLOT( onTopCurrent() ) );
    orderMenu->insertItem( i18n("&Raise"), this, SLOT( raiseCurrent() ) );
    orderMenu->insertItem( i18n("&Lower"), this, SLOT( lowerCurrent() ) );
    orderMenu->insertItem( i18n("&To Background"), this, SLOT( backCurrent() ) );

    KMenu* centerMenu = new KMenu( m_mnuContext );
    centerMenu->insertItem( i18n("Center &Horizontally"), this, SLOT( centerHorizontal() ) );
    centerMenu->insertItem( i18n("Center &Vertically"), this, SLOT( centerVertical() ) );

    m_mnuContext->insertItem( i18n("&Order"), orderMenu );
    m_mnuContext->insertItem( i18n("&Center"), centerMenu );
    m_mnuContext->insertSeparator();
    m_mnuContext->insertItem( SmallIcon("editdelete"), i18n("&Delete"), cv, SLOT( deleteCurrent() ) );
    m_mnuContext->insertItem( i18n("&Protect Position and Size"), this, SLOT( lockItem() ), 0, ID_LOCK_ITEM );
    m_mnuContext->insertSeparator();
    m_mnuContext->insertItem( i18n("&Properties"), this, SLOT( doubleClickedCurrent() ) );
}

void LabelEditor::insertBarcode()
{
    NewBarcodeCommand* bc = new NewBarcodeCommand( cv, m_token );
    bc->execute();

    BarcodeItem* bcode = static_cast<BarcodeItem*>((static_cast<TCanvasItem*>(bc->createdItem()))->item());
    if( !bcode )
        return;

    history->addCommand( bc, false );
}

void LabelEditor::insertPicture()
{
    NewPictureCommand* pc = new NewPictureCommand( cv );
    history->addCommand( pc, true );

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
    history->addCommand( tc, true );
}

//NY30
void LabelEditor::insertTextLine()
{
    insertTextLine( "Some Plain Text" );
}

void LabelEditor::insertTextLine( QString caption )
{
    NewTextLineCommand* tc = new NewTextLineCommand( caption, cv, m_token );
    history->addCommand( tc, true );
}
//NY30

void LabelEditor::insertRect()
{
    NewRectCommand* rc = new NewRectCommand( cv );
    history->addCommand( rc, true );
}

void LabelEditor::insertCircle()
{
    NewRectCommand* rc = new NewRectCommand( cv, true );
    history->addCommand( rc, true );
}

void LabelEditor::insertLine()
{
    NewLineCommand* lc = new NewLineCommand( cv );
    history->addCommand( lc, true );
}

void LabelEditor::changeDes()
{
    QString tmp = QInputDialog::getText( i18n("Label Description"),
            i18n("Please enter a description:"), QLineEdit::Normal, description );
    if( !tmp.isEmpty() )
        description = tmp;
}

void LabelEditor::changeSize()
{
    NewLabel* nl = new NewLabel( this, true, true );
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
        cv->repaintContents();
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
    
    m_mnuContext->setItemChecked( ID_LOCK_ITEM, (list[0])->item()->locked() );
    m_mnuContext->popup( pos );
}

void LabelEditor::lockItem()
{
    TCanvasItemList list = cv->getSelected();
    KMacroCommand* mc = new KMacroCommand( i18n("Protected Item") );
    
    DocumentItem* item = NULL;
    LockCommand* lc = NULL;
    for( unsigned int i=0;i<list.count();i++)
    {
        item = list[i]->item();
        lc = new LockCommand( !item->locked(), list[i] );
        lc->execute();
        mc->addCommand( lc );
    }
    
    history->addCommand( mc );
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
    KFileDialog fd( ":save_image", KImageIO::pattern( KImageIO::Writing ), this,  true );
    fd.setMode( KFile::File );
    fd.setOperationMode( KFileDialog::Saving );
    if( fd.exec() == QDialog::Accepted ) {
        QString path = fd.selectedURL().path();
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
    KMacroCommand* sc = new KMacroCommand( i18n("Spellchecking") );
    QCanvasItemList list = c->allItems();
    for( unsigned int i = 0; i < list.count(); i++ )
        if( list[i]->rtti() == eRtti_Text ) {
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
                KSpell::modalCheck( text );
                if( text != textbefore ) {
                    TextChangeCommand* tc = new TextChangeCommand( mytext, text );
                    tc->execute();
                    sc->addCommand( tc );
                }
            }
        }

    history->addCommand( sc, false );
}

void LabelEditor::centerHorizontal()
{
    if( !cv->getActive() )
        return;

    TCanvasItem* item = cv->getActive();
    
    MoveCommand* mv = new MoveCommand( int( ((d->getMeasurements().widthMM() * 1000.0 - item->item()->rectMM().width())/2 )) - item->item()->rectMM().x(), 0, item );
    history->addCommand( mv, true );
}

void LabelEditor::centerVertical()
{
    if( !cv->getActive() )
        return;

    TCanvasItem* item = cv->getActive();

    MoveCommand* mv = new MoveCommand( 0, int( ((d->getMeasurements().heightMM() * 1000.0 - item->item()->rectMM().height())/2 ) - item->item()->rectMM().y() ), item );
    history->addCommand( mv, true );
}

void LabelEditor::raiseCurrent()
{
    if( !cv->getActive() )
        return;

    ChangeZCommand* czc = new ChangeZCommand( (int)cv->getActive()->z() + 1, cv->getActive() );
    history->addCommand( czc, true );
}

void LabelEditor::lowerCurrent()
{
    if( !cv->getActive() )
        return;

    ChangeZCommand* czc = new ChangeZCommand( (int)cv->getActive()->z() - 1, cv->getActive() );
    history->addCommand( czc, true );
}

void LabelEditor::onTopCurrent()
{
    if( !cv->getActive() )
        return;

    int z = 0;

    QCanvasItemList list = c->allItems();
    for( unsigned int i = 0; i < list.count(); i++ )
        if( list[i]->z() > z )
            z = (int)list[i]->z();


    ChangeZCommand* czc = new ChangeZCommand( z + 1, cv->getActive() );
    history->addCommand( czc, true );
}

void LabelEditor::backCurrent()
{
    if( !cv->getActive() )
        return;

    int z = 0;

    QCanvasItemList list = c->allItems();
    for( unsigned int i = 0; i < list.count(); i++ )
        if( list[i]->z() < z )
            z = (int)list[i]->z();

    ChangeZCommand* czc = new ChangeZCommand( z - 1, cv->getActive() );
    history->addCommand( czc, true );
}

const QString LabelEditor::fileName() const
{
    return filename.right( filename.length() - filename.findRev( "/" ) - 1 );
}

void LabelEditor::preview()
{
    QBuffer buffer;
    if( !buffer.open( QIODevice::WriteOnly ) )
        return;

    save( &buffer );

    // No need to delete pd as it has WDestructiveClose set!
    PreviewDialog* pd = new PreviewDialog( &buffer, d, fileName(), this );
    pd->exec();
}

void LabelEditor::toggleGrid()
{
    c->setGrid( gridAct->isChecked() );
    cv->repaintContents();
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
    for( unsigned int i=0;i<list.count();i++)
        items.append( (list[i])->item() );

    DocumentItemDrag* drag = new DocumentItemDrag();
    drag->setDocumentItem( &items );
#if QT_VERSION >= 0x030100
    kapp->clipboard()->setData( drag, QClipboard::Clipboard );
#else
    kapp->clipboard()->setData( drag );
#endif
}

void LabelEditor::paste()
{
    QMimeSource* data = QApplication::clipboard()->data();
    if ( DocumentItemDrag::canDecode( data ) )
        DocumentItemDrag::decode( data, cv, m_token, history );
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
        recentAct->removeURL( url );
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

void LabelEditor::closeEvent( QCloseEvent* e )
{
    if( !isChanged() ) {
        saveConfig();
        e->accept();
        delete this;
        return;
    }

    int m = KMessageBox::warningYesNoCancel( this,
        i18n("<qt>The document has been modified.<br><br>Do you want to save it ?</qt>") );

    if( m == KMessageBox::Cancel )
        e->ignore();
    else if( m == KMessageBox::No ) {
        saveConfig();
        e->accept();
        delete this;
    } else if( m == KMessageBox::Yes ) {
        if( save() ) {
            saveConfig();
            e->accept();
            delete this;
        }
    }
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

void LabelEditor::setEdited()
{
    setCaption( filename, true );
    m_edited = true;
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
    KRun::runCommand( "kaddressbook" );
}

#include "labeleditor.moc"
