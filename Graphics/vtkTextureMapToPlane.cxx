/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1995 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
#include <math.h>
#include "vtkTextureMapToPlane.hh"
#include "vtkMath.hh"
#include "vtkFloatTCoords.hh"

// Description:
// Construct with s,t range=(0,1) and automatic plane generation turned on.
vtkTextureMapToPlane::vtkTextureMapToPlane()
{
  this->Normal[0] = 0.0;
  this->Normal[1] = 0.0;
  this->Normal[2] = 1.0;

  this->SRange[0] = 0.0;
  this->SRange[1] = 1.0;

  this->TRange[0] = 0.0;
  this->TRange[1] = 1.0;

  this->AutomaticPlaneGeneration = 1;
}

void vtkTextureMapToPlane::Execute()
{
  vtkMath math;
  float tcoords[2];
  int numPts;
  vtkFloatTCoords *newTCoords;
  int i, j;
  float *bounds;
  float proj, minProj, axis[3], sAxis[3], tAxis[3];
  int dir = 0;
  float s, t, sSf, tSf, *p;
  vtkDataSet *output=this->Output;

  vtkDebugMacro(<<"Generating texture coordinates!");
  if ( (numPts=this->Input->GetNumberOfPoints()) < 3 )
    {
    vtkErrorMacro(<< "Not enough points to map with\n");
    return;
    }
//
//  Allocate texture data
//
  newTCoords = new vtkFloatTCoords(numPts,2);
//
//  Compute least squares plane if on automatic mode; otherwise use
//  point and normal specified.
//
  if ( this->AutomaticPlaneGeneration )
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
  bounds = output->GetBounds();
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
    p = output->GetPoint(i);
    for (j=0; j<3; j++) axis[j] = p[j] - bounds[2*j];

    tcoords[0] = this->SRange[0] + math.Dot(sAxis,axis) * sSf;
    tcoords[1] = this->TRange[0] + math.Dot(tAxis,axis) * tSf;

    newTCoords->SetTCoord(i,tcoords);
    }
//
// Update ourselves
//
  output->GetPointData()->CopyTCoordsOff();
  output->GetPointData()->PassData(this->Input->GetPointData());

  output->GetPointData()->SetTCoords(newTCoords);
  newTCoords->Delete();
}

#define TOLERANCE 1.0e-03

void vtkTextureMapToPlane::ComputeNormal()
{
  int numPts=this->Output->GetNumberOfPoints();
  float m[9], v[3], *x;
  int i, ptId;
  int dir = 0;
  float length, w, *c1, *c2, *c3, det;
  float *bounds;
  vtkMath math;
//
//  First thing to do is to get an initial normal and point to define
//  the plane.  Then, use this information to construct better
//  matrices.  If problem occurs, then the point and plane becomes the
//  fallback value.
//
  //  Get minimum width of bounding box.
  bounds = this->Output->GetBounds();
  length = this->Output->GetLength();

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
  v[0] = v[1] = v[2] = 0.0;
  for (i=0; i<9; i++) m[i] = 0.0;

  for (ptId=0; ptId < numPts; ptId++) 
    {
    x = this->Output->GetPoint(ptId);

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
  if ( (det = math.Determinant3x3 (c1,c2,c3)) <= TOLERANCE )
    return;

  this->Normal[0] = math.Determinant3x3 (v,c2,c3) / det;
  this->Normal[1] = math.Determinant3x3 (c1,v,c3) / det;
  this->Normal[2] = -1.0; // because of the formulation

  return;
}

void vtkTextureMapToPlane::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataSetToDataSetFilter::PrintSelf(os,indent);

  os << indent << "S Range: (" << this->SRange[0] << ", "
                               << this->SRange[1] << ")\n";
  os << indent << "T Range: (" << this->TRange[0] << ", "
                               << this->TRange[1] << ")\n";
  os << indent << "Automatic Normal Generation: " << 
                  (this->AutomaticPlaneGeneration ? "On\n" : "Off\n");
  os << indent << "Normal: (" << this->Normal[0] << ", "
                                << this->Normal[1] << ", "
                                << this->Normal[2] << ")\n";
}

