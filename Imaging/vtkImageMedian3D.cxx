/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to C. Charles Law who developed this class.

Copyright (c) 1993-1995 Ken Martin, Will Schroeder, Bill Lorensen.

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
#include "vtkImageMedian3D.h"



//----------------------------------------------------------------------------
// Description:
// Construct an instance of vtkImageMedian3D fitler.
vtkImageMedian3D::vtkImageMedian3D()
{
  this->SetKernelSize(1,1,1);
  this->HandleBoundaries = 1;

  this->SetFilteredAxes(VTK_IMAGE_X_AXIS, VTK_IMAGE_Y_AXIS, VTK_IMAGE_Z_AXIS);
}


//----------------------------------------------------------------------------
void vtkImageMedian3D::SetFilteredAxes(int axis0, int axis1, int axis2)
{
  int axes[3];

  axes[0] = axis0;
  axes[1] = axis1;
  axes[2] = axis2;
  this->vtkImageSpatialFilter::SetFilteredAxes(3, axes);
}

//----------------------------------------------------------------------------
// Description:
// This method sets the size of the neighborhood.  It also sets the 
// default middle of the neighborhood 
void vtkImageMedian3D::SetKernelSize(int size0, int size1, int size2)
{  
  int volume;
  
  if (this->KernelSize[0] == size0 && this->KernelSize[1] == size1 && 
      this->KernelSize[2] == size2)
    {
    return;
    }
  
  // Set the kernel size and middle
  volume = 1;
  this->KernelSize[0] = size0;
  this->KernelMiddle[0] = size0 / 2;
  volume *= size0;
  this->KernelSize[1] = size1;
  this->KernelMiddle[1] = size1 / 2;
  volume *= size1;
  this->KernelSize[2] = size2;
  this->KernelMiddle[2] = size2 / 2;
  volume *= size2;

  this->KernelSize[3] = 1;
  this->KernelMiddle[3] = 0;

  this->SetNumberOfElements(volume);
}


