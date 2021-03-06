/***************************************************************************
 *  qgsmilxmaptools.cpp                                                    *
 *  -------------------                                                    *
 *  begin                : Oct 01, 2015                                    *
 *  copyright            : (C) 2015 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmilxmaptools.h"
#include "qgsmilxannotationitem.h"
#include "qgsmilxlayer.h"
#include "qgsmapcanvas.h"
#include <QMouseEvent>

QgsMilXCreateTool::QgsMilXCreateTool( QgsMapCanvas* canvas, QgsMilXLayer* layer, const QString& symbolXml, const QString &symbolMilitaryName, int nMinPoints, bool hasVariablePoints, const QPixmap& preview )
    : QgsMapTool( canvas ), mSymbolXml( symbolXml ), mSymbolMilitaryName( symbolMilitaryName ), mMinNPoints( nMinPoints ), mNPressedPoints( 0 ), mHasVariablePoints( hasVariablePoints ), mItem( 0 ), mLayer( layer )
{
  setCursor( QCursor( preview, -0.5 * preview.width(), -0.5 * preview.height() ) );
  // If layer is deleted or layers are changed, quit tool
  connect( mLayer, SIGNAL( destroyed( QObject* ) ), this, SLOT( deleteLater() ) );
  connect( canvas, SIGNAL( layersChanged( QStringList ) ), this, SLOT( deleteLater() ) );
}

QgsMilXCreateTool::~QgsMilXCreateTool()
{
  // If an item is still set, it means that positioning was not finished when the tool is disabled -> delete it
  delete mItem.data();
}

void QgsMilXCreateTool::canvasPressEvent( QMouseEvent * e )
{
  if ( e->button() == Qt::LeftButton )
  {
    if ( mItem == 0 )
    {
      // Deselect any previously selected items
      QgsAnnotationItem* selectedItem = mCanvas->selectedAnnotationItem();
      if ( selectedItem )
      {
        selectedItem->setSelected( false );
      }

      mItem = new QgsMilXAnnotationItem( mCanvas );
      mItem->setMapPosition( toMapCoordinates( e->pos() ) );
      mItem->setSymbolXml( mSymbolXml, mSymbolMilitaryName );
      mItem->setSelected( true );
      setCursor( Qt::CrossCursor );
      mNPressedPoints = 1;
      // Only actually add the point if more than the minimum number have been specified
      // The server automatically adds points up to the minimum number
      if ( mNPressedPoints >= mMinNPoints && mHasVariablePoints )
      {
        mItem->appendPoint( e->pos() );
      }
    }
    else if ( mNPressedPoints < mMinNPoints || mHasVariablePoints )
    {
      ++mNPressedPoints;
      // Only actually add the point if more than the minimum number have been specified
      // The server automatically adds points up to the minimum number
      if ( mNPressedPoints >= mMinNPoints && mHasVariablePoints )
      {
        mItem->appendPoint( e->pos() );
      }
    }

    if ( mNPressedPoints >= mMinNPoints && !mHasVariablePoints )
    {
      // Max points reached, stop
      mItem->finalize();
      mLayer->addItem( mItem->toMilxItem() );
      mNPressedPoints = 0;
      // Delay delete until after refresh, to avoid flickering
      connect( mCanvas, SIGNAL( mapCanvasRefreshed() ), this, SLOT( deleteLater() ) );
      mLayer->triggerRepaint();
    }
  }
  else if ( e->button() == Qt::RightButton && mItem != 0 )
  {
    if ( mNPressedPoints + 1 >= mMinNPoints )
    {
      // Done with N point symbol, stop
      mItem->finalize();
      mLayer->addItem( mItem->toMilxItem() );
      // Delay delete until after refresh, to avoid flickering
      connect( mCanvas, SIGNAL( mapCanvasRefreshed() ), this, SLOT( deleteLater() ) );
      mLayer->triggerRepaint();
    }
    else if ( mNPressedPoints + 1 < mMinNPoints )
    {
      // premature stop
      deleteLater();
    }
  }
}

void QgsMilXCreateTool::canvasMoveEvent( QMouseEvent * e )
{
  if ( mItem != 0 && e->buttons() == Qt::NoButton )
  {
    mItem->movePoint( mItem->absolutePointIdx( mNPressedPoints ), e->pos() );
  }
}

void QgsMilXCreateTool::keyReleaseEvent( QKeyEvent *e )
{
  if ( e->key() == Qt::Key_Escape )
  {
    deleteLater(); // quit tool
  }
}

///////////////////////////////////////////////////////////////////////////////

QgsMilXEditTool::QgsMilXEditTool( QgsMapCanvas* canvas, QgsMilXLayer* layer, QgsMilXItem* item )
    : QgsMapToolPan( canvas ), mLayer( layer )
{
  mItem = new QgsMilXAnnotationItem( canvas );
  mItem->fromMilxItem( item );
  mItem->setSelected( true );
  connect( this, SIGNAL( deactivated() ), this, SLOT( deleteLater() ) );
  // If layer is deleted or layers are changed, quit tool
  connect( mLayer, SIGNAL( destroyed( QObject* ) ), this, SLOT( deleteLater() ) );
  connect( canvas, SIGNAL( layersChanged( QStringList ) ), this, SLOT( deleteLater() ) );
}

QgsMilXEditTool::~QgsMilXEditTool()
{
  if ( !mItem.isNull() )
  {
    if ( mLayer )
    {
      mLayer->addItem( mItem->toMilxItem() );
      // Delay delete until after refresh, to avoid flickering
      connect( mCanvas, SIGNAL( mapCanvasRefreshed() ), mItem.data(), SLOT( deleteLater() ) );
      mLayer->triggerRepaint();
    }
    else
    {
      delete mItem.data();
    }
  }
}

void QgsMilXEditTool::canvasReleaseEvent( QMouseEvent * e )
{
  QgsMapToolPan::canvasReleaseEvent( e );
  if ( mCanvas->selectedAnnotationItem() != mItem )
  {
    deleteLater(); // quit tool
  }
}

void QgsMilXEditTool::keyReleaseEvent( QKeyEvent *e )
{
  if ( e->key() == Qt::Key_Escape )
  {
    deleteLater(); // quit tool
  }
}
