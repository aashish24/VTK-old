/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

// .NAME vtkOpenGLVolumeRayCastMapper - OpenGL subclass that draws the image to the screen
// .SECTION Description
// This is the concrete implementation of a ray cast mapper - it is 
// responsible for drawing the image to the screen. The remaining
// functionality all comes from vtkVolumeRayCastMapper

// .SECTION see also
// vtkVolumeRayCastMapper vtkVolumeMapper

#ifndef __vtkOpenGLVolumeRayCastMapper_h
#define __vtkOpenGLVolumeRayCastMapper_h

#include "vtkVolumeRayCastMapper.h"

class VTK_RENDERING_EXPORT vtkOpenGLVolumeRayCastMapper : public vtkVolumeRayCastMapper
{
public:
  static vtkOpenGLVolumeRayCastMapper *New();
  vtkTypeRevisionMacro(vtkOpenGLVolumeRayCastMapper,vtkVolumeRayCastMapper);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkOpenGLVolumeRayCastMapper();
  ~vtkOpenGLVolumeRayCastMapper();

  void RenderTexture( vtkVolume *vol, vtkRenderer *ren);

private:
  vtkOpenGLVolumeRayCastMapper(const vtkOpenGLVolumeRayCastMapper&);  // Not implemented.
  void operator=(const vtkOpenGLVolumeRayCastMapper&);  // Not implemented.
};

#endif