//----------------------------------------------------------------------------
// Description:
// This method contains the second switch statement that calls the correct
// templated function for the mask types.
template <class T>
static void vtkImageMedian3DExecute(vtkImageMedian3D *self,
			   vtkImageRegion *inRegion, T *inPtr, 
			   vtkImageRegion *outRegion, T *outPtr)
{
  int *kernelMiddle, *kernelSize;
  // For looping though output (and input) pixels.
  int outMin0, outMax0, outMin1, outMax1, outMin2, outMax2;
  int outIdx0, outIdx1, outIdx2;
  int inInc0, inInc1, inInc2;
  int outInc0, outInc1, outInc2;
  T *inPtr0, *inPtr1, *inPtr2;
  T *outPtr0, *outPtr1, *outPtr2;
  // For looping through hood pixels
  int hoodMin0, hoodMax0, hoodMin1, hoodMax1, hoodMin2, hoodMax2;
  int hoodStartMin0, hoodStartMax0, hoodStartMin1, hoodStartMax1;
  int hoodIdx0, hoodIdx1, hoodIdx2;
  T *tmpPtr0, *tmpPtr1, *tmpPtr2;
  // The portion of the out image that needs no boundary processing.
  int middleMin0, middleMax0, middleMin1, middleMax1, middleMin2, middleMax2;
  
  // Get information to march through data
  inRegion->GetIncrements(inInc0, inInc1, inInc2); 
  outRegion->GetIncrements(outInc0, outInc1, outInc2); 
  outRegion->GetExtent(outMin0, outMax0, outMin1, outMax1, outMin2, outMax2);
  kernelMiddle = self->KernelMiddle;
  kernelSize = self->KernelSize;
  
  hoodMin0 = outMin0 - kernelMiddle[0]; 
  hoodMin1 = outMin1 - kernelMiddle[1]; 
  hoodMin2 = outMin2 - kernelMiddle[2]; 
  hoodMax0 = kernelSize[0] + hoodMin0 - 1;
  hoodMax1 = kernelSize[1] + hoodMin1 - 1;
  hoodMax2 = kernelSize[2] + hoodMin2 - 1;
  
  // Clip by the input image extent
  inRegion->GetWholeExtent(middleMin0, middleMax0, 
			   middleMin1, middleMax1, 
			   middleMin2, middleMax2);
  hoodMin0 = (hoodMin0 > middleMin0) ? hoodMin0 : middleMin0;
  hoodMin1 = (hoodMin1 > middleMin1) ? hoodMin1 : middleMin1;
  hoodMin2 = (hoodMin2 > middleMin2) ? hoodMin2 : middleMin2;
  hoodMax0 = (hoodMax0 < middleMax0) ? hoodMax0 : middleMax0;
  hoodMax1 = (hoodMax1 < middleMax1) ? hoodMax1 : middleMax1;
  hoodMax2 = (hoodMax2 < middleMax2) ? hoodMax2 : middleMax2;

  // Save the starting neighborhood dimensions (2 loops only once)
  hoodStartMin0 = hoodMin0;    hoodStartMax0 = hoodMax0;
  hoodStartMin1 = hoodMin1;    hoodStartMax1 = hoodMax1;
  
  // The portion of the output that needs no boundary computation.
  middleMin0 += kernelMiddle[0];
  middleMax0 -= (kernelSize[0] - 1) - kernelMiddle[0];
  middleMin1 += kernelMiddle[1];
  middleMax1 -= (kernelSize[1] - 1) - kernelMiddle[1];
  middleMin2 += kernelMiddle[2];
  middleMax2 -= (kernelSize[2] - 1) - kernelMiddle[2];
  
  // loop through pixel of output
  outPtr2 = outPtr;
  inPtr2 = inPtr;
  for (outIdx2 = outMin2; outIdx2 <= outMax2; ++outIdx2)
    {
    outPtr1 = outPtr2;
    inPtr1 = inPtr2;
    hoodMin1 = hoodStartMin1;
    hoodMax1 = hoodStartMax1;
    for (outIdx1 = outMin1; outIdx1 <= outMax1; ++outIdx1)
      {
      outPtr0 = outPtr1;
      inPtr0 = inPtr1;
      hoodMin0 = hoodStartMin0;
      hoodMax0 = hoodStartMax0;
      for (outIdx0 = outMin0; outIdx0 <= outMax0; ++outIdx0)
	{
	
	// Compute median of neighborhood
	// Note: For boundary, NumNeighborhood could be changed for
	// a faster sort.
	self->ClearMedian();
	// loop through neighborhood pixels
	tmpPtr2 = inPtr0;
	for (hoodIdx2 = hoodMin2; hoodIdx2 <= hoodMax2; ++hoodIdx2)
	  {
	  tmpPtr1 = tmpPtr2;
	  for (hoodIdx1 = hoodMin1; hoodIdx1 <= hoodMax1; ++hoodIdx1)
	    {
	    tmpPtr0 = tmpPtr1;
	    for (hoodIdx0 = hoodMin0; hoodIdx0 <= hoodMax0; ++hoodIdx0)
	      {
	      // Add this pixel to the median
	      self->AccumulateMedian(double(*tmpPtr0));
	      
	      tmpPtr0 += inInc0;
	      }
	    tmpPtr1 += inInc1;
	    }
	  tmpPtr2 += inInc2;
	  }
	
	// Replace this pixel with the hood median
        *outPtr0 = (T)(self->GetMedian());
	
	// shift neighborhood considering boundaries
	if (outIdx0 >= middleMin0)
	  {
	  inPtr0 += inInc0;
	  ++hoodMin0;
	  }
	if (outIdx0 < middleMax0)
	  {
	  ++hoodMax0;
	  }
	outPtr0 += outInc0;
      }
      // shift neighborhood considering boundaries
      if (outIdx1 >= middleMin1)
	{
	inPtr1 += inInc1;
	++hoodMin1;
	}
      if (outIdx1 < middleMax1)
	{
	++hoodMax1;
	}
      outPtr1 += outInc1;
    }
    // shift neighborhood considering boundaries
    if (outIdx2 >= middleMin2)
      {
      inPtr2 += inInc2;
      ++hoodMin2;
      }
    if (outIdx2 < middleMax2)
      {
      ++hoodMax2;
      }
    outPtr2 += outInc2;
    }
}

//----------------------------------------------------------------------------
// Description:
// This method contains the first switch statement that calls the correct
// templated function for the input and output region types.
void vtkImageMedian3D::Execute(vtkImageRegion *inRegion, 
			     vtkImageRegion *outRegion)
{
  void *inPtr = inRegion->GetScalarPointer();
  void *outPtr = outRegion->GetScalarPointer();
  
  vtkDebugMacro(<< "Execute: inRegion = " << inRegion 
		<< ", outRegion = " << outRegion);
  
  // this filter expects that input is the same type as output.
  if (inRegion->GetScalarType() != outRegion->GetScalarType())
    {
    vtkErrorMacro(<< "Execute: input ScalarType, " << inRegion->GetScalarType()
             << ", must match out ScalarType " << outRegion->GetScalarType());
    return;
    }
  
  switch (inRegion->GetScalarType())
    {
    case VTK_FLOAT:
      vtkImageMedian3DExecute(this, 
			  inRegion, (float *)(inPtr), 
			  outRegion, (float *)(outPtr));
      break;
    case VTK_INT:
      vtkImageMedian3DExecute(this, 
			  inRegion, (int *)(inPtr), 
			  outRegion, (int *)(outPtr));
      break;
    case VTK_SHORT:
      vtkImageMedian3DExecute(this, 
			  inRegion, (short *)(inPtr), 
			  outRegion, (short *)(outPtr));
      break;
    case VTK_UNSIGNED_SHORT:
      vtkImageMedian3DExecute(this, 
			  inRegion, (unsigned short *)(inPtr), 
			  outRegion, (unsigned short *)(outPtr));
      break;
    case VTK_UNSIGNED_CHAR:
      vtkImageMedian3DExecute(this, 
			  inRegion, (unsigned char *)(inPtr), 
			  outRegion, (unsigned char *)(outPtr));
      break;
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }
}



