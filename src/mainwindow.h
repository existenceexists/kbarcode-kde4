/* mainwindow.h - KBarcode Main Window Base
 * 
 * Copyright 2003-2008 Dominik Seichter, domseichter@web.de
 * Copyright 2008 Váradi Zsolt Gyula, karmaxxl@gmail.com
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <qwidget.h>
/*#include <kmainwindow.h>*/
#include <KXmlGuiWindow>

class KAction;
class KMenu;

/** The base class of all main windows of KBarcode. It provides standard menu items
  * for database connections, preferences, help, the first start assistant and donations.
  * It checks also if this is the first start of KBarcode and if the configuration assistant
  * should be started.
  *
  * @author Dominik Seichter
  */

class MainWindow : public KXmlGuiWindow {
    Q_OBJECT
    public:
        MainWindow(QWidget *parent=0, Qt::WFlags f = Qt::WType_TopLevel | Qt::WDestructiveClose );
        ~MainWindow();

        /** Is this the first start of KBarcode ?
          * If true, the configuration assistant will
          * be started automatically.
          */
        bool isFirstStart() const {
            return first;
        }


        /** Do a system check and return a text
          * informing the user about missing, programs
          * and drivers for barcode support and database
          * access.
          */
        static QString systemCheck();
        QString kbarcodeDirectoryName;// -!F: delete

    public slots:
        void assistant();

    signals:
        void connectedSQL();

    private slots:
        void connectMySQL();
        void appHelpActivated();
        void startInfo();
        bool newTables();
        void donations();
        void showCheck();
        //void slotFunctionMap();

    protected slots:
        void loadConfig();
        void saveConfig();

    protected:
        void setupActions(QString directoryName);

        static bool autoconnect;
        static bool startassistant;

        bool first;
        
        KAction* connectAct;
        KAction* importLabelDefAct;
        KAction* importExampleAct;
        
        KMenu* file;
        KMenu* settings;
        KMenu* hlpMenu;
};
#endif
