/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Library. No part of this file
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
#include "Quad.hh"
#include "Polygon.hh"
#include "Plane.hh"
#include "vlMath.hh"
#include "CellArr.hh"
#include "Line.hh"

// Description:
// Deep copy of cell.
vlQuad::vlQuad(const vlQuad& q)
{
  this->Points = q.Points;
  this->PointIds = q.PointIds;
}

#define MAX_ITERATION 10
#define CONVERGED 1.e-03

int vlQuad::EvaluatePosition(float x[3], float closestPoint[3],
                             int& subId, float pcoords[3], 
                             float& dist2, float weights[MAX_CELL_SIZE])
{
  int i, j;
  vlPolygon poly;
  float *pt1, *pt2, *pt3, *pt, n[3];
  float det;
  vlPlane plane;
  vlMath math;
  float maxComponent;
  int idx, indices[2];
  int iteration, converged;
  float  params[2];
  float  fcol[2], rcol[3], scol[3];
  float derivs[8];

  subId = 0;
  pcoords[0] = pcoords[1] = pcoords[2] = params[0] = params[1] = 0.0;
//
// Get normal for quadrilateral
//
  pt1 = this->Points.GetPoint(0);
  pt2 = this->Points.GetPoint(1);
  pt3 = this->Points.GetPoint(2);

  poly.ComputeNormal (pt1, pt2, pt3, n);
//
// Project point to plane
//
  plane.ProjectPoint(x,pt1,n,closestPoint);
//
// Construct matrices.  Since we have over determined system, need to find
// which 2 out of 3 equations to use to develop equations. (Any 2 should 
// work since we've projected point to plane.)
//
  for (maxComponent=0.0, i=0; i<3; i++)
    {
    if (fabs(n[i]) > maxComponent)
      {
      maxComponent = fabs(n[i]);
      idx = i;
      }
    }
  for (j=0, i=0; i<3; i++)  
    {
    if ( i != idx ) indices[j++] = i;
    }
//
// Use Newton's method to solve for parametric coordinates
//  
  for (iteration=converged=0; !converged && (iteration < MAX_ITERATION);
  iteration++) 
    {
//
//  calculate element shape functions and derivatives
//
    this->ShapeFunctions(pcoords, weights);
    this->ShapeDerivs(pcoords, derivs);
//
//  calculate newton functions
//
    for (i=0; i<2; i++) 
      {
      fcol[i] = rcol[i] = scol[i] = 0.0;
      }
    for (i=0; i<4; i++)
      {
      pt = this->Points.GetPoint(i);
      for (j=0; j<2; j++)
        {
        fcol[j] += pt[indices[j]] * weights[i];
        rcol[j] += pt[indices[j]] * derivs[i];
        scol[j] += pt[indices[j]] * derivs[i+4];
        }
      }

    for (j=0; j<2; j++) fcol[j] -= x[indices[j]];
//
//  compute determinants and generate improvements
//
    if ( (det=math.Determinant2x2(rcol,scol)) == 0.0 )
      {
      return 0;
      }

    pcoords[0] = params[0] - math.Determinant2x2 (fcol,scol) / det;
    pcoords[1] = params[1] - math.Determinant2x2 (rcol,fcol) / det;
//
//  check for convergence
//
    if ( ((fabs(pcoords[0]-params[0])) < CONVERGED) &&
    ((fabs(pcoords[1]-params[1])) < CONVERGED) )
      {
      converged = 1;
      }
//
//  if not converged, repeat
//
    else 
      {
      params[0] = pcoords[0];
      params[1] = pcoords[1];
      }
    }
//
//  if not converged, set the parametric coordinates to arbitrary values
//  outside of element
//
  if ( !converged )
    {
    pcoords[0] = pcoords[1] =  pcoords[2] = 10.0;
    dist2 = LARGE_FLOAT;
    return 0;
    }
  else
    {
    // shift to (0,1)
    for(i=0; i<3; i++) pcoords[i] = 0.5*(pcoords[i]+1.0); 

    if ( pcoords[0] >= 0.0 && pcoords[0] <= 1.0 &&
    pcoords[1] >= 0.0 && pcoords[1] <= 1.0 )
      {
      dist2 = math.Distance2BetweenPoints(closestPoint,x); //projection distance
      return 1;
      }
    else
      {
      for (i=0; i<2; i++)
        {
        if (pcoords[i] < 0.0) pcoords[i] = 0.0;
        if (pcoords[i] > 1.0) pcoords[i] = 1.0;
        }
      this->EvaluateLocation(subId, pcoords, closestPoint, weights);
      dist2 = math.Distance2BetweenPoints(closestPoint,x);
      return 0;
      }
    }
}

