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
// .NAME vtkImageNonMaximumSuppression - Performs non-maximum suppression
// .SECTION Description
// vtkImageNonMaximumSuppression Sets to zero any pixel that is not a peak.
// If a pixel has a neighbor along the vector that has larger magnitude, the
// smaller pixel is set to zero.  The filter takes two inputs: a magnitude
// and a vector.  Output is magnitude information and is always in floats.
// Typically this filter is used with vtkImageGradient and
// vtkImageGradientMagnitude as inputs.


#ifndef __vtkImageNonMaximumSuppression_h
#define __vtkImageNonMaximumSuppression_h

#define VTK_IMAGE_NON_MAXIMUM_SUPPRESSION_MAGNITUDE_INPUT 0
#define VTK_IMAGE_NON_MAXIMUM_SUPPRESSION_VECTOR_INPUT 1

#include "vtkImageTwoInputFilter.h"

class VTK_EXPORT vtkImageNonMaximumSuppression : public vtkImageTwoInputFilter
{
public:
  static vtkImageNonMaximumSuppression *New();
  const char *GetClassName() {return "vtkImageNonMaximumSuppression";};
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Set the magnitude and vector inputs.
  void SetMagnitudeInput(vtkImageData *input) {this->SetInput1(input);};
  void SetVectorInput(vtkImageData *input) {this->SetInput2(input);};
  
  // Description:
  // If "HandleBoundariesOn" then boundary pixels are duplicated
  // So central differences can get values.
  vtkSetMacro(HandleBoundaries, int);
  vtkGetMacro(HandleBoundaries, int);
  vtkBooleanMacro(HandleBoundaries, int);

  // Description:
  // Determines how the input is interpreted (set of 2d slices or a 3D volume)
  vtkSetClampMacro(Dimensionality,int,2,3);
  vtkGetMacro(Dimensionality,int);
  
protected:
  vtkImageNonMaximumSuppression();
  ~vtkImageNonMaximumSuppression() {};
  vtkImageNonMaximumSuppression(const vtkImageNonMaximumSuppression&) {};
  void operator=(const vtkImageNonMaximumSuppression&) {};

  int HandleBoundaries;
  int Dimensionality;
  
  void ExecuteInformation(vtkImageData **inDatas, vtkImageData *outData);
  virtual void ComputeInputUpdateExtent(int inExt[6], int outExt[6],
					int whichInput);
  void ExecuteInformation(){this->vtkImageTwoInputFilter::ExecuteInformation();};
  void ThreadedExecute(vtkImageData **inDatas, vtkImageData *outData,
		       int extent[6], int id);
  
};

#endif



