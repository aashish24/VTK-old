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
#include "vtkImageWrapPad.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkImageWrapPad, "$Revision$");
vtkStandardNewMacro(vtkImageWrapPad);

//----------------------------------------------------------------------------
// Just clip the request.
void vtkImageWrapPad::ComputeInputUpdateExtent(int inExt[6],
                                               int outExt[6])
{
  int idx;
  int min, max, width, imageMin, imageMax, imageWidth;
  int *wholeExtent;
  
  wholeExtent = this->GetInput()->GetWholeExtent();

  // Clip
  for (idx = 0; idx < 3; ++idx)
    {
    min = outExt[idx * 2];
    max = outExt[idx * 2 + 1];
    width = max - min + 1;
    imageMin = wholeExtent[idx * 2];
    imageMax = wholeExtent[idx * 2 + 1];
    imageWidth = imageMax - imageMin + 1;
    
    // convert min max to image extent range.
    min = ((min - imageMin) % imageWidth);
    if (min < 0)
      { // Mod does not handle negative numbers as I think it should.
      min += imageWidth;
      }
    min += imageMin;
    max = min + width - 1;
    // if request region wraps, we need the whole input 
    // (unless we make multiple requests! Write Update instead??)
    if (max > imageMax)
      {
      max = imageMax;
      min = imageMin;
      }
    
    inExt[idx * 2] = min;
    inExt[idx * 2 + 1] = max;
    }
}




//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
template <class T>
static void vtkImageWrapPadExecute(vtkImageWrapPad *self,
                                   vtkImageData *inData, T *vtkNotUsed(inPtr),
                                   vtkImageData *outData, T *outPtr,
                                   int outExt[6], int id)
{
  int min0, max0;
  int imageMin0, imageMax0, imageMin1, imageMax1, 
    imageMin2, imageMax2;
  int outIdx0, outIdx1, outIdx2;
  int start0, start1, start2;
  int inIdx0, inIdx1, inIdx2;
  int inInc0, inInc1, inInc2;
  int outIncX, outIncY, outIncZ;
  T *inPtr0, *inPtr1, *inPtr2;
  unsigned long count = 0;
  unsigned long target;
  int inMaxC, idxC, maxC;
  
  // Get information to march through data 
  inData->GetIncrements(inInc0, inInc1, inInc2);
  self->GetInput()->GetWholeExtent(imageMin0, imageMax0, imageMin1, imageMax1, 
                                   imageMin2, imageMax2);
  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);
  
  // initialize pointers to coresponding pixels.
  start0 = ((outExt[0] - imageMin0) % (imageMax0-imageMin0+1)) + imageMin0;
  if (start0 < 0)
    {
    start0 += (imageMax0-imageMin0+1);
    }
  start1 = ((outExt[2] - imageMin1) % (imageMax1-imageMin1+1)) + imageMin1;
  if (start1 < 0)
    {
    start1 += (imageMax1-imageMin1+1);
    }
  start2 = ((outExt[4] - imageMin2) % (imageMax2-imageMin2+1)) + imageMin2;
  if (start2 < 0)
    {
    start2 += (imageMax2-imageMin2+1);
    }
  inPtr2 = (T *)(inData->GetScalarPointer(start0, start1, start2));
  
  min0 = outExt[0];
  max0 = outExt[1];
  inMaxC = inData->GetNumberOfScalarComponents();
  maxC = outData->GetNumberOfScalarComponents();
  target = (unsigned long)((outExt[5]-outExt[4]+1)*
                           (outExt[3]-outExt[2]+1)/50.0);
  target++;
  
  inIdx2 = start2;
  for (outIdx2 = outExt[4]; outIdx2 <= outExt[5]; ++outIdx2, ++inIdx2)
    {
    if (inIdx2 > imageMax2) 
      { // we need to wrap(rewind) the input on this axis
      inIdx2 = imageMin2;
      inPtr2 -= (imageMax2-imageMin2+1)*inInc2;
      }
    inPtr1 = inPtr2;
    inIdx1 = start1;
    for (outIdx1 = outExt[2]; 
         !self->AbortExecute && outIdx1 <= outExt[3]; ++outIdx1, ++inIdx1)
      {
      if (!id) 
        {
        if (!(count%target))
          {
          self->UpdateProgress(count/(50.0*target));
          }
        count++;
        }
      if (inIdx1 > imageMax1) 
        { // we need to wrap(rewind) the input on this axis
        inIdx1 = imageMin1;
        inPtr1 -= (imageMax1-imageMin1+1)*inInc1;
        }
      inPtr0 = inPtr1;
      inIdx0 = start0;
      // if components are same much faster
      if ((maxC == inMaxC) && (maxC == 1))
        {
        for (outIdx0 = min0; outIdx0 <= max0; ++outIdx0, ++inIdx0)
          {
          if (inIdx0 > imageMax0) 
            { // we need to wrap(rewind) the input on this axis
            inIdx0 = imageMin0;
            inPtr0 -= (imageMax0-imageMin0+1)*inInc0;
            }
          // Copy Pixel
          *outPtr = *inPtr0;
          outPtr++; inPtr0++;
          }
        }
      else
        {
        for (outIdx0 = min0; outIdx0 <= max0; ++outIdx0, ++inIdx0)
          {
          if (inIdx0 > imageMax0) 
            { // we need to wrap(rewind) the input on this axis
            inIdx0 = imageMin0;
            inPtr0 -= (imageMax0-imageMin0+1)*inInc0;
            }   
          for (idxC = 0; idxC < maxC; idxC++)
            {
            // Copy Pixel
            *outPtr = inPtr0[idxC%inMaxC];
            outPtr++;
            }
          inPtr0 += inInc0;
          }
        }
      outPtr += outIncY;
      inPtr1 += inInc1;
      }
    outPtr += outIncZ;
    inPtr2 += inInc2;
    }
}



//----------------------------------------------------------------------------
// This method is passed a input and output data, and executes the filter
// algorithm to fill the output from the input.
// It just executes a switch statement to call the correct function for
// the regions data types.
void vtkImageWrapPad::ThreadedExecute(vtkImageData *inData, 
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
    vtkTemplateMacro7(vtkImageWrapPadExecute, this, inData, (VTK_TT *)inPtr, 
                      outData, (VTK_TT *)(outPtr), outExt, id);
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }
}












