/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1998 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
// .NAME vtkUnstructuredGrid - dataset represents arbitrary combinations of all possible cell types
// .SECTION Description
// vtkUnstructuredGrid is a data object that is a concrete implementation 
// of vtkDataSet. vtkUnstructuredGrid represents any combinations of any cell
// types. This includes 0D (e.g., points), 1D (e.g., lines, polylines), 2D 
// (e.g., triangles, polygons), and 3D (e.g., hexahedron, tetrahedron).

#ifndef __vtkUnstructuredGrid_h
#define __vtkUnstructuredGrid_h

#include "vtkPointSet.h"
#include "vtkIdList.h"
#include "vtkCellArray.h"
#include "vtkCellTypes.h"
#include "vtkCellLinks.h"
class vtkVertex;
class vtkPolyVertex;
class vtkLine;
class vtkPolyLine;
class vtkTriangle;
class vtkTriangleStrip;
class vtkPixel;
class vtkQuad;
class vtkPolygon;
class vtkTetra;
class vtkVoxel;
class vtkHexahedron;
class vtkWedge;
class vtkPyramid;

class VTK_EXPORT vtkUnstructuredGrid : public vtkPointSet {
public:
  vtkUnstructuredGrid();
  vtkUnstructuredGrid(const vtkUnstructuredGrid& up);
  ~vtkUnstructuredGrid();
  static vtkUnstructuredGrid *New() {return new vtkUnstructuredGrid;};
  const char *GetClassName() {return "vtkUnstructuredGrid";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Standard vtkDataSet API methods. See vtkDataSet for more information.
  int GetDataSetType() {return VTK_UNSTRUCTURED_GRID;};
  void Allocate(int numCells=1000, int extSize=1000);
  int InsertNextCell(int type, int npts, int *pts);
  int InsertNextCell(int type, vtkIdList *ptIds);
  void Reset();
  void SetCells(int *types, vtkCellArray *cells);
  vtkCellArray *GetCells() {return this->Connectivity;};
  vtkDataObject *MakeObject() {return new vtkUnstructuredGrid;};
  void CopyStructure(vtkDataSet *ds);
  int GetNumberOfCells();
  vtkCell *GetCell(int cellId);
  void GetCell(int cellId, vtkGenericCell *cell);
  void GetCellBounds(int cellId, float bounds[6]);
  void GetCellPoints(int cellId, vtkIdList *ptIds);
  void GetPointCells(int ptId, vtkIdList *cellIds);
  int GetCellType(int cellId);
  void Squeeze();
  void Initialize();
  int GetMaxCellSize();
  void BuildLinks();
  void GetCellPoints(int cellId, int& npts, int* &pts);
  void ReplaceCell(int cellId, int npts, int *pts);
  int InsertNextLinkedCell(int type, int npts, int *pts); 
  void RemoveReferenceToCell(int ptId, int cellId);
  void AddReferenceToCell(int ptId, int cellId);
  void ResizeCellList(int ptId, int size);

  // Description:
  // For legacy compatibility. Do not use.
  void GetCellPoints(int cellId, vtkIdList &ptIds)
    {this->GetCellPoints(cellId, &ptIds);}
  void GetPointCells(int ptId, vtkIdList &cellIds)
    {this->GetPointCells(ptId, &cellIds);}
  int InsertNextCell(int type, vtkIdList &pts) {return this->InsertNextCell(type, &pts);}
  

protected:
  // used by GetCell method
  vtkVertex *Vertex;
  vtkPolyVertex *PolyVertex;
  vtkLine *Line;
  vtkPolyLine *PolyLine;
  vtkTriangle *Triangle;
  vtkTriangleStrip *TriangleStrip;
  vtkPixel *Pixel;
  vtkQuad *Quad;
  vtkPolygon *Polygon;
  vtkTetra *Tetra;
  vtkVoxel *Voxel;
  vtkHexahedron *Hexahedron;
  vtkWedge *Wedge;
  vtkPyramid *Pyramid;
  
  // points inherited
  // point data (i.e., scalars, vectors, normals, tcoords) inherited
  vtkCellTypes *Cells;
  vtkCellArray *Connectivity;
  vtkCellLinks *Links;
};

#endif


