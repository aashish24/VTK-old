/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to Matt Turek who developed this class.

Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkImageMapper - 2D image display
// .SECTION Description
// vtkImageMapper provides 2D image display support for vtk.
// It is a Mapper2D subclass that can be associated with an Actor2D
// and placed withint a RenderWindow or ImageWindow.

// .SECTION See Also
// vtkMapper2D vtkActor2D

#ifndef __vtkImageMapper_h
#define __vtkImageMapper_h

#include "vtkMapper2D.h"

class vtkWindow;
class vtkViewport;
class vtkActor2D;
#include "vtkImageData.h"

class VTK_EXPORT vtkImageMapper : public vtkMapper2D
{
public:
  vtkTypeMacro(vtkImageMapper,vtkMapper2D);
  static vtkImageMapper *New();
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the window value for window/level
  vtkSetMacro(ColorWindow, float);
  vtkGetMacro(ColorWindow, float);
  
  // Description:
  // Set/Get the level value for window/level
  vtkSetMacro(ColorLevel, float);
  vtkGetMacro(ColorLevel, float);

  // Description:
  // Set/Get the input for the image mapper.  
  vtkSetObjectMacro(Input, vtkImageData);
  vtkGetObjectMacro(Input,vtkImageData);

  // Description:
  // Set/Get the current slice number. The axis Z in ZSlice does not
  // neccessarily have any relation to the z axis of the data on disk.
  // It is simply the axis orthogonal to the x,y, display plane.
  // GetWholeZMax and Min are convinience methods for obtaining
  // the number of slices that can be displayed. Again the number
  // of slices is in reference to the display z axis, which is not
  // neccessarily the z axis on disk. (ue to reformating etc)
  vtkSetMacro(ZSlice,int);
  vtkGetMacro(ZSlice,int);
  int GetWholeZMin();
  int GetWholeZMax();

  // Description:
  // Draw the image to the screen.
  void RenderStart(vtkViewport* viewport, vtkActor2D* actor);

  // Description:
  // Function called by Render to actually draw the image to to the screen
  virtual void RenderData(vtkViewport* , vtkImageData *, vtkActor2D* )=0; 

  // Description:
  // Methods used internally for performing the Window/Level mapping.
  float GetColorShift();
  float GetColorScale();

  // Public for templated functions.
  int DisplayExtent[6];

protected:
  vtkImageMapper();
  ~vtkImageMapper();
  vtkImageMapper(const vtkImageMapper&) {};
  void operator=(const vtkImageMapper&) {};

  vtkImageData* Input;
  float ColorWindow;
  float ColorLevel;
 
  int PositionAdjustment[2];
  int ZSlice;
};



#endif


