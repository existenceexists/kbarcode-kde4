/***************************************************************************
                          configdialog.h  -  description
                             -------------------
    begin                : Fre Apr 26 2002
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

#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <kpagedialog.h>

struct labelprinterdata;
struct mysqldata;
class KComboBox;
class KColorButton;
class KIntNumInput;
class KDoubleNumInput;
class KLineEdit;
class KPushButton;
class KUrlRequester;
class QButtonGroup;
class QRadioButton;
class QCheckBox;
class QLabel;
class QString;
class QWidget;
class SqlWidget;
/** KBarcodes configuration dialog for advanced settings.
  */

class ConfigDialog : public KPageDialog  {
    Q_OBJECT
    public:
        ConfigDialog( QWidget* parent );
        ~ConfigDialog();

        KLineEdit* comment;
        KLineEdit* separator;
        KLineEdit* quote;
        KLineEdit* date;

        KIntNumInput* spinGrid;

        KComboBox* printerQuality;
        KComboBox* pageFormat;

        QLabel* labelDate;

        QCheckBox* checkNewDlg;
        QCheckBox* checkUseCustomNo;

        KComboBox* combo1;
        KComboBox* combo2;
        KComboBox* combo3;

        KComboBox* onNewArticle1;
        KComboBox* onNewArticle2;
        KComboBox* onNewArticle3;
        KComboBox* onNewArticle4;

        KComboBox* onNewGroup1;
        KComboBox* onNewGroup2;
        KComboBox* onNewGroup3;
        KComboBox* onNewGroup4;

        KColorButton* colorGrid;
        
        QButtonGroup* groupPostscript;
        QRadioButton* radioAutomatic;
        QRadioButton* radioLib;
        QRadioButton* radioKBarcodes;
        QRadioButton* radioCustom;
        KUrlRequester* m_url;
        QLabel* purePostscriptFileLabel;
        
        int getPurePostscriptMethod();

    private:
        void accept();

        void setupTab1();
        void setupTab2();
        void setupTab3();
        void setupTab4();
        void setupTab5();
        void setupTab6();

    private slots:
        void updateDatePreview();
        void updatePurePostscriptFileLabel(int);
        void updateCustomFileLabel( const QString & );

    protected:
        KComboBox* comboDataMatrix;

        SqlWidget* sqlwidget;
};

#endif
