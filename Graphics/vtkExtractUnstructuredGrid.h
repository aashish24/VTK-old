/*=========================================================================

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
// .NAME vtkExtractUnstructuredGrid - extract subset of unstructured grid geometry
// .SECTION Description
// vtkExtractUnstructuredGrid is a general-purpose filter to
// extract geometry (and associated data) from an unstructured grid
// dataset. The extraction process is controlled by specifying a range
// of point ids, cell ids, or a bounding box (referred to as "Extent").
// 

// .SECTION Caveats
// The input points are copied through to the output. This means
// unused points may be present in the output data.

// .SECTION See Also
// vtkStructuredPointsGeometryFilter vtkStructuredGridGeometryFilter
// vtkRectilinearGridGeometryFilter
// vtkExtractGeometry vtkExtractVOI

#ifndef __vtkExtractUnstructuredGrid_h
#define __vtkExtractUnstructuredGrid_h

#include "vtkUnstructuredGridToUnstructuredGridFilter.h"

class VTK_EXPORT vtkExtractUnstructuredGrid : public vtkUnstructuredGridToUnstructuredGridFilter
{
public:
  vtkTypeMacro(vtkExtractUnstructuredGrid,vtkUnstructuredGridToUnstructuredGridFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct with all types of clipping turned off.
  static vtkExtractUnstructuredGrid *New();

  // Description:
  // Turn on/off selection of geometry by point id.
  vtkSetMacro(PointClipping,int);
  vtkGetMacro(PointClipping,int);
  vtkBooleanMacro(PointClipping,int);

  // Description:
  // Turn on/off selection of geometry by cell id.
  vtkSetMacro(CellClipping,int);
  vtkGetMacro(CellClipping,int);
  vtkBooleanMacro(CellClipping,int);

  // Description:
  // Turn on/off selection of geometry via bounding box.
  vtkSetMacro(ExtentClipping,int);
  vtkGetMacro(ExtentClipping,int);
  vtkBooleanMacro(ExtentClipping,int);

  // Description:
  // Specify the minimum point id for point id selection.
  vtkSetClampMacro(PointMinimum,int,0,VTK_LARGE_INTEGER);
  vtkGetMacro(PointMinimum,int);

  // Description:
  // Specify the maximum point id for point id selection.
  vtkSetClampMacro(PointMaximum,int,0,VTK_LARGE_INTEGER);
  vtkGetMacro(PointMaximum,int);

  // Description:
  // Specify the minimum cell id for point id selection.
  vtkSetClampMacro(CellMinimum,int,0,VTK_LARGE_INTEGER);
  vtkGetMacro(CellMinimum,int);

  // Description:
  // Specify the maximum cell id for point id selection.
  vtkSetClampMacro(CellMaximum,int,0,VTK_LARGE_INTEGER);
  vtkGetMacro(CellMaximum,int);

  // Description:
  // Specify a (xmin,xmax, ymin,ymax, zmin,zmax) bounding box to clip data.
  void SetExtent(float xMin, float xMax, float yMin, float yMax, 
		 float zMin, float zMax);

  // Description:
  // Set / get a (xmin,xmax, ymin,ymax, zmin,zmax) bounding box to clip data.
  void SetExtent(float *extent);
  float *GetExtent() { return this->Extent;};

protected:
  vtkExtractUnstructuredGrid();
  ~vtkExtractUnstructuredGrid() {};
  vtkExtractUnstructuredGrid(const vtkExtractUnstructuredGrid&) {};
  void operator=(const vtkExtractUnstructuredGrid&) {};

  void Execute();

  int PointMinimum;
  int PointMaximum;
  int CellMinimum;
  int CellMaximum;
  float Extent[6];
  int PointClipping;
  int CellClipping;
  int ExtentClipping;
};

#endif


