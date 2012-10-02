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
#include <qprinter.h>
#include <QPaintDevice>
#include <QDesktopWidget>
#include <QAction>
#include <QGraphicsItem>
#include <QUndoCommand>
#include <QTextDocument>

// KDE includes
#include <kaction.h>
#include <kapplication.h>
#include <kcolordialog.h>
#include <kundostack.h>
#include <kcombobox.h>
#include <kfiledialog.h>
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

#define CANVAS_UPDATE_PERIOD 50

#define KBARCODE_UNDO_LIMIT 25

LabelEditor::LabelEditor( QWidget *parent, QString _filename, Qt::WindowFlags f, Qt::WidgetAttribute waf )
    : MainWindow( parent, f )
{
    setAttribute( waf );
    
    
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
    m_token = new TokenProvider( KApplication::desktop() );

    statusBar()->insertPermanentItem( "", STATUS_ID_TEMPLATE, 0 );
    statusBar()->insertPermanentItem( "", STATUS_ID_SIZE, 0 );
    statusBar()->insertPermanentItem( "", STATUS_ID_MOUSE, 2 );
    statusBar()->setSizeGripEnabled( true );
    statusBar()->show();

    c = new MyCanvas( this );

    cv = new MyCanvasView( d, c, this );
    cv->setPosLabel( statusBar(), STATUS_ID_MOUSE );
    setCentralWidget( cv );

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
    history = new KUndoStack( this );
    cv->setHistory( history );

    // Don't set the undo limit anymore becouse in KDE4 it causes deletion ot items if the limit is reached:
    //history->setUndoLimit( KBARCODE_UNDO_LIMIT );
    
    connect( history, SIGNAL(cleanChanged(bool)), this, SLOT(setEdited(bool)) );
}

