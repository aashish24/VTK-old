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
#include "vtkRectilinearGridToPolyDataFilter.h"

#include "vtkRectilinearGrid.h"

vtkCxxRevisionMacro(vtkRectilinearGridToPolyDataFilter, "$Revision$");

//----------------------------------------------------------------------------
vtkRectilinearGridToPolyDataFilter::vtkRectilinearGridToPolyDataFilter()
{
  this->NumberOfRequiredInputs = 1;
}

//----------------------------------------------------------------------------
vtkRectilinearGridToPolyDataFilter::~vtkRectilinearGridToPolyDataFilter()
{
}

//----------------------------------------------------------------------------
// Specify the input data or filter.
void vtkRectilinearGridToPolyDataFilter::SetInput(vtkRectilinearGrid *input)
{
  this->vtkProcessObject::SetNthInput(0, input);
}

//----------------------------------------------------------------------------
// Specify the input data or filter.
vtkRectilinearGrid *vtkRectilinearGridToPolyDataFilter::GetInput()
{
  if (this->NumberOfInputs < 1)
    {
    return NULL;
    }
  
  return (vtkRectilinearGrid *)(this->Inputs[0]);
}

//----------------------------------------------------------------------------
void vtkRectilinearGridToPolyDataFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
