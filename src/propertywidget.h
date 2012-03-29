/***************************************************************************
                         propertywidget.h  -  description
                             -------------------
    begin                : Do Sep 10 2004
    copyright            : (C) 2004 by Dominik Seichter
    email                : domseichter@web.de
 ***************************************************************************/

/***************************************************************************
                                                                          
    This program is free software; you can redistribute it and/or modify  
    it under the terms of the GNU General Public License as published by  
    the Free Software Foundation; either version 2 of the License, or     
    (at your option) any later version.                                   
                                                                          
 ***************************************************************************/
 
#ifndef PROPERTYWIDGET_H
#define PROPERTYWIDGET_H

#include <qwidget.h>
//Added by qt3to4:
#include <QGridLayout>
#include "labelutils.h"

class BarcodeWidget;
class DocumentItem;
class KColorButton;
class KComboBox;
class KIntNumInput;
class KDoubleNumInput;
class K3MacroCommand;
class KTextEdit;
class MultiLineEditor;
class QCheckBox;
class QGridLayout;
class QRadioButton;
//NY23
class TextLineEditor;
//NY23
class TokenProvider;

class QWidget;
class KUrlRequester;
class KLineEdit;
class KPushButton;

/**
@author Dominik Seichter
*/
class PropertyWidget : public QWidget
{
    Q_OBJECT
    public:
        PropertyWidget( QWidget* parent );

        virtual void applySettings( DocumentItem* item, K3MacroCommand* command ) = 0;
        virtual void initSettings( DocumentItem* item ) = 0;
    
    protected:
        QGridLayout* grid;
};

class PropertyBorder : public PropertyWidget {
    Q_OBJECT
    public:
        PropertyBorder( QWidget* parent );

        virtual void applySettings( DocumentItem* item, K3MacroCommand* command );
        virtual void initSettings( DocumentItem* item );

    private slots:
        void enableControls();
        
    private:
        KColorButton* buttonColor;
        KComboBox* comboLine;
        KIntNumInput* spinWidth;    
        QCheckBox* checkBorder;
};

class PropertyRotation : public PropertyWidget {
    Q_OBJECT
    public:
        PropertyRotation( QWidget* parent );

        virtual void applySettings( DocumentItem* item, K3MacroCommand* command );
        virtual void initSettings( DocumentItem* item );

    private:        
        KComboBox* comboRotation;
};

class PropertyFill : public PropertyWidget {
    Q_OBJECT
    public:
        PropertyFill( QWidget* parent );

        virtual void applySettings( DocumentItem* item, K3MacroCommand* command );
        virtual void initSettings( DocumentItem* item );
        
    private:
        KColorButton* buttonColor;
};

class PropertyBarcode : public PropertyWidget, private LabelUtils {
    Q_OBJECT
    public:
        PropertyBarcode( TokenProvider* token, QWidget* parent );

        virtual void applySettings( DocumentItem* item, K3MacroCommand* command );
        virtual void initSettings( DocumentItem* item );

    private slots:
        void changedCombo();
        
    private:
        KComboBox* comboComplex;
        BarcodeWidget* barcode;
};

class PropertyText : public PropertyWidget, private LabelUtils {
    Q_OBJECT
    public:
        PropertyText( TokenProvider* token, QWidget* parent );

        virtual void applySettings( DocumentItem* item, K3MacroCommand* command );
        virtual void initSettings( DocumentItem* item );

    private:
        MultiLineEditor* m_editor;
};

//NY22
class PropertyTextLine : public PropertyWidget, private LabelUtils {
    Q_OBJECT
    public:
        PropertyTextLine( TokenProvider* token, QWidget* parent );

        virtual void applySettings( DocumentItem* item, K3MacroCommand* command );
        virtual void initSettings( DocumentItem* item );

    private:
        TextLineEditor* m_editor;
};
//NY22

class PropertySize : public PropertyWidget, private LabelUtils {
    Q_OBJECT
    public:
        PropertySize( QWidget* parent );
        
        virtual void applySettings( DocumentItem* item, K3MacroCommand* command );
        virtual void initSettings( DocumentItem* item );
    
    private slots:
        void enableControls();
        
    private:
        QCheckBox* checkLock;
        KDoubleNumInput* numTop;
        KDoubleNumInput* numLeft;
        KDoubleNumInput* numWidth;
        KDoubleNumInput* numHeight;
};

class PropertyImage : public PropertyWidget {
    Q_OBJECT
    public:
        PropertyImage( TokenProvider* token, QWidget* parent );
        
        virtual void applySettings( DocumentItem* item, K3MacroCommand* command );
        virtual void initSettings( DocumentItem* item );

    private slots:
	void enableControls();
        void slotTokens();

    private:
        TokenProvider* m_token;

        QCheckBox* checkMirrorH;
        QCheckBox* checkMirrorV;
	
	QWidget* imgHBox;

        KComboBox* comboRotation;
	KUrlRequester* imgUrl;
	KLineEdit* imgExpression;
	KPushButton* buttonToken;

	QRadioButton* radioImagePath;
	QRadioButton* radioImageExpression;

        QRadioButton* radioOriginal;
        QRadioButton* radioScaled;
        QRadioButton* radioZoomed;        
};

class PropertyVisible : public PropertyWidget {
    Q_OBJECT
    public:
        PropertyVisible( QWidget* parent );
        
        virtual void applySettings( DocumentItem* item, K3MacroCommand* command );
        virtual void initSettings( DocumentItem* item );

    private:
	KTextEdit* m_script;
};

#endif
