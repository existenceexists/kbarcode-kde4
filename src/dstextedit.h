/***************************************************************************
                         dstextedit.h  -  description
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

#ifndef _DS_TEXT_EDIT_H_
#define _DS_TEXT_EDIT_H_

#include <qnamespace.h> 

#if QT_VERSION >= 0x030100
    #include <k3textedit.h>
#else
    #include <q3textedit.h>
#endif

// Otherwise moc will complain
#if QT_VERSION >= 0x030100
  #define TextEditBase K3TextEdit
#else
  #define TextEditBase Q3TextEdit
#endif 

class DSTextEdit : public TextEditBase {
    Q_OBJECT
        
  public:
    DSTextEdit( QWidget* parent=0 );

    //virtual void moveCursor( CursorAction action, bool select );
  private slots:
    void fixParagraphs();
      
  private:
    /** 
     * returns true if the cursor is currently inside
     * of a token of the form [value].
     */
    //bool cursorIsInToken();
};

#endif // _DS_TEXT_EDIT_H_

