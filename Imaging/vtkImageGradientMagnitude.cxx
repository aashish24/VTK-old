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
#include <math.h>

#include "vtkImageGradientMagnitude.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkImageGradientMagnitude* vtkImageGradientMagnitude::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageGradientMagnitude");
  if(ret)
    {
    return (vtkImageGradientMagnitude*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageGradientMagnitude;
}





//----------------------------------------------------------------------------
// Construct an instance of vtkImageGradientMagnitude fitler.
vtkImageGradientMagnitude::vtkImageGradientMagnitude()
{
  this->Dimensionality = 2;
  this->HandleBoundaries = 1;
}


//----------------------------------------------------------------------------
void vtkImageGradientMagnitude::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkImageToImageFilter::PrintSelf(os, indent);
  os << indent << "HandleBoundaries: " << this->HandleBoundaries << "\n";
  os << indent << "Dimensionality: " << this->Dimensionality << "\n";
}

//----------------------------------------------------------------------------
// This method is passed a region that holds the image extent of this filters
// input, and changes the region to hold the image extent of this filters
// output.
void vtkImageGradientMagnitude::ExecuteInformation(vtkImageData *inData, 
						   vtkImageData *outData)
{  
  int extent[6];
  int idx;

  inData->GetWholeExtent(extent);
  if ( ! this->HandleBoundaries)
    {
    // shrink output image extent.
    for (idx = 0; idx < this->Dimensionality; ++idx)
      {
      extent[idx*2] += 1;
      extent[idx*2 + 1] -= 1;
      }
    }
  
  outData->SetWholeExtent(extent);
}


//----------------------------------------------------------------------------
// This method computes the input extent necessary to generate the output.
void vtkImageGradientMagnitude::ComputeInputUpdateExtent(int inExt[6],
							 int outExt[6])
{
  int *wholeExtent;
  int idx;

  wholeExtent = this->GetInput()->GetWholeExtent();
  
  memcpy(inExt,outExt,6*sizeof(int));
  
  // grow input whole extent.
  for (idx = 0; idx < this->Dimensionality; ++idx)
    {
    inExt[idx*2] -= 1;
    inExt[idx*2+1] += 1;
    if (this->HandleBoundaries)
      {
      // we must clip extent with whole extent is we hanlde boundaries.
      if (inExt[idx*2] < wholeExtent[idx*2])
	{
	inExt[idx*2] = wholeExtent[idx*2];
	}
      if (inExt[idx*2 + 1] > wholeExtent[idx*2 + 1])
	{
	inExt[idx*2 + 1] = wholeExtent[idx*2 + 1];
	}
      }
    }
}





//----------------------------------------------------------------------------
// This execute method handles boundaries.
// it handles boundaries. Pixels are just replicated to get values 
// out of extent.
template <class T>
static void vtkImageGradientMagnitudeExecute(vtkImageGradientMagnitude *self,
					     vtkImageData *inData, T *inPtr,
					     vtkImageData *outData, T *outPtr,
					     int outExt[6], int id)
{
  int idxC, idxX, idxY, idxZ;
  int maxC, maxX, maxY, maxZ;
  int inIncX, inIncY, inIncZ;
  int outIncX, outIncY, outIncZ;
  unsigned long count = 0;
  unsigned long target;
  int axesNum;
  int *wholeExtent, *inIncs;
  float r[3], d, sum;
  int useZMin, useZMax, useYMin, useYMax, useXMin, useXMax;
  
  // find the region to loop over
  maxC = outData->GetNumberOfScalarComponents();
  maxX = outExt[1] - outExt[0];
  maxY = outExt[3] - outExt[2]; 
  maxZ = outExt[5] - outExt[4];
  target = (unsigned long)((maxZ+1)*(maxY+1)/50.0);
  target++;

  // Get the dimensionality of the gradient.
  axesNum = self->GetDimensionality();
  
  // Get increments to march through data 
  inData->GetContinuousIncrements(outExt, inIncX, inIncY, inIncZ);
  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);

  // The data spacing is important for computing the gradient.
  inData->GetSpacing(r);
  r[0] = 0.5 / r[0];
  r[1] = 0.5 / r[1];
  r[2] = 0.5 / r[2];

  // get some other info we need
  inIncs = inData->GetIncrements(); 
  wholeExtent = inData->GetExtent(); 

  // Loop through ouput pixels
  for (idxZ = 0; idxZ <= maxZ; idxZ++)
    {
    useZMin = ((idxZ + outExt[4]) <= wholeExtent[4]) ? 0 : -inIncs[2];
    useZMax = ((idxZ + outExt[4]) >= wholeExtent[5]) ? 0 : inIncs[2];
    for (idxY = 0; !self->AbortExecute && idxY <= maxY; idxY++)
      {
      if (!id) 
	{
	if (!(count%target))
	  {
	  self->UpdateProgress(count/(50.0*target));
	  }
	count++;
	}
      useYMin = ((idxY + outExt[2]) <= wholeExtent[2]) ? 0 : -inIncs[1];
      useYMax = ((idxY + outExt[2]) >= wholeExtent[3]) ? 0 : inIncs[1];
      for (idxX = 0; idxX <= maxX; idxX++)
	{
	useXMin = ((idxX + outExt[0]) <= wholeExtent[0]) ? 0 : -inIncs[0];
	useXMax = ((idxX + outExt[0]) >= wholeExtent[1]) ? 0 : inIncs[0];
	for (idxC = 0; idxC < maxC; idxC++)
	  {
	  // do X axis
	  d = (float)(inPtr[useXMin]);
	  d -= (float)(inPtr[useXMax]);
	  d *= r[0]; // multiply by the data spacing
	  sum = d * d;
	  // do y axis
	  d = (float)(inPtr[useYMin]);
	  d -= (float)(inPtr[useYMax]);
	  d *= r[1]; // multiply by the data spacing
	  sum += (d * d);
	  if (axesNum == 3)
	    {
	    // do z axis
	    d = (float)(inPtr[useZMin]);
	    d -= (float)(inPtr[useZMax]);
	    d *= r[2]; // multiply by the data spacing
	    sum += (d * d);
	    }
	  *outPtr = (T)(sqrt(sum));
	  outPtr++;
	  inPtr++;
	  }
	}
      outPtr += outIncY;
      inPtr += inIncY;
      }
    outPtr += outIncZ;
    inPtr += inIncZ;
    }
}


