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
#include "vtkWin32OpenGLRenderWindow.h"

#include "vtkObjectFactory.h"
#include "vtkOpenGLActor.h"
#include "vtkOpenGLCamera.h"
#include "vtkOpenGLLight.h"
#include "vtkOpenGLPolyDataMapper.h"
#include "vtkOpenGLProperty.h"
#include "vtkOpenGLRenderer.h"
#include "vtkOpenGLTexture.h"
#include "vtkRendererCollection.h"
#include "vtkString.h"
#include "vtkWin32RenderWindowInteractor.h"

#include <math.h>

#if defined(_MSC_VER) || defined (__BORLANDC__)
#include <GL/glaux.h>
#else
#include <GL/gl.h>
#endif

vtkCxxRevisionMacro(vtkWin32OpenGLRenderWindow, "$Revision$");
vtkStandardNewMacro(vtkWin32OpenGLRenderWindow);

#define VTK_MAX_LIGHTS 8

#if ( _MSC_VER >= 1300 ) // Visual studio .NET
#pragma warning ( disable : 4311 )
#pragma warning ( disable : 4312 )
#  define vtkGetWindowLong GetWindowLongPtr
#  define vtkSetWindowLong SetWindowLongPtr
#else // regular Visual studio 
#  define vtkGetWindowLong GetWindowLong
#  define vtkSetWindowLong SetWindowLong
#endif // 

vtkWin32OpenGLRenderWindow::vtkWin32OpenGLRenderWindow()
{
  this->ApplicationInstance =  NULL;
  this->Palette = NULL;
  this->ContextId = 0;
  this->MultiSamples = 8;
  this->WindowId = 0;
  this->ParentId = 0;
  this->NextWindowId = 0;
  this->DeviceContext = (HDC)0;         // hsr
  this->MFChandledWindow = FALSE;       // hsr
  this->StereoType = VTK_STEREO_CRYSTAL_EYES;  
  this->CursorHidden = 0;
}

vtkWin32OpenGLRenderWindow::~vtkWin32OpenGLRenderWindow()
{
  if (this->CursorHidden)
    {
    this->ShowCursor();
    }

  if (this->OffScreenRendering)
    {
    this->CleanUpOffScreenRendering();
    }

  if (this->WindowId && this->OwnWindow)
    {
    this->Clean();
    ReleaseDC(this->WindowId, this->DeviceContext);
    // can't set WindowId=NULL, needed for DestroyWindow
    this->DeviceContext = NULL;
    
    // clear the extra data before calling destroy
    vtkSetWindowLong(this->WindowId,4,(LONG)0);
    DestroyWindow(this->WindowId);
    }
}

void vtkWin32OpenGLRenderWindow::Clean()
{
  vtkRenderer *ren;
  GLuint id;
  
  /* finish OpenGL rendering */
  if (this->ContextId) 
    {
    this->MakeCurrent();

    /* now delete all textures */
    glDisable(GL_TEXTURE_2D);
    for (int i = 1; i < this->TextureResourceIds->GetNumberOfIds(); i++)
      {
      id = (GLuint) this->TextureResourceIds->GetId(i);
#ifdef GL_VERSION_1_1
      if (glIsTexture(id))
        {
        glDeleteTextures(1, &id);
        }
#else
      if (glIsList(id))
        {
        glDeleteLists(id,1);
        }
#endif
      }

    // tell each of the renderers that this render window/graphics context
    // is being removed (the RendererCollection is removed by vtkRenderWindow's
    // destructor)
    this->Renderers->InitTraversal();
    for ( ren = (vtkOpenGLRenderer *) this->Renderers->GetNextItemAsObject();
          ren != NULL;
          ren = (vtkOpenGLRenderer *) this->Renderers->GetNextItemAsObject() )
      {
      ren->SetRenderWindow(NULL);
      }
    
    if (wglMakeCurrent(NULL, NULL) != TRUE) 
      {
      vtkErrorMacro("wglMakeCurrent failed in Clean(), error: " << GetLastError());
      }
    if (wglDeleteContext(this->ContextId) != TRUE) 
      {
      vtkErrorMacro("wglDeleteContext failed in Clean(), error: " << GetLastError());
      }
    this->ContextId = NULL;
    }
  if (this->Palette)
    {
    SelectPalette(this->DeviceContext, this->OldPalette, FALSE); // SVA delete the old palette
    DeleteObject(this->Palette);
    this->Palette = NULL;
    }
}

