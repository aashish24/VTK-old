/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen.

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
#include <stdlib.h>
#include <math.h>
#include <iostream.h>
#include "vtkMesaImageWindow.h"
#include "vtkMesaImager.h"
#include "vtkObjectFactory.h"

#ifdef VTK_MANGLE_MESA
#define USE_MGL_NAMESPACE
#include "mesagl.h"
#else
#include "GL/gl.h"
#endif


//------------------------------------------------------------------------------
vtkMesaImageWindow* vtkMesaImageWindow::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMesaImageWindow");
  if(ret)
    {
    return (vtkMesaImageWindow*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMesaImageWindow;
}




// a couple of routines for offscreen rendering
void vtkOSMesaDestroyImageWindow(void *Window) 
{
  free(Window);
}

void *vtkOSMesaCreateImageWindow(int width, int height) 
{
  return malloc(width*height*4);
}

XVisualInfo *vtkMesaImageWindowTryForVisual(Display *DisplayId,
					      int doublebuff)
{
  int           index;
  static int	attributes[50];

  // setup the default stuff we ask for
  index = 0;
  attributes[index++] = GLX_RGBA;
  attributes[index++] = GLX_RED_SIZE;
  attributes[index++] = 1;
  attributes[index++] = GLX_GREEN_SIZE;
  attributes[index++] = 1;
  attributes[index++] = GLX_BLUE_SIZE;
  attributes[index++] = 1;
  attributes[index++] = GLX_DEPTH_SIZE;
  attributes[index++] = 1;
  if (doublebuff)
    {
    attributes[index++] = GLX_DOUBLEBUFFER;
    }
    
  attributes[index++] = None;

  return glXChooseVisual(DisplayId, DefaultScreen(DisplayId), attributes );
}

XVisualInfo *vtkMesaImageWindow::GetDesiredVisualInfo()
{
  XVisualInfo   *v = NULL;
  
  // get the default display connection 
  if (!this->DisplayId)
    {
    this->DisplayId = XOpenDisplay((char *)NULL); 
    if (this->DisplayId == NULL) 
      {
      vtkErrorMacro(<< "bad X server connection.\n");
      }
    this->OwnDisplay = 1;
    }

  // try every possibility stoping when we find one that works
  v = vtkMesaImageWindowTryForVisual(this->DisplayId,
				       this->DoubleBuffer);

  if (!v) 
    {
      v = vtkMesaImageWindowTryForVisual(this->DisplayId,
					   !this->DoubleBuffer);
      if (v) this->DoubleBuffer = !this->DoubleBuffer;
    }

  if (!v) 
    {
    vtkErrorMacro(<< "Could not find a decent visual\n");
    }
  return ( v );
}

vtkMesaImageWindow::vtkMesaImageWindow()
{
  this->OffScreenContextId = NULL;
  this->ContextId = NULL;

  if ( this->WindowName ) 
    {
    delete [] this->WindowName;
    }
  this->WindowName = new char[strlen("Visualization Toolkit - Mesa")+1];
  strcpy( this->WindowName, "Visualization Toolkit - Mesa" );

  // Default to double buffer off since some systems cannot get deep grayscale
  // visuals. This is inconsistent with the WIn32 class, but necessary.
  this->DoubleBuffer = 0;
  this->Erase = 1;
}

// free up memory & close the window
vtkMesaImageWindow::~vtkMesaImageWindow()
{
  // make sure we have been initialized 
  if (this->ContextId || this->OffScreenContextId)
    {
    this->MakeCurrent();

    glFinish();

    // then close the old window 
    if (this->OffScreenRendering)
      {
      OSMesaDestroyContext(this->OffScreenContextId);
      this->OffScreenContextId = NULL;
      vtkOSMesaDestroyImageWindow(this->OffScreenWindow);
      this->OffScreenWindow = NULL;
      }
    else
      {
      glXDestroyContext( this->DisplayId, this->ContextId);
      // then close the old window 
      if (this->WindowCreated && this->DisplayId && this->WindowId)
	{
	XDestroyWindow(this->DisplayId,this->WindowId);
	this->WindowId = (Window)NULL;
	}
      }
    this->ContextId = NULL;
    }
}

// Begin the Imaging process.
void vtkMesaImageWindow::Render()
{
  if (this->WindowCreated)
    {
    this->MakeCurrent();
    }
  if (this->DoubleBuffer)
    {
    glDrawBuffer(GL_BACK);
    }
  this->vtkImageWindow::Render();
}

// End the Imaging process and display the image.
void vtkMesaImageWindow::SwapBuffers()
{
  glFlush();
  if (this->DoubleBuffer)
    {
    glXSwapBuffers(this->DisplayId, this->WindowId);
    vtkDebugMacro(<< " glXSwapBuffers\n");
    }
}
 

// Initialize the window for Imaging.
void vtkMesaImageWindow::MakeDefaultWindow()
{
  XVisualInfo  *v, matcher;
  XSetWindowAttributes	attr;
  int x, y, width, height, nItems;
  XWindowAttributes winattr;
  XSizeHints xsh;

  xsh.flags = USSize;
  if ((this->Position[0] >= 0)&&(this->Position[1] >= 0))
    {
    xsh.flags |= USPosition;
    xsh.x =  (int)(this->Position[0]);
    xsh.y =  (int)(this->Position[1]);
    }
  
  x = ((this->Position[0] >= 0) ? this->Position[0] : 5);
  y = ((this->Position[1] >= 0) ? this->Position[1] : 5);
  width = ((this->Size[0] > 0) ? this->Size[0] : 256);
  height = ((this->Size[1] > 0) ? this->Size[1] : 256);

  xsh.width  = width;
  xsh.height = height;

  if (!this->OffScreenRendering)
    {
    // get the default display connection 
    if (!this->DisplayId)
      {
      this->DisplayId = XOpenDisplay((char *)NULL); 
      if (this->DisplayId == NULL) 
	{
	vtkErrorMacro(<< "bad X server connection.\n");
	}
      this->OwnDisplay = 1;
      }
    
    attr.override_redirect = False;
    
    // create our own window ? 
    this->WindowCreated = 0;
    if (!this->WindowId)
      {
      v = this->GetDesiredVisualInfo();
      this->ColorMap = XCreateColormap(this->DisplayId,
				     RootWindow( this->DisplayId, v->screen),
				       v->visual, AllocNone );
      
      attr.background_pixel = 0;
      attr.border_pixel = 0;
      attr.colormap = this->ColorMap;
      attr.event_mask = StructureNotifyMask | ExposureMask;
      
      // get a default parent if one has not been set.
      if (! this->ParentId)
	{
	this->ParentId = RootWindow(this->DisplayId, v->screen);
	}
      
      this->WindowId = 
	XCreateWindow(this->DisplayId,
		      this->ParentId,
		      x, y, width, height, 0, v->depth, InputOutput, v->visual,
		      CWBackPixel | CWBorderPixel | CWColormap | 
		      CWOverrideRedirect | CWEventMask, 
		      &attr);
      XSync(this->DisplayId,False);
      
      XStoreName(this->DisplayId, this->WindowId, this->WindowName);
      XSetNormalHints(this->DisplayId,this->WindowId,&xsh);
      this->WindowCreated = 1;
      }
    else
      {
      XChangeWindowAttributes(this->DisplayId,this->WindowId,
			      CWOverrideRedirect, &attr);
      XGetWindowAttributes(this->DisplayId,
			   this->WindowId,&winattr);
      matcher.visualid = XVisualIDFromVisual(winattr.visual);
      matcher.screen = DefaultScreen(DisplayId);
      v = XGetVisualInfo(this->DisplayId, VisualIDMask | VisualScreenMask,
			 &matcher, &nItems);
      }
    
    // RESIZE THE WINDOW TO THE DESIRED SIZE
    vtkDebugMacro(<< "Resizing the xwindow\n");
    XResizeWindow(this->DisplayId,this->WindowId,
		  ((this->Size[0] > 0) ? 
		   (int)(this->Size[0]) : 256),
		  ((this->Size[1] > 0) ? 
		   (int)(this->Size[1]) : 256));
    XSync(this->DisplayId,False);
    
    this->ContextId = glXCreateContext(this->DisplayId, v, 0, GL_TRUE);
    XSync(this->DisplayId,False);
    this->MakeCurrent();
    XSync(this->DisplayId,False);

    vtkDebugMacro(" Mapping the xwindow\n");
    XMapWindow(this->DisplayId, this->WindowId);
    XSync(this->DisplayId,False);
    XGetWindowAttributes(this->DisplayId,
			 this->WindowId,&winattr);
    while (winattr.map_state == IsUnmapped)
      {
      XGetWindowAttributes(this->DisplayId,
			   this->WindowId,&winattr);
      };
    // free the visual info
    if (v)
      {
      XFree(v);
      }
    }
  else
    {
    this->DoubleBuffer = 0;
    if (!this->OffScreenWindow)
      {
      this->OffScreenWindow = vtkOSMesaCreateImageWindow(width,height);
      this->Size[0] = width;
      this->Size[1] = height;      
      }    
    this->OffScreenContextId = OSMesaCreateContext(GL_RGBA, NULL);
    this->MakeCurrent();
    this->Mapped = 0;
    }
  
  glMatrixMode( GL_MODELVIEW );
  glClearColor(0,0,0,1);
  glDisable(GL_DEPTH_TEST);

  this->Mapped = 1;
}

int vtkMesaImageWindow::GetDesiredDepth()
{
  XVisualInfo *v;
  int depth = 0;
  
  // get the default visual to use 
  v = this->GetDesiredVisualInfo();
  
  if (v)
    {
    depth = v->depth;  
    XFree(v);
    }

  return depth;
}

// Get a visual from the windowing system.
Visual *vtkMesaImageWindow::GetDesiredVisual ()
{
  XVisualInfo *v;
  Visual *vis;
  
  // get the default visual to use 
  v = this->GetDesiredVisualInfo();

  if (v)
    {
    vis = v->visual;  
    XFree(v);
    }
  
  return vis;  
}


// Get a colormap from the windowing system.
Colormap vtkMesaImageWindow::GetDesiredColormap ()
{
  XVisualInfo *v;
  
  if (this->ColorMap) return this->ColorMap;
  
  // get the default visual to use 
  v = this->GetDesiredVisualInfo();

  this->ColorMap = XCreateColormap(this->DisplayId,
				   RootWindow( this->DisplayId, v->screen),
				   v->visual, AllocNone ); 
  if (v)
    {
    XFree(v);
    }

  return this->ColorMap;  
}

void vtkMesaImageWindow::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkImageWindow::PrintSelf(os,indent);

  os << indent << "ContextId: " << this->ContextId << "\n";
  os << indent << "OffScreenContextId: " << this->OffScreenContextId << "\n";
}


