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
// .NAME vtkWindow - window superclass for ImageWindow and RenderWindow
// .SECTION Description
// vtkWindow is an abstract object to specify the behavior of a
// rendering or imaging window. It contains vtkViewports.

// .SECTION see also
// vtkImageWindow vtkRenderWindow vtkViewport

#ifndef __vtkWindow_h
#define __vtkWindow_h

#include "vtkObject.h"
#include <stdio.h>

class VTK_EXPORT vtkWindow : public vtkObject
{
public:
  vtkTypeMacro(vtkWindow,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // These are window system independent methods that are used
  // to help interface vtkWindow to native windowing systems.
  virtual void SetDisplayId(void *) = 0;
  virtual void SetWindowId(void *)  = 0;
  virtual void SetParentId(void *)  = 0;
  virtual void *GetGenericDisplayId() = 0;
  virtual void *GetGenericWindowId()  = 0;
  virtual void *GetGenericParentId()  = 0;
  virtual void *GetGenericContext()   = 0;
  virtual void *GetGenericDrawable()  = 0;  
  virtual void SetWindowInfo(char *) = 0;

  // Description:
  // Set/Get the position in screen coordinates of the rendering window.
  virtual int *GetPosition();
  virtual void SetPosition(int,int);
  virtual void SetPosition(int a[2]);

  // Description:
  // Set/Get the size of the window in screen coordinates.
  virtual int *GetSize();
  virtual void SetSize(int,int);
  virtual void SetSize(int a[2]);

  // Description:
  // Keep track of whether the rendering window has been mapped to screen.
  vtkSetMacro(Mapped,int);
  vtkGetMacro(Mapped,int);
  vtkBooleanMacro(Mapped,int);

  // Description:
  // Turn on/off erasing the screen between images. This allows multiple 
  // exposure sequences if turned on. You will need to turn double 
  // buffering off or make use of the SwapBuffers methods to prevent
  // you from swapping buffers between exposures.
  vtkSetMacro(Erase,int);
  vtkGetMacro(Erase,int);
  vtkBooleanMacro(Erase,int);

  // Description:
  // Keep track of whether double buffering is on or off
  vtkSetMacro(DoubleBuffer,int);
  vtkGetMacro(DoubleBuffer,int);
  vtkBooleanMacro(DoubleBuffer,int);

  // Description:
  // Get name of rendering window
  vtkGetStringMacro(WindowName);
  virtual void SetWindowName(char * );

  // Description:
  // Ask each viewport owned by this Window to render its image and 
  // synchronize this process.
  virtual void Render() = 0;

  // Description:
  // Get the pixel data of an image, transmitted as RGBRGBRGB. The
  // front argument indicates if the front buffer should be used or the back 
  // buffer. It is the caller's responsibility to delete the resulting 
  // array. It is very important to realize that the memory in this array
  // is organized from the bottom of the window to the top. The origin
  // of the screen is in the lower left corner. The y axis increases as
  // you go up the screen. So the storage of pixels is from left to right
  // and from bottom to top.
  virtual unsigned char *GetPixelData(int, int, int, int, int) {
    return (unsigned char *)NULL;};

  // Description:
  // Return a best estimate to the dots per inch of the display
  // device being rendered (or printed).
  vtkGetMacro(DPI,int);
  vtkSetClampMacro(DPI,int,1,3000);
  
  // Description:
  // Create a window in memory instead of on the screen. This may not be
  // supported for every type of window and on some windows you may need to
  // invoke this prior to the first render.
  vtkSetMacro(OffScreenRendering,int);
  vtkGetMacro(OffScreenRendering,int);
  vtkBooleanMacro(OffScreenRendering,int);

protected:
  int OffScreenRendering;
  vtkWindow();
  ~vtkWindow();
  vtkWindow(const vtkWindow&) {};
  void operator=(const vtkWindow&) {};

  char *WindowName;
  int Size[2];
  int Position[2];
  int Mapped;
  int Erase;
  int DoubleBuffer;
  int DPI;
};

#endif


