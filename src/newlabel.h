/***************************************************************************
                          newlabel.h  -  description
                             -------------------
    begin                : Son Mai 5 2002
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

#ifndef NEWLABEL_H
#define NEWLABEL_H

#include <qdialog.h>
#include <qwidget.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>

#include "definition.h"

class QVBoxLayout;
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QComboBox;
class KComboBox;
class KPushButton;
class QLabel;
class QString;
class LabelPreview;
class NewLabel : public QDialog
{ 
    Q_OBJECT
    public:
        NewLabel( QWidget* parent = 0, bool change = FALSE, Qt::WFlags fl = 0 );
        ~NewLabel();

        int labelId() const { return curid; }
        bool empty() const;

        void setLabelId( int id );

        static bool isInCombo( QComboBox* combo, QString text );

    private slots:
        void updateType();
        void updateText();
        void add();
        
    private:
        void fillData();

        QLabel* TextLabel1;
        QLabel* TextLabel2;
        QLabel* TextLabel3;
        QCheckBox* checkEmpty;
        KComboBox* comboProducer;
        KComboBox* comboType;
        KPushButton* buttonOwnFormat;
        QLabel* TextLabel4;
        KPushButton* buttonOk;
        KPushButton* buttonCancel;
        LabelPreview* preview;
        QStringList* types;
        
    protected:
        int curid;
        QVBoxLayout* NewLabelLayout;
        QHBoxLayout* Layout5;
        QHBoxLayout* Layout2;
        QHBoxLayout* Layout1;
};

#endif // NEWLABEL_H
