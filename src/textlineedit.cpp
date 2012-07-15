//
// C++ Implementation: textlineedit
//
// Description: 
//
//
// Author: Dominik Seichter <domseichter@web.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "textlineedit.h"
#include "tokendialog.h"

// KDE includes
#include <knuminput.h>
#include <kaction.h>
#include <kdeversion.h>
#include <kcolordialog.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <sonnet/speller.h>
#if QT_VERSION >= 0x030100
    #include <klineedit.h>
#else
    #include <qlineedit.h>
//Added by qt3to4:
#include <QVBoxLayout>
#endif
#include <ktoolbar.h>
#include <kcombobox.h>
#include <kactioncollection.h>
#include <kstandardaction.h>

// Qt includes
#include <q3dockarea.h>
#include <qregexp.h>
#include <qlabel.h>
#include <qlayout.h>

TextLineEditor::TextLineEditor( TokenProvider* token, QWidget *parent )
    : QWidget( parent ), m_token( token )
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setContentsMargins( 6, 6, 6, 6 );
    layout->setSpacing( 6 );

#if QT_VERSION >= 0x030100
    editor = new KLineEdit( this );
#else
    editor = new QLineEdit( this );
#endif        

    editor->setFocus();

    /*Q3DockArea* area = new Q3DockArea( Qt::Horizontal, Q3DockArea::Normal, this );*/
    toolBar = new KToolBar( this );
    tool2Bar = new KToolBar( this );
    tool3Bar = new KToolBar( this );
    
    setupActions();

    /*layout->addWidget( area );*/
    layout->addWidget( toolBar );
    layout->addWidget( tool2Bar );
    layout->addWidget( tool3Bar );
    /*layout->addWidget( mag_vert );
    layout->addWidget( mag_hor );*/
    layout->addWidget( editor );
    
    setLayout( layout );

    

    
    

}

TextLineEditor::~TextLineEditor()
{
}