LRESULT APIENTRY vtkWin32OpenGLRenderWindow::WndProc(HWND hWnd, UINT message, 
                                                     WPARAM wParam, 
                                                     LPARAM lParam)
{
  vtkWin32OpenGLRenderWindow *me = 
    (vtkWin32OpenGLRenderWindow *)vtkGetWindowLong(hWnd,4);

  // forward to actual object
  if (me)
    {
    return me->MessageProc(hWnd, message, wParam, lParam);
    }

  return DefWindowProc(hWnd, message, wParam, lParam);
}

void vtkWin32OpenGLRenderWindow::SetWindowName( const char * _arg )
{
  vtkWindow::SetWindowName(_arg);
  if (this->WindowId)
    {
    SetWindowText(this->WindowId,this->WindowName);
    }
}

int vtkWin32OpenGLRenderWindow::GetEventPending()
{
  MSG msg;

  if (PeekMessage(&msg,this->WindowId,WM_MOUSEFIRST,WM_MOUSELAST,PM_NOREMOVE))
    {
    if ((msg.message == WM_LBUTTONDOWN) ||
        (msg.message == WM_RBUTTONDOWN) ||
        (msg.message == WM_MBUTTONDOWN))
      {
      return 1;
      }
    }

  return 0;
}

// this is a global because we really don't think you are doing
// multithreaded OpenGL rendering anyhow. Most current libraries
// don't support it.
HGLRC vtkWin32OpenGLGlobalContext = 0;

void vtkWin32OpenGLRenderWindow::MakeCurrent()
{
  if (this->ContextId != vtkWin32OpenGLGlobalContext)
    {
    // Try to avoid doing anything (for performance).
    if (this->ContextId)
      { 
      if (wglMakeCurrent(this->DeviceContext, this->ContextId) != TRUE) 
        {
        LPVOID lpMsgBuf;
        ::FormatMessage( 
          FORMAT_MESSAGE_ALLOCATE_BUFFER | 
          FORMAT_MESSAGE_FROM_SYSTEM | 
          FORMAT_MESSAGE_IGNORE_INSERTS,
          NULL,
          GetLastError(),
          MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
          (LPTSTR) &lpMsgBuf,
          0,
          NULL 
          );
        vtkErrorMacro("wglMakeCurrent failed in MakeCurrent(), error: " 
                      << (LPCTSTR)lpMsgBuf);
        ::LocalFree( lpMsgBuf );
        }
      }
    vtkWin32OpenGLGlobalContext = this->ContextId;
    }
}

