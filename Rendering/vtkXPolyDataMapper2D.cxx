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
#include <stdlib.h>
#include <math.h>
#include "vtkXPolyDataMapper2D.h"
#include "vtkXImageWindow.h"
#include "vtkObjectFactory.h"


#ifndef VTK_REMOVE_LEGACY_CODE

vtkCxxRevisionMacro(vtkXPolyDataMapper2D, "$Revision$");
vtkStandardNewMacro(vtkXPolyDataMapper2D);

void vtkXPolyDataMapper2D::RenderOverlay(vtkViewport* viewport, vtkActor2D* actor)
{
  int numPts;
  vtkPolyData *input= (vtkPolyData *)this->Input;
  int npts, j;
  vtkPoints *p, *displayPts;
  vtkCellArray *aPrim;
  vtkUnsignedCharArray *c=NULL;
  unsigned char *rgba;
  int *pts;
  float *ftmp;
  int cellScalars = 0;
  int cellNum = 0;
  float tran;
  int lastX, lastY, X, Y; 
  XPoint *points = new XPoint [1024];
  int currSize = 1024;
 
  vtkDebugMacro (<< "vtkXPolyDataMapper2D::RenderOverlay");

  if ( input == NULL ) 
    {
    vtkErrorMacro(<< "No input!");
    return;
    }
  else
    {
    input->Update();
    numPts = input->GetNumberOfPoints();
    } 

  if (numPts == 0)
    {
    vtkDebugMacro(<< "No points!");
    return;
    }
  
  if ( this->LookupTable == NULL )
    {
    this->CreateDefaultLookupTable();
    }

  //
  // if something has changed regenrate colors and display lists
  // if required
  //
  tran = actor->GetProperty()->GetOpacity();

  if ( this->GetMTime() > this->BuildTime || 
       input->GetMTime() > this->BuildTime || 
       this->LookupTable->GetMTime() > this->BuildTime ||
       actor->GetProperty()->GetMTime() > this->BuildTime)
    {
    // sets this->Colors as side effect
    this->MapScalars(tran);
    this->BuildTime.Modified();
    }

  // Get the window info
  vtkWindow*  window = viewport->GetVTKWindow();
  Display* displayId = (Display*) window->GetGenericDisplayId();
  GC gc = (GC) window->GetGenericContext();
  Window windowId = (Window) window->GetGenericWindowId();

  // Get the drawable to draw into
  Drawable drawable = (Drawable) window->GetGenericDrawable();
  if (!drawable) vtkErrorMacro(<<"Window returned NULL drawable!");
  
  // Get the position of the text actor
  int* actorPos = 
    actor->GetPositionCoordinate()->GetComputedLocalDisplayValue(viewport);

  // Set up the forground color
  XWindowAttributes attr;
  XGetWindowAttributes(displayId,windowId,&attr);
  XColor aColor;
  float* actorColor = actor->GetProperty()->GetColor();
  aColor.red = (unsigned short) (actorColor[0] * 65535.0);
  aColor.green = (unsigned short) (actorColor[1] * 65535.0);
  aColor.blue = (unsigned short) (actorColor[2] * 65535.0);
  XAllocColor(displayId, attr.colormap, &aColor);
  XSetForeground(displayId, gc, aColor.pixel);
  XSetFillStyle(displayId, gc, FillSolid);
  
  // Transform the points, if necessary
  p = input->GetPoints();
  if ( this->TransformCoordinate )
    {
    int *itmp;
    numPts = p->GetNumberOfPoints();
    displayPts = vtkPoints::New();
    displayPts->SetNumberOfPoints(numPts);
    for ( j=0; j < numPts; j++ )
      {
      this->TransformCoordinate->SetValue(p->GetPoint(j));
      itmp = this->TransformCoordinate->GetComputedDisplayValue(viewport);
      displayPts->SetPoint(j, itmp[0], itmp[1], 0.0);
      }
    p = displayPts;
    }

  // Get colors
  if ( this->Colors )
    {
    c = this->Colors;
    if (!input->GetPointData()->GetScalars())
      {
      cellScalars = 1;
      }
    }

  aPrim = input->GetPolys();
  
  for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); cellNum++)
    { 
    if (c) 
      {
      // if cell scalars are present, color poly with those, otherwise
      // use the color of the first point
      if (cellScalars) 
        {
        rgba = c->GetPointer(4*cellNum);
        }
      else
        {
        rgba = c->GetPointer(4*pts[0]);
        }
      aColor.red = (unsigned short) (rgba[0] * 256);
      aColor.green = (unsigned short) (rgba[1] * 256);
      aColor.blue = (unsigned short) (rgba[2] * 256);
      XAllocColor(displayId, attr.colormap, &aColor);
      XSetForeground(displayId, gc, aColor.pixel);
      }
    if (npts > currSize)
      {
      delete [] points;
      points = new XPoint [npts];
      currSize = npts;
      }
    for (j = 0; j < npts; j++) 
      {
      ftmp = p->GetPoint(pts[j]);
      points[j].x = (short)(actorPos[0] + ftmp[0]);
      points[j].y = (short)(actorPos[1] - ftmp[1]);
      }
    XFillPolygon(displayId, drawable, gc, points, npts, 
                 Complex, CoordModeOrigin);
    }

  aPrim = input->GetLines();
  
  for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); cellNum++)
    { 
    if (c && cellScalars) 
      {
      rgba = c->GetPointer(4*cellNum);
      aColor.red = (unsigned short) (rgba[0] * 256);
      aColor.green = (unsigned short) (rgba[1] * 256);
      aColor.blue = (unsigned short) (rgba[2] * 256);
      XAllocColor(displayId, attr.colormap, &aColor);
      XSetForeground(displayId, gc, aColor.pixel);
      }
    ftmp = p->GetPoint(pts[0]);

    lastX = (int)(actorPos[0] + ftmp[0]);
    lastY = (int)(actorPos[1] - ftmp[1]);

    for (j = 1; j < npts; j++) 
      {
      ftmp = p->GetPoint(pts[j]);
      if (c && !cellScalars)
        {
        rgba = c->GetPointer(4*pts[j]);
        aColor.red = (unsigned short) (rgba[0] * 256);
        aColor.green = (unsigned short) (rgba[1] * 256);
        aColor.blue = (unsigned short) (rgba[2] * 256);
        XAllocColor(displayId, attr.colormap, &aColor);
        XSetForeground(displayId, gc, aColor.pixel);
        }
      X = (int)(actorPos[0] + ftmp[0]);
      Y = (int)(actorPos[1] - ftmp[1]);
      XDrawLine(displayId, drawable, gc, lastX, lastY, X, Y);
      lastX = X;
      lastY = Y;
      }
    }

  // Flush the X queue
  XFlush(displayId);
  XSync(displayId, False);

  delete [] points;
  if ( this->TransformCoordinate )
    {
    p->Delete();
    }
}
#endif
