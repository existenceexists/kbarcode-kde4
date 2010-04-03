/***************************************************************************
                          multilineeditdlg.h  -  description
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

#ifndef MULTILINEEDITDLG_H
#define MULTILINEEDITDLG_H

#include <qwidget.h>
#include "documentitem.h"

class KActionCollection;
class KToggleAction;
class KFontAction;
class KFontSizeAction;
class KToolBar;
class KSpell;
class KSpellConfig;
class DSTextEdit;
class TokenProvider;

class MultiLineEditor : public QWidget {
    Q_OBJECT
    public:
        MultiLineEditor( TokenProvider* token, QWidget *parent=0 );
        ~MultiLineEditor();
        QString text();
        void setText( const QString & t );
        
    private slots:
        void setupActions();
        void updateActions();
        void spellCheckDone(const QString & buffer);

        void save();
        void updateFont();
        void updateCharFmt();
        void updateAligment();
        void formatColor();
        void checkSpelling();
        void setAlignLeft( bool yes );
        void setAlignRight( bool yes );
        void setAlignCenter( bool yes );
        void setAlignJustify( bool yes );               
        void insertNewField();
        
    protected:
        TokenProvider* m_token;

	DSTextEdit* editor;
        
        KActionCollection* ac;
        
        KToggleAction *action_bold;
        KToggleAction *action_italic;
        KToggleAction *action_underline;

        KFontAction *action_font;
        KFontSizeAction *action_font_size;

        KToggleAction *action_align_left;
        KToggleAction *action_align_right;
        KToggleAction *action_align_center;
        KToggleAction *action_align_justify;

        KToolBar* toolBar;
        KToolBar* tool2Bar;
        KToolBar* tool3Bar;

        KSpell* spell;
};

#endif
