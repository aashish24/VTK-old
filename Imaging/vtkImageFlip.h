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
// .NAME vtkImageFlip - This flips an axis of an image. Right becomes left ...
// .SECTION Description
// vtkImageFlip will reflect the data along the filtered axis.
// If PreserveImageExtent is "On", then the 
// image is shifted so that it has the same image extent, and the origin is
// shifted appropriately. When PreserveImageExtent is "off",
// The Origin  is not changed, min and max of extent (of filtered axis) are
// negated, and are swapped. The default preserves the extent of the input.

#ifndef __vtkImageFlip_h
#define __vtkImageFlip_h


#include "vtkImageToImageFilter.h"

class VTK_EXPORT vtkImageFlip : public vtkImageToImageFilter
{
public:
  vtkImageFlip();
  static vtkImageFlip *New() {return new vtkImageFlip;};
  const char *GetClassName() {return "vtkImageFlip";};

  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify which axes will be flipped.
  vtkSetMacro(FilteredAxis, int);
  vtkGetMacro(FilteredAxis, int);

  // Description:
  // Specify which axes will be flipped.
  // For compatability with old scripts
  void SetFilteredAxes(int axis) {this->SetFilteredAxis(axis);}
  
  // Description:
  // If PreseveImageExtent is off, then extent of axis0 is simply
  // multiplied by -1.  If it is on, then the new image min (-imageMax0)
  // is shifted to old image min (imageMin0).
  vtkSetMacro(PreserveImageExtent, int);
  vtkGetMacro(PreserveImageExtent, int);
  vtkBooleanMacro(PreserveImageExtent, int);
  
protected:
  int FilteredAxis;
  int PreserveImageExtent;
  
  void ExecuteInformation(vtkImageData *inData, vtkImageData *outData);
  void ComputeRequiredInputUpdateExtent(int inExt[6], int outExt[6]);
  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData,
		       int outExt[6], int id);
};

#endif



