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
// .NAME vtkOpenGLPolyDataMapper - a PolyDataMapper for the OpenGL library
// .SECTION Description
// vtkOpenGLPolyDataMapper is a subclass of vtkPolyDataMapper.
// vtkOpenGLPolyDataMapper is a geometric PolyDataMapper for the OpenGL 
// rendering library.

#ifndef __vtkOpenGLPolyDataMapper_h
#define __vtkOpenGLPolyDataMapper_h

#include "vtkPolyDataMapper.h"
#include <stdlib.h>
#ifndef VTK_IMPLEMENT_MESA_CXX
  #ifdef __APPLE__
    #include <OpenGL/gl.h>
  #else
    #include <GL/gl.h>
  #endif
#endif

class vtkProperty;
class vtkRenderWindow;
class vtkOpenGLRenderer;

class VTK_RENDERING_EXPORT vtkOpenGLPolyDataMapper : public vtkPolyDataMapper
{
public:
  static vtkOpenGLPolyDataMapper *New();
  vtkTypeRevisionMacro(vtkOpenGLPolyDataMapper,vtkPolyDataMapper);

  // Description:
  // Implement superclass render method.
  virtual void RenderPiece(vtkRenderer *ren, vtkActor *a);

  // Description:
  // Release any graphics resources that are being consumed by this mapper.
  // The parameter window could be used to determine which graphic
  // resources to release.
  void ReleaseGraphicsResources(vtkWindow *);

  // Description:
  // Draw method for OpenGL.
  virtual int Draw(vtkRenderer *ren, vtkActor *a);
  
  //BTX  begin tcl exclude
  
  // Description:
  // Get the lmcolor property, this is a pretty important little 
  // function.  It determines how vertex colors will be handled  
  // in gl.  When a PolyDataMapper has vertex colors it will use this 
  // method to determine what lmcolor mode to set.               
  GLenum GetLmcolorMode(vtkProperty *prop);
  //ETX

protected:
  vtkOpenGLPolyDataMapper();
  ~vtkOpenGLPolyDataMapper();

  int ListId;
private:
  vtkOpenGLPolyDataMapper(const vtkOpenGLPolyDataMapper&);  // Not implemented.
  void operator=(const vtkOpenGLPolyDataMapper&);  // Not implemented.
};

#endif
