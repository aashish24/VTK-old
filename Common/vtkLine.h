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
// .NAME vtkLine - cell represents a 1D line
// .SECTION Description
// vtkLine is a concrete implementation of vtkCell to represent a 1D line.

#ifndef __vtkLine_h
#define __vtkLine_h

#include "vtkCell.h"

class VTK_COMMON_EXPORT vtkLine : public vtkCell
{
public:
  static vtkLine *New();
  vtkTypeRevisionMacro(vtkLine,vtkCell);

  // Description:
  // See the vtkCell API for descriptions of these methods.
  int GetCellType() {return VTK_LINE;};
  int GetCellDimension() {return 1;};
  int GetNumberOfEdges() {return 0;};
  int GetNumberOfFaces() {return 0;};
  vtkCell *GetEdge(int) {return 0;};
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
  int Triangulate(int index, vtkIdList *ptIds, vtkPoints *pts);
  void Derivatives(int subId, float pcoords[3], float *values, 
                   int dim, float *derivs);

  // Description:
  // Clip this line using scalar value provided. Like contouring, except
  // that it cuts the line to produce other lines.
  void Clip(float value, vtkDataArray *cellScalars, 
            vtkPointLocator *locator, vtkCellArray *lines,
            vtkPointData *inPd, vtkPointData *outPd,
            vtkCellData *inCd, vtkIdType cellId, vtkCellData *outCd,
            int insideOut);

  // Description:
  // Line-line intersection. Intersection has to occur within [0,1] parametric
  // coordinates and with specified tolerance.
  int IntersectWithLine(float p1[3], float p2[3], float tol, float& t,
                        float x[3], float pcoords[3], int& subId);


  // Description:
  // Performs intersection of two finite 3D lines. An intersection is found if
  // the projection of the two lines onto the plane perpendicular to the cross
  // product of the two lines intersect. The parameters (u,v) are the 
  // parametric coordinates of the lines at the position of closest approach.
  static int Intersection(float p1[3], float p2[3], float x1[3], float x2[3],
                          float& u, float& v);

  
  // Description:
  // Compute distance to finite line. Returns parametric coordinate t 
  // and point location on line.
  static float DistanceToLine(float x[3], float p1[3], float p2[3], 
                              float &t, float closestPoint[3]);
  
  
  // Description:
  // Determine the distance of the current vertex to the edge defined by
  // the vertices provided.  Returns distance squared. Note: line is assumed
  // infinite in extent.
  static float DistanceToLine(float x[3], float p1[3], float p2[3]);

  // Description:
  // Line specific methods.
  static void InterpolationFunctions(float pcoords[3], float weights[2]);

protected:
  vtkLine();
  ~vtkLine() {};

private:
  vtkLine(const vtkLine&);  // Not implemented.
  void operator=(const vtkLine&);  // Not implemented.
};

#endif


