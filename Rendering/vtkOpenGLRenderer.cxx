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
#include "vtkCuller.h"
#include <GL/gl.h>


#define VTK_MAX_LIGHTS 8

vtkOpenGLRenderer::vtkOpenGLRenderer()
{
  this->NumberOfLightsBound = 0;
}

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
  for (curLight = GL_LIGHT0; curLight < GL_LIGHT0 + VTK_MAX_LIGHTS; curLight++)
    {
    glDisable((GLenum)curLight);
    }

  this->NumberOfLightsBound = 0;
}

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

  for(this->Lights->InitTraversal(); 
      (light = this->Lights->GetNextItem()); )
    {
    status = light->GetSwitch();
    if ((status > 0.0)&& (curLight < (GL_LIGHT0+VTK_MAX_LIGHTS)))
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

  for(this->Lights->InitTraversal(); 
      (light = this->Lights->GetNextItem()); )
    {

    status = light->GetSwitch();

    // if the light is on then define it and bind it. 
    // also make sure we still have room.             
    if ((status > 0.0)&& (curLight < (GL_LIGHT0+VTK_MAX_LIGHTS)))
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

// Concrete open gl render method.
void vtkOpenGLRenderer::DeviceRender(void)
{
  int    actor_count;
  int    volume_count;
  float  scale_factor;
  float  saved_viewport[4];
  float  new_viewport[4];
  int    saved_erase;

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

  // set matrix mode for actors 
  glMatrixMode(GL_MODELVIEW);
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
    vtkDebugMacro(<< "No actors or volumes are on.");
    }

  // clean up the model view matrix set up by the camera 
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

void vtkOpenGLRenderer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkRenderer::PrintSelf(os,indent);

  os << indent << "Number Of Lights Bound: " << 
    this->NumberOfLightsBound << "\n";
}

void vtkOpenGLRenderer::Clear(void)
{
  glClearColor( ((GLclampf)(this->Background[0])),
                ((GLclampf)(this->Background[1])),
                ((GLclampf)(this->Background[2])),
                ((GLclampf)(1.0)) );

  glClearDepth( (GLclampd)( 1.0 ) );
  vtkDebugMacro(<< "glClear\n");
  glClear((GLbitfield)(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

