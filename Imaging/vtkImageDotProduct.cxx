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
#include "vtkImageRegion.h"
#include "vtkImageDotProduct.h"



//----------------------------------------------------------------------------
vtkImageDotProduct::vtkImageDotProduct()
{
  this->SetAxes(VTK_IMAGE_COMPONENT_AXIS);

  this->NumberOfExecutionAxes = 2;
}




//----------------------------------------------------------------------------
// Description:
// Colapse the first axis
void 
vtkImageDotProduct::ComputeOutputImageInformation(vtkImageRegion *inRegion1,
						  vtkImageRegion *inRegion2,
						  vtkImageRegion *outRegion)

{
  inRegion1 = inRegion2;
  outRegion->SetImageExtent(0, 0);
}



//----------------------------------------------------------------------------
// Description:
// This method computes the extent of the input region necessary to generate
// an output region.  
void vtkImageDotProduct::ComputeRequiredInputRegionExtent(
					       vtkImageRegion *outRegion,
					       vtkImageRegion *inRegion1,
					       vtkImageRegion *inRegion2)
{
  outRegion = outRegion;
  // They should both be the same.
  inRegion1->SetExtent(1, inRegion1->GetImageExtent());
  inRegion2->SetExtent(1, inRegion1->GetImageExtent());
}





//----------------------------------------------------------------------------
// Description:
// This templated function executes the filter for any type of data.
template <class T>
static void vtkImageDotProductExecute(vtkImageDotProduct *self,
			       vtkImageRegion *in1Region, T *in1Ptr,
			       vtkImageRegion *in2Region, T *in2Ptr,
			       vtkImageRegion *outRegion, T *outPtr)
{
  float dot;
  int min0, max0, min1, max1;
  int idx0, idx1;
  int in1Inc0, in1Inc1;
  int in2Inc0, in2Inc1;
  int outInc0, outInc1;
  T *in1Ptr0, *in1Ptr1;
  T *in2Ptr0, *in2Ptr1;
  T *outPtr1;

  self = self;
  // Get information to march through data 
  in1Region->GetIncrements(in1Inc0, in1Inc1);
  in2Region->GetIncrements(in2Inc0, in2Inc1);
  outRegion->GetIncrements(outInc0, outInc1);
  in1Region->GetExtent(min0, max0, min1, max1);
  
  // Loop through ouput pixels
  in1Ptr1 = in1Ptr;
  in2Ptr1 = in2Ptr;
  outPtr1 = outPtr;
  for (idx1 = min1; idx1 <= max1; ++idx1)
    {
    dot = 0.0;
    in1Ptr0 = in1Ptr1;
    in2Ptr0 = in2Ptr1;
    for (idx0 = min0; idx0 <= max0; ++idx0)
      {
      dot += (float)(*in1Ptr0 * *in2Ptr0);
      in1Ptr0 += in1Inc0;
      in2Ptr0 += in2Inc0;
      }
    *outPtr1 = (T)(dot);
    outPtr1 += outInc1;
    in1Ptr1 += in1Inc1;
    in2Ptr1 += in2Inc1;
    }
}



//----------------------------------------------------------------------------
// Description:
// This method is passed a input and output regions, and executes the filter
// algorithm to fill the output from the inputs.
// It just executes a switch statement to call the correct function for
// the regions data types.
void vtkImageDotProduct::Execute(vtkImageRegion *inRegion1, 
				 vtkImageRegion *inRegion2, 
				 vtkImageRegion *outRegion)
{
  void *inPtr1 = inRegion1->GetScalarPointer();
  void *inPtr2 = inRegion2->GetScalarPointer();
  void *outPtr = outRegion->GetScalarPointer();
  
  // this filter expects that inputs are the same type as output.
  if (inRegion1->GetScalarType() != outRegion->GetScalarType() ||
      inRegion2->GetScalarType() != outRegion->GetScalarType())
    {
    vtkErrorMacro(<< "Execute: input ScalarTypes, " 
        << inRegion1->GetScalarType() << " and " << inRegion2->GetScalarType()
        << ", must match out ScalarType " << outRegion->GetScalarType());
    return;
    }
  
  switch (inRegion1->GetScalarType())
    {
    case VTK_FLOAT:
      vtkImageDotProductExecute(this, 
			  inRegion1, (float *)(inPtr1), 
			  inRegion2, (float *)(inPtr2), 
			  outRegion, (float *)(outPtr));
      break;
    case VTK_INT:
      vtkImageDotProductExecute(this, 
			  inRegion1, (int *)(inPtr1), 
			  inRegion2, (int *)(inPtr2), 
			  outRegion, (int *)(outPtr));
      break;
    case VTK_SHORT:
      vtkImageDotProductExecute(this, 
			  inRegion1, (short *)(inPtr1), 
			  inRegion2, (short *)(inPtr2), 
			  outRegion, (short *)(outPtr));
      break;
    case VTK_UNSIGNED_SHORT:
      vtkImageDotProductExecute(this, 
			  inRegion1, (unsigned short *)(inPtr1), 
			  inRegion2, (unsigned short *)(inPtr2), 
			  outRegion, (unsigned short *)(outPtr));
      break;
    case VTK_UNSIGNED_CHAR:
      vtkImageDotProductExecute(this, 
			  inRegion1, (unsigned char *)(inPtr1), 
			  inRegion2, (unsigned char *)(inPtr2), 
			  outRegion, (unsigned char *)(outPtr));
      break;
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }
}
















