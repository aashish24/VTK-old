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
#include "vtkMultiBlockDataSet.h"

#include "vtkMultiBlockDataIterator.h"
#include "vtkMultiBlockDataSetInternal.h"
#include "vtkMultiBlockDataVisitor.h"

#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkMultiBlockDataSet, "$Revision$");
vtkStandardNewMacro(vtkMultiBlockDataSet);

//----------------------------------------------------------------------------
vtkMultiBlockDataSet::vtkMultiBlockDataSet()
{
  this->Internal = new vtkMultiBlockDataSetInternal;
}

//----------------------------------------------------------------------------
vtkMultiBlockDataSet::~vtkMultiBlockDataSet()
{
  delete this->Internal;
}

//----------------------------------------------------------------------------
unsigned int vtkMultiBlockDataSet::GetNumberOfDataSets()
{
  return this->Internal->DataSets.size();
}

//----------------------------------------------------------------------------
vtkDataObject* vtkMultiBlockDataSet::GetDataSet(unsigned int idx)
{
  if ( idx >= this->Internal->DataSets.size() )
    {
    return 0;
    }
  return this->Internal->DataSets[idx];
}

//----------------------------------------------------------------------------
unsigned int vtkMultiBlockDataSet::AddDataSet(vtkDataObject* data)
{
  if (data)
    {
    this->Internal->DataSets.push_back(data);
    this->Modified();
    }
  return this->Internal->DataSets.size() - 1;
}

//----------------------------------------------------------------------------
vtkCompositeDataIterator* vtkMultiBlockDataSet::NewIterator()
{
  vtkMultiBlockDataIterator* iter = vtkMultiBlockDataIterator::New();
  iter->SetDataSet(this);
  return iter;
}

//----------------------------------------------------------------------------
vtkCompositeDataVisitor* vtkMultiBlockDataSet::NewVisitor()
{
  vtkMultiBlockDataVisitor* vis = vtkMultiBlockDataVisitor::New();
  vtkMultiBlockDataIterator* it = 
    vtkMultiBlockDataIterator::SafeDownCast(this->NewIterator());
  vis->SetDataIterator(it);
  it->Delete();
  return vis;
}

//----------------------------------------------------------------------------
void vtkMultiBlockDataSet::Initialize()
{
  this->Superclass::Initialize();
  this->Internal->DataSets.clear();
}

//----------------------------------------------------------------------------
void vtkMultiBlockDataSet::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

