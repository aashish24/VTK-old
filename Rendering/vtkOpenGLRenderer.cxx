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
#include <math.h>
#include "vtkOpenGLRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkOpenGLProperty.h"
#include "vtkOpenGLCamera.h"
#include "vtkOpenGLLight.h"
#include "vtkRayCaster.h"
#include <GL/gl.h>


#define MAX_LIGHTS 8

vtkOpenGLRenderer::vtkOpenGLRenderer()
{
}

// Description:
// Ask actors to render themselves. As a side effect will cause 
// visualization network to update.
int vtkOpenGLRenderer::UpdateActors()
{
  vtkActor *anActor;
  int count = 0;
 
  // set matrix mode for actors 
  glMatrixMode(GL_MODELVIEW);

  // loop through actors 
  for (this->Actors.InitTraversal(); (anActor = this->Actors.GetNextItem()); )
    {
    // if it's invisible, we can skip the rest 
    if (anActor->GetVisibility())
      {
      count++;
      anActor->Render((vtkRenderer *)this);
      }
    }
  return count;
}

// Description:
// Ask volumes to render themselves.
int vtkOpenGLRenderer::UpdateVolumes()
{
  int volume_count=0;    // Number of visible volumes

  volume_count = this->VisibleVolumeCount();

  // Render the volumes
  if ( volume_count > 0 )
    {

    // Render the volume
    this->RayCaster->Render((vtkRenderer *)this);

    }

  return volume_count;
}

// Description:
// Ask active camera to load its view matrix.
int vtkOpenGLRenderer::UpdateCameras ()
{
  if (!this->ActiveCamera)
    {
    vtkDebugMacro(<< "No cameras are on, creating one.");
    // the get method will automagically create a camera
    // and reset it since one hasn't been specified yet
    this->GetActiveCamera();
    }

  // update the viewing transformation
  this->ActiveCamera->Render((vtkRenderer *)this);

  return 1;
}

// Description:
// Internal method temporarily removes lights before reloading them
// into graphics pipeline.
void vtkOpenGLRenderer::ClearLights (void)
{
  short curLight;
  float Info[4];

  // define a lighting model and set up the ambient light.
  // use index 11 for the heck of it. Doesn't matter except for 0.
   
  // update the ambient light 
  Info[0] = this->Ambient[0];
  Info[1] = this->Ambient[1];
  Info[2] = this->Ambient[2];
  Info[3] = 1.0;
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, Info);

  if ( this->TwoSidedLighting )
    {
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    }
  else
    {
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
    }

  // now delete all the old lights 
  for (curLight = GL_LIGHT0; curLight < GL_LIGHT0 + MAX_LIGHTS; curLight++)
    {
    glDisable((GLenum)curLight);
    }

  this->NumberOfLightsBound = 0;
}

// Description:
// Ask lights to load themselves into graphics pipeline.
int vtkOpenGLRenderer::UpdateLights ()
{
  vtkLight *light;
  short curLight;
  float status;
  int count;

  // Check if a light is on. If not then make a new light.
  count = 0;
  curLight= this->NumberOfLightsBound + GL_LIGHT0;

  for(this->Lights.InitTraversal(); 
      (light = this->Lights.GetNextItem()); )
    {
    status = light->GetSwitch();
    if ((status > 0.0)&& (curLight < (GL_LIGHT0+MAX_LIGHTS)))
      {
      curLight++;
      count++;
      }
    }

  if( !count )
    {
    vtkDebugMacro(<<"No lights are on, creating one.");
    this->CreateLight();
    }

  count = 0;
  curLight= this->NumberOfLightsBound + GL_LIGHT0;

  // set the matrix mode for lighting. ident matrix on viewing stack  
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  for(this->Lights.InitTraversal(); 
      (light = this->Lights.GetNextItem()); )
    {

    status = light->GetSwitch();

    // if the light is on then define it and bind it. 
    // also make sure we still have room.             
    if ((status > 0.0)&& (curLight < (GL_LIGHT0+MAX_LIGHTS)))
      {
      light->Render((vtkRenderer *)this,curLight);
      glEnable((GLenum)curLight);
      // increment the current light by one 
      curLight++;
      count++;
      }
    }
  
  this->NumberOfLightsBound = curLight - GL_LIGHT0;
  
  glPopMatrix();
  glEnable(GL_LIGHTING);
  return count;
}

