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
// Computational class for hexahedron
//
#ifndef __vlHexahedron_h
#define __vlHexahedron_h

#include "Cell.hh"

class vlHexahedron : public vlCell
{
public:
  vlHexahedron() {};
  char *GetClassName() {return "vlHexahedron";};

  int CellDimension() {return 3;};
  void Contour(float value, vlFloatScalars *cellScalars, 
               vlFloatPoints *points, vlCellArray *verts, 
               vlCellArray *lines, vlCellArray *polys, vlFloatScalars *s);
  float EvaluatePosition(float x[3], int& subId, float pcoords[3]);
  void EvaluateLocation(int& subId, float pcoords[3], float x[3]);
  void ShapeFunctions(float pcoords[3], float sf[8]);
  void ShapeDerivs(float pcoords[3], float derivs[24]);

};

#endif


