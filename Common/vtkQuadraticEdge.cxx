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
#include "vtkQuadraticEdge.h"
#include "vtkPolyData.h"
#include "vtkPointLocator.h"
#include "vtkMath.h"
#include "vtkLine.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkQuadraticEdge, "$Revision$");
vtkStandardNewMacro(vtkQuadraticEdge);

// Construct the line with two points.
vtkQuadraticEdge::vtkQuadraticEdge()
{
  int i;
  
  this->Points->SetNumberOfPoints(3);
  this->PointIds->SetNumberOfIds(3);
  for (i = 0; i < 3; i++)
    {
    this->Points->SetPoint(i, 0.0, 0.0, 0.0);
    }
  for (i = 0; i < 3; i++)
    {
    this->PointIds->SetId(i,0);
    }
  
  this->Tesselation = NULL;
  this->InternalDataSet = NULL;
}

vtkQuadraticEdge::~vtkQuadraticEdge()
{
  if ( this->Tesselation )
    {
    this->Tesselation->Delete();
    this->InternalDataSet->Delete();
    }
}


vtkCell *vtkQuadraticEdge::MakeObject()
{
  vtkQuadraticEdge *cell = vtkQuadraticEdge::New();
  cell->DeepCopy(this);
  cell->SetError(this->GetError());
  return (vtkCell *)cell;
}

static const int VTK_QUADRATIC_EDGE_MAX_ITERATION=10;
int vtkQuadraticEdge::EvaluatePosition(float* x, 
                                       float* closestPoint, 
                                       int& subId, float pcoords[3],
                                       float& vtkNotUsed(dist2), 
                                       float *weights)
{
  subId = 0;
  pcoords[1] = pcoords[2] = 0.0;

  //Bisection method to determine closest point
  float tl[3], tm[3], tr[3];
  float xl[3], wl[3], xm[3], wm[3], xr[3], wr[3], dl2, dm2, dr2;
  int iterNum, converged;

  //Note: the initial left and right parametric values tl,tr are set past
  //the valid parametric range (0,1). This is to allow the convergence of
  //the bisection method to end up outside the cell.
  tl[0] = -0.1;
  vtkQuadraticEdge::EvaluateLocation(subId,tl,xl,wl);
  dl2 = vtkMath::Distance2BetweenPoints(x,xl);
  
  tm[0] = 0.5;
  vtkQuadraticEdge::EvaluateLocation(subId,tm,xm,wm);
  dm2 = vtkMath::Distance2BetweenPoints(x,xm);
  
  tr[0] = 1.1;
  vtkQuadraticEdge::EvaluateLocation(subId,tr,xr,wr);
  dr2 = vtkMath::Distance2BetweenPoints(x,xr);
  
  for ( iterNum=0, converged=0; 
        !converged && iterNum<VTK_QUADRATIC_EDGE_MAX_ITERATION; iterNum++ )
    {
    if ( dl2 == dm2 && dm2 == dr2 ) //special case, at center of circle
      {
      break;
      }
    else
      {
      if ( dl2 < dr2 ) //move to the left
        {
        tr[0] = tm[0];
        tm[0] = (tl[0] + tm[0]) / 2.0;
        dr2 = dm2;
        xr[0] = xm[0];
        xr[1] = xm[1];
        xr[2] = xm[2];
        wr[0] = wm[0];
        wr[1] = wm[1];
        wr[2] = wm[2];
        }
      else //move to the right
        {
        tl[0] = tm[0];
        tm[0] = (tr[0] + tm[0]) / 2.0;
        dl2 = dm2;
        xl[0] = xm[0];
        xl[1] = xm[1];
        xl[2] = xm[2];
        wl[0] = wm[0];
        wl[1] = wm[1];
        wl[2] = wm[2];
        }
        
      vtkQuadraticEdge::EvaluateLocation(subId,tm,xm,wm);
      dm2 = vtkMath::Distance2BetweenPoints(x,xm);
      }

    //check convergence in parametric space
    if ( (tr[0]-tl[0]) < 0.01 ) converged = 1;
    }
  
  //outta here
  pcoords[0] = tm[0];
  weights[0] = wm[0];
  weights[1] = wm[1];
  weights[2] = wm[2];
  closestPoint[0] = xm[0];
  closestPoint[1] = xm[1];
  closestPoint[2] = xm[2];

  if ( pcoords[0] < 0.0 || pcoords[0] > 1.0 )
    {
    return 0;
    }
  else
    {
    return 1;
    }
}

