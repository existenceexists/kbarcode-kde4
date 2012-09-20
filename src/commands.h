/***************************************************************************
                          commands.h  -  description
                             -------------------
    begin                : Don Dez 19 2002
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

#ifndef COMMANDS_H
#define COMMANDS_H

#include "mycanvasitem.h"

#include <qobject.h>
#include <QUndoCommand>
//Added by qt3to4:
#include <QPixmap>

#include <klocale.h>

#include "barcodeitem.h"
#include "tcanvasitem.h"
#include "imageitem.h"

struct barcodeData;
class RectItem;
class MyCanvasLine;
class MyCanvasRectangle;
class MyCanvasView;
class CanvasBarcode;
class PictureRectangle;
class QGraphicsScene;
class QGraphicsItem;
class QColor;
class QFont;
class QImage;
class QPen;
class QPixmap;
class QPoint;
class QString;
class TextItem;
//NY29
class TextLineItem;
//NY29
class TokenProvider;

// Stuff for undo redo

class CommandUtils : public QObject {
    Q_OBJECT
    public:
        CommandUtils(TCanvasItem* item);
        ~CommandUtils();

        bool canvasHasItem();

    private slots:
        void documentItemDeleted();
        
    protected:
        QGraphicsScene* c;
        TCanvasItem* m_canvas_item;
};

/**
  * NewItemCommand is the base class for all classes
  * that create a new item in the label editor (e.g. a
  * barcode or a text element).
  * You have to implement void create() which creates
  * a QCanvasItem in item.
  * NewItemCommand takes care about possitioning and
  * undo/redo (because of KCommand).
  *
  * @author Dominik Seichter
  */
class NewItemCommand : public QObject, public QUndoCommand {
    Q_OBJECT
    public:
        NewItemCommand( MyCanvasView* view, const QString & name, QUndoCommand* parent = 0 );
        virtual ~NewItemCommand();

        void redo();
        void undo();
        QString name() const {
            return m_name;
        };

        /** @returns a pointer to the TCanvasItem created by this class 
	  */
        inline TCanvasItem* createdItem() const { return m_item; }
    
    private slots:
        void documentItemDeleted();
        
    protected:
        /** This function has to be reimplemented in all subclasses
          * and has to create a QCanvasItem and store a pointer to it
          * in item. Otherwise KBarcode is going to crash.
          *
          * Example:
          *
          * <pre>
          * void create() {
          *     DrawingRect* r = new DrawingRect( 0 ); // Drawing rect is a subclass of QCanvasItem
          *     r->setCircle( m_circle );   // Do something with r
          *     item = r;   // save r into item, so that NewItemCommand knows about it
          * }
          * </pre>
          */
        virtual void create() = 0;
        
        MyCanvasView* cv;
        TCanvasItem* m_item;
        DocumentItem* m_object;
        QPoint m_point;
        QString m_name;    
};

class ResizeCommand : public QUndoCommand, CommandUtils {
    public:
        ResizeCommand( TCanvasItem* it, bool shift = false, int id = -1, QUndoCommand* parent = 0 ) 
            : QUndoCommand( parent ), CommandUtils( it )
        {
	    orect = rect = m_canvas_item->item()->rectMM();
	    m_shift = shift;
            m_id = id;
            setText( name() + "\n" + name() );
        }
        ~ResizeCommand() {}

        void setRect( int cx, int cy, int cw, int ch );
        
        void redo();
        void undo();
        QString name() const {
            return i18n("Resized Item");
        }
        int id() const {
            return m_id;
        }
        bool mergeWith( const QUndoCommand* other );

    protected:
        QRect orect;
        QRect rect;
        bool m_shift;
        int m_id;
};

/** Move a TCanvasItem on the canvas
 */
class MoveCommand : public QUndoCommand, CommandUtils {
    public:
        /**
         * @param cx move in x direction cx mm
         * @param cy move in y direction cy mm
         */
        MoveCommand( int cx, int cy, TCanvasItem* it, int id = -1, QUndoCommand* parent = 0 )
            : QUndoCommand( parent ), CommandUtils( it )
        {
            x = cx;
            y = cy;
            m_id = id;
            setText( name() + "\n" + name() );
        }
        ~MoveCommand() {}

        void redo();
        void undo();
        QString name() const {
            return i18n("Moved Item");
        }
        int id() const {
            return m_id;
        }
        bool mergeWith( const QUndoCommand* other );

    protected:
        int x;
        int y;
        int m_id;
};

class ChangeZCommand : public QUndoCommand, CommandUtils {
    public:
        ChangeZCommand( int z, TCanvasItem* it );
        
        void redo();
        void undo();
        QString name() const {
            return i18n("Raised or lowered an item");
        }

    protected:
        int m_z, m_oldz;
};

class LockCommand : public QUndoCommand, CommandUtils {
    public:
        LockCommand( bool lock, TCanvasItem* it, QUndoCommand* parent = 0 )
            : QUndoCommand( parent ), CommandUtils( it )
        {
            m_locked = lock;
            setText( name() + "\n" + name() );
        }
        
        void redo();
        void undo();
        QString name() const {
            return i18n("Protected Item");
        }

    protected:
        bool m_locked;
};

class PictureCommand : public QUndoCommand, CommandUtils {
    public:
        PictureCommand( double r, bool mirrorh, bool mirrorv, EImageScaling s, ImageItem* it, QUndoCommand* parent = 0 );
        ~PictureCommand() {}