// Description:
// Concrete open gl render method.
void vtkOpenGLRenderer::Render(void)
{
  int    actor_count;
  int    volume_count;
  float  scale_factor;
  float  saved_viewport[4];
  float  new_viewport[4];
  int    saved_erase;

  if (this->StartRenderMethod) 
    {
    (*this->StartRenderMethod)(this->StartRenderMethodArg);
    }

  volume_count = this->VisibleVolumeCount();

  // If there is a volume renderer, get it's desired viewport size
  // since it may want to render actors into a smaller area for multires
  // rendering during motion
  if ( volume_count > 0 )
    {
    // Get the scale factor 
    scale_factor = this->RayCaster->GetViewportScaleFactor( (vtkRenderer *)this );
    
    // If the volume renderer wants a different resolution than this
    // renderer was going to produce we need to set up the viewport
    if ( scale_factor != 1.0 )
      {
      // Get the current viewport 
      this->GetViewport( saved_viewport );
      
      // Create a new viewport size based on the scale factor
      new_viewport[0] = saved_viewport[0];
      new_viewport[1] = saved_viewport[1];
      new_viewport[2] = saved_viewport[0] + 
	scale_factor * ( saved_viewport[2] - saved_viewport[0] );
      new_viewport[3] = saved_viewport[1] + 
	scale_factor * ( saved_viewport[3] - saved_viewport[1] );
      
      // Set this as the new viewport.  This will cause the OpenGL
      // viewport to be set correctly in the camera render method
      this->SetViewport( new_viewport );
      }
    }

  // standard render method 
  this->ClearLights();

  this->UpdateCameras();
  this->UpdateLights();

  actor_count =  this->UpdateActors();

  // If we are rendering with a reduced size image for the volume
  // rendering, then we need to reset the viewport so that the
  // volume renderer can access the whole window to draw the image.
  // We'll pop off what we've done so far, then we'll save the state
  // of the erase variable in the render window. We will then set the
  // erase variable in the render window to 0, and render the camera
  // again.  This will set our viewport back to the right size.
  // Finally, we restore the erase variable in the render window
  if ( volume_count > 0  && scale_factor != 1.0 )
    {
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    saved_erase = this->RenderWindow->GetErase();
    this->RenderWindow->SetErase( 0 );
    this->SetViewport( saved_viewport );
    this->ActiveCamera->Render( (vtkRenderer *)this );
    this->RenderWindow->SetErase( saved_erase );
    }

  volume_count = this->UpdateVolumes();

  if ( !(actor_count + volume_count) )
    {
    vtkWarningMacro(<< "No actors or volumes are on.");
    }

  // clean up the model view matrix set up by the camera 
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  if (this->EndRenderMethod) 
    {
    (*this->EndRenderMethod)(this->EndRenderMethodArg);
    }
}

void vtkOpenGLRenderer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkRenderer::PrintSelf(os,indent);

  os << indent << "Number Of Lights Bound: " << 
    this->NumberOfLightsBound << "\n";
}


// Description:
// Return center of renderer in display coordinates.
float *vtkOpenGLRenderer::GetCenter()
{
  int *size;
  
  // get physical window dimensions 
  size = this->RenderWindow->GetSize();

  if (this->RenderWindow->GetStereoRender())
    {
    // take into account stereo effects
    switch (this->RenderWindow->GetStereoType()) 
      {
      case VTK_STEREO_CRYSTAL_EYES:
	{
	this->Center[0] = ((this->Viewport[2]+this->Viewport[0])
				/2.0*(float)size[0]);
	this->Center[1] = ((this->Viewport[3]+this->Viewport[1])
				/2.0*(float)size[1]);
	this->Center[1] = this->Center[1]*(491.0/1024.0);
	}
	break;
      default:
	{
	this->Center[0] = ((this->Viewport[2]+this->Viewport[0])
			   /2.0*(float)size[0]);
	this->Center[1] = ((this->Viewport[3]+this->Viewport[1])
			   /2.0*(float)size[1]);
	}
      }
    }
  else
    {
    this->Center[0] = ((this->Viewport[2]+this->Viewport[0])
			    /2.0*(float)size[0]);
    this->Center[1] = ((this->Viewport[3]+this->Viewport[1])
			    /2.0*(float)size[1]);
    }

  return this->Center;
}


