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
#include "vtkQuadraticPyramid.h"

#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkTetra.h"
#include "vtkPyramid.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointLocator.h"
#include "vtkPolyData.h"
#include "vtkQuadraticEdge.h"
#include "vtkQuadraticQuad.h"
#include "vtkQuadraticTriangle.h"

vtkCxxRevisionMacro(vtkQuadraticPyramid, "$Revision$");
vtkStandardNewMacro(vtkQuadraticPyramid);

// Construct the wedge with 13 points + 1 extra point for internal
// computation.
vtkQuadraticPyramid::vtkQuadraticPyramid()
{
  int i;

  // At creation time the cell looks like it has 14 points (during interpolation)
  // We initially allocate for 14.
  this->Points->SetNumberOfPoints(14);
  this->PointIds->SetNumberOfIds(14);
  for (i = 0; i < 14; i++)
    {
    this->Points->SetPoint(i, 0.0, 0.0, 0.0);
    this->PointIds->SetId(i,0);
    }
  this->Points->SetNumberOfPoints(13);
  this->PointIds->SetNumberOfIds(13);

  this->Edge = vtkQuadraticEdge::New();
  this->Face = vtkQuadraticQuad::New();
  this->TriangleFace = vtkQuadraticTriangle::New();
  this->Tetra = vtkTetra::New();
  this->Pyramid = vtkPyramid::New();

  this->PointData = vtkPointData::New();
  this->CellData = vtkCellData::New();
  this->Scalars = vtkDoubleArray::New();
  this->Scalars->SetNumberOfTuples(5);  //num of vertices
}

vtkQuadraticPyramid::~vtkQuadraticPyramid()
{
  this->Edge->Delete();
  this->Face->Delete();
  this->TriangleFace->Delete();
  this->Tetra->Delete();
  this->Pyramid->Delete();

  this->PointData->Delete();
  this->CellData->Delete();
  this->Scalars->Delete();
}

static int LinearPyramids[10][5] = { {0,5,13,8,9},
                                     {5,1,6,13,10},
                                     {8,13,7,3,12},
                                     {13,6,2,7,11},
                                     {9,10,11,12,4},
                                     {9,12,11,10,13},
                                     {5,10,9,13,0},
                                     {6,11,10,13,0},
                                     {7,12,11,13,0},
                                     {8,9,12,13,0} };

static int PyramidFaces[5][8] = { {0,1,2,3,5,6,7,8}, 
                                  {0,1,4,5,10,9,0,0},
                                  {1,2,4,6,11,10,0,0},
                                  {2,3,4,7,12,11,0,0},
                                  {3,0,4,8,9,12,0,0}};

static int PyramidEdges[8][3] = { {0,1,5}, {1,2,6}, {2,3,7},
                                  {3,0,8},{0,4,9},{1,4,10},
                                  {2,4,11}, {3,4,12} };

static double MidPoints[1][3] = { {0.5,0.5,0.0} };

vtkCell *vtkQuadraticPyramid::GetEdge(int edgeId)
{
  edgeId = (edgeId < 0 ? 0 : (edgeId > 7 ? 7 : edgeId ));

  for (int i=0; i<3; i++)
    {
    this->Edge->PointIds->SetId(i,this->PointIds->GetId(PyramidEdges[edgeId][i]));
    this->Edge->Points->SetPoint(i,this->Points->GetPoint(PyramidEdges[edgeId][i]));
    }

  return this->Edge;
}

vtkCell *vtkQuadraticPyramid::GetFace(int faceId)
{
  faceId = (faceId < 0 ? 0 : (faceId > 4 ? 4 : faceId ));

  // load point id's and coordinates
  // be carefull with the first one:
  if(faceId > 0)
    {
    for (int i=0; i<6; i++)
      {
      this->TriangleFace->PointIds->SetId(i,this->PointIds->GetId(PyramidFaces[faceId][i]));
      this->TriangleFace->Points->SetPoint(i,this->Points->GetPoint(PyramidFaces[faceId][i]));
      }
    return this->TriangleFace;
    }
  else
    {
    for (int i=0; i<8; i++)
      {
      this->Face->PointIds->SetId(i,this->PointIds->GetId(PyramidFaces[faceId][i]));
      this->Face->Points->SetPoint(i,this->Points->GetPoint(PyramidFaces[faceId][i]));
      }
    return this->Face;
    }
}