void vtkQuadraticEdge::EvaluateLocation(int& vtkNotUsed(subId), 
                                        float pcoords[3], 
                                        float x[3], float *weights)
{
  int i;
  float *a0 = this->Points->GetPoint(0);
  float *a1 = this->Points->GetPoint(1);
  float *a2 = this->Points->GetPoint(2); //midside node

  this->InterpolationFunctions(pcoords,weights);
  
  for (i=0; i<3; i++) 
    {
    x[i] = a0[i]*weights[0] + a1[i]*weights[1] + a2[i]*weights[2];
    }
}

int vtkQuadraticEdge::CellBoundary(int vtkNotUsed(subId), float pcoords[3], 
                                   vtkIdList *pts)
{
  pts->SetNumberOfIds(1);

  if ( pcoords[0] >= 0.5 )
    {
    pts->SetId(0,this->PointIds->GetId(1));
    if ( pcoords[0] > 1.0 )
      {
      return 0;
      }
    else
      {
      return 1;
      }
    }
  else
    {
    pts->SetId(0,this->PointIds->GetId(0));
    if ( pcoords[0] < 0.0 )
      {
      return 0;
      }
    else
      {
      return 1;
      }
    }
}

void vtkQuadraticEdge::Contour(float vtkNotUsed(value), 
                               vtkDataArray* vtkNotUsed(cellScalars), 
                               vtkPointLocator* vtkNotUsed(locator), 
                               vtkCellArray *vtkNotUsed(verts), 
                               vtkCellArray* vtkNotUsed(lines), 
                               vtkCellArray* vtkNotUsed(polys), 
                               vtkPointData* vtkNotUsed(inPd), 
                               vtkPointData* vtkNotUsed(outPd),
                               vtkCellData* vtkNotUsed(inCd), 
                               vtkIdType vtkNotUsed(cellId), 
                               vtkCellData* vtkNotUsed(outCd))
{
}

// Line-line intersection. Intersection has to occur within [0,1] parametric
// coordinates and with specified tolerance.

// The following arguments were modified to avoid warnings:
// float p1[3], float p2[3], float x[3], float pcoords[3], 

int vtkQuadraticEdge::IntersectWithLine(float* vtkNotUsed(p1), 
                                        float* vtkNotUsed(p2), 
                                        float vtkNotUsed(tol), 
                                        float& vtkNotUsed(t),
                                        float* vtkNotUsed(x), 
                                        float* vtkNotUsed(pcoords), 
                                        int& vtkNotUsed(subId))
{
  return 0;
}

int vtkQuadraticEdge::Triangulate(int vtkNotUsed(index), vtkIdList *ptIds, 
                                  vtkPoints *pts)
{
  pts->Reset();
  ptIds->Reset();

  ptIds->InsertId(0,this->PointIds->GetId(0));
  pts->InsertPoint(0,this->Points->GetPoint(0));

  ptIds->InsertId(1,this->PointIds->GetId(1));
  pts->InsertPoint(1,this->Points->GetPoint(1));

  return 1;
}

void vtkQuadraticEdge::Derivatives(int vtkNotUsed(subId), 
                                   float pcoords[3], float *values, 
                                   int dim, float *derivs)
{
  float *x0, *x1, *x2, deltaX[3], weights[3];
  int i, j;

  x0 = this->Points->GetPoint(0);
  x1 = this->Points->GetPoint(1);
  x2 = this->Points->GetPoint(2);

  this->InterpolationFunctions(pcoords,weights);
  this->InterpolationDerivs(pcoords,derivs);
  
  for (i=0; i<3; i++)
    {
    deltaX[i] = x1[i] - x0[i]              - x2[i];
    }
  for (i=0; i<dim; i++) 
    {
    for (j=0; j<3; j++)
      {
      if ( deltaX[j] != 0 )
        {
        derivs[3*i+j] = (values[2*i+1] - values[2*i]) / deltaX[j];
        }
      else
        {
        derivs[3*i+j] =0;
        }
      }
    }
}


// Clip this line using scalar value provided. Like contouring, except
// that it cuts the line to produce other lines.
void vtkQuadraticEdge::Clip(float vtkNotUsed(value), 
                            vtkDataArray* vtkNotUsed(cellScalars), 
                            vtkPointLocator* vtkNotUsed(locator),
                            vtkCellArray* vtkNotUsed(lines),
                            vtkPointData* vtkNotUsed(inPd), 
                            vtkPointData* vtkNotUsed(outPd),
                            vtkCellData* vtkNotUsed(inCd), 
                            vtkIdType vtkNotUsed(cellId), 
                            vtkCellData* vtkNotUsed(outCd),
                            int vtkNotUsed(insideOut))
{
}

// Compute interpolation functions. Node [2] is the mid-edge node.
void vtkQuadraticEdge::InterpolationFunctions(float pcoords[3], float weights[3])
{
  weights[0] = 2.0 * (pcoords[0] - 0.5) * (pcoords[0] - 1.0);
  weights[1] = 2.0 * pcoords[0] * (pcoords[0] - 0.5);
  weights[2] = 4.0 * pcoords[0] * (1.0 - pcoords[0]);
}

