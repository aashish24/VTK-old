/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1998 Ken Martin, Will Schroeder, Bill Lorensen.

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
#include "vtkVoxel.h"
#include "vtkMath.h"
#include "vtkLine.h"
#include "vtkPixel.h"
#include "vtkCellArray.h"
#include "vtkPointLocator.h"

// Description:
// Construct the voxel with eight points.
vtkVoxel::vtkVoxel()
{
  this->Points.SetNumberOfPoints(8);
  this->PointIds.SetNumberOfIds(8);
}

// Description:
// Deep copy of cell.
vtkVoxel::vtkVoxel(const vtkVoxel& b)
{
  this->Points = b.Points;
  this->PointIds = b.PointIds;
}

int vtkVoxel::EvaluatePosition(float x[3], float closestPoint[3],
                              int& subId, float pcoords[3], 
                              float& dist2, float *weights)
{
  float *pt1, *pt2, *pt3, *pt4;
  int i;

  subId = 0;
//
// Get coordinate system
//
  pt1 = this->Points.GetPoint(0);
  pt2 = this->Points.GetPoint(1);
  pt3 = this->Points.GetPoint(2);
  pt4 = this->Points.GetPoint(4);
//
// Develop parametric coordinates
//
  pcoords[0] = (x[0] - pt1[0]) / (pt2[0] - pt1[0]);
  pcoords[1] = (x[1] - pt1[1]) / (pt3[1] - pt1[1]);
  pcoords[2] = (x[2] - pt1[2]) / (pt4[2] - pt1[2]);

  if ( pcoords[0] >= 0.0 && pcoords[1] <= 1.0 &&
  pcoords[1] >= 0.0 && pcoords[1] <= 1.0 &&
  pcoords[2] >= 0.0 && pcoords[2] <= 1.0 )
    {
    closestPoint[0] = x[0]; closestPoint[1] = x[1]; closestPoint[2] = x[2];
    dist2 = 0.0; // inside voxel
    this->InterpolationFunctions(pcoords,weights);
    return 1;
    }
  else
    {
    float pc[3], w[8];
    for (i=0; i<3; i++)
      {
      if (pcoords[i] < 0.0) pc[i] = 0.0;
      else if (pcoords[i] > 1.0) pc[i] = 1.0;
      else pc[i] = pcoords[i];
      }
    this->EvaluateLocation(subId, pc, closestPoint, (float *)w);
    dist2 = vtkMath::Distance2BetweenPoints(closestPoint,x);
    return 0;
    }
}

void vtkVoxel::EvaluateLocation(int& vtkNotUsed(subId), float pcoords[3], 
				float x[3], float *weights)
{
  float *pt1, *pt2, *pt3, *pt4;
  int i;

  pt1 = this->Points.GetPoint(0);
  pt2 = this->Points.GetPoint(1);
  pt3 = this->Points.GetPoint(2);
  pt4 = this->Points.GetPoint(4);

  for (i=0; i<3; i++)
    {
    x[i] = pt1[i] + pcoords[0]*(pt2[i] - pt1[i]) +
                    pcoords[1]*(pt3[i] - pt1[i]) +
                    pcoords[2]*(pt4[i] - pt1[i]);
    }
  
  this->InterpolationFunctions(pcoords,weights);
}

//
// Compute Interpolation functions
//
void vtkVoxel::InterpolationFunctions(float pcoords[3], float sf[8])
{
  float rm, sm, tm;

  float r = pcoords[0], s = pcoords[1], t = pcoords[2];

  rm = 1. - r;
  sm = 1. - s;
  tm = 1. - t;

  sf[0] = rm * sm * tm;
  sf[1] = r * sm * tm;
  sf[2] = rm * s * tm;
  sf[3] = r * s * tm;
  sf[4] = rm * sm * t;
  sf[5] = r * sm * t;
  sf[6] = rm * s * t;
  sf[7] = r * s * t;
}

