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
// .NAME vtkImagePadFilter - Super class for filters that fill in extra pixels.
// .SECTION Description
// vtkImagePadFilter Changes the image extent of an image.  If the image
// extent is larger than the input image extent, the extra pixels are
// filled by an alogorithm detemined by the subclass.
// The image extent of the output has to be specified.


#ifndef __vtkImagePadFilter_h
#define __vtkImagePadFilter_h

class vtkImageCache;
#include "vtkImageFilter.h"

class VTK_EXPORT vtkImagePadFilter : public vtkImageFilter
{
public:

// Description:
// Constructor sets default values
  vtkImagePadFilter();

  static vtkImagePadFilter *New() {return new vtkImagePadFilter;};
  const char *GetClassName() {return "vtkImagePadFilter";};

  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // The image extent of the output has to be set explicitely.
  void SetOutputWholeExtent(int extent[6]);
  void SetOutputWholeExtent(int minX, int maxX, int minY, int maxY, 
			    int minZ, int maxZ);
  void GetOutputWholeExtent(int extent[6]);
  int *GetOutputWholeExtent() {return this->OutputWholeExtent;}

  // Pad the scalar components as well.
  vtkSetMacro(OutputNumberOfScalarComponents, int);
  vtkGetMacro(OutputNumberOfScalarComponents, int);
  
protected:
  int OutputWholeExtent[6];
  int OutputNumberOfScalarComponents;

  void ExecuteImageInformation();
  void ComputeRequiredInputUpdateExtent(int inExt[6], int outExt[6]);
};

#endif



