/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1995 Ken Martin, Will Schroeder, Bill Lorensen.

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
#include <math.h>
#include <stdlib.h>
#include <iostream.h>
#include "vtkXRenderWindow.hh"
#include "vtkXRenderWindowInteractor.hh"

vtkXRenderWindow::vtkXRenderWindow()
{
  this->DisplayId = (Display *)NULL;
  this->WindowId = (Window)NULL;
  this->NextWindowId = (Window)NULL;
  this->ColorMap = (Colormap)NULL;
}

vtkXRenderWindow::~vtkXRenderWindow()
{
  if (this->Interactor) this->Interactor->Delete();
}

// Description:
// Get the size of the screen in pixels
int *vtkXRenderWindow::GetScreenSize()
{
  // get the default display connection 
  if (!this->DisplayId)
    {
    this->DisplayId = XOpenDisplay((char *)NULL); 
    if (this->DisplayId == NULL) 
      {
      vtkErrorMacro(<< "bad X server connection.\n");
      }
    }

  this->ScreenSize[0] = 
    DisplayWidth(this->DisplayId, DefaultScreen(this->DisplayId));
  this->ScreenSize[1] = 
    DisplayHeight(this->DisplayId, DefaultScreen(this->DisplayId));

  return this->ScreenSize;
}

// Description:
// Get the current size of the window in pixels.
int *vtkXRenderWindow::GetSize(void)
{
  XWindowAttributes attribs;

  // if we aren't mapped then just return the ivar 
  if (!this->Mapped)
    {
    return(this->Size);
    }

  //  Find the current window size 
  XGetWindowAttributes(this->DisplayId, 
		       this->WindowId, &attribs);

  this->Size[0] = attribs.width;
  this->Size[1] = attribs.height;
  
  return this->Size;
}

// Description:
// Get the position in screen coordinates (pixels) of the window.
int *vtkXRenderWindow::GetPosition(void)
{
  XWindowAttributes attribs;
  int x,y;
  Window child;
  
  // if we aren't mapped then just return the ivar 
  if (!this->Mapped)
    {
    return(this->Position);
    }

  //  Find the current window size 
  XGetWindowAttributes(this->DisplayId, this->WindowId, &attribs);
  x = attribs.x;
  y = attribs.y;

  XTranslateCoordinates(this->DisplayId,this->WindowId,
		 RootWindowOfScreen(ScreenOfDisplay(this->DisplayId,0)),
			x,y,&this->Position[0],&this->Position[1],&child);

  return this->Position;
}

// Description:
// Get this RenderWindow's X display id.
Display *vtkXRenderWindow::GetDisplayId()
{
  vtkDebugMacro(<< "Returning DisplayId of " << (void *)this->DisplayId << "\n"); 

  return this->DisplayId;
}

// Description:
// Get this RenderWindow's X window id.
Window vtkXRenderWindow::GetWindowId()
{
  vtkDebugMacro(<< "Returning WindowId of " << (void *)this->WindowId << "\n"); 

  return this->WindowId;
}

// Description:
// Set this RenderWindow's X window id to a pre-existing window.
void vtkXRenderWindow::SetWindowId(Window arg)
{
  vtkDebugMacro(<< "Setting WindowId to " << (void *)arg << "\n"); 

  this->WindowId = arg;
}
void vtkXRenderWindow::SetWindowId(void *arg)
{
  this->SetWindowId((Window)arg);
}

// Description:
// Specify the X window id to use if a WindowRemap is done.
void vtkXRenderWindow::SetNextWindowId(Window arg)
{
  vtkDebugMacro(<< "Setting NextWindowId to " << (void *)arg << "\n"); 

  this->NextWindowId = arg;
}

// Description:
// Set the X display id for this RenderWindow to use to a pre-existing 
// X display id.
void vtkXRenderWindow::SetDisplayId(Display  *arg)
{
  vtkDebugMacro(<< "Setting DisplayId to " << (void *)arg << "\n"); 

  this->DisplayId = arg;
}
void vtkXRenderWindow::SetDisplayId(void *arg)
{
  this->SetDisplayId((Display *)arg);
}

// Description:
// Create an interactor that will work with this renderer.
// Since all subclasses of this class will be running on
// machines that are running X Windows. The correct vtkRenderWindowInteractor
// is the vtkXRenderWindowInteractor. So this object creates one, then type 
// casts it and returns a pointer to it.
vtkRenderWindowInteractor *vtkXRenderWindow::MakeRenderWindowInteractor()
{
  this->Interactor = (vtkRenderWindowInteractor *)new vtkXRenderWindowInteractor;
  this->Interactor->SetRenderWindow((vtkRenderWindow *)this);
  return this->Interactor;
}

void vtkXRenderWindow::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkRenderWindow::PrintSelf(os,indent);

  os << indent << "Color Map: " << this->ColorMap << "\n";
  os << indent << "Display Id: " << this->GetDisplayId() << "\n";
  os << indent << "Next Window Id: " << this->NextWindowId << "\n";
  os << indent << "Window Id: " << this->GetWindowId() << "\n";
}

