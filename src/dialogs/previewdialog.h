/***************************************************************************
                          previewdialog.h  -  description
                             -------------------
    begin                : Die Dez 10 2002
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

#ifndef PREVIEWDIALOG_H
#define PREVIEWDIALOG_H

#include <qdialog.h>
//Added by qt3to4:
#include <QLabel>
#include <kabc/addressee.h>

class Definition;
class KComboBox;
class KIntNumInput;
class KLineEdit;
class KPushButton;
class QIODevice;
class QLabel;
class QWidget;
class PreviewDialog : public QDialog  {
    Q_OBJECT
    public: 
        PreviewDialog( QIODevice* device, Definition* d, QString filename, QWidget *parent=0);
        ~PreviewDialog();

    private slots:
        void updatechanges();
        void customerNameChanged( int index );
        void customerIdChanged( int index );
        void setupSql();
        void selectAddress();

    private:
        QLabel* preview;
        
        KLineEdit* articleId;
        KLineEdit* groupName;
        KLineEdit* serialStart;
        KLineEdit* lineAddr;

        KIntNumInput* spinIndex;
        KIntNumInput* serialInc;
    
        KComboBox* customerName;
        KComboBox* customerId;
        
        KPushButton* buttonUpdate;
        KPushButton* buttonClose;
        KPushButton* buttonAddr;

        KABC::Addressee m_address;

        Definition* def;
        QIODevice* file;

        QString m_filename;

        static int customer_index;
        static int m_index;
        static QString group;
        static QString article;
};

#endif
