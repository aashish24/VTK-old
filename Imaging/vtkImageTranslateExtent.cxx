/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to C. Charles Law who developed this class.

Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/

#include "vtkImageTranslateExtent.h"
#include "vtkObjectFactory.h"



//----------------------------------------------------------------------------
vtkImageTranslateExtent* vtkImageTranslateExtent::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageTranslateExtent");
  if(ret)
    {
    return (vtkImageTranslateExtent*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageTranslateExtent;
}





//----------------------------------------------------------------------------
vtkImageTranslateExtent::vtkImageTranslateExtent()
{
  int idx;

  for (idx = 0; idx < 3; ++idx)
    {
    this->Translation[idx]  = 0;
    }
}


//----------------------------------------------------------------------------
void vtkImageTranslateExtent::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkImageToImageFilter::PrintSelf(os,indent);

  os << indent << "Translation: (" << this->Translation[0]
     << "," << this->Translation[1] << "," << this->Translation[2] << endl;
}
  



//----------------------------------------------------------------------------
// Change the WholeExtent
void vtkImageTranslateExtent::ExecuteInformation(vtkImageData *inData, 
						 vtkImageData *outData)
{
  int idx, extent[6];
  float *spacing, origin[3];
  
  inData->GetWholeExtent(extent);
  inData->GetOrigin(origin);
  spacing = inData->GetSpacing();

  // TranslateExtent the OutputWholeExtent with the input WholeExtent
  for (idx = 0; idx < 3; ++idx)
    {
    // change extent
    extent[2*idx] += this->Translation[idx];
    extent[2*idx+1] += this->Translation[idx];
    // change origin so the data does not shift
    origin[idx] -= (float)(this->Translation[idx]) * spacing[idx];
    }
  
  outData->SetWholeExtent(extent);
  outData->SetOrigin(origin);
}


//----------------------------------------------------------------------------
// This method simply copies by reference the input data to the output.
void vtkImageTranslateExtent::UpdateData(vtkDataObject *data)
{
  vtkImageData *inData, *outData = (vtkImageData*)(data);
  int extent[6], idx;
  
  // Make sure the Input has been set.
  if ( ! this->GetInput())
    {
    vtkErrorMacro(<< "Input is not set.");
    return;
    }

  this->GetOutput()->GetUpdateExtent(extent);
  for (idx = 0; idx < 3; ++idx)
    {
    extent[idx*2] -= this->Translation[idx];
    extent[idx*2+1] -= this->Translation[idx];
    }
  
  this->GetInput()->SetUpdateExtent(extent);

  this->GetInput()->Update();
  inData = this->GetInput();
  // since inData can be larger than update extent.
  inData->GetExtent(extent);
  for (idx = 0; idx < 3; ++idx)
    {
    extent[idx*2] += this->Translation[idx];
    extent[idx*2+1] += this->Translation[idx];
    }
  outData->SetExtent(extent);
  outData->GetPointData()->PassData(inData->GetPointData());
  outData->DataHasBeenGenerated();
  
  // release input data
  if (this->GetInput()->ShouldIReleaseData())
    {
    this->GetInput()->ReleaseData();
    }
}


//----------------------------------------------------------------------------
void vtkImageTranslateExtent::ComputeInputUpdateExtent(int extent[6], 
						       int inExtent[6])
{
  extent[0] = inExtent[0] - this->Translation[0];
  extent[1] = inExtent[1] - this->Translation[0];
  extent[2] = inExtent[2] - this->Translation[1];
  extent[3] = inExtent[3] - this->Translation[1];
  extent[4] = inExtent[4] - this->Translation[2];
  extent[5] = inExtent[5] - this->Translation[2];
}
