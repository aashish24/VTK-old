/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    to Horst Schreiber for developing this MFC code

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

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <GL/glaux.h>
#include "vtkWin32OpenGLRenderWindow.h"
#include "vtkWin32RenderWindowInteractor.h"
#include "vtkOpenGLRenderer.h"
#include "vtkOpenGLProperty.h"
#include "vtkOpenGLTexture.h"
#include "vtkOpenGLCamera.h"
#include "vtkOpenGLActor.h"
#include "vtkOpenGLLight.h"
#include "vtkOpenGLPolyDataMapper.h"

#define MAX_LIGHTS 8

vtkWin32OpenGLRenderWindow::vtkWin32OpenGLRenderWindow()
{
  this->ApplicationInstance = NULL;
  this->Palette = NULL;
  this->ContextId = 0;
  this->MultiSamples = 8;
  this->WindowId = 0;
  this->ParentId = 0;
  this->NextWindowId = 0;
  this->DeviceContext = (HDC)0;		// hsr
  this->MFChandledWindow = FALSE;	// hsr

  if ( this->WindowName ) delete [] this->WindowName;
  this->WindowName = new char[strlen("Visualization Toolkit - Win32OpenGL")+1];
    strcpy( this->WindowName, "Visualization Toolkit - Win32OpenGL" );
}

int vtkWin32OpenGLRenderWindow::GetEventPending()
{
  MSG msg;
  
  return PeekMessage(&msg,this->WindowId,WM_LBUTTONDOWN,WM_MBUTTONDOWN,PM_NOREMOVE);
}

// Description:
// Begin the rendering process.
void vtkWin32OpenGLRenderWindow::Start(void)
{
  // if the renderer has not been initialized, do so now
  if (!this->ContextId)
    {
    this->Initialize();
    }

  // set the current window 
  this->MakeCurrent();
}

void vtkWin32OpenGLRenderWindow::MakeCurrent()
{
  wglMakeCurrent(this->DeviceContext, this->ContextId);
}

void vtkWin32OpenGLRenderWindow::SetSize(int x, int y)
{
  static int resizing = 0;

  if ((this->Size[0] != x) || (this->Size[1] != y))
    {
    this->Modified();
    this->Size[0] = x;
    this->Size[1] = y;
    if (this->Mapped)
      {
      if (!resizing)
        {
        resizing = 1;
   
        if (this->ParentId)
          {
          SetWindowExtEx(this->DeviceContext,x,y,NULL);
          SetViewportExtEx(this->DeviceContext,x,y,NULL);
          SetWindowPos(this->WindowId,HWND_TOP,0,0,
            x, y, SWP_NOMOVE | SWP_NOZORDER);
          }
        else
          {
          SetWindowPos(this->WindowId,HWND_TOP,0,0,
            x+2*GetSystemMetrics(SM_CXFRAME),
            y+2*GetSystemMetrics(SM_CYFRAME) +GetSystemMetrics(SM_CYCAPTION),
            SWP_NOMOVE | SWP_NOZORDER);
          }
        resizing = 0;
        }
      }
    }
}

void vtkWin32OpenGLRenderWindow::SetPosition(int x, int y)
{
  static int resizing = 0;

  if ((this->Position[0] != x) || (this->Position[1] != y))
    {
    this->Modified();
    this->Position[0] = x;
    this->Position[1] = y;
    if (this->Mapped)
      {
      if (!resizing)
        {
        resizing = 1;
   
        SetWindowPos(this->WindowId,HWND_TOP,x,y,
            0, 0, SWP_NOSIZE | SWP_NOZORDER);
        resizing = 0;
        }
      }
    }
}

static void vtkWin32OpenGLSwapBuffers(HDC hdc)
{
  SwapBuffers(hdc);
}

// Description:
// End the rendering process and display the image.
void vtkWin32OpenGLRenderWindow::Frame(void)
{
  glFlush();
  if (!this->AbortRender && this->DoubleBuffer)
    {
    vtkWin32OpenGLSwapBuffers(wglGetCurrentDC());
    vtkDebugMacro(<< " SwapBuffers\n");
    }
}
 

