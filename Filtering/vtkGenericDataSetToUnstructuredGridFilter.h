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
// .NAME vtkGenericDataSetToUnstructuredGridFilter - abstract filter class
// .SECTION Description
// vtkGenericDataSetToUnstructuredGridFilter is an abstract filter class whose 
// subclasses take as input any dataset and generate an unstructured
// grid on output.

// .SECTION See Also
// vtkAppendFilter vtkConnectivityFilter vtkExtractGeometry
// vtkShrinkFilter vtkThreshold

#ifndef __vtkGenericDataSetToUnstructuredGridFilter_h
#define __vtkGenericDataSetToUnstructuredGridFilter_h

#include "vtkUnstructuredGridSource.h"

class vtkGenericDataSet;

class VTK_FILTERING_EXPORT vtkGenericDataSetToUnstructuredGridFilter
  : public vtkUnstructuredGridSource
{
public:
  vtkTypeRevisionMacro(vtkGenericDataSetToUnstructuredGridFilter,
                       vtkUnstructuredGridSource);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set / get the input data or filter.
  virtual void SetInput(vtkGenericDataSet *input);
  vtkGenericDataSet *GetInput();
  
protected:
  vtkGenericDataSetToUnstructuredGridFilter()
    {
      this->NumberOfRequiredInputs = 1;
    }
  
  ~vtkGenericDataSetToUnstructuredGridFilter()
    {
    }

private:
  vtkGenericDataSetToUnstructuredGridFilter(const vtkGenericDataSetToUnstructuredGridFilter&);  // Not implemented.
  void operator=(const vtkGenericDataSetToUnstructuredGridFilter&);  // Not implemented.
};

#endif


