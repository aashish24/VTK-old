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

#include "vtkImageShrink3D.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkImageShrink3D* vtkImageShrink3D::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageShrink3D");
  if(ret)
    {
    return (vtkImageShrink3D*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageShrink3D;
}





//----------------------------------------------------------------------------
// Constructor: Sets default filter to be identity.
vtkImageShrink3D::vtkImageShrink3D()
{
  this->ShrinkFactors[0] = this->ShrinkFactors[1] = this->ShrinkFactors[2] = 1;
  this->Shift[0] = this->Shift[1] = this->Shift[2] = 0;
  this->Mean = 1;
  this->Median = 0;
  this->Maximum = 0;
  this->Minimum = 0;
}

void vtkImageShrink3D::SetMean (int value)
{
  if (value != this->Mean)
    {
    this->Mean = value;
    if (value == 1)
      {
      this->Minimum = 0;
      this->Maximum = 0;
      this->Median = 0;
      }      
    this->Modified();
    }
}

void vtkImageShrink3D::SetMinimum (int value)
{
  if (value != this->Minimum)
    {
    this->Minimum = value;
    if (value == 1)
      {
      this->Mean = 0;
      this->Maximum = 0;
      this->Median = 0;
      }      
    this->Modified();
    }
}

void vtkImageShrink3D::SetMaximum (int value)
{
  if (value != this->Maximum)
    {
    this->Maximum = value;
    if (value == 1)
      {
      this->Minimum = 0;
      this->Mean = 0;
      this->Median = 0;
      }      
    this->Modified();
    }
}

void vtkImageShrink3D::SetMedian (int value)
{
  if (value != this->Median)
    {
    this->Median = value;
    if (value == 1)
      {
      this->Minimum = 0;
      this->Maximum = 0;
      this->Mean = 0;
      }      
    this->Modified();
    }
}

void vtkImageShrink3D::SetAveraging (int value)
{
  this->SetMean(value);
}


//----------------------------------------------------------------------------
void vtkImageShrink3D::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkImageToImageFilter::PrintSelf(os,indent);

  os << indent << "ShrinkFactors: (" << this->ShrinkFactors[0] << ", "
     << this->ShrinkFactors[1] << ", " << this->ShrinkFactors[2] << ")\n";
  os << indent << "Shift: (" << this->Shift[0] << ", "
     << this->Shift[1] << ", " << this->Shift[2] << ")\n";

  os << indent << "Averaging: " << (this->Mean ? "On\n" : "Off\n");
  os << indent << "Mean: " << (this->Mean ? "On\n" : "Off\n");
  os << indent << "Minimum: " << (this->Minimum ? "On\n" : "Off\n");
  os << indent << "Maximum: " << (this->Maximum ? "On\n" : "Off\n");
  os << indent << "Median: " << (this->Median ? "On\n" : "Off\n");
  

}

//----------------------------------------------------------------------------
// This method computes the Region of input necessary to generate outRegion.
void vtkImageShrink3D::ComputeRequiredInputUpdateExtent(int inExt[6], 
							int outExt[6])
{
  int idx;
  
  for (idx = 0; idx < 3; ++idx)
    {
    // For Min.
    inExt[idx*2] = outExt[idx*2] * this->ShrinkFactors[idx] 
      + this->Shift[idx];
    // For Max.
    inExt[idx*2+1] = outExt[idx*2+1] * this->ShrinkFactors[idx]
      + this->Shift[idx];
    // If we are not sub sampling, we need a little more
    if (this->Mean || this->Minimum || this->Maximum || this->Median)
      {
      inExt[idx*2+1] += this->ShrinkFactors[idx] - 1;
      }
    }
}


//----------------------------------------------------------------------------
// Computes any global image information associated with regions.
// Any problems with roundoff or negative numbers ???
void vtkImageShrink3D::ExecuteInformation(vtkImageData *inData, 
					  vtkImageData *outData)
{
  int idx;
  int wholeExtent[6];
  float spacing[3];
  

  inData->GetWholeExtent(wholeExtent);
  inData->GetSpacing(spacing);

  for (idx = 0; idx < 3; ++idx)
    {
    // Scale the output extent
    wholeExtent[2*idx] = 
      (int)(ceil((float)(wholeExtent[2*idx] - this->Shift[idx]) 
		 / (float)(this->ShrinkFactors[idx])));
    wholeExtent[2*idx+1] = (int)(floor(
     (float)(wholeExtent[2*idx+1]-this->Shift[idx]-this->ShrinkFactors[idx]+1)
         / (float)(this->ShrinkFactors[idx])));
    // Change the data spacing
    spacing[idx] *= (float)(this->ShrinkFactors[idx]);
    }

  outData->SetWholeExtent(wholeExtent);
  outData->SetSpacing(spacing);
}



