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
#include "vtkImageMagnify.h"



//----------------------------------------------------------------------------
// Constructor: Sets default filter to be identity.
vtkImageMagnify::vtkImageMagnify()
{
  int idx;
  
  this->Interpolate = 0;
  for (idx = 0; idx < 3; ++idx)
    {
    this->MagnificationFactors[idx] = 1;
    }
}


//----------------------------------------------------------------------------
// Computes any global image information associated with regions.
void vtkImageMagnify::ExecuteInformation()
{
  float *spacing;
  int idx;
  int *inExt;
  float outSpacing[3];
  int outExt[6];
  
  inExt = this->GetInput()->GetWholeExtent();
  spacing = this->GetInput()->GetSpacing();
  for (idx = 0; idx < 3; idx++)
    {
    // Scale the output extent
    outExt[idx*2] = inExt[idx*2] * this->MagnificationFactors[idx];
    outExt[idx*2+1] = outExt[idx*2] + 
      (inExt[idx*2+1] - inExt[idx*2] + 1)*this->MagnificationFactors[idx] - 1;
    
    // Change the data spacing
    outSpacing[idx] = spacing[idx] / (float)(this->MagnificationFactors[idx]);
    }
  
  this->GetOutput()->SetWholeExtent(outExt);
  this->GetOutput()->SetSpacing(outSpacing);
}

//----------------------------------------------------------------------------
// This method computes the Region of input necessary to generate outRegion.
// It assumes offset and size are multiples of Magnify Factors.
void vtkImageMagnify::ComputeInputUpdateExtent(int inExt[6],
						       int outExt[6])
{
  int idx;
  
  for (idx = 0; idx < 3; idx++)
    {
    // For Min. Round Down
    inExt[idx*2] = (int)(floor((float)(outExt[idx*2]) / 
			       (float)(this->MagnificationFactors[idx])));
    inExt[idx*2+1] = (int)(floor((float)(outExt[idx*2+1]) / 
				 (float)(this->MagnificationFactors[idx])));
    }
}




//----------------------------------------------------------------------------
// The templated execute function handles all the data types.
// 2d even though operation is 1d.
// Note: Slight misalignment (pixel replication is not nearest neighbor).
template <class T>
static void vtkImageMagnifyExecute(vtkImageMagnify *self,
				  vtkImageData *inData, T *inPtr, int inExt[6],
				  vtkImageData *outData, T *outPtr,
				  int outExt[6], int id)
{
  int idxC, idxX, idxY, idxZ;
  int inIdxX, inIdxY, inIdxZ;
  int inMaxX, inMaxY, inMaxZ;
  int maxC, maxX, maxY, maxZ;
  int inIncX, inIncY, inIncZ;
  int outIncX, outIncY, outIncZ;
  unsigned long count = 0;
  unsigned long target;
  int interpolate;
  int magXIdx, magX;
  int magYIdx, magY;
  int magZIdx, magZ;
  T *inPtrZ, *inPtrY, *inPtrX, *outPtrC;
  float iMag, iMagP, iMagPY, iMagPZ, iMagPYZ;
  T dataP, dataPX, dataPY, dataPZ;
  T dataPXY, dataPXZ, dataPYZ, dataPXYZ;
  int interpSetup;
  
  interpolate = self->GetInterpolate();
  magX = self->GetMagnificationFactors()[0];
  magY = self->GetMagnificationFactors()[1];
  magZ = self->GetMagnificationFactors()[2];
  iMag = 1.0/(magX*magY*magZ);
  
  // find the region to loop over
  maxC = outData->GetNumberOfScalarComponents();
  maxX = outExt[1] - outExt[0];
  maxY = outExt[3] - outExt[2]; 
  maxZ = outExt[5] - outExt[4];
  target = (unsigned long)(maxC*(maxZ+1)*(maxY+1)/50.0);
  target++;
  
  // Get increments to march through data 
  inData->GetIncrements(inIncX, inIncY, inIncZ);
  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);

  // Now I am putting in my own boundary check because of ABRs and FMRs
  // And I do not understand (nor do I care to figure out) what
  // Ken is doing with his checks. (Charles)
  inMaxX = inExt[1];
  inMaxY = inExt[3];
  inMaxZ = inExt[5];
  inData->GetExtent(idxC, inMaxX, idxC, inMaxY, idxC, inMaxZ);
  
  // Loop through ouput pixels
  for (idxC = 0; idxC < maxC; idxC++)
    {
    inPtrZ = inPtr + idxC;
    inIdxZ = inExt[4];
    outPtrC = outPtr + idxC;
    magZIdx = magZ - outExt[4]%magZ - 1;
    for (idxZ = 0; idxZ <= maxZ; idxZ++, magZIdx--)
      {
      inPtrY = inPtrZ;
      inIdxY = inExt[2];
      magYIdx = magY - outExt[2]%magY - 1;
      for (idxY = 0; !self->AbortExecute && idxY <= maxY; idxY++, magYIdx--)
	{
	if (!id) 
	  {
	  if (!(count%target))
	    {
	    self->UpdateProgress(count/(50.0*target));
	    }
	  count++;
	  }
	
	if (interpolate)
	  {
	  // precompute some values for interpolation
	  iMagP = (magYIdx + 1)*(magZIdx + 1)*iMag;
	  iMagPY = (magY - magYIdx - 1)*(magZIdx + 1)*iMag;
	  iMagPZ = (magYIdx + 1)*(magZ - magZIdx - 1)*iMag;
	  iMagPYZ = (magY - magYIdx - 1)*(magZ - magZIdx - 1)*iMag;
	  }
	
	magXIdx = magX - outExt[0]%magX - 1;
	inPtrX = inPtrY;
	inIdxX = inExt[0];
	interpSetup = 0;
	for (idxX = 0; idxX <= maxX; idxX++, magXIdx--)
	  {
	  // Pixel operation
	  if (!interpolate)
	    {
	    *outPtrC = *inPtrX;
	    }
	  else
	    {
	    // setup data values for interp, overload dataP as an 
	    // indicator of if this has been done yet
	    if (!interpSetup) 
	      {
	      int tiX, tiY, tiZ;
	      
	      dataP = *inPtrX;

	      // Now I am putting in my own boundary check because of 
	      // ABRs and FMRs
	      // And I do not understand (nor do I care to figure out) what
	      // Ken was doing with his checks. (Charles)
	      if (inIdxX < inMaxX) 
		{
		tiX = inIncX;
		}
	      else
		{
		tiX = 0;
		}
	      if (inIdxY < inMaxY) 
		{
		tiY = inIncY;
		}
	      else
		{
		tiY = 0;
		}
	      if (inIdxZ < inMaxZ)
		{
		tiZ = inIncZ;
		}
	      else
		{
		tiZ = 0;
		}
	      dataPX = *(inPtrX + tiX); 
	      dataPY = *(inPtrX + tiY); 
	      dataPZ = *(inPtrX + tiZ);
	      dataPXY = *(inPtrX + tiX + tiY); 
	      dataPXZ = *(inPtrX + tiX + tiZ); 
	      dataPYZ = *(inPtrX + tiY + tiZ); 
	      dataPXYZ = *(inPtrX + tiX + tiY + tiZ); 
	      interpSetup = 1;
	      }
	    *outPtrC = (T)
	      (dataP*(magXIdx + 1)*iMagP + 
	       dataPX*(magX - magXIdx - 1)*iMagP +
	       dataPY*(magXIdx + 1)*iMagPY + 
	       dataPXY*(magX - magXIdx - 1)*iMagPY +
	       dataPZ*(magXIdx + 1)*iMagPZ + 
	       dataPXZ*(magX - magXIdx - 1)*iMagPZ +
	       dataPYZ*(magXIdx + 1)*iMagPYZ + 
	       dataPXYZ*(magX - magXIdx - 1)*iMagPYZ);
	    }
	  outPtrC += maxC;
	  if (!magXIdx) 
	    {
	    inPtrX += inIncX;
	    ++inIdxX;
	    magXIdx = magX;
	    interpSetup = 0;
	    }
	  }
	outPtrC += outIncY;
	if (!magYIdx) 
	  {
	  inPtrY += inIncY;
	  ++inIdxY;
	  magYIdx = magY;
	  }
	}
      outPtrC += outIncZ;
      if (!magZIdx) 
	{
	inPtrZ += inIncZ;
	++inIdxZ;
	magZIdx = magZ;
	}
      }
    }
}

