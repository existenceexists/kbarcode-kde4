/***************************************************************************
                          barcodegenerator.h  -  description
                             -------------------
    begin                : Son Dez 29 2002
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

#ifndef BARCODEDIALOG_H
#define BARCODEDIALOG_H

#include <qdialog.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPixmap>
#include <QHBoxLayout>
#include <QLabel>

class BarcodeCombo;
class BarcodeWidget;
class KAction;
class KColorButton;
class KMenuBar;
class KToolBar;
class KIntNumInput;
class KLineEdit;
class KPushButton;
class KMenu;
class QCheckBox;
class QLabel;
class QPixmap;
class QVBoxLayout;
class QHBoxLayout; 
class QGridLayout;
class TokenProvider;
class QScrollArea;

/** This class provides a dialog, where the user can create a single barcode
  * and print it, copy it to the clipboard or save it in various image formats.
  * This dialog can also be seen as a powerful replacement for e.g. xbarcode.
  *
  * All barcoding features are available for the user. This dialog allows also
  * to just experiment a little bit with barcodes.
  */
class BarcodeGenerator : public QDialog
{ 
    Q_OBJECT

    public:
        BarcodeGenerator( QWidget* parent = 0 );
        ~BarcodeGenerator();

    private:
        BarcodeWidget* widget;
        TokenProvider* m_token;

        KPushButton* buttonGenerate;
        KPushButton* buttonPrint;
        KPushButton* buttonSave;
        KPushButton* buttonCopy;
        KPushButton* buttonClose;
        QLabel* barcode;
        QScrollArea* sv;

    private slots:
        void generate();
        void save();
        void print();
        void copy();

    protected:
        QHBoxLayout* BarcodeGeneratorLayout;
        QVBoxLayout* Layout6;
        QVBoxLayout* Layout5;
};

#endif // BARCODEDIALOG_H
