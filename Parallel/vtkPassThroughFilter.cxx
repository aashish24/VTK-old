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
#include "vtkPassThroughFilter.h"

#include "vtkCellData.h"
#include "vtkDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"

vtkCxxRevisionMacro(vtkPassThroughFilter, "$Revision$");
vtkStandardNewMacro(vtkPassThroughFilter);

void vtkPassThroughFilter::Execute()
{
  vtkDataSet *input = static_cast<vtkDataSet*>(this->GetInput());
  vtkDataSet *output = static_cast<vtkDataSet*>(this->GetOutput());

  // This has to be here because it initialized all field datas.
  output->CopyStructure( input );
  
  // Pass all. (data object's field data is passed by the
  // superclass after this method)
  output->GetPointData()->PassData( input->GetPointData() );
  output->GetCellData()->PassData( input->GetCellData() );

}

void vtkPassThroughFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