template <class T>
static int compare(const T *y1,const T *y2)
{
  if ( *y1 <  *y2) 
    {
    return -1; 
    }
  
  if ( *y1 == *y2) 
    {
    return  0;
    }
  
  return  1;
}

//----------------------------------------------------------------------------
// The templated execute function handles all the data types.
template <class T>
static void vtkImageShrink3DExecute(vtkImageShrink3D *self,
				    vtkImageData *inData, T *inPtr,
				    vtkImageData *outData, T *outPtr,
				    int outExt[6], int id)
{
  int outIdx0, outIdx1, outIdx2, inIdx0, inIdx1, inIdx2;
  int inInc0, inInc1, inInc2;
  T *inPtr0, *inPtr1, *inPtr2;
  int outInc0, outInc1, outInc2;
  int tmpInc0, tmpInc1, tmpInc2;
  T *tmpPtr0, *tmpPtr1, *tmpPtr2;
  int factor0, factor1, factor2;
  float sum, norm;
  unsigned long count = 0;
  unsigned long target;
  int idxC, maxC, maxX;
  T *outPtr2;

  // black magic to force the correct version of the comparison function
  // to be instantiated AND used.
  int (*compareF1)(const T*, const T*) = compare;
  int (*compareFn)(const void*, const void*)
    = (int (*)(const void*, const void*)) compareF1;

  self->GetShrinkFactors(factor0, factor1, factor2);
  
  // Get information to march through data 
  inData->GetIncrements(inInc0, inInc1, inInc2);
  tmpInc0 = inInc0 * factor0;
  tmpInc1 = inInc1 * factor1;
  tmpInc2 = inInc2 * factor2;
  outData->GetContinuousIncrements(outExt,outInc0, outInc1, outInc2);

  target = (unsigned long)((outExt[5] - outExt[4] + 1)*
			   (outExt[3] - outExt[2] + 1)/50.0);
  target++;

  maxX = outExt[1] - outExt[0];
  maxC = inData->GetNumberOfScalarComponents();
  
  if (self->GetMean())
    {
    norm = 1.0 / (float)(factor0 * factor1 * factor2);
    // Loop through output pixels
    for (idxC = 0; idxC < maxC; idxC++)
      {
      tmpPtr2 = inPtr + idxC;
      outPtr2 = outPtr + idxC;
      for (outIdx2 = outExt[4]; outIdx2 <= outExt[5]; ++outIdx2)
	{
	tmpPtr1 = tmpPtr2;
	for (outIdx1 = outExt[2]; 
	     !self->AbortExecute && outIdx1 <= outExt[3]; ++outIdx1)
	  {
	  if (!id) 
	    {
	    if (!(count%target))
	      {
	      self->UpdateProgress(count/(50.0*target));
	      }
	    count++;
	    }
	  tmpPtr0 = tmpPtr1;
	  for (outIdx0 = 0; outIdx0 <= maxX; ++outIdx0)
	    {
	    sum = 0.0;
	    // Loop through neighborhood pixels
	    inPtr2 = tmpPtr0;
	    for (inIdx2 = 0; inIdx2 < factor2; ++inIdx2)
	      {
	      inPtr1 = inPtr2;
	      for (inIdx1 = 0; inIdx1 < factor1; ++inIdx1)
		{
		inPtr0 = inPtr1;
		for (inIdx0 = 0; inIdx0 < factor0; ++inIdx0)
		  {
		  sum += (float)(*inPtr0);
		  inPtr0 += inInc0;
		  }
		inPtr1 += inInc1;
		}
	      inPtr2 += inInc2;
	      }
	    *outPtr2 = (T)(sum * norm);
	    tmpPtr0 += tmpInc0;
	    outPtr2 += maxC;
	    }
	  tmpPtr1 += tmpInc1;
	  outPtr2 += outInc1;
	  }
	tmpPtr2 += tmpInc2;
	outPtr2 += outInc2;
	}
      }
    }
  else if (self->GetMinimum())
    {
    T minValue;
    // Loop through output pixels
    for (idxC = 0; idxC < maxC; idxC++)
      {
      tmpPtr2 = inPtr + idxC;
      outPtr2 = outPtr + idxC;
      for (outIdx2 = outExt[4]; outIdx2 <= outExt[5]; ++outIdx2)
	{
	tmpPtr1 = tmpPtr2;
	for (outIdx1 = outExt[2]; 
	     !self->AbortExecute && outIdx1 <= outExt[3]; ++outIdx1)
	  {
	  if (!id) 
	    {
	    if (!(count%target))
	      {
	      self->UpdateProgress(count/(50.0*target));
	      }
	    count++;
	    }
	  tmpPtr0 = tmpPtr1;
	  for (outIdx0 = 0; outIdx0 <= maxX; ++outIdx0)
	    {
            minValue = (T) self->GetOutput()->GetScalarTypeMax();
	    // Loop through neighborhood pixels
	    inPtr2 = tmpPtr0;
	    for (inIdx2 = 0; inIdx2 < factor2; ++inIdx2)
	      {
	      inPtr1 = inPtr2;
	      for (inIdx1 = 0; inIdx1 < factor1; ++inIdx1)
		{
		inPtr0 = inPtr1;
		for (inIdx0 = 0; inIdx0 < factor0; ++inIdx0)
		  {
		  if (*inPtr0 < minValue)
		    {
		    minValue = *inPtr0;
		    }
		  inPtr0 += inInc0;
		  }
		inPtr1 += inInc1;
		}
	      inPtr2 += inInc2;
	      }
	    *outPtr2 = minValue;
	    tmpPtr0 += tmpInc0;
	    outPtr2 += maxC;
	    }
	  tmpPtr1 += tmpInc1;
	  outPtr2 += outInc1;
	  }
	tmpPtr2 += tmpInc2;
	outPtr2 += outInc2;
	}
      }
    }
  else if (self->GetMaximum())
    {
    T maxValue;
    // Loop through output pixels
    for (idxC = 0; idxC < maxC; idxC++)
      {
      tmpPtr2 = inPtr + idxC;
      outPtr2 = outPtr + idxC;
      for (outIdx2 = outExt[4]; outIdx2 <= outExt[5]; ++outIdx2)
	{
	tmpPtr1 = tmpPtr2;
	for (outIdx1 = outExt[2]; 
	     !self->AbortExecute && outIdx1 <= outExt[3]; ++outIdx1)
	  {
	  if (!id) 
	    {
	    if (!(count%target))
	      {
	      self->UpdateProgress(count/(50.0*target));
	      }
	    count++;
	    }
	  tmpPtr0 = tmpPtr1;
	  for (outIdx0 = 0; outIdx0 <= maxX; ++outIdx0)
	    {
            maxValue = (T) self->GetOutput()->GetScalarTypeMin();
	    // Loop through neighborhood pixels
	    inPtr2 = tmpPtr0;
	    for (inIdx2 = 0; inIdx2 < factor2; ++inIdx2)
	      {
	      inPtr1 = inPtr2;
	      for (inIdx1 = 0; inIdx1 < factor1; ++inIdx1)
		{
		inPtr0 = inPtr1;
		for (inIdx0 = 0; inIdx0 < factor0; ++inIdx0)
		  {
		  if (*inPtr0 > maxValue)
		    {
		    maxValue = *inPtr0;
		    }
		  inPtr0 += inInc0;
		  }
		inPtr1 += inInc1;
		}
	      inPtr2 += inInc2;
	      }
	    *outPtr2 = maxValue;
	    tmpPtr0 += tmpInc0;
	    outPtr2 += maxC;
	    }
	  tmpPtr1 += tmpInc1;
	  outPtr2 += outInc1;
	  }
	tmpPtr2 += tmpInc2;
	outPtr2 += outInc2;
	}
      }
    }
  else if (self->GetMedian())
    {
    T* kernel = new T [factor0 * factor1 * factor2];
    int index;
    
    // Loop through output pixels
    for (idxC = 0; idxC < maxC; idxC++)
      {
      tmpPtr2 = inPtr + idxC;
      outPtr2 = outPtr + idxC;
      for (outIdx2 = outExt[4]; outIdx2 <= outExt[5]; ++outIdx2)
	{
	tmpPtr1 = tmpPtr2;
	for (outIdx1 = outExt[2]; 
	     !self->AbortExecute && outIdx1 <= outExt[3]; ++outIdx1)
	  {
	  if (!id) 
	    {
	    if (!(count%target))
	      {
	      self->UpdateProgress(count/(50.0*target));
	      }
	    count++;
	    }
	  tmpPtr0 = tmpPtr1;
	  for (outIdx0 = 0; outIdx0 <= maxX; ++outIdx0)
	    {
	    // Loop through neighborhood pixels
	    inPtr2 = tmpPtr0;
	    index = 0;
	    for (inIdx2 = 0; inIdx2 < factor2; ++inIdx2)
	      {
	      inPtr1 = inPtr2;
	      for (inIdx1 = 0; inIdx1 < factor1; ++inIdx1)
		{
		inPtr0 = inPtr1;
		for (inIdx0 = 0; inIdx0 < factor0; ++inIdx0)
		  {
		  kernel[index++] = *inPtr0;

		  inPtr0 += inInc0;
		  }
		inPtr1 += inInc1;
		}
	      inPtr2 += inInc2;
	      }
	    qsort(kernel,index,sizeof(T),compareFn);
	    *outPtr2 = *(kernel + index/2);

	    tmpPtr0 += tmpInc0;
	    outPtr2 += maxC;
	    }
	  tmpPtr1 += tmpInc1;
	  outPtr2 += outInc1;
	  }
	tmpPtr2 += tmpInc2;
	outPtr2 += outInc2;
	}
      }
    delete [] kernel;
    }
  else // Just SubSample
    {
    // Loop through output pixels
    for (idxC = 0; idxC < maxC; idxC++)
      {
      tmpPtr2 = inPtr + idxC;
      outPtr2 = outPtr + idxC;
      for (outIdx2 = outExt[4]; outIdx2 <= outExt[5]; ++outIdx2)
	{
	tmpPtr1 = tmpPtr2;
	for (outIdx1 = outExt[2]; 
	     !self->AbortExecute && outIdx1 <= outExt[3]; ++outIdx1)
	  {
	  if (!id) 
	    {
	    if (!(count%target))
	      {
	      self->UpdateProgress(count/(50.0*target));
	      }
	    count++;
	    }
	  tmpPtr0 = tmpPtr1;
	  for (outIdx0 = 0; outIdx0 <= maxX; ++outIdx0)
	    {
	    *outPtr2 = *tmpPtr0;

	    tmpPtr0 += tmpInc0;
	    outPtr2 += maxC;
	    }
	  tmpPtr1 += tmpInc1;
	  outPtr2 += outInc1;
	  }
	tmpPtr2 += tmpInc2;
	outPtr2 += outInc2;
	}
      }
    }
}

    
//----------------------------------------------------------------------------
// This method uses the input data to fill the output data.
// It can handle any type data, but the two datas must have the same 
// data type.
void vtkImageShrink3D::ThreadedExecute(vtkImageData *inData, 
				       vtkImageData *outData,
				       int outExt[6], int id)
{
  int inExt[6];
  void *outPtr = outData->GetScalarPointerForExtent(outExt);
  
  vtkDebugMacro(<< "Execute: inData = " << inData 
  << ", outData = " << outData);

  this->ComputeRequiredInputUpdateExtent(inExt,outExt);
  void *inPtr = inData->GetScalarPointerForExtent(inExt);

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
      vtkImageShrink3DExecute(this, 
			      inData, (double *)(inPtr), 
			      outData, (double *)(outPtr), outExt, id);
      break;
    case VTK_FLOAT:
      vtkImageShrink3DExecute(this, 
			      inData, (float *)(inPtr), 
			      outData, (float *)(outPtr), outExt, id);
      break;
    case VTK_LONG:
      vtkImageShrink3DExecute(this, 
			      inData, (long *)(inPtr), 
			      outData, (long *)(outPtr), outExt, id);
      break;
    case VTK_UNSIGNED_LONG:
      vtkImageShrink3DExecute(this, 
			      inData, (unsigned long *)(inPtr), 
			      outData, (unsigned long *)(outPtr), outExt, id);
      break;
    case VTK_INT:
      vtkImageShrink3DExecute(this, 
			      inData, (int *)(inPtr), 
			      outData, (int *)(outPtr), outExt, id);
      break;
    case VTK_UNSIGNED_INT:
      vtkImageShrink3DExecute(this, 
			      inData, (unsigned int *)(inPtr), 
			      outData, (unsigned int *)(outPtr), outExt, id);
      break;
    case VTK_SHORT:
      vtkImageShrink3DExecute(this, 
			      inData, (short *)(inPtr), 
			      outData, (short *)(outPtr), outExt, id);
      break;
    case VTK_UNSIGNED_SHORT:
      vtkImageShrink3DExecute(this, 
			      inData, (unsigned short *)(inPtr), 
			      outData, (unsigned short *)(outPtr), outExt, id);
      break;
    case VTK_CHAR:
      vtkImageShrink3DExecute(this, 
			      inData, (char *)(inPtr), 
			      outData, (char *)(outPtr), outExt, id);
      break;
    case VTK_UNSIGNED_CHAR:
      vtkImageShrink3DExecute(this, 
			      inData, (unsigned char *)(inPtr), 
			      outData, (unsigned char *)(outPtr), outExt, id);
      break;
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }
}














