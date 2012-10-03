//
// C++ Interface: textlineedit
//
// Description: 
//
//
// Author: Dominik Seichter <domseichter@web.de>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef TECLINEEDITDLG_H
#define TECLINEEDITDLG_H

#include <qwidget.h>
#include <knuminput.h>

class KActionCollection;
class KToggleAction;
class KFontAction;
class KFontSizeAction;
class KToolBar;
class KSpell;
class KSpellConfig;
class KComboBox;
class TokenProvider;
class KAction;
class QEvent;
class QObject;

#if QT_VERSION >= 0x030100
    class KLineEdit;
#else
    class QLineEdit;
#endif

class TextLineEditor : public QWidget {
    Q_OBJECT
    public:
        TextLineEditor( TokenProvider* token, QWidget *parent=0);
        ~TextLineEditor();
        QString text();
	KIntNumInput* mag_vert;
	KIntNumInput* mag_hor;
	
        void setText( const QString & t );
	int  getFontType();
	int  getHorMag();
	int  getVertMag();
    public slots:
        void setFontType(int index);
        void setHorMag( int index );
        void setVertMag( int index );
    private slots:
        void setupActions();
        void updateActions(const QString & text=QString());
           
        void insertNewField();
	
        void cut();
        void copy();
        void paste();
        
    protected:
        bool eventFilter(QObject *obj, QEvent *ev);
        void saveSelection();
        
        int selectionStartIndex;
        int selectionLength;
        
        TokenProvider* m_token;
        KAction* action_undo;
        KAction* action_redo;
        KAction* action_copy;
        KAction* action_cut;
        KAction* action_paste;

#if QT_VERSION >= 0x030100
        KLineEdit* editor;
#else
        QLineEdit* editor;
#endif
        
        KActionCollection* ac;
        
	KComboBox *action_font_type ;
        KToolBar* toolBar;
        KToolBar* tool2Bar;
	KToolBar* tool3Bar;
  
};

#endif
