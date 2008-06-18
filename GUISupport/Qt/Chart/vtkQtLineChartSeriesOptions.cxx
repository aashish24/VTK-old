/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/

/// \file vtkQtLineChartSeriesOptions.cxx
/// \date February 15, 2008

#include "vtkQtLineChartSeriesOptions.h"

#include "vtkQtChartStyleGenerator.h"
#include <QBrush>


vtkQtLineChartSeriesOptions::vtkQtLineChartSeriesOptions(QObject *parentObject)
  : vtkQtChartSeriesOptions(parentObject)
{
  this->AxesCorner = vtkQtChartLayer::BottomLeft;
  this->PointStyle = vtkQtPointMarker::Circle;
  this->PointSize = new QSizeF(5.0, 5.0);
  this->ShowPoints = false;

  this->setBrush(Qt::white);
}

vtkQtLineChartSeriesOptions::~vtkQtLineChartSeriesOptions()
{
  delete this->PointSize;
}

void vtkQtLineChartSeriesOptions::setStyle(int style,
    vtkQtChartStyleGenerator *generator)
{
  vtkQtChartSeriesOptions::setStyle(style, generator);
  if(generator)
    {
    this->setPen(generator->getSeriesPen(style));
    }
}

vtkQtChartLayer::AxesCorner vtkQtLineChartSeriesOptions::getAxesCorner() const
{
  return this->AxesCorner;
}

void vtkQtLineChartSeriesOptions::setAxesCorner(
    vtkQtChartLayer::AxesCorner axes)
{
  if(axes != this->AxesCorner)
    {
    vtkQtChartLayer::AxesCorner previous = this->AxesCorner;
    this->AxesCorner = axes;
    emit this->axesCornerChanged(axes, previous);
    }
}

void vtkQtLineChartSeriesOptions::setPointsVisible(bool visible)
{
  if(this->ShowPoints != visible)
    {
    this->ShowPoints = visible;
    emit this->pointVisibilityChanged(visible);
    }
}

vtkQtPointMarker::MarkerStyle
vtkQtLineChartSeriesOptions::getMarkerStyle() const
{
  return this->PointStyle;
}

void vtkQtLineChartSeriesOptions::setMarkerStyle(
    vtkQtPointMarker::MarkerStyle style)
{
  if(style != this->PointStyle)
    {
    this->PointStyle = style;
    emit this->pointMarkerChanged();
    }
}

const QSizeF &vtkQtLineChartSeriesOptions::getMarkerSize() const
{
  return *this->PointSize;
}

void vtkQtLineChartSeriesOptions::setMarkerSize(const QSizeF &size)
{
  if(size != *this->PointSize)
    {
    *this->PointSize = size;
    emit this->pointMarkerChanged();
    }
}


