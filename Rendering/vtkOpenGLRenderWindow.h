/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen 
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
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkOpenGLRenderWindow - OpenGL rendering window
// .SECTION Description
// vtkOpenGLRenderWindow is a concrete implementation of the abstract class
// vtkRenderWindow. vtkOpenGLRenderer interfaces to the OpenGL graphics
// library. Application programmers should normally use vtkRenderWindow
// instead of the OpenGL specific version.

#ifndef __vtkOpenGLRenderWindow_h
#define __vtkOpenGLRenderWindow_h

#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "vtkXRenderWindow.h"
#include "GL/glx.h"

class vtkIdList;

class VTK_EXPORT vtkOpenGLRenderWindow : public vtkXRenderWindow
{
protected:
  GLXContext ContextId;
  int MultiSamples;
  long OldMonitorSetting;

public:
  static vtkOpenGLRenderWindow *New();
  vtkTypeMacro(vtkOpenGLRenderWindow,vtkXRenderWindow);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Begin the rendering process.
  virtual void Start(void);

  // Description:
  // End the rendering process and display the image.
  virtual void Frame(void);

  // Description:
  // Specify various window parameters.
  virtual void WindowConfigure(void);

  // Description:
  // Initialize the window for rendering.
  virtual void WindowInitialize(void);

  // Description:
  // Initialize the rendering window.
  virtual void Initialize(void);

  // Description:
  // Change the window to fill the entire screen.
  virtual void SetFullScreen(int);

  // Description:
  // Resize the window.
  virtual void WindowRemap(void);

  // Description:
  // Set the preferred window size to full screen.
  virtual void PrefFullScreen(void);

  // Description:
  // Specify the size of the rendering window.
  virtual void SetSize(int,int);
  virtual void SetSize(int a[2]) {this->SetSize(a[0], a[1]);};

  // Description:
  // Get the X properties of an ideal rendering window.
  virtual Colormap GetDesiredColormap();
  virtual Visual  *GetDesiredVisual();
  virtual XVisualInfo     *GetDesiredVisualInfo();
  virtual int      GetDesiredDepth();

  // Description:
  // Set/Get the maximum number of multisamples
  static void SetGlobalMaximumNumberOfMultiSamples(int val);
  static int  GetGlobalMaximumNumberOfMultiSamples();

  // Description:
  // Set / Get the number of multisamples to use for hardware antialiasing.
  vtkSetMacro(MultiSamples,int);
  vtkGetMacro(MultiSamples,int);

  // Description:
  // Update system if needed due to stereo rendering.
  virtual void StereoUpdate();

  // Description:
  // Prescribe that the window be created in a stereo-capable mode. This
  // method must be called before the window is realized. This method
  // overrides the superclass method since this class can actually check
  // whether the window has been realized yet.
  virtual void SetStereoCapableWindow(int capable);

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
  virtual unsigned char *GetRGBACharPixelData(int x,int y,int x2,int y2,
					      int front);
  virtual void SetRGBACharPixelData(int x,int y,int x2,int y2,unsigned char *,
				    int front, int blend=0);  

  // Description:
  // Set/Get the zbuffer data from an image
  virtual float *GetZbufferData( int x1, int y1, int x2, int y2 );
  virtual void SetZbufferData( int x1, int y1, int x2, int y2, float *buffer );

  // Description:
  // Make this window the current OpenGL context.
  void MakeCurrent();
  
  // Description:
  // Register a texture name with this render window.
  void RegisterTextureResource (GLuint id);

  // Description:
  // Get the size of the depth buffer.
  int GetDepthBufferSize();
  
protected:
  vtkOpenGLRenderWindow();
  ~vtkOpenGLRenderWindow();
  vtkOpenGLRenderWindow(const vtkOpenGLRenderWindow&) {};
  void operator=(const vtkOpenGLRenderWindow&) {};

  vtkIdList *TextureResourceIds;
};

#endif
