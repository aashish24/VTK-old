/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// .NAME vtkOpenGLRayCastImageDisplayHelper - OpenGL subclass that draws the image to the screen
// .SECTION Description
// This is the concrete implementation of a ray cast image display helper -
// a helper class responsible for drawing the image to the screen.

// .SECTION see also
// vtkRayCastImageDisplayHelper

#ifndef __vtkOpenGLRayCastImageDisplayHelper_h
#define __vtkOpenGLRayCastImageDisplayHelper_h

#include "vtkRayCastImageDisplayHelper.h"

class vtkVolume;
class vtkRenderer;

class VTK_RENDERING_EXPORT vtkOpenGLRayCastImageDisplayHelper : public vtkRayCastImageDisplayHelper
{
public:
  static vtkOpenGLRayCastImageDisplayHelper *New();
  vtkTypeRevisionMacro(vtkOpenGLRayCastImageDisplayHelper,vtkRayCastImageDisplayHelper);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  void RenderTexture( vtkVolume *vol, vtkRenderer *ren,
                      int imageMemorySize[2],
                      int imageViewportSize[2],
                      int imageInUseSize[2],
                      int imageOrigin[2],
                      float requestedDepth,
                      unsigned char *image );

protected:
  vtkOpenGLRayCastImageDisplayHelper();
  ~vtkOpenGLRayCastImageDisplayHelper();

private:
  vtkOpenGLRayCastImageDisplayHelper(const vtkOpenGLRayCastImageDisplayHelper&);  // Not implemented.
  void operator=(const vtkOpenGLRayCastImageDisplayHelper&);  // Not implemented.
};

#endif

