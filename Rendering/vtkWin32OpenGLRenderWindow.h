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
// .NAME vtkWin32OpenGLRenderWindow - OpenGL rendering window
// .SECTION Description
// vtkWin32OpenGLRenderWindow is a concrete implementation of the abstract class
// vtkRenderWindow. vtkWin32OpenGLRenderer interfaces to the standard
// OpenGL graphics library in the Windows/NT environment..

#ifndef __vtkWin32OpenGLRenderWindow_h
#define __vtkWin32OpenGLRenderWindow_h

#include <stdlib.h>
#include "vtkRenderWindow.h"

class VTK_EXPORT vtkWin32OpenGLRenderWindow : public vtkRenderWindow
{
public:
  HINSTANCE ApplicationInstance;
  HPALETTE  Palette;
  HGLRC     ContextId;
  HDC	      DeviceContext;			//	hsr	
  BOOL      MFChandledWindow;		//  hsr
  HWND      WindowId;
  HWND      ParentId;
  HWND      NextWindowId;
  int       OwnWindow;
  int       ScreenSize[2];
  int       MultiSamples;
  // Pen for 2D graphics line drawing
  HPEN Pen;

  // Pen color for the graphics;
  COLORREF PenColor;
public:
  vtkWin32OpenGLRenderWindow();
  ~vtkWin32OpenGLRenderWindow();
  static vtkWin32OpenGLRenderWindow *New() {return new vtkWin32OpenGLRenderWindow;};
  const char *GetClassName() {return "vtkWin32OpenGLRenderWindow";};
  void PrintSelf(ostream& os, vtkIndent indent);
  
  void Start(void);
  void Frame(void);
  void WindowConfigure(void);
  void WindowInitialize(void);
  void Initialize(void);
  virtual void SetFullScreen(int);
  void WindowRemap(void);
  void PrefFullScreen(void);
  void SetSize(int,int);
  int *GetSize();
  void SetPosition(int,int);
  int *GetScreenSize();
  int *GetPosition();

  virtual void SetWindowName(char *);
  
  void SetWindowInfo(char *);
  //BTX
  virtual void *GetGenericDisplayId() {return NULL;};
  virtual void *GetGenericWindowId()  {return (void *)this->WindowId;};
  virtual void *GetGenericParentId()  {return (void *)this->ParentId;};
  virtual void *GetGenericContext()   {return (void *)this->DeviceContext;};
  virtual void SetDisplayId(void *) {};
  
  HWND  GetWindowId();
  void  SetWindowId(void *foo) {this->SetWindowId((HWND)foo);};
  void  SetWindowId(HWND);
  void  SetParentId(void *foo) {this->SetParentId((HWND)foo);};
  void  SetParentId(HWND);
  void  SetContextId(HGLRC);	// hsr
  void  SetDeviceContext(HDC);	// hsr
  void  SetNextWindowId(HWND);
  //ETX

  // supply base class virtual function
  vtkSetMacro(MultiSamples,int);
  vtkGetMacro(MultiSamples,int);

  // stereo rendering stuff
  virtual void StereoUpdate();
  
  // Description:
  // Set/Get the pixel data of an image, transmitted as RGBRGB... 
  virtual unsigned char *GetPixelData(int x,int y,int x2,int y2,int front);
  virtual void SetPixelData(int x,int y,int x2,int y2,unsigned char *,int front);

  // Description:
  // Set/Get the pixel data of an image, transmitted as RGBARGBA... 
  virtual float *GetRGBAPixelData(int x,int y,int x2,int y2,int front);
  virtual void SetRGBAPixelData(int x,int y,int x2,int y2,float *,int front,
                                int blend=0);
  virtual void ReleaseRGBAPixelData(float *data);

  // Description:
  // Set/Get the zbuffer data from an image
  virtual float *GetZbufferData( int x1, int y1, int x2, int y2 );
  virtual void SetZbufferData( int x1, int y1, int x2, int y2, float *buffer );

  void MakeCurrent();
  virtual  int GetEventPending();
  void PenLineTo(int x,int y);
  void PenMoveTo(int x,int y);
  void SetPenColor(float r,float g,float b);

};


#endif

