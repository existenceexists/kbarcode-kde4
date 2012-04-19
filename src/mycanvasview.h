/***************************************************************************
                          mycanvasview.h  -  description
                             -------------------
    begin                : Die Apr 23 2002
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

#ifndef MYCANVASVIEW_H
#define MYCANVASVIEW_H

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QList>
//Added by qt3to4:
#include <QResizeEvent>
#include <QLabel>
#include <QMouseEvent>
#include "documentitem.h"

class TCanvasItem;

typedef QList<TCanvasItem*> TCanvasItemList;


class QRect;
class QPainter;
class MyCanvas : public QGraphicsScene {
    Q_OBJECT

    public:
        MyCanvas( QObject* parent );
        ~MyCanvas();

        void setRect( QRect r ) {
            m_rect = r;
        }

        QRect rect() const {
            return m_rect;
        }

        void setGrid( bool enabled ) {
                m_grid = enabled;
        }

        bool grid() const { return m_grid; }

    protected:
        /*void drawBackground( QPainter & painter, const QRect & clip );*/// -!F: original, delete
        void drawBackground( QPainter * painter, const QRectF & rect );

    private:
        QRect m_rect;
        bool m_grid;
};

class Definition;
class QColor;
class QLabel;
class QPoint;
class K3CommandHistory;
class K3MacroCommand;
class KRuler;
class KStatusBar;
class QGraphicsItem;
class MyCanvasView : public QGraphicsView
{
    Q_OBJECT

    enum edges {
        TopLeft,
        TopMiddle,
        TopRight,
        RightMiddle,
        LeftMiddle,
        BottomLeft,
        BottomMiddle,
        BottomRight,
        Inside,
        Outside,
        Barcode
    };
    
    public:
        MyCanvasView( Definition* d, MyCanvas *c, QWidget* parent=0, Qt::WFlags f=0);
        ~MyCanvasView();

	/** return a list of all DocumentItems on the canvas
	 */
	DocumentItemList getAllItems();

        TCanvasItemList getSelected();

        TCanvasItem* getActive();
        void setActive( QGraphicsItem* item = 0, bool control = false );

        void setCurrent( QGraphicsItem* item );

        void setHistory( K3CommandHistory* hist ) {
            history = hist;
        }

        QPoint getTranslation() const {
            return translation;
        }

        void setDefinition( Definition* d );
        void setPosLabel( KStatusBar* s, int id ) {
            mouseid = id;
            statusbar = s;
        }

        static int getLowestZ( QList<QGraphicsItem *> list );
        static int getHighestZ( QList<QGraphicsItem *> list );
        
        void snapPoint(QPoint* point, TCanvasItem* item) ;
        
    public slots:
        void selectAll();
        void deSelectAll();
        
        void deleteCurrent();

        void updateGUI() {
            scene()->update();
            repaint();
        }

    protected:
        void mousePressEvent(QMouseEvent*);
        void mouseMoveEvent(QMouseEvent*);
        void mouseReleaseEvent(QMouseEvent *);
        void mouseDoubleClickEvent(QMouseEvent*);
        void resizeEvent( QResizeEvent * r );
        
    signals:
        void selectionChanged();
        void movedSomething();
        void doubleClickedItem( TCanvasItem* );
        void showContextMenu( QPoint );
        
    private:
        void setSelected( QGraphicsItem* item = 0, bool control = false );
        K3MacroCommand* getMoveCommand();
        
        Definition* def;

        KRuler* rulerv;
        KRuler* rulerh;

        K3CommandHistory* history;
        K3MacroCommand* m_commov;
        MyCanvas* canv;

        int mouseid;
        KStatusBar* statusbar;

        QPoint moving_start;
        QPoint translation;

        QRect old;

        bool isInside( QPoint p, QGraphicsItem* item );
        int isEdge(  QPoint p, QGraphicsItem* item  );
        void reposition();
        void updateRuler();
        int updateCursor( QPoint pos, bool pressed = false );
        int m_mode;
        QPoint delta_pt ;
};

#endif
