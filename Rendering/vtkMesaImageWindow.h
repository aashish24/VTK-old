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
// .NAME vtkMesaImageWindow - Mesa Imageing window
// .SECTION Description
// vtkMesaImageWindow is a concrete implementation of the abstract
// class vtkImageWindow. vtkMesaImageer interfaces to the standard
// Mesa graphics library in the Windows/NT environment..

#ifndef __vtkMesaImageWindow_h
#define __vtkMesaImageWindow_h

#include <stdlib.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "vtkXImageWindow.h"

#ifdef VTK_MANGLE_MESA
#define USE_MGL_NAMESPACE
#include "mesaglx.h"
#else
#include "GL/glx.h"
#endif
#include "GL/osmesa.h"


class VTK_EXPORT vtkMesaImageWindow : public vtkXImageWindow
{
public:
  static vtkMesaImageWindow *New();
  vtkTypeMacro(vtkMesaImageWindow,vtkXImageWindow);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Initialize the window for rendering.
  virtual void MakeDefaultWindow();

  // Description:
  // Swap the front and back buffers if double buffering is being used.
  void SwapBuffers();
  
  // Description:
  // Draw the contents of the window
  void Render();

  // Description:
  // Xwindow get set functions
  virtual void *GetGenericDisplayId() {return (void *)this->DisplayId;};
  virtual void *GetGenericWindowId();
  virtual void *GetGenericParentId()  {return (void *)this->ParentId;};
  virtual void *GetGenericContext();
  virtual void *GetGenericDrawable()  {return (void *)this->WindowId;};
  
  // Description:
  // Get the X properties of an ideal rendering window.
  virtual Colormap GetDesiredColormap();
  virtual Visual  *GetDesiredVisual();
  XVisualInfo     *GetDesiredVisualInfo();
  virtual int      GetDesiredDepth();

  // Description:
  // Set/Get the pixel data of an image, transmitted as RGBRGB... 
  virtual unsigned char *GetPixelData(int x,int y,int x2,int y2,int front);
  virtual void SetPixelData(int x,int y,int x2,int y2,unsigned char *,
			    int front);

  // Description:
  // Set/Get the pixel data of an image, transmitted as RGBARGBA... 
  virtual float *GetRGBAPixelData(int x,int y,int x2,int y2,int front);
  virtual void SetRGBAPixelData(int x,int y,int x2,int y2,float *,int front,
                                int blend=0);

  // Description:
  // Make this windows Mesa context the current context.
  void MakeCurrent();

  // Description:
  // Erase the window contents 
  virtual void EraseWindow() {this->vtkImageWindow::EraseWindow();};

  // Description:
  // Override the default implmentation so that we can activly switch between
  // on and offscreen rendering.
  virtual void SetOffScreenRendering(int i);

protected:
  vtkMesaImageWindow();
  ~vtkMesaImageWindow();
  vtkMesaImageWindow(const vtkMesaImageWindow&) {};
  void operator=(const vtkMesaImageWindow&) {};

  GLXContext ContextId;
  OSMesaContext OffScreenContextId;
  void *OffScreenWindow;
  int ScreenDoubleBuffer;
  int ScreenMapped;
};


#endif