// Description:
// Update system if needed due to stereo rendering.
void vtkWin32OpenGLRenderWindow::StereoUpdate(void)
{
  // no stereo right now
}

// Description:
// Specify various window parameters.
void vtkWin32OpenGLRenderWindow::WindowConfigure()
{
  // this is all handles by the desiredVisualInfo method
}

void vtkWin32OpenGLSetupPixelFormat(HDC hDC)
{
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),  /* size */
        1,                              /* version */
        PFD_SUPPORT_OPENGL |
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,               /* support double-buffering */
        PFD_TYPE_RGBA,                  /* color type */
        16,                             /* prefered color depth */
        0, 0, 0, 0, 0, 0,               /* color bits (ignored) */
        0,                              /* no alpha buffer */
        0,                              /* alpha bits (ignored) */
        0,                              /* no accumulation buffer */
        0, 0, 0, 0,                     /* accum bits (ignored) */
        16,                             /* depth buffer */
        0,                              /* no stencil buffer */
        0,                              /* no auxiliary buffers */
        PFD_MAIN_PLANE,                 /* main layer */
        0,                              /* reserved */
        0, 0, 0,                        /* no layer, visible, damage masks */
    };
    int pixelFormat;

    pixelFormat = ChoosePixelFormat(hDC, &pfd);
    if (pixelFormat == 0) {
        MessageBox(WindowFromDC(hDC), "ChoosePixelFormat failed.", "Error",
                MB_ICONERROR | MB_OK);
        exit(1);
    }

    if (SetPixelFormat(hDC, pixelFormat, &pfd) != TRUE) {
        MessageBox(WindowFromDC(hDC), "SetPixelFormat failed.", "Error",
                MB_ICONERROR | MB_OK);
        exit(1);
    }
}

vtkWin32OpenGLRenderWindow *vtkWin32OpenGLRenderWindowPtr = NULL;

void vtkWin32OpenGLSetupPalette(HDC hDC, vtkWin32OpenGLRenderWindow *me)
{
    int pixelFormat = GetPixelFormat(hDC);
    PIXELFORMATDESCRIPTOR pfd;
    LOGPALETTE* pPal;
    int paletteSize;

    DescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    if (pfd.dwFlags & PFD_NEED_PALETTE) {
        paletteSize = 1 << pfd.cColorBits;
    } else {
        return;
    }

    pPal = (LOGPALETTE*)
        malloc(sizeof(LOGPALETTE) + paletteSize * sizeof(PALETTEENTRY));
    pPal->palVersion = 0x300;
    pPal->palNumEntries = paletteSize;

    /* build a simple RGB color palette */
    {
        int redMask = (1 << pfd.cRedBits) - 1;
        int greenMask = (1 << pfd.cGreenBits) - 1;
        int blueMask = (1 << pfd.cBlueBits) - 1;
        int i;

        for (i=0; i<paletteSize; ++i) {
            pPal->palPalEntry[i].peRed =
                    (((i >> pfd.cRedShift) & redMask) * 255) / redMask;
            pPal->palPalEntry[i].peGreen =
                    (((i >> pfd.cGreenShift) & greenMask) * 255) / greenMask;
            pPal->palPalEntry[i].peBlue =
                    (((i >> pfd.cBlueShift) & blueMask) * 255) / blueMask;
            pPal->palPalEntry[i].peFlags = 0;
        }
    }

    me->Palette = CreatePalette(pPal);
    free(pPal);

    if (me->Palette) {
        SelectPalette(hDC, me->Palette, FALSE);
        RealizePalette(hDC);
    }
}

void vtkWin32OpenGLInit()
{
  glMatrixMode( GL_MODELVIEW );
  glDepthFunc( GL_LEQUAL );
  glEnable( GL_DEPTH_TEST );
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

  // initialize blending for transparency
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  glEnable( GL_NORMALIZE );
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
}


