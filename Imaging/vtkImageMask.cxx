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
#include "vtkImageMask.h"



//----------------------------------------------------------------------------
vtkImageMask::vtkImageMask()
{
  this->MaskedOutputValue = 0.0;
  this->NotMask = 0;
}

//----------------------------------------------------------------------------
// Description:
// This templated function executes the filter for any type of data.
template <class T>
static void vtkImageMaskExecute(vtkImageMask *self, int ext[6],
			 vtkImageData *in1Data, T *in1Ptr,
			 vtkImageData *in2Data, unsigned char *in2Ptr,
			 vtkImageData *outData, T *outPtr)
{
  int num0, num1, num2, numC, pixSize;
  int idx0, idx1, idx2;
  int in1Inc0, in1Inc1, in1Inc2;
  int in2Inc0, in2Inc1, in2Inc2;
  int outInc0, outInc1, outInc2;
  T *maskedValue;
  int maskState;
  
  numC = outData->GetNumberOfScalarComponents();
  maskedValue = new T[numC];
  for (idx0 = 0; idx0 < numC; ++idx0)
    {
    maskedValue[idx0] = (T)(self->GetMaskedOutputValue());
    }
  pixSize = numC * sizeof(T);
  maskState = self->GetNotMask();
  
  // Get information to march through data 
  in1Data->GetContinuousIncrements(ext, in1Inc0, in1Inc1, in1Inc2);
  in2Data->GetContinuousIncrements(ext, in2Inc0, in2Inc1, in2Inc2);
  outData->GetContinuousIncrements(ext, outInc0, outInc1, outInc2);
  num0 = ext[1] - ext[0] + 1;
  num1 = ext[3] - ext[2] + 1;
  num2 = ext[5] - ext[4] + 1;
  
  // Loop through ouput pixels
  for (idx2 = 0; idx2 < num2; ++idx2)
    {
    for (idx1 = 0; idx1 < num1; ++idx1)
      {
      for (idx0 = 0; idx0 < num0; ++idx0)
	{
      
	// Pixel operation
	if (*in2Ptr && maskState == 1)
	  {
	  memcpy(outPtr, maskedValue, pixSize);
	  }
	else if ( ! *in2Ptr && maskState == 0)
	  {
	  memcpy(outPtr, maskedValue, pixSize);
	  }
	else
	  {
	  memcpy(outPtr, in1Ptr, pixSize);
	  }
	
	in1Ptr += numC;
	outPtr += numC;
	in2Ptr += 1;
	}
      in1Ptr += in1Inc1;
      in2Ptr += in2Inc1;
      outPtr += outInc1;
      }
    in1Ptr += in1Inc2;
    in2Ptr += in2Inc2;
    outPtr += outInc2;
    }
  
  delete maskedValue;
}



//----------------------------------------------------------------------------
// Description:
// This method is passed a input and output Datas, and executes the filter
// algorithm to fill the output from the inputs.
// It just executes a switch statement to call the correct function for
// the Datas data types.
void vtkImageMask::ThreadedExecute(vtkImageData **inData, 
				   vtkImageData *outData,
				   int outExt[6], int id)
{
  void *inPtr1 = inData[0]->GetScalarPointerForExtent(outExt);
  void *inPtr2 = inData[1]->GetScalarPointerForExtent(outExt);
  void *outPtr = outData->GetScalarPointerForExtent(outExt);
  int *tExt;
  id = id;
  
  tExt = inData[1]->GetExtent();
  if (tExt[0] > outExt[0] || tExt[1] < outExt[1] || 
      tExt[2] > outExt[2] || tExt[3] < outExt[3] ||
      tExt[4] > outExt[4] || tExt[5] < outExt[5])
    {
    vtkErrorMacro("Mask extent not large enough");
    return;
    }
  
  if (inData[1]->GetNumberOfScalarComponents() != 1)
    {
    vtkErrorMacro("Maks can have one comenent");
    }
    
  if (inData[0]->GetScalarType() != outData->GetScalarType() ||
      inData[1]->GetScalarType() != VTK_UNSIGNED_CHAR)
    {
    vtkErrorMacro(<< "Execute: image ScalarType (" 
      << inData[0]->GetScalarType() << ") must match out ScalarType (" 
      << outData->GetScalarType() << "), and mask scalar type (" 
      << inData[1]->GetScalarType() << ") must be unsigned char.");
    return;
    }
  
  switch (inData[0]->GetScalarType())
    {
    case VTK_FLOAT:
      vtkImageMaskExecute(this, outExt,
			  inData[0], (float *)(inPtr1), 
			  inData[1], (unsigned char *)(inPtr2), 
			  outData, (float *)(outPtr));
      break;
    case VTK_INT:
      vtkImageMaskExecute(this,  outExt,
			  inData[0], (int *)(inPtr1), 
			  inData[1], (unsigned char *)(inPtr2), 
			  outData, (int *)(outPtr));
      break;
    case VTK_SHORT:
      vtkImageMaskExecute(this,  outExt,
			  inData[0], (short *)(inPtr1), 
			  inData[1], (unsigned char *)(inPtr2), 
			  outData, (short *)(outPtr));
      break;
    case VTK_UNSIGNED_SHORT:
      vtkImageMaskExecute(this,  outExt,
			  inData[0], (unsigned short *)(inPtr1), 
			  inData[1], (unsigned char *)(inPtr2), 
			  outData, (unsigned short *)(outPtr));
      break;
    case VTK_UNSIGNED_CHAR:
      vtkImageMaskExecute(this,  outExt,
			  inData[0], (unsigned char *)(inPtr1), 
			  inData[1], (unsigned char *)(inPtr2), 
			  outData, (unsigned char *)(outPtr));
      break;
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }
}


void vtkImageMask::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkImageTwoInputFilter::PrintSelf(os,indent);

}