unsigned char *vtkMesaImageWindow::GetPixelData(int x1, int y1, 
						   int x2, int y2, 
						   int front)
{
  int     y_low, y_hi;
  int     x_low, x_hi;
  unsigned char   *data = NULL;

  // set the current window 
  this->MakeCurrent();

  if (y1 < y2)
    {
    y_low = y1; 
    y_hi  = y2;
    }
  else
    {
    y_low = y2; 
    y_hi  = y1;
    }

  if (x1 < x2)
    {
    x_low = x1; 
    x_hi  = x2;
    }
  else
    {
    x_low = x2; 
    x_hi  = x1;
    }

  if (front)
    {
    glReadBuffer(GL_FRONT);
    }
  else
    {
    glReadBuffer(GL_BACK);
    }

  data = new unsigned char[(x_hi - x_low + 1)*(y_hi - y_low + 1)*3];

#ifdef sparc
  // We need to read the image data one row at a time and convert it
  // from RGBA to RGB to get around a bug in Sun Mesa 1.1
  long    xloop, yloop;
  unsigned char *buffer;
  unsigned char *p_data = NULL;
  
  buffer = new unsigned char [4*(x_hi - x_low + 1)];
  p_data = data;
  for (yloop = y_low; yloop <= y_hi; yloop++)
    {
    // read in a row of pixels
    glReadPixels(x_low,yloop,(x_hi-x_low+1),1,
		 GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    for (xloop = 0; xloop <= x_hi-x_low; xloop++)
      {
      *p_data = buffer[xloop*4]; p_data++;
      *p_data = buffer[xloop*4+1]; p_data++;
      *p_data = buffer[xloop*4+2]; p_data++;
      }
    }
  
  delete [] buffer;  
#else
  // If the Sun bug is ever fixed, then we could use the following
  // technique which provides a vast speed improvement on the SGI
  
  // Calling pack alignment ensures that we can grab the any size window
  glPixelStorei( GL_PACK_ALIGNMENT, 1 );
  glReadPixels(x_low, y_low, x_hi-x_low+1, y_hi-y_low+1, GL_RGB,
               GL_UNSIGNED_BYTE, data);
#endif
  
  return data;
}

void vtkMesaImageWindow::SetPixelData(int x1, int y1, int x2, int y2,
				       unsigned char *data, int front)
{
  int     y_low, y_hi;
  int     x_low, x_hi;

  // set the current window 
  this->MakeCurrent();

  if (front)
    {
    glDrawBuffer(GL_FRONT);
    }
  else
    {
    glDrawBuffer(GL_BACK);
    }

  if (y1 < y2)
    {

    y_low = y1; 
    y_hi  = y2;
    }
  else
    {
    y_low = y2; 
    y_hi  = y1;
    }
  
  if (x1 < x2)
    {
    x_low = x1; 
    x_hi  = x2;
    }
  else
    {
    x_low = x2; 
    x_hi  = x1;
    }

#ifdef sparc
  // We need to read the image data one row at a time and convert it
  // from RGBA to RGB to get around a bug in Sun Mesa 1.1
  long    xloop, yloop;
  unsigned char *buffer;
  unsigned char *p_data = NULL;
  
  buffer = new unsigned char [4*(x_hi - x_low + 1)];

  // now write the binary info one row at a time
  glDisable(GL_BLEND);
  p_data = data;
  for (yloop = y_low; yloop <= y_hi; yloop++)
    {
    for (xloop = 0; xloop <= x_hi - x_low; xloop++)
      {
      buffer[xloop*4] = *p_data; p_data++;
      buffer[xloop*4+1] = *p_data; p_data++;
      buffer[xloop*4+2] = *p_data; p_data++;
      buffer[xloop*4+3] = 0xff;
      }
    /* write out a row of pixels */
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();
    glRasterPos3f( (2.0 * (GLfloat)(x_low) / this->Size[0] - 1),
		   (2.0 * (GLfloat)(yloop) / this->Size[1] - 1),
		   -1.0 );
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();

    glDrawPixels((x_hi-x_low+1),1, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    }
  glEnable(GL_BLEND);
#else
  // If the Sun bug is ever fixed, then we could use the following
  // technique which provides a vast speed improvement on the SGI
  
  // now write the binary info
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glRasterPos3f( (2.0 * (GLfloat)(x_low) / this->Size[0] - 1), 
                 (2.0 * (GLfloat)(y_low) / this->Size[1] - 1),
                 -1.0 );
  glMatrixMode( GL_PROJECTION );
  glPopMatrix();
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();

  glPixelStorei( GL_UNPACK_ALIGNMENT, 1);
  glDisable(GL_BLEND);
  glDrawPixels((x_hi-x_low+1), (y_hi - y_low + 1),
               GL_RGB, GL_UNSIGNED_BYTE, data);
  glEnable(GL_BLEND);
#endif
}

float *vtkMesaImageWindow::GetRGBAPixelData(int x1, int y1, int x2, int y2, int front)
{
  int     y_low, y_hi;
  int     x_low, x_hi;
  int     width, height;
  float   *data = NULL;

  // set the current window 
  this->MakeCurrent();

  if (y1 < y2)
    {
    y_low = y1; 
    y_hi  = y2;
    }
  else
    {
    y_low = y2; 
    y_hi  = y1;
    }

  if (x1 < x2)
    {
    x_low = x1; 
    x_hi  = x2;
    }
  else
    {
    x_low = x2; 
    x_hi  = x1;
    }

  if (front)
    {
    glReadBuffer(GL_FRONT);
    }
  else
    {
    glReadBuffer(GL_BACK);
    }

  width  = abs(x_hi - x_low) + 1;
  height = abs(y_hi - y_low) + 1;

  data = new float[ (width*height*4) ];

  glReadPixels( x_low, y_low, width, height, GL_RGBA, GL_FLOAT, data);

  return data;
}

void vtkMesaImageWindow::SetRGBAPixelData(int x1, int y1, int x2, int y2,
				       float *data, int front, int blend)
{
  int     y_low, y_hi;
  int     x_low, x_hi;
  int     width, height;

  // set the current window 
  this->MakeCurrent();

  if (front)
    {
    glDrawBuffer(GL_FRONT);
    }
  else
    {
    glDrawBuffer(GL_BACK);
    }

  if (y1 < y2)
    {
    y_low = y1; 
    y_hi  = y2;
    }
  else
    {
    y_low = y2; 
    y_hi  = y1;
    }
  
  if (x1 < x2)
    {
    x_low = x1; 
    x_hi  = x2;
    }
  else
    {
    x_low = x2; 
    x_hi  = x1;
    }
  
  width  = abs(x_hi-x_low) + 1;
  height = abs(y_hi-y_low) + 1;

  /* write out a row of pixels */
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glRasterPos3f( (2.0 * (GLfloat)(x_low) / this->Size[0] - 1), 
                 (2.0 * (GLfloat)(y_low) / this->Size[1] - 1),
		 -1.0 );
  glMatrixMode( GL_PROJECTION );
  glPopMatrix();
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();

  if (!blend)
    {
    glDisable(GL_BLEND);
    glDrawPixels( width, height, GL_RGBA, GL_FLOAT, data);
    glEnable(GL_BLEND);
    }
  else
    {
    glDrawPixels( width, height, GL_RGBA, GL_FLOAT, data);
    }
}


void vtkMesaImageWindow::MakeCurrent()
{
  // set the current window 
  if (this->OffScreenRendering)
    {
    if (this->OffScreenContextId) 
      {
      if (OSMesaMakeCurrent(this->OffScreenContextId, 
			    this->OffScreenWindow, GL_UNSIGNED_BYTE, 
			    this->Size[0], this->Size[1]) != GL_TRUE) 
	{
	vtkWarningMacro("failed call to OSMesaMakeCurrent");
	}
      }
    }
  else
    {
    if (this->ContextId && (this->ContextId != glXGetCurrentContext()))
      {
      glXMakeCurrent(this->DisplayId,this->WindowId,this->ContextId);
      }
    }
}

// Set the X display id for this RenderWindow to use to a pre-existing 
// X display id.

void *vtkMesaImageWindow::GetGenericContext()
{
  if (this->OffScreenRendering)
    {
    return (void *)this->OffScreenContextId;
    }
  else
    {
    return (void *)this->ContextId;
    }
}

void *vtkMesaImageWindow::GetGenericWindowId()
{
  if (this->OffScreenRendering)
    {
    return (void *)this->OffScreenWindow;
    }
  else
    {
    return (void *)this->WindowId;
    }
}

void vtkMesaImageWindow::SetOffScreenRendering(int i)
{
  if (this->OffScreenRendering == i)
    {
    return;
    }
  
  // invoke super
  this->vtkImageWindow::SetOffScreenRendering(i);
  
  // setup everything
  if (i)
    {
    this->ScreenDoubleBuffer = this->DoubleBuffer;    
    this->DoubleBuffer = 0;
    this->ScreenMapped = this->Mapped;
    this->Mapped = 0;
    if (!this->OffScreenWindow)
      {
      this->MakeDefaultWindow();
      }    
    }
  else
    {
    if (!this->OffScreenWindow)
      {
      OSMesaDestroyContext(this->OffScreenContextId);
      this->OffScreenContextId = NULL;
      vtkOSMesaDestroyImageWindow(this->OffScreenWindow);
      this->OffScreenWindow = NULL;      
      }
    this->DoubleBuffer = this->ScreenDoubleBuffer;
    this->Mapped = this->ScreenMapped;
    this->MakeCurrent();
    // reset the size based on the screen window
    this->GetSize();
    }
}


