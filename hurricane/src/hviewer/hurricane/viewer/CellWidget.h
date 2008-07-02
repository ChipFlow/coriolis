
// -*- C++ -*-


# ifndef  __CELL_WIDGET_H__
#   define  __CELL_WIDGET_H__

# include  <math.h>

# include  <vector>

# include  <QWidget>
# include  <QPixmap>
# include  <QPainter>
# include  <QRect>
# include  <QPoint>

class QCursor;
class QResizeEvent;
class QMouseEvent;
class QKeyEvent;
class QAction;
class QStatusBar;


# include  "hurricane/Commons.h"
# include  "hurricane/Point.h"
# include  "hurricane/Box.h"
# include  "hurricane/Transformation.h"

# include  "hurricane/viewer/DisplayStyle.h"




namespace Hurricane {

  class BasicLayer;
  class Go;
  class Cell;
  class Instance;
  class Slice;
  class Segment;
  class Contact;
  class Pad;

  class PaletteEntry;
  class Palette;
  class DynamicLabel;
//class MapView;


  class CellWidget : public QWidget {
      Q_OBJECT;

    public:
    // Constructor & Destructor.
                                      CellWidget         ( QWidget* parent=NULL );
      virtual                        ~CellWidget         ();
    // Accessors.
    //        MapView*                getMapView         () { return _mapView; };
              void                    setCell            ( Cell* cell );
      inline  Cell*                   getCell            () const;
      inline  Palette*                getPalette         ();
      inline  QStatusBar*             getStatusBar       ();
      inline  bool                    showBoundaries     () const;
    // Painter control & Hurricane objects drawing primitives.
              void                    drawBoundaries     ( const Cell*    , const Box&, const Transformation& );
              void                    drawBoundaries     ( const Instance*, const Box&, const Transformation& );
              bool                    isDrawable         ( PaletteEntry* entry );
              void                    drawCell           ( const Cell*    , const BasicLayer*, const Box&, const Transformation& );
              void                    drawInstance       ( const Instance*, const BasicLayer*, const Box&, const Transformation& );
              void                    drawSlice          ( const Slice*   , const BasicLayer*, const Box&, const Transformation& );
              void                    drawGo             ( const Go*      , const BasicLayer*, const Box&, const Transformation& );
              void                    drawSegment        ( const Segment* , const BasicLayer*, const Box&, const Transformation& );
              void                    drawContact        ( const Contact* , const BasicLayer*, const Box&, const Transformation& );
              void                    drawPad            ( const Pad*     , const BasicLayer*, const Box&, const Transformation& );
              void                    drawBox            ( const Box& );
              void                    drawLine           ( const Point&, const Point& );
    // Geometric conversions.
              QRect                   dbuToDisplayRect   ( DbU::Unit x1, DbU::Unit y1, DbU::Unit x2, DbU::Unit y2 ) const;
              QRect                   dbuToDisplayRect   ( const Box& box ) const;
              QPoint                  dbuToDisplayPoint  ( DbU::Unit x, DbU::Unit y ) const;
              QPoint                  dbuToDisplayPoint  ( const Point& point ) const;
      inline  int                     dbuToDisplayX      ( DbU::Unit x ) const;
      inline  int                     dbuToDisplayY      ( DbU::Unit y ) const;
      inline  int                     dbuToDisplayLength ( DbU::Unit length ) const;
      inline  DbU::Unit               displayToDbuX      ( int  x ) const;
      inline  DbU::Unit               displayToDbuY      ( int  y ) const;
      inline  DbU::Unit               displayToDbuLength ( int  length ) const;
      inline  Box                     displayToDbuBox    ( const QRect& rect ) const;
      inline  DbU::Unit               screenToDbuX       ( int  x ) const;
      inline  DbU::Unit               screenToDbuY       ( int  y ) const;
    // Qt QWidget Functions Overloads.
              void                    pushCursor         ( Qt::CursorShape cursor );
              void                    popCursor          ();
              QSize                   minimumSizeHint    () const;
              void                    paintEvent         ( QPaintEvent* );
              void                    resizeEvent        ( QResizeEvent* );
              void                    keyPressEvent      ( QKeyEvent* );
              void                    mouseMoveEvent     ( QMouseEvent* );
              void                    mousePressEvent    ( QMouseEvent* );
              void                    mouseReleaseEvent  ( QMouseEvent* );
    public slots:
    // Qt QWidget Slots Overload & CellWidget Specifics.
              void                    redraw             ( QRect redrawArea );
      inline  void                    redraw             ();
              void                    goLeft             ( int dx = 0 );
              void                    goRight            ( int dx = 0 );
              void                    goUp               ( int dy = 0 );
              void                    goDown             ( int dy = 0 );
              void                    fitToContents      ();
              void                    setScale           ( float scale );
              void                    setShowBoundaries  ( bool state );
              void                    reframe            ( const Box& box );
              void                    screenReframe      ();
              void                    shiftLeft          ( int dx );
              void                    shiftRight         ( int dx );
              void                    shiftUp            ( int dy );
              void                    shiftDown          ( int dy );

