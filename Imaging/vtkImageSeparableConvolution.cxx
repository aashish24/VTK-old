/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkImageSeparableConvolution.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkImageSeparableConvolution, "$Revision$");
vtkStandardNewMacro(vtkImageSeparableConvolution);

// Actually do the convolution
void ExecuteConvolve ( float* kernel, int kernelSize, float* image, float* outImage, int imageSize )
{

  // Consider the kernel to be centered at (int) ( (kernelSize - 1 ) / 2.0 )
  
  int center = (int) ( (kernelSize - 1 ) / 2.0 );
  int i, j, k, kStart, iStart, iEnd, count;
  
  for ( i = 0; i < imageSize; ++i )
    {
    outImage[i] = 0.0;

    iStart = i - center;
    if ( iStart < 0 )
      {
      iStart = 0;
      }
    
    iEnd = i + center;
    if ( iEnd > imageSize - 1 )
      {
      iEnd = imageSize - 1;
      }

    // Handle padding
    iStart = i - center;
    k = kernelSize - 1;
    while ( iStart < 0 )
      {
      outImage[i] += image[0] * kernel[k];
      ++iStart;
      --k;
      }
    
    iEnd = i + center;
    k = 0;
    while ( iEnd > imageSize - 1 )
      {
      outImage[i] += image[imageSize - 1] * kernel[k];
      ++k;
      --iEnd;
      }


    kStart = center + i;
    if ( kStart > kernelSize - 1 )
      {
      kStart = kernelSize - 1;
      }
    count = iEnd - iStart + 1;
    for ( j = 0; j < count; ++j )
      {
      outImage[i] += image[j+iStart] * kernel[kStart-j];
      }
    }
}

// Description:
// Overload standard modified time function. If kernel arrays are modified,
// then this object is modified as well.
unsigned long vtkImageSeparableConvolution::GetMTime()
{
  unsigned long mTime=this->vtkImageDecomposeFilter::GetMTime();
  unsigned long kTime;

  if ( this->XKernel )
    {
    kTime = this->XKernel->GetMTime();
    mTime = kTime > mTime ? kTime : mTime;
    }
  if ( this->YKernel )
    {
    kTime = this->YKernel->GetMTime();
    mTime = kTime > mTime ? kTime : mTime;
    }
  if ( this->YKernel )
    {
    kTime = this->YKernel->GetMTime();
    mTime = kTime > mTime ? kTime : mTime;
    }
  return mTime;
}


//----------------------------------------------------------------------------
vtkImageSeparableConvolution::~vtkImageSeparableConvolution()
{
  SetXKernel ( NULL );
  SetYKernel ( NULL );
  SetZKernel ( NULL );
}

//----------------------------------------------------------------------------
vtkImageSeparableConvolution::vtkImageSeparableConvolution()
{
  XKernel = YKernel = ZKernel = NULL;
}

//----------------------------------------------------------------------------
// This extent of the components changes to real and imaginary values.
void vtkImageSeparableConvolution::ExecuteInformation(vtkImageData *, 
                                                      vtkImageData *output)
{
  output->SetNumberOfScalarComponents(1);
  output->SetScalarType(VTK_FLOAT);
}

//----------------------------------------------------------------------------
// This method tells the superclass that the whole input array is needed
// to compute any output region.

void vtkImageSeparableConvolution::ComputeInputUpdateExtent(int inExt[6],
                                                            int outExt[6])
{
  int *wholeExtent;

  if ( ! this->GetInput())
    {
    vtkErrorMacro(<< "Input not set.");
    return;
    }

  vtkFloatArray* KernelArray = NULL;
  switch ( this->GetIteration() )
    {
    case 0:
      KernelArray = this->GetXKernel();
      break;
    case 1:
      KernelArray = this->GetYKernel();
      break;
    case 2:
      KernelArray = this->GetZKernel();
      break;
    }
  int kernelSize = 0;
  if ( KernelArray )
    {
    kernelSize = KernelArray->GetNumberOfTuples();
    kernelSize = (int) ( ( kernelSize - 1 ) / 2.0 ); 
    }
  
  
  
  // Assumes that the input update extent has been initialized to output ...
  memcpy(inExt, outExt, 6 * sizeof(int));
  wholeExtent = this->GetInput()->GetWholeExtent();
  inExt[this->Iteration * 2] = outExt[this->Iteration * 2] - kernelSize;
  if ( inExt[this->Iteration * 2] < wholeExtent[this->Iteration * 2] )
    {
    inExt[this->Iteration * 2] = wholeExtent[this->Iteration * 2];
    }
  
  inExt[this->Iteration * 2 + 1] = outExt[this->Iteration * 2 + 1] + kernelSize;
  if ( inExt[this->Iteration * 2 + 1] > wholeExtent[this->Iteration * 2 + 1] )
    {
    inExt[this->Iteration * 2 + 1] = wholeExtent[this->Iteration * 2 + 1];
    }
}

