/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkQuad - a cell that represents a 2D quadrilateral
// .SECTION Description
// vtkQuad is a concrete implementation of vtkCell to represent a 2D 
// quadrilateral.

#ifndef __vtkQuad_h
#define __vtkQuad_h

#include "vtkCell.h"
#include "vtkLine.h"

class VTK_EXPORT vtkQuad : public vtkCell
{
public:
  static vtkQuad *New();
  vtkTypeMacro(vtkQuad,vtkCell);

  // Description:
  // See the vtkCell API for descriptions of these methods.
  vtkCell *MakeObject();
  int GetCellType() {return VTK_QUAD;};
  int GetCellDimension() {return 2;};
  int GetNumberOfEdges() {return 4;};
  int GetNumberOfFaces() {return 0;};
  vtkCell *GetEdge(int edgeId);
  vtkCell *GetFace(int) {return 0;};
  int CellBoundary(int subId, float pcoords[3], vtkIdList *pts);
  void Contour(float value, vtkScalars *cellScalars, 
               vtkPointLocator *locator, vtkCellArray *verts, 
               vtkCellArray *lines, vtkCellArray *polys,
               vtkPointData *inPd, vtkPointData *outPd,
               vtkCellData *inCd, int cellId, vtkCellData *outCd);
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
  void Clip(float value, vtkScalars *cellScalars, 
            vtkPointLocator *locator, vtkCellArray *polys,
            vtkPointData *inPd, vtkPointData *outPd,
            vtkCellData *inCd, int cellId, vtkCellData *outCd, int insideOut);

  // Description:
  // vtkQuad specific methods.
  static void InterpolationFunctions(float pcoords[3], float sf[4]);
  static void InterpolationDerivs(float pcoords[3], float derivs[8]);

#ifndef VTK_REMOVE_LEGACY_CODE
  // Description:
  // For legacy compatibility. Do not use.
  int CellBoundary(int subId, float pcoords[3], vtkIdList &pts)
    {VTK_LEGACY_METHOD(CellBoundary,"3.2"); return this->CellBoundary(subId, pcoords, &pts);}
  int Triangulate(int index, vtkIdList &ptIds, vtkPoints &pts)
    {VTK_LEGACY_METHOD(Triangulate,"3.2"); return this->Triangulate(index, &ptIds, &pts);}
#endif
  

protected:
  vtkQuad();
  ~vtkQuad();
  vtkQuad(const vtkQuad&) {};
  void operator=(const vtkQuad&) {};

  vtkLine *Line;

};

#endif


