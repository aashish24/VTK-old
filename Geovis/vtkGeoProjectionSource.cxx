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
#include "vtkGeoProjectionSource.h"

#include "vtkActor.h"
#include "vtkBox.h"
#include "vtkCellArray.h"
#include "vtkClipPolyData.h"
#include "vtkDoubleArray.h"
#include "vtkGeoGraticule.h"
#include "vtkGeoProjection.h"
#include "vtkGeoTerrainNode.h"
#include "vtkGeoTransform.h"
#include "vtkImageData.h"
#include "vtkMath.h"
#include "vtkMutexLock.h"
#include "vtkObjectFactory.h"
#include "vtkPainter.h"
#include "vtkPainterPolyDataMapper.h"
#include "vtkPlane.h"
#include "vtkPlanes.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkPropCollection.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"
#include "vtkTransformFilter.h"

#include <vtksys/stl/stack>
#include <vtksys/stl/utility>

vtkStandardNewMacro(vtkGeoProjectionSource);
vtkCxxRevisionMacro(vtkGeoProjectionSource, "$Revision$");
vtkCxxSetObjectMacro(vtkGeoProjectionSource, Transform, vtkTransformFilter);
//----------------------------------------------------------------------------
vtkGeoProjectionSource::vtkGeoProjectionSource()
{
  this->Transform = 0;
  this->MinCellsPerNode = 20;

  this->TransformLock = vtkMutexLock::New();
}

//----------------------------------------------------------------------------
vtkGeoProjectionSource::~vtkGeoProjectionSource()
{
  this->TransformLock->Delete();
  this->SetTransform(0);
}

