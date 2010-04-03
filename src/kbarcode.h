/***************************************************************************
                          kbarcode.h  -  description
                             -------------------
    begin                : Don Apr 18 12:34:56 CEST 2002
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

#ifndef KBARCODE_H
#define KBARCODE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qstring.h>

enum linebreak {
    NO_BREAK,
    LINE_BREAK,
    LABEL_X,
    NEW_PAGE,
    ARTICLE_GROUP_NO
};

#include "mainwindow.h"

class KAction;
class KPushButton;
/** The main window of KBarcode. All submodules
  * are started from here. Submodules are the LabelEditor
  * LabelPrinter (Batchprinting), DatabaseBrowser and BarcodeDialog.
  * There are also a few smaller modules.
  */
class KBarcode: public MainWindow {
    Q_OBJECT
    public:
        KBarcode( QWidget *parent=0, Qt::WFlags f = Qt::WType_TopLevel | Qt::WDestructiveClose );
        ~KBarcode();

        /** Parse all commandline options and start batchprinting
         *  if necessary. 
         *  \returns true if the application should exit after this function
         */          
        bool parseCmdLine();

    private slots:
        void startBarcode();
        void startLabelEditor();
        void startBatchPrint();

        void editArticles();
        void editCustomers();
        void editCustomerText();
        void editLabelDef();

        void enableData();

        void importCSV();

    private:
        void setupActions();

		// TODO: DCOP must die!
		
        /** allow the DCOP interface
          * to check for an SQL connection.
          */
        bool isSQLConnected() const;
        /** allow the DCOP interface
          * to connect to the databases.
          */
        bool connectSQL();
        /** allow DCOP interface
          * to start the configuration wizard.
          */
        void showWizard();
        /** show preferences from DCOP.
          */
        void showConfigure();


        KAction* connectAct;
        KPushButton* buttonSingle;
        KPushButton* buttonEditor;
        KPushButton* buttonBatch;
        KPushButton* buttonData;
};

#endif
