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
#include "vtkStructuredGridToStructuredGridFilter.h"

#include "vtkStructuredGrid.h"

vtkCxxRevisionMacro(vtkStructuredGridToStructuredGridFilter, "$Revision$");

//----------------------------------------------------------------------------
vtkStructuredGridToStructuredGridFilter::vtkStructuredGridToStructuredGridFilter()
{
  this->NumberOfRequiredInputs = 1;
}

//----------------------------------------------------------------------------
vtkStructuredGridToStructuredGridFilter::~vtkStructuredGridToStructuredGridFilter()
{
}

//----------------------------------------------------------------------------
// Specify the input data or filter.
void vtkStructuredGridToStructuredGridFilter::SetInput(vtkStructuredGrid *input)
{
  this->vtkProcessObject::SetNthInput(0, input);
}

//----------------------------------------------------------------------------
// Specify the input data or filter.
vtkStructuredGrid *vtkStructuredGridToStructuredGridFilter::GetInput()
{
  if (this->NumberOfInputs < 1)
    {
    return NULL;
    }
  
  return (vtkStructuredGrid *)(this->Inputs[0]);
}


//----------------------------------------------------------------------------
void vtkStructuredGridToStructuredGridFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
