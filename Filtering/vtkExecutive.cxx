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
#include "vtkExecutive.h"

#include "vtkAlgorithm.h"
#include "vtkDataObject.h"
#include "vtkInformation.h"
#include "vtkInformationExecutiveKey.h"
#include "vtkInformationIntegerKey.h"
#include "vtkGarbageCollector.h"

vtkCxxRevisionMacro(vtkExecutive, "$Revision$");

vtkInformationKeyMacro(vtkExecutive, EXECUTIVE, Executive);
vtkInformationKeyMacro(vtkExecutive, PORT_NUMBER, Integer);

//----------------------------------------------------------------------------
vtkExecutive::vtkExecutive()
{
  this->GarbageCollecting = 0;
}

//----------------------------------------------------------------------------
vtkExecutive::~vtkExecutive()
{
}

//----------------------------------------------------------------------------
void vtkExecutive::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkExecutive::UnRegister(vtkObjectBase* o)
{
  int check = (this->GetReferenceCount() > 1);
  this->Superclass::UnRegister(o);
  if(check && !this->GarbageCollecting)
    {
    vtkGarbageCollector::Check(this);
    }
}

//----------------------------------------------------------------------------
void vtkExecutive::GarbageCollectionStarting()
{
  this->GarbageCollecting = 1;
  this->Superclass::GarbageCollectionStarting();
}
