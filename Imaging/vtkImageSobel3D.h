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
// .NAME vtkImageSobel3D - Computes a vector field using sobel functions.
// .SECTION Description
// vtkImageSobel3D computes a vector field from a scalar field by using
// Sobel functions.  The number of vector components is 3 because
// the input is a volume.  Output is always floats.  A little creative 
// liberty was used to extend the 2D sobel kernels into 3D.




#ifndef __vtkImageSobel3D_h
#define __vtkImageSobel3D_h


#include "vtkImageSpatialFilter.h"

class VTK_EXPORT vtkImageSobel3D : public vtkImageSpatialFilter
{
public:
  static vtkImageSobel3D *New() {return new vtkImageSobel3D;};
  const char *GetClassName() {return "vtkImageSobel3D";};
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkImageSobel3D();
  ~vtkImageSobel3D() {};

  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData,
		       int outExt[6], int id);
  void ExecuteInformation(vtkImageData *inData, vtkImageData *outData);
};

#endif



