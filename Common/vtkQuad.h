/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkQuad - a cell that represents a 2D quadrilateral
// .SECTION Description
// vtkQuad is a concrete implementation of vtkCell to represent a 2D 
// quadrilateral. vtkQuad is defined by the four points (0,1,2,3) in
// counterclockwise order. vtkQuad uses the standard isoparametric
// interpolation functions for a linear quadrilateral.

#ifndef __vtkQuad_h
#define __vtkQuad_h

#include "vtkCell.h"

class vtkLine;
class vtkTriangle;

class VTK_COMMON_EXPORT vtkQuad : public vtkCell
{
public:
  static vtkQuad *New();
  vtkTypeRevisionMacro(vtkQuad,vtkCell);

  // Description:
  // See the vtkCell API for descriptions of these methods.
  int GetCellType() {return VTK_QUAD;};
  int GetCellDimension() {return 2;};
  int GetNumberOfEdges() {return 4;};
  int GetNumberOfFaces() {return 0;};
  vtkCell *GetEdge(int edgeId);
  vtkCell *GetFace(int) {return 0;};
  int CellBoundary(int subId, float pcoords[3], vtkIdList *pts);
  void Contour(float value, vtkDataArray *cellScalars, 
               vtkPointLocator *locator, vtkCellArray *verts, 
               vtkCellArray *lines, vtkCellArray *polys,
               vtkPointData *inPd, vtkPointData *outPd,
               vtkCellData *inCd, vtkIdType cellId, vtkCellData *outCd);
  int EvaluatePosition(float x[3], float* closestPoint,
                       int& subId, float pcoords[3],
                       float& dist2, float *weights);
  void EvaluateLocation(int& subId, float pcoords[3], float x[3],
                        float *weights);
  int IntersectWithLine(float p1[3], float p2[3], float tol, float& t,
                        float x[3], float pcoords[3], int& subId);
  int Triangulate(int index, vtkIdList *ptIds, vtkPoints *pts);
  void Derivatives(int subId, float pcoords[3], float *values, 
                   int dim, float *derivs);

  // Description:
  // Clip this quad using scalar value provided. Like contouring, except
  // that it cuts the quad to produce other quads and/or triangles.
  void Clip(float value, vtkDataArray *cellScalars, 
            vtkPointLocator *locator, vtkCellArray *polys,
            vtkPointData *inPd, vtkPointData *outPd,
            vtkCellData *inCd, vtkIdType cellId, vtkCellData *outCd,
            int insideOut);

  // Description:
  // vtkQuad specific methods.
  static void InterpolationFunctions(float pcoords[3], float sf[4]);
  static void InterpolationDerivs(float pcoords[3], float derivs[8]);

protected:
  vtkQuad();
  ~vtkQuad();

  vtkLine     *Line;
  vtkTriangle *Triangle;

private:
  vtkQuad(const vtkQuad&);  // Not implemented.
  void operator=(const vtkQuad&);  // Not implemented.
};

#endif


