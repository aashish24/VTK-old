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
#include "vtkInformationVector.h"

#include "vtkGarbageCollector.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"

#include <vtkstd/vector>

vtkCxxRevisionMacro(vtkInformationVector, "$Revision$");
vtkStandardNewMacro(vtkInformationVector);

class vtkInformationVectorInternals
{
public:
  vtkstd::vector<vtkInformation*> Vector;

  ~vtkInformationVectorInternals();
};

//----------------------------------------------------------------------------
vtkInformationVectorInternals::~vtkInformationVectorInternals()
{
  // Delete all the information objects.
  for(vtkstd::vector<vtkInformation*>::iterator i = this->Vector.begin();
      i != this->Vector.end(); ++i)
    {
    if(vtkInformation* info = *i)
      {
      info->Delete();
      }
    }
}

//----------------------------------------------------------------------------
vtkInformationVector::vtkInformationVector()
{
  this->Internal = new vtkInformationVectorInternals;
}

//----------------------------------------------------------------------------
vtkInformationVector::~vtkInformationVector()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
void vtkInformationVector::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
int vtkInformationVector::GetNumberOfInformationObjects()
{
  return static_cast<int>(this->Internal->Vector.size());
}

//----------------------------------------------------------------------------
void vtkInformationVector::SetNumberOfInformationObjects(int newNumber)
{
  // Adjust the number of objects.
  int oldNumber = static_cast<int>(this->Internal->Vector.size());
  if(newNumber > oldNumber)
    {
    // Create new information objects.
    this->Internal->Vector.resize(newNumber, 0);
    for(int i=oldNumber; i < newNumber; ++i)
      {
      this->Internal->Vector[i] = vtkInformation::New();
      }
    }
  else if(newNumber < oldNumber)
    {
    // Delete old information objects.
    for(int i=newNumber; i < oldNumber; ++i)
      {
      if(vtkInformation* info = this->Internal->Vector[i])
        {
        // Set the pointer to NULL first to avoid reporting of the
        // entry if deleting the information object causes a garbage
        // collection reference walk.
        this->Internal->Vector[i] = 0;
        info->Delete();
        }
      }
    this->Internal->Vector.resize(newNumber);
    }
}

//----------------------------------------------------------------------------
void vtkInformationVector::SetInformationObject(int index,
                                                vtkInformation* newInfo)
{
  // We do not allow NULL information objects.  If one is not given,
  // create an empty one.
  vtkInformation* info = 0;
  if(!newInfo)
    {
    newInfo = vtkInformation::New();
    info = newInfo;
    }

  if(index >= 0 && index < this->GetNumberOfInformationObjects())
    {
    // Replace an existing information object.
    vtkInformation* oldInfo = this->Internal->Vector[index];
    if(oldInfo != newInfo)
      {
      newInfo->Register(this);
      this->Internal->Vector[index] = newInfo;
      oldInfo->UnRegister(this);
      }
    }
  else if(index >= 0)
    {
    // If a hole will be created fill it with empty objects.
    if(index > this->GetNumberOfInformationObjects())
      {
      this->SetNumberOfInformationObjects(index);
      }

    // Store the information object in a new entry.
    newInfo->Register(this);
    this->Internal->Vector.push_back(newInfo);
    }

  // Delete the artifically created empty information object.
  if(info)
    {
    info->Delete();
    }
}

//----------------------------------------------------------------------------
vtkInformation* vtkInformationVector::GetInformationObject(int index)
{
  if(index >= 0 && index < this->GetNumberOfInformationObjects())
    {
    return this->Internal->Vector[index];
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkInformationVector::Append(vtkInformation* info)
{
  // Setting an entry beyond the end will automatically append.
  this->SetInformationObject(this->GetNumberOfInformationObjects(), info);
}

//----------------------------------------------------------------------------
void vtkInformationVector::Remove(vtkInformation* info)
{
  // Search for the information object and remove it.
  for(unsigned int i=0; i < this->Internal->Vector.size(); ++i)
    {
    if(this->Internal->Vector[i] == info)
      {
      this->Internal->Vector.erase(this->Internal->Vector.begin()+i);
      info->UnRegister(this);
      }
    }
}

//----------------------------------------------------------------------------
void vtkInformationVector::Register(vtkObjectBase* o)
{
  this->RegisterInternal(o, 1);
}

//----------------------------------------------------------------------------
void vtkInformationVector::UnRegister(vtkObjectBase* o)
{
  this->UnRegisterInternal(o, 1);
}

//----------------------------------------------------------------------------
void vtkInformationVector::ReportReferences(vtkGarbageCollector* collector)
{
  this->Superclass::ReportReferences(collector);
  for(unsigned int i=0; i < this->Internal->Vector.size(); ++i)
    {
    vtkGarbageCollectorReport(collector, this->Internal->Vector[i], "Entry");
    }
}
