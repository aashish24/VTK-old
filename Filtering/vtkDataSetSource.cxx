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
#include "vtkDataSetSource.h"

#include "vtkObjectFactory.h"
#include "vtkDataSet.h"

vtkCxxRevisionMacro(vtkDataSetSource, "$Revision$");

vtkDataSetSource::vtkDataSetSource()
{

}

//----------------------------------------------------------------------------
vtkDataSet *vtkDataSetSource::GetOutput()
{
  if (this->NumberOfOutputs < 1)
    {
    return NULL;
    }
  
  return (vtkDataSet *)(this->Outputs[0]);
}

//----------------------------------------------------------------------------
void vtkDataSetSource::SetOutput(vtkDataSet *output)
{
  this->vtkSource::SetNthOutput(0, output);
}

vtkDataSet *vtkDataSetSource::GetOutput(int idx)
{
  return static_cast<vtkDataSet *>( this->vtkSource::GetOutput(idx) ); 
}

//----------------------------------------------------------------------------
void vtkDataSetSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