void vtkImageMagnify::ThreadedExecute(vtkImageData *inData, 
				      vtkImageData *outData,
				      int outExt[6], int id)
{
  int inExt[6];
  this->ComputeInputUpdateExtent(inExt,outExt);
  void *inPtr = inData->GetScalarPointerForExtent(inExt);
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
      vtkImageMagnifyExecute(this, 
			     inData, (double *)(inPtr), inExt,
			     outData, (double *)(outPtr), outExt, id);
      break;
    case VTK_FLOAT:
      vtkImageMagnifyExecute(this, 
			     inData, (float *)(inPtr), inExt,
			     outData, (float *)(outPtr), outExt, id);
      break;
    case VTK_LONG:
      vtkImageMagnifyExecute(this, 
			     inData, (long *)(inPtr), inExt,
			     outData, (long *)(outPtr), outExt, id);
      break;
    case VTK_UNSIGNED_LONG:
      vtkImageMagnifyExecute(this, 
			     inData, (unsigned long *)(inPtr), inExt,
			     outData, (unsigned long *)(outPtr), outExt, id);
      break;
    case VTK_INT:
      vtkImageMagnifyExecute(this, 
			     inData, (int *)(inPtr), inExt,
			     outData, (int *)(outPtr), outExt, id);
      break;
    case VTK_UNSIGNED_INT:
      vtkImageMagnifyExecute(this, 
			     inData, (unsigned int *)(inPtr), inExt,
			     outData, (unsigned int *)(outPtr), outExt, id);
      break;
    case VTK_SHORT:
      vtkImageMagnifyExecute(this, 
			     inData, (short *)(inPtr), inExt,
			     outData, (short *)(outPtr), outExt, id);
      break;
    case VTK_UNSIGNED_SHORT:
      vtkImageMagnifyExecute(this, 
			     inData, (unsigned short *)(inPtr), inExt,
			     outData, (unsigned short *)(outPtr), outExt, id);
      break;
    case VTK_CHAR:
      vtkImageMagnifyExecute(this, 
			     inData, (char *)(inPtr), inExt,
			     outData, (char *)(outPtr), outExt, id);
      break;
    case VTK_UNSIGNED_CHAR:
      vtkImageMagnifyExecute(this, 
			     inData, (unsigned char *)(inPtr), inExt,
			     outData, (unsigned char *)(outPtr), outExt, id);
      break;
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }
}

void vtkImageMagnify::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkImageToImageFilter::PrintSelf(os,indent);

  os << indent << "MagnificationFactors: ( "
     << this->MagnificationFactors[0] << ", "
     << this->MagnificationFactors[1] << ", "
     << this->MagnificationFactors[2] << " )\n";

  os << indent << "Interpolate: " << (this->Interpolate ? "On\n" : "Off\n");

}

