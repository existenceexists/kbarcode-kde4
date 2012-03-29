/***************************************************************************
                          rectsettingsdlg.h  -  description
                             -------------------
    begin                : Mit Jun 18 2003
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

#ifndef RECTSETTINGSDLG_H
#define RECTSETTINGSDLG_H

#include <kdialog.h>

class KColorButton;
class KComboBox;
class KIntNumInput;
class QCheckBox;
class QColor;

class RectSettingsDlg : public KDialog  {
    Q_OBJECT
    public: 
        RectSettingsDlg(QWidget *parent=0);
        ~RectSettingsDlg();

        void setBorderColor( const QColor & c );
        void setFillColor( const QColor & c );
        void setBorderWidth( int w );
        void setPenStyle( int s );

        const QColor borderColor() const;
        const QColor fillColor() const;
        int borderWidth() const;
        int penStyle() const;

    private slots:
        void enableControls();
        
    private:
        KColorButton* buttonBorder;
        KColorButton* buttonFill;

        KComboBox* comboLine;
        QCheckBox* checkBorder;

        KIntNumInput* spinWidth;
};

class QPen;
class LineSettingsDlg : public KDialog  {
    Q_OBJECT
    public:
        LineSettingsDlg(QWidget *parent=0);
        ~LineSettingsDlg();

        void setPen( const QPen p );
        QPen pen() const;

    private:
        KColorButton* buttonColor;
        KComboBox* comboLine;
        KIntNumInput* spinWidth;
};
#endif