    protected:
    // Internal: Attributes.
      static  const int               _stripWidth;
              vector<Qt::CursorShape> _cursors;
              QStatusBar*             _statusBar;
    //        MapView*                _mapView;
              Palette*                _palette;
              DynamicLabel*           _xPosition;
              DynamicLabel*           _yPosition;
              Box                     _displayArea;
              Box                     _visibleArea;
              float                   _scale;
              QPoint                  _offsetVA;
              QPixmap                 _drawingBuffer;
              QPainter                _painter;
              QPoint                  _lastMousePosition;
              Cell*                   _cell;
              bool                    _mouseGo;
              bool                    _showBoundaries;
              size_t                  _redrawRectCount;
  };




inline void  CellWidget::redraw ()
{ redraw ( QRect(QPoint(0,0),_drawingBuffer.size()) ); }


inline int  CellWidget::dbuToDisplayX ( DbU::Unit x ) const
{ return (int)rint ( (float)( x - _displayArea.getXMin() ) * _scale ); }


inline int  CellWidget::dbuToDisplayY ( DbU::Unit y ) const
{ return (int)rint ( (float)( _displayArea.getYMax() - y ) * _scale ); }


inline int  CellWidget::dbuToDisplayLength ( DbU::Unit length ) const
{ return (int)rint ( (float)length * _scale ); }


inline DbU::Unit  CellWidget::displayToDbuX ( int x ) const
{ return (DbU::Unit)(x/_scale) + _displayArea.getXMin(); }


inline DbU::Unit  CellWidget::displayToDbuY ( int y ) const
{ return _displayArea.getYMax() - (DbU::Unit)(y/_scale); }


inline DbU::Unit  CellWidget::displayToDbuLength ( int length ) const
{ return (int)( (float)length / _scale ); }


inline Box  CellWidget::displayToDbuBox ( const QRect& rect ) const
{
  return Box ( displayToDbuX(rect.x())
             , displayToDbuY(rect.y())
             , displayToDbuX(rect.x()+rect.width ())
             , displayToDbuY(rect.y()+rect.height())
             );
}


inline DbU::Unit  CellWidget::screenToDbuX ( int x ) const
{ return displayToDbuX(x+_offsetVA.x()); }


inline DbU::Unit  CellWidget::screenToDbuY ( int y ) const
{ return displayToDbuY(y+_offsetVA.y()); }


inline Cell* CellWidget::getCell () const
{ return _cell; }


inline Palette* CellWidget::getPalette ()
{ return _palette; }


inline QStatusBar* CellWidget::getStatusBar ()
{ return _statusBar; }


inline bool  CellWidget::showBoundaries () const
{ return _showBoundaries; }


} // End of Hurricane namespace.


# endif