void LabelEditor::createCommandHistoryActions()
{
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
    KStandardAction::openNew( this, SLOT(startEditor()), actionCollection() );
    KStandardAction::open( this, SLOT(startLoadEditor()), actionCollection() );
    KStandardAction::quit(kapp, SLOT(quit()), actionCollection());
    KStandardAction::close( this, SLOT( close() ), actionCollection() );
    closeLabelAct = new KAction( this );
    closeLabelAct->setText( i18n("Close &Label") );
    actionCollection()->addAction( "closeLabelAct", closeLabelAct );
    connect( closeLabelAct, SIGNAL(triggered(bool)), this, SLOT(closeLabel()) );

    recentAct = new KRecentFilesAction( this );
    recentAct->setText( i18n("&Recent Files") );
    actionCollection()->addAction( "recentAct", recentAct );
    connect( recentAct, SIGNAL( urlSelected( const KUrl& ) ), this, SLOT( startLoadRecentEditor( const KUrl& ) ) );

    KAction* importPrintFileAct = new KAction( this );
    importPrintFileAct->setText( i18n("&Import and Print Batch File...") );
    importPrintFileAct->setIcon( KIcon( "document-print" ) );
    actionCollection()->addAction( "importPrintFileAct", importPrintFileAct );
    connect( importPrintFileAct, SIGNAL(triggered(bool)), this, SLOT(batchPrint()) );

    saveAct = KStandardAction::save( this, SLOT( save() ), actionCollection() );
    saveAsAct = KStandardAction::saveAs( this, SLOT( saveas() ), actionCollection() );
    descriptionAct = new KAction( this );
    descriptionAct->setText( i18n("&Change description...") );
    actionCollection()->addAction( "descriptionAct", descriptionAct );
    connect( descriptionAct, SIGNAL(triggered(bool)), this, SLOT(changeDes()) );
    deleteAct = new KAction( this );
    deleteAct->setText( i18n("&Delete Object") );
    deleteAct->setIcon( KIcon( "edit-delete" ) );
    deleteAct->setShortcut( Qt::Key_Delete );
    actionCollection()->addAction( "deleteAct", deleteAct );
    connect( deleteAct, SIGNAL(triggered(bool)), cv, SLOT(deleteCurrent()) );
    editPropAct = new KAction( this );
    editPropAct->setText( i18n("&Properties...") );
    actionCollection()->addAction( "editPropAct", editPropAct );
    connect( editPropAct, SIGNAL(triggered(bool)), this, SLOT(doubleClickedCurrent()) );
    printAct = KStandardAction::print( this, SLOT( print() ), actionCollection() );
    bcpAct = new KAction( this );
    bcpAct->setText( i18n("Print to &Barcode Printer...") );
    actionCollection()->addAction( "bcpAct", bcpAct );
    connect( bcpAct, SIGNAL(triggered(bool)), this, SLOT(printBCP()) );
    imgAct = new KAction( this );
    imgAct->setText( i18n("Print to &Image...") );
    actionCollection()->addAction( "imgAct", imgAct );
    connect( imgAct, SIGNAL(triggered(bool)), this, SLOT(printImage()) );
    changeSizeAct = new KAction( this );
    changeSizeAct->setText( i18n("&Change Label...") );
    actionCollection()->addAction( "changeSizeAct", changeSizeAct );
    connect( changeSizeAct, SIGNAL(triggered(bool)), this, SLOT(changeSize()) );
    barcodeAct = new KAction( this );
    barcodeAct->setText( i18n("Insert &Barcode") );
    barcodeAct->setIcon( KIcon( "view-barcode-add" ) );
    actionCollection()->addAction( "barcodeAct", barcodeAct );
    connect( barcodeAct, SIGNAL(triggered(bool)), this, SLOT(insertBarcode()) );
    barcodeAct->setEnabled( Barkode::haveBarcode() );

    pictureAct = new KAction( this );
    pictureAct->setText( i18n("Insert &Picture") );
    pictureAct->setIcon( KIcon( "insert-image" ) );
    actionCollection()->addAction( "pictureAct", pictureAct );
    connect( pictureAct, SIGNAL(triggered(bool)), this, SLOT(insertPicture()) );
    textAct = new KAction( this );
    textAct->setText( i18n("Insert &Text") );
    textAct->setIcon( KIcon( "text-field" ) );
    actionCollection()->addAction( "textAct", textAct );
    connect( textAct, SIGNAL(triggered(bool)), this, SLOT(insertText()) );
    textDataAct = new KAction( this );
    textDataAct->setText( i18n("Insert &Data Field") );
    textDataAct->setIcon( KIcon( "view-table-of-contents-ltr" ) );
    actionCollection()->addAction( "textDataAct", textDataAct );
    connect( textDataAct, SIGNAL(triggered(bool)), this, SLOT(insertDataText()) );
    textLineAct = new KAction( this );
    textLineAct->setText( i18n("Insert &Text Line") );
    textLineAct->setIcon( KIcon( "insert-text" ) );
    actionCollection()->addAction( "textLineAct", textLineAct );
    connect( textLineAct, SIGNAL(triggered(bool)), this, SLOT(insertTextLine()) );
    lineAct = new KAction( this );
    lineAct->setText( i18n("Insert &Line") );
    lineAct->setIcon( KIcon( "kbarcodelinetool" ) );
    actionCollection()->addAction( "lineAct", lineAct );
    connect( lineAct, SIGNAL(triggered(bool)), this, SLOT(insertLine()) );
    rectAct = new KAction( this );
    rectAct->setText( i18n("Insert &Rectangle") );
    rectAct->setIcon( KIcon( "kbarcoderect" ) );
    actionCollection()->addAction( "rectAct", rectAct );
    connect( rectAct, SIGNAL(triggered(bool)), this, SLOT(insertRect()) );
    circleAct = new KAction( this );
    circleAct->setText( i18n("Insert &Ellipse") );
    circleAct->setIcon( KIcon( "kbarcodeellipse" ) );
    actionCollection()->addAction( "circleAct", circleAct );
    connect( circleAct, SIGNAL(triggered(bool)), this, SLOT(insertCircle()) );
    spellAct = KStandardAction::spelling( this, SLOT(spellCheck()), actionCollection() );
    gridAct = new KToggleAction( KIcon( "kbarcodegrid" ), i18n("&Grid"), this );
    actionCollection()->addAction( "gridAct", gridAct );
    connect( gridAct, SIGNAL(triggered(bool)), this, SLOT(toggleGrid()) );
    previewAct = new KAction( this );
    previewAct->setText( i18n("&Preview...") );
    actionCollection()->addAction( "previewAct", previewAct );
    connect( previewAct, SIGNAL(triggered(bool)), this, SLOT(preview()) );
    cutAct = KStandardAction::cut( this, SLOT( cut() ), actionCollection() );
    copyAct = KStandardAction::copy( this, SLOT( copy() ), actionCollection() );
    pasteAct = KStandardAction::paste( this, SLOT( paste() ), actionCollection() );
    selectAllAct = KStandardAction::selectAll( cv, SLOT( selectAll() ), actionCollection() );
    deSelectAllAct = KStandardAction::deselect( cv, SLOT( deSelectAll() ), actionCollection() );
    addressBookAct = new KAction( this );
    addressBookAct->setText( i18n("Address&book") );
    addressBookAct->setIcon( KIcon( "kaddressbook" ) );
    actionCollection()->addAction( "addressBookAct", addressBookAct );
    connect( addressBookAct, SIGNAL(triggered(bool)), this, SLOT(launchAddressBook()) );
    KAction* singleBarcodeAct = new KAction( this );
    singleBarcodeAct->setText( i18n("&Create Single Barcode...") );
    actionCollection()->addAction( "create", singleBarcodeAct );
    connect( singleBarcodeAct, SIGNAL(triggered(bool)), this, SLOT(startBarcodeGen()) );
    singleBarcodeAct->setEnabled( Barkode::haveBarcode() );
    
    setupGUI( Default, "labeleditorui.rc" );
    
    createCustomHelpMenu();

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
    protectAction = m_mnuContext->addAction( i18n("&Protect Position and Size"), this, SLOT( lockItem() ) );
    protectAction->setCheckable( true );
    m_mnuContext->addSeparator();
    m_mnuContext->addAction( i18n("&Properties"), this, SLOT( doubleClickedCurrent() ) );
}

