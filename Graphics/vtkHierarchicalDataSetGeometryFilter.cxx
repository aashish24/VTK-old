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
#include "vtkHierarchicalDataSetGeometryFilter.h"

#include "vtkAppendPolyData.h"
#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataPipeline.h"
#include "vtkDataSet.h"
#include "vtkGeometryFilter.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkHierarchicalDataSet.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkHierarchicalDataSetGeometryFilter, "$Revision$");
vtkStandardNewMacro(vtkHierarchicalDataSetGeometryFilter);

vtkHierarchicalDataSetGeometryFilter::vtkHierarchicalDataSetGeometryFilter()
{
}

vtkHierarchicalDataSetGeometryFilter::~vtkHierarchicalDataSetGeometryFilter()
{
}

int vtkHierarchicalDataSetGeometryFilter::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  // now add our info
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
  info->Set(vtkCompositeDataPipeline::INPUT_REQUIRED_COMPOSITE_DATA_TYPE(), 
            "vtkHierarchicalDataSet");
  return 1;
}

int vtkHierarchicalDataSetGeometryFilter::ProcessRequest(
  vtkInformation* request, 
  vtkInformationVector** inputVector, 
  vtkInformationVector* outputVector)
{
  // generate the data
  if(request->Has(vtkCompositeDataPipeline::REQUEST_DATA()))
    {
    int retVal = this->RequestCompositeData(request, inputVector, outputVector);
    return retVal;
    }

 return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

int vtkHierarchicalDataSetGeometryFilter::RequestCompositeData(
  vtkInformation*, 
  vtkInformationVector** inputVector, 
  vtkInformationVector*  outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkHierarchicalDataSet *input = vtkHierarchicalDataSet::SafeDownCast(
    inInfo->Get(vtkCompositeDataSet::COMPOSITE_DATA_SET()));
  if (!input) {return 0;}

  vtkInformation* info = outputVector->GetInformationObject(0);
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    info->Get(vtkDataObject::DATA_OBJECT()));
  if (!output) {return 0;}

  vtkCompositeDataIterator* iter = input->NewIterator();
  iter->GoToFirstItem();
  vtkAppendPolyData* append = vtkAppendPolyData::New();
  while (!iter->IsDoneWithTraversal())
    {
    vtkDataSet* ds = vtkDataSet::SafeDownCast(iter->GetCurrentDataObject());
    if (ds)
      {
      vtkGeometryFilter* geom = vtkGeometryFilter::New();
      geom->SetInput(ds);
      geom->Update();
      append->AddInput(geom->GetOutput());
      geom->Delete();
      }
    iter->GoToNextItem();
    }
  iter->Delete();
  append->Update();

  output->ShallowCopy(append->GetOutput());

  append->Delete();

  return 1;
}

vtkExecutive* vtkHierarchicalDataSetGeometryFilter::CreateDefaultExecutive()
{
  return vtkCompositeDataPipeline::New();
}

void vtkHierarchicalDataSetGeometryFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