void TextLineEditor::setupActions()
{
    ac = new KActionCollection( this );
   
    action_undo = KStandardAction::undo( editor, SLOT( undo() ), ac );
    action_undo->setEnabled( false );
    /*connect( editor, SIGNAL( undoAvailable(bool) ), action_undo, SLOT( setEnabled(bool) ) );*/// -!F: original, runtime warning: no such signal

    action_redo = KStandardAction::redo( editor, SLOT( redo() ), ac );
    action_redo->setEnabled( false );
    /*connect( editor, SIGNAL( redoAvailable(bool) ), action_redo, SLOT( setEnabled(bool) ) );*/// -!F: original, runtime warning: no such signal

    action_cut = KStandardAction::cut( editor, SLOT( cut() ), ac );
    /*action_cut->setEnabled( false );
    connect( editor, SIGNAL( copyAvailable(bool) ), action_cut, SLOT( setEnabled(bool) ) );*/// -!F: original

    action_copy = KStandardAction::copy( editor, SLOT( copy() ), ac );
    /*action_copy->setEnabled( false );
    connect( editor, SIGNAL( copyAvailable(bool) ), action_copy, SLOT( setEnabled(bool) ) );*/// -!F: original

    action_paste = KStandardAction::paste( editor, SLOT( paste() ), ac );
    
    connect( editor, SIGNAL( textChanged(const QString &) ), this, SLOT( updateActions(const QString &) ) );

    /*KAction* textDataAct = new KAction( i18n("Insert &Data Field"), "contents", 0, this, SLOT( insertNewField() ), ac, "text_data_act");*/// -!F: original, delete
    KAction* textDataAct = new KAction( this );
    textDataAct->setText( i18n("Insert &Data Field") );
    textDataAct->setIcon( KIcon( "view-table-of-contents-ltr" ) );
    ac->addAction( "text_data_act", textDataAct );
    connect( textDataAct, SIGNAL(triggered(bool)), this, SLOT(insertNewField()) );
    
    toolBar->addAction( action_undo );
    toolBar->addAction( action_redo );
    toolBar->addSeparator();
    toolBar->addAction( action_cut );
    toolBar->addAction( action_copy );
    toolBar->addAction( action_paste );
 

    QStringList fuentes;
    fuentes += "Times Roman (Medium) 8 point";
    fuentes += "Times Roman (Medium) 10 point";
    fuentes += "Times Roman (Bold) 10 point";
    fuentes += "Times Roman (Bold) 12 point";
    fuentes += "Times Roman (Bold) 14 point";
    fuentes += "Times Roman (Italic) 12 point";
    fuentes += "Helvetica (Medium) 6 point";
    fuentes += "Helvetica (Medium) 10 point";
    fuentes += "Helvetica (Medium) 12 point";
    fuentes += "Helvetica (Bold) 12 point";
    fuentes += "Helvetica (Bold) 14 point";
    fuentes += "Helvetica (Italic) 12 point";
    fuentes += "Presentation (Bold) 18 point";
    fuentes += "Letter Gothic (Medium) 9.5 point";
    fuentes += "Prestige Elite (Medium) 7 point";
    fuentes += "Prestige Elite (Bold) 10 point";
    fuentes += "Courier (Medium) 10 point";
    fuentes += "Courier (Bold) 12 point";
    fuentes += "OCR-A 12 point";
    fuentes += "OCR-B 12 point";


    action_font_type = new KComboBox(this) ;
    connect( action_font_type, SIGNAL( activated(int) ), this, SLOT( setFontType(int) ) );
    action_font_type->addItems(fuentes) ;
    tool2Bar->addWidget( action_font_type );
    
    tool2Bar->addAction( textDataAct );
    
    /*int minWidth = tool2Bar->widgetForAction( textDataAct )->width() + action_font_type->width() + 400;*/
    int minWidth = 600;
    tool2Bar->setMinimumWidth( minWidth );
    setMinimumWidth( minWidth );
    
    
    
    QLabel* labelv = new QLabel( i18n("&Mag. Vert.:"), tool3Bar );
    tool3Bar->addWidget( labelv );
    mag_vert = new KIntNumInput( tool3Bar);
    tool3Bar->addWidget( mag_vert );
    QLabel* labelh = new QLabel( i18n("&Mag. Hor.:"), tool3Bar );
    tool3Bar->addWidget( labelh );
    mag_hor = new KIntNumInput( tool3Bar );
    tool3Bar->addWidget( mag_hor );
    connect( mag_vert, SIGNAL( activated(int) ), this, SLOT( setVerMag(int) ) );
    connect( mag_hor, SIGNAL( activated(int) ), this, SLOT( setHorMag(int) ) );  
    mag_vert->setRange( 1, 9, 1 );
    mag_vert->setSliderEnabled( false );
    mag_hor->setRange( 1, 9, 1 );
    mag_vert->setSliderEnabled( false );
        
       
    labelv->setBuddy( mag_vert );
    labelh->setBuddy( mag_hor );

    
    
    updateActions();


}

QString TextLineEditor::text()
{
    return editor->text();
}

void TextLineEditor::setText( const QString & t )
{
    editor->setText( t );
}


void TextLineEditor::updateActions(const QString & text)
{
    if( !text.isEmpty() ) {
        
        if( editor->isUndoAvailable() ) {
            action_undo->setEnabled( true );
        } else {
            action_undo->setEnabled( false );
        }
        
        if( editor->isRedoAvailable() ) {
            action_redo->setEnabled( true );
        } else {
            action_redo->setEnabled( false );
        }
    }
}


void TextLineEditor::insertNewField()
{
    TokenDialog dlg( m_token, this);
    if( dlg.exec() == QDialog::Accepted )
        editor->insert( dlg.token() ) ;
}

void TextLineEditor::setFontType( int index )
{    
    action_font_type->setCurrentIndex(index);
}

int TextLineEditor::getFontType()
{    
    return action_font_type->currentIndex();
}
void TextLineEditor::setVertMag( int index )
{    
    mag_vert->setValue(index);
}

int TextLineEditor::getVertMag()
{    
    return mag_vert->value();
}

void TextLineEditor::setHorMag( int index )
{    
    mag_hor->setValue(index);
}

int TextLineEditor::getHorMag()
{    
    return mag_hor->value();
}
#include "textlineedit.moc"
