/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Library. No part of this file
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/X.h>
#include <X11/keysym.h>
#include "XInter.hh"
#include "XRenWin.hh"
#include "Actor.hh"
#include <X11/Shell.h>
#include <math.h>

typedef struct
{
  Visual	*visual;
  int	depth;
} OptionsRec;
OptionsRec	Options;

XtResource resources[] =
{
	{"visual", "Visual", XtRVisual, sizeof (Visual *),
	XtOffsetOf (OptionsRec, visual), XtRImmediate, NULL},
	{"depth", "Depth", XtRInt, sizeof (int),
	XtOffsetOf (OptionsRec, depth), XtRImmediate, NULL},
};

XrmOptionDescRec Desc[] =
{
	{"-visual", "*visual", XrmoptionSepArg, NULL},
	{"-depth", "*depth", XrmoptionSepArg, NULL}
};


// states
#define VLXI_START  0
#define VLXI_ROTATE 1
#define VLXI_ZOOM   2
#define VLXI_PAN    3


// Description:
// Construct object so that light follows camera motion.
vlXInteractiveRenderer::vlXInteractiveRenderer()
{
  this->State = VLXI_START;
  this->App = 0;
}

vlXInteractiveRenderer::~vlXInteractiveRenderer()
{
}

void  vlXInteractiveRenderer::Start()
{
  XtAppMainLoop(this->App);
}

// Description:
// Initializes the event handlers
void vlXInteractiveRenderer::Initialize(XtAppContext app)
{
  this->App = app;

  this->Initialize();
}

// Description:
// Begin processing keyboard strokes.
void vlXInteractiveRenderer::Initialize()
{
  Display *display;
  static int any_initialized = 0;
  static XtAppContext app;
  vlXRenderWindow *ren;
  int depth;
  Colormap cmap;
  Visual  *vis;
  int *size;
  int *position;
  int argc = 0;

  // make sure we have a RenderWindow and camera
  if ( ! this->RenderWindow)
    {
    vlErrorMacro(<<"No renderer defined!");
    return;
    }

  /* do initialization stuff if not initialized yet */
  if (this->App)
    {
    any_initialized = 1;
    app = this->App;
    }
  if (!any_initialized)
    {
    XtToolkitInitialize();
    app = XtCreateApplicationContext();
    any_initialized = 1;
    }
  this->App = app;

  display = XtOpenDisplay(this->App,NULL,"VL","vl",NULL,0,&argc,NULL);

  // get the info we need from the RenderingWindow
  ren = (vlXRenderWindow *)(this->RenderWindow);
  ren->SetDisplayId(display);
  depth   = ren->GetDesiredDepth();
  cmap    = ren->GetDesiredColormap();
  vis     = ren->GetDesiredVisual();
  size    = ren->GetSize();
  position= ren->GetPosition();

  this->top = XtVaAppCreateShell(this->RenderWindow->Name,"vl",
				 applicationShellWidgetClass,
				 display,
				 XtNdepth, depth,
				 XtNcolormap, cmap,
				 XtNvisual, vis,
				 XtNx, position[0],
				 XtNy, position[1],
				 XtNwidth, size[0],
				 XtNheight, size[1],
				 XtNmappedWhenManaged, 0,
				 NULL);

  XtRealizeWidget(this->top);

  /* add callback */
  XSync(display,False);
  ren->SetWindowId(XtWindow(this->top));
  ren->Render();
  XtAddEventHandler(this->top,
		    KeyPressMask | ButtonPressMask | ExposureMask |
		    StructureNotifyMask | ButtonReleaseMask,
		    False,vlXInteractiveRendererCallback,(XtPointer)this);
  this->Size[0] = size[0];
  this->Size[1] = size[1];
}

void vlXInteractiveRenderer::PrintSelf(ostream& os, vlIndent indent)
{
  if (this->ShouldIPrint(vlXInteractiveRenderer::GetClassName()))
    {
    vlInteractiveRenderer::PrintSelf(os,indent);
    }
}


void  vlXInteractiveRenderer::UpdateSize(int x,int y)
{
  // if the size changed send this on to the RenderWindow
  if ((x != this->Size[0])||(y != this->Size[1]))
    {
    this->Size[0] = x;
    this->Size[1] = y;
    this->RenderWindow->SetSize(x,y);
    }

}
 
