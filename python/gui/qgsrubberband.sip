class QgsRubberBand: QgsMapCanvasItem
{
%TypeHeaderCode
#include <qgsrubberband.h>
%End

  public:
    /** Icons */
    enum IconType
    {
      /**
       * No icon is used
       */
      ICON_NONE,
      /**
       * A cross is used to highlight points (+)
       */
      ICON_CROSS,
      /**
       * A cross is used to highlight points (x)
       */
      ICON_X,
      /**
       * A box is used to highlight points (□)
       */
      ICON_BOX,
      /**
       * A circle is used to highlight points (○)
       */
      ICON_CIRCLE,
      /**
       * A full box is used to highlight points (■)
       */
      ICON_FULL_BOX
    };

    /**
     * Creates a new RubberBand.
     *  @param mapCanvas The map canvas to draw onto. It's CRS will be used map points onto screen coordinates.
     *  @param geometryType Defines how the data should be drawn onto the screen. (Use QGis::Line, QGis::Polygon or QGis::Point)
     */
    QgsRubberBand( QgsMapCanvas* mapCanvas /TransferThis/, QGis::GeometryType geometryType = QGis::Line );
    /**
     * Creates a new RubberBand.
     *  @deprecated Use the constructor which takes QGis::GeometryType as second argument instead
     *  @param mapCanvas The map canvas to draw onto. It's CRS will be used map points onto screen coordinates.
     *  @param isPolygon true: draw as (multi-)polygon, false draw as (multi-)linestring
     */
    QgsRubberBand( QgsMapCanvas* mapCanvas /TransferThis/, bool isPolygon ) /Deprecated/;
    ~QgsRubberBand();

    /**
     * Set the color for the rubberband
     *  @param color  The color used to render this rubberband
     */
    void setColor( const QColor & color );

    /**
     * Set the fill color for the rubberband
     *  @param color  The color used to render this rubberband
     *  @note Added in 2.6
     */
    void setFillColor( const QColor & color );

    /**
     * Set the border color for the rubberband
     *  @param color  The color used to render this rubberband
     *  @note Added in 2.6
     */
    void setBorderColor( const QColor & color );

    /**
     * Set the width of the line. Outline width for polygon.
     *  @param width The width for any lines painted for this rubberband
     */
    void setWidth( int width );

    /**
     * Set the icon type to highlight point geometries.
     *  @param icon The icon to visualize point geometries
     */
    void setIcon( IconType icon );

    /**
     * Set the size of the point icons
     */
    void setIconSize( int iconSize );

    /**
    * Set the style of the line
    */
    void setLineStyle( Qt::PenStyle penStyle );

    /**
    * Set the style of the brush
    */
    void setBrushStyle( Qt::BrushStyle brushStyle );

    /**
     * Clears all the geometries in this rubberband.
     * Sets the representation type according to geometryType.
     *  @param geometryType Defines how the data should be drawn onto the screen. (Use QGis::Line, QGis::Polygon or QGis::Point)
     */
    void reset( QGis::GeometryType geometryType = QGis::Line );

    /**
     * @deprecated Use the reset method which takes QGis::GeometryType as second argument instead
     * Clears all the geometries in this rubberband.
     * Sets the representation type according to isPolygon.
     *  @param isPolygon true: draw as (multi-)polygon, false draw as (multi-)linestring
     */
    void reset( bool isPolygon );

    /**
     * Add a vertex to the rubberband and update canvas.
     * The rendering of the vertex depends on the current GeometryType and icon.
     * If adding more points consider using update=false for better performance
     *  @param p             The vertex/point to add
     *  @param doUpdate      Should the map canvas be updated immediately?
     *  @param geometryIndex The index of the feature part (in case of multipart geometries)
     */
    void addPoint( const QgsPoint & p, bool doUpdate = true, int geometryIndex = 0 );

    /**
    * Remove a vertex from the rubberband and (optionally) update canvas.
    * @param index The index of the vertex/point to remove, negative indexes start at end
    * @param doUpdate Should the map canvas be updated immediately?
    * @param geometryIndex The index of the feature part (in case of multipart geometries)
    */
    void removePoint( int index = 0, bool doUpdate = true, int geometryIndex = 0 );

    /**
     * Removes the last point. Most useful in connection with undo operations
     */
    void removeLastPoint( int geometryIndex = 0, bool doUpdate = true );

    /**
    * Remove the last geometry part
    * @param doUpdate Should the map canvas be updated immediately?
    */
    void removeLastPart(bool doUpdate = true);

    /**
     * Moves the rubber band point specified by index. Note that if the rubber band is
     * not used to track the last mouse position, the first point of the rubber band has two vertices
     */
    void movePoint( const QgsPoint & p, int geometryIndex = 0, bool doUpdate = true );

    /**
     * Moves the rubber band point specified by index. Note that if the rubber band is
     * not used to track the last mouse position, the first point of the rubber band has two vertices
     */
    void movePoint( int index, const QgsPoint& p, int geometryIndex = 0, bool doUpdate = true );

    /**
     * Returns number of vertices in feature part
     *  @param geometryIndex The index of the feature part (in case of multipart geometries)
     *  @return number of vertices
     */
    int partSize( int geometryIndex ) const;

    /**
     * Sets this rubber band to the geometry of an existing feature.
     * This is useful for feature highlighting.
     * In contrast to {@link addGeometry}, this method does also change the geometry type of the rubberband.
     *  @param geom the geometry object
     *  @param layer the layer containing the feature, used for coord transformation to map
     *               crs. In case of 0 pointer, the coordinates are not going to be transformed.
     */
    void setToGeometry( QgsGeometry* geom, QgsVectorLayer* layer );

    /**
     * Sets this rubber band to a map canvas rectangle
     *  @param rect rectangle in canvas coordinates
     */
    void setToCanvasRectangle( const QRect& rect );

    /**
     * Add the geometry of an existing feature to a rubberband
     * This is useful for multi feature highlighting.
     * As of 2.0, this method does not change the GeometryType any more. You need to set the GeometryType
     * of the rubberband explicitly by calling {@link reset} or {@link setToGeometry} with appropriate arguments.
     * {@link setToGeometry} is also to be preferred for backwards-compatibility.
     *
     *  @param geom the geometry object. Will be treated as a collection of vertices.
     *  @param layer the layer containing the feature, used for coord transformation to map
     *               crs. In case of 0 pointer, the coordinates are not going to be transformed.
     */
    void addGeometry( QgsGeometry* geom, QgsVectorLayer* layer );

    /**
     * Adds translation to original coordinates (all in map coordinates)
     *  @param dx  x translation
     *  @param dy  y translation
     */
    void setTranslationOffset( double dx, double dy );

    /*
     * @brief Returns the translation offset
     * @param dx Output parameter for the x translation
     * @param dy Output parameter for the y translation
     */
    void translationOffset(double& dx, double& dy) const;

    /**
     * Returns number of geometries
     *  @return number of geometries
     */
    int size() const;

    /**
     * Returns count of vertices in all lists of mPoint
     *  @return The total number of vertices
     */
    int numberOfVertices() const;

    /**
     * Return vertex
     *  @param i   The geometry index
     *  @param j   The vertex index within geometry i
     */
    const QgsPoint *getPoint( int i, int j = 0 ) const;

    /**
     * Return current point list
     * @return The current point list
     */
    const QList< QList <QgsPoint> >& getPoints() const;

    /**
     * Set the point list
     */
    void setPoints(const QList< QList <QgsPoint> >& points, bool doUpdate = true);

    /**
     * Returns the rubberband as a Geometry.
     *  @return A geometry object which reflects the current state of the rubberband.
     */
    QgsGeometry* asGeometry();

    virtual void updatePosition();

  protected:
    virtual void paint( QPainter* p );

    //! recalculates needed rectangle
    void updateRect();

};
