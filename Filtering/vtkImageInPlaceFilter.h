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
// .NAME vtkImageInPlaceFilter - Filter that operates in place.
// .SECTION Description
// vtkImageInPlaceFilter is a filter super class that 
// operates directly on the input region.  The data is copied
// if the requested region has different extent than the input region
// or some other object is referencing the input region.  

// .SECTION See Also
// vtkImageToImageFilter vtImageMultipleInputFilter vtkImageTwoInputFilter
// vtkImageTwoOutputFilter


#ifndef __vtkImageInPlaceFilter_h
#define __vtkImageInPlaceFilter_h

#include "vtkImageToImageFilter.h"

class VTK_EXPORT vtkImageInPlaceFilter : public vtkImageToImageFilter
{
public:
  static vtkImageInPlaceFilter *New() {return new vtkImageInPlaceFilter;};
  const char *GetClassName() {return "vtkImageInPlaceFilter";};

  // Description:
  // This method is called by the cache.  It eventually calls the
  // Execute(vtkImageData *, vtkImageData *) method.  Information has
  // already been updated by this point, and outRegion is in local
  // coordinates.  This method will stream to get the input. Only the
  // UpdateExtent from output will get updated.
  virtual void InternalUpdate(vtkDataObject *data);

protected:
  vtkImageInPlaceFilter() {};
  ~vtkImageInPlaceFilter() {};

  virtual void RecursiveStreamUpdate(vtkImageData *outData,int splitAxis);
  void CopyData(vtkImageData *in, vtkImageData *out);

};

#endif







