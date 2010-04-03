/***************************************************************************
                          printlabeldlg.h  -  description
                             -------------------
    begin                : Mon Jun 30 2003
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

#ifndef PRINTLABELDLG_H
#define PRINTLABELDLG_H

#include <qwidget.h>
#include <kdialog.h>

class QCheckBox;
class KIntNumInput;

/**
  *@author Dominik Seichter
  *
  * This small dialogs allows the user to select how many
  * pages shall be printed and at which position the first
  * label should be printed.
  */
class PrintLabelDlg : public KDialog  {
    Q_OBJECT
    public: 
        PrintLabelDlg(QWidget *parent=0);
        ~PrintLabelDlg();

        /**
          * Allow/disallow the user to enter the
          * numer of labels to print.
          * @param b bool
          */
        void setLabelsEnabled( bool b );
        
        /**
          * returns how many labels should be printed
          */
        int labels() const;

        /**
          * returns at which position the first label shall be printed
          * (counting from the first label being number 1 _not_ 0 )
          */
        int position() const;

        /**
          * returns wether crop marks shall be printed or not
          */
        bool border() const;

    private:
        KIntNumInput* numLabels;
        KIntNumInput* numPos;
        QCheckBox* checkBorder;
};

#endif