LRESULT APIENTRY vtkWin32OpenGLWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  vtkWin32OpenGLRenderWindow *me = 
    (vtkWin32OpenGLRenderWindow *)GetWindowLong(hWnd,GWL_USERDATA);

  switch (message) 
    {
    case WM_CREATE:
      {
        /* initialize OpenGL rendering */
        me = vtkWin32OpenGLRenderWindowPtr; 
        SetWindowLong(hWnd,GWL_USERDATA,(LONG)me);
        me->DeviceContext = GetDC(hWnd);
        vtkWin32OpenGLSetupPixelFormat(me->DeviceContext);
        vtkWin32OpenGLSetupPalette(me->DeviceContext,me);
		    me->ContextId = wglCreateContext(me->DeviceContext);
        wglMakeCurrent(me->DeviceContext, me->ContextId);
        vtkWin32OpenGLInit();
        return 0;
      }
    case WM_DESTROY:
        /* finish OpenGL rendering */
        if (me->ContextId) 
          {
          wglMakeCurrent(NULL, NULL);
          wglDeleteContext(me->ContextId);
          me->ContextId = NULL;
          }
        if (me->Palette)
          {
          DeleteObject(me->Palette);
          me->Palette = NULL;
          }
        ReleaseDC(me->WindowId, me->DeviceContext);
        return 0;
    case WM_SIZE:
        /* track window size changes */
        if (me->ContextId) 
          {
          me->SetSize((int) LOWORD(lParam),(int) HIWORD(lParam));
          return 0;
          }
    case WM_PALETTECHANGED:
        /* realize palette if this is *not* the current window */
        if (me->ContextId && me->Palette && (HWND) wParam != hWnd) 
          {
          UnrealizeObject(me->Palette);
          SelectPalette(me->DeviceContext, me->Palette, FALSE);
          RealizePalette(me->DeviceContext);
          me->Render();
          break;
          }
        break;
    case WM_QUERYNEWPALETTE:
        /* realize palette if this is the current window */
        if (me->ContextId && me->Palette) 
          {
          UnrealizeObject(me->Palette);
          SelectPalette(me->DeviceContext, me->Palette, FALSE);
          RealizePalette(me->DeviceContext);
          me->Render();
          return TRUE;
          }
        break;
    case WM_PAINT:
        {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        if (me->ContextId) 
          {
          me->Render();
          }
        EndPaint(hWnd, &ps);
        return 0;
        }
        break;
    default:
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}