	void setExpression( const QString & expr );
	void setPixmap( const QPixmap & pix );

        void redo();
        void undo();
        QString name() const {
            return i18n("Changed Settings");
        }
    protected:
        double rotate, orotate;
        bool mirrorv, omirrorv;
        bool mirrorh, omirrorh;
	QString expression, oexpression;
        EImageScaling scaling, oscaling;
	QPixmap pixmap, opixmap;
	QSize oldsize;
	int pixserial, opixserial;
        
        ImageItem* m_item;
};

class TextChangeCommand : public QUndoCommand, CommandUtils {
    public:
        TextChangeCommand( TextItem* it, QString t, QUndoCommand* parent = 0 );
        ~TextChangeCommand() { }

        void redo();
        void undo();
        QString name() const {
            return i18n("Changed Text");
        }
    protected:
        QString oldtext, text;
        TextItem* m_item;
};

class TextRotationCommand : public QUndoCommand, protected CommandUtils {
    public:
        TextRotationCommand( double rot, TextItem* t, QUndoCommand* parent = 0 );

        void redo();
        void undo();
        QString name() const {
            return i18n("Rotated Text");
        }

    protected:
        double rot1, rot2;
        TextItem* m_item;
};


//NY28
class TextLineChangeCommand : public QUndoCommand, CommandUtils {
    public:
        TextLineChangeCommand( TextLineItem* it, QString t, int font, int magvert, int maghor, QUndoCommand* parent = 0 );
        ~TextLineChangeCommand() { }

        void redo();
        void undo();
        QString name() const {
            return i18n("Changed Text");
        }
    protected:
        QString oldtext, text;
        TextLineItem* m_item;
        int m_font;
        int m_mag_vert;
        int m_mag_hor;
};
//NY28

class BarcodeCommand : public QUndoCommand, CommandUtils {
    public:
        BarcodeCommand( BarcodeItem* bcode, Barkode* d, QUndoCommand* parent = 0 );
        ~BarcodeCommand() {
            delete data;
        }

        void redo();
        void undo();
        QString name() const {
            return i18n("Changed Barcode");
        }
    protected:
        Barkode olddata;
        Barkode* data;
        BarcodeItem* m_item;
};

class NewPictureCommand : public NewItemCommand {
    public:
        NewPictureCommand( MyCanvasView* v )
	    : NewItemCommand( v, i18n("New Picture") + "\n" + i18n("New Picture") )
	    {
	    }

    protected:
        void create();
};

class NewTextCommand : public NewItemCommand {
    public:
        NewTextCommand( QString t, MyCanvasView* v, TokenProvider* token );

    protected:
        void create();
        QString text;
        TokenProvider* m_token;
};

//NY27
class NewTextLineCommand : public NewItemCommand {
    public:
        NewTextLineCommand( QString t, MyCanvasView* v, TokenProvider* token );

    protected:
        void create();
        QString text;
	TokenProvider* m_token;
};
//NY27

class NewRectCommand : public NewItemCommand {
    public:
        NewRectCommand( MyCanvasView* v, bool circle = false );

    protected:
        void create();
        bool m_circle;
};

class NewLineCommand : public NewItemCommand {
    public:
        NewLineCommand( MyCanvasView* v );

    protected:
        void create();
};

class NewBarcodeCommand : public NewItemCommand {
    public:
        NewBarcodeCommand( MyCanvasView* v, TokenProvider* token );

    protected:
        void create();

    private:
	TokenProvider* m_token;
};

class DeleteCommand : public QUndoCommand, CommandUtils {
    public:
        DeleteCommand( TCanvasItem* it, QUndoCommand* parent = 0 ) 
            : QUndoCommand( parent ), CommandUtils( it )
        {
            setText( name() + "\n" + name() );
        }
        ~DeleteCommand();
        
        void redo();
        void undo();
        QString name() const {
            return i18n("Delete Item");
        }
};

class BorderCommand : public QUndoCommand, protected CommandUtils {
    public:
        BorderCommand( bool border, const QPen & pen, DocumentItem* item, QUndoCommand* parent = 0 );
        
        void redo();
        void undo();
        QString name() const {
            return i18n("Modified Border");
        }
    
    protected:
        bool m_new_border;
        bool m_old_border;
        QPen m_new_pen;
        QPen m_old_pen;
        
        DocumentItem* m_item;
};

class FillCommand : public QUndoCommand, protected CommandUtils {
    public:
        FillCommand(  QColor c, RectItem* r, QUndoCommand* parent = 0  );

        void redo();
        void undo();
        QString name() const {
            return i18n("Modified Rectangle or Ellipse");
        }

    protected:
        QColor fill, fill2;
        RectItem* m_item;
};

class ScriptCommand : public QUndoCommand, CommandUtils {
    public:
        ScriptCommand( const QString & script, TCanvasItem* it, QUndoCommand* parent = 0 )
            : QUndoCommand( parent ), CommandUtils( it )
        {
            m_script = script;
            setText( name() + "\n" + name() );
        }
        ~ScriptCommand() {}
        
        void redo();
        void undo();
        QString name() const {
            return i18n("Changed visibility JavaScript");
        }

    protected:
        QString m_script;
	QString m_old_script;
};

#endif
