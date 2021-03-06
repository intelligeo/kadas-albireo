/**An annotation item can be either placed either on screen corrdinates or on map coordinates.
  It may reference a feature and displays that associatiation with a balloon like appearance*/
class QgsAnnotationItem: QgsMapCanvasItem
{
%TypeHeaderCode
#include <qgsannotationitem.h>
%End

  public:
    enum MouseMoveAction
    {
      NoAction,
      MoveMapPosition,
      MoveFramePosition,
      ResizeFrameUp,
      ResizeFrameDown,
      ResizeFrameLeft,
      ResizeFrameRight,
      ResizeFrameLeftUp,
      ResizeFrameRightUp,
      ResizeFrameLeftDown,
      ResizeFrameRightDown
    };

    QgsAnnotationItem( QgsMapCanvas* mapCanvas );
    virtual ~QgsAnnotationItem();

    void updatePosition();

    QRectF boundingRect() const;

    virtual QSizeF minimumFrameSize() const;

    /**Returns the mouse move behaviour for a given position
      @param pos the position in scene coordinates*/
    int moveActionForPosition( const QPointF& pos ) const;
    /**Returns suitable cursor shape for mouse move action*/
    Qt::CursorShape cursorShapeForAction( int moveAction ) const;

    //setters and getters
    void setMapPositionFixed( bool fixed );
    bool mapPositionFixed() const;

    virtual void setMapPosition( const QgsPoint& pos );
    const QgsPoint& mapPosition() const;
    const QgsPoint& mapGeoPos() const;
    const QgsCoordinateReferenceSystem& mapGeoPosCrs() const;

    void setFrameSize( const QSizeF& size );
    QSizeF frameSize() const;

    void setOffsetFromReferencePoint( const QPointF& offset );
    QPointF offsetFromReferencePoint() const;

    /**Set symbol that is drawn on map position. Takes ownership*/
    void setMarkerSymbol( QgsMarkerSymbolV2* symbol /Transfer/ );
    const QgsMarkerSymbolV2* markerSymbol() const;

    void setFrameBorderWidth( double w );
    double frameBorderWidth() const;

    void setFrameColor( const QColor& c );
    QColor frameColor() const;

    void setFrameBackgroundColor( const QColor& c );
    QColor frameBackgroundColor() const;

    virtual void writeXML( QDomDocument& doc ) const = 0;
    virtual void readXML( const QDomDocument& doc, const QDomElement& itemElem ) = 0;

    void _writeXML( QDomDocument& doc, QDomElement& itemElem ) const;
    void _readXML( const QDomDocument& doc, const QDomElement& annotationElem );

    void showItemEditor();
    virtual void showContextMenu( const QPoint& screenPos );

  protected:
    void updateBoundingRect();
    /**Check where to attach the balloon connection between frame and map point*/
    void updateBalloon();

    void drawFrame( QPainter* p );
    void drawMarkerSymbol( QPainter* p );
    void drawSelectionBoxes( QPainter* p );
    /**Returns frame width in painter units*/
    //double scaledFrameWidth( QPainter* p) const;
    /**Gets the frame line (0 is the top line, 1 right, 2 bottom, 3 left)*/
    QLineF segment( int index );
    /**Returns a point on the line from startPoint to directionPoint that is a certain distance away from the starting point*/
    QPointF pointOnLineWithDistance( const QPointF& startPoint, const QPointF& directionPoint, double distance ) const;
    /**Returns the symbol size scaled in (mapcanvas) pixels. Used for the counding rect calculation*/
    double scaledSymbolSize() const;

  private:
    virtual void _showItemEditor() = 0;
};