// Description:
// Convert display coordinates to view coordinates.
void vtkOpenGLRenderer::DisplayToView()
{
  float vx,vy,vz;
  int sizex,sizey;
  int *size;
  
  /* get physical window dimensions */
  size = this->RenderWindow->GetSize();
  sizex = size[0];
  sizey = size[1];

  if (this->RenderWindow->GetStereoRender())
    {
    // take into account stereo effects
    switch (this->RenderWindow->GetStereoType()) 
      {
      case VTK_STEREO_CRYSTAL_EYES:
	{
	vx = 2.0 * (this->DisplayPoint[0] - sizex*this->Viewport[0])/ 
	  (sizex*(this->Viewport[2]-this->Viewport[0])) - 1.0;

	vy = 2.0 * (this->DisplayPoint[1]*(1024.0/491.0) - 
		    sizey*this->Viewport[1])/ 
	  (sizey*(this->Viewport[3]-this->Viewport[1])) - 1.0;
	}
	break;
      default:
	{
	vx = 2.0 * (this->DisplayPoint[0] - sizex*this->Viewport[0])/ 
	  (sizex*(this->Viewport[2]-this->Viewport[0])) - 1.0;
	vy = 2.0 * (this->DisplayPoint[1] - sizey*this->Viewport[1])/ 
	  (sizey*(this->Viewport[3]-this->Viewport[1])) - 1.0;
	}
      }
    }
  else
    {
    vx = 2.0 * (this->DisplayPoint[0] - sizex*this->Viewport[0])/ 
      (sizex*(this->Viewport[2]-this->Viewport[0])) - 1.0;
    vy = 2.0 * (this->DisplayPoint[1] - sizey*this->Viewport[1])/ 
      (sizey*(this->Viewport[3]-this->Viewport[1])) - 1.0;
    }

  vz = this->DisplayPoint[2];

  this->SetViewPoint(vx*this->Aspect[0],vy*this->Aspect[1],vz);
}

// Description:
// Convert view coordinates to display coordinates.
void vtkOpenGLRenderer::ViewToDisplay()
{
  float dx,dy;
  int sizex,sizey;
  int *size;
  
  /* get physical window dimensions */
  size = this->RenderWindow->GetSize();
  sizex = size[0];
  sizey = size[1];

  if (this->RenderWindow->GetStereoRender())
    {
    // take into account stereo effects
    switch (this->RenderWindow->GetStereoType()) 
      {
      case VTK_STEREO_CRYSTAL_EYES:
	{
	dx = (this->ViewPoint[0]/this->Aspect[0] + 1.0) * 
	  (sizex*(this->Viewport[2]-this->Viewport[0])) / 2.0 +
	  sizex*this->Viewport[0];
	dy = (this->ViewPoint[1]/this->Aspect[1] + 1.0) * 
	  (sizey*(this->Viewport[3]-this->Viewport[1])) / 2.0 +
	  sizey*this->Viewport[1];
	dy = dy*(491.0/1024.0);
	}
	break;
      default:
	{
	dx = (this->ViewPoint[0]/this->Aspect[0] + 1.0) * 
	  (sizex*(this->Viewport[2]-this->Viewport[0])) / 2.0 +
	  sizex*this->Viewport[0];
	dy = (this->ViewPoint[1]/this->Aspect[1] + 1.0) * 
	  (sizey*(this->Viewport[3]-this->Viewport[1])) / 2.0 +
	  sizey*this->Viewport[1];
	}
      }
    }
  else
    {
    dx = (this->ViewPoint[0]/this->Aspect[0] + 1.0) * 
      (sizex*(this->Viewport[2]-this->Viewport[0])) / 2.0 +
      sizex*this->Viewport[0];
    dy = (this->ViewPoint[1]/this->Aspect[1] + 1.0) * 
      (sizey*(this->Viewport[3]-this->Viewport[1])) / 2.0 +
      sizey*this->Viewport[1];
    }

  this->SetDisplayPoint(dx,dy,this->ViewPoint[2]);
}


// Description:
// Is a given display point in this renderer's viewport.
int vtkOpenGLRenderer::IsInViewport(int x,int y)
{
  int *size;
  
  // get physical window dimensions 
  size = this->RenderWindow->GetSize();


  if (this->RenderWindow->GetStereoRender())
    {
    // take into account stereo effects
    switch (this->RenderWindow->GetStereoType()) 
      {
      case VTK_STEREO_CRYSTAL_EYES:
	{
	int ty = (int)(y*(1023.0/491.0));

	if ((this->Viewport[0]*size[0] <= x)&&
	    (this->Viewport[2]*size[0] >= x)&&
	    (this->Viewport[1]*size[1] <= ty)&&
	    (this->Viewport[3]*size[1] >= ty))
	  {
	  return 1;
	  }
	}
	break;
      default:
	{
	if ((this->Viewport[0]*size[0] <= x)&&
	    (this->Viewport[2]*size[0] >= x)&&
	    (this->Viewport[1]*size[1] <= y)&&
	    (this->Viewport[3]*size[1] >= y))
	  {
	  return 1;
	  }
	}
      }
    }
  else
    {
    if ((this->Viewport[0]*size[0] <= x)&&
	(this->Viewport[2]*size[0] >= x)&&
	(this->Viewport[1]*size[1] <= y)&&
	(this->Viewport[3]*size[1] >= y))
      {
      return 1;
      }
    }
  
  return 0;
}
