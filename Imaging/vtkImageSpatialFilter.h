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
// .NAME vtkImageSpatialFilter - Filters that operate on pixel neighborhoods.
// .SECTION Description
// vtkImageSpatialFilter is a super class for filters that operate on an
// input neighborhood for each output pixel. It handels even sized
// neighborhoods, but their can be a half pixel shift associated with
// processing.  This superclass has some logic for handling boundaries.  It
// can split regions into boundary and non-boundary pieces and call different
// execute methods.


#ifndef __vtkImageSpatialFilter_h
#define __vtkImageSpatialFilter_h


#include "vtkImageToImageFilter.h"

class VTK_EXPORT vtkImageSpatialFilter : public vtkImageToImageFilter
{
public:
  static vtkImageSpatialFilter *New();
  const char *GetClassName() {return "vtkImageSpatialFilter";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the Kernel size.
  int *GetKernelSize() {return this->KernelSize;}
  
  // Description:
  // Get the Kernel middle.
  int *GetKernelMiddle() {return this->KernelMiddle;}

protected:
  vtkImageSpatialFilter();
  ~vtkImageSpatialFilter() {};
  vtkImageSpatialFilter(const vtkImageSpatialFilter&) {};
  void operator=(const vtkImageSpatialFilter&) {};

  int   KernelSize[3];
  int   KernelMiddle[3];      // Index of kernel origin
  int   Strides[3];      // Shrink factor
  int   HandleBoundaries;     // Output shrinks if boundaries aren't handled

  // Called by the superclass
  void ExecuteInformation();
  // Override this method if you have to.
  virtual void ExecuteInformation(vtkImageData *inData, vtkImageData *outData);

  void ComputeOutputWholeExtent(int extent[6], int handleBoundaries);
  void ComputeInputUpdateExtent(int extent[6], int wholeExtent[6]);

};

#endif