void vlQuad::EvaluateLocation(int& subId, float pcoords[3], float x[3],
                              float weights[MAX_CELL_SIZE])
{
  int i, j;
  float *pt, pc[3];

  for (i=0; i<2; i++) pc[i] = 2.0*pcoords[i] - 1.0; //shift to -1<=r,s,t<=1
  this->ShapeFunctions(pc, weights);

  x[0] = x[1] = x[2] = 0.0;
  for (i=0; i<4; i++)
    {
    pt = this->Points.GetPoint(i);
    for (j=0; j<3; j++)
      {
      x[j] += pt[j] * weights[i];
      }
    }
}

//
// Compute iso-parametrix shape functions
//
void vlQuad::ShapeFunctions(float pcoords[3], float sf[4])
{
  double rm, rp, sm, sp;

  rm = 1. - pcoords[0];
  rp = 1. + pcoords[0];
  sm = 1. - pcoords[1];
  sp = 1. + pcoords[1];

  sf[0] = 0.25 * rm * sm;
  sf[1] = 0.25 * rp * sm;
  sf[2] = 0.25 * rp * sp;
  sf[3] = 0.25 * rm * sp;
}

void vlQuad::ShapeDerivs(float pcoords[3], float derivs[8])
{
  double rm, rp, sm, sp;

  rm = 1. - pcoords[0];
  rp = 1. + pcoords[0];
  sm = 1. - pcoords[1];
  sp = 1. + pcoords[1];

  derivs[0] = -0.25*sm;
  derivs[1] = 0.25*sm;
  derivs[2] = 0.25*sp;
  derivs[3] = -0.25*sp;
  derivs[4] = -0.25*rm;
  derivs[5] = -0.25*rp;
  derivs[6] = 0.25*rp;
  derivs[7] = 0.25*rm;
}

//
// Marching (convex) quadrilaterals
//
static int edges[4][2] = { {0,1}, {1,2}, {2,3}, {3,0} };

typedef int EDGE_LIST;
typedef struct {
       EDGE_LIST edges[5];
} LINE_CASES;

static LINE_CASES lineCases[] = { 
  {-1, -1, -1, -1, -1},
  {0, 3, -1, -1, -1},
  {1, 0, -1, -1, -1},
  {1, 3, -1, -1, -1},
  {2, 1, -1, -1, -1},
  {0, 3, 2, 1, -1},
  {2, 0, -1, -1, -1},
  {2, 3, -1, -1, -1},
  {3, 2, -1, -1, -1},
  {0, 2, -1, -1, -1},
  {1, 0, 3, 2, -1},
  {1, 2, -1, -1, -1},
  {3, 1, -1, -1, -1},
  {0, 1, -1, -1, -1},
  {3, 0, -1, -1, -1},
  {-1, -1, -1, -1, -1}
};

void vlQuad::Contour(float value, vlFloatScalars *cellScalars, 
                     vlFloatPoints *points, vlCellArray *verts, 
                     vlCellArray *lines, vlCellArray *polys, 
                     vlFloatScalars *scalars)
{
  static int CASE_MASK[4] = {1,2,4,8};
  LINE_CASES *lineCase;
  EDGE_LIST  *edge;
  int i, j, index, *vert;
  int pts[2];
  float t, *x1, *x2, x[3];

  // Build the case table
  for ( i=0, index = 0; i < 4; i++)
      if (cellScalars->GetScalar(i) >= value)
          index |= CASE_MASK[i];

  lineCase = lineCases + index;
  edge = lineCase->edges;

  for ( ; edge[0] > -1; edge += 2 )
    {
    for (i=0; i<2; i++) // insert line
      {
      vert = edges[edge[i]];
      t = (value - cellScalars->GetScalar(vert[0])) /
          (cellScalars->GetScalar(vert[1]) - cellScalars->GetScalar(vert[0]));
      x1 = this->Points.GetPoint(vert[0]);
      x2 = this->Points.GetPoint(vert[1]);
      for (j=0; j<3; j++) x[j] = x1[j] + t * (x2[j] - x1[j]);
      pts[i] = points->InsertNextPoint(x);
      scalars->InsertNextScalar(value);
      }
    lines->InsertNextCell(2,pts);
    }
}


vlCell *vlQuad::GetEdge(int edgeId)
{
  static vlLine line;

  // load point id's
  line.PointIds.SetId(0,this->PointIds.GetId(edgeId));
  line.PointIds.SetId(1,this->PointIds.GetId((edgeId+1) % 4));

  // load coordinates
  line.Points.SetPoint(0,this->Points.GetPoint(edgeId));
  line.Points.SetPoint(1,this->Points.GetPoint((edgeId+1) % 4));

  return &line;
}


