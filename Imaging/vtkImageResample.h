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
// .NAME vtkImageResample - Resamples an image to be larger or smaller.
// .SECTION Description
// This filter produces an output with different spacing (and extent)
// than the input.  Linear interpolation can be used to resample the data.
// The Output spacing can be set explicitly or relative to input spacing
// with the SetAxisMagnificationFactor method.


#ifndef __vtkImageResample_h
#define __vtkImageResample_h


#include "vtkImageToImageFilter.h"

class VTK_EXPORT vtkImageResample : public vtkImageToImageFilter
{
public:
  static vtkImageResample *New();
  const char *GetClassName() {return "vtkImageResample";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set desired spacing.  
  // Zero is a reserved value indicating spacing has not been set.
  void SetAxisOutputSpacing(int axis, float spacing);
  
  // Description:
  // Set/Get Magnification factors.
  // Zero is a reserved value indicating values have not been computed.
  void SetAxisMagnificationFactor(int axis, float factor);
  float GetAxisMagnificationFactor(int axis);

  // Description:
  // Turn interpolation on and off (pixel replication is used when off).
  vtkSetMacro(Interpolate,int);
  vtkGetMacro(Interpolate,int);
  vtkBooleanMacro(Interpolate,int);
  
  // Description:
  // Dimensionality is the number of axes which are considered durring
  // execution. To process images dimensionality would be set to 2.
  // This has the same effect as setting the output spacing of the third
  // axis to 1.0
  vtkSetMacro(Dimensionality,int);
  vtkGetMacro(Dimensionality,int);

protected:
  vtkImageResample();
  ~vtkImageResample() {};
  vtkImageResample(const vtkImageResample&) {};
  void operator=(const vtkImageResample&) {};

  float MagnificationFactors[3];
  float OutputSpacing[3];
  int Interpolate;
  int Dimensionality;
  
  void ComputeRequiredInputUpdateExtent(int inExt[6], int outExt[6]);
  void ExecuteInformation(vtkImageData *inData, vtkImageData *outData);
  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData,
		       int extent[6], int id);
};

#endif
