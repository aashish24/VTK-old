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
#include "vtkImageButterworthHighPass.h"



//----------------------------------------------------------------------------
vtkImageButterworthHighPass::vtkImageButterworthHighPass()
{
  int idx;
  
  this->SetOutputScalarType(VTK_FLOAT);
  for (idx = 0; idx < 4; ++idx)
    {
    this->CutOff[idx] = VTK_LARGE_FLOAT;
    }
  this->Order = 1;
  
  // One complex number at a time. (sssssslowwww)
  this->SetExecutionAxes(VTK_IMAGE_COMPONENT_AXIS);
}


//----------------------------------------------------------------------------
void vtkImageButterworthHighPass::SetXCutOff(float cutOff)
{
  if (cutOff == this->CutOff[0])
    {
    return;
    }
  this->CutOff[0] = cutOff;
  this->Modified();
}
//----------------------------------------------------------------------------
void vtkImageButterworthHighPass::SetYCutOff(float cutOff)
{
  if (cutOff == this->CutOff[1])
    {
    return;
    }
  this->CutOff[1] = cutOff;
  this->Modified();
}
//----------------------------------------------------------------------------
void vtkImageButterworthHighPass::SetZCutOff(float cutOff)
{
  if (cutOff == this->CutOff[2])
    {
    return;
    }
  this->CutOff[2] = cutOff;
  this->Modified();
}
//----------------------------------------------------------------------------
void vtkImageButterworthHighPass::SetTimeCutOff(float cutOff)
{
  if (cutOff == this->CutOff[3])
    {
    return;
    }
  this->CutOff[3] = cutOff;
  this->Modified();
}



//----------------------------------------------------------------------------
// Description:
// This function zeros a portion of the image.  Zero is assumed
// to be the origin. (1d easy but slow)
void vtkImageButterworthHighPass::Execute(vtkImageRegion *inRegion, 
					  vtkImageRegion *outRegion)
{
  int idx;
  float *inPtr = (float *)(inRegion->GetScalarPointer());
  float *outPtr = (float *)(outRegion->GetScalarPointer());
  int *extent, *wholeExtent;
  float *spacing;
  int inInc;
  int outInc;
  float temp, freq, mid;
  float sum;
  
  // Make sure we have real and imaginary components.
  extent = inRegion->GetExtent();
  if (extent[0] != 0 || extent[1] != 1)
    {
    vtkErrorMacro(<< "Execute: Components mismatch");
    return;
    }
  
  // this filter expects that input is the same type as output (float).
  if (inRegion->GetScalarType() != VTK_FLOAT ||
      outRegion->GetScalarType() != VTK_FLOAT)
    {
    vtkErrorMacro(<< "Execute: input " 
      << vtkImageScalarTypeNameMacro(inRegion->GetScalarType()) 
      << " and output " 
      << vtkImageScalarTypeNameMacro(outRegion->GetScalarType()) 
      << " must be floats");
    return;
    }

  wholeExtent = inRegion->GetWholeExtent();
  spacing = inRegion->GetSpacing();
  sum = 0.0;
  // Sum up distance squared for each axis (except for component)
  // This assumes the order of the regions axes is C,X,Y,Z,T.
  for (idx = 1; idx < VTK_IMAGE_DIMENSIONS; ++idx)
    {
    temp = (float)(extent[2*idx]);
    // Assumes image min is 0.
    mid = (float)(wholeExtent[2*idx + 1] + 1) / 2.0;
    // Wrap back to 0.
    if (temp > mid)
      {
      temp = mid + mid - temp;
      }
    // spacing == 0 implies there is no spatial meaning for this axis
    if (spacing[idx] > 0.0)
      {
      // Convert location into cycles / world unit
      freq = temp / (spacing[idx] * 2.0 * mid);
      // Scale to unit circle (Pass band does not include Component Axis)
      temp = this->CutOff[idx - 1];
      if (temp > 0)
	{
	temp = freq / temp;
	sum += temp * temp;
	}
      else
	{
	sum = VTK_LARGE_FLOAT;
	}
      }
    }
  
  // compute Butterworth1D function from sum = d^2
  if (sum == 0.0)
    {
    sum = VTK_LARGE_FLOAT;
    }
  else
    {
    sum = 1.0 / sum;
    }
  if (this->Order == 1)
    {
    sum = 1.0 / (1.0 + sum);
    }
  else
    {
    sum = 1.0 / (1.0 + pow(sum, this->Order));
    }
  
  inRegion->GetIncrements(inInc);
  outRegion->GetIncrements(outInc);
  
  // real component
  *outPtr = *inPtr * sum;
  // imaginary component
  outPtr[outInc] = inPtr[inInc] * sum;
}
