//----------------------------------------------------------------------------
// This method contains a switch statement that calls the correct
// templated function for the input data type.  The output data
// must match input type.  This method does handle boundary conditions.
void vtkImageGradientMagnitude::ThreadedExecute(vtkImageData *inData, 
					 vtkImageData *outData,
					 int outExt[6], int id)
{
  void *inPtr = inData->GetScalarPointerForExtent(outExt);
  void *outPtr = outData->GetScalarPointerForExtent(outExt);
  
  vtkDebugMacro(<< "Execute: inData = " << inData 
		<< ", outData = " << outData);
  
  // this filter expects that input is the same type as output.
  if (inData->GetScalarType() != outData->GetScalarType())
    {
    vtkErrorMacro(<< "Execute: input ScalarType, " << inData->GetScalarType()
                  << ", must match out ScalarType " << outData->GetScalarType());
    return;
    }
  
  switch (inData->GetScalarType())
    {
    case VTK_DOUBLE:
      vtkImageGradientMagnitudeExecute(this, 
			  inData, (double *)(inPtr), 
			  outData, (double *)(outPtr), outExt, id);
      break;
    case VTK_FLOAT:
      vtkImageGradientMagnitudeExecute(this, 
			  inData, (float *)(inPtr), 
			  outData, (float *)(outPtr), outExt, id);
      break;
    case VTK_LONG:
      vtkImageGradientMagnitudeExecute(this, 
			  inData, (long *)(inPtr), 
			  outData, (long *)(outPtr), outExt, id);
      break;
    case VTK_UNSIGNED_LONG:
      vtkImageGradientMagnitudeExecute(this, 
			  inData, (unsigned long *)(inPtr), 
			  outData, (unsigned long *)(outPtr), outExt, id);
      break;
    case VTK_INT:
      vtkImageGradientMagnitudeExecute(this, 
			  inData, (int *)(inPtr), 
			  outData, (int *)(outPtr), outExt, id);
      break;
    case VTK_UNSIGNED_INT:
      vtkImageGradientMagnitudeExecute(this, 
			  inData, (unsigned int *)(inPtr), 
			  outData, (unsigned int *)(outPtr), outExt, id);
      break;
    case VTK_SHORT:
      vtkImageGradientMagnitudeExecute(this, 
			  inData, (short *)(inPtr), 
			  outData, (short *)(outPtr), outExt, id);
      break;
    case VTK_UNSIGNED_SHORT:
      vtkImageGradientMagnitudeExecute(this, 
			  inData, (unsigned short *)(inPtr), 
			  outData, (unsigned short *)(outPtr), outExt, id);
      break;
    case VTK_CHAR:
      vtkImageGradientMagnitudeExecute(this, 
			  inData, (char *)(inPtr), 
			  outData, (char *)(outPtr), outExt, id);
      break;
    case VTK_UNSIGNED_CHAR:
      vtkImageGradientMagnitudeExecute(this, 
			  inData, (unsigned char *)(inPtr), 
			  outData, (unsigned char *)(outPtr), outExt, id);
      break;
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }
}