void vtkWin32OpenGLRenderWindow::SetSize(int x, int y)
{
  static int resizing = 0;

  if ((this->Size[0] != x) || (this->Size[1] != y))
    {
    this->Modified();
    this->Size[0] = x;
    this->Size[1] = y;
    if (this->OffScreenRendering)
      {
      if (!resizing)
        {
        resizing = 1;
        this->CleanUpOffScreenRendering();
        HDC dc = CreateDC("DISPLAY", 0, 0, 0);
        this->CreateOffScreenDC(x, y, dc);
        DeleteDC(dc);
        resizing = 0;
        }
      }

    else if (this->Mapped)
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


// End the rendering process and display the image.
void vtkWin32OpenGLRenderWindow::Frame(void)
{
  this->MakeCurrent();
  if (!this->AbortRender && this->DoubleBuffer && this->SwapBuffers)
    {
    // use global scope to get Win32 API SwapBuffers and not be
    // confused with this->SwapBuffers
    ::SwapBuffers(this->DeviceContext);
    vtkDebugMacro(<< " SwapBuffers\n");
    }
  else
    {
    glFlush();
    }
}
 
void vtkWin32OpenGLRenderWindow::SetupPixelFormat(HDC hDC, DWORD dwFlags, 
                                                  int debug, int bpp, 
                                                  int zbpp)
{
  PIXELFORMATDESCRIPTOR pfd = {
    sizeof(PIXELFORMATDESCRIPTOR),  /* size */
    1,                              /* version */
    dwFlags         ,               /* support double-buffering */
    PFD_TYPE_RGBA,                  /* color type */
    bpp,                             /* prefered color depth */
    0, 0, 0, 0, 0, 0,               /* color bits (ignored) */
    0,                              /* no alpha buffer */
    0,                              /* alpha bits (ignored) */
    0,                              /* no accumulation buffer */
    0, 0, 0, 0,                     /* accum bits (ignored) */
    zbpp,                           /* depth buffer */
    0,                              /* no stencil buffer */
    0,                              /* no auxiliary buffers */
    PFD_MAIN_PLANE,                 /* main layer */
    0,                              /* reserved */
    0, 0, 0,                        /* no layer, visible, damage masks */
  };
  int pixelFormat;
  // Only try to set pixel format if we do not currently have one
  int currentPixelFormat = GetPixelFormat(hDC);
  // if there is a current pixel format, then make sure it
  // supports OpenGL
  if (currentPixelFormat != 0)
    {
    DescribePixelFormat(hDC, currentPixelFormat,sizeof(pfd), &pfd);
    if (!(pfd.dwFlags & PFD_SUPPORT_OPENGL))
      {
      MessageBox(WindowFromDC(hDC), 
                 "Invalid pixel format, no OpenGL support",
                 "Error",
                 MB_ICONERROR | MB_OK);
      exit(1);
      }         
    }
  else
    {
    // hDC has no current PixelFormat, so 
    pixelFormat = ChoosePixelFormat(hDC, &pfd);
    if (pixelFormat == 0)
      {
      MessageBox(WindowFromDC(hDC), "ChoosePixelFormat failed.", "Error",
                 MB_ICONERROR | MB_OK);
      exit(1);
      }
    DescribePixelFormat(hDC, pixelFormat,sizeof(pfd), &pfd); 
    if (SetPixelFormat(hDC, pixelFormat, &pfd) != TRUE) 
      {
      // int err = GetLastError();
      MessageBox(WindowFromDC(hDC), "SetPixelFormat failed.", "Error",
                 MB_ICONERROR | MB_OK);
      exit(1);
      }
    }
  if (debug && (dwFlags & PFD_STEREO) && !(pfd.dwFlags & PFD_STEREO))
    {
    vtkGenericWarningMacro("No Stereo Available!");
    this->StereoCapableWindow = 0;
    }
}

void vtkWin32OpenGLRenderWindow::SetupPalette(HDC hDC)
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

    this->Palette = CreatePalette(pPal);
    free(pPal);

    if (this->Palette) {
        this->OldPalette = SelectPalette(hDC, this->Palette, FALSE);
        RealizePalette(hDC);
    }
}


LRESULT vtkWin32OpenGLRenderWindow::MessageProc(HWND hWnd, UINT message, 
                                                WPARAM wParam, LPARAM lParam)
{
  switch (message) 
    {
    case WM_CREATE:
      {
      // nothing to be done here, opengl is initilized after the call to
      // create now
      return 0;
      }
    case WM_DESTROY:
      this->Clean();
      if (this->DeviceContext)
        {
        ReleaseDC(this->WindowId, this->DeviceContext);
        this->DeviceContext = NULL;
        this->WindowId = NULL;
        }
      return 0;
    case WM_SIZE:
        /* track window size changes */
        if (this->ContextId) 
          {
          this->SetSize((int) LOWORD(lParam),(int) HIWORD(lParam));
          return 0;
          }
    case WM_PALETTECHANGED:
        /* realize palette if this is *not* the current window */
        if (this->ContextId && this->Palette && (HWND) wParam != hWnd) 
          {
          SelectPalette(this->DeviceContext, this->OldPalette, FALSE);
          UnrealizeObject(this->Palette);
          this->OldPalette = SelectPalette(this->DeviceContext, 
                                           this->Palette, FALSE);
          RealizePalette(this->DeviceContext);
          this->Render();
          }
        break;
    case WM_QUERYNEWPALETTE:
        /* realize palette if this is the current window */
        if (this->ContextId && this->Palette) 
          {
          SelectPalette(this->DeviceContext, this->OldPalette, FALSE);
          UnrealizeObject(this->Palette);
          this->OldPalette = SelectPalette(this->DeviceContext, 
                                           this->Palette, FALSE);
          RealizePalette(this->DeviceContext);
          this->Render();
          return TRUE;
          }
        break;
    case WM_PAINT:
        {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        if (this->ContextId) 
          {
          this->Render();
          }
        EndPaint(hWnd, &ps);
        return 0;
        }
        break;
    case WM_ERASEBKGND:
      return TRUE;
    default:
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}


void vtkWin32OpenGLRenderWindow::InitializeApplication()
{
  // get the applicaiton instance if we don't have one already
  if (!this->ApplicationInstance)
    {
    // if we have a parent window get the app instance from it
    if (this->ParentId)
      {
      this->ApplicationInstance = (HINSTANCE)vtkGetWindowLong(this->ParentId,GWL_HINSTANCE);
      }
    else
      {
      this->ApplicationInstance = GetModuleHandle(NULL); /*AfxGetInstanceHandle();*/
      }
    }
}

void vtkWin32OpenGLRenderWindow::CreateAWindow(int x, int y, int width,
                                              int height)
{
  static int count=1;
  char *windowName;

  if (!this->WindowId)
    {
    WNDCLASS wndClass;
      
    int len = vtkString::Length( "Visualization Toolkit - Win32OpenGL #") 
      + (int)ceil( (double) log10( (double)(count+1) ) )
      + 1; 
    windowName = new char [ len ];
    sprintf(windowName,"Visualization Toolkit - Win32OpenGL #%i",count++);
    this->SetWindowName(windowName);
    delete [] windowName;
      
    // has the class been registered ?
    if (!GetClassInfo(this->ApplicationInstance,"vtkOpenGL",&wndClass))
      {
      wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
      wndClass.lpfnWndProc = vtkWin32OpenGLRenderWindow::WndProc;
      wndClass.cbClsExtra = 0;
      wndClass.hInstance = this->ApplicationInstance;
      wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
      wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
      wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
      wndClass.lpszMenuName = NULL;
      wndClass.lpszClassName = "vtkOpenGL";
      // vtk doesn't use the first extra 4 bytes, but app writers
      // may want them, so we provide them. VTK does use the second 
      // four bytes of extra space.
      wndClass.cbWndExtra = 8;
      RegisterClass(&wndClass);
      }
      
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
      DWORD style;
      if (this->Borders)
        {
        style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN /*| WS_CLIPSIBLINGS*/;
        }
      else
        {
        style = WS_POPUP | WS_CLIPCHILDREN /*| WS_CLIPSIBLINGS*/;
        }
      this->WindowId = CreateWindow(
        "vtkOpenGL", this->WindowName, style,
        x,y, width+2*GetSystemMetrics(SM_CXFRAME),
        height+2*GetSystemMetrics(SM_CYFRAME) +GetSystemMetrics(SM_CYCAPTION),
        NULL, NULL, this->ApplicationInstance, NULL);
      }
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
    vtkSetWindowLong(this->WindowId,4,(LONG)this);
    }
  this->DeviceContext = GetDC(this->WindowId);
  if (this->StereoCapableWindow)
    {
    this->SetupPixelFormat(this->DeviceContext, PFD_SUPPORT_OPENGL |
                           PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER |
                           PFD_STEREO, this->GetDebug(), 32, 32);
    }
  else
    {
    this->SetupPixelFormat(this->DeviceContext, PFD_SUPPORT_OPENGL |
                           PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER,
                           this->GetDebug(), 32, 32);
    }
  this->SetupPalette(this->DeviceContext);
  this->ContextId = wglCreateContext(this->DeviceContext);
  if (this->ContextId == NULL) 
    {
    vtkErrorMacro("wglCreateContext failed in CreateAWindow(), error: " << GetLastError());
    }
  this->MakeCurrent();
  this->OpenGLInit();
  this->Mapped = 1;
}

// Initialize the window for rendering.
void vtkWin32OpenGLRenderWindow::WindowInitialize()
{
  int x, y, width, height;
  x = ((this->Position[0] >= 0) ? this->Position[0] : 5);
  y = ((this->Position[1] >= 0) ? this->Position[1] : 5);
  height = ((this->Size[1] > 0) ? this->Size[1] : 300);
  width = ((this->Size[0] > 0) ? this->Size[0] : 300);

  
  // create our own window if not already set
  this->OwnWindow = 0;
  if (!this->MFChandledWindow)
    {
    this->InitializeApplication();
    this->CreateAWindow(x,y,width,height);
    }   
  else 
    {
    this->MakeCurrent(); // hsr
    this->OpenGLInit();
    }
  
  // set the DPI
  this->SetDPI(GetDeviceCaps(this->DeviceContext, LOGPIXELSY));
}

// Initialize the rendering window.
void vtkWin32OpenGLRenderWindow::Initialize (void)
{
  // make sure we havent already been initialized 
  if (this->ContextId)
    {
    return;
    }

  // now initialize the window
  if (this->OffScreenRendering)
    {
    this->InitializeApplication();
    }
  else
    {
    this->WindowInitialize();
    }
}


// Get the current size of the window.
int *vtkWin32OpenGLRenderWindow::GetSize(void)
{
  // if we aren't mapped then just return the ivar 
  if (this->Mapped)
    {
    RECT rect;

    //  Find the current window size 
    GetClientRect(this->WindowId, &rect);
    
    this->Size[0] = rect.right;
    this->Size[1] = rect.bottom;
    }

  return(this->vtkOpenGLRenderWindow::GetSize());
}

// Get the current size of the window.
int *vtkWin32OpenGLRenderWindow::GetScreenSize(void)
{
  RECT rect;

  SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);
                            
  this->Size[0] = rect.right - rect.left;
  this->Size[1] = rect.bottom - rect.top;
  
  return this->Size;
}

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

