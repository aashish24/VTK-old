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
#include "vtkQuadraticTriangle.h"
#include "vtkPolyData.h"
#include "vtkPointLocator.h"
#include "vtkMath.h"
#include "vtkLine.h"
#include "vtkQuadraticEdge.h"
#include "vtkTriangle.h"
#include "vtkFloatArray.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkQuadraticTriangle, "$Revision$");
vtkStandardNewMacro(vtkQuadraticTriangle);

// Construct the line with two points.
vtkQuadraticTriangle::vtkQuadraticTriangle()
{
  this->Edge = vtkQuadraticEdge::New();
  this->Face = vtkTriangle::New();
  this->Scalars = vtkFloatArray::New();
  this->Scalars->SetNumberOfTuples(3);

  int i;
  this->Points->SetNumberOfPoints(6);
  this->PointIds->SetNumberOfIds(6);
  for (i = 0; i < 6; i++)
    {
    this->Points->SetPoint(i, 0.0, 0.0, 0.0);
    }
  for (i = 0; i < 6; i++)
    {
    this->PointIds->SetId(i,0);
    }
  
}

vtkQuadraticTriangle::~vtkQuadraticTriangle()
{
  this->Edge->Delete();
  this->Face->Delete();
  this->Scalars->Delete();
}

vtkCell *vtkQuadraticTriangle::MakeObject()
{
  vtkQuadraticTriangle *cell = vtkQuadraticTriangle::New();
  cell->DeepCopy(this);
  return (vtkCell *)cell;
}

vtkCell *vtkQuadraticTriangle::GetEdge(int edgeId)
{
  edgeId = (edgeId < 0 ? 0 : (edgeId > 2 ? 2 : edgeId ));
  int p = (edgeId+1) % 3;

  // load point id's
  this->Edge->PointIds->SetId(0,this->PointIds->GetId(edgeId));
  this->Edge->PointIds->SetId(1,this->PointIds->GetId(p));
  this->Edge->PointIds->SetId(2,this->PointIds->GetId(edgeId+3));

  // load coordinates
  this->Edge->Points->SetPoint(0,this->Points->GetPoint(edgeId));
  this->Edge->Points->SetPoint(1,this->Points->GetPoint(p));
  this->Edge->Points->SetPoint(2,this->Points->GetPoint(edgeId+3));

  return this->Edge;
}

// order picked carefully for parametric coordinate conversion
static int linearTris[4][3] = { {0,3,5}, {3, 1,4}, {5,4,2}, {4,5,3} };

int vtkQuadraticTriangle::EvaluatePosition(float* x, float* closestPoint, 
                                           int& subId, float pcoords[3],
                                           float& minDist2, float *weights)
{
  float pc[3], dist2;
  int ignoreId, i, returnStatus=0, status;
  float tempWeights[3];
  float closest[3];

  //four linear triangles are used
  for (minDist2=VTK_LARGE_FLOAT, i=0; i < 4; i++)
    {
    this->Face->Points->SetPoint(
      0,this->Points->GetPoint(linearTris[i][0]));
    this->Face->Points->SetPoint(
      1,this->Points->GetPoint(linearTris[i][1]));
    this->Face->Points->SetPoint(
      2,this->Points->GetPoint(linearTris[i][2]));

    status = this->Face->EvaluatePosition(x,closest,ignoreId,pc,dist2,
                                          tempWeights);
    if ( status != -1 && dist2 < minDist2 )
      {
      returnStatus = status;
      minDist2 = dist2;
      subId = i;
      pcoords[0] = pc[0];
      pcoords[1] = pc[1];
      }
    }

  // adjust parametric coordinates
  if ( returnStatus != -1 )
    {
    if ( subId == 0 )
      {
      pcoords[0] /= 2.0;
      pcoords[1] /= 2.0;
      }
    else if ( subId == 1 )
      {
      pcoords[0] = 0.5 + (pcoords[0]/2.0);
      pcoords[1] /= 2.0;
      }
    else if ( subId == 2 )
      {
      pcoords[0] /= 2.0;
      pcoords[1] = 0.5 + (pcoords[1]/2.0);
      }
    else 
      {
      pcoords[0] = 1.0 - (0.5 + pcoords[0]/2.0);
      pcoords[1] = 1.0 - (0.5 + pcoords[1]/2.0);
      }
    pcoords[2] = 1.0 - pcoords[0] - pcoords[1];
    this->EvaluateLocation(subId,pcoords,closestPoint,weights);
    }

  return returnStatus;
}