// Description:
// Initialize the window for rendering.
void vtkWin32OpenGLRenderWindow::WindowInitialize (void)
{
  int x, y, width, height;
  GLenum type;
  static int count = 1;
  
  x = ((this->Position[0] >= 0) ? this->Position[0] : 5);
  y = ((this->Position[1] >= 0) ? this->Position[1] : 5);
  width = ((this->Size[0] > 0) ? this->Size[0] : 300);
  height = ((this->Size[1] > 0) ? this->Size[1] : 300);

  // create our own window if not already set
  this->OwnWindow = 0;
  if (!this->MFChandledWindow)
    { 
    // get the applicaiton instance if we don't have one already
    if (!this->ApplicationInstance)
      {
      // if we have a parent window get the app instance from it
      if (this->ParentId)
        {
        this->ApplicationInstance = (HINSTANCE)GetWindowLong(this->ParentId,GWL_HINSTANCE);
        }
      else
        {
        this->ApplicationInstance = GetModuleHandle(NULL); /*AfxGetInstanceHandle();*/
        }
      }
	  if (!this->WindowId)
		  {
      WNDCLASS wndClass;

      if(this->WindowName) delete [] this->WindowName;
      int len = strlen( "Visualization Toolkit - Win32OpenGL #") 
                          + (int)ceil( (double) log10( (double)(count+1) ) )
                          + 1; 
      this->WindowName = new char [ len ];
		  sprintf(this->WindowName,"Visualization Toolkit - Win32OpenGL #%i",count++);

      // has the class been registered ?
      if (!GetClassInfo(this->ApplicationInstance,"vtkOpenGL",&wndClass))
        {
        wndClass.style = CS_HREDRAW | CS_VREDRAW;
        wndClass.lpfnWndProc = vtkWin32OpenGLWndProc;
        wndClass.cbClsExtra = 0;
        wndClass.cbWndExtra = 0;
        wndClass.hInstance = this->ApplicationInstance;
        wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wndClass.lpszMenuName = NULL;
        wndClass.lpszClassName = "vtkOpenGL";
        RegisterClass(&wndClass);
        }
      
      // use poor mans mutex
      if (vtkWin32OpenGLRenderWindowPtr)
        {
        vtkErrorMacro("Two windows being created at the same time");
        }
      vtkWin32OpenGLRenderWindowPtr = this;
      /* create window */
      if (this->ParentId)
        {
        this->WindowId = CreateWindow(
          "vtkOpenGL", this->WindowName,
          WS_CHILD | WS_CLIPCHILDREN /*| WS_CLIPSIBLINGS*/,
          x, y, width, height,
          this->ParentId, NULL, this->ApplicationInstance, NULL);
        }
      else
        {
        this->WindowId = CreateWindow(
            "vtkOpenGL", this->WindowName,
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN /*| WS_CLIPSIBLINGS*/,
            x,y, width+2*GetSystemMetrics(SM_CXFRAME),
            height+2*GetSystemMetrics(SM_CYFRAME) +GetSystemMetrics(SM_CYCAPTION),
            NULL, NULL, this->ApplicationInstance, NULL);
        }
      vtkWin32OpenGLRenderWindowPtr = NULL;
      if (!this->WindowId)
        {
        vtkErrorMacro("Could not create window, error:  " << GetLastError());
        return;
        }
      // extract the create info

      /* display window */
      ShowWindow(this->WindowId, SW_SHOW);
      //UpdateWindow(this->WindowId);
     
		  this->OwnWindow = 1;
		  }
    this->Mapped = 1;

	  // wglMakeCurrent(GetDC(this->WindowId), this->ContextId);
    }	
  else 
    {
    wglMakeCurrent(this->DeviceContext, this->ContextId); // hsr
    vtkWin32OpenGLInit();
    }

}

// Description:
// Initialize the rendering window.
void vtkWin32OpenGLRenderWindow::Initialize (void)
{
  // make sure we havent already been initialized 
  if (this->ContextId)
    return;

  // now initialize the window 
  this->WindowInitialize();
}


// Description:
// Get the current size of the window.
int *vtkWin32OpenGLRenderWindow::GetSize(void)
{
  RECT rect;

  // if we aren't mapped then just return the ivar 
  if (!this->Mapped)
    {
    return(this->Size);
    }

  //  Find the current window size 
  GetClientRect(this->WindowId, &rect);

  this->Size[0] = rect.right;
  this->Size[1] = rect.bottom;
  
  return this->Size;
}

// Get the current size of the window.
int *vtkWin32OpenGLRenderWindow::GetScreenSize(void)
{
  RECT rect;

  // if we aren't mapped then just return the ivar 
  if (!this->Mapped)
    {
    return(this->Size);
    }

  //  Find the current window size 
  GetClientRect(this->WindowId, &rect);

  this->Size[0] = rect.right;
  this->Size[1] = rect.bottom;
  
  return this->Size;
}

// Description:
// Get the position in screen coordinates of the window.
int *vtkWin32OpenGLRenderWindow::GetPosition(void)
{
  // if we aren't mapped then just return the ivar 
  if (!this->Mapped)
    {
    return(this->Position);
    }

  //  Find the current window position 
//  x,y,&this->Position[0],&this->Position[1],&child);

  return this->Position;
}