template <class T>
static void vtkImageSeparableConvolutionExecute ( vtkImageSeparableConvolution* self,
                                                  vtkImageData* inData,
                                                  vtkImageData* outData,
                                                  T* vtkNotUsed ( dummy ) )
{
  T *inPtr0, *inPtr1, *inPtr2;
  float *outPtr0, *outPtr1, *outPtr2;
  int inInc0, inInc1, inInc2;
  int outInc0, outInc1, outInc2;
  int inMin0, inMax0, inMin1, inMax1, inMin2, inMax2;
  int outMin0, outMax0, outMin1, outMax1, outMin2, outMax2;
  int idx0, idx1, idx2;
  int outExt[6], inExt[6];
  int i;
  unsigned long count = 0;
  unsigned long target;

  inData->GetUpdateExtent ( inExt );
  outData->GetUpdateExtent ( outExt );

  // Reorder axes (the in and out extents are assumed to be the same)
  // (see intercept cache update)
  self->PermuteExtent(outExt, outMin0, outMax0, outMin1, outMax1, outMin2, outMax2);
  self->PermuteExtent(inExt, inMin0, inMax0, inMin1, inMax1, inMin2, inMax2);
  self->PermuteIncrements(inData->GetIncrements(), inInc0, inInc1, inInc2);
  self->PermuteIncrements(outData->GetIncrements(), outInc0, outInc1, outInc2);
  
  target = (unsigned long)((inMax2-inMin2+1)*(inMax1-inMin1+1)/50.0);
  target++;

  vtkFloatArray* KernelArray = NULL;
  switch ( self->GetIteration() )
    {
    case 0:
      KernelArray = self->GetXKernel();
      break;
    case 1:
      KernelArray = self->GetYKernel();
      break;
    case 2:
      KernelArray = self->GetZKernel();
      break;
    }
  int kernelSize = 0;
  float* kernel = NULL;

  if ( KernelArray )
    {
    // Allocate the arrays
    kernelSize = KernelArray->GetNumberOfTuples();
    kernel = new float[kernelSize];
    // Copy the kernel
    for ( i = 0; i < kernelSize; i++ )
      {
      kernel[i] = KernelArray->GetValue ( i );
      }
    }

  int imageSize = inMax0 + 1;
  float* image = new float[imageSize];
  float* outImage = new float[imageSize];
  float* imagePtr = NULL;

  
  // loop over all the extra axes
  inPtr2 = (T *)inData->GetScalarPointerForExtent(inExt);
  outPtr2 = (float *)outData->GetScalarPointerForExtent(outExt);
  for (idx2 = inMin2; idx2 <= inMax2; ++idx2)
    {
    inPtr1 = inPtr2;
    outPtr1 = outPtr2;
    for (idx1 = inMin1; !self->AbortExecute && idx1 <= inMax1; ++idx1)
      {
      if (!(count%target))
        {
        self->UpdateProgress(count/(50.0*target));
        }
      count++;
      inPtr0 = inPtr1;
      imagePtr = image;
      for (idx0 = inMin0; idx0 <= inMax0; ++idx0)
        {
        *imagePtr = (float)(*inPtr0);
        inPtr0 += inInc0;
        ++imagePtr;
        }

      // Call the method that performs the convolution
      if ( kernel )
        {
        ExecuteConvolve ( kernel, kernelSize, image, outImage, imageSize );
        imagePtr = outImage;
        }
      else
        {
        // If we don't have a kernel, just copy to the output
        imagePtr = image;
        }
      
      // Copy to output, be aware that we only copy to the extent that was asked for
      outPtr0 = outPtr1;
      imagePtr = imagePtr + (outMin0 - inMin0);
      for (idx0 = outMin0; idx0 <= outMax0; ++idx0)
        {
        *outPtr0 = (*imagePtr);
        outPtr0 += outInc0;
        ++imagePtr;
        }
      inPtr1 += inInc1;
      outPtr1 += outInc1;
      }
    inPtr2 += inInc2;
    outPtr2 += outInc2;
    }
  
  delete [] image;
  delete [] outImage;
  if ( kernel )
    {
    delete [] kernel;
    }

}




//----------------------------------------------------------------------------
// This is writen as a 1D execute method, but is called several times.
void vtkImageSeparableConvolution::IterativeExecuteData(vtkImageData *inData, 
                                                        vtkImageData *outData)
{

  if ( XKernel )
    {
    // Check for a filter of odd length
    if ( 1 - ( XKernel->GetNumberOfTuples() % 2 ) )
      {
      vtkErrorMacro ( << "Execute:  XKernel must have odd length" );
      return;
      }
    }
  if ( YKernel )
    {
    // Check for a filter of odd length
    if ( 1 - ( YKernel->GetNumberOfTuples() % 2 ) )
      {
      vtkErrorMacro ( << "Execute:  YKernel must have odd length" );
      return;
      }
    }
  if ( ZKernel )
    {
    // Check for a filter of odd length
    if ( 1 - ( ZKernel->GetNumberOfTuples() % 2 ) )
      {
      vtkErrorMacro ( << "Execute:  ZKernel must have odd length" );
      return;
      }
    }
  
  if (inData->GetNumberOfScalarComponents() != 1)
    {
    vtkErrorMacro(<< "ImageSeparableConvolution only works on 1 component input for the moment.");
    return;
    }
  
  // this filter expects that the output be floats.
  if (outData->GetScalarType() != VTK_FLOAT)
    {
    vtkErrorMacro(<< "Execute: Output must be be type float.");
    return;
    }

  // choose which templated function to call.
  switch (inData->GetScalarType())
    {
    vtkTemplateMacro4(vtkImageSeparableConvolutionExecute, this, inData, outData, static_cast<VTK_TT*>(0) );
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }
}


void vtkImageSeparableConvolution::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if ( this->XKernel )
    {
    os << indent << "XKernel:\n";
    this->XKernel->PrintSelf ( os, indent.GetNextIndent() );
    }
  else
    {
    os << indent << "XKernel: (not defined)\n";
    }
  if ( this->YKernel )
    {
    os << indent << "YKernel:\n";
    this->YKernel->PrintSelf ( os, indent.GetNextIndent() );
    }
  else
    {
    os << indent << "YKernel: (not defined)\n";
    }
  if ( this->ZKernel )
    {
    os << indent << "ZKernel:\n";
    this->ZKernel->PrintSelf ( os, indent.GetNextIndent() );
    }
  else
    {
    os << indent << "ZKernel: (not defined)\n";
    }
}
