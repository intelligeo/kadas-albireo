/***************************************************************************
 *  qgsmaptoolviewshed.h                                                   *
 *  -------------------                                                    *
 *  begin                : Nov 12, 2015                                    *
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

#ifndef QGSMAPTOOLVIEWSHED_H
#define QGSMAPTOOLVIEWSHED_H

#include "qgsmaptooldrawshape.h"

#include <QDialog>

class QComboBox;
class QDoubleSpinBox;

class APP_EXPORT QgsViewshedDialog : public QDialog
{
    Q_OBJECT
  public:
    enum DisplayMode { DisplayVisibleArea, DisplayInvisibleArea };

    QgsViewshedDialog( double radius, QWidget* parent = 0 );
    double getObserverHeight() const;
    double getTargetHeight() const;
    bool getHeightRelativeToGround() const;
    DisplayMode getDisplayMode() const;

  signals:
    void radiusChanged( double radius );

  private:
    enum HeightMode { HeightRelToGround, HeightRelToSeaLevel };
    QDoubleSpinBox* mSpinBoxObserverHeight;
    QDoubleSpinBox* mSpinBoxTargetHeight;
    QComboBox* mComboHeightMode;
    QComboBox* mDisplayModeCombo;
};

class APP_EXPORT QgsMapToolViewshed : public QgsMapToolDrawCircularSector
{
    Q_OBJECT
  public:
    QgsMapToolViewshed( QgsMapCanvas* mapCanvas );
    void activate();

  private slots:
    void drawFinished();
    void adjustRadius( double newRadius );
};

#endif // QGSMAPTOOLVIEWSHED_H
