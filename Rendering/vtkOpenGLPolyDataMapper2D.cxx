/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include <stdlib.h>
#include <math.h>
#include "vtkOpenGLPolyDataMapper2D.h"
#include <GL/gl.h>
#include "vtkObjectFactory.h"
#include "vtkgluPickMatrix.h"


//------------------------------------------------------------------------------
vtkOpenGLPolyDataMapper2D* vtkOpenGLPolyDataMapper2D::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkOpenGLPolyDataMapper2D");
  if(ret)
    {
    return (vtkOpenGLPolyDataMapper2D*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkOpenGLPolyDataMapper2D;
}




void vtkOpenGLPolyDataMapper2D::RenderOpaqueGeometry(vtkViewport* viewport,
						     vtkActor2D* actor)
{
  int            numPts;
  vtkPolyData    *input= (vtkPolyData *)this->Input;
  int            npts, j;
  vtkPoints      *p, *displayPts;
  vtkCellArray   *aPrim;
  vtkScalars     *c=NULL;
  unsigned char  *rgba;
  unsigned char  color[4];
  int            *pts;
  int            cellScalars = 0;
  int            cellNum = 0;
  
  vtkDebugMacro (<< "vtkOpenGLPolyDataMapper2D::Render");

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
  if ( this->GetMTime() > this->BuildTime || 
       input->GetMTime() > this->BuildTime || 
       this->LookupTable->GetMTime() > this->BuildTime ||
       actor->GetProperty()->GetMTime() > this->BuildTime)
    {
    // sets this->Colors as side effect
    this->GetColors();
    this->BuildTime.Modified();
    }

  // Get the position of the actor
  int *size = viewport->GetSize();
  int* actorPos = 
    actor->GetPositionCoordinate()->GetComputedViewportValue(viewport);

  // Set up the font color from the text actor
  float*  actorColor = actor->GetProperty()->GetColor();
  color[0] = (unsigned char) (actorColor[0] * 255.0);
  color[1] = (unsigned char) (actorColor[1] * 255.0);
  color[2] = (unsigned char) (actorColor[2] * 255.0);
  color[3] = (unsigned char) (255.0*actor->GetProperty()->GetOpacity());

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
      itmp = this->TransformCoordinate->GetComputedViewportValue(viewport);
      displayPts->SetPoint(j,itmp[0], itmp[1], 0.0);
      }
    p = displayPts;
    }

  // Set up the coloring
  if ( this->Colors )
    {
    c = this->Colors;
    c->InitColorTraversal(actor->GetProperty()->GetOpacity(), 
			  this->LookupTable, this->ColorMode);
    if (!input->GetPointData()->GetScalars())
      {
      cellScalars = 1;
      }
    }
  vtkDebugMacro(<< c);
  vtkDebugMacro(<< cellScalars);

  // set the colors for the foreground
  glColor4ubv(color);

  // push a 2D matrix on the stack
  glMatrixMode( GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  if(viewport->GetIsPicking())
    {
    vtkgluPickMatrix(viewport->GetPickX(), viewport->GetPickY(),
		     1, 1, viewport->GetOrigin(), viewport->GetSize());
    }
  
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
  glLoadIdentity();

  glDisable(GL_TEXTURE_2D);
  glDisable( GL_LIGHTING);
  if ( actor->GetProperty()->GetDisplayLocation() == 
       VTK_FOREGROUND_LOCATION )
    {
    glOrtho(-actorPos[0],-actorPos[0] + size[0],
            -actorPos[1], -actorPos[1] +size[1], 0, 1);
    }  
  else
    {
    glOrtho(-actorPos[0],-actorPos[0] + size[0],
            -actorPos[1], -actorPos[1] +size[1], -1, 0);
    }
    
  aPrim = input->GetPolys();
  for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); cellNum++)
    { 
    glBegin(GL_POLYGON);
    for (j = 0; j < npts; j++) 
      {
      if (c) 
	{
	if (cellScalars) 
	  {
	  rgba = c->GetColor(cellNum);
	  }
	else
	  {
	  rgba = c->GetColor(pts[j]);
	  }
	glColor4ubv(rgba);
	}
      glVertex2fv(p->GetPoint(pts[j]));
      }
    glEnd();
    }

  // Set the LineWidth
  glLineWidth(actor->GetProperty()->GetLineWidth());

  aPrim = input->GetLines();
  for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); cellNum++)
    { 
    glBegin(GL_LINE_STRIP);
    for (j = 0; j < npts; j++) 
      {
      if (c) 
	{
	if (cellScalars) 
	  {
	  rgba = c->GetColor(cellNum);
	  }
	else
	  {
	  rgba = c->GetColor(pts[j]);
	  }
	glColor4ubv(rgba);
	}
      glVertex2fv(p->GetPoint(pts[j]));
      }
    glEnd();
    }

  // Set the PointSize
  glPointSize(actor->GetProperty()->GetPointSize());

  aPrim = input->GetVerts();
  glBegin(GL_POINTS);
  for (aPrim->InitTraversal(); aPrim->GetNextCell(npts,pts); cellNum++)
    { 
    for (j = 0; j < npts; j++) 
      {
      if (c) 
	{
	if (cellScalars) 
	  {
	  rgba = c->GetColor(cellNum);
	  }
	else
	  {
	  rgba = c->GetColor(pts[j]);
	  }
	glColor4ubv(rgba);
	}
      glVertex2fv(p->GetPoint(pts[j]));
      }
    }
  glEnd();
  
  if ( this->TransformCoordinate )
    {
    p->Delete();
    }

  // push a 2D matrix on the stack
  glMatrixMode( GL_PROJECTION);
  glPopMatrix();
  glMatrixMode( GL_MODELVIEW);
  glPopMatrix();
  glEnable( GL_LIGHTING);
}


  