//----------------------------------------------------------------------------
void vtkGeoProjectionSource::RefineAndComputeError(vtkGeoTerrainNode* node)
{
  double* latRange = node->GetLatitudeRange();
  double* lonRange = node->GetLongitudeRange();

  int level = node->GetGraticuleLevel();
  double latDelta = vtkGeoGraticule::GetLatitudeDelta(level);
  double lonDelta = vtkGeoGraticule::GetLongitudeDelta(level);
  while ((latRange[1] - latRange[0])*(lonRange[1] - lonRange[0])/(latDelta*lonDelta) < this->MinCellsPerNode)
    {
    ++level;
    latDelta = vtkGeoGraticule::GetLatitudeDelta(level);
    lonDelta = vtkGeoGraticule::GetLongitudeDelta(level);
    }

  vtkSmartPointer<vtkGeoGraticule> grat = vtkSmartPointer<vtkGeoGraticule>::New();
  vtkSmartPointer<vtkGeoGraticule> refinedGrat = vtkSmartPointer<vtkGeoGraticule>::New();
  grat->SetGeometryType(vtkGeoGraticule::QUADRILATERALS);
  grat->SetLatitudeBounds(latRange);
  grat->SetLongitudeBounds(lonRange);
  refinedGrat->SetGeometryType(vtkGeoGraticule::QUADRILATERALS);

  vtkSmartPointer<vtkPolyData> geom = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPolyData> refined = vtkSmartPointer<vtkPolyData>::New();
  do
    {
    grat->SetLatitudeLevel(level);
    grat->SetLongitudeLevel(level);
    this->TransformLock->Lock();
    this->Transform->SetInputConnection(grat->GetOutputPort());
    this->Transform->Update();
    geom->ShallowCopy(this->Transform->GetOutput());
    this->TransformLock->Unlock();
    refinedGrat->SetLatitudeLevel(level+1);
    refinedGrat->SetLongitudeLevel(level+1);
    double* curLatRange = geom->GetPointData()->GetArray("LatLong")->GetRange(0);
    refinedGrat->SetLatitudeBounds(curLatRange);
    double* curLonRange = geom->GetPointData()->GetArray("LatLong")->GetRange(1);
    refinedGrat->SetLongitudeBounds(curLonRange);
    this->TransformLock->Lock();
    this->Transform->SetInputConnection(refinedGrat->GetOutputPort());
    this->Transform->Update();
    refined->ShallowCopy(this->Transform->GetOutput());
    this->TransformLock->Unlock();
    ++level;
    }
  while (geom->GetNumberOfCells() < this->MinCellsPerNode);
  node->SetGraticuleLevel(level);

  // Compute grid size
  vtkDataArray* latLonArr = geom->GetPointData()->GetArray("LatLong");
  double firstLon = latLonArr->GetComponent(0, 1);
  vtkIdType gridSize[2] = {1, 0};
  while (latLonArr->GetComponent(gridSize[0], 1) != firstLon)
    {
    gridSize[0]++;
    }
  gridSize[1] = geom->GetNumberOfPoints() / gridSize[0];

  // Compute refined grid size
  latLonArr = refined->GetPointData()->GetArray("LatLong");
  firstLon = latLonArr->GetComponent(0, 1);
  vtkIdType rgridSize[2] = {1, 0};
  while (latLonArr->GetComponent(rgridSize[0], 1) != firstLon)
    {
    rgridSize[0]++;
    }
  rgridSize[1] = refined->GetNumberOfPoints() / rgridSize[0];

  // Calculate error.
  double error = 0.0;
  vtkIdType skip = (rgridSize[0]-1) / (gridSize[0]-1);
  for (vtkIdType latInd = 0; latInd < rgridSize[1]-skip; ++latInd)
    {
    for (vtkIdType lonInd = 0; lonInd < rgridSize[0]-skip; ++lonInd)
      {
      vtkIdType ind00 =   latInd          * rgridSize[0] + lonInd;
      vtkIdType ind01 =   latInd          * rgridSize[0] + lonInd + skip;
      vtkIdType ind11 = ( latInd + skip ) * rgridSize[0] + lonInd + skip;
      vtkIdType ind10 = ( latInd + skip ) * rgridSize[0] + lonInd;
      double pt00[3];
      double pt01[3];
      double pt11[3];
      double pt10[3];
      refined->GetPoint(ind00, pt00);
      refined->GetPoint(ind01, pt01);
      refined->GetPoint(ind11, pt11);
      refined->GetPoint(ind10, pt10);
      for (vtkIdType rlatInd = latInd + 1; rlatInd < latInd + skip; ++rlatInd)
        {
        double latFrac = static_cast<double>(rlatInd - latInd) / skip;
        for (vtkIdType rlonInd = lonInd + 1; rlonInd < lonInd + skip; ++rlonInd)
          {
          double lonFrac = static_cast<double>(rlonInd - lonInd) / skip;
          double curPt[3];
          refined->GetPoint(rlatInd * rgridSize[0] + rlonInd, curPt);
          double interpPt[3];
          for (int c = 0; c < 3; ++c)
            {
            double interpLon0 = (1.0 - lonFrac)*pt00[c] + lonFrac*pt01[c];
            double interpLon1 = (1.0 - lonFrac)*pt10[c] + lonFrac*pt11[c];
            interpPt[c] = (1.0 - latFrac)*interpLon0 + latFrac*interpLon1;
            }
          double curError = vtkMath::Distance2BetweenPoints(curPt, interpPt);
          if (curError > error)
            {
            error = curError;
            }
          }
        }
      }
    }

  node->GetModel()->ShallowCopy(geom);
  node->SetError(sqrt(error));
}

