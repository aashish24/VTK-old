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
#include "vtkQuadraticWedge.h"

#include "vtkCellData.h"
#include "vtkDoubleArray.h"
#include "vtkWedge.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointLocator.h"
#include "vtkPolyData.h"
#include "vtkQuadraticEdge.h"
#include "vtkQuadraticQuad.h"
#include "vtkQuadraticTriangle.h"

vtkCxxRevisionMacro(vtkQuadraticWedge, "$Revision$");
vtkStandardNewMacro(vtkQuadraticWedge);

// Construct the wedge with 15 points + 3 extra points for internal
// computation.
vtkQuadraticWedge::vtkQuadraticWedge()
{
  int i;

  // At times the cell looks like it has 18 points (during interpolation)
  // We initially allocate for 18.
  this->Points->SetNumberOfPoints(18);
  this->PointIds->SetNumberOfIds(18);
  for (i = 0; i < 18; i++)
    {
    this->Points->SetPoint(i, 0.0, 0.0, 0.0);
    this->PointIds->SetId(i,0);
    }
  this->Points->SetNumberOfPoints(15);
  this->PointIds->SetNumberOfIds(15);

  this->Edge = vtkQuadraticEdge::New();
  this->Face = vtkQuadraticQuad::New();
  this->TriangleFace = vtkQuadraticTriangle::New();
  this->Wedge = vtkWedge::New();

  this->PointData = vtkPointData::New();
  this->CellData = vtkCellData::New();
  this->Scalars = vtkDoubleArray::New();
  this->Scalars->SetNumberOfTuples(6);  //num of vertices
}

vtkQuadraticWedge::~vtkQuadraticWedge()
{
  this->Edge->Delete();
  this->Face->Delete();
  this->TriangleFace->Delete();
  this->Wedge->Delete();

  this->PointData->Delete();
  this->CellData->Delete();
  this->Scalars->Delete();
}

// instead of using an hexahedron we could use two prims/wedge...
static int LinearWedges[8][6] = { {0,6,8,12,15,17},
                                  {6,7,8,15,16,17},
                                  {6,1,7,15,13,16},
                                  {8,7,2,17,16,14},
                                  {12,15,17,3,9,11},
                                  {15,16,17,9,10,11},
                                  {15,13,16,9,4,10},
                                  {17,16,14,11,10,5} };

static int WedgeFaces[5][8] = { {0,1,4,3,6,13,9,12}, 
                                {1,2,5,4,7,14,10,13},
                                {0,3,5,2,12,11,14,8},
                                {0,2,1,8,7,6,0,0},
                                {3,4,5,9,10,11,0,0}};

static int WedgeEdges[9][3] = { {0,1,6}, {1,2,7}, {0,2,8},
                                {0,3,12},{1,4,13},{2,5,14},
                                {3,4,9}, {4,5,10}, {3,5,11} };

static double MidPoints[3][3] = { {0.5,0.0,0.5},
                                  {0.5,0.5,0.5}, 
                                  {0.0,0.5,0.5} };

vtkCell *vtkQuadraticWedge::GetEdge(int edgeId)
{
  edgeId = (edgeId < 0 ? 0 : (edgeId > 8 ? 8 : edgeId ));

  for (int i=0; i<3; i++)
    {
    this->Edge->PointIds->SetId(i,this->PointIds->GetId(WedgeEdges[edgeId][i]));
    this->Edge->Points->SetPoint(i,this->Points->GetPoint(WedgeEdges[edgeId][i]));
    }

  return this->Edge;
}