void vtkVoxel::InterpolationDerivs(float pcoords[3], float derivs[24])
{
  double rm, sm, tm;

  rm = 1. - pcoords[0];
  sm = 1. - pcoords[1];
  tm = 1. - pcoords[2];

  // r derivatives
  derivs[0] = -sm*tm;
  derivs[1] = sm*tm;
  derivs[2] = -pcoords[1]*tm;
  derivs[3] = pcoords[1]*tm;
  derivs[4] = -sm*pcoords[2];
  derivs[5] = sm*pcoords[2];
  derivs[6] = -pcoords[1]*pcoords[2];
  derivs[7] = pcoords[1]*pcoords[2];

  // s derivatives
  derivs[8] = -rm*tm;
  derivs[9] = -pcoords[0]*tm;
  derivs[10] = rm*tm;
  derivs[11] = pcoords[0]*tm;
  derivs[12] = -rm*pcoords[2];
  derivs[13] = -pcoords[0]*pcoords[2];
  derivs[14] = rm*pcoords[2];
  derivs[15] = pcoords[0]*pcoords[2];

  // t derivatives
  derivs[16] = -rm*sm;
  derivs[17] = -pcoords[0]*sm;
  derivs[18] = -rm*pcoords[1];
  derivs[19] = -pcoords[0]*pcoords[1];
  derivs[20] = rm*sm;
  derivs[21] = pcoords[0]*sm;
  derivs[22] = rm*pcoords[1];
  derivs[23] = pcoords[0]*pcoords[1];
}

int vtkVoxel::CellBoundary(int vtkNotUsed(subId), float pcoords[3],
			   vtkIdList& pts)
{
  float t1=pcoords[0]-pcoords[1];
  float t2=1.0-pcoords[0]-pcoords[1];
  float t3=pcoords[1]-pcoords[2];
  float t4=1.0-pcoords[1]-pcoords[2];
  float t5=pcoords[2]-pcoords[0];
  float t6=1.0-pcoords[2]-pcoords[0];

  pts.SetNumberOfIds(4);

  // compare against six planes in parametric space that divide element
  // into six pieces.
  if ( t3 >= 0.0 && t4 >= 0.0 && t5 < 0.0 && t6 >= 0.0 )
    {
    pts.SetId(0,this->PointIds.GetId(0));
    pts.SetId(1,this->PointIds.GetId(1));
    pts.SetId(2,this->PointIds.GetId(3));
    pts.SetId(3,this->PointIds.GetId(2));
    }

  else if ( t1 >= 0.0 && t2 < 0.0 && t5 < 0.0 && t6 < 0.0 )
    {
    pts.SetId(0,this->PointIds.GetId(1));
    pts.SetId(1,this->PointIds.GetId(3));
    pts.SetId(2,this->PointIds.GetId(7));
    pts.SetId(3,this->PointIds.GetId(5));
    }

  else if ( t1 >= 0.0 && t2 >= 0.0 && t3 < 0.0 && t4 >= 0.0 )
    {
    pts.SetId(0,this->PointIds.GetId(0));
    pts.SetId(1,this->PointIds.GetId(1));
    pts.SetId(2,this->PointIds.GetId(5));
    pts.SetId(3,this->PointIds.GetId(4));
    }

  else if ( t3 < 0.0 && t4 < 0.0 && t5 >= 0.0 && t6 < 0.0 )
    {
    pts.SetId(0,this->PointIds.GetId(4));
    pts.SetId(1,this->PointIds.GetId(5));
    pts.SetId(2,this->PointIds.GetId(7));
    pts.SetId(3,this->PointIds.GetId(6));
    }

  else if ( t1 < 0.0 && t2 >= 0.0 && t5 >= 0.0 && t6 >= 0.0 )
    {
    pts.SetId(0,this->PointIds.GetId(0));
    pts.SetId(1,this->PointIds.GetId(4));
    pts.SetId(2,this->PointIds.GetId(6));
    pts.SetId(3,this->PointIds.GetId(2));
    }

  else // if ( t1 < 0.0 && t2 < 0.0 && t3 >= 0.0 && t6 < 0.0 )
    {
    pts.SetId(0,this->PointIds.GetId(3));
    pts.SetId(1,this->PointIds.GetId(2));
    pts.SetId(2,this->PointIds.GetId(6));
    pts.SetId(3,this->PointIds.GetId(7));
    }

  if ( pcoords[0] < 0.0 || pcoords[0] > 1.0 ||
  pcoords[1] < 0.0 || pcoords[1] > 1.0 || pcoords[2] < 0.0 || pcoords[2] > 1.0 )
    return 0;
  else
    return 1;
}

static int edges[12][2] = { {0,1}, {1,3}, {2,3}, {0,2},
                            {4,5}, {5,7}, {6,7}, {4,6},
                            {0,4}, {1,5}, {2,6}, {3,7}};
// define in terms vtkPixel understands
static int faces[6][4] = { {0,2,4,6}, {1,3,5,7},
                           {0,1,4,5}, {2,3,6,7},
                           {0,1,2,3}, {4,5,6,7} };

