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
#include "vtkShader2Collection.h"
#include "vtkObjectFactory.h"
#include "vtkShader2.h"

vtkCxxRevisionMacro(vtkShader2Collection, "$Revision$");
vtkStandardNewMacro(vtkShader2Collection);

// ----------------------------------------------------------------------------
// Description: 
// Reentrant safe way to get an object in a collection. Just pass the
// same cookie back and forth. 
vtkShader2 *vtkShader2Collection::GetNextShader(
vtkCollectionSimpleIterator &cookie)
{
  return static_cast<vtkShader2 *>(this->GetNextItemAsObject(cookie));
}

// ----------------------------------------------------------------------------
vtkShader2Collection::vtkShader2Collection()
{
}

// ----------------------------------------------------------------------------
vtkShader2Collection::~vtkShader2Collection()
{
}
  
// ----------------------------------------------------------------------------
// hide the standard AddItem from the user and the compiler.
void vtkShader2Collection::AddItem(vtkObject *o)
{
  this->vtkCollection::AddItem(o);
}

// ----------------------------------------------------------------------------
void vtkShader2Collection::AddItem(vtkShader2 *a) 
{
  this->vtkCollection::AddItem(a);
}

// ----------------------------------------------------------------------------
vtkShader2 *vtkShader2Collection::GetNextShader() 
{ 
  return static_cast<vtkShader2 *>(this->GetNextItemAsObject());
}

// ----------------------------------------------------------------------------
vtkShader2 *vtkShader2Collection::GetLastShader() 
{ 
  if ( this->Bottom == NULL )
    {
    return NULL;
    }
  else
    {
    return static_cast<vtkShader2 *>(this->Bottom->Item);
    }
}

// ----------------------------------------------------------------------------
void vtkShader2Collection::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
