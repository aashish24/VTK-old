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
// .NAME vtkImageDilateErode3D - Dilates one value and erodes another.
// .SECTION Description
// vtkImageDilateErode3D will dilate one value and erode another.
// It uses an eliptical foot print, and only erodes/dilates on the
// boundary of the two values.  The filter is restricted to the 
// X, Y, and Z axes for now.  It can degenerate to a 2 or 1 dimensional
// filter by setting the kernal size to 1 for a specific axis.


#ifndef __vtkImageDilateErode3D_h
#define __vtkImageDilateErode3D_h


#include "vtkImageSpatialFilter.h"

class vtkImageEllipsoidSource;

class VTK_EXPORT vtkImageDilateErode3D : public vtkImageSpatialFilter
{
public:
  // Description:
  // Construct an instance of vtkImageDilateErode3D fitler.
  // By default zero values are dilated.
  static vtkImageDilateErode3D *New() 
    {return new vtkImageDilateErode3D;};
  const char *GetClassName() {return "vtkImageDilateErode3D";};
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // This method sets the size of the neighborhood.  It also sets the 
  // default middle of the neighborhood and computes the eliptical foot print.
  void SetKernelSize(int size0, int size1, int size2);

  
  // Description:
  // Set/Get the Dilate and Erode values to bue used by this filter.
  vtkSetMacro(DilateValue, float);
  vtkGetMacro(DilateValue, float);
  vtkSetMacro(ErodeValue, float);
  vtkGetMacro(ErodeValue, float);

protected:
  vtkImageDilateErode3D();
  ~vtkImageDilateErode3D();
  vtkImageDilateErode3D(const vtkImageDilateErode3D&) {};
  void operator=(const vtkImageDilateErode3D&) {};

  vtkImageEllipsoidSource *Ellipse;
  float DilateValue;
  float ErodeValue;
    
  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData, 
		       int extent[6], int id);
};

#endif

