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
#include "vtkImageIslandRemoval2D.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkImageIslandRemoval2D* vtkImageIslandRemoval2D::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageIslandRemoval2D");
  if(ret)
    {
    return (vtkImageIslandRemoval2D*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageIslandRemoval2D;
}







//----------------------------------------------------------------------------
// Constructor: Sets default filter to be identity.
vtkImageIslandRemoval2D::vtkImageIslandRemoval2D()
{
  this->AreaThreshold = 0;
  this->SetAreaThreshold(4);
  this->SquareNeighborhood = 1;
  this->SquareNeighborhoodOff();
  this->ReplaceValue = 0;
  this->SetReplaceValue(255);
  this->IslandValue = 255;
  this->SetIslandValue(0);
}

//----------------------------------------------------------------------------
void vtkImageIslandRemoval2D::PrintSelf(vtkOstream& os, vtkIndent indent)
{
  vtkImageToImageFilter::PrintSelf(os,indent);
  os << indent << "AreaThreshold: " << this->AreaThreshold;
  if (this->SquareNeighborhood)
    {
    os << indent << "Neighborhood: Square";
    }
  else
    {
    os << indent << "Neighborhood: Cross";
    }
  os << indent << "IslandValue: " << this->IslandValue;
  os << indent << "ReplaceValue: " << this->ReplaceValue;
  
}


//----------------------------------------------------------------------------
void vtkImageIslandRemoval2D::EnlargeOutputUpdateExtents( vtkDataObject *vtkNotUsed(data) )
{
  int wholeExtent[6];
  int extent[6];
  
  memcpy(wholeExtent,this->GetOutput()->GetWholeExtent(),6*sizeof(int));
  memcpy(extent,this->GetOutput()->GetUpdateExtent(),6*sizeof(int));
  extent[0] = wholeExtent[0];
  extent[1] = wholeExtent[1];
  extent[2] = wholeExtent[2];
  extent[3] = wholeExtent[3];
  this->GetOutput()->SetUpdateExtent(extent);
}


//----------------------------------------------------------------------------
// The templated execute function handles all the data types.
// Codes:  0 => unvisited. 1 => visted don't know. 
//         2 => visted keep.  3 => visited replace.
// Please excuse the length of this function.  The easiest way to choose
// neighborhoods is to check neighbors one by one directly.  Also, I did
// not want to break the templated function into pieces.
template <class T>
static void vtkImageIslandRemoval2DExecute(vtkImageIslandRemoval2D *self,
					  vtkImageData *inData, T *inPtr,
					  vtkImageData *outData, T *outPtr,
					   int outExt[6])
{
  int outIdx0, outIdx1, outIdx2;
  int outInc0, outInc1, outInc2;
  T *outPtr0, *outPtr1, *outPtr2;
  int inInc0, inInc1, inInc2;
  T *inPtr0, *inPtr1, *inPtr2;
  vtkImage2DIslandPixel *pixels;  // All the pixels visited so far.
  int numPixels;      // The number of pixels visited so far.
  vtkImage2DIslandPixel *newPixel;   // The last pixel in the list.
  int nextPixelIdx;   // The index of the next pixel to grow.
  vtkImage2DIslandPixel *nextPixel;  // The next pixel to grow.
  int keepValue;
  int area;
  int squareNeighborhood;
  T islandValue;
  T replaceValue;
  T *inNeighborPtr, *outNeighborPtr;
  int idxC, maxC;
  unsigned long count = 0;
  unsigned long target;

  squareNeighborhood = self->GetSquareNeighborhood();
  area = self->GetAreaThreshold();
  islandValue = (T)(self->GetIslandValue());
  replaceValue = (T)(self->GetReplaceValue());
  numPixels = 0;
  // In case all 8 neighbors get added before we test the number.
  pixels = new vtkImage2DIslandPixel [area + 8]; 
  
  outData->GetIncrements(outInc0, outInc1, outInc2);
  inData->GetIncrements(inInc0, inInc1, inInc2);
  maxC = outData->GetNumberOfScalarComponents();
  
  // Loop through pixels setting all output to 0 (unvisited).
  for (idxC = 0; idxC < maxC; idxC++)
    {
    outPtr2 = outPtr + idxC;
    for (outIdx2 = outExt[4]; outIdx2 <= outExt[5]; ++outIdx2)
      {
      outPtr1 = outPtr2;
      for (outIdx1 = outExt[2]; outIdx1 <= outExt[3]; ++outIdx1)
	{
	outPtr0 = outPtr1;
	for (outIdx0 = outExt[0]; outIdx0 <= outExt[1]; ++outIdx0)
	  {
	  *outPtr0 = (T)(0);  // Unvisited
	  outPtr0 += outInc0;
	  }
	outPtr1 += outInc1;
	}
      outPtr2 += outInc2;
      }
    }

  // update the progress and possibly abort
  self->UpdateProgress(0.1);
  if (self->AbortExecute)
    {
    return;
    }
		       
  target = (unsigned long)(maxC*(outExt[5]-outExt[4]+1)*
			   (outExt[3]-outExt[2]+1)/50.0);
  target++;

  // Loop though all pixels looking for islands
  for (idxC = 0; idxC < maxC; idxC++)
    {
    outPtr2 = outPtr + idxC;
    inPtr2 = inPtr + idxC;
    for (outIdx2 = outExt[4]; 
	 !self->AbortExecute && outIdx2 <= outExt[5]; ++outIdx2)
      {
      if (!(count%target))
	{
	self->UpdateProgress(0.1 + 0.8*count/(50.0*target));
	}
      count++;
      outPtr1 = outPtr2;
      inPtr1 = inPtr2;
      for (outIdx1 = outExt[2]; outIdx1 <= outExt[3]; ++outIdx1)
	{
	outPtr0 = outPtr1;
	inPtr0 = inPtr1;
	for (outIdx0 = outExt[0]; outIdx0 <= outExt[1]; ++outIdx0)
	  {
	  if (*outPtr0 == 0)
	    {
	    if (*inPtr0 != islandValue)
	      {
	      // not an island, keep and go on
	      *outPtr0 = 2;
	      }
	    else
	      {
	    
	      // Start an island search
	      // Save first pixel.
	      newPixel = pixels;
	      newPixel->inPtr = (void *)(inPtr0);
	      newPixel->outPtr = (void *)(outPtr0);
	      newPixel->idx0 = outIdx0;
	      newPixel->idx1 = outIdx1;
	      numPixels = 1;
	      nextPixelIdx = 0;
	      nextPixel = pixels;
	      *outPtr0 = 1;  // visited don't know
	      keepValue = 1;
	      // breadth first search
	      while (keepValue == 1)  // don't know
		{
		// Check all the neighbors
		// left
		if (nextPixel->idx0 > outExt[0])
		  {
		  inNeighborPtr = (T *)(nextPixel->inPtr) - inInc0;
		  if ( *inNeighborPtr == islandValue)
		    {
		    outNeighborPtr = (T *)(nextPixel->outPtr) - outInc0;
		    if ( *outNeighborPtr == 2)
		      {
		      // This is part of a bigger island.
		      keepValue = 2;
		      }
		    if ( *outNeighborPtr == 0)
		      {
		      // New pixel to add
		      ++newPixel;
		      ++numPixels;
		      newPixel->inPtr = (void *)(inNeighborPtr);
		      newPixel->outPtr = (void *)(outNeighborPtr);
		      newPixel->idx0 = nextPixel->idx0 - 1;
		      newPixel->idx1 = nextPixel->idx1;
		      *outNeighborPtr = 1;  // visited don't know
		      }
		    }
		  }
		// right
		if (nextPixel->idx0 < outExt[1])
		  {
		  inNeighborPtr = (T *)(nextPixel->inPtr) + inInc0;
		  if ( *inNeighborPtr == islandValue)
		    {
		    outNeighborPtr = (T *)(nextPixel->outPtr) + outInc0;
		    if ( *outNeighborPtr == 2)
		      {
		      // This is part of a bigger island.
		      keepValue = 2;
		      }
		    if ( *outNeighborPtr == 0)
		      {
		      // New pixel to add
		      ++newPixel;
		      ++numPixels;
		      newPixel->inPtr = (void *)(inNeighborPtr);
		      newPixel->outPtr = (void *)(outNeighborPtr);
		      newPixel->idx0 = nextPixel->idx0 + 1;
		      newPixel->idx1 = nextPixel->idx1;
		      *outNeighborPtr = 1;  // visited don't know
		      }
		    }
		  }
		// up
		if (nextPixel->idx1 > outExt[2])
		  {
		  inNeighborPtr = (T *)(nextPixel->inPtr) - inInc1;
		  if ( *inNeighborPtr == islandValue)
		    {
		    outNeighborPtr = (T *)(nextPixel->outPtr) - outInc1;
		    if ( *outNeighborPtr == 2)
		      {
		      // This is part of a bigger island.
		      keepValue = 2;
		      }
		    if ( *outNeighborPtr == 0)
		      {
		      // New pixel to add
		      ++newPixel;
		      ++numPixels;
		      newPixel->inPtr = (void *)(inNeighborPtr);
		      newPixel->outPtr = (void *)(outNeighborPtr);
		      newPixel->idx0 = nextPixel->idx0;
		      newPixel->idx1 = nextPixel->idx1 - 1;
		      *outNeighborPtr = 1;  // visited don't know
		      }
		    }
		  }
		// down
		if (nextPixel->idx1 < outExt[3])
		  {
		  inNeighborPtr = (T *)(nextPixel->inPtr) + inInc1;
		  if ( *inNeighborPtr == islandValue)
		    {
		    outNeighborPtr = (T *)(nextPixel->outPtr) + outInc1;
		    if ( *outNeighborPtr == 2)
		      {
		      // This is part of a bigger island.
		      keepValue = 2;
		      }
		    if ( *outNeighborPtr == 0)
		      {
		      // New pixel to add
		      ++newPixel;
		      ++numPixels;
		      newPixel->inPtr = (void *)(inNeighborPtr);
		      newPixel->outPtr = (void *)(outNeighborPtr);
		      newPixel->idx0 = nextPixel->idx0;
		      newPixel->idx1 = nextPixel->idx1 + 1;
		      *outNeighborPtr = 1;  // visited don't know
		      }
		    }
		  }
		// Corners
		if (squareNeighborhood)
		  {
		  // upper left
		  if (nextPixel->idx0 > outExt[0] && nextPixel->idx1 > outExt[2])
		    {
		    inNeighborPtr = (T *)(nextPixel->inPtr) - inInc0 - inInc1;
		    if ( *inNeighborPtr == islandValue)
		      {
		      outNeighborPtr = (T *)(nextPixel->outPtr) - outInc0 -outInc1;
		      if ( *outNeighborPtr == 2)
			{
			// This is part of a bigger island.
			keepValue = 2;
			}
		      if ( *outNeighborPtr == 0)
			{
			// New pixel to add
			++newPixel;
			++numPixels;
			newPixel->inPtr = (void *)(inNeighborPtr);
			newPixel->outPtr = (void *)(outNeighborPtr);
			newPixel->idx0 = nextPixel->idx0 - 1;
			newPixel->idx1 = nextPixel->idx1 - 1;
			*outNeighborPtr = 1;  // visited don't know
			}
		      }
		    }
		  // upper right
		  if (nextPixel->idx0 < outExt[1] && nextPixel->idx1 > outExt[2])
		    {
		    inNeighborPtr = (T *)(nextPixel->inPtr) + inInc0 - inInc1;
		    if ( *inNeighborPtr == islandValue)
		      {
		      outNeighborPtr = (T *)(nextPixel->outPtr) + outInc0 -outInc1;
		      if ( *outNeighborPtr == 2)
			{
			// This is part of a bigger island.
			keepValue = 2;
			}
		      if ( *outNeighborPtr == 0)
			{
			// New pixel to add
			++newPixel;
			++numPixels;
			newPixel->inPtr = (void *)(inNeighborPtr);
			newPixel->outPtr = (void *)(outNeighborPtr);
			newPixel->idx0 = nextPixel->idx0 + 1;
			newPixel->idx1 = nextPixel->idx1 - 1;
			*outNeighborPtr = 1;  // visited don't know
			}
		      }
		    }
		  // lower left
		  if (nextPixel->idx0 > outExt[0] && nextPixel->idx1 < outExt[3])
		    {
		    inNeighborPtr = (T *)(nextPixel->inPtr) - inInc0 + inInc1;
		    if ( *inNeighborPtr == islandValue)
		      {
		      outNeighborPtr = (T *)(nextPixel->outPtr) - outInc0 +outInc1;
		      if ( *outNeighborPtr == 2)
			{
			// This is part of a bigger island.
			keepValue = 2;
			}
		      if ( *outNeighborPtr == 0)
			{
			// New pixel to add
			++newPixel;
			++numPixels;
			newPixel->inPtr = (void *)(inNeighborPtr);
			newPixel->outPtr = (void *)(outNeighborPtr);
			newPixel->idx0 = nextPixel->idx0 - 1;
			newPixel->idx1 = nextPixel->idx1 + 1;
			*outNeighborPtr = 1;  // visited don't know
			}
		      }
		    }
		  // lower right
		  if (nextPixel->idx0 < outExt[1] && nextPixel->idx1 < outExt[3])
		    {
		    inNeighborPtr = (T *)(nextPixel->inPtr) + inInc0 + inInc1;
		    if ( *inNeighborPtr == islandValue)
		      {
		      outNeighborPtr = (T *)(nextPixel->outPtr) + outInc0 +outInc1;
		      if ( *outNeighborPtr == 2)
			{
			// This is part of a bigger island.
			keepValue = 2;
			}
		      if ( *outNeighborPtr == 0)
			{
			// New pixel to add
			++newPixel;
			++numPixels;
			newPixel->inPtr = (void *)(inNeighborPtr);
			newPixel->outPtr = (void *)(outNeighborPtr);
			newPixel->idx0 = nextPixel->idx0 + 1;
			newPixel->idx1 = nextPixel->idx1 + 1;
			*outNeighborPtr = 1;  // visited don't know
			}
		      }
		    }
		  }
	      
		// Move to the next pixel to grow.
		++nextPixel;
		++nextPixelIdx;
	      
		// Have we visted enogh pixels to determine this is a keeper?
		if (keepValue == 1 && numPixels >= area)
		  {
		  keepValue = 2;
		  }
	      
		// Have we run out of pixels to grow?
		if (keepValue == 1 && nextPixelIdx >= numPixels)
		  {
		  // The island is too small. Set island values too replace.
		  keepValue = 3;
		  }
		}
	    
	      // Change "don't knows" to keep value
	      nextPixel = pixels;
	      for (nextPixelIdx = 0; nextPixelIdx < numPixels; ++nextPixelIdx)
		{
		*((T *)(nextPixel->outPtr)) = keepValue;
		++nextPixel;
		}
	      }
	    }
	
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
  
  delete [] pixels;
  
  // update the progress and possibly abort
  self->UpdateProgress(0.9);
  if (self->AbortExecute)
    {
    return;
    }

  // Loop though all pixels actually copying and replacing.
  for (idxC = 0; idxC < maxC; idxC++)
    {
    outPtr2 = outPtr + idxC;
    inPtr2 = inPtr + idxC;
    for (outIdx2 = outExt[4]; outIdx2 <= outExt[5]; ++outIdx2)
      {
      outPtr1 = outPtr2;
      inPtr1 = inPtr2;
      for (outIdx1 = outExt[2]; outIdx1 <= outExt[3]; ++outIdx1)
	{
	outPtr0 = outPtr1;
	inPtr0 = inPtr1;
	for (outIdx0 = outExt[0]; outIdx0 <= outExt[1]; ++outIdx0)
	  {
	  if (*outPtr0 == 3)
	    {
	    *outPtr0 = replaceValue;
	    }
	  else
	    {
	    *outPtr0 = *inPtr0;
	    }
	  inPtr0 += inInc0;
	  outPtr0 += outInc0;
	  }
	inPtr1 += inInc1;
	outPtr1 += outInc1;
	}
      inPtr2 += inInc2;
      outPtr2 += outInc2;
      }
    }
}

    
//----------------------------------------------------------------------------
// This method uses the input data to fill the output data.
// It can handle any type data, but the two datas must have the same 
// data type.  Assumes that in and out have the same lower extent.
void vtkImageIslandRemoval2D::Execute(vtkImageData *inData, 
				      vtkImageData *outData)
{
  int *outExt;
  
  
  // this filter expects that input is the same type as output.
  if (inData->GetScalarType() != outData->GetScalarType())
    {
    vtkErrorMacro(<< "Execute: input ScalarType, " 
                  << vtkImageScalarTypeNameMacro(inData->GetScalarType())
                  << ", must match out ScalarType "
                  << vtkImageScalarTypeNameMacro(outData->GetScalarType()));
    return;
    }

  outExt = this->GetOutput()->GetUpdateExtent();
  void *inPtr = inData->GetScalarPointerForExtent(outExt);
  void *outPtr = outData->GetScalarPointerForExtent(outExt);
  
  switch (inData->GetScalarType())
    {
    case VTK_DOUBLE:
      vtkImageIslandRemoval2DExecute(this, 
			   inData, (double *)(inPtr), 
			   outData, (double *)(outPtr), outExt);
      break;
    case VTK_FLOAT:
      vtkImageIslandRemoval2DExecute(this, 
			   inData, (float *)(inPtr), 
			   outData, (float *)(outPtr), outExt);
      break;
    case VTK_LONG:
      vtkImageIslandRemoval2DExecute(this, 
			   inData, (long *)(inPtr), 
			   outData, (long *)(outPtr), outExt);
      break;
    case VTK_UNSIGNED_LONG:
      vtkImageIslandRemoval2DExecute(this, 
			   inData, (unsigned long *)(inPtr), 
			   outData, (unsigned long *)(outPtr), outExt);
      break;
    case VTK_INT:
      vtkImageIslandRemoval2DExecute(this, 
			   inData, (int *)(inPtr), 
			   outData, (int *)(outPtr), outExt);
      break;
    case VTK_UNSIGNED_INT:
      vtkImageIslandRemoval2DExecute(this, 
			   inData, (unsigned int *)(inPtr), 
			   outData, (unsigned int *)(outPtr), outExt);
      break;
    case VTK_SHORT:
      vtkImageIslandRemoval2DExecute(this, 
			   inData, (short *)(inPtr), 
			   outData, (short *)(outPtr), outExt);
      break;
    case VTK_UNSIGNED_SHORT:
      vtkImageIslandRemoval2DExecute(this, 
			   inData, (unsigned short *)(inPtr), 
			   outData, (unsigned short *)(outPtr), outExt);
      break;
    case VTK_CHAR:
      vtkImageIslandRemoval2DExecute(this, 
			   inData, (char *)(inPtr), 
			   outData, (char *)(outPtr), outExt);
      break;
    case VTK_UNSIGNED_CHAR:
      vtkImageIslandRemoval2DExecute(this, 
			   inData, (unsigned char *)(inPtr), 
			   outData, (unsigned char *)(outPtr), outExt);
      break;
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }  
}
















