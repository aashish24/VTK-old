/*========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen 
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
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkPolyLine - cell represents a set of 1D lines
// .SECTION Description
// vtkPolyLine is a concrete implementation of vtkCell to represent a set
// of 1D lines.

#ifndef __vtkPolyLine_h
#define __vtkPolyLine_h

#include "vtkCell.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkNormals.h"
#include "vtkLine.h"

class VTK_EXPORT vtkPolyLine : public vtkCell
{
public:
  static vtkPolyLine *New();
  vtkTypeMacro(vtkPolyLine,vtkCell);

  // Description:
  // Given points and lines, compute normals to lines. These are not true 
  // normals, they are "orientation" normals used by classes like vtkTubeFilter
  // that control the rotation around the line. The normals try to stay pointing
  // in the same direction as much as possible (i.e., minimal rotation).
  int GenerateSlidingNormals(vtkPoints *pts, vtkCellArray *ca, vtkNormals *n)
    { return this->GenerateSlidingNormals(pts,ca, n->GetData()); }
  int GenerateSlidingNormals(vtkPoints *, vtkCellArray *, vtkDataArray *);

  // Description:
  // See the vtkCell API for descriptions of these methods.
  vtkCell *MakeObject();
  int GetCellType() {return VTK_POLY_LINE;};
  int GetCellDimension() {return 1;};
  int GetNumberOfEdges() {return 0;};
  int GetNumberOfFaces() {return 0;};
  vtkCell *GetEdge(int vtkNotUsed(edgeId)) {return 0;};
  vtkCell *GetFace(int vtkNotUsed(faceId)) {return 0;};
  int CellBoundary(int subId, float pcoords[3], vtkIdList *pts);
  void Contour(float value, vtkDataArray *cellScalars, 
               vtkPointLocator *locator, vtkCellArray *verts, 
               vtkCellArray *lines, vtkCellArray *polys, 
               vtkPointData *inPd, vtkPointData *outPd,
               vtkCellData *inCd, vtkIdType cellId, vtkCellData *outCd);
  virtual void Contour(float value, vtkScalars *cellScalars, 
                       vtkPointLocator *locator, vtkCellArray *verts, 
                       vtkCellArray *lines, vtkCellArray *polys, 
                       vtkPointData *inPd, vtkPointData *outPd,
                       vtkCellData *inCd, vtkIdType cellId,
                       vtkCellData *outCd)
    {
      VTK_LEGACY_METHOD("Contour", "4.0");
      this->Contour(value, cellScalars->GetData(), locator, verts, 
		    lines, polys, inPd, outPd, inCd, cellId, outCd);
    }
  void Clip(float value, vtkDataArray *cellScalars, 
            vtkPointLocator *locator, vtkCellArray *lines,
            vtkPointData *inPd, vtkPointData *outPd,
            vtkCellData *inCd, vtkIdType cellId, vtkCellData *outCd,
            int insideOut);
  virtual void Clip(float value, vtkScalars *cellScalars, 
                    vtkPointLocator *locator, vtkCellArray *connectivity,
                    vtkPointData *inPd, vtkPointData *outPd,
                    vtkCellData *inCd, vtkIdType cellId, vtkCellData *outCd, 
                    int insideOut)
    {
      vtkWarningMacro("The use of this method has been deprecated.You should use vtkGenericCell::Clip(float, vtkDataArray*, vtkPointLocator*, vtkCellArray*, vtkPointData*, vtkPointData*, vtkCellData*, vtkIdType, vtkCellData*, int) instead.");
      this->Clip(value, cellScalars->GetData(), locator, connectivity, 
		 inPd, outPd, inCd, cellId, outCd, insideOut);
    }
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
  // Return the center of the point cloud in parametric coordinates.
  int GetParametricCenter(float pcoords[3]);

  
protected:
  vtkPolyLine();
  ~vtkPolyLine();
  vtkPolyLine(const vtkPolyLine&);
  void operator=(const vtkPolyLine&);

  vtkLine *Line;

};

#endif


