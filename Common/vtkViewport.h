/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1998 Ken Martin, Will Schroeder, Bill Lorensen.

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
// .NAME vtkViewport - abstract specification for Viewports
// .SECTION Description
// vtkViewport provides an abstract specification for Viewports. A Viewport
// is an object that controls the rendering process for objects. Rendering
// is the process of converting geometry, a specification for lights, and 
// a camera view into an image. vtkViewport also performs coordinate 
// transformation between world coordinates, view coordinates (the computer
// graphics rendering coordinate system), and display coordinates (the 
// actual screen coordinates on the display device). Certain advanced 
// rendering features such as two-sided lighting can also be controlled.

// .SECTION See Also
// vtkRenderWindow vtkActor vtkCamera vtkLight vtkVolume vtkRayCaster

#ifndef __vtkViewport_h
#define __vtkViewport_h

#include "vtkObject.h"

class vtkWindow;

class VTK_EXPORT vtkViewport : public vtkObject
{
public:
  vtkViewport();
  const char *GetClassName() {return "vtkViewport";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the background color of the rendering screen using an rgb color
  // specification.
  vtkSetVector3Macro(Background,float);
  vtkGetVectorMacro(Background,float,3);

  // Description:
  // Set the aspect ratio of the rendered image. This is computed 
  // automatically and should not be set by the user.
  vtkSetVector2Macro(Aspect,float);
  vtkGetVectorMacro(Aspect,float,2);

  // Description:
  // Specify the viewport for the Viewport to draw in the rendering window. 
  // Coordinates are expressed as (xmin,ymin,xmax,ymax), where each
  // coordinate is 0 <= coordinate <= 1.0.
  vtkSetVector4Macro(Viewport,float);
  vtkGetVectorMacro(Viewport,float,4);

  virtual float *GetCenter();
  virtual int    IsInViewport(int x,int y); 

  virtual vtkWindow *GetVTKWindow() = 0;
  
  void SetStartRenderMethod(void (*f)(void *), void *arg);
  void SetEndRenderMethod(void (*f)(void *), void *arg);
  void SetStartRenderMethodArgDelete(void (*f)(void *));
  void SetEndRenderMethodArgDelete(void (*f)(void *));

protected:
  vtkWindow *VTKWindow;
  float Background[3];  
  float Viewport[4];
  float Aspect[2];
  float Center[2];
  void (*StartRenderMethod)(void *);
  void (*StartRenderMethodArgDelete)(void *);
  void *StartRenderMethodArg;
  void (*EndRenderMethod)(void *);
  void (*EndRenderMethodArgDelete)(void *);
  void *EndRenderMethodArg;
};

// Description:

#endif
