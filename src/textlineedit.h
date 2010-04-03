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
        void setFontType(int index);
	int  getFontType();
	int  getHorMag();
	void setHorMag( int index );
	int  getVertMag();
	void setVertMag( int index );
    private slots:
        void setupActions();
        void updateActions();
           
        void insertNewField();
	
        
    protected:
        TokenProvider* m_token;

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
