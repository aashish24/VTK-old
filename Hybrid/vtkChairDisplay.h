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
PARTICULAR PURPOSE, AND -INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
// .NAME vtkChairDisplay - generate isosurface(s) from volume/images
// .SECTION Description


#ifndef __vtkChairDisplay_h
#define __vtkChairDisplay_h

#include "vtkImageData.h"
#include "vtkPolyDataSource.h"

class VTK_EXPORT vtkChairDisplay : public vtkPolyDataSource
{
public:
  vtkChairDisplay();
  ~vtkChairDisplay();
  static vtkChairDisplay *New() {return new vtkChairDisplay;};
  const char *GetClassName() {return "vtkChairDisplay";};
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Set/Get the source for the scalar data to contour.
  void SetInput(vtkImageData *input);
  vtkImageData *GetInput();
  
  // Description:
  // Set/Get the size of the notch.
  vtkSetMacro(XNotchSize, int);
  vtkGetMacro(XNotchSize, int);
  vtkSetMacro(YNotchSize, int);
  vtkGetMacro(YNotchSize, int);
  vtkSetMacro(ZNotchSize, int);
  vtkGetMacro(ZNotchSize, int);

  void Update();
  
  vtkStructuredPoints *GetTextureOutput() {return this->TextureOutput;};

  void GenerateTexture(vtkImageData *inData, vtkScalars *scalars,
                       int xstart, int ystart,int xsize, int ysize, int p2x);
  
protected:
  vtkScalars *Scalars;
  vtkStructuredPoints *TextureOutput;
  
  int MaxYZSize;

  int XNotchSize;
  int YNotchSize;
  int ZNotchSize;
  
  void Execute(int recomputeTexture);
  void GeneratePolyData(int *, float *, float *, int, int,
			vtkCellArray *, vtkPoints *, vtkTCoords *);
};

#endif



