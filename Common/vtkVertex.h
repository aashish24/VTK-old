/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Toolkit. No part of this file
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
// .NAME vtkVertex - a cell that represents a 3D point
// .SECTION Description
// vtkVertex is a concrete implementation of vtkCell to represent a 3D point.

#ifndef __vtkVertex_h
#define __vtkVertex_h

#include "Cell.hh"

class vtkVertex : public vtkCell
{
public:
  vtkVertex() {};
  vtkVertex(const vtkVertex& p);
  char *GetClassName() {return "vtkVertex";};

  vtkCell *MakeObject() {return new vtkVertex(*this);};
  int GetCellType() {return vtkVERTEX;};
  int GetCellDimension() {return 0;};
  int GetNumberOfEdges() {return 0;};
  int GetNumberOfFaces() {return 0;};
  vtkCell *GetEdge(int edgeId) {return 0;};
  vtkCell *GetFace(int faceId) {return 0;};
  int CellBoundary(int subId, float pcoords[3], vtkIdList& pts);

  void Contour(float value, vtkFloatScalars *cellScalars, 
               vtkFloatPoints *points, vtkCellArray *verts, 
               vtkCellArray *lines, vtkCellArray *polys, vtkFloatScalars *s);
  int EvaluatePosition(float x[3], float closestPoint[3], 
                       int& subId, float pcoords[3], 
                       float& dist2, float weights[MAX_CELL_SIZE]);
  void EvaluateLocation(int& subId, float pcoords[3], float x[3],
                        float weights[MAX_CELL_SIZE]);
  int IntersectWithLine(float p1[3], float p2[3], float tol, float& t,
                        float x[3], float pcoords[3], int& subId);
  int Triangulate(int index, vtkFloatPoints &pts);
  void Derivatives(int subId, float pcoords[3], float *values, 
                   int dim, float *derivs);
};

#endif


