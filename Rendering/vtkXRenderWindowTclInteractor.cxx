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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/X.h>
#include <X11/keysym.h>
#include "vtkXRenderWindowTclInteractor.h"
#include "vtkInteractorStyle.h"
#include "vtkXRenderWindow.h"
#include "vtkActor.h"
#include <X11/Shell.h>
#include <math.h>
#include "tk.h"
#include "vtkActorCollection.h"
#include "vtkPoints.h"

// steal the first two elements of the TkMainInfo stuct
// we don't care about the rest of the elements.
struct TkMainInfo
{
  int refCount;
  struct TkWindow *winPtr;
};

#if ((TK_MAJOR_VERSION <= 4)||((TK_MAJOR_VERSION == 8)&&(TK_MINOR_VERSION == 0)))
extern TkMainInfo *tkMainWindowList;
#else
extern "C" {TkMainInfo *TkGetMainInfoList();}
#endif

// returns 1 if done
static int vtkTclEventProc(XtPointer clientData,XEvent *event)
{
  Boolean ctd;
  vtkXRenderWindow *rw;
      
  rw = (vtkXRenderWindow *)
    (((vtkXRenderWindowTclInteractor *)clientData)->GetRenderWindow());
  
  if (rw->GetWindowId() == ((XAnyEvent *)event)->window)
    {
    vtkXRenderWindowTclInteractorCallback((Widget)NULL,clientData, event, &ctd);
    ctd = 0;
    }
  else
    {
    ctd = 1;
    }

  return !ctd;
}

static void vtkXTclTimerProc(ClientData clientData)
{
  XtIntervalId id;
  vtkXRenderWindowTclInteractorTimer((XtPointer)clientData,&id);
}



// Construct object so that light follows camera motion.
vtkXRenderWindowTclInteractor::vtkXRenderWindowTclInteractor()
{
  this->App = 0;
  this->top = 0;
  this->TopLevelShell = NULL;
}

vtkXRenderWindowTclInteractor::~vtkXRenderWindowTclInteractor()
{
  if (this->Initialized)
    {
    Tk_DeleteGenericHandler((Tk_GenericProc *)vtkTclEventProc,
			    (ClientData)this);
    }
}

void  vtkXRenderWindowTclInteractor::SetWidget(Widget foo)
{
  this->top = foo;
} 

// This method will store the top level shell widget for the interactor.
// This method and the method invocation sequence applies for:
//     1 vtkRenderWindow-Interactor pair in a nested widget heirarchy
//     multiple vtkRenderWindow-Interactor pairs in the same top level shell
// It is not needed for
//     1 vtkRenderWindow-Interactor pair as the direct child of a top level shell
//     multiple vtkRenderWindow-Interactor pairs, each in its own top level shell
//
// The method, along with EnterNotify event, changes the keyboard focus among
// the widgets/vtkRenderWindow(s) so the Interactor(s) can receive the proper
// keyboard events. The following calls need to be made:
//     vtkRenderWindow's display ID need to be set to the top level shell's
//           display ID.
//     vtkXRenderWindowTclInteractor's Widget has to be set to the vtkRenderWindow's
//           container widget
//     vtkXRenderWindowTclInteractor's TopLevel has to be set to the top level
//           shell widget
// note that the procedure for setting up render window in a widget needs to
// be followed.  See vtkRenderWindowInteractor's SetWidget method.
//
// If multiple vtkRenderWindow-Interactor pairs in SEPARATE windows are desired,
// do not set the display ID (Interactor will create them as needed.  Alternatively,
// create and set distinct DisplayID for each vtkRenderWindow. Using the same
// display ID without setting the parent widgets will cause the display to be
// reinitialized every time an interactor is initialized), do not set the
// widgets (so the render windows would be in their own windows), and do
// not set TopLevelShell (each has its own top level shell already)
void vtkXRenderWindowTclInteractor::SetTopLevelShell(Widget topLevel)
{
  this->TopLevelShell = topLevel;
}


void  vtkXRenderWindowTclInteractor::Start()
{
  Tk_MainLoop();
}

// Initializes the event handlers
void vtkXRenderWindowTclInteractor::Initialize(XtAppContext app)
{
  this->App = app;

  this->Initialize();
}