// Derivatives in parametric space.
void vtkQuadraticEdge::InterpolationDerivs(float pcoords[3], float derivs[3])
{
  derivs[0] = 4.0 * pcoords[0] - 3.0;
  derivs[1] = 4.0 * pcoords[0] - 1.0;
  derivs[2] = 4.0 - pcoords[0] * 8.0;
}

// Create linear primitives from this quadratic cell.
void vtkQuadraticEdge::Tesselate(vtkIdType cellId, 
                                 vtkDataSet *input, vtkPolyData *output, 
                                 vtkPointLocator *locator)
{
  vtkPointData *inPD = input->GetPointData();
  vtkCellData *inCD = input->GetCellData();
  vtkPointData *outPD = output->GetPointData();
  vtkCellData *outCD = output->GetCellData();

  vtkCellArray *outputLines = output->GetLines();
  vtkPoints *pts = output->GetPoints();

  float *x0 = this->Points->GetPoint(0);
  float *x1 = this->Points->GetPoint(1);
  float *x2 = this->Points->GetPoint(2);
  
  float l2 = vtkMath::Distance2BetweenPoints(x0,x1);
  float d2 = vtkLine::DistanceToLine(x2, x0,x1); //midnode to endpoints
  float e2 = this->Error*this->Error;
  
  //the error divided by the maximum permissable error is an approximation to
  //the number of subdivisions.
  int numDivs = static_cast<int>(ceil( d2/(l2*e2) ));
  int numPts = numDivs + 1;
  
  //add new points to the output
  vtkIdType newCellId;
  vtkIdType p0 = this->InsertPoint(locator,pts,x0);
  vtkIdType p1 = this->InsertPoint(locator,pts,x1);
  vtkIdType p2 = this->InsertPoint(locator,pts,x2);
  outPD->CopyData(inPD,this->PointIds->GetId(0),p0);
  outPD->CopyData(inPD,this->PointIds->GetId(1),p1);
  outPD->CopyData(inPD,this->PointIds->GetId(2),p2);
  
  //at a minimum the edge is subdivided into two polylines
  if ( numPts <= 3 )
    {//end points plus mid-node
    newCellId = outputLines->InsertNextCell(3);
    outputLines->InsertCellPoint(p0);
    outputLines->InsertCellPoint(p1);
    outputLines->InsertCellPoint(p2);
    outCD->CopyData(inCD,cellId,newCellId);
    }
  else
    {//have to interpolate points
    float pcoords[3], weights[3], x[3];
    vtkIdType p;

    newCellId = outputLines->InsertNextCell(numDivs);
    outputLines->InsertCellPoint(p0);
    float delta = 1.0/numDivs;
    for (int i=1; i<(numPts-1); i++)
      {
      pcoords[0] = i*delta;
      this->InterpolationFunctions(pcoords,weights);
      x[0] = x0[0]*weights[0] + x1[0]*weights[1] + x2[0]*weights[2];
      x[1] = x0[1]*weights[0] + x1[1]*weights[1] + x2[1]*weights[2];
      x[2] = x0[2]*weights[0] + x1[2]*weights[1] + x2[2]*weights[2];
      p = this->InsertPoint(locator,pts,x);
      outPD->InterpolatePoint(inPD,p,this->PointIds,weights);
      outputLines->InsertCellPoint(p);
      }
    outputLines->InsertCellPoint(p1);
    outCD->CopyData(inCD,cellId,newCellId);
    }
}

// The second Tesselate() method is empty (intended only for 3D cells).
void vtkQuadraticEdge::Tesselate(vtkIdType vtkNotUsed(cellId),
                                 vtkDataSet* vtkNotUsed(input),
                                 vtkUnstructuredGrid* vtkNotUsed(output),
                                 vtkPointLocator* vtkNotUsed(locator))
{
}

void vtkQuadraticEdge::InternalTesselate()
{
  vtkPoints *pts;
  vtkCellArray *lines;
  
  if ( this->Tesselation == NULL )
    {
    this->Tesselation = vtkPolyData::New();
    vtkPoints *pts = vtkPoints::New();
    this->Tesselation->SetPoints(pts);
    pts->Delete();
    vtkCellArray *lines = vtkCellArray::New();
    this->Tesselation->SetLines(lines);
    lines->Delete();

    this->InternalDataSet = vtkPolyData::New();
    }
  else
    {
    pts = this->Tesselation->GetPoints();
    pts->Reset();
    lines = this->Tesselation->GetLines();
    lines->Reset();
    }
}
