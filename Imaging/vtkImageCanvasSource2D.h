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
// .NAME vtkImageCanvasSource2D - Paints on a canvas
// .SECTION Description
// vtkImageCanvasSource2D is a source that starts as a blank image.
// you may add to the image with two-dimensional drawing routines.
// It can paint multi-spectral images.


#ifndef __vtkImageCanvasSource2D_h
#define __vtkImageCanvasSource2D_h

#include <math.h>
#include "vtkImageData.h"

//
// Special classes for manipulating data
//
//BTX - begin tcl exclude
//
// For the fill functionality (use connector ??)
class vtkImageCanvasSource2DPixel { //;prevent man page generation
public:
  static vtkImageCanvasSource2DPixel *New() 
    {return new vtkImageCanvasSource2DPixel;};
  int X;
  int Y;
  void *Pointer;
  vtkImageCanvasSource2DPixel *Next;
};
//ETX - end tcl exclude
//


class VTK_EXPORT vtkImageCanvasSource2D : public vtkImageData
{
public:

  // Description:
  // Construct an instance of vtkImageCanvasSource2D with no data.
  vtkImageCanvasSource2D();


  // Description:
  // Destructor: Deleting a vtkImageCanvasSource2D automatically deletes the
  // associated vtkImageData.  However, since the data is reference counted,
  // it may not actually be deleted.
  ~vtkImageCanvasSource2D();

  static vtkImageCanvasSource2D *New() {return new vtkImageCanvasSource2D;};
  const char *GetClassName() {return "vtkImageCanvasSource2D";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // To drawing into a different image, set it with this method.
  vtkSetObjectMacro(ImageData, vtkImageData);
  vtkGetObjectMacro(ImageData, vtkImageData);
  
  // Description:
  // Set/Get DrawValue.  This is the value that is used when filling datas
  // or drawing lines.
  void SetDrawColor(int dim, float *color);
  void GetDrawColor(int dim, float *color);
  vtkSetVector4Macro(DrawColor, float);
  vtkGetVector4Macro(DrawColor, float);
  void SetDrawColor(float a) {this->SetDrawColor(a, 0.0, 0.0, 0.0);}
  void SetDrawColor(float a,float b) {this->SetDrawColor(a, b, 0.0, 0.0);}
  void SetDrawColor(float a, float b, float c) {this->SetDrawColor(a, b, c, 0.0);}
    
  void FillBox(int min0, int max0, int min1, int max1);
  void FillTube(int x0, int y0, int x1, int y1, float radius);
  void FillTriangle(int x0, int y0, int x1, int y1, int x2, int y2);
  void DrawCircle(int c0, int c1, float radius);
  void DrawPoint(int p0, int p1);
  void DrawSegment(int x0, int y0, int x1, int y1);
  void DrawSegment3D(float *p0, float *p1);


  // Description:
  // Fill a colored area with another color. (like connectivity)
  // All pixels connected to pixel (x, y) get replaced by draw color.
  void FillPixel(int x, int y);

  // Description:
  // To make Canvas source more like other sources, this get output
  // method should be used.
  vtkImageData *GetOutput() {return this;}
  
protected:
  vtkImageData *ImageData;
  float DrawColor[4];
  
  int ClipSegment(int &a0, int &a1, int &b0, int &b1);
};



#endif


