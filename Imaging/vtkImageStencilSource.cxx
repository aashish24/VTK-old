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
#include "vtkImageStencilSource.h"

#include "vtkImageStencilData.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"

#include <math.h>

vtkCxxRevisionMacro(vtkImageStencilSource, "$Revision$");
vtkStandardNewMacro(vtkImageStencilSource);

//----------------------------------------------------------------------------
vtkImageStencilSource::vtkImageStencilSource()
{
  // A source has no inputs by default.
  this->SetNumberOfInputPorts(0);

  this->vtkSource::SetNthOutput(0,vtkImageStencilData::New());
  // Releasing data for pipeline parallism.
  // Filters will know it is empty. 
  this->Outputs[0]->ReleaseData();
  this->Outputs[0]->Delete();
}

//----------------------------------------------------------------------------
vtkImageStencilSource::~vtkImageStencilSource()
{
}

//----------------------------------------------------------------------------
void vtkImageStencilSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkImageStencilSource::SetOutput(vtkImageStencilData *output)
{
  this->vtkSource::SetNthOutput(0, output);
}

//----------------------------------------------------------------------------
vtkImageStencilData *vtkImageStencilSource::GetOutput()
{
  if (this->NumberOfOutputs < 1)
    {
    return NULL;
    }
  
  return (vtkImageStencilData *)(this->Outputs[0]);
}

//----------------------------------------------------------------------------
vtkImageStencilData * 
vtkImageStencilSource::AllocateOutputData(vtkDataObject *out)
{
  vtkImageStencilData *res = vtkImageStencilData::SafeDownCast(out);
  if (!res)
    {
    vtkWarningMacro("Call to AllocateOutputData with non vtkImageStencilData"
                    " output");
    return NULL;
    }
  res->SetExtent(res->GetUpdateExtent());
  res->SetOldSpacing(res->GetSpacing());
  res->SetOldOrigin(res->GetOrigin());
  res->AllocateExtents();

  return res;
}  

//----------------------------------------------------------------------------
void vtkImageStencilSource::ExecuteData(vtkDataObject *out)
{
  vtkImageStencilData *output = this->AllocateOutputData(out);

  // no multithreading yet...
  this->ThreadedExecute(output, output->GetExtent(), 0);
}

//----------------------------------------------------------------------------
void vtkImageStencilSource::ThreadedExecute(vtkImageStencilData *vtkNotUsed(o),
                                            int extent[6], int threadId)
{
  extent = extent; // this silly line avoids compiler warnings
  if (threadId == 0)
    {
    vtkErrorMacro("subclass should override ThreadedExecute!!!");
    }
}

//----------------------------------------------------------------------------
int vtkImageStencilSource::FillOutputPortInformation(int port,
                                                     vtkInformation* info)
{
  if(!this->Superclass::FillOutputPortInformation(port, info))
    {
    return 0;
    }
  info->Set(vtkInformation::OUTPUT_DATA_TYPE(), "vtkImageStencilData");
  return 1;
}
