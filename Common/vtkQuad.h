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
// Computational class for quads.
//
#ifndef __vlQuad_h
#define __vlQuad_h

#include "Cell.hh"

class vlQuad : public vlCell
{
public:
  vlQuad() {};
  char *GetClassName() {return "vlQuad";};

  int CellDimension() {return 2;};
  void Contour(float value, vlFloatScalars *cellScalars, 
               vlFloatPoints *points, vlCellArray *verts, 
               vlCellArray *lines, vlCellArray *polys, vlFloatScalars *s);
  int EvaluatePosition(float x[3], int& subId, float pcoords[3], float& dist2);
  void EvaluateLocation(int& subId, float pcoords[3], float x[3]);
  void ShapeFunctions(float pcoords[3], float sf[4]);
  void ShapeDerivs(float pcoords[3], float derivs[12]);

};

#endif


