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
#include "vtkImageData.h"
#include "vtkImageIdealLowPass.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkImageIdealLowPass* vtkImageIdealLowPass::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageIdealLowPass");
  if(ret)
    {
    return (vtkImageIdealLowPass*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageIdealLowPass;
}






//----------------------------------------------------------------------------
vtkImageIdealLowPass::vtkImageIdealLowPass()
{
  int idx;
  
  for (idx = 0; idx < 3; ++idx)
    {
    this->CutOff[idx] = VTK_LARGE_FLOAT;
    }
}


//----------------------------------------------------------------------------
void vtkImageIdealLowPass::SetXCutOff(float cutOff)
{
  if (cutOff == this->CutOff[0])
    {
    return;
    }
  this->CutOff[0] = cutOff;
  this->Modified();
}
//----------------------------------------------------------------------------
void vtkImageIdealLowPass::SetYCutOff(float cutOff)
{
  if (cutOff == this->CutOff[1])
    {
    return;
    }
  this->CutOff[1] = cutOff;
  this->Modified();
}
//----------------------------------------------------------------------------
void vtkImageIdealLowPass::SetZCutOff(float cutOff)
{
  if (cutOff == this->CutOff[2])
    {
    return;
    }
  this->CutOff[2] = cutOff;
  this->Modified();
}


//----------------------------------------------------------------------------
void vtkImageIdealLowPass::ThreadedExecute(vtkImageData *inData, 
					   vtkImageData *outData,
					   int ext[6], int id)
{
  int idx0, idx1, idx2;
  int min0, max0;
  float *inPtr;
  float *outPtr;
  int *wholeExtent;
  float *spacing;
  int inInc0, inInc1, inInc2;
  int outInc0, outInc1, outInc2;
  float temp0, temp1, temp2, mid0, mid1, mid2;
  // normalization factors
  float norm0, norm1, norm2;
  float sum1, sum0;
  unsigned long count = 0;
  unsigned long target;

  // Error checking
  if (inData->GetNumberOfScalarComponents() != 2)
    {
    vtkErrorMacro("Expecting 2 components not " 
		  << inData->GetNumberOfScalarComponents());
    return;
    }
  if (inData->GetScalarType() != VTK_FLOAT || 
      outData->GetScalarType() != VTK_FLOAT)
    {
    vtkErrorMacro("Expecting input and output to be of type float");
    return;
    }
  
  wholeExtent = this->GetInput()->GetWholeExtent();
  spacing = inData->GetSpacing();

  inPtr = (float *)(inData->GetScalarPointerForExtent(ext));
  outPtr = (float *)(outData->GetScalarPointerForExtent(ext));

  inData->GetContinuousIncrements(ext, inInc0, inInc1, inInc2);
  outData->GetContinuousIncrements(ext, outInc0, outInc1, outInc2);  
  
  min0 = ext[0];
  max0 = ext[1];
  mid0 = (float)(wholeExtent[0] + wholeExtent[1] + 1) / 2.0;
  mid1 = (float)(wholeExtent[2] + wholeExtent[3] + 1) / 2.0;
  mid2 = (float)(wholeExtent[4] + wholeExtent[5] + 1) / 2.0;
  if ( this->CutOff[0] == 0.0)
    {
    norm0 = VTK_LARGE_FLOAT;
    }
  else
    {
    norm0 = 1.0 / ((spacing[0] * 2.0 * mid0) * this->CutOff[0]);
    }
  if ( this->CutOff[1] == 0.0)
    {
    norm1 = VTK_LARGE_FLOAT;
    }
  else
    {
    norm1 = 1.0 / ((spacing[1] * 2.0 * mid1) * this->CutOff[1]);
    }
  if ( this->CutOff[2] == 0.0)
    {
    norm2 = VTK_LARGE_FLOAT;
    }
  else
    {
    norm2 = 1.0 / ((spacing[2] * 2.0 * mid2) * this->CutOff[2]);
    }
  
  target = (unsigned long)((ext[5]-ext[4]+1)*(ext[3]-ext[2]+1)/50.0);
  target++;

  // loop over all the pixels (keeping track of normalized distance to origin.
  for (idx2 = ext[4]; idx2 <= ext[5]; ++idx2)
    {
    // distance to min (this axis' contribution)
    temp2 = (float)idx2;
    // Wrap back to 0.
    if (temp2 > mid2)
      {
      temp2 = mid2 + mid2 - temp2;
      }
    // Convert location into normalized cycles/world unit
    temp2 = temp2 * norm2;

    for (idx1 = ext[2]; !this->AbortExecute && idx1 <= ext[3]; ++idx1)
      {
      if (!id) 
	{
	if (!(count%target))
	  {
	  this->UpdateProgress(count/(50.0*target));
	  }
	count++;
	}
      // distance to min (this axis' contribution)
      temp1 = (float)idx1;
      // Wrap back to 0.
      if (temp1 > mid1)
	{
	temp1 = mid1 + mid1 - temp1;
	}
      // Convert location into cycles / world unit
      temp1 = temp1 * norm1;
      sum1 = temp2 * temp2 + temp1 * temp1;
      
      for (idx0 = min0; idx0 <= max0; ++idx0)
	{
	// distance to min (this axis' contribution)
	temp0 = (float)idx0;
	// Wrap back to 0.
	if (temp0 > mid0)
	  {
	  temp0 = mid0 + mid0 - temp0;
	  }
	// Convert location into cycles / world unit
	temp0 = temp0 * norm0;
	sum0 = sum1 + temp0 * temp0;
	
	if (sum0 > 1.0)
	  {
	  // real component
	  *outPtr++ = 0.0;
	  ++inPtr;
	  // imaginary component	
	  *outPtr++ = 0.0;
	  ++inPtr;
	  }
	else
	  {
	  // real component
	  *outPtr++ = *inPtr++;
	  // imaginary component	
	  *outPtr++ = *inPtr++;
	  }
	}
      inPtr += inInc1;
      outPtr += outInc1;
      }
    inPtr += inInc2;
    outPtr += outInc2;    
    }
}

void vtkImageIdealLowPass::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkImageToImageFilter::PrintSelf(os,indent);

  os << indent << "CutOff: ( "
     << this->CutOff[0] << ", "
     << this->CutOff[1] << ", "
     << this->CutOff[2] << " )\n";
}

