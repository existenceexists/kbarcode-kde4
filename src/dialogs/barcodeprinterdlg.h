/***************************************************************************
                         barcodeprinterdlg.h  -  description
                             -------------------
    begin                : Fri Oct 01 2004
    copyright            : (C) 2004 by Dominik Seichter
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
#ifndef BARCODEPRINTERDLG_H
#define BARCODEPRINTERDLG_H

#include <kdialog.h>

#include <QLabel>

class KComboBox;
class KUrlRequester;
class QCheckBox;
class QLabel;

/**
@author Dominik Seichter
*/
class BarcodePrinterDlg : public KDialog
{
    Q_OBJECT
    public:
        BarcodePrinterDlg(QWidget *parent = 0);
        ~BarcodePrinterDlg();

        int outputFormat() const;
        bool printToFile() const;
        const QString deviceName() const;
        const QString fileName() const;
        
    private slots:
        void enableControls();
        
    private:
        KComboBox* comboFormat;
        KComboBox* comboDevice;
        KUrlRequester* requester;
        
        QCheckBox* checkFile;
        QLabel* label2;
        QLabel* label3;
};

#endif
