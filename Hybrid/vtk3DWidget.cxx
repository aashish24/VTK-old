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
#include "vtk3DWidget.h"
#include "vtkCallbackCommand.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtk3DWidget, "$Revision$");

vtk3DWidget::vtk3DWidget()
{
  this->Placed = 1;
  this->Prop3D = NULL;
  this->Input = NULL;
}

vtk3DWidget::~vtk3DWidget()
{
  if ( this->Input )
    {
    this->Input->Delete();
    this->Input = NULL;
    }
  if ( this->Prop3D )
    {
    this->Prop3D->Delete();
    this->Prop3D = NULL;
    }
}

void vtk3DWidget::PlaceWidget()
{
  float bounds[6];

  if ( this->Prop3D )
    {
    this->Prop3D->GetBounds(bounds);
    }
  else if ( this->Input )
    {
    this->Input->Update();
    this->Input->GetBounds(bounds);
    }
  else
    {
    vtkErrorMacro(<<"No input or prop defined for widget placement");
    bounds[0] = -1.0;
    bounds[1] = 1.0;
    bounds[2] = -1.0;
    bounds[3] = 1.0;
    bounds[4] = -1.0;
    bounds[5] = 1.0;
    }
  
  this->PlaceWidget(bounds);
}

void vtk3DWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "Prop3D: " << this->Prop3D << "\n";
  os << indent << "Input: " << this->Input << "\n";
}


