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
#include "vtkPiecewiseFunctionToPiecewiseFunctionFilter.h"

#include "vtkInformation.h"
#include "vtkPiecewiseFunction.h"

vtkCxxRevisionMacro(vtkPiecewiseFunctionToPiecewiseFunctionFilter, "$Revision$");

//----------------------------------------------------------------------------
vtkPiecewiseFunctionToPiecewiseFunctionFilter::vtkPiecewiseFunctionToPiecewiseFunctionFilter() 
{
  this->NumberOfRequiredInputs = 1;
  this->SetNumberOfInputPorts(1);
}
//----------------------------------------------------------------------------
// Specify the input data or filter.
void vtkPiecewiseFunctionToPiecewiseFunctionFilter::SetInput(vtkPiecewiseFunction *input)
{
  this->vtkProcessObject::SetNthInput(0, input);
}

//----------------------------------------------------------------------------
// Specify the input data or filter.
vtkPiecewiseFunction *vtkPiecewiseFunctionToPiecewiseFunctionFilter::GetInput()
{
  if (this->NumberOfInputs < 1)
    {
    return NULL;
    }
  
  return (vtkPiecewiseFunction *)(this->Inputs[0]);
}

//----------------------------------------------------------------------------
int
vtkPiecewiseFunctionToPiecewiseFunctionFilter
::FillInputPortInformation(int port, vtkInformation* info)
{
  if(!this->Superclass::FillInputPortInformation(port, info))
    {
    return 0;
    }
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPiecewiseFunction");
  return 1;
}

//----------------------------------------------------------------------------
void vtkPiecewiseFunctionToPiecewiseFunctionFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
