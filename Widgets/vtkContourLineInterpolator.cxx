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
#include "vtkContourLineInterpolator.h"

vtkCxxRevisionMacro(vtkContourLineInterpolator, "$Revision$");

//----------------------------------------------------------------------
vtkContourLineInterpolator::vtkContourLineInterpolator()
{
}

//----------------------------------------------------------------------
vtkContourLineInterpolator::~vtkContourLineInterpolator()
{
}

//----------------------------------------------------------------------
int vtkContourLineInterpolator::UpdateNode( vtkRenderer *ren, 
                                            vtkContourRepresentation *rep,
                                            double *node, int idx )
{
  return 0;
}

//----------------------------------------------------------------------
void vtkContourLineInterpolator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);  
}