//----------------------------------------------------------------------------
bool vtkGeoProjectionSource::FetchRoot(vtkGeoTreeNode* r)
{
  vtkGeoTerrainNode* root = 0;
  if (!(root = vtkGeoTerrainNode::SafeDownCast(r)))
    {
    vtkErrorMacro(<< "Can only fetch surface nodes from this source.");
    }

  // Let's start with graticule level 2 ... why not?
  root->SetGraticuleLevel(2);

  vtkSmartPointer<vtkGeoGraticule> grat = vtkSmartPointer<vtkGeoGraticule>::New();
  grat->SetLatitudeLevel(root->GetGraticuleLevel());
  grat->SetLongitudeLevel(root->GetGraticuleLevel());
  grat->SetLongitudeBounds(-180.0, 180.0);
  grat->SetLatitudeBounds(-90.0, 90.0);
  grat->SetGeometryType(vtkGeoGraticule::QUADRILATERALS);
  this->Transform->SetInputConnection(grat->GetOutputPort());
  this->Transform->Update();
  double* realBounds = this->Transform->GetOutput()->GetBounds();

  // Extend the bounds just a tad to be safe
  double bounds[4];
  bounds[0] = realBounds[0] - (realBounds[1] - realBounds[0])*0.01;
  bounds[1] = realBounds[1] + (realBounds[1] - realBounds[0])*0.01;
  bounds[2] = realBounds[2] - (realBounds[3] - realBounds[2])*0.01;
  bounds[3] = realBounds[3] + (realBounds[3] - realBounds[2])*0.01;

  // Make the bounds square
  if (bounds[1] - bounds[0] > bounds[3] - bounds[2])
    {
    double size = bounds[1] - bounds[0];
    double center = (bounds[2] + bounds[3])/2.0;
    bounds[2] = center - size/2.0;
    bounds[3] = center + size/2.0;
    }
  else
    {
    double size = bounds[3] - bounds[2];
    double center = (bounds[0] + bounds[1])/2.0;
    bounds[0] = center - size/2.0;
    bounds[1] = center + size/2.0;
    }

  root->GetModel()->ShallowCopy(this->Transform->GetOutput());
  root->SetLatitudeRange(-90.0, 90.0);
  root->SetLongitudeRange(-180.0, 180.0);
  root->SetProjectionBounds(bounds);
  root->SetLevel(0);
  this->RefineAndComputeError(root);

  // Make sure bounds are up to date so we don't have threading issues
  // when we hand this off to the main thread.
  root->GetModel()->ComputeBounds();

  return true;
}