// Change the window to fill the entire screen.
void vtkWin32OpenGLRenderWindow::SetFullScreen(int arg)
{
  int *temp;
  
  if (this->FullScreen == arg)
    {
    return;
    }
  
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

//
// Set the variable that indicates that we want a stereo capable window
// be created. This method can only be called before a window is realized.
//
void vtkWin32OpenGLRenderWindow::SetStereoCapableWindow(int capable)
{
  if (this->WindowId == 0)
    {
    vtkRenderWindow::SetStereoCapableWindow(capable);
    }
  else
    {
    vtkWarningMacro(<< "Requesting a StereoCapableWindow must be performed "
                    << "before the window is realized, i.e. before a render.");
    }
}


// Set the preferred window size to full screen.
void vtkWin32OpenGLRenderWindow::PrefFullScreen()
{
  int *size;

  size = this->GetScreenSize();

  // use full screen 
  this->Position[0] = 0;
  this->Position[1] = 0;
  this->Size[0] = size[0] - 2*GetSystemMetrics(SM_CXFRAME);
  this->Size[1] = size[1] - 
    2*GetSystemMetrics(SM_CYFRAME) - GetSystemMetrics(SM_CYCAPTION);

  // don't show borders 
  this->Borders = 0;
}

// Remap the window.
void vtkWin32OpenGLRenderWindow::WindowRemap()
{
  short cur_light;

  /* first delete all the old lights */
  for (cur_light = GL_LIGHT0; cur_light < GL_LIGHT0+VTK_MAX_LIGHTS; cur_light++)
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
  this->Superclass::PrintSelf(os,indent);

  os << indent << "ContextId: " << this->ContextId << "\n";
  os << indent << "Next Window Id: " << this->NextWindowId << "\n";
  os << indent << "Window Id: " << this->WindowId << "\n";
}

// Get the window id.
HWND vtkWin32OpenGLRenderWindow::GetWindowId()
{
  vtkDebugMacro(<< "Returning WindowId of " << this->WindowId << "\n"); 

  return this->WindowId;
}

// Set the window id to a pre-existing window.
void vtkWin32OpenGLRenderWindow::SetWindowId(HWND arg)
{
  vtkDebugMacro(<< "Setting WindowId to " << arg << "\n"); 

  this->WindowId = arg;
}

// Set this RenderWindow's X window id to a pre-existing window.
void vtkWin32OpenGLRenderWindow::SetWindowInfo(char *info)
{
  int tmp;
  
  sscanf(info,"%i",&tmp);
 
  this->WindowId = (HWND)tmp;
  vtkDebugMacro(<< "Setting WindowId to " << this->WindowId << "\n"); 
}

// Sets the HWND id of the window that WILL BE created.
void vtkWin32OpenGLRenderWindow::SetParentInfo(char *info)
{
  int tmp;
  
  sscanf(info,"%i",&tmp);
 
  this->ParentId = (HWND)tmp;
  vtkDebugMacro(<< "Setting ParentId to " << this->ParentId << "\n"); 
}

// Set the window id to a pre-existing window.
void vtkWin32OpenGLRenderWindow::SetParentId(HWND arg)
{
  vtkDebugMacro(<< "Setting ParentId to " << arg << "\n"); 

  this->ParentId = arg;
}

// Set the window id of the new window once a WindowRemap is done.
void vtkWin32OpenGLRenderWindow::SetNextWindowId(HWND arg)
{
  vtkDebugMacro(<< "Setting NextWindowId to " << arg << "\n"); 

  this->NextWindowId = arg;
}

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


void vtkWin32OpenGLRenderWindow::SetOffScreenRendering(int offscreen)
{
  if (offscreen == this->OffScreenRendering)
    {
    return;
    }

  this->vtkRenderWindow::SetOffScreenRendering(offscreen);

  if (offscreen)
    {
    int size[2];
    size[0] = (this->Size[0] > 0) ? this->Size[0] : 300;
    size[1] = (this->Size[1] > 0) ? this->Size[1] : 300;

    HDC dc = CreateDC("DISPLAY", 0, 0, 0);
    this->SetupMemoryRendering(size[0], size[1], dc);
    DeleteDC(dc); 
    }
  else
    {
    if (!this->WindowId)
      {
      vtkRenderer* ren;
      this->CleanUpOffScreenRendering();
      this->WindowInitialize();
      for (this->Renderers->InitTraversal(); 
           (ren = this->Renderers->GetNextItem());)
        {
        ren->SetRenderWindow(this);
        }
      this->OpenGLInit();
      if (this->Interactor)
        {
        this->Interactor->ReInitialize();
        }
      this->DoubleBuffer = 1;
      }
    else
      {
      this->ResumeScreenRendering();
      }
    }
}

void vtkWin32OpenGLRenderWindow::CreateOffScreenDC(int xsize, int ysize,
                                                   HDC aHdc)
{
  int dataWidth = ((xsize*3+3)/4)*4;
  
  this->MemoryDataHeader.bmiHeader.biSize = 40;
  this->MemoryDataHeader.bmiHeader.biWidth = xsize;
  this->MemoryDataHeader.bmiHeader.biHeight = ysize;
  this->MemoryDataHeader.bmiHeader.biPlanes = 1;
  this->MemoryDataHeader.bmiHeader.biBitCount = 24;
  this->MemoryDataHeader.bmiHeader.biCompression = BI_RGB;
  this->MemoryDataHeader.bmiHeader.biClrUsed = 0;
  this->MemoryDataHeader.bmiHeader.biClrImportant = 0;
  this->MemoryDataHeader.bmiHeader.biSizeImage = dataWidth*ysize;
  this->MemoryDataHeader.bmiHeader.biXPelsPerMeter = 10000;
  this->MemoryDataHeader.bmiHeader.biYPelsPerMeter = 10000;
  
  HBITMAP dib = CreateDIBSection(aHdc,
                     &this->MemoryDataHeader, DIB_RGB_COLORS,
                     (void **)(&(this->MemoryData)),  NULL, 0);
  SIZE oldSize;
  SetBitmapDimensionEx(dib, xsize, ysize, &oldSize);
  // try using a DIBsection
  this->CreateOffScreenDC(dib, aHdc);
}

void vtkWin32OpenGLRenderWindow::CreateOffScreenDC(HBITMAP hbmp, HDC aHdc)
{
  BITMAP bm;
  GetObject(hbmp, sizeof(BITMAP), &bm);

  this->MemoryBuffer = hbmp;
  
  // Create a compatible device context
  this->MemoryHdc = (HDC)CreateCompatibleDC(aHdc);
  
  // Put the bitmap into the device context
  SelectObject(this->MemoryHdc, this->MemoryBuffer);
  
  // we need to release resources
  vtkRenderer *ren;
  for (this->Renderers->InitTraversal(); (ren = this->Renderers->GetNextItem());)
    {
    ren->SetRenderWindow(NULL);
    }

  // adjust settings for renderwindow
  this->Mapped =0;
  this->Size[0] = bm.bmWidth;
  this->Size[1] = bm.bmHeight;
  
  this->DeviceContext = this->MemoryHdc;
  this->DoubleBuffer = 0;
  this->SetupPixelFormat(this->DeviceContext, 
                         PFD_SUPPORT_OPENGL | PFD_SUPPORT_GDI | 
                         PFD_DRAW_TO_BITMAP, this->GetDebug(), 24, 32);
  this->SetupPalette(this->DeviceContext);
  this->ContextId = wglCreateContext(this->DeviceContext);
  if (this->ContextId == NULL) 
    {
    vtkErrorMacro("wglCreateContext failed in CreateOffScreenDC(), error: " << GetLastError());
    }
  this->MakeCurrent();
  
  // we need to release resources
  for (this->Renderers->InitTraversal(); 
       (ren = this->Renderers->GetNextItem());)
    {
    ren->SetRenderWindow(this);
    }
  
  this->OpenGLInit();
}

void vtkWin32OpenGLRenderWindow::SetupMemoryRendering(int xsize, int ysize,
                                                      HDC aHdc)
{
  // save the current state
  this->ScreenMapped = this->Mapped;
  this->ScreenWindowSize[0] = this->Size[0];
  this->ScreenWindowSize[1] = this->Size[1];
  this->ScreenDeviceContext = this->DeviceContext;
  this->ScreenDoubleBuffer = this->DoubleBuffer;
  this->ScreenContextId = this->ContextId;

  this->CreateOffScreenDC(xsize, ysize, aHdc);
}

void vtkWin32OpenGLRenderWindow::SetupMemoryRendering(HBITMAP hbmp)
{
  HDC dc = CreateDC("DISPLAY", 0, 0, 0);

  // save the current state
  this->ScreenMapped = this->Mapped;
  this->ScreenWindowSize[0] = this->Size[0];
  this->ScreenWindowSize[1] = this->Size[1];
  this->ScreenDeviceContext = this->DeviceContext;
  this->ScreenDoubleBuffer = this->DoubleBuffer;
  this->ScreenContextId = this->ContextId;

  this->CreateOffScreenDC(hbmp, dc);
  DeleteDC(dc); 
}

HDC vtkWin32OpenGLRenderWindow::GetMemoryDC()
{
  return this->MemoryHdc;
}

void vtkWin32OpenGLRenderWindow::CleanUpOffScreenRendering(void)
{
  GdiFlush();
  
  // we need to release resources
  vtkRenderer *ren;
  for (this->Renderers->InitTraversal(); 
       (ren = this->Renderers->GetNextItem());)
    {
    ren->SetRenderWindow(NULL);
    }
  DeleteDC(this->MemoryHdc);
  DeleteObject(this->MemoryBuffer);
  if (wglDeleteContext(this->ContextId) != TRUE) 
    {
    vtkErrorMacro("wglDeleteContext failed in CleanUpOffScreenRendering(), error: " << GetLastError());
    }
}

void vtkWin32OpenGLRenderWindow::ResumeScreenRendering(void)
{
  this->CleanUpOffScreenRendering();
  this->Mapped = this->ScreenMapped;
  this->Size[0] = this->ScreenWindowSize[0];
  this->Size[1] = this->ScreenWindowSize[1];
  this->DeviceContext = this->ScreenDeviceContext;
  this->DoubleBuffer = this->ScreenDoubleBuffer;
  this->ContextId = this->ScreenContextId;
  this->MakeCurrent();

  vtkRenderer* ren;
  for (this->Renderers->InitTraversal();
       (ren = this->Renderers->GetNextItem());)
    {
    ren->SetRenderWindow(this);
    }
}

void vtkWin32OpenGLRenderWindow::SetContextId(HGLRC arg) // hsr
{                                                                                                          // hsr       
  this->ContextId = arg;                                                           // hsr
}                                                                                                          // hsr

void vtkWin32OpenGLRenderWindow::SetDeviceContext(HDC arg) // hsr
{                                                                                                                // hsr
  this->DeviceContext = arg;                                                     // hsr
  this->MFChandledWindow = TRUE;                                                 // hsr
}                                                                                                                // hsr

//----------------------------------------------------------------------------
void vtkWin32OpenGLRenderWindow::HideCursor()
{
  if (this->CursorHidden)
    {
    return;
    }
  this->CursorHidden = 1;

  ::ShowCursor(!this->CursorHidden);
}

//----------------------------------------------------------------------------
void vtkWin32OpenGLRenderWindow::ShowCursor()
{
  if (!this->CursorHidden)
    {
    return;
    }
  this->CursorHidden = 0;

  ::ShowCursor(!this->CursorHidden);
}                                  

//----------------------------------------------------------------------------
void vtkWin32OpenGLRenderWindow::SetCursorPosition(int x, int y) 
{
  int *size = this->GetSize();

  POINT point;
  point.x = x;
  point.y = size[1] - y - 1;

  if (ClientToScreen(this->WindowId, &point))
    {
    SetCursorPos(point.x, point.y);
    }
};

void vtkWin32OpenGLRenderWindow::SetCurrentCursor(int shape)
{
  this->Superclass::SetCurrentCursor(shape);
  LPCTSTR cursorName = 0;
  switch (shape)
    {
    case VTK_CURSOR_DEFAULT:
    case VTK_CURSOR_ARROW:
      cursorName = IDC_ARROW;
      break;
    case VTK_CURSOR_SIZENE:
    case VTK_CURSOR_SIZESW:
      cursorName = IDC_SIZENESW;
      break;
    case VTK_CURSOR_SIZENW:
    case VTK_CURSOR_SIZESE:
      cursorName = IDC_SIZENWSE;
      break;
    case VTK_CURSOR_SIZENS:
      cursorName = IDC_SIZENS;
      break;
    case VTK_CURSOR_SIZEWE:
      cursorName = IDC_SIZEWE;
      break;
    case VTK_CURSOR_SIZEALL:
      cursorName = IDC_SIZEALL;
      break;
    case VTK_CURSOR_HAND:
#if(WINVER >= 0x0500)      
      cursorName = IDC_HAND;
#else
      cursorName = IDC_ARROW;
#endif
      break;
    }
  
  if (cursorName)
    {
    HANDLE cursor = 
      LoadImage(0,cursorName,IMAGE_CURSOR,0,0,LR_SHARED | LR_DEFAULTSIZE);
    SetCursor((HCURSOR)cursor);
    }
}
