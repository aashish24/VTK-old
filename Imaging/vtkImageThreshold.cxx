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
#include "vtkImageThreshold.h"



//----------------------------------------------------------------------------
// Description:
// Constructor sets default values
vtkImageThreshold::vtkImageThreshold()
{
  this->UpperThreshold = VTK_LARGE_FLOAT;
  this->LowerThreshold = -VTK_LARGE_FLOAT;
  this->ReplaceIn = 0;
  this->InValue = 0.0;
  this->ReplaceOut = 0;
  this->OutValue = 0.0;

  this->OutputScalarType = -1; // invalid; output same as input
}


//----------------------------------------------------------------------------
void vtkImageThreshold::SetInValue(float val)
{
  if (val != this->InValue || this->ReplaceIn != 1)
    {
    this->InValue = val;
    this->ReplaceIn = 1;
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkImageThreshold::SetOutValue(float val)
{
  if (val != this->OutValue || this->ReplaceOut != 1)
    {
    this->OutValue = val;
    this->ReplaceOut = 1;
    this->Modified();
    }
}



//----------------------------------------------------------------------------
// Description:
// The values greater than or equal to the value match.
void vtkImageThreshold::ThresholdByUpper(float thresh)
{
  if (this->LowerThreshold != thresh || this->UpperThreshold < VTK_LARGE_FLOAT)
    {
    this->LowerThreshold = thresh;
    this->UpperThreshold = VTK_LARGE_FLOAT;
    this->Modified();
    }
}


//----------------------------------------------------------------------------
// Description:
// The values less than or equal to the value match.
void vtkImageThreshold::ThresholdByLower(float thresh)
{
  if (this->UpperThreshold != thresh || this->LowerThreshold > -VTK_LARGE_FLOAT)
    {
    this->UpperThreshold = thresh;
    this->LowerThreshold = -VTK_LARGE_FLOAT;
    this->Modified();
    }
}


//----------------------------------------------------------------------------
// Description:
// The values in a range (inclusive) match
void vtkImageThreshold::ThresholdBetween(float lower, float upper)
{
  if (this->LowerThreshold != lower || this->UpperThreshold != upper)
    {
    this->LowerThreshold = lower;
    this->UpperThreshold = upper;
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkImageThreshold::ExecuteImageInformation()
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
static void vtkImageThresholdExecute(vtkImageThreshold *self,
				     vtkImageData *inData, IT *inPtr,
				     vtkImageData *outData, OT *outPtr, 
				     int outExt[6], int id)
{
  int idxR, idxY, idxZ;
  int maxY, maxZ;
  int inIncX, inIncY, inIncZ;
  int outIncX, outIncY, outIncZ;
  int rowLength;
  float  lowerThreshold = self->GetLowerThreshold();
  float  upperThreshold = self->GetUpperThreshold();
  int replaceIn = self->GetReplaceIn();
  OT  inValue = (OT)(self->GetInValue());
  int replaceOut = self->GetReplaceOut();
  OT  outValue = (OT)(self->GetOutValue());
  float temp;
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
	if (!(count%target))
	  {
	  self->UpdateProgress(count/(50.0*target));
	  }
	count++;
	}
      for (idxR = 0; idxR < rowLength; idxR++)
	{
	// Pixel operation
	temp = (float)(*inPtr);
	if (lowerThreshold <= temp && temp <= upperThreshold)
	  {
	  // match
	  if (replaceIn)
	    {
	    *outPtr = inValue;
	    }
	  else
	    {
	    *outPtr = (OT)(temp);
	    }
	  }
	else
	  {
	  // not match
	  if (replaceOut)
	    {
	    *outPtr = outValue;
	    }
	  else
	    {
	    *outPtr = (OT)(temp);
	    }
	  }
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
static void vtkImageThresholdExecute1(vtkImageThreshold *self,
				      vtkImageData *inData, T *inPtr,
				      vtkImageData *outData,
				      int outExt[6], int id)
{
  void *outPtr = outData->GetScalarPointerForExtent(outExt);
  
  switch (outData->GetScalarType())
    {
    case VTK_FLOAT:
      vtkImageThresholdExecute(self, inData, inPtr,
			       outData, (float *)(outPtr),outExt, id);
      break;
    case VTK_INT:
      vtkImageThresholdExecute(self, inData, inPtr, 
			       outData, (int *)(outPtr),outExt, id);
      break;
    case VTK_SHORT:
      vtkImageThresholdExecute(self, inData, inPtr, 
			       outData, (short *)(outPtr),outExt, id);
      break;
    case VTK_UNSIGNED_SHORT:
      vtkImageThresholdExecute(self, inData, inPtr, 
			       outData, (unsigned short *)(outPtr),outExt, id);
      break;
    case VTK_UNSIGNED_CHAR:
      vtkImageThresholdExecute(self, inData, inPtr, 
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
void vtkImageThreshold::ThreadedExecute(vtkImageData *inData, 
					vtkImageData *outData,
					int outExt[6], int id)
{
  void *inPtr = inData->GetScalarPointerForExtent(outExt);
  
  switch (inData->GetScalarType())
    {
    case VTK_FLOAT:
      vtkImageThresholdExecute1(this, inData, (float *)(inPtr), 
			       outData, outExt, id);
      break;
    case VTK_INT:
      vtkImageThresholdExecute1(this, inData, (int *)(inPtr), 
			       outData, outExt, id);
      break;
    case VTK_SHORT:
      vtkImageThresholdExecute1(this, inData, (short *)(inPtr), 
			       outData, outExt, id);
      break;
    case VTK_UNSIGNED_SHORT:
      vtkImageThresholdExecute1(this, inData, (unsigned short *)(inPtr), 
			       outData, outExt, id);
      break;
    case VTK_UNSIGNED_CHAR:
      vtkImageThresholdExecute1(this, inData, (unsigned char *)(inPtr), 
			       outData, outExt, id);
      break;
    default:
      vtkErrorMacro(<< "Execute: Unknown input ScalarType");
      return;
    }
}

void vtkImageThreshold::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkImageFilter::PrintSelf(os,indent);

  os << indent << "OutputScalarType: " << this->OutputScalarType << "\n";
  os << indent << "InValue: " << this->InValue << "\n";
  os << indent << "OutValue: " << this->OutValue << "\n";
  os << indent << "LowerThreshold: " << this->LowerThreshold << "\n";
  os << indent << "UpperThreshold: " << this->UpperThreshold << "\n";
  os << indent << "ReplaceIn: " << this->ReplaceIn << "\n";
  os << indent << "ReplaceOut: " << this->ReplaceOut << "\n";
}