vtkCell *vtkQuadraticWedge::GetFace(int faceId)
{
  faceId = (faceId < 0 ? 0 : (faceId > 4 ? 4 : faceId ));

  // load point id's and coordinates
  // be carefull with the last two one:
  if(faceId > 2)
    {
    for (int i=0; i<6; i++)
      {
      this->TriangleFace->PointIds->SetId(i,this->PointIds->GetId(WedgeFaces[faceId][i]));
      this->TriangleFace->Points->SetPoint(i,this->Points->GetPoint(WedgeFaces[faceId][i]));
      }
    return this->TriangleFace;
    }
  else
    {
    for (int i=0; i<8; i++)
      {
      this->Face->PointIds->SetId(i,this->PointIds->GetId(WedgeFaces[faceId][i]));
      this->Face->Points->SetPoint(i,this->Points->GetPoint(WedgeFaces[faceId][i]));
      }
    return this->Face;
    }
}

static const double VTK_DIVERGED = 1.e6;
static const int VTK_WEDGE_MAX_ITERATION=10;
static const double VTK_WEDGE_CONVERGED=1.e-03;

int vtkQuadraticWedge::EvaluatePosition(double* x, 
                                        double* closestPoint, 
                                        int& subId, double pcoords[3],
                                        double& dist2, double *weights)
{
  int iteration, converged;
  double  params[3];
  double  fcol[3], rcol[3], scol[3], tcol[3];
  int i, j;
  double  d, pt[3];
  double derivs[3*15];

  //  set initial position for Newton's method
  subId = 0;
  pcoords[0] = pcoords[1] = pcoords[2] = params[0] = params[1] = params[2]=0.5;

  //  enter iteration loop
  for (iteration=converged=0;
       !converged && (iteration < VTK_WEDGE_MAX_ITERATION);  iteration++) 
    {
    //  calculate element interpolation functions and derivatives
    this->InterpolationFunctions(pcoords, weights);
    this->InterpolationDerivs(pcoords, derivs);

    //  calculate newton functions
    for (i=0; i<3; i++) 
      {
      fcol[i] = rcol[i] = scol[i] = tcol[i] = 0.0;
      }
    for (i=0; i<15; i++)
      {
      this->Points->GetPoint(i, pt);
      for (j=0; j<3; j++)
        {
        fcol[j] += pt[j] * weights[i];
        rcol[j] += pt[j] * derivs[i];
        scol[j] += pt[j] * derivs[i+15];
        tcol[j] += pt[j] * derivs[i+30];
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
    if ( ((fabs(pcoords[0]-params[0])) < VTK_WEDGE_CONVERGED) &&
         ((fabs(pcoords[1]-params[1])) < VTK_WEDGE_CONVERGED) &&
         ((fabs(pcoords[2]-params[2])) < VTK_WEDGE_CONVERGED) )
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
      dist2 = 0.0; //inside wedge
      }
    return 1;
    }
  else
    {
    double pc[3], w[15];
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

void vtkQuadraticWedge::EvaluateLocation(int& vtkNotUsed(subId), 
                                         double pcoords[3], 
                                         double x[3], double *weights)
{
  double pt[3];

  this->InterpolationFunctions(pcoords, weights);

  x[0] = x[1] = x[2] = 0.0;
  for (int i=0; i<15; i++)
    {
    this->Points->GetPoint(i, pt);
    for (int j=0; j<3; j++)
      {
      x[j] += pt[j] * weights[i];
      }
    }
}

int vtkQuadraticWedge::CellBoundary(int subId, double pcoords[3], 
                                    vtkIdList *pts)
{
  return this->Wedge->CellBoundary(subId, pcoords, pts);
}

void vtkQuadraticWedge::Subdivide(vtkPointData *inPd, vtkCellData *inCd, 
                                  vtkIdType cellId)
{
  int numMidPts, i, j;
  double weights[15];
  double x[3];

  //Copy point and cell attribute data
  this->PointData->CopyAllocate(inPd,18);
  this->CellData->CopyAllocate(inCd,6);
  for (i=0; i<15; i++)
    {
    this->PointData->CopyData(inPd,this->PointIds->GetId(i),i);
    }
  this->CellData->CopyData(inCd,cellId,0);
  
  //Interpolate new values
  this->PointIds->SetNumberOfIds(15);
  double *p;
  for ( numMidPts=0; numMidPts < 3; numMidPts++ )
    {
    this->InterpolationFunctions(MidPoints[numMidPts], weights);

    x[0] = 0.0;
    x[1] = 0.0;
    x[2] = 0.0;
    for (i=0; i<15; i++) 
      {
      p = this->Points->GetPoint(i);
      for (j=0; j<3; j++) 
        {
        x[j] += p[j] * weights[i];
        }
      }
    this->Points->SetPoint(15+numMidPts,x);
    this->PointData->InterpolatePoint(inPd, 15+numMidPts, 
                                      this->PointIds, weights);
    }
  this->PointIds->SetNumberOfIds(18);
}

void vtkQuadraticWedge::Contour(double value, 
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
  //subdivide into 8 linear wedges
  this->Subdivide(inPd,inCd,cellId);
  
  //contour each linear wedge separately
  vtkDataArray *localScalars = this->PointData->GetScalars();
  for (int i=0; i<8; i++) //for each wedge
    {
    for (int j=0; j<6; j++) //for each point of wedge
      {
      this->Wedge->Points->SetPoint(j,this->Points->GetPoint(LinearWedges[i][j]));
      this->Wedge->PointIds->SetId(j,this->PointIds->GetId(LinearWedges[i][j]));
      this->Scalars->SetValue(j,localScalars->GetTuple1(LinearWedges[i][j]));
      }
    this->Wedge->Contour(value,this->Scalars,locator,verts,lines,polys,
                       this->PointData,outPd,this->CellData,0,outCd);
    }

}

// Line-hex intersection. Intersection has to occur within [0,1] parametric
// coordinates and with specified tolerance.
int vtkQuadraticWedge::IntersectWithLine(double* p1, double* p2, 
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
    if(faceNum > 2)
      {
      for (int i=0; i<6; i++)
        {
        this->TriangleFace->PointIds->SetId(i,
              this->PointIds->GetId(WedgeFaces[faceNum][i]));
        }
      inter = this->TriangleFace->IntersectWithLine(p1, p2, tol, tTemp, 
                                      xTemp, pc, subId);
      }
    else
      {
      for (int i=0; i<8; i++)
        {
        this->Face->Points->SetPoint(i,
              this->Points->GetPoint(WedgeFaces[faceNum][i]));
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

int vtkQuadraticWedge::Triangulate(int vtkNotUsed(index), 
                                        vtkIdList *ptIds, vtkPoints *pts)
{
  pts->Reset();
  ptIds->Reset();

  for ( int i=0; i < 8; i++)
    {
    for ( int j=0; j < 6; j++)
      {
      ptIds->InsertId(6*i+j,this->PointIds->GetId(LinearWedges[i][j]));
      pts->InsertPoint(6*i+j,this->Points->GetPoint(LinearWedges[i][j]));
      }
    }

  return 1;
}

// Given parametric coordinates compute inverse Jacobian transformation
// matrix. Returns 9 elements of 3x3 inverse Jacobian plus interpolation
// function derivatives.
void vtkQuadraticWedge::JacobianInverse(double pcoords[3], double **inverse, 
                                        double derivs[45])
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

  for ( j=0; j < 15; j++ )
    {
    this->Points->GetPoint(j, x);
    for ( i=0; i < 3; i++ )
      {
      m0[i] += x[i] * derivs[j];
      m1[i] += x[i] * derivs[15 + j];
      m2[i] += x[i] * derivs[30 + j];
      }
    }

  // now find the inverse
  if ( vtkMath::InvertMatrix(m,inverse,3) == 0 )
    {
    vtkErrorMacro(<<"Jacobian inverse not found");
    return;
    }
}

void vtkQuadraticWedge::Derivatives(int vtkNotUsed(subId), 
                                    double pcoords[3], double *values, 
                                    int dim, double *derivs)
{
  double *jI[3], j0[3], j1[3], j2[3];
  double functionDerivs[3*15], sum[3];
  int i, j, k;

  // compute inverse Jacobian and interpolation function derivatives
  jI[0] = j0; jI[1] = j1; jI[2] = j2;
  this->JacobianInverse(pcoords, jI, functionDerivs);

  // now compute derivates of values provided
  for (k=0; k < dim; k++) //loop over values per vertex
    {
    sum[0] = sum[1] = sum[2] = 0.0;
    for ( i=0; i < 15; i++) //loop over interp. function derivatives
      {
      sum[0] += functionDerivs[i] * values[dim*i + k]; 
      sum[1] += functionDerivs[15 + i] * values[dim*i + k];
      sum[2] += functionDerivs[30 + i] * values[dim*i + k];
      }
    for (j=0; j < 3; j++) //loop over derivative directions
      {
      derivs[3*k + j] = sum[0]*jI[j][0] + sum[1]*jI[j][1] + sum[2]*jI[j][2];
      }
    }
}


// Clip this quadratic wedge using scalar value provided. Like contouring, 
// except that it cuts the wedge to produce tetrahedra.
void vtkQuadraticWedge::Clip(double value, vtkDataArray* vtkNotUsed(cellScalars), 
                             vtkPointLocator* locator, vtkCellArray* tets,
                             vtkPointData* inPd, vtkPointData* outPd,
                             vtkCellData* inCd, vtkIdType cellId, 
                             vtkCellData* outCd, int insideOut)
{
  // create eight linear hexes
  this->Subdivide(inPd,inCd,cellId);
  
  //contour each linear hex separately
  vtkDataArray *localScalars = this->PointData->GetScalars();
  for (int i=0; i<8; i++) //for each subdivided wedge
    {
    for (int j=0; j<6; j++) //for each of the six vertices of the wedge
      {
      this->Wedge->Points->SetPoint(j,this->Points->GetPoint(LinearWedges[i][j]));
      this->Wedge->PointIds->SetId(j,this->PointIds->GetId(LinearWedges[i][j]));
      this->Scalars->SetValue(j,localScalars->GetTuple1(LinearWedges[i][j]));
      }
    this->Wedge->Clip(value,this->Scalars,locator,tets,this->PointData,outPd,
                    this->CellData,cellId,outCd,insideOut);
    }

}

// Return the center of the quadratic wedge in parametric coordinates.
int vtkQuadraticWedge::GetParametricCenter(double pcoords[3])
{
  pcoords[0] = pcoords[1] = 0.333; pcoords[2] = 0.5;
  return 0;
}

// Compute interpolation functions for the fifteen nodes.
void vtkQuadraticWedge::InterpolationFunctions(double pcoords[3], 
                                               double weights[15])
{
  // VTK needs parametric coordinates to be between (0,1). Isoparametric
  // shape functions are formulated between (-1,1). Here we do a 
  // coordinate system conversion from (0,1) to (-1,1).
  double r = pcoords[0];
  double s = pcoords[1];
  double t = pcoords[2];
  // corners
  weights[0] = 2*(1-r-s)*(1-t)*(.5-r-s-t);
  weights[1] = 2*r*(1-t)*(r-t-0.5);
  weights[2] = 2*s*(1-t)*(s-t-0.5);
  weights[3] = 2*(1-r-s)*t*(t-r-s-0.5);
  weights[4] = 2*r*t*(r+t-1.5);
  weights[5] = 2*s*t*(s+t-1.5);

  // midsides of triangles
  weights[6]  = 4*r*(1-r-s)*(1-t);
  weights[7]  = 4*r*s*(1-t);
  weights[8]  = 4*(1-r-s)*s*(1-t);
  weights[9]  = 4*r*(1-r-s)*t;
  weights[10] = 4*r*s*t;
  weights[11] = 4*(1-r-s)*s*t;

  // midsides of rectangles
  weights[12] = 4*t*(1-r-s)*(1-t);
  weights[13] = 4*t*r*(1-t);
  weights[14] = 4*t*s*(1-t);
}

// Derivatives in parametric space.
void vtkQuadraticWedge::InterpolationDerivs(double pcoords[3], 
                                            double derivs[45])
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
  derivs[5] = 2*s*t*(s+t-1.5);
  // midsides of triangles
  derivs[6]  = 4*r*(1-r-s)*(1-t);
  derivs[7]  = 4*r*s*(1-t);
  derivs[8]  = 4*(1-r-s)*s*(1-t);
  derivs[9]  = 4*r*(1-r-s)*t;
  derivs[10] = 4*r*s*t;
  derivs[11] = 4*(1-r-s)*s*t;
  // midsides of rectangles
  derivs[12] = 4*t*(1-r-s)*(1-t);
  derivs[13] = 4*t*r*(1-t);
  derivs[14] = 4*t*s*(1-t);

  //s-derivatives
  // corners
  derivs[15] = 2*(1-r-s)*(1-t)*(.5-r-s-t);
  derivs[16] = 2*r*(1-t)*(r-t-0.5);
  derivs[17] = 2*s*(1-t)*(s-t-0.5);
  derivs[18] = 2*(1-r-s)*t*(t-r-s-0.5);
  derivs[19] = 2*r*t*(r+t-1.5);
  derivs[20] = 2*s*t*(s+t-1.5);
  // midsides of triangles
  derivs[21]  = 4*r*(1-r-s)*(1-t);
  derivs[22]  = 4*r*s*(1-t);
  derivs[23]  = 4*(1-r-s)*s*(1-t);
  derivs[24]  = 4*r*(1-r-s)*t;
  derivs[25] = 4*r*s*t;
  derivs[26] = 4*(1-r-s)*s*t;
  // midsides of rectangles
  derivs[27] = 4*t*(1-r-s)*(1-t);
  derivs[28] = 4*t*r*(1-t);
  derivs[29] = 4*t*s*(1-t);

  //t-derivatives
  // corners
  derivs[30] = 2*(1-r-s)*(1-t)*(.5-r-s-t);
  derivs[31] = 2*r*(1-t)*(r-t-0.5);
  derivs[32] = 2*s*(1-t)*(s-t-0.5);
  derivs[33] = 2*(1-r-s)*t*(t-r-s-0.5);
  derivs[34] = 2*r*t*(r+t-1.5);
  derivs[35] = 2*s*t*(s+t-1.5);
  // midsides of triangles
  derivs[36]  = 4*r*(1-r-s)*(1-t);
  derivs[37]  = 4*r*s*(1-t);
  derivs[38]  = 4*(1-r-s)*s*(1-t);
  derivs[39]  = 4*r*(1-r-s)*t;
  derivs[40] = 4*r*s*t;
  derivs[41] = 4*(1-r-s)*s*t;
  // midsides of rectangles
  derivs[42] = 4*t*(1-r-s)*(1-t);
  derivs[43] = 4*t*r*(1-t);
  derivs[44] = 4*t*s*(1-t);
}

static double vtkQWedgeCellPCoords[45] = {0.0,0.0,0.0, 1.0,0.0,0.0, 0.0,1.0,0.0, 
                                          0.0,0.0,1.0, 1.0,0.0,1.0, 0.0,1.0,1.0,
                                          0.5,0.0,0.0, 0.5,0.5,0.0, 0.0,0.5,0.0,
                                          0.5,0.0,1.0, 0.5,0.5,1.0, 0.0,0.5,1.0,
                                          0.0,0.0,0.5, 1.0,0.0,0.5, 0.0,1.0,0.5};
double *vtkQuadraticWedge::GetParametricCoords()
{
  return vtkQWedgeCellPCoords;
}

