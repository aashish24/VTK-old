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
// .NAME vtkImageMedian3D - Median Filter
// .SECTION Description
// vtkImageMedian3D a Median filter that replaces each pixel with the 
// median value from a rectangular neighborhood around that pixel.
// Neighborhoods can be no more than 3 dimensional.  Setting one
// axis of the neighborhood kernelSize to 1 changes the filter
// into a 2D median.  


#ifndef __vtkImageMedian3D_h
#define __vtkImageMedian3D_h


#include "vtkImageSpatialFilter.h"

class VTK_EXPORT vtkImageMedian3D : public vtkImageSpatialFilter
{
public:
  static vtkImageMedian3D *New() {return new vtkImageMedian3D;};
  const char *GetClassName() {return "vtkImageMedian3D";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // This method sets the size of the neighborhood.  It also sets the 
  // default middle of the neighborhood 
  void SetKernelSize(int size0, int size1, int size2);

  // Description:
  // Return the number of elements in the median mask
  vtkGetMacro(NumberOfElements,int);
  
protected:
  vtkImageMedian3D();
  ~vtkImageMedian3D() {};
  vtkImageMedian3D(const vtkImageMedian3D&) {};
  void operator=(const vtkImageMedian3D&) {};

  int NumberOfElements;

  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData, 
		       int extent[6], int id);

};

#endif



