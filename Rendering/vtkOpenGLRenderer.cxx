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
#include <math.h>
#include "vtkOpenGLRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkOpenGLProperty.h"
#include "vtkOpenGLCamera.h"
#include "vtkOpenGLLight.h"
#include "vtkRayCaster.h"
#include "vtkCuller.h"

// if this .cxx file is being used to implement
// the mesa renderer, then do not include GL/gl.h
// because with the mesa renderer we need to include
// mesagl.h instead
#ifndef VTK_IMPLEMENT_MESA_CXX
#include <GL/gl.h>
#endif
#include "vtkObjectFactory.h"

class vtkGLPickInfo
{
public:
  GLuint* PickBuffer;
  GLuint PickedID;
};


//------------------------------------------------------------------------------
vtkOpenGLRenderer* vtkOpenGLRenderer::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkOpenGLRenderer");
  if(ret)
    {
    return (vtkOpenGLRenderer*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkOpenGLRenderer;
}





#define VTK_MAX_LIGHTS 8

vtkOpenGLRenderer::vtkOpenGLRenderer()
{
  this->PickInfo = new vtkGLPickInfo;
  this->NumberOfLightsBound = 0;
  this->PickInfo->PickBuffer = 0;
  this->PickInfo->PickedID = 0;
  this->PickedZ = 0;
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
  float  scaleFactor;
  float  saved_viewport[4];
  float  new_viewport[4];
  int    saved_erase;
    
  // Do not remove this MakeCurrent! Due to Start / End methods on
  // some objects which get executed during a pipeline update, 
  // other windows might get rendered since the last time
  // a MakeCurrent was called.
  this->RenderWindow->MakeCurrent();

  scaleFactor = 1.0;

  // If there is a volume renderer, get its desired viewport size
  // since it may want to render actors into a smaller area for multires
  // rendering during motion
  if ( ( this->NumberOfPropsToRayCast + 
	 this->NumberOfPropsToRenderIntoImage ) > 0 )
    {
    // Get the scale factor 
    scaleFactor = this->RayCaster->GetViewportScaleFactor( (vtkRenderer *)this );
    
    // If the volume renderer wants a different resolution than this
    // renderer was going to produce we need to set up the viewport
    if ( scaleFactor != 1.0 )
      {
      // Get the current viewport 
      this->GetViewport( saved_viewport );
      
      // Create a new viewport size based on the scale factor
      new_viewport[0] = saved_viewport[0];
      new_viewport[1] = saved_viewport[1];
      new_viewport[2] = saved_viewport[0] + 
	scaleFactor * ( saved_viewport[2] - saved_viewport[0] );
      new_viewport[3] = saved_viewport[1] + 
	scaleFactor * ( saved_viewport[3] - saved_viewport[1] );
      
      // Set this as the new viewport.  This will cause the OpenGL
      // viewport to be set correctly in the camera render method
      this->SetViewport( new_viewport );
      }
    }

  // standard render method 
  this->ClearLights();

  this->UpdateCamera();
  this->UpdateLights();

  // set matrix mode for actors 
  glMatrixMode(GL_MODELVIEW);

  this->UpdateGeometry();

  // If we are rendering with a reduced size image for the volume
  // rendering, then we need to reset the viewport so that the
  // volume renderer can access the whole window to draw the image.
  // We'll pop off what we've done so far, then we'll save the state
  // of the erase variable in the render window. We will then set the
  // erase variable in the render window to 0, and render the camera
  // again.  This will set our viewport back to the right size.
  // Finally, we restore the erase variable in the render window
  if ( scaleFactor != 1.0 )
    {
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    saved_erase = this->RenderWindow->GetErase();
    this->RenderWindow->SetErase( 0 );
    this->SetViewport( saved_viewport );
    this->ActiveCamera->Render( (vtkRenderer *)this );
    this->RenderWindow->SetErase( saved_erase );
    }

  // Let the ray caster do its thing - this will both update
  // props that are rendered using ray casting, and props that
  // are rendered into an image. The final image is merges with the
  // results of the geometry rendering, and then placed up on the
  // screen.
  if ( ( this->NumberOfPropsToRayCast + 
	 this->NumberOfPropsToRenderIntoImage ) > 0 )
    {
    this->RayCaster->Render( (vtkRenderer *)this,
			     this->NumberOfPropsToRayCast,
			     this->RayCastPropArray,
			     this->NumberOfPropsToRenderIntoImage,
			     this->RenderIntoImagePropArray );
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
  os << indent << "PickBuffer " << this->PickInfo->PickBuffer << "\n";
  os << indent << "PickedID " << this->PickInfo->PickedID << "\n";
  os << indent << "PickedZ " << this->PickedZ << "\n";
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

void vtkOpenGLRenderer::StartPick(unsigned int pickFromSize)
{
  int bufferSize = pickFromSize * 4;
  this->PickInfo->PickBuffer = new GLuint[bufferSize];
  glSelectBuffer(bufferSize, this->PickInfo->PickBuffer);
  // change to selection mode
  (void)glRenderMode(GL_SELECT);
  // initialize the pick names and add a 0 name, for no pick
  glInitNames();
  glPushName(0);
}



void vtkOpenGLRenderer::SetPickId(unsigned int pickID)
{
  glLoadName(pickID);
}


void vtkOpenGLRenderer::DevicePickRender()
{ 
  float  scaleFactor;
  float  saved_viewport[4];
  float  new_viewport[4];
  int    saved_erase;
    
  // Do not remove this MakeCurrent! Due to Start / End methods on
  // some objects which get executed during a pipeline update, 
  // other windows might get rendered since the last time
  // a MakeCurrent was called.
  this->RenderWindow->MakeCurrent();

  scaleFactor = 1.0;

  // If there is a volume renderer, get its desired viewport size
  // since it may want to render actors into a smaller area for multires
  // rendering during motion
  if ( ( this->NumberOfPropsToRayCast + 
	 this->NumberOfPropsToRenderIntoImage ) > 0 )
    {
    // Get the scale factor 
    scaleFactor = this->RayCaster->GetViewportScaleFactor( (vtkRenderer *)this );
    
    // If the volume renderer wants a different resolution than this
    // renderer was going to produce we need to set up the viewport
    if ( scaleFactor != 1.0 )
      {
      // Get the current viewport 
      this->GetViewport( saved_viewport );
      
      // Create a new viewport size based on the scale factor
      new_viewport[0] = saved_viewport[0];
      new_viewport[1] = saved_viewport[1];
      new_viewport[2] = saved_viewport[0] + 
	scaleFactor * ( saved_viewport[2] - saved_viewport[0] );
      new_viewport[3] = saved_viewport[1] + 
	scaleFactor * ( saved_viewport[3] - saved_viewport[1] );
      
      // Set this as the new viewport.  This will cause the OpenGL
      // viewport to be set correctly in the camera render method
      this->SetViewport( new_viewport );
      }
    }

  // standard render method 
  this->ClearLights();

  this->UpdateCamera();
  this->UpdateLights();

  // set matrix mode for actors 
  glMatrixMode(GL_MODELVIEW);

  this->PickGeometry();

  // If we are rendering with a reduced size image for the volume
  // rendering, then we need to reset the viewport so that the
  // volume renderer can access the whole window to draw the image.
  // We'll pop off what we've done so far, then we'll save the state
  // of the erase variable in the render window. We will then set the
  // erase variable in the render window to 0, and render the camera
  // again.  This will set our viewport back to the right size.
  // Finally, we restore the erase variable in the render window
  if ( scaleFactor != 1.0 )
    {
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    saved_erase = this->RenderWindow->GetErase();
    this->RenderWindow->SetErase( 0 );
    this->SetViewport( saved_viewport );
    this->ActiveCamera->Render( (vtkRenderer *)this );
    this->RenderWindow->SetErase( saved_erase );
    }

  // Let the ray caster do its thing - this will both update
  // props that are rendered using ray casting, and props that
  // are rendered into an image. The final image is merges with the
  // results of the geometry rendering, and then placed up on the
  // screen.
  if ( ( this->NumberOfPropsToRayCast + 
	 this->NumberOfPropsToRenderIntoImage ) > 0 )
    {
   //   this->RayCaster->Render( (vtkRenderer *)this,
//  			     this->NumberOfPropsToRayCast,
//  			     this->RayCastPropArray,
//  			     this->NumberOfPropsToRenderIntoImage,
//  			     this->RenderIntoImagePropArray );
    }

  // clean up the model view matrix set up by the camera 
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}


void vtkOpenGLRenderer::DonePick()
{
  glFlush();
  GLuint hits = glRenderMode(GL_RENDER); 
  unsigned int depth = (unsigned int)-1;
  GLuint* ptr = this->PickInfo->PickBuffer;
  this->PickInfo->PickedID = 0;
  for(unsigned int k =0; k < hits; k++)
    {
    int num_names = *ptr;
    int save = 0;
    ptr++; // move to first depth value
    if(*ptr <= depth)      {
      depth = *ptr;
      save = 1;
      }
    ptr++; // move to next depth value
    if(*ptr <= depth)      {
      depth = *ptr;
      save = 1;
      }
    // move to first name picked
    ptr++;
    if(save)
      {
      this->PickInfo->PickedID = *ptr;
      }
    // skip additonal names
    ptr += num_names;
    }
  // If there was a pick, then get the Z value
  if(this->PickInfo->PickedID)
    {
    // convert from pick depth described as:
    // Returned depth values are mapped such that the largest unsigned 
    // integer value corresponds to window coordinate depth 1.0, 
    // and zero corresponds to window coordinate depth 0.0.
    
    this->PickedZ = ((double)depth / (double)VTK_UNSIGNED_INT_MAX);

    // Clamp to range [0,1]
    this->PickedZ = (this->PickedZ < 0.0) ? 0.0 : this->PickedZ;
    this->PickedZ = (this->PickedZ > 1.0) ? 1.0: this->PickedZ;
    }
  delete [] this->PickInfo->PickBuffer;
}



float vtkOpenGLRenderer::GetPickedZ()
{
  return this->PickedZ;
}

unsigned int vtkOpenGLRenderer::GetPickedID()
{
  return (unsigned int)this->PickInfo->PickedID;
}

vtkOpenGLRenderer::~vtkOpenGLRenderer()
{
  delete [] this->PickInfo->PickBuffer;
  delete this->PickInfo;
}

