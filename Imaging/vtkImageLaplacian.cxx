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
#include <math.h>
#include "vtkImageRegion.h"
#include "vtkImageCache.h"
#include "vtkImageLaplacian.h"


//----------------------------------------------------------------------------
// Description:
// Construct an instance of vtkImageLaplacian fitler.
vtkImageLaplacian::vtkImageLaplacian()
{
  this->SetOutputScalarType(VTK_FLOAT);

  // The execute handles three axes.
  this->NumberOfExecutionAxes = 3;
}


//----------------------------------------------------------------------------
void vtkImageLaplacian::PrintSelf(ostream& os, vtkIndent indent)
{
  int idx;
  
  this->vtkImageFilter::PrintSelf(os, indent);
  os << indent << "FitleredAxes: " 
     << vtkImageAxisNameMacro(this->FilteredAxes[0]);
  for (idx = 1; idx < this->NumberOfFilteredAxes; ++idx)
    {
    os << ", " << vtkImageAxisNameMacro(this->FilteredAxes[idx]);
    }
  os << "\n";
}


//----------------------------------------------------------------------------
// Description:
// Determines how the input is interpreted (set of 2d slices ...)
// and cannot be more than 3.
void vtkImageLaplacian::SetFilteredAxes(int num, int *axes)
{
  if (num > 3)
    {
    vtkErrorMacro("SetFilteredAxes: This filter is only ");
    num = 3;
    }
  this->vtkImageFilter::SetFilteredAxes(num, axes);
  this->NumberOfExecutionAxes = 3;
}


//----------------------------------------------------------------------------
// Description:
// This method computes the input extent necessary to generate the output.
void vtkImageLaplacian::ComputeRequiredInputUpdateExtent()
{
  int extent[8];
  int *wholeExtent;
  int idx, axis;

  wholeExtent = this->Input->GetWholeExtent();
  this->Output->GetUpdateExtent(extent);
  
  // grow input image extent.
  for (idx = 0; idx < this->NumberOfFilteredAxes; ++idx)
    {
    axis = this->FilteredAxes[idx];
    extent[axis*2] -= 1;
    extent[axis*2+1] += 1;
    // we must clip extent with image extent if we hanlde boundaries.image
    if (extent[axis*2] < wholeExtent[axis*2])
      {
      extent[axis*2] = wholeExtent[axis*2];
      }
    if (extent[axis*2 + 1] > wholeExtent[axis*2 + 1])
      {
      extent[axis*2 + 1] = wholeExtent[axis*2 + 1];
      }
    }
  
  this->Input->SetUpdateExtent(extent);
}





//----------------------------------------------------------------------------
// Description:
// This execute method handles boundaries.
// it handles boundaries. Pixels are just replicated to get values 
// out of extent.
template <class T>
static void vtkImageLaplacianExecute(vtkImageLaplacian *self,
				     vtkImageRegion *inRegion, T *inPtr, 
				     vtkImageRegion *outRegion, float *outPtr)
{
  int axisIdx, axesNum;
  float d, sum;
  float r[3];
  // For looping though output (and input) pixels.
  int min0, max0, min1, max1, min2, max2;
  int outIdx0, outIdx1, outIdx2;
  int outInc0, outInc1, outInc2;
  float *outPtr0, *outPtr1, *outPtr2;
  int inInc0, inInc1, inInc2;
  T *inPtr0, *inPtr1, *inPtr2;
  // For computation of Laplacian (everything has to be arrays for loop).
  int *incs, *imageExtent, *idxs, outIdxs[3];
  
  // Get the dimensionality of the Laplacian.
  axesNum = self->GetNumberOfFilteredAxes();
  
  // Get information to march through data (skip component)
  inRegion->GetIncrements(inInc0, inInc1, inInc2); 
  outRegion->GetIncrements(outInc0, outInc1, outInc2); 
  outRegion->GetExtent(min0,max0, min1,max1, min2,max2);
    
  // We want the input pixel to correspond to output
  inPtr = (T *)(inRegion->GetScalarPointer(min0,min1,min2));

  // The data spcing is important for computing the Laplacian.
  // Divid by dx twice (second derivative).
  inRegion->GetSpacing(4, r);
  r[0] = 1.0 / r[0] * r[0];
  r[1] = 1.0 / r[1] * r[1];
  r[2] = 1.0 / r[2] * r[2];
  
  // loop through pixels of output
  outPtr2 = outPtr;
  inPtr2 = inPtr;
  for (outIdx2 = min2; outIdx2 <= max2; ++outIdx2)
    {
    outIdxs[2] = outIdx2;
    outPtr1 = outPtr2;
    inPtr1 = inPtr2;
    for (outIdx1 = min1; outIdx1 <= max1; ++outIdx1)
      {
      outIdxs[1] = outIdx1;
      outPtr0 = outPtr1;
      inPtr0 = inPtr1;
      for (outIdx0 = min0; outIdx0 <= max0; ++outIdx0)
	{
	*outIdxs = outIdx0;
	
	// compute Laplacian.
	sum = 0.0;
	idxs = outIdxs;
	incs = inRegion->GetIncrements(); 
	imageExtent = inRegion->GetWholeExtent(); 
	for(axisIdx = 0; axisIdx < axesNum; ++axisIdx)
	  {
	  // Compute difference using central differences (if in extent).
	  d = -2.0 * *inPtr0;
	  d += (*idxs == *imageExtent++) ? *inPtr0 : inPtr0[-*incs];
	  d += (*idxs == *imageExtent++) ? *inPtr0 : inPtr0[*incs];
	  sum += d * r[axisIdx]; // divide by spacing squared
	  ++idxs;
	  ++incs;
	  }
	*outPtr0 = sum;

	outPtr0 += outInc0;
	inPtr0 += inInc0;
	}
      outPtr1 += outInc1;
      inPtr1 += inInc1;
      }
    outPtr2 += outInc2;
    inPtr2 += inInc2;
    }
}


//----------------------------------------------------------------------------
// Description:
// This method contains a switch statement that calls the correct
// templated function for the input region type.  The output region
// must be of type float.  This method does handle boundary conditions.
void vtkImageLaplacian::Execute(vtkImageRegion *inRegion, 
				vtkImageRegion *outRegion)
{
  void *inPtr = inRegion->GetScalarPointer();
  void *outPtr = outRegion->GetScalarPointer();
  
  // this filter expects that output is type float.
  if (outRegion->GetScalarType() != VTK_FLOAT)
    {
    vtkErrorMacro(<< "Execute: output ScalarType, "
        << vtkImageScalarTypeNameMacro(outRegion->GetScalarType())
        << ", must be float");
    return;
    }
  
  switch (inRegion->GetScalarType())
    {
    case VTK_FLOAT:
      vtkImageLaplacianExecute(this, inRegion, (float *)(inPtr), 
			       outRegion, (float *)(outPtr));
      break;
    case VTK_INT:
      vtkImageLaplacianExecute(this, inRegion, (int *)(inPtr), 
			       outRegion, (float *)(outPtr));
      break;
    case VTK_SHORT:
      vtkImageLaplacianExecute(this, inRegion, (short *)(inPtr), 
			       outRegion, (float *)(outPtr));
      break;
    case VTK_UNSIGNED_SHORT:
      vtkImageLaplacianExecute(this, inRegion, (unsigned short *)(inPtr), 
			       outRegion, (float *)(outPtr));
      break;
    case VTK_UNSIGNED_CHAR:
      vtkImageLaplacianExecute(this, inRegion, (unsigned char *)(inPtr), 
			       outRegion, (float *)(outPtr));
      break;
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }
}






