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
// .NAME vtkImageShrink3D - Subsamples an image.
// .SECTION Description
// vtkImageShrink3D shrinks an image by sub sampling on a 
// uniform grid (integer multiples).  

#ifndef __vtkImageShrink3D_h
#define __vtkImageShrink3D_h


#include "vtkImageToImageFilter.h"

class VTK_EXPORT vtkImageShrink3D : public vtkImageToImageFilter
{
public:
  static vtkImageShrink3D *New();
  const char *GetClassName() {return "vtkImageShrink3D";};
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Set/Get the shrink factors
  vtkSetVector3Macro(ShrinkFactors,int);
  vtkGetVector3Macro(ShrinkFactors,int);

  // Description:
  // Set/Get the pixel to use as origin.
  vtkSetVector3Macro(Shift,int);
  vtkGetVector3Macro(Shift,int);

  // Description:
  // Choose Mean, Minimum, Maximum, Median or sub sampling.
  // The neighborhood operations are not centered on the sampled pixel.
  // This may cause a half pixel shift in your output image.
  // You can changed "Shift" to get around this.
  // vtkImageGaussianSmooth or vtkImageMean with strides.
  void SetAveraging(int);
  int GetAveraging() {return this->GetMean();};
  vtkBooleanMacro(Averaging,int);
  
  void SetMean(int);
  vtkGetMacro(Mean,int);
  vtkBooleanMacro(Mean,int);
  
  void SetMinimum(int);
  vtkGetMacro(Minimum,int);
  vtkBooleanMacro(Minimum,int);
  
  void SetMaximum(int);
  vtkGetMacro(Maximum,int);
  vtkBooleanMacro(Maximum,int);
  
  void SetMedian(int);
  vtkGetMacro(Median,int);
  vtkBooleanMacro(Median,int);
  
  
protected:
  vtkImageShrink3D();
  ~vtkImageShrink3D() {};
  vtkImageShrink3D(const vtkImageShrink3D&) {};
  void operator=(const vtkImageShrink3D&) {};

  int ShrinkFactors[3];
  int Shift[3];
  int Mean;
  int Minimum;
  int Maximum;
  int Median;

  void ExecuteInformation(vtkImageData *inData, vtkImageData *outData);
  void ComputeInputUpdateExtent(int inExt[6], int outExt[6]);
  void ExecuteInformation(){this->vtkImageToImageFilter::ExecuteInformation();};
  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData, 
		       int ext[6], int id);  
};

#endif