void vtkQuadraticTriangle::EvaluateLocation(int& vtkNotUsed(subId), 
                                        float pcoords[3], 
                                        float x[3], float *weights)
{
  int i;
  float *a0 = this->Points->GetPoint(0);
  float *a1 = this->Points->GetPoint(1);
  float *a2 = this->Points->GetPoint(2);
  float *a3 = this->Points->GetPoint(3);
  float *a4 = this->Points->GetPoint(4);
  float *a5 = this->Points->GetPoint(5);

  this->InterpolationFunctions(pcoords,weights);
  
  for (i=0; i<6; i++) 
    {
    x[i] = a0[i]*weights[0] + a1[i]*weights[1] + a2[i]*weights[2] +
      a3[i]*weights[3] + a4[i]*weights[4] + a5[i]*weights[5];
    }
}

int vtkQuadraticTriangle::CellBoundary(int subId, float pcoords[3], 
                                       vtkIdList *pts)
{
  return this->Face->CellBoundary(subId, pcoords, pts);
}

void vtkQuadraticTriangle::Contour(float value, 
                                   vtkDataArray* cellScalars, 
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
  for ( int i=0; i < 4; i++)
    {
    this->Face->Points->SetPoint(0,this->Points->GetPoint(linearTris[i][0]));
    this->Face->Points->SetPoint(1,this->Points->GetPoint(linearTris[i][1]));
    this->Face->Points->SetPoint(2,this->Points->GetPoint(linearTris[i][2]));

    if ( outPd )
      {
      this->Face->PointIds->SetId(0,this->PointIds->GetId(linearTris[i][0]));
      this->Face->PointIds->SetId(1,this->PointIds->GetId(linearTris[i][1]));
      this->Face->PointIds->SetId(2,this->PointIds->GetId(linearTris[i][2]));
      }

    this->Scalars->SetTuple(0,cellScalars->GetTuple(linearTris[i][0]));
    this->Scalars->SetTuple(1,cellScalars->GetTuple(linearTris[i][1]));
    this->Scalars->SetTuple(2,cellScalars->GetTuple(linearTris[i][2]));

    this->Face->Contour(value, this->Scalars, locator, verts,
                        lines, polys, inPd, outPd, inCd, cellId, outCd);
    }
}

// Line-line intersection. Intersection has to occur within [0,1] parametric
// coordinates and with specified tolerance.
int vtkQuadraticTriangle::IntersectWithLine(float* p1, 
                                            float* p2, 
                                            float tol, 
                                            float& t,
                                            float* x, 
                                            float* pcoords, 
                                            int& subId)
{
  int subTest, i;
  subId = 0;

  for (i=0; i < 4; i++)
    {
    this->Face->Points->SetPoint(0,this->Points->GetPoint(linearTris[i][0]));
    this->Face->Points->SetPoint(1,this->Points->GetPoint(linearTris[i][1]));
    this->Face->Points->SetPoint(2,this->Points->GetPoint(linearTris[i][2]));

    if (this->Face->IntersectWithLine(p1, p2, tol, t, x, pcoords, subTest) )
      {
      return 1;
      }
    }

  return 0;
}

