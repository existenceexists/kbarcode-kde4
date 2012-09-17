/***************************************************************************
                          multilineeditdlg.cpp  -  description
                             -------------------
    begin                : Sam Jan 11 2003
    copyright            : (C) 2003 by Dominik Seichter
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

#include "multilineeditdlg.h"
#include "tokendialog.h"

// KDE includes
#include <kaction.h>
#include <kdeversion.h>
#include <kcolordialog.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <sonnet/speller.h>
#include <dstextedit.h>
#include <ktoolbar.h>
#include <kcombobox.h>
#include <kactioncollection.h>
#include <kstandardaction.h>
#include <ktoggleaction.h>
#include <kfontaction.h>
#include <kfontsizeaction.h>
#include <KUrl>
#include <kshortcut.h>

// Qt includes
#include <qregexp.h>
#include <QActionGroup>
//Added by qt3to4:
#include <QVBoxLayout>

MultiLineEditor::MultiLineEditor( TokenProvider* token, QWidget *parent )
    : QWidget( parent ), m_token( token )
{
    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->setContentsMargins( 6, 6, 6, 6 );
    layout->setSpacing( 6 );

//    ksc = new KSpellConfig( this );

    editor = new DSTextEdit( this );
    //editor->setText( text, "" );
    editor->setFocus();

    toolBar = new KToolBar( this );
    tool2Bar = new KToolBar( this );
    tool3Bar = new KToolBar( this );
    tool4Bar = new KToolBar( this );
    tool5Bar = new KToolBar( this );
    
    setupActions();

    layout->addWidget( toolBar );
    layout->addWidget( tool2Bar );
    layout->addWidget( tool3Bar );
    layout->addWidget( tool4Bar );
    layout->addWidget( tool5Bar );
    layout->addWidget( editor );
    
    setMinimumWidth( 550 );

}

MultiLineEditor::~MultiLineEditor()
{
}


void MultiLineEditor::setupActions()
{
    ac = new KActionCollection( this );

    KAction* action_export = new KAction( this );
    action_export->setText( i18n("Export") );
    ac->addAction( "action_export", action_export );
    connect( action_export, SIGNAL(triggered(bool)), this, SLOT(save()) );

    //
    // Edit Actions
    //
    KAction *action_undo = KStandardAction::undo( editor, SLOT( undo() ), ac );
    action_undo->setEnabled( false );
    connect( editor, SIGNAL( undoAvailable(bool) ), action_undo, SLOT( setEnabled(bool) ) );

    
    KAction *action_redo = KStandardAction::redo( editor, SLOT( redo() ), ac );
    action_redo->setEnabled( false );
    connect( editor, SIGNAL( redoAvailable(bool) ), action_redo, SLOT( setEnabled(bool) ) );

    KAction *action_cut = KStandardAction::cut( editor, SLOT( cut() ), ac );
    action_cut->setEnabled( false );
    connect( editor, SIGNAL( copyAvailable(bool) ), action_cut, SLOT( setEnabled(bool) ) );

    KAction *action_copy = KStandardAction::copy( editor, SLOT( copy() ), ac );
    action_copy->setEnabled( false );
    connect( editor, SIGNAL( copyAvailable(bool) ), action_copy, SLOT( setEnabled(bool) ) );

    KAction* action_paste = KStandardAction::paste( editor, SLOT( paste() ), ac );

    //
    // Character Formatting
    //
    action_bold = new KToggleAction( KIcon("format-text-bold"), i18n("&Bold"), this );
    action_bold->setShortcut( KShortcut( Qt::CTRL + Qt::Key_B ) );
    ac->addAction( "format_bold", action_bold );
    connect( action_bold, SIGNAL( toggled(bool) ), this, SLOT( setBold(bool) ) );

    action_italic = new KToggleAction( KIcon("format-text-italic"), i18n("&Italic"), this );
    action_italic->setShortcut( KShortcut( Qt::CTRL+Qt::Key_I ) );
    ac->addAction( "format_italic", action_italic );
    connect( action_italic, SIGNAL( toggled(bool) ), editor, SLOT( setFontItalic(bool) ) );

    action_underline = new KToggleAction( KIcon("format-text-underline"), i18n("&Underline"), this );
    action_underline->setShortcut( KShortcut( Qt::CTRL+Qt::Key_U ) );
    ac->addAction( "format_underline", action_underline );
    connect( action_underline, SIGNAL( toggled(bool) ), editor, SLOT( setFontUnderline(bool) ) );

    KAction* action_color = new KAction( this );
    action_color->setText( i18n("Text &Color...") );
    action_color->setIcon( KIcon( "format-text-color" ) );
    ac->addAction( "format_color", action_color );
    connect( action_color, SIGNAL(triggered(bool)), this, SLOT(formatColor()) );

    //
    // Font
    //
    action_font = new KFontAction( i18n("&Font"), this );
    ac->addAction( "format_font", action_font );
    connect( action_font, SIGNAL( triggered( const QString & ) ), editor, SLOT( setFontFamily( const QString & ) ) );

    action_font_size = new KFontSizeAction( i18n("Font &Size"), this );
    ac->addAction( "format_font_size", action_font_size );
    connect( action_font_size, SIGNAL( fontSizeChanged(int) ), this, SLOT( setFontSize(int) ) );

    //
    // Alignment
    //
    action_align_left = new KToggleAction( KIcon("align-horizontal-left"), i18n("Align &Left"), this );
    ac->addAction( "format_align_left", action_align_left );
    connect( action_align_left, SIGNAL( toggled(bool) ), this, SLOT( setAlignLeft(bool) ) );

    action_align_center = new KToggleAction( KIcon("align-horizontal-center"), i18n("Align &Center"), this );
    ac->addAction( "format_align_center", action_align_center );
    connect( action_align_center, SIGNAL( toggled(bool) ), this, SLOT( setAlignCenter(bool) ) );

    action_align_right = new KToggleAction( KIcon("align-horizontal-right"), i18n("Align &Right"), this );
    ac->addAction( "format_align_right", action_align_right );
    connect( action_align_right, SIGNAL( toggled(bool) ), this, SLOT( setAlignRight(bool) ) );

    action_align_justify = new KToggleAction( i18n("&Justify"), this );
    ac->addAction( "format_align_justify", action_align_justify );
    connect( action_align_justify, SIGNAL( toggled(bool) ), this, SLOT( setAlignJustify(bool) ) );

    QActionGroup* alignmentGroup = new QActionGroup(this);
    action_align_left->setActionGroup( alignmentGroup );
    action_align_center->setActionGroup( alignmentGroup );
    action_align_right->setActionGroup( alignmentGroup );
    action_align_justify->setActionGroup( alignmentGroup );

    //KAction* action_spell = KStandardAction::spelling( this, SLOT( checkSpelling() ), ac );

    KAction* textDataAct = new KAction( this );
    textDataAct->setText( i18n("Insert &Data Field") );
    textDataAct->setIcon( KIcon( "view-table-of-contents-ltr" ) );
    ac->addAction( "text_data_act", textDataAct );
    connect( textDataAct, SIGNAL(triggered(bool)), this, SLOT(insertNewField()) );
    
    toolBar->addAction( action_export );
    toolBar->addSeparator();
    toolBar->addAction( textDataAct );
    
    tool2Bar->addAction( action_undo );
    tool2Bar->addAction( action_redo );
    tool2Bar->addSeparator();
    tool2Bar->addAction( action_cut );
    tool2Bar->addAction( action_copy );
    tool2Bar->addAction( action_paste );
    
    tool3Bar->addAction( action_bold );
    tool3Bar->addAction( action_italic );
    tool3Bar->addAction( action_underline );
//#if KDE_IS_VERSION( 3, 1, 90 )
//    action_spell->plug( toolBar );
//#endif
           
    tool4Bar->addAction( action_font );
    tool4Bar->addAction( action_font_size );
    tool4Bar->addAction( action_color );
    
    tool5Bar->addAction( action_align_left );
    tool5Bar->addAction( action_align_center );
    tool5Bar->addAction( action_align_right );
    tool5Bar->addAction( action_align_justify );
    
    //
    // Setup enable/disable
    //
    updateActions();

    connect( editor, SIGNAL( currentFontChanged( const QFont & ) ), this, SLOT( updateFont() ) );
    connect( editor, SIGNAL( currentFontChanged( const QFont & ) ), this, SLOT( updateCharFmt() ) );
    connect( editor, SIGNAL( cursorPositionChanged() ), this, SLOT( updateAligment() ) );
}

QString MultiLineEditor::text()
{
    return editor->toHtml();
}

void MultiLineEditor::setText( const QString & t )
{
    editor->setHtml( t );
}

void MultiLineEditor::updateCharFmt()
{
    action_bold->setChecked( editor->fontWeight() >= QFont::Bold );
    action_italic->setChecked( editor->fontItalic() );
    action_underline->setChecked( editor->fontUnderline() );
}

void MultiLineEditor::updateAligment()
{
    int align = editor->alignment();

    switch ( align ) {
        case Qt::AlignRight:
            action_align_right->setChecked( true );
            break;
        case Qt::AlignCenter:
            action_align_center->setChecked( true );
            break;
        case Qt::AlignLeft:
            action_align_left->setChecked( true );
            break;
        case Qt::AlignJustify:
            action_align_justify->setChecked( true );
            break;
        default:
            break;
    }
}

void MultiLineEditor::updateFont()
{
    if ( int(editor->fontPointSize() + 0.5) > 0 )
        action_font_size->setFontSize( int(editor->fontPointSize() + 0.5) );
    action_font->setFont( editor->fontFamily() );
}

void MultiLineEditor::updateActions()
{
    updateCharFmt();
    updateAligment();
    updateFont();
}

void MultiLineEditor::formatColor()
{
    QColor col;

    int s = KColorDialog::getColor( col, editor->textColor(), editor );
    if ( s != QDialog::Accepted )
        return;

    editor->setTextColor( col );
}

void MultiLineEditor::setAlignLeft( bool yes )
{
    if ( yes )
        editor->setAlignment( Qt::AlignLeft );
}

void MultiLineEditor::setAlignRight( bool yes )
{
    if ( yes )
        editor->setAlignment( Qt::AlignRight );
}

void MultiLineEditor::setAlignCenter( bool yes )
{
    if ( yes )
        editor->setAlignment( Qt::AlignCenter );
}

void MultiLineEditor::setAlignJustify( bool yes )
{
    if ( yes )
        editor->setAlignment( Qt::AlignJustify );
}

void MultiLineEditor::insertNewField()
{
    TokenDialog dlg( m_token, this );
    if( dlg.exec() == QDialog::Accepted )
        editor->insertHtml( dlg.token() );
}

void MultiLineEditor::checkSpelling()
{
/*
#if KDE_IS_VERSION( 3, 1, 90 )
    QString s;
    if ( editor->hasSelectedText() )
        s = editor->selectedText();
    else
        s = editor->text();

    spell = new KSpell( this, i18n("Spell Checking"), 0, 0, 0, true, true, KSpell::HTML );
    spell->setAutoDelete( true );
    spell->check( s, true );
    connect( spell, SIGNAL( done(const QString &) ), this, SLOT( spellCheckDone(const QString &) ) );

    spell->cleanUp();
#endif
*/
}

void MultiLineEditor::spellCheckDone(const QString & buffer)
{
    editor->setText( buffer );
    /*spell->cleanUp();*/
}

void MultiLineEditor::save()
{
    QString name = KFileDialog::getSaveFileName ( KUrl(), "*", this );
    if( name.isEmpty() )
        return;

    QFile file( name );
    if ( file.open( QIODevice::WriteOnly ) ) {
        QTextStream ts( &file );
        ts << editor->toHtml();
    }
}

void MultiLineEditor::setBold( bool bold )
{
    QFont::Weight fontWeight;
    if( bold ) {
        fontWeight = QFont::Bold;
    } else {
        fontWeight = QFont::Normal;
    }
    editor->setFontWeight( fontWeight );
}

void MultiLineEditor::setFontSize( int size )
{
    editor->setFontPointSize( (qreal) size );
}

#include "multilineeditdlg.moc"
