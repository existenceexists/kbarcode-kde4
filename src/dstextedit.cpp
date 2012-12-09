/***************************************************************************
                         dstextedit.cpp  -  description
                             -------------------
    begin                : Sam Jun 04 2005
    copyright            : (C) 2005 by Dominik Seichter
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

#include "dstextedit.h"
#include "dstextedit.moc"
#include <qregexp.h>
#include <QDebug>
//Added by qt3to4:
#include <QList>

DSTextEdit::DSTextEdit( QWidget* parent )
    : TextEditBase( parent )
{
    /*connect( this, SIGNAL( textChanged() ), this, SLOT( fixParagraphs() ) );*/// If we call fixParagraphs() undo and redo don't work
}

/* fixParagraphs() removes all but the first occurence of the opening paragraph tag <p ...>
   and also removes all but the last occurence of the closing paragraph tag </p>
   so when fixParagraphs() finishes all the editor's text is enclosed 
   between just one html tag <p ...> and just one html tag </p>
 */
void DSTextEdit::fixParagraphs()
{
    struct { 
        QFont  font;
        QColor color;
        Qt::Alignment alignment;
    } tFormattings;

    QString t;
    int pos = 0;
    int count = 0;
    int indexSelectionFrom, indexSelectionTo; // needed to save the selection
    QRegExp reg("<p[^>]*>");

    // disconnect us first as we change the text here
    disconnect( this, SIGNAL( textChanged() ), this, SLOT( fixParagraphs() ) );

    indexSelectionFrom = textCursor().selectionStart();
    indexSelectionTo = textCursor().selectionEnd();

    t                      = this->toHtml();
    tFormattings.font      = this->currentFont();
    tFormattings.color     = this->textColor();
    tFormattings.alignment = this->alignment();

    while( pos != -1 ) 
    {
        pos = reg.indexIn( t, pos );
        if( pos != -1 )
        {

            ++count;

            if( count > 1 ) //&& pos != -1 ) 
                t = t.remove( pos, reg.matchedLength() );
            else
                pos += reg.matchedLength();
        }
    }

    pos = t.length() - 1;
    count = 0;

    while( pos != -1 ) 
    {
        pos = t.lastIndexOf( "</p>", pos );
        if( pos != -1 )
        {
            ++count;

            if( count > 1 ) {//&& pos != -1 ) 
                //t = t.replace( pos, 4, "<br />" ); // no need to replace with <br />" as it is inserted by the QTextEditor automatically
                t = t.replace( pos, 4, "" );
            } else
                pos -= 4;
        }
    }

    this->setHtml( t );
    this->setCurrentFont( tFormattings.font );
    this->setTextColor( tFormattings.color );
    this->setAlignment( tFormattings.alignment );
    QTextCursor cursor = textCursor();
    cursor.setPosition( indexSelectionFrom );// Set the cursor position and a selection if any
    cursor.setPosition( indexSelectionTo, QTextCursor::KeepAnchor );// Set the cursor position and a selection if any
    setTextCursor( cursor ); 



    connect( this, SIGNAL( textChanged() ), this, SLOT( fixParagraphs() ) );
}

/*
void DSTextEdit::moveCursor( CursorAction action, bool select )
{
    do { 
        TextEditBase::moveCursor( action, select );
    } while( cursorIsInToken() );
}

bool DSTextEdit::cursorIsInToken() 
{
    int para, index;
    int firstopen, firstclose;
    QString data;

    getCursorPosition( &para, &index );

    data = text( para );

    qDebug("data=" + data );
    --index;
    firstopen = data.findRev( "[", index );
    firstclose = data.findRev( "]", index );
    ++index;

    if( firstopen != -1 && firstopen > firstclose )
    {
        firstopen = data.find( "[", index );
        firstclose = data.find( "]", index );
        if( ( firstclose != -1 && firstopen != -1 && firstclose < firstopen ) ||
            ( firstclose != -1 && firstopen == -1 ) )
            return true;
    }

    return false;
}

*/
