/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

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
#include "vtkImageShiftScaleFilter.hh"


//----------------------------------------------------------------------------
// Description:
// Constructor: Set defualt Scale and Shift values.
vtkImageShiftScaleFilter::vtkImageShiftScaleFilter()
{
  this->Scale = 1.0;
  this->Shift = 0.0;
}





//----------------------------------------------------------------------------
// Description:
// This method computes the Region of the input necessary to generate 
// out Region.  For this filter the two Regions are the same.
void vtkImageShiftScaleFilter::RequiredRegion(int *outOffset, int *outSize,
				  int *inOffset, int *inSize)
{
  int idx;
  
  for (idx = 0; idx < 3; ++idx)
    {
    inOffset[idx] = outOffset[idx];
    inSize[idx] = outSize[idx];
    }
}



//----------------------------------------------------------------------------
// Description:
// This method is passed a input and output tile, and executes the filter
// algorithm to fill the output from the input.
void vtkImageShiftScaleFilter::Execute(vtkImageRegion *inRegion, 
				       vtkImageRegion *outRegion)
{
  int *size;
  int size0, size1, size2;
  int idx0, idx1, idx2;
  int *inInc;
  int inInc0, inInc1, inInc2;
  int *outInc;
  int outInc0, outInc1, outInc2;
  float *inPtr0, *inPtr1, *inPtr2;
  float *outPtr0, *outPtr1, *outPtr2;
  
  
  // Get information to march through data 
  inPtr2 = inRegion->GetPointer(inRegion->GetOffset());
  inInc = inRegion->GetInc();
  inInc0 = inInc[0];  inInc1 = inInc[1];  inInc2 = inInc[2];  
  outPtr2 = outRegion->GetPointer(outRegion->GetOffset());
  outInc = outRegion->GetInc();
  outInc0 = outInc[0];  outInc1 = outInc[1];  outInc2 = outInc[2];  
  size = outRegion->GetSize();
  size0 = size[0];  size1 = size[1];  size2 = size[2];  
  
  vtkDebugMacro(<< "Execute: inRegion = " << inRegion 
                << ", outRegion = " << outRegion);
  
  // Loop through output pixels
  for (idx2 = 0; idx2 < size2; ++idx2)
    {
    outPtr1 = outPtr2;
    inPtr1 = inPtr2;
    for (idx1 = 0; idx1 < size1; ++idx1)
      {
      outPtr0 = outPtr1;
      inPtr0 = inPtr1;
      for (idx0 = 0; idx0 < size0; ++idx0)
	{
	
	// perform pixel operation
	*outPtr0 = (*inPtr0 + this->Shift) * this->Scale;
	
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




