int vtkQuadraticTriangle::Triangulate(int vtkNotUsed(index), vtkIdList *ptIds, 
                                      vtkPoints *pts)
{
  pts->Reset();
  ptIds->Reset();

  // Create four linear triangles
  for ( int i=0; i < 4; i++)
    {
    ptIds->InsertId(3*i,this->PointIds->GetId(linearTris[i][0]));
    pts->InsertPoint(3*i,this->Points->GetPoint(linearTris[i][0]));
    ptIds->InsertId(3*i+1,this->PointIds->GetId(linearTris[i][1]));
    pts->InsertPoint(3*i+1,this->Points->GetPoint(linearTris[i][1]));
    ptIds->InsertId(3*i+2,this->PointIds->GetId(linearTris[i][2]));
    pts->InsertPoint(3*i+2,this->Points->GetPoint(linearTris[i][2]));
    }

  return 1;
}

void vtkQuadraticTriangle::Derivatives(int vtkNotUsed(subId), 
                                       float pcoords[3], float *values, 
                                       int dim, float *derivs)
{
}


// Clip this line using scalar value provided. Like contouring, except
// that it cuts the line to produce other lines.
void vtkQuadraticTriangle::Clip(float value, 
                                vtkDataArray* cellScalars, 
                                vtkPointLocator* locator,
                                vtkCellArray* polys,
                                vtkPointData* inPd, 
                                vtkPointData* outPd,
                                vtkCellData* inCd, 
                                vtkIdType cellId, 
                                vtkCellData* outCd,
                                int insideOut)
{
  for ( int i=0; i < 4; i++)
    {
    this->Face->Points->SetPoint(0,this->Points->GetPoint(linearTris[i][0]));
    this->Face->Points->SetPoint(1,this->Points->GetPoint(linearTris[i][1]));
    this->Face->Points->SetPoint(2,this->Points->GetPoint(linearTris[i][2]));

    this->Face->PointIds->SetId(0,this->PointIds->GetId(linearTris[i][0]));
    this->Face->PointIds->SetId(1,this->PointIds->GetId(linearTris[i][1]));
    this->Face->PointIds->SetId(2,this->PointIds->GetId(linearTris[i][2]));

    this->Scalars->SetTuple(0,cellScalars->GetTuple(linearTris[i][0]));
    this->Scalars->SetTuple(1,cellScalars->GetTuple(linearTris[i][1]));
    this->Scalars->SetTuple(2,cellScalars->GetTuple(linearTris[i][2]));

    this->Face->Clip(value, this->Scalars, locator, polys, inPd, outPd, 
                     inCd, cellId, outCd, insideOut);
    }
}

int vtkQuadraticTriangle::GetParametricCenter(float pcoords[3])
{
  pcoords[0] = pcoords[1] = 0.333; pcoords[2] = 0.0;
  return 0;
}

// Compute interpolation functions. Node [2] is the mid-edge node.
void vtkQuadraticTriangle::InterpolationFunctions(float pcoords[3], 
                                                  float weights[3])
{
  float r = pcoords[0];
  float s = pcoords[1];
  float t = 1.0 - r - s;

  weights[0] = t*(2.0*t - 1.0);
  weights[1] = r*(2.0*r - 1.0);
  weights[2] = s*(2.0*s - 1.0);
  weights[3] = 4.0 * r * t;
  weights[4] = 4.0 * r * s;
  weights[5] = 4.0 * s * t;
}

// Derivatives in parametric space.
void vtkQuadraticTriangle::InterpolationDerivs(float pcoords[3], 
                                               float derivs[3])
{
  float r = pcoords[0];
  float s = pcoords[1];

  // r-derivatives
  derivs[0] = 4.0*r + 4.0*s - 3.0;
  derivs[1] = 4.0*r - 1.0;
  derivs[2] = 0.0;
  derivs[3] = 4.0 - 8.0*r - 4.0*s;
  derivs[4] = 4.0*s;
  derivs[5] = -4.0*s;

  // s-derivatives
  derivs[6] = 4.0*r + 4.0*s - 3.0;
  derivs[7] = 0.0;
  derivs[8] = 4.0*s - 1.0;
  derivs[9] = -4.0*r;
  derivs[10] = 4.0*s;
  derivs[11] = 4.0 - 8.0*s - 4.0*r;

}