// Begin processing keyboard strokes.
void vtkXRenderWindowTclInteractor::Initialize()
{
  vtkXRenderWindow *ren;
  int *size;

  // make sure we have a RenderWindow and camera
  if ( ! this->RenderWindow)
    {
    vtkErrorMacro(<<"No renderer defined!");
    return;
    }

  this->Initialized = 1;
  ren = (vtkXRenderWindow *)(this->RenderWindow);

  // use the same display as tcl/tk
#if ((TK_MAJOR_VERSION <= 4)||((TK_MAJOR_VERSION == 8)&&(TK_MINOR_VERSION == 0)))
  ren->SetDisplayId(Tk_Display(tkMainWindowList->winPtr));
#else
  ren->SetDisplayId(Tk_Display(TkGetMainInfoList()->winPtr));
#endif
  this->DisplayId = ren->GetDisplayId();
  
  // get the info we need from the RenderingWindow
  size    = ren->GetSize();
  
  size = ren->GetSize();
  ren->Start();
  this->WindowId = ren->GetWindowId();
  size = ren->GetSize();

  this->Size[0] = size[0];
  this->Size[1] = size[1];

  this->Enable();

  // Set the event handler
  Tk_CreateGenericHandler((Tk_GenericProc *)vtkTclEventProc,(ClientData)this);
}


void vtkXRenderWindowTclInteractor::Enable()
{
  // avoid cycles of calling Initialize() and Enable()
  if (this->Enabled)
    {
    return;
    }

  // Select the events that we want to respond to
  // (Multiple calls to XSelectInput overrides the previous settings)
  XSelectInput(this->DisplayId,this->WindowId,
	       KeyPressMask | ButtonPressMask | ExposureMask |
	       StructureNotifyMask | ButtonReleaseMask | EnterWindowMask);

  this->Enabled = 1;

  this->Modified();
}

void vtkXRenderWindowTclInteractor::Disable()
{
  if (!this->Enabled)
    {
    return;
    }
  
  // Remove the all the events that we registered for EXCEPT for
  // StructureNotifyMask event since we need to keep track of the window
  // size (we will not render if we are disabled, we simply track the window
  // size changes for a possible Enable()). Expose events are disabled.
  // (Multiple calls to XSelectInput overrides the previous settings)
  XSelectInput(this->DisplayId,this->WindowId,
	       StructureNotifyMask );

  this->Enabled = 0;
  this->Modified();
}


void vtkXRenderWindowTclInteractor::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkRenderWindowInteractor::PrintSelf(os,indent);
  if (this->App)
    {
    os << indent << "App: " << this->App << "\n";
    }
  else
    {
    os << indent << "App: (none)\n";
    }
}


void  vtkXRenderWindowTclInteractor::UpdateSize(int x,int y)
{
  // if the size changed send this on to the RenderWindow
  if ((x != this->Size[0])||(y != this->Size[1]))
    {
    this->Size[0] = x;
    this->Size[1] = y;
    this->RenderWindow->SetSize(x,y);
    }

} 


