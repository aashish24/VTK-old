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
#include "vtkInformationExecutiveKey.h"

#include "vtkExecutive.h"

vtkCxxRevisionMacro(vtkInformationExecutiveKey, "$Revision$");

//----------------------------------------------------------------------------
vtkInformationExecutiveKey::vtkInformationExecutiveKey(const char* name, const char* location):
  vtkInformationKey(name, location)
{
  vtkFilteringInformationKeyManager::Register(this);
}

//----------------------------------------------------------------------------
vtkInformationExecutiveKey::~vtkInformationExecutiveKey()
{
}

//----------------------------------------------------------------------------
void vtkInformationExecutiveKey::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkInformationExecutiveKey::Set(vtkInformation* info,
                                     vtkExecutive* value)
{
  this->SetAsObjectBase(info, value);
}

//----------------------------------------------------------------------------
vtkExecutive* vtkInformationExecutiveKey::Get(vtkInformation* info)
{
  return vtkExecutive::SafeDownCast(this->GetAsObjectBase(info));
}

//----------------------------------------------------------------------------
int vtkInformationExecutiveKey::Has(vtkInformation* info)
{
  return vtkExecutive::SafeDownCast(this->GetAsObjectBase(info))?1:0;
}

//----------------------------------------------------------------------------
void vtkInformationExecutiveKey::Copy(vtkInformation* from,
                                      vtkInformation* to)
{
  this->Set(to, this->Get(from));
}

//----------------------------------------------------------------------------
void vtkInformationExecutiveKey::Report(vtkInformation* info,
                                        vtkGarbageCollector* collector)
{
  this->ReportAsObjectBase(info, collector);
}
