/***************************************************************************
                          barcodedialogs.h  -  description
                             -------------------
    begin                : Fre Sep 5 2003
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

#ifndef BARCODEDIALOGS_H
#define BARCODEDIALOGS_H

#include <QMap>
#include <QList>
#include <QWidget>

#include <kpagedialog.h>
#include <kpagewidgetmodel.h>

class Barkode;
class KColorButton;
class KComboBox;
class KDoubleNumInput;
class KIntNumInput;
class QCheckBox;
class QRadioButton;

/** A base class for all widgets, that will be used in AdvancedBarcodeDialog
  * to modify the settings of a barcode. The API is simple. You can only set the
  * current barcode settings and retrieve them after the user modified them.
  *
  * @author Dominik Seichter
  */
class BarcodeDlgBase {
    public:
        virtual ~BarcodeDlgBase() { }

        virtual void setData( Barkode* b ) = 0;
        virtual void getData( Barkode* b ) const = 0;
};

/**
  * A configuration dialog for advanced barcode settings.
  * Used in BarcodeWidget. It loads the wigets below into tabs.
  * @see TBarcodeDlg
  * @see PDF417BarcodeDlg
  * @see DataMatrixDlg
  * @seeSequenceDlg
  *
  * @author Dominik Seichter
  */
class AdvancedBarcodeDialog : public KPageDialog {
    Q_OBJECT
    public:
        AdvancedBarcodeDialog( QString type, QWidget* parent = 0 );
        ~AdvancedBarcodeDialog();

        void setData( Barkode* b );
        void getData( Barkode* b );

    private:
        QList<BarcodeDlgBase*> list;
};

/** A configuration widget for TBarcode settings.
  * @author Dominik Seichter
  */
class TBarcodeDlg : public QWidget, public BarcodeDlgBase {
   Q_OBJECT
    public: 
        TBarcodeDlg(QWidget *parent=0 );

        void setData( Barkode* b );
        void getData( Barkode* b ) const;

    private:
        KDoubleNumInput* spinModule;
        KIntNumInput* spinHeight;
        QCheckBox* checkEscape;
        QCheckBox* checkAbove;
        QCheckBox* checkAutoCorrect;
        KComboBox* comboCheckSum;

        QMap<int, QString> map;
};

/** A configuration Dialog for PDF417 settings.
  * @author Dominik Seichter
  */
class PDF417BarcodeDlg : public QWidget, public BarcodeDlgBase {
   Q_OBJECT
    public:
        PDF417BarcodeDlg(QWidget *parent=0);

        void setData( Barkode* b );
        void getData( Barkode* b ) const;
        
    private:
        KIntNumInput* spinRow;
        KIntNumInput* spinCol;
        KIntNumInput* spinErr;
};

/** A configuration widget for DataMatrix settings.
  * @author Dominik Seichter
  */
class DataMatrixDlg : public QWidget, public BarcodeDlgBase {
   Q_OBJECT
    public:
        DataMatrixDlg(QWidget *parent=0);

        void setData( Barkode* b );
        void getData( Barkode* b ) const;
        
    private:
        KComboBox* comboDataMatrix;
};

/** A configuration widget for barcode sequences.
  * @author Dominik Seichter
  */
class SequenceDlg : public QWidget, public BarcodeDlgBase {
    Q_OBJECT
    public:
        SequenceDlg(QWidget *parent=0);

        void setData( Barkode* b );
        void getData( Barkode* b ) const;

    private slots:
        void enableControls();
        
    private:
        QCheckBox* checkSequence;
        QRadioButton* radioNumbers;
        QRadioButton* radioAlpha;
        QRadioButton* radioAlphaNum;

        KIntNumInput* spinStep;
        KIntNumInput* spinStart;
};

/** A configuration widget for colors in pure postscript barcodes
  * @author Dominik Seichter
  */
class ColorDlg : public QWidget, public BarcodeDlgBase {
    Q_OBJECT
    public:
        ColorDlg(QWidget *parent=0);

        void setData( Barkode* b );
        void getData( Barkode* b ) const;

    private:
        KColorButton* buttonBarColor;
        KColorButton* buttonBackColor;
        KColorButton* buttonTextColor;
};

/** A configuration widget for colors in pure postscript barcodes
  * @author Dominik Seichter
  */
class PurePostscriptDlg : public QWidget, public BarcodeDlgBase {
    Q_OBJECT
    public:
        PurePostscriptDlg(QWidget *parent=0);

        void setData( Barkode* b );
        void getData( Barkode* b ) const;

    private:
        QCheckBox* checkChecksum;
};

#endif
