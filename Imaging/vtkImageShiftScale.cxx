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
#include "vtkImageShiftScale.h"



//----------------------------------------------------------------------------
// Description:
// Constructor sets default values
vtkImageShiftScale::vtkImageShiftScale()
{
  this->Shift = 0.0;
  this->Scale = 1.0;
  this->OutputScalarType = -1;
}



//----------------------------------------------------------------------------
void vtkImageShiftScale::ExecuteImageInformation()
{
  if (this->OutputScalarType != -1)
    {
    this->Output->SetScalarType(this->OutputScalarType);
    }
}




//----------------------------------------------------------------------------
// Description:
// This templated function executes the filter for any type of data.
template <class IT, class OT>
static void vtkImageShiftScaleExecute(vtkImageShiftScale *self,
				      vtkImageData *inData, IT *inPtr,
				      vtkImageData *outData, OT *outPtr,
				      int outExt[6], int id)
{
  float shift = self->GetShift();
  float scale = self->GetScale();
  int idxR, idxY, idxZ;
  int maxY, maxZ;
  int inIncX, inIncY, inIncZ;
  int outIncX, outIncY, outIncZ;
  int rowLength;
  unsigned long count = 0;
  unsigned long target;
  
  // find the region to loop over
  rowLength = (outExt[1] - outExt[0]+1)*inData->GetNumberOfScalarComponents();
  maxY = outExt[3] - outExt[2]; 
  maxZ = outExt[5] - outExt[4];
  target = (unsigned long)((maxZ+1)*(maxY+1)/50.0);
  target++;
  
  // Get increments to march through data 
  inData->GetContinuousIncrements(outExt, inIncX, inIncY, inIncZ);
  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);

  // Loop through ouput pixels
  for (idxZ = 0; idxZ <= maxZ; idxZ++)
    {
    for (idxY = 0; idxY <= maxY; idxY++)
      {
      if (!id) 
	{
	if (!(count%target)) self->UpdateProgress(count/(50.0*target));
	count++;
	}
      for (idxR = 0; idxR < rowLength; idxR++)
	{
	// Pixel operation
	*outPtr = (OT)(((float)(*inPtr) + shift) * scale);
	outPtr++;
	inPtr++;
	}
      outPtr += outIncY;
      inPtr += inIncY;
      }
    outPtr += outIncZ;
    inPtr += inIncZ;
    }
}



//----------------------------------------------------------------------------
template <class T>
static void vtkImageShiftScaleExecute1(vtkImageShiftScale *self,
				      vtkImageData *inData, T *inPtr,
				      vtkImageData *outData,
				      int outExt[6], int id)
{
  void *outPtr = outData->GetScalarPointerForExtent(outExt);
  
  switch (outData->GetScalarType())
    {
    case VTK_FLOAT:
      vtkImageShiftScaleExecute(self, inData, inPtr,
			       outData, (float *)(outPtr),outExt, id);
      break;
    case VTK_INT:
      vtkImageShiftScaleExecute(self, inData, inPtr, 
			       outData, (int *)(outPtr),outExt, id);
      break;
    case VTK_SHORT:
      vtkImageShiftScaleExecute(self, inData, inPtr, 
			       outData, (short *)(outPtr),outExt, id);
      break;
    case VTK_UNSIGNED_SHORT:
      vtkImageShiftScaleExecute(self, inData, inPtr, 
			       outData, (unsigned short *)(outPtr),outExt, id);
      break;
    case VTK_UNSIGNED_CHAR:
      vtkImageShiftScaleExecute(self, inData, inPtr, 
			       outData, (unsigned char *)(outPtr),outExt, id);
      break;
    default:
      vtkGenericWarningMacro("Execute: Unknown input ScalarType");
      return;
    }
}




//----------------------------------------------------------------------------
// Description:
// This method is passed a input and output data, and executes the filter
// algorithm to fill the output from the input.
// It just executes a switch statement to call the correct function for
// the datas data types.
void vtkImageShiftScale::ThreadedExecute(vtkImageData *inData, 
					 vtkImageData *outData,
					 int outExt[6], int id)
{
  void *inPtr = inData->GetScalarPointerForExtent(outExt);
  
  switch (inData->GetScalarType())
    {
    case VTK_FLOAT:
      vtkImageShiftScaleExecute1(this, 
			  inData, (float *)(inPtr), 
			  outData, outExt, id);
      break;
    case VTK_INT:
      vtkImageShiftScaleExecute1(this, 
			  inData, (int *)(inPtr), 
			  outData, outExt, id);
      break;
    case VTK_SHORT:
      vtkImageShiftScaleExecute1(this, 
			  inData, (short *)(inPtr), 
			  outData, outExt, id);
      break;
    case VTK_UNSIGNED_SHORT:
      vtkImageShiftScaleExecute1(this, 
			  inData, (unsigned short *)(inPtr), 
			  outData, outExt, id);
      break;
    case VTK_UNSIGNED_CHAR:
      vtkImageShiftScaleExecute1(this, 
			  inData, (unsigned char *)(inPtr), 
			  outData, outExt, id);
      break;
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }
}
















