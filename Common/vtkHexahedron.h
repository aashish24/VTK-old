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

  int GetCellType() {return vlHEXAHEDRON;};
  int GetCellDimension() {return 3;};
  int GetNumberOfEdges() {return 12;};
  int GetNumberOfFaces() {return 6;};
  vlCell *GetEdge(int edgeId);
  vlCell *GetFace(int faceId);

  void Contour(float value, vlFloatScalars *cellScalars, 
               vlFloatPoints *points, vlCellArray *verts, 
               vlCellArray *lines, vlCellArray *polys, vlFloatScalars *s);
  int EvaluatePosition(float x[3], float closestPoint[3],
                       int& subId, float pcoords[3],
                       float& dist2, float weights[MAX_CELL_SIZE]);
  void EvaluateLocation(int& subId, float pcoords[3], float x[3],
                        float weights[MAX_CELL_SIZE]);

  void ShapeFunctions(float pcoords[3], float sf[8]);
  void ShapeDerivs(float pcoords[3], float derivs[24]);

};

#endif


