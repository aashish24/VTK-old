/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1996 Ken Martin, Will Schroeder, Bill Lorensen.

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
#include "vtkLine.h"
#include "vtkMath.h"
#include "vtkCellArray.h"
#include "vtkPointLocator.h"

// Description:
// Deep copy of cell.
vtkLine::vtkLine(const vtkLine& l)
{
  this->Points = l.Points;
  this->PointIds = l.PointIds;
}

#define NO_INTERSECTION 1
#define INTERSECTION 2
#define ON_LINE 6

int vtkLine::EvaluatePosition(float x[3], float closestPoint[3], 
                             int& subId, float pcoords[3],
                             float& dist2, float *weights)
{
  float *a1, *a2;

  subId = 0;
  pcoords[1] = pcoords[2] = 0.0;

  a1 = this->Points.GetPoint(0);
  a2 = this->Points.GetPoint(1);

  dist2 = this->DistanceToLine(x,a1,a2,pcoords[0],closestPoint);

  weights[0] = pcoords[0];
  weights[1] = 1.0 - pcoords[0];

  if ( pcoords[0] < 0.0 || pcoords[0] > 1.0 ) return 0;
  else return 1;
}

void vtkLine::EvaluateLocation(int& vtkNotUsed(subId), float pcoords[3], 
			       float x[3], float *weights)
{
  int i;
  float *a1 = this->Points.GetPoint(0);
  float *a2 = this->Points.GetPoint(1);

  for (i=0; i<3; i++) 
    {
    x[i] = a1[i] + pcoords[0]*(a2[i] - a1[i]);
    }

  weights[0] = pcoords[0];
  weights[1] = 1.0 - pcoords[0];
}

//
//  Intersect two 3D lines
//
int vtkLine::Intersection (float a1[3], float a2[3], float b1[3], float b2[3],
                           float& u, float& v)
{
  float a21[3], b21[3], b1a1[3];
  float sys[2][2], c[2], det;
  int i;
//
//  Initialize 
//
  u = v = 0.0;
//
//   Determine line vectors.
//
  for (i=0; i<3; i++) 
    {
    a21[i] = a2[i] - a1[i];
    b21[i] = b2[i] - b1[i];
    b1a1[i] = b1[i] - a1[i];
    }
//
//   Compute the system (least squares) matrix.
//
  sys[0][0] = vtkMath::Dot ( a21, a21 );
  sys[0][1] = -vtkMath::Dot ( a21, b21 );
  sys[1][0] = sys[0][1];
  sys[1][1] = vtkMath::Dot ( b21, b21 );
//
//   Compute the least squares system constant term.
//
  c[0] = vtkMath::Dot ( a21, b1a1 );
  c[1] = -vtkMath::Dot ( b21, b1a1 );
//
//  Solve the system of equations
//
  if ( (det=vtkMath::Determinant2x2(sys[0],sys[1])) <= VTK_TOL )
    {
    return ON_LINE;
    }
  else 
    {
    u = vtkMath::Determinant2x2(c,sys[1]) / det;
    v = vtkMath::Determinant2x2(sys[0],c) / det;
    }
//
//  Check parametric coordinates for intersection.
//
  if ( (0.0 <= u) && (u <= 1.0) && (0.0 <= v) && (v <= 1.0) )
    {
    return INTERSECTION;
    }
  else
    {
    return NO_INTERSECTION;
    }
}

int vtkLine::CellBoundary(int vtkNotUsed(subId), float pcoords[3], 
			  vtkIdList& pts)
{
 pts.Reset();

  if ( pcoords[0] >= 0.5 )
    {
    pts.SetId(0,this->PointIds.GetId(1));
    if ( pcoords[0] > 1.0 ) return 0;
    else return 1;
    }
  else
    {
    pts.SetId(0,this->PointIds.GetId(0));
    if ( pcoords[0] < 0.0 ) return 0;
    else return 1;
    }
}

//
// marching lines case table
//
typedef int VERT_LIST;

typedef struct {
  VERT_LIST verts[2];
} LINE_CASES;

static LINE_CASES lineCases[4]= {
  {{-1,-1}},
  {{1,0}},
  {{0,1}},
  {{-1,-1}}};

void vtkLine::Contour(float value, vtkFloatScalars *cellScalars, 
		      vtkPointLocator *locator, vtkCellArray *verts, 
		      vtkCellArray *vtkNotUsed(lines), 
		      vtkCellArray *vtkNotUsed(polys), 
		      vtkFloatScalars *scalars)
{
  static int CASE_MASK[2] = {1,2};
  int index, i;
  LINE_CASES *lineCase;
  VERT_LIST *vert;
  float t, x[3], *x1, *x2;
  int pts[1];

//
// Build the case table
//
  for ( i=0, index = 0; i < 2; i++)
    if (cellScalars->GetScalar(i) >= value) 
      index |= CASE_MASK[i];

  lineCase = lineCases + index;
  vert = lineCase->verts;

  if ( vert[0] > -1 )
    {
    t = (value - cellScalars->GetScalar(vert[0])) /
        (cellScalars->GetScalar(vert[1]) - cellScalars->GetScalar(vert[0]));
    x1 = this->Points.GetPoint(vert[0]);
    x2 = this->Points.GetPoint(vert[1]);
    for (i=0; i<3; i++) x[i] = x1[i] + t * (x2[i] - x1[i]);

    if ( (pts[0] = locator->IsInsertedPoint(x)) < 0 )
      {
      pts[0] = locator->InsertNextPoint(x);
      scalars->InsertScalar(pts[0],value);
      }
    verts->InsertNextCell(1,pts);
    }
}

