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
// .NAME vtkQuadraticQuad - cell represents a parabolic, 8-node isoparametric quad
// .SECTION Description
// vtkQuadraticQuad is a concrete implementation of vtkNonLinearCell to
// represent a two-dimensional, 8-node isoparametric parabolic quadrilateral
// element. The interpolation is the standard finite element, quadratic
// isoparametric shape function. The cell includes a mid-edge node for each
// of the four edges of the cell. The ordering of the eight points defining
// the cell are point ids (0-3,4-7) where ids 0-3 define the four corner
// vertices of the quad; ids 4-7 define the midedge nodes (0,1), (1,2),
// (2,3), (3,0).

// .SECTION See Also
// vtkQuadraticEdge vtkQuadraticTriangle vtkQuadraticTetra
// vtkQuadraticHexahedron


#ifndef __vtkQuadraticQuad_h
#define __vtkQuadraticQuad_h

#include "vtkNonLinearCell.h"

class vtkPolyData;
class vtkQuadraticEdge;
class vtkQuad;
class vtkPointData;
class vtkCellData;
class vtkDataArray;
class vtkDoubleArray;

class VTK_COMMON_EXPORT vtkQuadraticQuad : public vtkNonLinearCell
{
public:
  static vtkQuadraticQuad *New();
  vtkTypeRevisionMacro(vtkQuadraticQuad,vtkNonLinearCell);

  // Description:
  // Implement the vtkCell API. See the vtkCell API for descriptions 
  // of these methods.
  int GetCellType() {return VTK_QUADRATIC_QUAD;};
  int GetCellDimension() {return 2;}
  int GetNumberOfEdges() {return 4;}
  int GetNumberOfFaces() {return 0;}
  vtkCell *GetEdge(int);
  vtkCell *GetFace(int) {return 0;}

  int CellBoundary(int subId, double pcoords[3], vtkIdList *pts);
  void Contour(double value, vtkDataArray *cellScalars, 
               vtkPointLocator *locator, vtkCellArray *verts, 
               vtkCellArray *lines, vtkCellArray *polys, 
               vtkPointData *inPd, vtkPointData *outPd,
               vtkCellData *inCd, vtkIdType cellId, vtkCellData *outCd);
  int EvaluatePosition(double x[3], double* closestPoint,
                       int& subId, double pcoords[3], 
                       double& dist2, double *weights);
  void EvaluateLocation(int& subId, double pcoords[3], double x[3],
                        double *weights);
  int Triangulate(int index, vtkIdList *ptIds, vtkPoints *pts);
  void Derivatives(int subId, double pcoords[3], double *values, 
                   int dim, double *derivs);
  virtual double *GetParametricCoords();

  // Description:
  // Clip this quadratic quad using scalar value provided. Like contouring, 
  // except that it cuts the quad to produce linear triangles.
  void Clip(double value, vtkDataArray *cellScalars, 
            vtkPointLocator *locator, vtkCellArray *polys,
            vtkPointData *inPd, vtkPointData *outPd,
            vtkCellData *inCd, vtkIdType cellId, vtkCellData *outCd,
            int insideOut);

  // Description:
  // Line-edge intersection. Intersection has to occur within [0,1] parametric
  // coordinates and with specified tolerance.
  int IntersectWithLine(double p1[3], double p2[3], double tol, double& t,
                        double x[3], double pcoords[3], int& subId);

  
  // Description:
  // Quadratic quad specific methods. 
  static void InterpolationFunctions(double pcoords[3], double weights[8]);
  static void InterpolationDerivs(double pcoords[3], double derivs[16]);

protected:
  vtkQuadraticQuad();
  ~vtkQuadraticQuad();

  vtkQuadraticEdge *Edge;
  vtkQuad          *Quad;
  vtkPointData     *PointData;
  vtkCellData      *CellData;
  vtkDoubleArray    *Scalars;

  void Subdivide(double *weights);
  void InterpolateAttributes(vtkPointData *inPd, vtkCellData *inCd,
                             vtkIdType cellId, double *weights);

private:
  vtkQuadraticQuad(const vtkQuadraticQuad&);  // Not implemented.
  void operator=(const vtkQuadraticQuad&);  // Not implemented.
};

#endif


