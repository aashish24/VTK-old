/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to C. Charles Law who developed this class.

Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen.

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
// .NAME vtkImageHSVToRGB - Converts HSV components to RGB.
// .SECTION Description
// For each pixel with hue, saturation and value components this filter
// outputs the color coded as red, green, blue.  Output type must be the same
// as input type.

// .SECTION See Also
// vtkImageRGBToHSV

#ifndef __vtkImageHSVToRGB_h
#define __vtkImageHSVToRGB_h


#include "vtkImageToImageFilter.h"

class VTK_EXPORT vtkImageHSVToRGB : public vtkImageToImageFilter
{
public:
  static vtkImageHSVToRGB *New();
  vtkTypeMacro(vtkImageHSVToRGB,vtkImageToImageFilter);

  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Hue is an angle. Maximum specifies when it maps back to 0.
  // HueMaximum defaults to 255 instead of 2PI, because unsigned char
  // is expected as input.
  // Maximum also specifies the maximum of the Saturation, and R, G, B.
  vtkSetMacro(Maximum,float);
  vtkGetMacro(Maximum,float);
  
protected:
  vtkImageHSVToRGB();
  ~vtkImageHSVToRGB() {};
  vtkImageHSVToRGB(const vtkImageHSVToRGB&) {};
  void operator=(const vtkImageHSVToRGB&) {};

  float Maximum;
  
  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData,
		       int ext[6], int id);
};

#endif