//
// Marching cubes case table
//
#include "vtkMarchingCubesCases.h"

void vtkVoxel::Contour(float value, vtkFloatScalars *cellScalars, 
		       vtkPointLocator *locator,
		       vtkCellArray *vtkNotUsed(verts), 
		       vtkCellArray *vtkNotUsed(lines), 
		       vtkCellArray *polys,
                       vtkPointData *inPd, vtkPointData *outPd)
{
  static int CASE_MASK[8] = {1,2,4,8,16,32,64,128};
  TRIANGLE_CASES *triCase;
  EDGE_LIST  *edge;
  int i, j, index, *vert;
  static int vertMap[8] = { 0, 1, 3, 2, 4, 5, 7, 6 };
  int pts[3];
  float t, *x1, *x2, x[3];

  // Build the case table
  for ( i=0, index = 0; i < 8; i++)
      if (cellScalars->GetScalar(vertMap[i]) >= value)
          index |= CASE_MASK[i];

  triCase = triCases + index;
  edge = triCase->edges;

  for ( ; edge[0] > -1; edge += 3 )
    {
    for (i=0; i<3; i++) // insert triangle
      {
      vert = edges[edge[i]];
      t = (value - cellScalars->GetScalar(vert[0])) /
          (cellScalars->GetScalar(vert[1]) - cellScalars->GetScalar(vert[0]));
      x1 = this->Points.GetPoint(vert[0]);
      x2 = this->Points.GetPoint(vert[1]);
      for (j=0; j<3; j++) x[j] = x1[j] + t * (x2[j] - x1[j]);
      if ( (pts[i] = locator->IsInsertedPoint(x)) < 0 )
        {
        pts[i] = locator->InsertNextPoint(x);
        if ( outPd ) 
          {
          int p1 = this->PointIds.GetId(vert[0]);
          int p2 = this->PointIds.GetId(vert[1]);
          outPd->InterpolateEdge(inPd,pts[i],p1,p2,t);
          }
        }
      }
    polys->InsertNextCell(3,pts);
    }
}


vtkCell *vtkVoxel::GetEdge(int edgeId)
{
  int *verts;
  static vtkLine line;
  
  verts = edges[edgeId];

  // load point id's
  line.PointIds.SetId(0,this->PointIds.GetId(verts[0]));
  line.PointIds.SetId(1,this->PointIds.GetId(verts[1]));

  // load coordinates
  line.Points.SetPoint(0,this->Points.GetPoint(verts[0]));
  line.Points.SetPoint(1,this->Points.GetPoint(verts[1]));

  return &line;
}

vtkCell *vtkVoxel::GetFace(int faceId)
{
  static vtkPixel pixel;
  int *verts, i;

  verts = faces[faceId];

  for (i=0; i<4; i++)
    {
    pixel.PointIds.SetId(i,this->PointIds.GetId(verts[i]));
    pixel.Points.SetPoint(i,this->Points.GetPoint(verts[i]));
    }

  return &pixel;
}

// 
// Intersect voxel with line using "bounding box" intersection.
//
int vtkVoxel::IntersectWithLine(float p1[3], float p2[3], float vtkNotUsed(tol), 
                               float& t, float x[3], float pcoords[3], int& subId)
{
  float *minPt, *maxPt;
  float bounds[6], p21[3];
  int i;

  subId = 0;

  minPt = this->Points.GetPoint(0);
  maxPt = this->Points.GetPoint(7);

  for (i=0; i<3; i++)
    {
    p21[i] = p2[i] - p1[i];
    bounds[2*i] = minPt[i];
    bounds[2*i+1] = maxPt[i];
    }

  if ( ! vtkCell::HitBBox(bounds, p1, p21, x, t) )
    return 0;
//
// Evaluate intersection
//
  for (i=0; i<3; i++)
    pcoords[i] = (x[i] - minPt[i]) / (maxPt[i] - minPt[i]);

  return 1;
}

