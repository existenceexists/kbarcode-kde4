
#ifndef TCANVASITEM_H
#define TCANVASITEM_H
#include <qstring.h>
#include <q3canvas.h>

#include "documentitem.h"
#include "referencecounted.h"

class MyCanvasView;
/**
 * Class TCanvasItem
 * Represents a DocumentItem on a QCanvas. 
 * This class is also responsible for drawing 
 * the drag marks provided by SpotProvider.
 *
 * The class is also ReferenceCounted!!!
 */
class TCanvasItem : public Q3CanvasRectangle, public ReferenceCounted {
public:

    TCanvasItem ( MyCanvasView* cv );
    ~TCanvasItem ( );

    void update();

    int rtti() const;

    void setZ( double z );
    void moveBy( double x, double y );
    void setSize( int width, int height );

    void moveMM( int x, int y );
    void moveByMM( int x, int y );
    void setSizeMM( int w, int h );

    void show();
    void hide();
    
    void setItem (DocumentItem* item);
    DocumentItem* item () const;

    MyCanvasView* canvasView() const;

protected:
    /**
     * 
     * @param painter 
     */
    void drawShape ( QPainter & p );

private:
    DocumentItem* m_item;
    MyCanvasView* m_view;
};
#endif //TCANVASITEM_H

