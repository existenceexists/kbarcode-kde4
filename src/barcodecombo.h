/***************************************************************************
                          barcodecombo.h  -  description
                             -------------------
    begin                : Son Apr 13 2003
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

#ifndef BARCODECOMBO_H
#define BARCODECOMBO_H

#include <qvalidator.h>
#include <qwidget.h>
//Added by qt3to4:
#include <QLabel>
#include <kcombobox.h>

#include "barkode.h"

/** A validator that takes to QRegExp's to check
 *  wether a barcode is valid or not.
 */
class BarcodeValidator : public QValidator {
 public:
    BarcodeValidator( QObject* parent = 0);

    QValidator::State validate( QString & input, int & pos ) const;

    /** validate a given input string agains a pattern using 
     *  Perl Compatible Regular Expressions
     *  \param pattern may be NULL
     *  \returns true if the pattern matches
     */
    bool pcreValidate( QString* pattern, const QString & input ) const;
    
    inline void setRegExp( QString* valid, QString* notValid ) {
        m_valid    = valid;
        m_notValid = notValid;
    }

 private:
    QString* m_valid;
    QString* m_notValid;
};

/** A combobox that lists all barcode encodign types
  * supported by KBarcode.
  */
class BarcodeCombo : public KComboBox  {
    Q_OBJECT
    public: 
        BarcodeCombo(QWidget *parent=0);
        ~BarcodeCombo();

        const QString getEncodingType();
        void setEncodingType( const QString & type );
        
    public slots:
        void reloadEncodingTypes();
};

class KIntNumInput;
class KLineEdit;
class KPushButton;
#if QT_VERSION >= 0x030100
    class KTextEdit;
#else
    class QTextEdit;
#endif
class QCheckBox;
class QLabel;

/** This widget is used in BarcodeGenerator and BarcodeSettingsDlg and
  * allows the user to change the data of a barcodeData struct. This powerful
  * widget is always used when the user has to change some property of
  * a barcode.
  *
  * @see BarcodeGenerator, @see BarcodeSettingsDlg
  * @author Dominik Seichter
  */
class BarcodeWidget : public QWidget {
    Q_OBJECT
    public:
        BarcodeWidget(QWidget *parent=0);
        ~BarcodeWidget() { }

        void getData( Barkode & barkode );
        void setData( const Barkode & b );

        void setStandardEnabled( bool b );
        void setDataEnabled( bool b );

        void defaults();

        inline void setTokenProvider( TokenProvider* token );

    private slots:
        void encodingChanged();
        void advanced();
        void changed();
	void tokens();
        void slotValidateValue();

    private:
        TokenProvider* m_token;

        BarcodeCombo* comboStandard;
        KLineEdit* data;
        BarcodeValidator m_validator;

#if QT_VERSION >= 0x030100
        KTextEdit* multi;
#else
        QTextEdit* multi;
#endif
        
        KIntNumInput* spinMargin;
        KIntNumInput* spinScale;
        KIntNumInput* spinRotation;
        KIntNumInput* spinCut;
        QCheckBox* checkText;

        KPushButton* buttonAdvanced;
	KPushButton* buttonToken;

        QLabel* labelStandard;
        QLabel* labelData;

        bool m_enabledata;
        bool m_multi;

        Barkode m_barcode;
};

void BarcodeWidget::setTokenProvider( TokenProvider* token )
{
    m_token = token;
}

#endif
