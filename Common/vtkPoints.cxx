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
//  3D Points, abstract representation
//
#include "Points.hh"
#include "FPoints.hh"
#include "IdList.hh"

vlPoints::vlPoints()
{
  this->Bounds[0] = this->Bounds[2] = this->Bounds[4] = 0.0;
  this->Bounds[1] = this->Bounds[3] = this->Bounds[5] = 1.0;
}

void vlPoints::GetPoints(vlIdList& ptId, vlFloatPoints& fp)
{
  for (int i=0; i<ptId.NumberOfIds(); i++)
    {
    fp.InsertPoint(i,this->GetPoint(ptId[i]));
    }
}
void vlPoints::ComputeBounds()
{
  int i, j;
  float *x;

  if ( this->GetMTime() > this->ComputeTime )
    {
    this->Bounds[0] = this->Bounds[2] = this->Bounds[4] =  LARGE_FLOAT;
    this->Bounds[1] = this->Bounds[3] = this->Bounds[5] = -LARGE_FLOAT;
    for (i=0; i<this->NumberOfPoints(); i++)
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

float *vlPoints::GetBounds()
{
  this->ComputeBounds();
  return this->Bounds;
}

void vlPoints::PrintSelf(ostream& os, vlIndent indent)
{
  if (this->ShouldIPrint(vlPoints::GetClassName()))
    {
    float *bounds;

    vlObject::PrintSelf(os,indent);

    os << indent << "Number Of Points: " << this->NumberOfPoints() << "\n";
    bounds = this->GetBounds();
    os << indent << "Bounds: \n";
    os << indent << "  Xmin,Xmax: (" << bounds[0] << ", " << bounds[1] << ")\n";
    os << indent << "  Ymin,Ymax: (" << bounds[2] << ", " << bounds[3] << ")\n";
    os << indent << "  Zmin,Zmax: (" << bounds[4] << ", " << bounds[5] << ")\n";
    }
}

