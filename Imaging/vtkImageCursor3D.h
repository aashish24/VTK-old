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
// .NAME vtkImageCursor3D - Paints a cursor on top of an image or volume.
// .SECTION Description
// vtkImageCursor3D will draw a cursor on a 2d image or 3d volume.

#ifndef __vtkImageCursor3D_h
#define __vtkImageCursor3D_h

#include "vtkImageInPlaceFilter.h"

class VTK_EXPORT vtkImageCursor3D : public vtkImageInPlaceFilter
{
public:
  static vtkImageCursor3D *New() {return new vtkImageCursor3D;};
  const char *GetClassName() {return "vtkImageCursor3D";};
  void PrintSelf(ostream& os, vtkIndent indent);


  // Description:
  // Sets/Gets the center point of the 3d cursor.
  vtkSetVector3Macro(CursorPosition, float);
  vtkGetVector3Macro(CursorPosition, float);

  // Description:
  // Sets/Gets what pixel value to draw the cursor in.
  vtkSetMacro(CursorValue, float);
  vtkGetMacro(CursorValue, float);
  
  // Description:
  // Sets/Gets the radius of the cursor. The radius determines
  // how far the axis lines project out from the cursors center.
  vtkSetMacro(CursorRadius, int);
  vtkGetMacro(CursorRadius, int);
  
  
protected:
  vtkImageCursor3D();
  ~vtkImageCursor3D() {};
  vtkImageCursor3D(const vtkImageCursor3D&) {};
  void operator=(const vtkImageCursor3D&) {};

  float CursorPosition[3];
  float CursorValue;
  int CursorRadius;
  
  // not threaded because it's too simple a filter
  void Execute(vtkImageData *inData, vtkImageData *outData);
};



#endif



