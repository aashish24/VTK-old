/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkTriangleStrip - a cell that represents a triangle strip
// .SECTION Description
// vtkTriangleStrip is a concrete implementation of vtkCell to represent a 2D 
// triangle strip. A triangle strip is a compact representation of triangles
// connected edge to edge in strip fashion. The connectivity of a triangle 
// strip is three points defining an initial triangle, then for each 
// additional triangle, a single point that, combined with the previous two
// points, defines the next triangle.

#ifndef __vtkTriangleStrip_h
#define __vtkTriangleStrip_h

#include "vtkCell.h"

class vtkLine;
class vtkTriangle;

class VTK_COMMON_EXPORT vtkTriangleStrip : public vtkCell
{
public:
  static vtkTriangleStrip *New();
  vtkTypeRevisionMacro(vtkTriangleStrip,vtkCell);

  // Description:
  // See the vtkCell API for descriptions of these methods.
  int GetCellType() {return VTK_TRIANGLE_STRIP;};
  int GetCellDimension() {return 2;};
  int GetNumberOfEdges() {return this->GetNumberOfPoints();};
  int GetNumberOfFaces() {return 0;};
  vtkCell *GetEdge(int edgeId);
  vtkCell *GetFace(int vtkNotUsed(faceId)) {return 0;};
  int CellBoundary(int subId, float pcoords[3], vtkIdList *pts);
  void Contour(float value, vtkDataArray *cellScalars, 
               vtkPointLocator *locator, vtkCellArray *verts, 
               vtkCellArray *lines, vtkCellArray *polys, 
               vtkPointData *inPd, vtkPointData *outPd,
               vtkCellData *inCd, vtkIdType cellId, vtkCellData *outCd);
  void Clip(float value, vtkDataArray *cellScalars, 
            vtkPointLocator *locator, vtkCellArray *polys,
            vtkPointData *inPd, vtkPointData *outPd,
            vtkCellData *inCd, vtkIdType cellId, vtkCellData *outCd,
            int insideOut);

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
  int IsPrimaryCell() {return 0;}

  // Description:
  // Return the center of the point cloud in parametric coordinates.
  int GetParametricCenter(float pcoords[3]);

  // Description:
  // Given a triangle strip, decompose it into a list of (triangle) 
  // polygons. The polygons are appended to the end of the list of triangles.
  static void DecomposeStrip(int npts, vtkIdType *pts, vtkCellArray *tris);
  

protected:
  vtkTriangleStrip();
  ~vtkTriangleStrip();

  vtkLine *Line;
  vtkTriangle *Triangle;
  
private:
  vtkTriangleStrip(const vtkTriangleStrip&);  // Not implemented.
  void operator=(const vtkTriangleStrip&);  // Not implemented.
};

#endif