// Description:
// Change the window to fill the entire screen.
void vtkWin32OpenGLRenderWindow::SetFullScreen(int arg)
{
  int *temp;
  
  if (this->FullScreen == arg) return;
  
  if (!this->Mapped)
    {
    this->PrefFullScreen();
    return;
    }

  // set the mode 
  this->FullScreen = arg;
  if (this->FullScreen <= 0)
    {
    this->Position[0] = this->OldScreen[0];
    this->Position[1] = this->OldScreen[1];
    this->Size[0] = this->OldScreen[2]; 
    this->Size[1] = this->OldScreen[3];
    this->Borders = this->OldScreen[4];
    }
  else
    {
    // if window already up get its values 
    if (this->WindowId)
      {
      //  Find the current window size 
//      XGetWindowAttributes(this->DisplayId, 
//			   this->WindowId, &attribs);
      
//      this->OldScreen[2] = attribs.width;
//      this->OldScreen[3] = attribs.height;;

      temp = this->GetPosition();      
      this->OldScreen[0] = temp[0];
      this->OldScreen[1] = temp[1];

      this->OldScreen[4] = this->Borders;
      this->PrefFullScreen();
      }
    }
  
  // remap the window 
  this->WindowRemap();

  this->Modified();
}

// Description:
// Set the preferred window size to full screen.
void vtkWin32OpenGLRenderWindow::PrefFullScreen()
{
  int *size;

  size = this->GetScreenSize();

  // use full screen 
  this->Position[0] = 0;
  this->Position[1] = 0;
  this->Size[0] = size[0];
  this->Size[1] = size[1];

  // don't show borders 
  this->Borders = 0;
}

// Description:
// Remap the window.
void vtkWin32OpenGLRenderWindow::WindowRemap()
{
  short cur_light;

  /* first delete all the old lights */
  for (cur_light = GL_LIGHT0; cur_light < GL_LIGHT0+MAX_LIGHTS; cur_light++)
    {
    glDisable(cur_light);
    }
  
  // then close the old window 
  if (this->OwnWindow)
    {
    SendMessage(this->WindowId, WM_CLOSE, 0, 0L );
    }
  
  // set the default windowid 
  this->WindowId = this->NextWindowId;
  this->NextWindowId = 0;

  // configure the window 
  this->WindowInitialize();
}

void vtkWin32OpenGLRenderWindow::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkRenderWindow::PrintSelf(os,indent);

  os << indent << "ContextId: " << this->ContextId << "\n";
  os << indent << "Next Window Id: " << this->NextWindowId << "\n";
  os << indent << "Window Id: " << this->WindowId << "\n";
}