//----------------------------------------------------------------------------
bool vtkGeoProjectionSource::FetchChild(vtkGeoTreeNode* p, int index, vtkGeoTreeNode* c)
{
  vtkGeoTerrainNode* parent = 0;
  if (!(parent = vtkGeoTerrainNode::SafeDownCast(p)))
    {
    vtkErrorMacro(<< "Can only fetch surface nodes from this source.");
    }
  vtkGeoTerrainNode* child = 0;
  if (!(child = vtkGeoTerrainNode::SafeDownCast(c)))
    {
    vtkErrorMacro(<< "Can only fetch surface nodes from this source.");
    }

  // Clip the cells of the children
  double bounds[4];
  parent->GetProjectionBounds(bounds);
  double center[3] = {
    (bounds[1] + bounds[0])/2.0,
    (bounds[3] + bounds[2])/2.0, 0.0};

  vtkSmartPointer<vtkClipPolyData> lonClip = vtkSmartPointer<vtkClipPolyData>::New();
  vtkSmartPointer<vtkPlane> lonClipPlane = vtkSmartPointer<vtkPlane>::New();
  lonClipPlane->SetOrigin(center);
  lonClipPlane->SetNormal(-1.0, 0.0, 0.0);
  lonClip->SetClipFunction(lonClipPlane);
  lonClip->GenerateClippedOutputOn();
  lonClip->SetInput(parent->GetModel());

  vtkSmartPointer<vtkPlane> latClipPlane = vtkSmartPointer<vtkPlane>::New();
  latClipPlane->SetOrigin(center);
  latClipPlane->SetNormal(0.0, -1.0, 0.0);
  vtkSmartPointer<vtkClipPolyData> latClip = vtkSmartPointer<vtkClipPolyData>::New();
  latClip->SetClipFunction(latClipPlane);
  latClip->GenerateClippedOutputOn();
  if (index % 2)
    {
    latClip->SetInputConnection(lonClip->GetOutputPort(1));
    bounds[0] = center[0];
    }
  else
    {
    latClip->SetInputConnection(lonClip->GetOutputPort(0));
    bounds[1] = center[0];
    }
  latClip->Update();
  if (index / 2)
    {
    child->GetModel()->ShallowCopy(latClip->GetOutput(1));
    bounds[2] = center[1];
    }
  else
    {
    child->GetModel()->ShallowCopy(latClip->GetOutput(0));
    bounds[3] = center[1];
    }
  int level = parent->GetLevel() + 1;
  child->SetLevel(level);
  child->SetProjectionBounds(bounds);

  // Set the id
  int id = parent->GetId() | (index << (level*2 - 2));
  child->SetId(id);

  double* latRange = 0;
  double* lonRange = 0;
  if (child->GetModel()->GetNumberOfPoints() > 0)
    {
    latRange = child->GetModel()->GetPointData()->GetArray("LatLong")->GetRange(0);
    child->SetLatitudeRange(latRange);
    lonRange = child->GetModel()->GetPointData()->GetArray("LatLong")->GetRange(1);
    child->SetLongitudeRange(lonRange);
    }
  else
    {
    child->SetLatitudeRange(0.0, 0.0);
    child->SetLongitudeRange(0.0, 0.0);
    return true;
    }

  // Start with at least graticule level 2.
  child->SetGraticuleLevel(2);

  // Refine the node using vtkGeoGraticule and compute the error of the node.
  this->RefineAndComputeError(child);

  // We need to do four planar clips to get the desired result.
  // Using vtkBox or vtkPlanes produces a fuzzy clip that is not acceptable.
  for (int i = 0; i < 4; ++i)
    {
    vtkSmartPointer<vtkClipPolyData> finalClip = vtkSmartPointer<vtkClipPolyData>::New();
    vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
    if (i == 0)
      {
      plane->SetOrigin(bounds[0], 0.0, 0.0);
      plane->SetNormal(1.0, 0.0, 0.0);
      }
    else if (i == 1)
      {
      plane->SetOrigin(bounds[1], 0.0, 0.0);
      plane->SetNormal(-1.0, 0.0, 0.0);
      }
    else if (i == 2)
      {
      plane->SetOrigin(0.0, bounds[2], 0.0);
      plane->SetNormal(0.0, 1.0, 0.0);
      }
    else if (i == 3)
      {
      plane->SetOrigin(0.0, bounds[3], 0.0);
      plane->SetNormal(0.0, -1.0, 0.0);
      }
    finalClip->SetClipFunction(plane);
    vtkSmartPointer<vtkPolyData> pd = vtkSmartPointer<vtkPolyData>::New();
    pd->ShallowCopy(child->GetModel());
    finalClip->SetInput(pd);
    finalClip->Update();
    child->GetModel()->ShallowCopy(finalClip->GetOutput());
    }

  // The lat/long range could have changed
  if (child->GetModel()->GetNumberOfPoints() > 0)
    {
    latRange = child->GetModel()->GetPointData()->GetArray("LatLong")->GetRange(0);
    child->SetLatitudeRange(latRange);
    lonRange = child->GetModel()->GetPointData()->GetArray("LatLong")->GetRange(1);
    child->SetLongitudeRange(lonRange);
    }
  else
    {
    child->SetLatitudeRange(0.0, 0.0);
    child->SetLongitudeRange(0.0, 0.0);
    }

  // Make sure bounds are up to date so we don't have threading issues
  // when we hand this off to the main thread.
  child->GetModel()->ComputeBounds();

  return true;
}

//----------------------------------------------------------------------------
void vtkGeoProjectionSource::SetProjection(int projection)
{
  this->Projection = projection;
  vtkSmartPointer<vtkTransformFilter> filter = vtkSmartPointer<vtkTransformFilter>::New();
  vtkSmartPointer<vtkGeoTransform> trans = vtkSmartPointer<vtkGeoTransform>::New();
  vtkSmartPointer<vtkGeoProjection> proj = vtkSmartPointer<vtkGeoProjection>::New();
  proj->SetName(vtkGeoProjection::GetProjectionName(projection));
  trans->SetDestinationProjection(proj);
  filter->SetTransform(trans);
  this->SetTransform(filter);
}

