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
#include "vtkUnstructuredGridToUnstructuredGridFilter.h"

#include "vtkUnstructuredGrid.h"

vtkCxxRevisionMacro(vtkUnstructuredGridToUnstructuredGridFilter, "$Revision$");

//----------------------------------------------------------------------------
// Specify the input data or filter.
void vtkUnstructuredGridToUnstructuredGridFilter::SetInput(vtkUnstructuredGrid *input)
{
  this->vtkProcessObject::SetNthInput(0, input);
}

//----------------------------------------------------------------------------
// Specify the input data or filter.
vtkUnstructuredGrid *vtkUnstructuredGridToUnstructuredGridFilter::GetInput()
{
  if (this->NumberOfInputs < 1)
    {
    return NULL;
    }
  
  return (vtkUnstructuredGrid *)(this->Inputs[0]);
}

//----------------------------------------------------------------------------
void vtkUnstructuredGridToUnstructuredGridFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
