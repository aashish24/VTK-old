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
#include "Point.hh"
#include "vlMath.hh"

float vlPoint::EvaluatePosition(float x[3], int& subId, float pcoords[3])
{
  int numPts;
  float *X;
  float dist2;
  vlMath math;

  subId = 0;
  pcoords[1] = pcoords[2] = 0.0;

  X = this->Points->GetPoint(0);

  dist2 = math.Distance2BetweenPoints(X,x);

  if (dist2 == 0.0)
    {
    pcoords[0] = 0.0;
    }
  else
    {
    pcoords[0] = -10.0;
    }

  return dist2;
}

void vlPoint::EvaluateLocation(int& subId, float pcoords[3], float x[3])
{
  float *X = this->Points->GetPoint(0);
  x[0] = X[0];
  x[1] = X[1];
  x[2] = X[2];
}