static const double VTK_DIVERGED = 1.e6;
static const int VTK_PYRAMID_MAX_ITERATION=10;
static const double VTK_PYRAMID_CONVERGED=1.e-03;

int vtkQuadraticPyramid::EvaluatePosition(double* x, 
                                          double* closestPoint, 
                                          int& subId, double pcoords[3],
                                          double& dist2, double *weights)
{
  int iteration, converged;
  double  params[3];
  double  fcol[3], rcol[3], scol[3], tcol[3];
  int i, j;
  double  d, pt[3];
  double derivs[3*13];

  //  set initial position for Newton's method
  subId = 0;
  pcoords[0] = pcoords[1] = pcoords[2] = params[0] = params[1] = params[2]=0.5;

  //  enter iteration loop
  for (iteration=converged=0;
       !converged && (iteration < VTK_PYRAMID_MAX_ITERATION);  iteration++) 
    {
    //  calculate element interpolation functions and derivatives
    this->InterpolationFunctions(pcoords, weights);
    this->InterpolationDerivs(pcoords, derivs);

    //  calculate newton functions
    for (i=0; i<3; i++) 
      {
      fcol[i] = rcol[i] = scol[i] = tcol[i] = 0.0;
      }
    for (i=0; i<13; i++)
      {
      this->Points->GetPoint(i, pt);
      for (j=0; j<3; j++)
        {
        fcol[j] += pt[j] * weights[i];
        rcol[j] += pt[j] * derivs[i];
        scol[j] += pt[j] * derivs[i+13];
        tcol[j] += pt[j] * derivs[i+26];
        }
      }

    for (i=0; i<3; i++)
      {
      fcol[i] -= x[i];
      }

    //  compute determinants and generate improvements
    d=vtkMath::Determinant3x3(rcol,scol,tcol);
    if ( fabs(d) < 1.e-20) 
      {
      return -1;
      }

    pcoords[0] = params[0] - vtkMath::Determinant3x3 (fcol,scol,tcol) / d;
    pcoords[1] = params[1] - vtkMath::Determinant3x3 (rcol,fcol,tcol) / d;
    pcoords[2] = params[2] - vtkMath::Determinant3x3 (rcol,scol,fcol) / d;

    //  check for convergence
    if ( ((fabs(pcoords[0]-params[0])) < VTK_PYRAMID_CONVERGED) &&
         ((fabs(pcoords[1]-params[1])) < VTK_PYRAMID_CONVERGED) &&
         ((fabs(pcoords[2]-params[2])) < VTK_PYRAMID_CONVERGED) )
      {
      converged = 1;
      }

    // Test for bad divergence (S.Hirschberg 11.12.2001)
    else if ((fabs(pcoords[0]) > VTK_DIVERGED) || 
             (fabs(pcoords[1]) > VTK_DIVERGED) || 
             (fabs(pcoords[2]) > VTK_DIVERGED))
      {
      return -1;
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
    if (closestPoint)
      {
      closestPoint[0] = x[0]; closestPoint[1] = x[1]; closestPoint[2] = x[2];
      dist2 = 0.0; //inside pyramid
      }
    return 1;
    }
  else
    {
    double pc[3], w[13];
    if (closestPoint)
      {
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
      this->EvaluateLocation(subId, pc, closestPoint, (double *)w);
      dist2 = vtkMath::Distance2BetweenPoints(closestPoint,x);
      }
    return 0;
    }
}

void vtkQuadraticPyramid::EvaluateLocation(int& vtkNotUsed(subId), 
                                           double pcoords[3], 
                                           double x[3], double *weights)
{
  int i, j;
  double pt[3];

  this->InterpolationFunctions(pcoords, weights);

  x[0] = x[1] = x[2] = 0.0;
  for (i=0; i<13; i++)
    {
    this->Points->GetPoint(i, pt);
    for (j=0; j<3; j++)
      {
      x[j] += pt[j] * weights[i];
      }
    }
}

int vtkQuadraticPyramid::CellBoundary(int subId, double pcoords[3], 
                                      vtkIdList *pts)
{
  return this->Pyramid->CellBoundary(subId, pcoords, pts);
}

void vtkQuadraticPyramid::Subdivide(vtkPointData *inPd, vtkCellData *inCd, 
                                    vtkIdType cellId)
{
  int numMidPts, i, j;
  double weights[13];
  double x[3];

  //Copy point and cell attribute data
  this->PointData->CopyAllocate(inPd,14);
  this->CellData->CopyAllocate(inCd,5);
  for (i=0; i<13; i++)
    {
    this->PointData->CopyData(inPd,this->PointIds->GetId(i),i);
    }
  this->CellData->CopyData(inCd,cellId,0);
  
  //Interpolate new values
  this->PointIds->SetNumberOfIds(13);
  double *p;
  for ( numMidPts=0; numMidPts < 1; numMidPts++ )
    {
    this->InterpolationFunctions(MidPoints[numMidPts], weights);

    x[0] = 0.0;
    x[1] = 0.0;
    x[2] = 0.0;
    for (i=0; i<13; i++) 
      {
      p = this->Points->GetPoint(i);
      for (j=0; j<3; j++) 
        {
        x[j] += p[j] * weights[i];
        }
      }
    this->Points->SetPoint(13+numMidPts,x);
    this->PointData->InterpolatePoint(inPd, 13+numMidPts, 
                                      this->PointIds, weights);
    }
  this->PointIds->SetNumberOfIds(14);
}

void vtkQuadraticPyramid::Contour(double value, 
                                  vtkDataArray* vtkNotUsed(cellScalars), 
                                  vtkPointLocator* locator, 
                                  vtkCellArray *verts, 
                                  vtkCellArray* lines, 
                                  vtkCellArray* polys, 
                                  vtkPointData* inPd, 
                                  vtkPointData* outPd,
                                  vtkCellData* inCd, 
                                  vtkIdType cellId, 
                                  vtkCellData* outCd)
{
  int i;
  //subdivide into 6 linear pyramids
  this->Subdivide(inPd,inCd,cellId);
  
  //contour each linear pyramid separately
  vtkDataArray *localScalars = this->PointData->GetScalars();
  this->Scalars->SetNumberOfTuples(5);  //num of vertices
  for (i=0; i<6; i++) //for each pyramid
    {
    for (int j=0; j<5; j++) //for each point of pyramid
      {
      this->Pyramid->Points->SetPoint(j,this->Points->GetPoint(LinearPyramids[i][j]));
      this->Pyramid->PointIds->SetId(j,this->PointIds->GetId(LinearPyramids[i][j]));
      this->Scalars->SetValue(j,localScalars->GetTuple1(LinearPyramids[i][j]));
      }
    this->Pyramid->Contour(value,this->Scalars,locator,verts,lines,polys,
                       this->PointData,outPd,this->CellData,0,outCd);
    }

  //contour each linear tetra separately
  this->Scalars->SetNumberOfTuples(4);  //num of vertices
  for (i=6; i<10; i++) //for each tetra
    {
    for (int j=0; j<4; j++) //for each point of tetra
      {
      this->Tetra->Points->SetPoint(j,this->Points->GetPoint(LinearPyramids[i][j]));
      this->Tetra->PointIds->SetId(j,this->PointIds->GetId(LinearPyramids[i][j]));
      this->Scalars->SetValue(j,localScalars->GetTuple1(LinearPyramids[i][j]));
      }
    this->Tetra->Contour(value,this->Scalars,locator,verts,lines,polys,
                       this->PointData,outPd,this->CellData,0,outCd);
    }
}

// Line-hex intersection. Intersection has to occur within [0,1] parametric
// coordinates and with specified tolerance.
int vtkQuadraticPyramid::IntersectWithLine(double* p1, double* p2, 
                                           double tol, double& t,
                                           double* x, double* pcoords, int& subId)
{
  int intersection=0;
  double tTemp;
  double pc[3], xTemp[3];
  int faceNum;
  bool inter;

  t = VTK_DOUBLE_MAX;
  for (faceNum=0; faceNum<5; faceNum++)
    {
// We have 8 nodes on rect face
// and 6 on triangle faces
    if(faceNum > 0)
      {
      for (int i=0; i<6; i++)
        {
        this->TriangleFace->PointIds->SetId(i,
              this->PointIds->GetId(PyramidFaces[faceNum][i]));
        }
      inter = this->TriangleFace->IntersectWithLine(p1, p2, tol, tTemp, 
                                      xTemp, pc, subId);
      }
    else
      {
      for (int i=0; i<8; i++)
        {
        this->Face->Points->SetPoint(i,
              this->Points->GetPoint(PyramidFaces[faceNum][i]));
        }
      inter = this->Face->IntersectWithLine(p1, p2, tol, tTemp, 
                                      xTemp, pc, subId);
      }
    if ( inter )
      {
      intersection = 1;
      if ( tTemp < t )
        {
        t = tTemp;
        x[0] = xTemp[0]; x[1] = xTemp[1]; x[2] = xTemp[2]; 
        switch (faceNum)
          {
          case 0:
            pcoords[0] = 0.0; pcoords[1] = pc[1]; pcoords[2] = pc[0];
            break;

          case 1:
            pcoords[0] = 1.0; pcoords[1] = pc[0]; pcoords[2] = pc[1];
            break;

          case 2:
            pcoords[0] = pc[0]; pcoords[1] = 0.0; pcoords[2] = pc[1];
            break;

          case 3:
            pcoords[0] = pc[1]; pcoords[1] = 1.0; pcoords[2] = pc[0];
            break;

          case 4:
            pcoords[0] = pc[1]; pcoords[1] = pc[0]; pcoords[2] = 0.0;
            break;

          case 5:
            pcoords[0] = pc[0]; pcoords[1] = pc[1]; pcoords[2] = 1.0;
            break;
          }
        }
      }
    }
  return intersection;
}

int vtkQuadraticPyramid::Triangulate(int vtkNotUsed(index), vtkIdList *ptIds, 
                                      vtkPoints *pts)
{
  int i;
  int ii;
  pts->Reset();
  ptIds->Reset();

  for (i=0; i < 6; i++)
    {
    for ( int j=0; j < 5; j++)
      {
      ptIds->InsertId(5*i+j,this->PointIds->GetId(LinearPyramids[i][j]));
      pts->InsertPoint(5*i+j,this->Points->GetPoint(LinearPyramids[i][j]));
      }
    }

  for (ii=0, i=6 ; i < 10; i++, ii++)
    {
    for ( int j=0; j < 4; j++)
      {
      ptIds->InsertId(4*ii+j+30,this->PointIds->GetId(LinearPyramids[i][j]));
      pts->InsertPoint(4*ii+j+30,this->Points->GetPoint(LinearPyramids[i][j]));
      }
    }

  return 1;
}

// Given parametric coordinates compute inverse Jacobian transformation
// matrix. Returns 9 elements of 3x3 inverse Jacobian plus interpolation
// function derivatives.
void vtkQuadraticPyramid::JacobianInverse(double pcoords[3], double **inverse, 
                                          double derivs[39])
{
  int i, j;
  double *m[3], m0[3], m1[3], m2[3];
  double x[3];

  // compute interpolation function derivatives
  this->InterpolationDerivs(pcoords, derivs);

  // create Jacobian matrix
  m[0] = m0; m[1] = m1; m[2] = m2;
  for (i=0; i < 3; i++) //initialize matrix
    {
    m0[i] = m1[i] = m2[i] = 0.0;
    }

  for ( j=0; j < 13; j++ )
    {
    this->Points->GetPoint(j, x);
    for ( i=0; i < 3; i++ )
      {
      m0[i] += x[i] * derivs[j];
      m1[i] += x[i] * derivs[13 + j];
      m2[i] += x[i] * derivs[26 + j];
      }
    }

  // now find the inverse
  if ( vtkMath::InvertMatrix(m,inverse,3) == 0 )
    {
    vtkErrorMacro(<<"Jacobian inverse not found");
    return;
    }
}

void vtkQuadraticPyramid::Derivatives(int vtkNotUsed(subId), 
                                      double pcoords[3], double *values, 
                                      int dim, double *derivs)
{
  double *jI[3], j0[3], j1[3], j2[3];
  double functionDerivs[3*13], sum[3];
  int i, j, k;

  // compute inverse Jacobian and interpolation function derivatives
  jI[0] = j0; jI[1] = j1; jI[2] = j2;
  this->JacobianInverse(pcoords, jI, functionDerivs);

  // now compute derivates of values provided
  for (k=0; k < dim; k++) //loop over values per vertex
    {
    sum[0] = sum[1] = sum[2] = 0.0;
    for ( i=0; i < 13; i++) //loop over interp. function derivatives
      {
      sum[0] += functionDerivs[i] * values[dim*i + k]; 
      sum[1] += functionDerivs[13 + i] * values[dim*i + k];
      sum[2] += functionDerivs[26 + i] * values[dim*i + k];
      }
    for (j=0; j < 3; j++) //loop over derivative directions
      {
      derivs[3*k + j] = sum[0]*jI[j][0] + sum[1]*jI[j][1] + sum[2]*jI[j][2];
      }
    }
}


// Clip this quadratic pyramid using scalar value provided. Like contouring, 
// except that it cuts the pyramid to produce tetrahedra.
void vtkQuadraticPyramid::Clip(double value, vtkDataArray* vtkNotUsed(cellScalars), 
                               vtkPointLocator* locator, vtkCellArray* tets,
                               vtkPointData* inPd, vtkPointData* outPd,
                               vtkCellData* inCd, vtkIdType cellId, 
                               vtkCellData* outCd, int insideOut)
{
  int i;
  // create six linear pyramid + 4 tetra
  this->Subdivide(inPd,inCd,cellId);

  //contour each linear pyramid separately
  vtkDataArray *localScalars = this->PointData->GetScalars();
  this->Scalars->SetNumberOfTuples(5);  //num of vertices
  for (i=0; i<6; i++) //for each subdivided pyramid
    {
    for (int j=0; j<5; j++) //for each of the five vertices of the pyramid
      {
      this->Pyramid->Points->SetPoint(j,this->Points->GetPoint(LinearPyramids[i][j]));
      this->Pyramid->PointIds->SetId(j,this->PointIds->GetId(LinearPyramids[i][j]));
      this->Scalars->SetValue(j,localScalars->GetTuple1(LinearPyramids[i][j]));
      }
    this->Pyramid->Clip(value,this->Scalars,locator,tets,this->PointData,outPd,
                    this->CellData,cellId,outCd,insideOut);
    }

  this->Scalars->SetNumberOfTuples(4);  //num of vertices
  for (i=6; i<10; i++) //for each subdivided tetra
    {
    for (int j=0; j<4; j++) //for each of the four vertices of the tetra
      {
      this->Tetra->Points->SetPoint(j,this->Points->GetPoint(LinearPyramids[i][j]));
      this->Tetra->PointIds->SetId(j,this->PointIds->GetId(LinearPyramids[i][j]));
      this->Scalars->SetValue(j,localScalars->GetTuple1(LinearPyramids[i][j]));
      }
    this->Tetra->Clip(value,this->Scalars,locator,tets,this->PointData,outPd,
                    this->CellData,cellId,outCd,insideOut);
    }
}

// Return the center of the quadratic pyramid in parametric coordinates.
int vtkQuadraticPyramid::GetParametricCenter(double pcoords[3])
{
  pcoords[0] = pcoords[1] = pcoords[2] = 0.5;
  return 0;
}

// Compute interpolation functions for the fifteen nodes.
void vtkQuadraticPyramid::InterpolationFunctions(double pcoords[3], 
                                                 double weights[13])
{
  // VTK needs parametric coordinates to be between (0,1). Isoparametric
  // shape functions are formulated between (-1,1). Here we do a 
  // coordinate system conversion from (0,1) to (-1,1).
  double r = pcoords[0];
  double s = pcoords[1];
  double t = pcoords[2];

  // corners
  weights[0] =  (2*r - 1.0)*(r - 1.0)*(2*s - 1.0)*(s - 1.0)*(2*t - 1.0)*(t - 1.0);
  weights[1] = -(2*r - 1.0)*      -r *(2*s - 1.0)*(s - 1.0);
  weights[2] =  (2*r - 1.0)*       r *(2*s - 1.0)*s;
  weights[3] =  (2*r - 1.0)*(r - 1.0)*(2*s - 1.0)*s;
  weights[4] =  t*(2*t - 1.0);

  // midsides of rectangles
  weights[5] =  4*r * (r - 1.0)*(s - 1.0)*(1.0-2*t);
  weights[6] = -4*r*  (1.0 - 2.0*r)*s*(1.0 - s);
  weights[7] = -4*r * (r - 1.0)*s*(1-2.0*t);
  weights[8] = -4*(2*r - 1.0)*(r - 1.0)*s*(s - 1.0)*(1-2.0*t);
  
  // midsides of triangles
  weights[9]  = -16*t * (t - 1.0)*(s - 0.5)*(r - 0.5);
  weights[10] = -8*r*t * (s - 0.5);
  weights[11] = 8*r*s*t;
  weights[12] = -8*(r - 0.5)*s*t;
  
}

// Derivatives in parametric space.
void vtkQuadraticPyramid::InterpolationDerivs(double pcoords[3], 
                                              double derivs[39])
{
  //VTK needs parametric coordinates to be between (0,1). Isoparametric
  //shape functions are formulated between (-1,1). Here we do a 
  //coordinate system conversion from (0,1) to (-1,1).
  double r = pcoords[0];
  double s = pcoords[1];
  double t = pcoords[2];
  //r-derivatives
  // corners
  derivs[0] = 2*(1-r-s)*(1-t)*(.5-r-s-t);
  derivs[1] = 2*r*(1-t)*(r-t-0.5);
  derivs[2] = 2*s*(1-t)*(s-t-0.5);
  derivs[3] = 2*(1-r-s)*t*(t-r-s-0.5);
  derivs[4] = 2*r*t*(r+t-1.5);
  // midsides of rectangles
  derivs[5]  = 4*r*(1-r-s)*(1-t);
  derivs[6]  = 4*r*(1-r-s)*(1-t);
  derivs[7]  = 4*r*s*(1-t);
  derivs[8]  = 4*(1-r-s)*s*(1-t);
  // midsides of triangles
  derivs[9] = 4*t*(1-r-s)*(1-t);
  derivs[10] = 4*t*(1-r-s)*(1-t);
  derivs[11] = 4*t*r*(1-t);
  derivs[12] = 4*t*s*(1-t);

  //s-derivatives
  // corners
  derivs[13] = 2*(1-r-s)*(1-t)*(.5-r-s-t);
  derivs[14] = 2*r*(1-t)*(r-t-0.5);
  derivs[15] = 2*s*(1-t)*(s-t-0.5);
  derivs[16] = 2*(1-r-s)*t*(t-r-s-0.5);
  derivs[17] = 2*r*t*(r+t-1.5);
  // midsides of rectangles
  derivs[18]  = 4*r*(1-r-s)*(1-t);
  derivs[19]  = 4*r*(1-r-s)*(1-t);
  derivs[20]  = 4*r*s*(1-t);
  derivs[21]  = 4*(1-r-s)*s*(1-t);
  // midsides of triangles
  derivs[22] = 4*t*(1-r-s)*(1-t);
  derivs[23] = 4*t*(1-r-s)*(1-t);
  derivs[24] = 4*t*r*(1-t);
  derivs[25] = 4*t*s*(1-t);

  //t-derivatives
  // corners
  derivs[26] = 2*(1-r-s)*(1-t)*(.5-r-s-t);
  derivs[27] = 2*r*(1-t)*(r-t-0.5);
  derivs[28] = 2*s*(1-t)*(s-t-0.5);
  derivs[29] = 2*(1-r-s)*t*(t-r-s-0.5);
  derivs[30] = 2*r*t*(r+t-1.5);
  // midsides of rectangles
  derivs[31]  = 4*r*(1-r-s)*(1-t);
  derivs[32]  = 4*r*(1-r-s)*(1-t);
  derivs[33]  = 4*r*s*(1-t);
  derivs[34]  = 4*(1-r-s)*s*(1-t);
  // midsides of triangles
  derivs[35] = 4*t*(1-r-s)*(1-t);
  derivs[36] = 4*t*(1-r-s)*(1-t);
  derivs[37] = 4*t*r*(1-t);
  derivs[38] = 4*t*s*(1-t);
}

static double vtkQPyramidCellPCoords[39] = {0.0,0.0,0.0, 1.0,0.0,0.0, 1.0,1.0,0.0, 
                                            0.0,1.0,0.0, 0.0,0.0,1.0, 0.5,0.0,0.0,
                                            1.0,0.5,0.0, 0.5,1.0,0.0, 0.0,0.5,0.0,
                                            0.0,0.0,0.5, 0.5,0.0,0.5, 0.5,0.5,0.5,
                                            0.0,0.5,0.5 };

double *vtkQuadraticPyramid::GetParametricCoords()
{
  return vtkQPyramidCellPCoords;
}