// Description:
// Compute distance to finite line. Returns parametric coordinate t 
// and point location on line.
float vtkLine::DistanceToLine(float x[3], float p1[3], float p2[3], 
                              float &t, float closestPoint[3])
{
  int i;
  float p21[3], denom, num;
  float *closest;
//
//   Determine appropriate vectors
// 
  for (i=0; i<3; i++) p21[i] = p2[i] - p1[i];
//
//   Get parametric location
//
  num = p21[0]*(x[0]-p1[0]) + p21[1]*(x[1]-p1[1]) + p21[2]*(x[2]-p1[2]);

  if ( (denom = vtkMath::Dot(p21,p21)) < fabs(VTK_TOL*num) ) //numerically bad!
    {
    closest = p1; //arbitrary, point is (numerically) far away
    }
//
// If parametric coordinate is within 0<=p<=1, then the point is closest to
// the line.  Otherwise, it's closest to a point at the end of the line.
//
  else if ( (t=num/denom) < 0.0 )
    {
    closest = p1;
    }
  else if ( t > 1.0 )
    {
    closest = p2;
    }
  else
    {
    closest = p21;
    for (i=0; i<3; i++) p21[i] = p1[i] + t*p21[i];
    }

  closestPoint[0] = closest[0]; 
  closestPoint[1] = closest[1]; 
  closestPoint[2] = closest[2]; 

  return vtkMath::Distance2BetweenPoints(closest,x);
}

//
// Description:
// Determine the distance of the current vertex to the edge defined by
// the vertices provided.  Returns distance squared. Note: line is assumed
// infinite in extent.
//
float vtkLine::DistanceToLine (float x[3], float p1[3], float p2[3])
{
  int i;
  float np1[3], p1p2[3], proj, den;

  for (i=0; i<3; i++) 
    {
    np1[i] = x[i] - p1[i];
    p1p2[i] = p1[i] - p2[i];
    }

  if ( (den=vtkMath::Norm(p1p2)) != 0.0 )
      for (i=0; i<3; i++)
          p1p2[i] /= den;
  else
      return vtkMath::Dot(np1,np1);

  proj = vtkMath::Dot(np1,p1p2);

  return (vtkMath::Dot(np1,np1) - proj*proj);
}

//
// Line-line intersection. Intersection has to occur within [0,1] parametric
// coordinates and with specified tolerance.
//
int vtkLine::IntersectWithLine(float p1[3], float p2[3], float tol, float& t,
                              float x[3], float pcoords[3], int& subId)
{
  float *a1, *a2;
  float projXYZ[3];
  int i;

  subId = 0;
  pcoords[1] = pcoords[2] = 0.0;

  a1 = this->Points.GetPoint(0);
  a2 = this->Points.GetPoint(1);

  if ( this->Intersection(p1, p2, a1, a2, t, pcoords[0]) == NO_INTERSECTION )
    {
    return 0;
    }
  else //check to make sure lies within tolerance
    {
    for (i=0; i<3; i++)
      {
      x[i] = a1[i] + pcoords[i]*(a2[i]-a1[i]);
      projXYZ[i] = p1[i] + t*(p2[i]-p1[i]);
      }
    if ( vtkMath::Distance2BetweenPoints(x,projXYZ) <= tol*tol )
      return 1;
    else
      return 0;
    }
}

int vtkLine::Triangulate(int vtkNotUsed(index), vtkFloatPoints &pts)
{
  pts.Reset();
  pts.InsertPoint(0,this->Points.GetPoint(0));
  pts.InsertPoint(1,this->Points.GetPoint(1));

  return 1;
}

void vtkLine::Derivatives(int vtkNotUsed(subId), 
			  float vtkNotUsed(pcoords)[3], 
			  float *values, 
                          int dim, float *derivs)
{
  float *x0, *x1, deltaX[3];
  int i, j;

  x0 = this->Points.GetPoint(0);
  x1 = this->Points.GetPoint(1);

  for (i=0; i<3; i++) deltaX[i] = x1[i] - x0[i];
  for (i=0; i<dim; i++) 
    {
    for (j=0; j<3; j++)
      {
      derivs[3*i+j] = (values[2*i+1] - values[2*i]) / deltaX[j];
      }
    }
}


