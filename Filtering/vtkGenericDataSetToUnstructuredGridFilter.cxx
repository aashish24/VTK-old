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
#include "vtkGenericDataSetToUnstructuredGridFilter.h"

#include "vtkGenericDataSet.h"

#if VTK_MAJOR_VERSION>4 || (VTK_MAJOR_VERSION==4 && VTK_MINOR_VERSION>4)
#include "vtkInformation.h"
#endif

vtkCxxRevisionMacro(vtkGenericDataSetToUnstructuredGridFilter, "$Revision$");

vtkGenericDataSetToUnstructuredGridFilter
::vtkGenericDataSetToUnstructuredGridFilter()
{
#if VTK_MAJOR_VERSION>4 || (VTK_MAJOR_VERSION==4 && VTK_MINOR_VERSION>4)
  this->SetNumberOfInputPorts(1);
#endif
  this->NumberOfRequiredInputs = 1;
}


//----------------------------------------------------------------------------
// Specify the input data or filter.
void vtkGenericDataSetToUnstructuredGridFilter::SetInput(
  vtkGenericDataSet *input)
{
  this->vtkProcessObject::SetNthInput(0, input);
}

//----------------------------------------------------------------------------
// Specify the input data or filter.
vtkGenericDataSet *vtkGenericDataSetToUnstructuredGridFilter::GetInput()
{
  if (this->NumberOfInputs < 1)
    {
    return NULL;
    }
  
  return static_cast<vtkGenericDataSet *>(this->Inputs[0]);
}

//----------------------------------------------------------------------------
void vtkGenericDataSetToUnstructuredGridFilter::PrintSelf(ostream& os,
                                                          vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

#if VTK_MAJOR_VERSION>4 || (VTK_MAJOR_VERSION==4 && VTK_MINOR_VERSION>4)
//----------------------------------------------------------------------------
int vtkGenericDataSetToUnstructuredGridFilter
::FillInputPortInformation(int port, vtkInformation* info)
{
  if(!this->Superclass::FillInputPortInformation(port, info))
    {
    return 0;
    }
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkGenericDataSet");
  return 1;
}
#endif
