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
#include "vtkAbstractVolumeMapper.h"

#include "vtkDataSet.h"
#include "vtkExecutive.h"
#include "vtkInformation.h"
#include "vtkMath.h"

vtkCxxRevisionMacro(vtkAbstractVolumeMapper, "$Revision$");

// Construct a vtkAbstractVolumeMapper 
vtkAbstractVolumeMapper::vtkAbstractVolumeMapper()
{
  vtkMath::UninitializeBounds(this->Bounds);
  this->Center[0] = this->Center[1] = this->Center[2] = 0.0;
}

vtkAbstractVolumeMapper::~vtkAbstractVolumeMapper()
{
}

// Get the bounds for the input of this mapper as 
// (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax).
double *vtkAbstractVolumeMapper::GetBounds()
{
  static double bounds[] = {-1.0,1.0, -1.0,1.0, -1.0,1.0};

  if ( ! this->GetDataSetInput() ) 
    {
    return bounds;
    }
  else
    {
    this->Update();
    this->GetDataSetInput()->GetBounds(this->Bounds);
    return this->Bounds;
    }
}

vtkDataSet *vtkAbstractVolumeMapper::GetDataSetInput()
{
  if (this->GetNumberOfInputConnections(0) < 1)
    {
    return 0;
    }
  return vtkDataSet::SafeDownCast(this->GetExecutive()->GetInputData(0, 0));
}

void vtkAbstractVolumeMapper::SetInput( vtkDataSet *vtkNotUsed(input) )
{
  vtkErrorMacro("Cannot set the input on the abstract volume mapper"
                " - must be set on a subclass" );
}

//----------------------------------------------------------------------------
int vtkAbstractVolumeMapper::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

// Print the vtkAbstractVolumeMapper
void vtkAbstractVolumeMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

