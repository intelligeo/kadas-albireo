/***************************************************************************
                              qgsmaptoolsvgannotation.cpp
                              ---------------------------
  begin                : November, 2012
  copyright            : (C) 2012 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolsvgannotation.h"
#include "qgssvgannotationitem.h"
#include <QMouseEvent>


QgsAnnotationItem* QgsMapToolSvgAnnotation::createItem( const QPoint &pos )
{
  QgsSvgAnnotationItem* svgItem = new QgsSvgAnnotationItem( mCanvas );
  svgItem->setMapPosition( toMapCoordinates( pos ) );
  svgItem->setSelected( true );
  svgItem->setFrameSize( QSizeF( 200, 100 ) );
  return svgItem;
}
