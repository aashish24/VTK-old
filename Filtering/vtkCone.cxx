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
#include "Cone.hh"

// Description
// Construct cone with angle of 45 degrees.
vlCone::vlCone()
{
  this->Angle = 45.0;
}

// Description
// Evaluate cone equation.
float vlCone::EvaluateFunction(float x[3])
{
  return 0;
}

// Description
// Evaluate cone normal.
void vlCone::EvaluateGradient(float x[3], float g[3])
{
}

void vlCone::PrintSelf(ostream& os, vlIndent indent)
{
  vlImplicitFunction::PrintSelf(os,indent);

  os << indent << "Angle: " << this->Angle << "\n";
}
