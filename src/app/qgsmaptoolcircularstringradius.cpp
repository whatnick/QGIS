/***************************************************************************
    qgsmaptoolcircularstringradius.h  -  map tool for adding circular strings
    ---------------------
    begin                : Feb 2015
    copyright            : (C) 2015 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolcircularstringradius.h"
#include "qgisapp.h"
#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgsgeometryutils.h"
#include "qgsgeometryrubberband.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgsstatusbar.h"
#include <QDoubleSpinBox>
#include <QMouseEvent>
#include <cmath>

QgsMapToolCircularStringRadius::QgsMapToolCircularStringRadius( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolAddCircularString( parentTool, canvas, mode )
  , mTemporaryEndPoint( QgsPoint() )
  , mRadius( 0.0 )

{

}

void QgsMapToolCircularStringRadius::deactivate()
{
  deleteRadiusSpinBox();
  QgsMapToolAddCircularString::deactivate();
}

void QgsMapToolCircularStringRadius::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  QgsPoint mapPoint( e->mapPoint() );

  if ( e->button() == Qt::LeftButton )
  {
    if ( mPoints.isEmpty() )
    {
      mPoints.append( mapPoint );
    }
    else
    {
      if ( mPoints.size() % 2 )
      {
        mTemporaryEndPoint = mapPoint;

        //initial radius is distance( tempPoint - mPoints.last ) / 2.0
        double minRadius = std::sqrt( QgsGeometryUtils::sqrDistance2D( mPoints.last(), mTemporaryEndPoint ) ) / 2.0;
        mRadius = minRadius + minRadius / 10.0;

        QgsPoint result;
        if ( QgsGeometryUtils::segmentMidPoint( mPoints.last(), mTemporaryEndPoint, result, mRadius, QgsPoint( mapPoint.x(), mapPoint.y() ) ) )
        {
          mPoints.append( result );
          createRadiusSpinBox();
          if ( mRadiusSpinBox )
          {
            mRadiusSpinBox->setMinimum( minRadius );
          }
        }
      }
      else
      {
        mPoints.append( mTemporaryEndPoint );
        deleteRadiusSpinBox();
      }
      recalculateRubberBand();
      recalculateTempRubberBand( e->mapPoint() );
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    if ( !( mPoints.size() % 2 ) )
      mPoints.removeLast();
    deactivate();
    if ( mParentTool )
    {
      mParentTool->canvasReleaseEvent( e );
    }
  }
}

void QgsMapToolCircularStringRadius::cadCanvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( !mPoints.isEmpty() )
  {
    recalculateTempRubberBand( e->mapPoint() );
    updateCenterPointRubberBand( mTemporaryEndPoint );
  }
}

void QgsMapToolCircularStringRadius::recalculateRubberBand()
{
  if ( mPoints.size() >= 3 )
  {
    QgsCircularString *cString = new QgsCircularString();
    int rubberBandSize = mPoints.size() - ( mPoints.size() + 1 ) % 2;
    cString->setPoints( mPoints.mid( 0, rubberBandSize ) );
    delete mRubberBand;
    mRubberBand = createGeometryRubberBand( ( mode() == CapturePolygon ) ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry );
    mRubberBand->setGeometry( cString );
    mRubberBand->show();
  }
}

void QgsMapToolCircularStringRadius::recalculateTempRubberBand( const QgsPointXY &mousePosition )
{
  QgsPointSequence rubberBandPoints;
  if ( !( mPoints.size() % 2 ) )
  {
    //recalculate midpoint on circle segment
    QgsPoint midPoint;
    if ( !QgsGeometryUtils::segmentMidPoint( mPoints.at( mPoints.size() - 2 ), mTemporaryEndPoint, midPoint, mRadius,
         QgsPoint( mousePosition ) ) )
    {
      return;
    }
    mPoints.replace( mPoints.size() - 1, midPoint );
    rubberBandPoints.append( mPoints.at( mPoints.size() - 2 ) );
    rubberBandPoints.append( mPoints.last() );
    rubberBandPoints.append( mTemporaryEndPoint );
  }
  else
  {
    rubberBandPoints.append( mPoints.last() );
    rubberBandPoints.append( QgsPoint( mousePosition ) );
  }
  QgsCircularString *cString = new QgsCircularString();
  cString->setPoints( rubberBandPoints );
  delete mTempRubberBand;
  mTempRubberBand = createGeometryRubberBand( ( mode() == CapturePolygon ) ? QgsWkbTypes::PolygonGeometry : QgsWkbTypes::LineGeometry, true );
  mTempRubberBand->setGeometry( cString );
  mTempRubberBand->show();
}

void QgsMapToolCircularStringRadius::createRadiusSpinBox()
{
  deleteRadiusSpinBox();
  mRadiusSpinBox = new QDoubleSpinBox();
  mRadiusSpinBox->setMaximum( 99999999 );
  mRadiusSpinBox->setDecimals( 2 );
  mRadiusSpinBox->setPrefix( tr( "Radius: " ) );
  mRadiusSpinBox->setValue( mRadius );
  QgisApp::instance()->addUserInputWidget( mRadiusSpinBox );
  connect( mRadiusSpinBox, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsMapToolCircularStringRadius::updateRadiusFromSpinBox );
  mRadiusSpinBox->setFocus( Qt::TabFocusReason );
}

void QgsMapToolCircularStringRadius::deleteRadiusSpinBox()
{
  if ( mRadiusSpinBox )
  {
    QgisApp::instance()->statusBarIface()->removeWidget( mRadiusSpinBox );
    delete mRadiusSpinBox;
    mRadiusSpinBox = nullptr;
  }
}

void QgsMapToolCircularStringRadius::updateRadiusFromSpinBox( double radius )
{
  mRadius = radius;
  recalculateTempRubberBand( toMapCoordinates( mCanvas->mouseLastXY() ).toQPointF() );
}
