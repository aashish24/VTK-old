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
#include <math.h>

#include "vtkImageSpatialFilter.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkImageSpatialFilter* vtkImageSpatialFilter::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageSpatialFilter");
  if(ret)
    {
    return (vtkImageSpatialFilter*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageSpatialFilter;
}





//----------------------------------------------------------------------------
// Construct an instance of vtkImageSpatialFilter fitler.
vtkImageSpatialFilter::vtkImageSpatialFilter()
{
  int idx;
  
  for (idx = 0; idx < 3; ++idx)
    {
    this->KernelSize[idx] = 1;
    this->KernelMiddle[idx] = 0;
    }
  
  this->HandleBoundaries = 1;
}


//----------------------------------------------------------------------------
void vtkImageSpatialFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  int idx;
  
  vtkImageToImageFilter::PrintSelf(os, indent);

  os << indent << "KernelSize: (" << this->KernelSize[0];
  for (idx = 1; idx < 3; ++idx)
    {
    os << ", " << this->KernelSize[idx];
    }
  os << ").\n";

  os << indent << "KernelMiddle: (" << this->KernelMiddle[0];
  for (idx = 1; idx < 3; ++idx)
    {
    os << ", " << this->KernelMiddle[idx];
    }
  os << ").\n";

}



//----------------------------------------------------------------------------
// This method is passed a region that holds the image extent of this filters
// input, and changes the region to hold the image extent of this filters
// output.
void vtkImageSpatialFilter::ExecuteInformation()
{
  vtkImageData *input = this->GetInput();
  vtkImageData *output = this->GetOutput();
  
  if (!input)
    {
    vtkErrorMacro(<< "Input not set.");
    return;
    }
  // Copy the defaults
  output->CopyTypeSpecificInformation( input );

  // Take this opportunity to override the defaults. 
  int extent[6];
  input->GetWholeExtent(extent);
  this->ComputeOutputWholeExtent(extent, this->HandleBoundaries);
  output->SetWholeExtent(extent);
  this->ExecuteInformation(input, output);
}
//----------------------------------------------------------------------------
void vtkImageSpatialFilter::ExecuteInformation(
           vtkImageData *vtkNotUsed(inData), vtkImageData *vtkNotUsed(outData))
{
}

//----------------------------------------------------------------------------
// A helper method to compute output image extent
void vtkImageSpatialFilter::ComputeOutputWholeExtent(int extent[6], 
						     int handleBoundaries)
{
  int idx;

  if ( ! handleBoundaries)
    {
    // Make extent a little smaller because of the kernel size.
    for (idx = 0; idx < 3; ++idx)
      {
      extent[idx*2] += this->KernelMiddle[idx];
      extent[idx*2+1] -= (this->KernelSize[idx]-1) - this->KernelMiddle[idx];
      }
    }
  
}



//----------------------------------------------------------------------------
// This method computes the extent of the input region necessary to generate
// an output region.  Before this method is called "region" should have the 
// extent of the output region.  After this method finishes, "region" should 
// have the extent of the required input region.
void vtkImageSpatialFilter::ComputeInputUpdateExtent(int extent[6], 
						     int inExtent[6])
{
  int idx;
  int *wholeExtent;
  
  if (!this->GetInput())
    {
    return;
    }

  wholeExtent = this->GetInput()->GetWholeExtent();
  for (idx = 0; idx < 3; ++idx)
    {
    // Magnify by strides
    extent[idx*2] = inExtent[idx*2];
    extent[idx*2+1] = inExtent[idx*2+1];

    // Expand to get inRegion Extent
    extent[idx*2] -= this->KernelMiddle[idx];
    extent[idx*2+1] += (this->KernelSize[idx]-1) - this->KernelMiddle[idx];
    
    // If the expanded region is out of the IMAGE Extent (grow min)
    if (extent[idx*2] < wholeExtent[idx*2])
      {
      if (this->HandleBoundaries)
	{
	// shrink the required region extent
	extent[idx*2] = wholeExtent[idx*2];
	}
      else
	{
	vtkWarningMacro(<< "Required region is out of the image extent.");
	}
      }
    // If the expanded region is out of the IMAGE Extent (shrink max)      
    if (extent[idx*2+1] > wholeExtent[idx*2+1])
      {
      if (this->HandleBoundaries)
	{
	// shrink the required region extent
	extent[idx*2+1] = wholeExtent[idx*2+1];
	}
      else
	{
	vtkWarningMacro(<< "Required region is out of the image extent.");
	}
      }
    }
}