void LabelEditor::insertBarcode()
{
    NewBarcodeCommand* bc = new NewBarcodeCommand( cv, m_token );
    history->push( bc );
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
    QString tmp = QInputDialog::getText( this, i18n("Label Description"),
            i18n("Please enter a description:"), QLineEdit::Normal, description );
    if( !tmp.isEmpty() )
        description = tmp;
}

void LabelEditor::changeSize()
{
    NewLabel* nl = new NewLabel( this, true, Qt::Window );
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
    if( m_sonnetDialog )
    {
        delete m_sonnetDialog;
        m_sonnetDialog = NULL;
    }
    wordWasReplaced = false;
    m_sonnetDialog = new Sonnet::Dialog( new Sonnet::BackgroundChecker( this ), this );
    connect( m_sonnetDialog, SIGNAL(done(const QString&)), this, SLOT(spellcheckDone(const QString&)) );
    connect( m_sonnetDialog, SIGNAL(replace(const QString&,int,const QString &)), this, SLOT(replaceWord(const QString&,int,const QString&)) );
    connect( m_sonnetDialog, SIGNAL(buttonClicked(KDialog::ButtonCode)), this, SLOT(spellCheckButtonPressed(KDialog::ButtonCode)) );
    m_sonnetDialog->setSpellCheckContinuedAfterReplacement( true );
    
    if( spellCheckedItems ) {
        delete spellCheckedItems;
        spellCheckedItems = NULL;
    }
    //QList<QGraphicsItem *> list = c->items();
    TCanvasItemList list = cv->getSelected();
    spellCheckedItems = new TCanvasItemList;
    for( int i = 0; i < list.count(); i++ ) {
        if( list[i]->rtti() == eRtti_Text ) {
            spellCheckedItems->append( list[i] );
        }
    }
    spellCheckedItemNumber = 0;
    positionInSpellCheckedText = -1;
    if( correctedTexts ) {
        delete correctedTexts;
        correctedTexts = NULL;
    }
    correctedTexts = new QList<QString>;
    
    m_sonnetDialog->setBuffer( QString() );
    
    m_sonnetDialog->show();
}

void LabelEditor::spellcheckDone( const QString & newText )
{
    if( positionInSpellCheckedText != -1 ) {
        QString word = findNextWord();
        if( !word.isEmpty() ) {
            m_sonnetDialog->setBuffer( word );
            return;
        }
    }
    
    if( spellCheckedItems ) {
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
    QUndoCommand* sc = new QUndoCommand( i18n("Spellchecking") );
    bool executeTextChangeCommand = false;
    if( wordWasReplaced ) {
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
    if( spellCheckedItems ) {
        //spellCheckedItems->clear();
        delete spellCheckedItems;
        spellCheckedItems = NULL;
    }
    if( correctedTexts ) {
        //correctedTexts->clear();
        delete correctedTexts;
        correctedTexts = NULL;
    }
    spellCheckedItemNumber = 0;
    positionInSpellCheckedText = -1;
}

void LabelEditor::spellCheckButtonPressed( KDialog::ButtonCode buttonNumber )
{
    if( buttonNumber == KDialog::User1 ) {
        applySpellCheckCorrection();
        spellCheckFinished();
    } else if( buttonNumber == KDialog::Cancel ) {
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
    setCaption( filename, !isInCleanState );
    m_edited = !isInCleanState;
    
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
