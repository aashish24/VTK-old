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
// .NAME vtkGenericDataSetTessellator - tessellates generic, higher-order datasets into linear cells
// .SECTION Description

// vtkGenericDataSetTessellator is a filter that subdivides a
// vtkGenericDataSet into linear elements (i.e., linear VTK
// cells). Tetrahedra are produced from 3D cells; triangles from 2D cells;
// and lines from 1D cells. The subdivision process depends on the cell
// tessellator associated with the input generic dataset, and its associated
// error metric. (These can be specified by the user if necessary.)
//
// This filter is typically used to convert a higher-order, complex dataset
// represented vtkGenericDataSet into a conventional vtkDataSet that can
// be operated on by linear VTK graphics filters.

// .SECTION See Also
// vtkGenericDataSetTessellator vtkGenericCellTessellator 
// vtkGenericSubdivisionErrorMetric


#ifndef __vtkGenericDataSetTessellator_h
#define __vtkGenericDataSetTessellator_h

#include "vtkGenericDataSetToUnstructuredGridFilter.h"

class VTK_GENERIC_FILTERING_EXPORT vtkGenericDataSetTessellator : public vtkGenericDataSetToUnstructuredGridFilter
{
public:
  // Description:
  // Standard VTK methods.
  static vtkGenericDataSetTessellator *New();
  vtkTypeRevisionMacro(vtkGenericDataSetTessellator,
                       vtkGenericDataSetToUnstructuredGridFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkGenericDataSetTessellator();
  ~vtkGenericDataSetTessellator();

  void Execute();

private:
  vtkGenericDataSetTessellator(const vtkGenericDataSetTessellator&);  // Not implemented.
  void operator=(const vtkGenericDataSetTessellator&);  // Not implemented.
};

#endif
