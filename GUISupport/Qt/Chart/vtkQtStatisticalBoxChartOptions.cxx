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

/// \file vtkQtStatisticalBoxChartOptions.cxx
/// \date May 15, 2008

#include "vtkQtStatisticalBoxChartOptions.h"


vtkQtStatisticalBoxChartOptions::vtkQtStatisticalBoxChartOptions(QObject *parentObject)
  : QObject(parentObject)
{
  this->AxesCorner = vtkQtChartLayer::BottomLeft;
  this->OutlineType = vtkQtStatisticalBoxChartOptions::Darker;
  this->BoxFraction = (float)0.8;
  //this->BoxFraction = (float)0.4;
}

vtkQtStatisticalBoxChartOptions::vtkQtStatisticalBoxChartOptions(const vtkQtStatisticalBoxChartOptions &other)
  : QObject()
{
  this->AxesCorner = other.AxesCorner;
  this->OutlineType = other.OutlineType;
  this->BoxFraction = other.BoxFraction;
}

void vtkQtStatisticalBoxChartOptions::setAxesCorner(vtkQtChartLayer::AxesCorner axes)
{
  if(this->AxesCorner != axes)
    {
    this->AxesCorner = axes;
    emit this->axesCornerChanged();
    }
}

void vtkQtStatisticalBoxChartOptions::setBoxWidthFraction(float fraction)
{
  if(this->BoxFraction != fraction)
    {
    this->BoxFraction = fraction;
    emit this->boxFractionChanged();
    }
}

void vtkQtStatisticalBoxChartOptions::setOutlineStyle(
    vtkQtStatisticalBoxChartOptions::OutlineStyle style)
{
  if(this->OutlineType != style)
    {
    this->OutlineType = style;
    emit this->outlineStyleChanged();
    }
}

vtkQtStatisticalBoxChartOptions &vtkQtStatisticalBoxChartOptions::operator=(
    const vtkQtStatisticalBoxChartOptions &other)
{
  this->AxesCorner = other.AxesCorner;
  this->OutlineType = other.OutlineType;
  this->BoxFraction = other.BoxFraction;
  return *this;
}


