/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkCell.h"

#include "vtkMarchingSquaresCases.h"
#include "vtkPoints.h"

vtkCxxRevisionMacro(vtkCell, "$Revision$");

// Construct cell.
vtkCell::vtkCell()
{
  this->Points = vtkPoints::New();
  this->PointIds = vtkIdList::New();
  // Consistent Register/Deletes (ShallowCopy uses Register.)
  this->Points->Register(this);
  this->Points->Delete();
  this->PointIds->Register(this);
  this->PointIds->Delete();
}  

vtkCell::~vtkCell()
{
  this->Points->UnRegister(this);
  this->PointIds->UnRegister(this);
}


//
// Instantiate cell from outside
//
void vtkCell::Initialize(int npts, vtkIdType *pts, vtkPoints *p)
{
  this->PointIds->Reset();
  this->Points->Reset();

  for (int i=0; i<npts; i++)
    {
    this->PointIds->InsertId(i,pts[i]);
    this->Points->InsertPoint(i,p->GetPoint(pts[i]));
    }
}
 
void vtkCell::ShallowCopy(vtkCell *c)
{
  this->Points->ShallowCopy(c->Points);
  if ( this->PointIds )
    {
    this->PointIds->UnRegister(this);
    this->PointIds = c->PointIds;
    this->PointIds->Register(this);
    }
}

void vtkCell::DeepCopy(vtkCell *c)
{
  this->Points->DeepCopy(c->Points);
  this->PointIds->DeepCopy(c->PointIds);
}

#define VTK_RIGHT 0
#define VTK_LEFT 1
#define VTK_MIDDLE 2

// Bounding box intersection modified from Graphics Gems Vol I. The method
// returns a non-zero value if the bounding box is hit. Origin[3] starts
// the ray, dir[3] is the vector components of the ray in the x-y-z
// directions, coord[3] is the location of hit, and t is the parametric
// coordinate along line. (Notes: the intersection ray dir[3] is NOT
// normalized.  Valid intersections will only occur between 0<=t<=1.)
char vtkCell::HitBBox (float bounds[6], float origin[3], float dir[3], 
                      float coord[3], float& t)
{
  char    inside=1;
  char    quadrant[3];
  int     i, whichPlane=0;
  float   maxT[3], candidatePlane[3];

  //  First find closest planes
  //
  for (i=0; i<3; i++) 
    {
    if ( origin[i] < bounds[2*i] ) 
      {
      quadrant[i] = VTK_LEFT;
      candidatePlane[i] = bounds[2*i];
      inside = 0;
      }
    else if ( origin[i] > bounds[2*i+1] ) 
      {
      quadrant[i] = VTK_RIGHT;
      candidatePlane[i] = bounds[2*i+1];
      inside = 0;
      }
    else 
      {
      quadrant[i] = VTK_MIDDLE;
      }
    }

  //  Check whether origin of ray is inside bbox
  //
  if (inside) 
    {
    coord[0] = origin[0];
    coord[1] = origin[1];
    coord[2] = origin[2];
    t = 0;
    return 1;
    }
  
  //  Calculate parametric distances to plane
  //
  for (i=0; i<3; i++)
    {
    if ( quadrant[i] != VTK_MIDDLE && dir[i] != 0.0 )
      {
      maxT[i] = (candidatePlane[i]-origin[i]) / dir[i];
      }
    else
      {
      maxT[i] = -1.0;
      }
    }

  //  Find the largest parametric value of intersection
  //
  for (i=0; i<3; i++)
    {
    if ( maxT[whichPlane] < maxT[i] )
      {
      whichPlane = i;
      }
    }

  //  Check for valid intersection along line
  //
  if ( maxT[whichPlane] > 1.0 || maxT[whichPlane] < 0.0 )
    {
    return 0;
    }
  else
    {
    t = maxT[whichPlane];
    }

  //  Intersection point along line is okay.  Check bbox.
  //
  for (i=0; i<3; i++) 
    {
    if (whichPlane != i) 
      {
      coord[i] = origin[i] + maxT[whichPlane]*dir[i];
      if ( coord[i] < bounds[2*i] || coord[i] > bounds[2*i+1] )
        {
        return 0;
        }
      } 
    else 
      {
      coord[i] = candidatePlane[i];
      }
    }

    return 1;
}

// Compute cell bounding box (xmin,xmax,ymin,ymax,zmin,zmax). Return pointer
// to array of six float values.
float *vtkCell::GetBounds ()
{
  float *x;
  int i, numPts=this->Points->GetNumberOfPoints();

  this->Bounds[0] = this->Bounds[2] = this->Bounds[4] =  VTK_LARGE_FLOAT;
  this->Bounds[1] = this->Bounds[3] = this->Bounds[5] = -VTK_LARGE_FLOAT;

  for (i=0; i<numPts; i++)
    {
    x = this->Points->GetPoint(i);

    this->Bounds[0] = (x[0] < this->Bounds[0] ? x[0] : this->Bounds[0]);
    this->Bounds[1] = (x[0] > this->Bounds[1] ? x[0] : this->Bounds[1]);
    this->Bounds[2] = (x[1] < this->Bounds[2] ? x[1] : this->Bounds[2]);
    this->Bounds[3] = (x[1] > this->Bounds[3] ? x[1] : this->Bounds[3]);
    this->Bounds[4] = (x[2] < this->Bounds[4] ? x[2] : this->Bounds[4]);
    this->Bounds[5] = (x[2] > this->Bounds[5] ? x[2] : this->Bounds[5]);
    
    }
  return this->Bounds;
}

