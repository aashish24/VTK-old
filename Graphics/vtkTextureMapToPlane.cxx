/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Description:
---------------------------------------------------------------------------
This file is part of the Visualization Library. No part of this file
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
#include <math.h>
#include "TMap2Pl.hh"
#include "vlMath.hh"
#include "FTCoords.hh"

vlTextureMapToPlane::vlTextureMapToPlane()
{
  this->Normal[0] = 0.0;
  this->Normal[1] = 0.0;
  this->Normal[2] = 1.0;

  this->SRange[0] = 0.0;
  this->SRange[1] = 1.0;

  this->TRange[0] = 0.0;
  this->TRange[1] = 1.0;

  this->AutomaticNormalGeneration = 1;
}

void vlTextureMapToPlane::PrintSelf(ostream& os, vlIndent indent)
{
  if (this->ShouldIPrint(vlTextureMapToPlane::GetClassName()))
    {
    vlDataSetToDataSetFilter::PrintSelf(os,indent);

    os << indent << "S Range: (" << this->SRange[0] << ", "
                                 << this->SRange[1] << ")\n";
    os << indent << "T Range: (" << this->TRange[0] << ", "
                                 << this->TRange[1] << ")\n";
    os << indent << "Automatic Normal Generation: " << 
                    (this->AutomaticNormalGeneration ? "On\n" : "Off\n");
    os << indent << "Normal: (" << this->Normal[0] << ", "
                                  << this->Normal[1] << ", "
                                  << this->Normal[2] << ")\n";
    }
}

void vlTextureMapToPlane::Execute()
{
  vlMath math;
  float tcoords[2];
  int numPts;
  vlFloatTCoords *newTCoords;
  int i, j;
  float *bounds;
  float proj, minProj, axis[3], sAxis[3], tAxis[3];
  int dir;
  float s, t, sSf, tSf, *p;
//
// get the data from display data
//
  this->Initialize();
  if ( numPts < 3 )
    {
    vlErrorMacro(<< "Not enough points to map with\n");
    return;
    }
//
//  Allocate texture data
//
  newTCoords = new vlFloatTCoords(numPts,2);
//
//  Compute least squares plane if on automatic mode; otherwise use
//  point and normal specified.
//
  if ( this->AutomaticNormalGeneration )
    this->ComputeNormal();

  math.Normalize (this->Normal);
//
//  Now project each point onto plane generating s,t texture coordinates
//
//  Create local s-t coordinate system.  Need to find the two axes on
//  the plane and encompassing all the points.  Hence use the bounding
//  box as a reference.
//
  for (minProj=1.0, i=0; i<3; i++) 
    {
    axis[0] = axis[1] = axis[2] = 0.0;
    axis[i] = 1.0;
    if ( (proj=fabs(math.Dot(this->Normal,axis))) < minProj ) 
      {
      minProj = proj;
      dir = i;
      }
    }
  axis[0] = axis[1] = axis[2] = 0.0;
  axis[dir] = 1.0;

  math.Cross (this->Normal, axis, tAxis);
  math.Normalize (tAxis);

  math.Cross (tAxis, this->Normal, sAxis);
//
//  Construct projection matrices
//
//
//  Arrange s-t axes so that parametric location of points will fall
//  between s_range and t_range.  Simplest to do by projecting maximum
//  corner of bounding box unto plane and backing out scale factors.
//
  bounds = this->GetBounds();
  for (i=0; i<3; i++) axis[i] = bounds[2*i+1] - bounds[2*i];

  s = math.Dot(sAxis,axis);
  t = math.Dot(tAxis,axis);

  sSf = (this->SRange[1] - this->SRange[0]) / s;
  tSf = (this->TRange[1] - this->TRange[0]) / t;
//
//  Now can loop over all points, computing parametric coordinates.
//
  for (i=0; i<numPts; i++) 
    {
    p = this->GetPoint(i);
    for (j=0; j<3; j++) axis[j] = p[j] - bounds[2*j];

    tcoords[0] = this->SRange[0] + math.Dot(sAxis,axis) * sSf;
    tcoords[1] = this->TRange[0] + math.Dot(tAxis,axis) * tSf;

    newTCoords->SetTCoord(i,tcoords);
    }
//
// Update ourselves
//
  this->PointData.SetTCoords(newTCoords);
}

#define TOLERANCE 1.0e-03

void vlTextureMapToPlane::ComputeNormal()
{
  int numPts=this->GetNumberOfPoints();
  float m[9], v[3], *x, d;
  int i, ptId, dir;
  float length, w, *c1, *c2, *c3, det;
  float *bounds;
  vlMath math;
//
//  First thing to do is to get an initial normal and point to define
//  the plane.  Then, use this information to construct better
//  matrices.  If problem occurs, then the point and plane becomes the
//  fallback value.
//
  //  Get minimum width of bounding box.
  bounds = this->GetBounds();
  length = this->GetLength();

  for (w=length, i=0; i<3; i++)
    {
    this->Normal[i] = 0.0;
    if ( (bounds[2*i+1] - bounds[2*i]) < w ) 
      {
      dir = i;
      w = bounds[2*i+1] - bounds[2*i];
      }
    }
//
//  If the bounds is perpendicular to one of the axes, then can
//  quickly compute normal.
//
  this->Normal[dir] = 1.0;
  if ( w <= (length*TOLERANCE) ) return;
//
//  Need to compute least squares approximation.  Depending on major
//  normal direction (dir), construct matrices appropriately.
//
    //  Compute 3x3 least squares matrix
  for (i=0; i<9; i++) m[i] = 0.0;

  for (ptId=0; ptId < numPts; ptId++) 
    {
    x = this->GetPoint(ptId);

    v[0] += x[0]*x[2];
    v[1] += x[1]*x[2];
    v[2] += x[2];

    m[0] += x[0]*x[0];
    m[1] += x[0]*x[1];
    m[2] += x[0];

    m[3] += x[0]*x[1];
    m[4] += x[1]*x[1];
    m[5] += x[1];

    m[6] += x[0];
    m[7] += x[1];
    }
  m[8] = numPts;
//
//  Solve linear system using Kramers rule
//
  c1 = m; c2 = m+3; c3 = m+6;
  if ( (det = math.Determinate3x3 (c1,c2,c3)) <= TOLERANCE )
    return;

  this->Normal[0] = math.Determinate3x3 (v,c2,c3) / det;
  this->Normal[1] = math.Determinate3x3 (c1,v,c3) / det;
  this->Normal[2] = -1.0; // because of the formulation

  return;
}
