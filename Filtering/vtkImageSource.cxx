/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to C. Charles Law who developed this class.

Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkImageSource.h"
#include "vtkObjectFactory.h"



//----------------------------------------------------------------------------
vtkImageSource* vtkImageSource::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageSource");
  if(ret)
    {
    return (vtkImageSource*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageSource;
}




//----------------------------------------------------------------------------
vtkImageSource::vtkImageSource()
{
  this->vtkSource::SetNthOutput(0,vtkImageData::New());
  // Releasing data for pipeline parallism.
  // Filters will know it is empty. 
  this->Outputs[0]->ReleaseData();
  this->Outputs[0]->Delete();
}

//----------------------------------------------------------------------------
// Specify the input data or filter.
void vtkImageSource::SetOutput(vtkImageData *output)
{
  this->vtkSource::SetNthOutput(0, output);
}

//----------------------------------------------------------------------------
// Specify the input data or filter.
vtkImageData *vtkImageSource::GetOutput()
{
  if (this->NumberOfOutputs < 1)
    {
    return NULL;
    }
  
  return (vtkImageData *)(this->Outputs[0]);
}


//----------------------------------------------------------------------------
void vtkImageSource::PropagateUpdateExtent(vtkDataObject *out)
{
  vtkImageData *output = (vtkImageData*)out;

#ifndef VTK_REMOVE_LEGACY_CODE
  // ----------------------------------------------
  // For legacy compatability
  this->LegacyHack = 1;
  this->InterceptCacheUpdate();
  if (this->LegacyHack)
    {
    vtkErrorMacro( << "Change your method InterceptCacheUpdate " 
                   << "to the name EnlargeOutputUpdateExtents.");
    return;
    }
#endif

  this->vtkSource::PropagateUpdateExtent(output);
}


//----------------------------------------------------------------------------
// Convert to Imaging API
void vtkImageSource::Execute()
{
  vtkImageData *output = this->GetOutput();

  // If we have multiple Outputs, they need to be allocate
  // in a subclass.  We cannot be sure all outputs are images.
  output->SetExtent(output->GetUpdateExtent());
  output->AllocateScalars();

  this->Execute(this->GetOutput());
}

//----------------------------------------------------------------------------
// This function can be defined in a subclass to generate the data
// for a region.
void vtkImageSource::Execute(vtkImageData *)
{
  vtkErrorMacro(<< "Execute(): Method not defined.");
}