// Compute cell bounding box (xmin,xmax,ymin,ymax,zmin,zmax). Copy result into
// user provided array.
void vtkCell::GetBounds(float bounds[6])
{
  this->GetBounds();
  for (int i=0; i < 6; i++)
    {
    bounds[i] = this->Bounds[i];
    }
}

// Compute Length squared of cell (i.e., bounding box diagonal squared).
float vtkCell::GetLength2 ()
{
  double diff, l=0.0;
  int i;

  this->GetBounds();
  for (i=0; i<3; i++)
    {
    diff = this->Bounds[2*i+1] - this->Bounds[2*i];
    l += diff * diff;
    }
  if(l > VTK_LARGE_FLOAT)
    {
    return VTK_LARGE_FLOAT;
    }
  return static_cast<float>(l);
}

// Return center of the cell in parametric coordinates.
// Note that the parametric center is not always located 
// at (0.5,0.5,0.5). The return value is the subId that
// the center is in (if a composite cell). If you want the
// center in x-y-z space, invoke the EvaluateLocation() method.
int vtkCell::GetParametricCenter(float pcoords[3])
{
  pcoords[0] = pcoords[1] = pcoords[2] = 0.5;
  return 0;
}

// This method works fine for all "rectangular" cells, not triangular
// and tetrahedral topologies.
float vtkCell::GetParametricDistance(float pcoords[3])
{
  int i;
  float pDist, pDistMax=0.0f;

  for (i=0; i<3; i++)
    {
    if ( pcoords[i] < 0.0 ) 
      {
      pDist = -pcoords[i];
      }
    else if ( pcoords[i] > 1.0 ) 
      {
      pDist = pcoords[i] - 1.0f;
      }
    else //inside the cell in the parametric direction
      {
      pDist = 0.0;
      }
    if ( pDist > pDistMax )
      {
      pDistMax = pDist;
      }
    }
  return pDistMax;
}


void vtkCell::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  int numIds=this->PointIds->GetNumberOfIds();
  
  os << indent << "Number Of Points: " << numIds << "\n";

  if ( numIds > 0 )
    {
    float *bounds=this->GetBounds();

    os << indent << "Bounds: \n";
    os << indent << "  Xmin,Xmax: (" << bounds[0] << ", " << bounds[1] << ")\n";
    os << indent << "  Ymin,Ymax: (" << bounds[2] << ", " << bounds[3] << ")\n";
    os << indent << "  Zmin,Zmax: (" << bounds[4] << ", " << bounds[5] << ")\n";

    os << indent << "  Point ids are: ";
    for (int i=0; i < numIds; i++)
      {
      os << this->PointIds->GetId(i);
      if ( i && !(i % 12) )
        {
        os << "\n\t";
        }
      else
        {
        if ( i != (numIds-1) )
          {
          os << ", ";
          }
        }
      }
    os << indent << "\n";
    }
}

// Note: the following code is placed here to deal with cross-library
// symbol export and import on Microsoft compilers.
static vtkMarchingSquaresLineCases VTK_MARCHING_SQUARES_LINECASES[] = { 
  {{-1, -1, -1, -1, -1}},
  {{0, 3, -1, -1, -1}},
  {{1, 0, -1, -1, -1}},
  {{1, 3, -1, -1, -1}},
  {{2, 1, -1, -1, -1}},
  {{0, 3, 2, 1, -1}},
  {{2, 0, -1, -1, -1}},
  {{2, 3, -1, -1, -1}},
  {{3, 2, -1, -1, -1}},
  {{0, 2, -1, -1, -1}},
  {{1, 0, 3, 2, -1}},
  {{1, 2, -1, -1, -1}},
  {{3, 1, -1, -1, -1}},
  {{0, 1, -1, -1, -1}},
  {{3, 0, -1, -1, -1}},
  {{-1, -1, -1, -1, -1}}
};

vtkMarchingSquaresLineCases* vtkMarchingSquaresLineCases::GetCases()
{
  return VTK_MARCHING_SQUARES_LINECASES;
}

//----------------------------------------------------------------------------
#ifndef VTK_REMOVE_LEGACY_CODE
vtkCell* vtkCell::MakeObject()
{
  VTK_LEGACY_METHOD(MakeObject, "4.2");
  vtkCell* c = this->NewInstance();
  c->DeepCopy(this);
  return c;
}
#endif
