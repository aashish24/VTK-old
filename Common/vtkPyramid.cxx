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
#include "vtkPyramid.h"
#include "vtkMath.h"
#include "vtkCellArray.h"
#include "vtkPointLocator.h"
#include "vtkUnstructuredGrid.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkPyramid* vtkPyramid::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkPyramid");
  if(ret)
    {
    return (vtkPyramid*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkPyramid;
}




// Construct the pyramid with five points.
vtkPyramid::vtkPyramid()
{
  this->Points->SetNumberOfPoints(5);
  this->PointIds->SetNumberOfIds(5);
  this->Line = vtkLine::New();
  this->Triangle = vtkTriangle::New();
  this->Quad = vtkQuad::New();
}

vtkPyramid::~vtkPyramid()
{
  this->Line->Delete();
  this->Triangle->Delete();
  this->Quad->Delete();
}

vtkCell *vtkPyramid::MakeObject()
{
  vtkCell *cell = vtkPyramid::New();
  cell->DeepCopy(*this);
  return cell;
}

#define VTK_MAX_ITERATION 10
#define VTK_CONVERGED 1.e-03
int vtkPyramid::EvaluatePosition(float x[3], float closestPoint[3],
                              int& subId, float pcoords[3], 
                              float& dist2, float *weights)
{
  int iteration, converged;
  float  params[3];
  float  fcol[3], rcol[3], scol[3], tcol[3];
  int i, j;
  float  d, *pt;
  float derivs[15];

  //  set initial position for Newton's method
  subId = 0;
  pcoords[0] = pcoords[1] = pcoords[2] = 0.5;
  params[0] = params[1] = params[2] = 0.3333333;

  //  enter iteration loop
  for (iteration=converged=0; !converged && (iteration < VTK_MAX_ITERATION);
  iteration++) 
    {
    //  calculate element interpolation functions and derivatives
    this->InterpolationFunctions(pcoords, weights);
    this->InterpolationDerivs(pcoords, derivs);

    //  calculate newton functions
    for (i=0; i<3; i++) 
      {
      fcol[i] = rcol[i] = scol[i] = tcol[i] = 0.0;
      }
    for (i=0; i<5; i++)
      {
      pt = this->Points->GetPoint(i);
      for (j=0; j<3; j++)
        {
        fcol[j] += pt[j] * weights[i];
        rcol[j] += pt[j] * derivs[i];
        scol[j] += pt[j] * derivs[i+5];
        tcol[j] += pt[j] * derivs[i+10];
        }
      }

    for (i=0; i<3; i++) 
      {
      fcol[i] -= x[i];
      }

    //  compute determinants and generate improvements
    if ( (d=vtkMath::Determinant3x3(rcol,scol,tcol)) == 0.0 ) 
      {
      return -1;
      }

    pcoords[0] = params[0] - vtkMath::Determinant3x3 (fcol,scol,tcol) / d;
    pcoords[1] = params[1] - vtkMath::Determinant3x3 (rcol,fcol,tcol) / d;
    pcoords[2] = params[2] - vtkMath::Determinant3x3 (rcol,scol,fcol) / d;

    //  check for convergence
    if ( ((fabs(pcoords[0]-params[0])) < VTK_CONVERGED) &&
    ((fabs(pcoords[1]-params[1])) < VTK_CONVERGED) &&
    ((fabs(pcoords[2]-params[2])) < VTK_CONVERGED) )
      {
      converged = 1;
      }

    //  if not converged, repeat
    else 
      {
      params[0] = pcoords[0];
      params[1] = pcoords[1];
      params[2] = pcoords[2];
      }
    }

  //  if not converged, set the parametric coordinates to arbitrary values
  //  outside of element
  if ( !converged ) 
    {
    return -1;
    }

  this->InterpolationFunctions(pcoords, weights);

  if ( pcoords[0] >= -0.001 && pcoords[0] <= 1.001 &&
  pcoords[1] >= -0.001 && pcoords[1] <= 1.001 &&
  pcoords[2] >= -0.001 && pcoords[2] <= 1.001 )
    {
    closestPoint[0] = x[0]; closestPoint[1] = x[1]; closestPoint[2] = x[2];
    dist2 = 0.0; //inside pyramid
    return 1;
    }
  else
    {
    float pc[3], w[5];
    for (i=0; i<3; i++) //only approximate, not really true for warped hexa
      {
      if (pcoords[i] < 0.0) 
	{
	pc[i] = 0.0;
	}
      else if (pcoords[i] > 1.0) 
	{
	pc[i] = 1.0;
	}
      else 
	{
	pc[i] = pcoords[i];
	}
      }
    this->EvaluateLocation(subId, pc, closestPoint, (float *)w);
    dist2 = vtkMath::Distance2BetweenPoints(closestPoint,x);
    return 0;
    }
}

void vtkPyramid::EvaluateLocation(int& vtkNotUsed(subId), float pcoords[3], 
                                float x[3], float *weights)
{
  int i, j;
  float *pt;

  this->InterpolationFunctions(pcoords, weights);

  x[0] = x[1] = x[2] = 0.0;
  for (i=0; i<5; i++)
    {
    pt = this->Points->GetPoint(i);
    for (j=0; j<3; j++)
      {
      x[j] += pt[j] * weights[i];
      }
    }
}

// Returns the closest face to the point specified. Closeness is measured
// parametrically.
int vtkPyramid::CellBoundary(int vtkNotUsed(subId), float pcoords[3], 
                           vtkIdList *pts)
{
  int i;

  // define 6 planes that separate regions
  static float normals[6][3] = { 
    {0.0,-0.5547002,0.8320503}, {0.5547002,0.0,0.8320503}, {0.0,0.5547002,0.8320503},
    {-0.5547002,0.0,0.8320503}, {0.70710670,-0.70710670,0.0}, {0.70710670,0.70710670,0.0} };
  static float point[3] = {0.5,0.5,0.3333333};
  float vals[6];

  // evaluate 6 plane equations
  for (i=0; i<6; i++)
    {
    vals[i] = normals[i][0]*(pcoords[0]-point[0]) + 
      normals[i][1]*(pcoords[1]-point[1]) + normals[i][2]*(pcoords[2]-point[2]);
    }

  // compare against six planes in parametric space that divide element
  // into five pieces (each corresponding to a face).
  if ( vals[4] >= 0.0 && vals[5] <= 0.0 && vals[0] >= 0.0 )
    {
    pts->SetNumberOfIds(3); //triangle face
    pts->SetId(0,this->PointIds->GetId(0));
    pts->SetId(1,this->PointIds->GetId(1));
    pts->SetId(2,this->PointIds->GetId(4));
    }

  else if ( vals[4] >= 0.0 && vals[5] >= 0.0 && vals[1] >= 0.0 )
    {
    pts->SetNumberOfIds(3); //triangle face
    pts->SetId(0,this->PointIds->GetId(1));
    pts->SetId(1,this->PointIds->GetId(2));
    pts->SetId(2,this->PointIds->GetId(4));
    }

  else if ( vals[4] <= 0.0 && vals[5] >= 0.0 && vals[2] >= 0.0 )
    {
    pts->SetNumberOfIds(3); //triangle face
    pts->SetId(0,this->PointIds->GetId(2));
    pts->SetId(1,this->PointIds->GetId(3));
    pts->SetId(2,this->PointIds->GetId(4));
    }

  else if ( vals[4] <= 0.0 && vals[5] <= 0.0 && vals[3] >= 0.0 )
    {
    pts->SetNumberOfIds(3); //triangle face
    pts->SetId(0,this->PointIds->GetId(3));
    pts->SetId(1,this->PointIds->GetId(0));
    pts->SetId(2,this->PointIds->GetId(4));
    }

  else 
    {
    pts->SetNumberOfIds(4); //quad face
    pts->SetId(0,this->PointIds->GetId(0));
    pts->SetId(1,this->PointIds->GetId(1));
    pts->SetId(2,this->PointIds->GetId(2));
    pts->SetId(3,this->PointIds->GetId(3));
    }

  if ( pcoords[0] < 0.0 || pcoords[0] > 1.0 ||
       pcoords[1] < 0.0 || pcoords[1] > 1.0 || 
       pcoords[2] < 0.0 || pcoords[2] > 1.0 )
    {
    return 0;
    }
  else
    {
    return 1;
    }
}

// Marching pyramids (contouring)
//
static int edges[8][2] = { {0,1}, {1,2}, {2,3}, 
                           {3,0}, {0,4}, {1,4},
                           {2,4}, {3,4} };
static int faces[5][4] = { {0,3,2,1}, {0,1,4,-1}, 
			   {1,2,4,-1}, {2,3,4,-1}, {3,0,4,-1} };

typedef int EDGE_LIST;
typedef struct {
       EDGE_LIST edges[13];
} TRIANGLE_CASES;

static TRIANGLE_CASES triCases[] = { 
  {{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}}, //0
  {{ 0,  4,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}}, //1
  {{ 0,  1,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}}, //2
  {{ 4,  1,  5,  4,  3,  1, -1, -1, -1, -1, -1, -1, -1}}, //3
  {{ 1,  2,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}}, //4
  {{ 0,  4,  3,  1,  2,  6, -1, -1, -1, -1, -1, -1, -1}}, //5
  {{ 0,  2,  5,  5,  2,  6, -1, -1, -1, -1, -1, -1, -1}}, //6
  {{ 4,  3,  2,  5,  4,  2,  6,  5,  2, -1, -1, -1, -1}}, //7
  {{ 3,  7,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}}, //8
  {{ 4,  7,  2,  2,  0,  4, -1, -1, -1, -1, -1, -1, -1}}, //9
  {{ 0,  1,  5,  3,  7,  2, -1, -1, -1, -1, -1, -1, -1}}, //10
  {{ 1,  5,  2,  2,  5,  4,  2,  4,  7, -1, -1, -1, -1}}, //11
  {{ 1,  3,  6,  6,  3,  7, -1, -1, -1, -1, -1, -1, -1}}, //12
  {{ 1,  7,  6,  1,  4,  7,  1,  0,  4, -1, -1, -1, -1}}, //13
  {{ 0,  3,  5,  5,  3,  6,  6,  3,  7, -1, -1, -1, -1}}, //14
  {{ 5,  4,  7,  6,  5,  7, -1, -1, -1, -1, -1, -1, -1}}, //15
  {{ 4,  5,  6,  4,  6,  7, -1, -1, -1, -1, -1, -1, -1}}, //16
  {{ 0,  7,  3,  0,  5,  7,  5,  6,  7, -1, -1, -1, -1}}, //17
  {{ 0,  1,  4,  1,  7,  4,  0,  6,  7, -1, -1, -1, -1}}, //18
  {{ 3,  1,  7,  1,  6,  7, -1, -1, -1, -1, -1, -1, -1}}, //19
  {{ 1,  2,  5,  2,  4,  5,  2,  7,  4, -1, -1, -1, -1}}, //20
  {{ 0,  7,  3,  0,  5,  7,  5,  2,  7,  5,  1,  2, -1}}, //21
  {{ 0,  2,  4,  4,  2,  7, -1, -1, -1, -1, -1, -1, -1}}, //22
  {{ 3,  2,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}}, //23
  {{ 3,  4,  2,  2,  4,  5,  2,  5,  6, -1, -1, -1, -1}}, //24
  {{ 0,  5,  2,  5,  6,  2, -1, -1, -1, -1, -1, -1, -1}}, //25
  {{ 0,  1,  6,  0,  6,  4,  4,  6,  3,  6,  2,  3, -1}}, //26
  {{ 1,  6,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}}, //27
  {{ 3,  4,  1,  4,  5,  1, -1, -1, -1, -1, -1, -1, -1}}, //28
  {{ 0,  5,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}}, //29
  {{ 0,  3,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}}, //30
  {{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}}  //31
};

void vtkPyramid::Contour(float value, vtkScalars *cellScalars, 
                       vtkPointLocator *locator,
                       vtkCellArray *vtkNotUsed(verts), 
                       vtkCellArray *vtkNotUsed(lines), 
                       vtkCellArray *polys,
                       vtkPointData *inPd, vtkPointData *outPd,
                       vtkCellData *inCd, int cellId, vtkCellData *outCd)
{
  static int CASE_MASK[5] = {1,2,4,8,16};
  TRIANGLE_CASES *triCase;
  EDGE_LIST  *edge;
  int i, j, index, *vert, newCellId;
  int pts[3];
  float t, *x1, *x2, x[3];

  // Build the case table
  for ( i=0, index = 0; i < 5; i++)
    {
    if (cellScalars->GetScalar(i) >= value)
      {
      index |= CASE_MASK[i];
      }
    }

  triCase = triCases + index;
  edge = triCase->edges;

  for ( ; edge[0] > -1; edge += 3 )
    {
    for (i=0; i<3; i++) // insert triangle
      {
      vert = edges[edge[i]];
      t = (value - cellScalars->GetScalar(vert[0])) /
          (cellScalars->GetScalar(vert[1]) - cellScalars->GetScalar(vert[0]));
      x1 = this->Points->GetPoint(vert[0]);
      x2 = this->Points->GetPoint(vert[1]);
      for (j=0; j<3; j++) 
	{
	x[j] = x1[j] + t * (x2[j] - x1[j]);
	}
      if ( (pts[i] = locator->IsInsertedPoint(x)) < 0 )
        {
        pts[i] = locator->InsertNextPoint(x);
        if ( outPd ) 
          {
          int p1 = this->PointIds->GetId(vert[0]);
          int p2 = this->PointIds->GetId(vert[1]);
          outPd->InterpolateEdge(inPd,pts[i],p1,p2,t);
          }
        }
      }
    // check for degenerate triangle
    if ( pts[0] != pts[1] &&
         pts[0] != pts[2] &&
         pts[1] != pts[2] )
      {
      newCellId = polys->InsertNextCell(3,pts);
      outCd->CopyData(inCd,cellId,newCellId);
      }
    }
}

vtkCell *vtkPyramid::GetEdge(int edgeId)
{
  int *verts;

  verts = edges[edgeId];

  // load point id's
  this->Line->PointIds->SetId(0,this->PointIds->GetId(verts[0]));
  this->Line->PointIds->SetId(1,this->PointIds->GetId(verts[1]));

  // load coordinates
  this->Line->Points->SetPoint(0,this->Points->GetPoint(verts[0]));
  this->Line->Points->SetPoint(1,this->Points->GetPoint(verts[1]));

  return this->Line;
}

vtkCell *vtkPyramid::GetFace(int faceId)
{
  int *verts;

  verts = faces[faceId];

  if ( verts[3] != -1 ) // quad cell
    {
    // load point id's
    this->Quad->PointIds->SetId(0,this->PointIds->GetId(verts[0]));
    this->Quad->PointIds->SetId(1,this->PointIds->GetId(verts[1]));
    this->Quad->PointIds->SetId(2,this->PointIds->GetId(verts[2]));
    this->Quad->PointIds->SetId(3,this->PointIds->GetId(verts[3]));

    // load coordinates
    this->Quad->Points->SetPoint(0,this->Points->GetPoint(verts[0]));
    this->Quad->Points->SetPoint(1,this->Points->GetPoint(verts[1]));
    this->Quad->Points->SetPoint(2,this->Points->GetPoint(verts[2]));
    this->Quad->Points->SetPoint(3,this->Points->GetPoint(verts[3]));

    return this->Quad;
    }
  else
    {
    // load point id's
    this->Triangle->PointIds->SetId(0,this->PointIds->GetId(verts[0]));
    this->Triangle->PointIds->SetId(1,this->PointIds->GetId(verts[1]));
    this->Triangle->PointIds->SetId(2,this->PointIds->GetId(verts[2]));

    // load coordinates
    this->Triangle->Points->SetPoint(0,this->Points->GetPoint(verts[0]));
    this->Triangle->Points->SetPoint(1,this->Points->GetPoint(verts[1]));
    this->Triangle->Points->SetPoint(2,this->Points->GetPoint(verts[2]));

    return this->Triangle;
    }
}

// Intersect faces against line.
//
int vtkPyramid::IntersectWithLine(float p1[3], float p2[3], float tol, float& t,
                               float x[3], float pcoords[3], int& subId)
{
  int intersection=0;
  float *pt1, *pt2, *pt3, *pt4;
  float tTemp;
  float pc[3], xTemp[3], dist2, weights[5];

  int faceNum;

  t = VTK_LARGE_FLOAT;

  //first intersect the triangle faces
  for (faceNum=1; faceNum<5; faceNum++)
    {
    pt1 = this->Points->GetPoint(faces[faceNum][0]);
    pt2 = this->Points->GetPoint(faces[faceNum][1]);
    pt3 = this->Points->GetPoint(faces[faceNum][2]);

    this->Triangle->Points->SetPoint(0,pt1);
    this->Triangle->Points->SetPoint(1,pt2);
    this->Triangle->Points->SetPoint(2,pt3);

    if ( this->Triangle->IntersectWithLine(p1, p2, tol, tTemp, xTemp, pc, subId) )
      {
      intersection = 1;
      if ( tTemp < t )
        {
        t = tTemp;
        x[0] = xTemp[0]; x[1] = xTemp[1]; x[2] = xTemp[2]; 
	this->EvaluatePosition(x, xTemp, subId, pcoords, dist2, weights);
        }
      }
    }

  //now intersect the quad face
  pt1 = this->Points->GetPoint(faces[0][0]);
  pt2 = this->Points->GetPoint(faces[0][1]);
  pt3 = this->Points->GetPoint(faces[0][2]);
  pt4 = this->Points->GetPoint(faces[0][3]);

  this->Quad->Points->SetPoint(0,pt1);
  this->Quad->Points->SetPoint(1,pt2);
  this->Quad->Points->SetPoint(2,pt3);
  this->Quad->Points->SetPoint(3,pt4);

  if ( this->Quad->IntersectWithLine(p1, p2, tol, tTemp, xTemp, pc, subId) )
    {
    intersection = 1;
    if ( tTemp < t )
      {
      t = tTemp;
      x[0] = xTemp[0]; x[1] = xTemp[1]; x[2] = xTemp[2]; 
      pcoords[0] = pc[0]; pcoords[1] = pc[1]; pcoords[2] = 0.0;
      }
    }

  return intersection;
}

int vtkPyramid::Triangulate(int vtkNotUsed(index), vtkIdList *ptIds, vtkPoints *pts)
{
  ptIds->Reset();
  pts->Reset();
    
  for ( int i=0; i < 4; i++ )
    {
    ptIds->InsertId(i,this->PointIds->GetId(i));
    pts->InsertPoint(i,this->Points->GetPoint(i));
    }

  return 1;
}


void vtkPyramid::Derivatives(int vtkNotUsed(subId), float pcoords[3],
			     float *values, int dim, float *derivs)
{
  double *jI[3], j0[3], j1[3], j2[3];
  float functionDerivs[15], sum[3], value;
  int i, j, k;

  // compute inverse Jacobian and interpolation function derivatives
  jI[0] = j0; jI[1] = j1; jI[2] = j2;
  this->JacobianInverse(pcoords, jI, functionDerivs);

  // now compute derivates of values provided
  for (k=0; k < dim; k++) //loop over values per vertex
    {
    sum[0] = sum[1] = sum[2] = 0.0;
    for ( i=0; i < 5; i++) //loop over interp. function derivatives
      {
      value = values[dim*i + k];
      sum[0] += functionDerivs[i] * value;
      sum[1] += functionDerivs[5 + i] * value;
      sum[2] += functionDerivs[10 + i] * value;
      }

    for (j=0; j < 3; j++) //loop over derivative directions
      {
      derivs[3*k + j] = sum[0]*jI[0][j] + sum[1]*jI[1][j] + sum[2]*jI[2][j];
      }
    }
}

// Compute iso-parametrix interpolation functions for pyramid
//
void vtkPyramid::InterpolationFunctions(float pcoords[3], float sf[5])
{
  if ( pcoords[2] == 1.0 )
    {
    sf[0] = sf[1] = sf[2] = sf[3] = 0.0;
    sf[4] = 1.0;
    }
  else
    {
    float f1 = (1.0 - pcoords[2]);
    float f2 = (2.0 * pcoords[0] - 1.0) / (1.0 - pcoords[2]);
    float f3 = (2.0 * pcoords[1] - 1.0) / (1.0 - pcoords[2]);
    sf[0] = 0.25 * f1 * (1.0 - f2) * (1.0 - f3);
    sf[1] = 0.25 * f1 * (1.0 + f2) * (1.0 - f3);
    sf[2] = 0.25 * f1 * (1.0 + f2) * (1.0 + f3);
    sf[3] = 0.25 * f1 * (1.0 - f2) * (1.0 + f3);
    sf[4] = pcoords[2];
    }
}

void vtkPyramid::InterpolationDerivs(float pcoords[3], float derivs[15])
{
  float f2 = (2.0 * pcoords[0] - 1.0) / (1.0 - pcoords[2]);
  float f3 = (2.0 * pcoords[1] - 1.0) / (1.0 - pcoords[2]);

  // r-derivatives
  derivs[0] = -0.5 * (1.0 - f3);
  derivs[1] =  0.5 * (1.0 - f3);
  derivs[2] =  0.5 * (1.0 + f3);
  derivs[3] = -0.5 * (1.0 + f3);
  derivs[4] = 0.0;

  // s-derivatives
  derivs[5] = -0.5 * (1.0 - f2);
  derivs[6] = -0.5 * (1.0 + f2);
  derivs[7] =  0.5 * (1.0 + f2);
  derivs[8] =  0.5 * (1.0 - f2);
  derivs[9] = 0.0;

  // t-derivatives
  derivs[10] = -0.25 * (1.0 - f2) * (1.0 - f3);
  derivs[11] = -0.25 * (1.0 + f2) * (1.0 - f3);
  derivs[12] = -0.25 * (1.0 + f2) * (1.0 + f3);
  derivs[13] = -0.25 * (1.0 - f2) * (1.0 + f3);
  derivs[14] = 1.0;
}

// Given parametric coordinates compute inverse Jacobian transformation
// matrix. Returns 9 elements of 3x3 inverse Jacobian plus interpolation
// function derivatives. Returns 0 if no inverse exists.
int vtkPyramid::JacobianInverse(float pcoords[3], double **inverse, float derivs[15])
{
  int i, j;
  double *m[3], m0[3], m1[3], m2[3];
  float *x;

  // compute interpolation function derivatives
  this->InterpolationDerivs(pcoords,derivs);

  // create Jacobian matrix
  m[0] = m0; m[1] = m1; m[2] = m2;
  for (i=0; i < 3; i++) //initialize matrix
    {
    m0[i] = m1[i] = m2[i] = 0.0;
    }

  for ( j=0; j < 5; j++ )
    {
    x = this->Points->GetPoint(j);
    for ( i=0; i < 3; i++ )
      {
      m0[i] += x[i] * derivs[j];
      m1[i] += x[i] * derivs[5 + j];
      m2[i] += x[i] * derivs[10 + j];
      }
    }

  // now find the inverse
  if ( vtkMath::InvertMatrix(m,inverse,3) == 0 )
    {
#define VTK_MAX_WARNS 3    
    static int numWarns=0;
    if ( numWarns++ < VTK_MAX_WARNS )
      {
      vtkErrorMacro(<<"Jacobian inverse not found");
      vtkErrorMacro(<<"Matrix:" << m[0][0] << " " << m[0][1] << " " << m[0][2]
      << m[1][0] << " " << m[1][1] << " " << m[1][2] 
      << m[2][0] << " " << m[2][1] << " " << m[2][2] );
      return 0;
      }
    }

  return 1;
}

// Clip this pyramid using scalar value provided. Like contouring, except
// that it cuts the pyramid to produce other pyramids or tetrahedra.
void vtkPyramid::Clip(float vtkNotUsed(value), 
		    vtkScalars *vtkNotUsed(cellScalars), 
		    vtkPointLocator *vtkNotUsed(locator), 
		    vtkCellArray *vtkNotUsed(tetras),
		    vtkPointData *vtkNotUsed(inPd), 
		    vtkPointData *vtkNotUsed(outPd),
		    vtkCellData *vtkNotUsed(inCd), int cellId, 
		    vtkCellData *vtkNotUsed(outCd), int insideOut)
{

}
