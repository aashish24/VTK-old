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
//
// Computational class for triangles
//
#ifndef __vlTriangle_h
#define __vlTriangle_h

#include "Cell.hh"

class vlTriangle : public vlCell
{
public:
  vlTriangle() {};
  char *GetClassName() {return "vlTriangle";};

  float DistanceToPoint(float *x);

};

#endif


