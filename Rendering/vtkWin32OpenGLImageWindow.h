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
// .NAME vtkWin32OpenGLImageWindow - OpenGL Imageing window
// .SECTION Description
// vtkWin32OpenGLImageWindow is a concrete implementation of the abstract
// class vtkImageWindow. vtkWin32OpenGLImageer interfaces to the standard
// OpenGL graphics library in the Windows/NT environment..

#ifndef __vtkWin32OpenGLImageWindow_h
#define __vtkWin32OpenGLImageWindow_h

#include <stdlib.h>
#include "vtkImageWindow.h"
#include "vtkMutexLock.h"

class VTK_EXPORT vtkWin32OpenGLImageWindow : public vtkImageWindow
{
public:
  static vtkWin32OpenGLImageWindow *New() {
    return new vtkWin32OpenGLImageWindow;};
  const char *GetClassName() {return "vtkWin32OpenGLImageWindow";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // output to the viewer.
  vtkWin32OpenGLImageWindow *GetOutput(){return this;};

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
  // Set the size of the window.
  virtual void SetSize(int,int);

  // Description:
  // Get the current size of the window.
  virtual int *GetSize();

  // Description:
  // Set the position of the window.
  virtual void SetPosition(int,int);
  
  // Description:
  // Get the position in screen coordinates of the window.
  virtual int *GetPosition();

  // Description:
  // Set the name of the window. This appears at the top of the window
  // normally.
  virtual void SetWindowName(char *);
  
  //BTX
  virtual void *GetGenericDisplayId() {return (void *)this->ContextId;};
  virtual void *GetGenericWindowId()  {return (void *)this->WindowId;};
  virtual void *GetGenericParentId()  {return (void *)this->ParentId;};
  virtual void *GetGenericContext()   {return (void *)this->DeviceContext;};
  virtual void SetDisplayId(void *) {};

  // Description:
  // Get the window id.
  virtual HWND  GetWindowId();
  void  SetWindowId(void *foo) {this->SetWindowId((HWND)foo);};

  // Description:
  // Set the window id to a pre-existing window.
  virtual void  SetWindowId(HWND);
  
  // Description:
  // Set the window's parent id to a pre-existing window.
  virtual void  SetParentId(HWND);
  void  SetParentId(void *foo) {this->SetParentId((HWND)foo);};

  void  SetContextId(HGLRC);	// hsr
  void  SetDeviceContext(HDC);	// hsr

  // Description:
  // Set the window id of the new window once a WindowRemap is done.
  virtual void  SetNextWindowId(HWND);
  //ETX

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
  virtual void ReleaseRGBAPixelData(float *data);

  // Description:
  // Make this windows OpenGL context the current context.
  void MakeCurrent();

  // Description:
  // These methods can be used by MFC applications 
  // to support print preview and printing, or more
  // general rendering into memory. 
  void SetupMemoryRendering(int x, int y, HDC prn);
  void ResumeScreenRendering();
  HDC GetMemoryDC();
  unsigned char *GetMemoryData(){return this->MemoryData;};  
  
  // Description:
  // Initialize OpenGL for this window.
  virtual void OpenGLInit();
  virtual void SetupPalette(HDC hDC);
  virtual void SetupPixelFormat(HDC hDC, DWORD dwFlags, int debug, 
				int bpp=16, int zbpp=16);
  
  // Description:
  // Clean up device contexts, rendering contexts, etc.
  void Clean();

protected:
  vtkWin32OpenGLImageWindow();
  ~vtkWin32OpenGLImageWindow();

  HINSTANCE ApplicationInstance;
  HPALETTE  Palette;
  HPALETTE  OldPalette;
  HGLRC     ContextId;
  HDC       DeviceContext;
  HWND      WindowId;
  HWND      ParentId;
  HWND      NextWindowId;
  int       OwnWindow;
  int       ScreenSize[2];

  // the following is used to support rendering into memory
  BITMAPINFO MemoryDataHeader;
  HBITMAP MemoryBuffer;
  unsigned char *MemoryData;	// the data in the DIBSection
  HDC MemoryHdc;

  int ScreenMapped;
  int ScreenWindowSize[2];
  HDC ScreenDeviceContext;
  int ScreenDoubleBuffer;
  HGLRC ScreenContextId;

  //BTX
  // message handler
  virtual LRESULT MessageProc(HWND hWnd, UINT message, 
			      WPARAM wParam, LPARAM lParam);

  static LRESULT APIENTRY WndProc(HWND hWnd, UINT message, 
				  WPARAM wParam, LPARAM lParam);
  //ETX
  
  // these are used in the window creation code
  static vtkWin32OpenGLImageWindow *TempPointerToThis;
  static vtkMutexLock *WindowMutex;
};


#endif

