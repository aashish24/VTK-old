/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1996 Ken Martin, Will Schroeder, Bill Lorensen.

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
// .NAME vtkStructuredData - abstract class for topologically regular data
// .SECTION Description
// vtkStructuredData is an abstract class that specifies an interface for
// topologically regular data. Regular data is data that can be accessed
// in rectangular fashion using an i-j-k index. A finite difference grid,
// a volume, or a pixmap are all considered regular.

#ifndef __vtkStructuredData_h
#define __vtkStructuredData_h

#include "vtkObject.h"
#include "vtkIdList.h"
#include "vtkDataSet.h"

#define VTK_UNCHANGED 0
#define VTK_SINGLE_POINT 1
#define VTK_X_LINE 2
#define VTK_Y_LINE 3
#define VTK_Z_LINE 4
#define VTK_XY_PLANE 5
#define VTK_YZ_PLANE 6
#define VTK_XZ_PLANE 7
#define VTK_XYZ_GRID 8

class VTK_EXPORT vtkStructuredData : public vtkObject 
{
public:
  static vtkStructuredData *New() {return new vtkStructuredData;};
  char *GetClassName() {return "vtkStructuredData";};
  static int SetDimensions(int inDim[3], int dim[3]);
  static int GetDataDimension(int dataDescription);

  static void GetCellPoints(int cellId, vtkIdList& ptIds, 
                     int dataDescription, int dim[3]);
  static void GetPointCells(int ptId, vtkIdList& cellIds, int dim[3]);

  static int ComputePointId(int dim[3], int ijk[3]);
  static int ComputeCellId(int dim[3], int ijk[3]);
};

// Description:
// Given a location in structured coordinates (i-j-k), and the dimensions
// of the structured dataset, return the point id.
inline int vtkStructuredData::ComputePointId(int dim[3], int ijk[3])
{
  return ijk[2]*dim[0]*dim[1] + ijk[1]*dim[0] + ijk[0];
}

// Description:
// Given a location in structured coordinates (i-j-k), and the dimensions
// of the structured dataset, return the cell id.
inline int vtkStructuredData::ComputeCellId(int dim[3], int ijk[3])
{
  return ijk[2]*(dim[0]-1)*(dim[1]-1) + ijk[1]*(dim[0]-1) + ijk[0];
}

#endif