void vtkXRenderWindowTclInteractorCallback(Widget vtkNotUsed(w),
					XtPointer client_data, 
					XEvent *event, 
					Boolean *vtkNotUsed(ctd))
{
  vtkXRenderWindowTclInteractor *me;
  
  me = (vtkXRenderWindowTclInteractor *)client_data;
  int xp,yp;
  
  switch (event->type) 
    {
    case Expose:
      {
      XEvent result;
      while (XCheckTypedWindowEvent(me->DisplayId, me->WindowId,
				    Expose, &result))
	{
	// just getting the expose configure event
	event = &result;
	}
      // only render if we are currently accepting events
      if (me->GetEnabled())
	{
	me->GetRenderWindow()->Render();
	}
      }
      break;

    case ConfigureNotify: 
      {
      XEvent result;
      while (XCheckTypedWindowEvent(me->DisplayId, me->WindowId,
				    ConfigureNotify, &result))
	{
	// just getting the last configure event
	event = &result;
	}
      if ((((XConfigureEvent *)event)->width != me->Size[0]) ||
	  (((XConfigureEvent *)event)->height != me->Size[1]))
	{
	me->UpdateSize(((XConfigureEvent *)event)->width,
		       ((XConfigureEvent *)event)->height); 
	
	// only render if we are currently accepting events
	if (me->GetEnabled())
	  {
	  me->GetRenderWindow()->Render();
	  }
	}
      }
      break;
      
    case ButtonPress: 
      {
      if (!me->Enabled) return;
      int ctrl = 0;
      if (((XButtonEvent *)event)->state & ControlMask)
        {
	ctrl = 1;
        }
      int shift = 0;
      if (((XButtonEvent *)event)->state & ShiftMask)
        {
	shift = 1;
        }
      xp = ((XButtonEvent*)event)->x;
      yp = me->Size[1] - ((XButtonEvent*)event)->y - 1;
      switch (((XButtonEvent *)event)->button)
	{
	case Button1: 
	  me->InteractorStyle->OnLeftButtonDown(ctrl, shift, xp, yp);
	  break;
	case Button2: 
	  me->InteractorStyle->OnMiddleButtonDown(ctrl, shift, xp, yp);
	  break;
	case Button3: 
	  me->InteractorStyle->OnRightButtonDown(ctrl, shift, xp, yp);
	  break;
	}
      }
      break;
      
    case ButtonRelease: 
      {
      if (!me->Enabled) return;
      int ctrl = 0;
      if (((XButtonEvent *)event)->state & ControlMask)
        {
	ctrl = 1;
        }
      int shift = 0;
      if (((XButtonEvent *)event)->state & ShiftMask)
        {
	shift = 1;
        }
      xp = ((XButtonEvent*)event)->x;
      yp = me->Size[1] - ((XButtonEvent*)event)->y - 1;
      switch (((XButtonEvent *)event)->button)
	{
	case Button1: 
	  me->InteractorStyle->OnLeftButtonUp(ctrl, shift, xp, yp);
	  break;
	case Button2: 
	  me->InteractorStyle->OnMiddleButtonUp(ctrl, shift, xp, yp);
	  break;
	case Button3: 
	  me->InteractorStyle->OnRightButtonUp(ctrl, shift, xp, yp);
	  break;
	}
      }
      break;

    case EnterNotify:
      {
      // Force the keyboard focus to be this render window
      if (me->TopLevelShell != NULL)
        {
        XtSetKeyboardFocus(me->TopLevelShell, me->top);
        }
      }
      break;

    case KeyPress:
      {
      int ctrl = 0;
      if (((XKeyEvent *)event)->state & ControlMask)
        {
	ctrl = 1;
        }
      int shift = 0;
      if (((XKeyEvent *)event)->state & ShiftMask)
        {
	shift = 1;
        }
      KeySym ks;
      static char buffer[20];
      buffer[0] = '\0';
      XLookupString((XKeyEvent *)event,buffer,20,&ks,NULL);
      xp = ((XKeyEvent*)event)->x;
      yp = me->Size[1] - ((XKeyEvent*)event)->y - 1;
      if (!me->Enabled) return;
      me->InteractorStyle->OnMouseMove(0,0,xp,yp);
      me->InteractorStyle->OnChar(ctrl, shift, buffer[0], 1);
      }
      break;      
      
    case MotionNotify: 
      {
      if (!me->Enabled) return;
      int ctrl = 0;
      if (((XMotionEvent *)event)->state & ControlMask)
        {
	ctrl = 1;
        }
      int shift = 0;
      if (((XMotionEvent *)event)->state & ShiftMask)
        {
	shift = 1;
        }
      xp = ((XMotionEvent*)event)->x;
      yp = me->Size[1] - ((XMotionEvent*)event)->y - 1;
      me->InteractorStyle->OnMouseMove(ctrl, shift, xp, yp);
      }
      break;
    }
}

void vtkXRenderWindowTclInteractorTimer(XtPointer client_data,
				     XtIntervalId *vtkNotUsed(id))
{
  vtkXRenderWindowTclInteractor *me;
  me = (vtkXRenderWindowTclInteractor *)client_data;
  Window root,child;
  int root_x,root_y;
  int x,y;
  unsigned int keys;

  // get the pointer position
  XQueryPointer(me->DisplayId,me->WindowId,
		&root,&child,&root_x,&root_y,&x,&y,&keys);
  if (!me->Enabled) return;
  me->InteractorStyle->OnMouseMove(0,0,x,me->Size[1] - y);
  me->InteractorStyle->OnTimer();
}

int vtkXRenderWindowTclInteractor::CreateTimer(int timertype) 
{
  Tk_CreateTimerHandler(10,vtkXTclTimerProc,(ClientData)this);
  return 1;
}

int vtkXRenderWindowTclInteractor::DestroyTimer(void) 
{
  // timers automatically expire in X windows
  return 1;
}

void vtkXRenderWindowTclInteractor::TerminateApp(void) 
{
  Tcl_Exit(1);
}