void  vlXInteractiveRenderer::StartRotate()
{
  if (this->State != VLXI_START) return;
  this->State = VLXI_ROTATE;
  XtAppAddTimeOut(this->App,10,vlXInteractiveRendererTimer,(XtPointer)this);
}
void  vlXInteractiveRenderer::EndRotate()
{
  if (this->State != VLXI_ROTATE) return;
  this->State = VLXI_START;
}

void  vlXInteractiveRenderer::StartZoom()
{
  if (this->State != VLXI_START) return;
  this->State = VLXI_ZOOM;
  XtAppAddTimeOut(this->App,10,vlXInteractiveRendererTimer,(XtPointer)this);
}
void  vlXInteractiveRenderer::EndZoom()
{
  if (this->State != VLXI_ZOOM) return;
  this->State = VLXI_START;
}

void  vlXInteractiveRenderer::StartPan()
{
  float *FocalPoint;
  float *Result;

  if (this->State != VLXI_START) return;

  this->State = VLXI_PAN;

  // calculate the focal depth since we'll be using it a lot
  FocalPoint = this->CurrentCamera->GetFocalPoint();
      
  this->CurrentRenderer->SetWorldPoint(FocalPoint[0],FocalPoint[1],
				       FocalPoint[2],1.0);
  this->CurrentRenderer->WorldToDisplay();
  Result = this->CurrentRenderer->GetDisplayPoint();
  this->FocalDepth = Result[2];

  XtAppAddTimeOut(this->App,10,vlXInteractiveRendererTimer,(XtPointer)this);
}
void  vlXInteractiveRenderer::EndPan()
{
  if (this->State != VLXI_PAN) return;
  this->State = VLXI_START;
}

void vlXInteractiveRendererCallback(Widget w,XtPointer client_data, 
				    XEvent *event, Boolean *ctd)
{
  vlXInteractiveRenderer *me;

  me = (vlXInteractiveRenderer *)client_data;

  switch (event->type) 
    {
    case Expose : me->GetRenderWindow()->Render(); break;
      
    case ConfigureNotify : 
      me->UpdateSize(((XConfigureEvent *)event)->width,
		     ((XConfigureEvent *)event)->height); 
      me->GetRenderWindow()->Render(); 
      break;

    case ButtonPress : 
      {
      switch (((XButtonEvent *)event)->button)
	{
	case Button1 : 
          me->FindPokedCamera(((XButtonEvent*)event)->x,
			      ((XButtonEvent*)event)->y);
	  me->StartRotate(); 
	  break;
	case Button2 : 
          me->FindPokedCamera(((XButtonEvent*)event)->x,
			      ((XButtonEvent*)event)->y);
	  me->StartPan(); 
	  break;
	case Button3 : 
          me->FindPokedCamera(((XButtonEvent*)event)->x,
			      ((XButtonEvent*)event)->y);
	  me->StartZoom(); 
	  break;
	}
      }
      break;

    case ButtonRelease : 
      {
      switch (((XButtonEvent *)event)->button)
	{
	case Button1 : me->EndRotate(); break;
	case Button2 : me->EndPan(); break;
	case Button3 : me->EndZoom(); break;
	}
      }
      break;

    case KeyPress :
      {
      KeySym ks;
      static char buffer[20];

      XLookupString((XKeyEvent *)event,buffer,20,&ks,NULL);
      switch (ks)
	{
	case XK_e : exit(1); break;
	case XK_w :
	  {
	  vlActorCollection *ac;
	  vlActor *anActor;
	  
          me->FindPokedRenderer(((XKeyEvent*)event)->x,
				((XKeyEvent*)event)->y);
	  ac = me->CurrentRenderer->GetActors();
	  for (ac->InitTraversal(); anActor = ac->GetNextItem(); )
	    {
	    anActor->GetProperty()->SetWireframe();
	    }
	  
	  me->RenderWindow->Render();
	  }
	  break;

	case XK_s :
	  {
	  vlActorCollection *ac;
	  vlActor *anActor;
	  
          me->FindPokedRenderer(((XKeyEvent*)event)->x,
			       ((XKeyEvent*)event)->y);
	  ac = me->CurrentRenderer->GetActors();
	  for (ac->InitTraversal(); anActor = ac->GetNextItem(); )
	    {
	    anActor->GetProperty()->SetSurface();
	    }
	  
	  me->RenderWindow->Render();
	  }
	  break;
        }
      }
      break;
    }
}