int vtkVoxel::Triangulate(int index, vtkIdList &ptIds, vtkFloatPoints &pts)
{
  int p[4], i;

  ptIds.Reset();
  pts.Reset();
//
// Create five tetrahedron. Triangulation varies depending upon index. This
// is necessary to insure compatible voxel triangulations.
//
  if ( (index % 2) )
    {
    p[0] = 0; p[1] = 1; p[2] = 4; p[3] = 2;
    for ( i=0; i < 4; i++ )
      {
      ptIds.InsertNextId(this->PointIds.GetId(p[i]));
      pts.InsertNextPoint(this->Points.GetPoint(p[i]));
      }

    p[0] = 1; p[1] = 4; p[2] = 7; p[3] = 5;
    for ( i=0; i < 4; i++ )
      {
      ptIds.InsertNextId(this->PointIds.GetId(p[i]));
      pts.InsertNextPoint(this->Points.GetPoint(p[i]));
      }

    p[0] = 1; p[1] = 4; p[2] = 2; p[3] = 7;
    for ( i=0; i < 4; i++ )
      {
      ptIds.InsertNextId(this->PointIds.GetId(p[i]));
      pts.InsertNextPoint(this->Points.GetPoint(p[i]));
      }

    p[0] = 1; p[1] = 2; p[2] = 3; p[3] = 7;
    for ( i=0; i < 4; i++ )
      {
      ptIds.InsertNextId(this->PointIds.GetId(p[i]));
      pts.InsertNextPoint(this->Points.GetPoint(p[i]));
      }

    p[0] = 2; p[1] = 7; p[2] = 4; p[3] = 6;
    for ( i=0; i < 4; i++ )
      {
      ptIds.InsertNextId(this->PointIds.GetId(p[i]));
      pts.InsertNextPoint(this->Points.GetPoint(p[i]));
      }
    }
  else
    {
    p[0] = 3; p[1] = 1; p[2] = 0; p[3] = 5;
    for ( i=0; i < 4; i++ )
      {
      ptIds.InsertNextId(this->PointIds.GetId(p[i]));
      pts.InsertNextPoint(this->Points.GetPoint(p[i]));
      }

    p[0] = 0; p[1] = 3; p[2] = 6; p[3] = 2;
    for ( i=0; i < 4; i++ )
      {
      ptIds.InsertNextId(this->PointIds.GetId(p[i]));
      pts.InsertNextPoint(this->Points.GetPoint(p[i]));
      }

    p[0] = 3; p[1] = 5; p[2] = 6; p[3] = 7;
    for ( i=0; i < 4; i++ )
      {
      ptIds.InsertNextId(this->PointIds.GetId(p[i]));
      pts.InsertNextPoint(this->Points.GetPoint(p[i]));
      }

    p[0] = 0; p[1] = 6; p[2] = 5; p[3] = 4;
    for ( i=0; i < 4; i++ )
      {
      ptIds.InsertNextId(this->PointIds.GetId(p[i]));
      pts.InsertNextPoint(this->Points.GetPoint(p[i]));
      }

    p[0] = 1; p[1] = 3; p[2] = 5; p[3] = 6;
    for ( i=0; i < 4; i++ )
      {
      ptIds.InsertNextId(this->PointIds.GetId(p[i]));
      pts.InsertNextPoint(this->Points.GetPoint(p[i]));
      }
    }

  return 1;
}

void vtkVoxel::Derivatives(int vtkNotUsed(subId), float pcoords[3], 
                           float *values, int dim, float *derivs)
{
  float functionDerivs[24], sum;
  int i, j, k;
  float *x0, *x1, *x2, *x4, spacing[3];

  x0 = this->Points.GetPoint(0);
  x1 = this->Points.GetPoint(1);
  spacing[0] = x1[0] - x0[0];

  x2 = this->Points.GetPoint(2);
  spacing[1] = x2[1] - x0[1];

  x4 = this->Points.GetPoint(4);
  spacing[2] = x4[2] - x0[2];

  // get derivatives in r-s-t directions
  this->InterpolationDerivs(pcoords, functionDerivs);

  // since the x-y-z axes are aligned with r-s-t axes, only need to scale
  // the derivative values by the data spacing.
  for (k=0; k < dim; k++) //loop over values per vertex
    {
    for (j=0; j < 3; j++) //loop over derivative directions
      {
      for (sum=0.0, i=0; i < 8; i++) //loop over interp. function derivatives
        {
        sum += functionDerivs[8*j + i] * values[dim*i + k];
        }
      derivs[3*k + j] = sum / spacing[j];
      }
    }
}

void vtkVoxel::Clip(float vtkNotUsed(value), 
			 vtkFloatScalars *vtkNotUsed(cellScalars), 
                         vtkPointLocator *vtkNotUsed(locator), 
			 vtkCellArray *vtkNotUsed(tetras),
                         vtkPointData *vtkNotUsed(inPd), 
			 vtkPointData *vtkNotUsed(outPd),
                         int insideOut)
{

}

