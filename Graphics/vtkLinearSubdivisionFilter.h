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
// .NAME vtkLinearSubdivisionFilter - generate a subdivision surface using the Linear Scheme
// .SECTION Description
// vtkLinearSubdivisionFilter is a filter that generates output by
// subdividing its input polydata. Each subdivision iteration create 4
// new triangles for each triangle in the polydata.

// .SECTION See Also
// vtkInterpolatingSubdivisionFilter vtkButterflySubdivisionFilter

#ifndef __vtkLinearSubdivisionFilter_h
#define __vtkLinearSubdivisionFilter_h

#include "vtkInterpolatingSubdivisionFilter.h"
#include "vtkIntArray.h"

class VTK_GRAPHICS_EXPORT vtkLinearSubdivisionFilter : public vtkInterpolatingSubdivisionFilter
{
public:
  // Description:
  // Construct object with NumberOfSubdivisions set to 1.
  static vtkLinearSubdivisionFilter *New();
  vtkTypeRevisionMacro(vtkLinearSubdivisionFilter,vtkInterpolatingSubdivisionFilter);

protected:
  vtkLinearSubdivisionFilter () {};
  ~vtkLinearSubdivisionFilter () {};

  void GenerateSubdivisionPoints (vtkPolyData *inputDS, vtkIntArray *edgeData, vtkPoints *outputPts, vtkPointData *outputPD);

private:
  vtkLinearSubdivisionFilter(const vtkLinearSubdivisionFilter&);  // Not implemented.
  void operator=(const vtkLinearSubdivisionFilter&);  // Not implemented.
};

#endif