void vlXInteractiveRendererTimer(XtPointer client_data,XtIntervalId *id)
{
  vlXInteractiveRenderer *me;
  Window root,child;
  int root_x,root_y;
  int x,y;
  float xf,yf;
  unsigned int keys;

  me = (vlXInteractiveRenderer *)client_data;

  switch (me->State)
    {
    case VLXI_ROTATE :
      // get the pointer position
      XQueryPointer(XtDisplay(me->top),XtWindow(me->top),
		    &root,&child,&root_x,&root_y,&x,&y,&keys);
      xf = (x - me->Center[0]) * me->DeltaAzimuth;
      yf = (y - me->Center[1]) * me->DeltaElevation;
      me->CurrentCamera->Azimuth(xf);
      me->CurrentCamera->Elevation(yf);
      me->CurrentCamera->OrthogonalizeViewUp();
      if (me->LightFollowCamera)
	{
	/* get the first light */
	me->CurrentLight->SetPosition(me->CurrentCamera->GetPosition());
	}
      me->RenderWindow->Render();
      XtAppAddTimeOut(me->App,10,vlXInteractiveRendererTimer,client_data);
      break;
    case VLXI_PAN :
      {
      float  FPoint[3];
      float *PPoint;
      float  APoint[3];
      float  RPoint[4];

      // get the current focal point and position
      memcpy(FPoint,me->CurrentCamera->GetFocalPoint(),sizeof(float)*3);
      PPoint = me->CurrentCamera->GetPosition();

      // get the pointer position
      XQueryPointer(XtDisplay(me->top),XtWindow(me->top),
		    &root,&child,&root_x,&root_y,&x,&y,&keys);

      APoint[0] = x;
      APoint[1] = me->Size[1] - y;
      APoint[2] = me->FocalDepth;
      me->CurrentRenderer->SetDisplayPoint(APoint);
      me->CurrentRenderer->DisplayToWorld();
      memcpy(RPoint,me->CurrentRenderer->GetWorldPoint(),sizeof(float)*4);
      if (RPoint[3])
	{
	RPoint[0] = RPoint[0]/RPoint[3];
	RPoint[1] = RPoint[1]/RPoint[3];
	RPoint[2] = RPoint[2]/RPoint[3];
	}

      /*
       * Compute a translation vector, moving everything 1/10 
       * the distance to the cursor. (Arbitrary scale factor)
       */
      me->CurrentCamera->SetFocalPoint(
	(FPoint[0]-RPoint[0])/10.0 + FPoint[0],
	(FPoint[1]-RPoint[1])/10.0 + FPoint[1],
	(FPoint[2]-RPoint[2])/10.0 + FPoint[2]);
      me->CurrentCamera->SetPosition(
	(FPoint[0]-RPoint[0])/10.0 + PPoint[0],
	(FPoint[1]-RPoint[1])/10.0 + PPoint[1],
	(FPoint[2]-RPoint[2])/10.0 + PPoint[2]);
      
      me->RenderWindow->Render();
      XtAppAddTimeOut(me->App,10,vlXInteractiveRendererTimer,client_data);
      }
      break;
    case VLXI_ZOOM :
      {
      float zoomFactor;
      float *clippingRange;

      // get the pointer position
      XQueryPointer(XtDisplay(me->top),XtWindow(me->top),
		    &root,&child,&root_x,&root_y,&x,&y,&keys);
      yf = (y - me->Center[1])/(float)me->Center[1];
      zoomFactor = pow(1.1,yf);
      clippingRange = me->CurrentCamera->GetClippingRange();
      me->CurrentCamera->SetClippingRange(clippingRange[0]/zoomFactor,
					  clippingRange[1]/zoomFactor);
      me->CurrentCamera->Zoom(zoomFactor);
      me->RenderWindow->Render();
      XtAppAddTimeOut(me->App,10,vlXInteractiveRendererTimer,client_data);
      }
      break;
    }
}  


