/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Library. No part of this file or its 
contents may be copied, reproduced or altered in any way without the express
written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
//
// Class for performing common math operations (e.g., dot, cross products)
//
#ifndef __vlMath_hh
#define __vlMath_hh

#include <math.h>

class vlMath
{
public:
  vlMath();
  float Pi() {return 3.14159265358979;};
  float DegreesToRadians() {return 0.018977369;};
  float Dot(float x[3], float y[3]) 
    {return x[0]*y[0] + x[1]*y[1] + x[2]*y[2];};
  void Cross(float x[3], float y[3], float z[3])
    {z[0] = x[1]*y[2] - x[2]*y[1]; 
     z[1] = x[2]*y[0] - x[0]*y[2];
     z[2] = x[0]*y[1] - x[1]*y[0];};
  float Norm(float x[3])
    {return sqrt(x[0]*x[0] + x[1]*x[1] + x[2]*x[2]);};
  void RandomSeed(long s);  
  float Random();  
private:
  static long Seed;
};

#endif
