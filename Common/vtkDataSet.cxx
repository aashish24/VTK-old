/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Library. No part of this file or its 
contents may be copied, reproduced or altered in any way without the express
written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
//
// DataSet methods
//
#include <math.h>
#include "DataSet.hh"

// Description:
// Constructor with default bounds (0,1, 0,1, 0,1).
vlDataSet::vlDataSet ()
{
  this->Bounds[0] = 0.0;
  this->Bounds[1] = 1.0;
  this->Bounds[2] = 0.0;
  this->Bounds[3] = 1.0;
  this->Bounds[4] = 0.0;
  this->Bounds[5] = 1.0;
}

// Description:
// Copy constructor.
vlDataSet::vlDataSet (const vlDataSet& ds) :
PointData(ds.PointData)
{
  for (int i=0; i < 6; i++) this->Bounds[i] = ds.Bounds[i];
}

void vlDataSet::Initialize()
{
  this->PointData.Initialize();
  this->Modified();
};

// Description:
// Compute the data bounding box from data points.
void vlDataSet::ComputeBounds()
{
  int i, j;
  float *x;

  if ( this->GetMTime() > this->ComputeTime )
    {
    this->Bounds[0] = this->Bounds[2] = this->Bounds[4] =  LARGE_FLOAT;
    this->Bounds[1] = this->Bounds[3] = this->Bounds[5] = -LARGE_FLOAT;
    for (i=0; i<this->GetNumberOfPoints(); i++)
      {
      x = this->GetPoint(i);
      for (j=0; j<3; j++)
        {
        if ( x[j] < this->Bounds[2*j] ) this->Bounds[2*j] = x[j];
        if ( x[j] > this->Bounds[2*j+1] ) this->Bounds[2*j+1] = x[j];
        }
      }

    this->ComputeTime.Modified();
    }
}

// Description:
// Return a pointer to the geometry bounding box in the form
// (xmin,xmax, ymin,ymax, zmin,zmax).
float *vlDataSet::GetBounds()
{
  this->ComputeBounds();
  return this->Bounds;
}
  
void vlDataSet::GetBounds(float bounds[6])
{
  this->ComputeBounds();
  for (int i=0; i<6; i++) bounds[i] = this->Bounds[i];
}
  
// Description:
// Get the center of the bounding box.
float *vlDataSet::GetCenter()
{
  static float center[3];

  this->ComputeBounds();
  for (int i=0; i<3; i++) 
    center[i] = (this->Bounds[2*i+1] + this->Bounds[2*i]) / 2.0;
  return center;
}

void vlDataSet::GetCenter(float center[3])
{
  float *c=this->GetCenter();
  for (int i=0; i<3; i++) center[i] = c[i];
}
  
// Description:
// Return the length of the diagonal of the bounding box.
float vlDataSet::GetLength()
{
  double diff, l=0.0;
  int i;

  this->ComputeBounds();
  for (i=0; i<3; i++)
    {
    diff = this->Bounds[2*i+1] - this->Bounds[2*i];
    l += diff * diff;
    }
 
  return (float)sqrt(l);
}

unsigned long int vlDataSet::GetMTime()
{
  if ( this->PointData.GetMTime() > this->MTime ) return this->PointData.GetMTime();
  else return this->MTime;
}

void vlDataSet::PrintSelf(ostream& os, vlIndent indent)
{
  float *bounds;

  vlObject::PrintSelf(os,indent);

  os << indent << "Number Of Points: " << this->GetNumberOfPoints() << "\n";
  os << indent << "Number Of Cells: " << this->GetNumberOfCells() << "\n";
  os << indent << "Point Data:\n";
  this->PointData.PrintSelf(os,indent.GetNextIndent());
  bounds = this->GetBounds();
  os << indent << "Bounds: \n";
  os << indent << "  Xmin,Xmax: (" <<bounds[0] << ", " << bounds[1] << ")\n";
  os << indent << "  Ymin,Ymax: (" <<bounds[2] << ", " << bounds[3] << ")\n";
  os << indent << "  Zmin,Zmax: (" <<bounds[4] << ", " << bounds[5] << ")\n";
  os << indent << "Compute Time: " <<this->ComputeTime.GetMTime() << "\n";
}

void vlDataSet::GetCellNeighbors(int cellId, vlIdList &ptIds,
                                 vlIdList &cellIds)
{
  int i;
  vlIdList otherCells(MAX_CELL_SIZE);

  // load list with candidate cells, remove current cell
  this->GetPointCells(ptIds.GetId(0),cellIds);
  cellIds.DeleteId(cellId);

  // now perform multiple intersections on list
  if ( cellIds.GetNumberOfIds() > 0 )
    {
    for ( i=1; i < ptIds.GetNumberOfIds(); i++)
      {
      this->GetPointCells(ptIds.GetId(i), otherCells);
      cellIds.IntersectWith(otherCells);
      }
    }
}

void vlDataSet::Squeeze()
{
  this->PointData.Squeeze();
}
