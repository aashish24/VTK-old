/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Description:
---------------------------------------------------------------------------
This file is part of the Visualization Library. No part of this file
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
#include "Line.hh"
#include "vlMath.hh"

#define NO_INTERSECTION 1
#define INTERSECTION 2
#define ON_LINE 6

float vlLine::EvaluatePosition(float x[3], int& subId, float pcoords[3])
{
  float *a1, *a2, a21[3], denom, num, *closestPoint;
  int i, numPts;
  vlMath math;

  subId = 0;
  pcoords[1] = pcoords[2] = 0.0;

  a1 = this->Points.GetPoint(0);
  a2 = this->Points.GetPoint(1);
//
//   Determine appropriate vectors
// 
  for (i=0; i<3; i++) a21[i] = a2[i] - a1[i];
//
//   Get parametric location
//
  num = a21[0]*(x[0]-a1[0]) + a21[1]*(x[1]-a1[1]) + a21[2]*(x[2]-a1[2]);
  denom = math.Dot(a21,a21);

  if ( (denom = math.Dot(a21,a21)) < fabs(TOL*num) )
    {
    return LARGE_FLOAT;
    }
  else 
    {
    pcoords[0] = num / denom;
    }
//
// If parametric coordinate is within 0<=p<=1, then the point is closest to
// the line.  Otherwise, it's closest to a point at the end of the line.
//
  if ( pcoords[0] < 0.0 )
    {
    closestPoint = a1;
    }
  else if ( pcoords[0] > 1.0 )
    {
    closestPoint = a2;
    }
  else
    {
    closestPoint = a21;
    for (i=0; i<3; i++) a21[i] = a1[i] + pcoords[0]*a21[i];
    }

  return math.Distance2BetweenPoints(closestPoint,x);
}

void vlLine::EvaluateLocation(int& subId, float pcoords[3], float x[3])
{
  int i;
  float *a1 = this->Points.GetPoint(0);
  float *a2 = this->Points.GetPoint(1);

  for (i=0; i<3; i++) 
    {
    x[i] = a1[i] + pcoords[0]*(a2[i] - a1[i]);
    }
}

//
//  Intersect two 3D lines
//
int vlLine::Intersection (float a1[3], float a2[3], float b1[3], float b2[3],
                          float& u, float& v)
{
  float a21[3], b21[3], b1a1[3];
  float sys[2][2], c[2], det;
  int i;
  vlMath math;
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
  sys[0][0] = math.Dot ( a21, a21 );
  sys[0][1] = -math.Dot ( a21, b21 );
  sys[1][0] = sys[0][1];
  sys[1][1] = math.Dot ( b21, b21 );
//
//   Compute the least squares system constant term.
//
  c[0] = math.Dot ( a21, b1a1 );
  c[1] = -math.Dot ( b21, b1a1 );
//
//  Solve the system of equations
//
  if ( (det=math.Determinate2x2(sys[0],sys[1])) <= TOL )
    {
    return ON_LINE;
    }
  else 
    {
    u = math.Determinate2x2(c,sys[1]) / det;
    v = math.Determinate2x2(sys[0],c) / det;
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