unsigned char *vtkWin32OpenGLRenderWindow::GetPixelData(int x1, int y1, int x2, int y2, 
						 int front)
{
  long     xloop,yloop;
  int     y_low, y_hi;
  int     x_low, x_hi;
  unsigned char   *buffer;
  unsigned char   *data = NULL;
  unsigned char   *p_data = NULL;

  // set the current window 
  this->MakeCurrent();

  buffer = new unsigned char [4*(abs(x2 - x1)+1)];
  data = new unsigned char [(abs(x2 - x1) + 1)*(abs(y2 - y1) + 1)*3];

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
  p_data = data;
  for (yloop = y_hi; yloop >= y_low; yloop--)
    {
    // read in a row of pixels 
    glReadPixels(x_low,yloop,(x_hi-x_low+1),1, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    for (xloop = 0; xloop <= (abs(x2-x1)); xloop++)
      {
      *p_data = buffer[xloop*4]; p_data++;
      *p_data = buffer[xloop*4+1]; p_data++;
      *p_data = buffer[xloop*4+2]; p_data++;
      }
    }
  
  delete [] buffer;

  return data;
}

void vtkWin32OpenGLRenderWindow::SetPixelData(int x1, int y1, int x2, int y2,
					    unsigned char *data, int front)
{
  int     y_low, y_hi;
  int     x_low, x_hi;
  int     xloop,yloop;
  unsigned char   *buffer;
  unsigned char   *p_data = NULL;

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

  buffer = new unsigned char [4*(abs(x2 - x1)+1)];

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
  
  // now write the binary info one row at a time 
  p_data = data;
  for (yloop = y_hi; yloop >= y_low; yloop--)
    {
    for (xloop = 0; xloop <= (abs(x2-x1)); xloop++)
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
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();

    glDrawPixels((x_hi-x_low+1),1, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    }
  
  delete [] buffer;
}

// Description:
// Get the window id.
HWND vtkWin32OpenGLRenderWindow::GetWindowId()
{
  vtkDebugMacro(<< "Returning WindowId of " << this->WindowId << "\n"); 

  return this->WindowId;
}

// Description:
// Set the window id to a pre-existing window.
void vtkWin32OpenGLRenderWindow::SetWindowId(HWND arg)
{
  vtkDebugMacro(<< "Setting WindowId to " << arg << "\n"); 

  this->WindowId = arg;
}

// Description:
// Set the window id to a pre-existing window.
void vtkWin32OpenGLRenderWindow::SetParentId(HWND arg)
{
  vtkDebugMacro(<< "Setting ParentId to " << arg << "\n"); 

  this->ParentId = arg;
}

// Description:
// Set the window id of the new window once a WindowRemap is done.
void vtkWin32OpenGLRenderWindow::SetNextWindowId(HWND arg)
{
  vtkDebugMacro(<< "Setting NextWindowId to " << arg << "\n"); 

  this->NextWindowId = arg;
}

void vtkWin32OpenGLRenderWindow::SetContextId(HGLRC arg) // hsr
{													   // hsr	
  this->ContextId = arg;							   // hsr
}													   // hsr

void vtkWin32OpenGLRenderWindow::SetDeviceContext(HDC arg) // hsr
{														 // hsr
  this->DeviceContext = arg;							 // hsr
  this->MFChandledWindow = TRUE;						 // hsr
}														 // hsr

float *vtkWin32OpenGLRenderWindow::GetRGBAPixelData(int x1, int y1, int x2, int y2, int front)
{
  long    xloop,yloop;
  int     y_low, y_hi;
  int     x_low, x_hi;
  int     width, height;

  float   *data = NULL;

  float   *p_data = NULL;
  unsigned long   *buffer;

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

void vtkWin32OpenGLRenderWindow::SetRGBAPixelData(int x1, int y1, 
						int x2, int y2,
						float *data, int front)
{
  int     y_low, y_hi;
  int     x_low, x_hi;
  int     width, height;
  int     xloop,yloop;
  float   *buffer;
  float   *p_data = NULL;

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
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();
  glMatrixMode( GL_PROJECTION );
  glPopMatrix();

  glDrawPixels( width, height, GL_RGBA, GL_FLOAT, data);

}

float *vtkWin32OpenGLRenderWindow::GetZbufferData( int x1, int y1, 
						 int x2, int y2  )
{
  int             y_low, y_hi;
  int             x_low, x_hi;
  int             width, height;
  float           *z_data = NULL;

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

  width =  abs(x2 - x1)+1;
  height = abs(y2 - y1)+1;

  z_data = new float[width*height];

  glReadPixels( x_low, y_low, 
		width, height,
		GL_DEPTH_COMPONENT, GL_FLOAT,
		z_data );

  return z_data;
}

void vtkWin32OpenGLRenderWindow::SetZbufferData( int x1, int y1, int x2, int y2,
					       float *buffer )
{
  int             y_low, y_hi;
  int             x_low, x_hi;
  int             width, height;

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

  width =  abs(x2 - x1)+1;
  height = abs(y2 - y1)+1;

  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glRasterPos2f( 2.0 * (GLfloat)(x_low) / this->Size[0] - 1, 
                 2.0 * (GLfloat)(y_low) / this->Size[1] - 1);
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();
  glMatrixMode( GL_PROJECTION );
  glPopMatrix();

  glDrawPixels( width, height, GL_DEPTH_COMPONENT, GL_FLOAT, buffer);

}

