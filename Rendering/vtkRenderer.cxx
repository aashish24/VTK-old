/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1996 Ken Martin, Will Schroeder, Bill Lorensen.

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
#include <stdlib.h>
#include <string.h>

#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkVolumeRenderer.h"
#include "vtkNewVolumeRenderer.h"
#include "vtkMath.h"

// Description:
// Create a vtkRenderer with a black background, a white ambient light, 
// two-sided lighting turned on, a viewport of (0,0,1,1), and backface culling
// turned off.
vtkRenderer::vtkRenderer()
{
  this->ActiveCamera = NULL;

  this->ViewRays = NULL;
  this->ViewRaysSize[0] = 0;
  this->ViewRaysSize[1] = 0;
  this->ViewRaysCamMtime = 0;

  this->Ambient[0] = 1;
  this->Ambient[1] = 1;
  this->Ambient[2] = 1;

  this->Background[0] = 0;
  this->Background[1] = 0;
  this->Background[2] = 0;

  this->WorldPoint[0] = 0;
  this->WorldPoint[1] = 0;
  this->WorldPoint[2] = 0;
  this->WorldPoint[3] = 0;

  this->DisplayPoint[0] = 0;
  this->DisplayPoint[1] = 0;
  this->DisplayPoint[2] = 0;

  this->ViewPoint[0] = 0;
  this->ViewPoint[1] = 0;
  this->ViewPoint[2] = 0;

  this->Viewport[0] = 0;
  this->Viewport[1] = 0;
  this->Viewport[2] = 1;
  this->Viewport[3] = 1;

  this->AllocatedRenderTime = 0;
  
  this->Aspect[0] = this->Aspect[1] = 1.0;
  this->VolumeRenderer = NULL;
  this->NewVolumeRenderer = NULL;
  this->SelfCreatedCamera = 0;
  this->SelfCreatedLight = 0;
  this->CreatedLight = NULL;
  
  this->StartRenderMethod = NULL;
  this->StartRenderMethodArgDelete = NULL;
  this->StartRenderMethodArg = NULL;
  this->EndRenderMethod = NULL;
  this->EndRenderMethodArgDelete = NULL;
  this->EndRenderMethodArg = NULL;

  this->TwoSidedLighting = 1;
}

vtkRenderer::~vtkRenderer()
{
  if ( this->SelfCreatedCamera && this->ActiveCamera != NULL) 
    this->ActiveCamera->Delete();
  if ( this->SelfCreatedLight && this->CreatedLight != NULL) 
    this->CreatedLight->Delete();
}

// Description:
// Specify the camera to use for this renderer.
void vtkRenderer::SetActiveCamera(vtkCamera *cam)
{
  if ( this->ActiveCamera != cam ) 
    {
    if ( this->SelfCreatedCamera ) this->ActiveCamera->Delete();
    this->SelfCreatedCamera = 0;
    this->ActiveCamera = cam;
    this->Modified();
    }
}

// Description:
// Get the current camera.
vtkCamera *vtkRenderer::GetActiveCamera()
{
  if ( this->ActiveCamera == NULL )
    {
    this->ActiveCamera = new vtkCamera;
    this->SelfCreatedCamera = 1;
    this->ResetCamera();
    }

  return this->ActiveCamera;
}

// Description:
// Specify a volume renderer to use. If this is set, then volume rendering
// will be done. 9It isn't a very good volume renderer, but it works.)
void vtkRenderer::SetVolumeRenderer(vtkVolumeRenderer *vol)
{
  this->VolumeRenderer = vol;
}

// Description:
// Get the volume renderer.
vtkVolumeRenderer *vtkRenderer::GetVolumeRenderer()
{
  return this->VolumeRenderer;
}

// Description:
// Set the volume renderer to use
void vtkRenderer::SetNewVolumeRenderer(vtkNewVolumeRenderer *vol)
{
  this->NewVolumeRenderer = vol;
}

// Description:
// Get the volume renderer.
vtkNewVolumeRenderer *vtkRenderer::GetNewVolumeRenderer()
{
  return this->NewVolumeRenderer;
}

// Description:
// Add a light to the list of lights.
void vtkRenderer::AddLights(vtkLight *light)
{
  this->Lights.AddItem(light);
}

// Description:
// Add an actor to the list of actors.
void vtkRenderer::AddActors(vtkActor *actor)
{
  this->Actors.AddItem(actor);
}

// Description:
// Remove a light from the list of lights.
void vtkRenderer::RemoveLights(vtkLight *light)
{
  this->Lights.RemoveItem(light);
}

// Description:
// Remove an actor from the list of actors.
void vtkRenderer::RemoveActors(vtkActor *actor)
{
  this->Actors.RemoveItem(actor);
}

void vtkRenderer::CreateLight(void)
{
    this->CreatedLight = new vtkLight;
    this->SelfCreatedLight = 1;
    this->AddLights(this->CreatedLight);
    this->CreatedLight->SetPosition(this->ActiveCamera->GetPosition());
    this->CreatedLight->SetFocalPoint(this->ActiveCamera->GetFocalPoint());
}

// Description:
// Automatically set up the camera based on the visible actors.
// The camera will reposition itself to view the center point of the actors,
// and move along its initial view plane normal (i.e., vector defined from 
// camera position to focal point) so that all of the actors can be seen.
void vtkRenderer::ResetCamera()
{
  vtkVolume *aVolume;
  vtkActor *anActor;
  float *bounds;
  float allBounds[6];
  int nothingVisible=1;

  allBounds[0] = allBounds[2] = allBounds[4] = VTK_LARGE_FLOAT;
  allBounds[1] = allBounds[3] = allBounds[5] = -VTK_LARGE_FLOAT;
  
  // loop through actors (and their parts)
  for ( this->Actors.InitTraversal(); (anActor = this->Actors.GetNextItem()); )
    {
    // if it's invisible, or has no geometry, we can skip the rest 
    if ( anActor->GetVisibility() )
      {
      nothingVisible = 0;
      bounds = anActor->GetBounds();

      if (bounds[0] < allBounds[0]) allBounds[0] = bounds[0]; 
      if (bounds[1] > allBounds[1]) allBounds[1] = bounds[1]; 
      if (bounds[2] < allBounds[2]) allBounds[2] = bounds[2]; 
      if (bounds[3] > allBounds[3]) allBounds[3] = bounds[3]; 
      if (bounds[4] < allBounds[4]) allBounds[4] = bounds[4]; 
      if (bounds[5] > allBounds[5]) allBounds[5] = bounds[5]; 
      }
    }

  // loop through volumes if any
  if (this->VolumeRenderer)
    {
    for (this->VolumeRenderer->GetVolumes()->InitTraversal(); 
	 (aVolume = this->VolumeRenderer->GetVolumes()->GetNextItem()); )
      {
      // if it's invisible, we can skip the rest 
      if ( aVolume->GetVisibility() )
	{
	nothingVisible = 0;
	bounds = aVolume->GetBounds();
	
	if (bounds[0] < allBounds[0]) allBounds[0] = bounds[0]; 
	if (bounds[1] > allBounds[1]) allBounds[1] = bounds[1]; 
	if (bounds[2] < allBounds[2]) allBounds[2] = bounds[2]; 
	if (bounds[3] > allBounds[3]) allBounds[3] = bounds[3]; 
	if (bounds[4] < allBounds[4]) allBounds[4] = bounds[4]; 
	if (bounds[5] > allBounds[5]) allBounds[5] = bounds[5]; 
	}
      }
    }

  if ( this->NewVolumeRenderer )
    {
    // Add bounds code!!!

    }

  if ( nothingVisible )
    {
    vtkErrorMacro(<< "Can't reset camera if no actors are visible");
    return;
    }

  this->ResetCamera(allBounds);
}

// Description:
// Automatically set up the camera based on a specified bounding box
// (xmin,xmax, ymin,ymax, zmin,zmax). Camera will reposition itself so
// that its focal point is the center of the bounding box, and adjust its
// distance and position to preserve its initial view plane normal 
// (i.e., vector defined from camera position to focal point). Note: is 
// the view plane is parallel to the view up axis, the view up axis will
// be reset to one of the three coordinate axes.
void vtkRenderer::ResetCamera(float bounds[6])
{
  float center[3];
  float distance;
  float width;
  float vn[3], *vup;;

  if ( this->ActiveCamera != NULL )
    {
    this->ActiveCamera->GetViewPlaneNormal(vn);
    }
  else
    {
    vtkErrorMacro(<< "Trying to reset non-existant camera");
    return;
    }

  center[0] = (bounds[0] + bounds[1])/2.0;
  center[1] = (bounds[2] + bounds[3])/2.0;
  center[2] = (bounds[4] + bounds[5])/2.0;

  width = bounds[3] - bounds[2];
  if (width < (bounds[1] - bounds[0]))
    {
    width = bounds[1] - bounds[0];
    }
  distance = 
    0.8*width/tan(this->ActiveCamera->GetViewAngle()*vtkMath::Pi()/360.0);
  distance = distance + (bounds[5] - bounds[4])/2.0;

  // check view-up vector against view plane normal
  vup = this->ActiveCamera->GetViewUp();
  if ( fabs(vtkMath::Dot(vup,vn)) > 0.999 )
    {
    vtkWarningMacro(<<"Resetting view-up since view plane normal is parallel");
    this->ActiveCamera->SetViewUp(-vup[2], vup[0], vup[1]);
    }

  // update the camera
  this->ActiveCamera->SetFocalPoint(center);
  this->ActiveCamera->SetPosition(center[0]+distance*vn[0],
				  center[1]+distance*vn[1],
				  center[2]+distance*vn[2]);
  this->ActiveCamera->SetClippingRange(distance/10.0,distance*5.0);
}
  
// Description:
// Specify the rendering window in which to draw. This is automatically set
// when the renderer is created by MakeRenderer.  The user probably
// shouldn't ever need to call this method.
void vtkRenderer::SetRenderWindow(vtkRenderWindow *renwin)
{
  this->RenderWindow = renwin;
}

// Description:
// Convert display coordinates to view coordinates.
void vtkRenderer::DisplayToView()
{
  float vx,vy,vz;
  int sizex,sizey;
  int *size;
  
  /* get physical window dimensions */
  size = this->RenderWindow->GetSize();
  sizex = size[0];
  sizey = size[1];

  vx = 2.0 * (this->DisplayPoint[0] - sizex*this->Viewport[0])/ 
    (sizex*(this->Viewport[2]-this->Viewport[0])) - 1.0;
  vy = 2.0 * (this->DisplayPoint[1] - sizey*this->Viewport[1])/ 
    (sizey*(this->Viewport[3]-this->Viewport[1])) - 1.0;
  vz = this->DisplayPoint[2];

  this->SetViewPoint(vx*this->Aspect[0],vy*this->Aspect[1],vz);
}

// Description:
// Convert view coordinates to display coordinates.
void vtkRenderer::ViewToDisplay()
{
  float dx,dy;
  int sizex,sizey;
  int *size;
  
  /* get physical window dimensions */
  size = this->RenderWindow->GetSize();
  sizex = size[0];
  sizey = size[1];

  dx = (this->ViewPoint[0]/this->Aspect[0] + 1.0) * 
    (sizex*(this->Viewport[2]-this->Viewport[0])) / 2.0 +
      sizex*this->Viewport[0];
  dy = (this->ViewPoint[1]/this->Aspect[1] + 1.0) * 
    (sizey*(this->Viewport[3]-this->Viewport[1])) / 2.0 +
      sizey*this->Viewport[1];

  this->SetDisplayPoint(dx,dy,this->ViewPoint[2]);
}

// Description:
// Convert view point coordinates to world coordinates.
void vtkRenderer::ViewToWorld()
{
  vtkMatrix4x4 mat;
  float result[4];

  // get the perspective transformation from the active camera 
  mat = this->ActiveCamera->GetCompositePerspectiveTransform(1,0,1);
  
  // use the inverse matrix 
  mat.Invert();
 
  // Transform point to world coordinates 
  result[0] = this->ViewPoint[0];
  result[1] = this->ViewPoint[1];
  result[2] = this->ViewPoint[2];
  result[3] = 1.0;

  mat.Transpose();
  mat.PointMultiply(result,result);
  
  // Get the transformed vector & set WorldPoint 
  // while we are at it try to keep w at one
  if (result[3])
    {
    result[0] /= result[3];
    result[1] /= result[3];
    result[2] /= result[3];
    result[3] = 1;
    }
  
  this->SetWorldPoint(result);
}

// Description:
// Convert world point coordinates to view coordinates.
void vtkRenderer::WorldToView()
{
  vtkMatrix4x4 matrix;
  float     view[4];
  float     *world;

  // get the perspective transformation from the active camera 
  matrix = this->ActiveCamera->GetCompositePerspectiveTransform(1,0,1);

  world = this->WorldPoint;
  view[0] = world[0]*matrix[0][0] + world[1]*matrix[0][1] +
    world[2]*matrix[0][2] + world[3]*matrix[0][3];
  view[1] = world[0]*matrix[1][0] + world[1]*matrix[1][1] +
    world[2]*matrix[1][2] + world[3]*matrix[1][3];
  view[2] = world[0]*matrix[2][0] + world[1]*matrix[2][1] +
    world[2]*matrix[2][2] + world[3]*matrix[2][3];
  view[3] = world[0]*matrix[3][0] + world[1]*matrix[3][1] +
    world[2]*matrix[3][2] + world[3]*matrix[3][3];

  if (view[3] != 0.0)
    {
    this->SetViewPoint(view[0]/view[3],
		       view[1]/view[3],
		       view[2]/view[3]);
    }
}

// Description:
// Return the center of this renderer in display coordinates.
float *vtkRenderer::GetCenter()
{
  int *size;
  
  // get physical window dimensions 
  size = this->RenderWindow->GetSize();

  this->Center[0] = ((this->Viewport[2]+this->Viewport[0])
		     /2.0*(float)size[0]);
  this->Center[1] = ((this->Viewport[3]+this->Viewport[1])
		     /2.0*(float)size[1]);

  return this->Center;
}

// Description:
// Is a given display point in this renderer's viewport.
int vtkRenderer::IsInViewport(int x,int y)
{
  int *size;
  
  // get physical window dimensions 
  size = this->RenderWindow->GetSize();

  if ((this->Viewport[0]*size[0] <= x)&&
      (this->Viewport[2]*size[0] >= x)&&
      (this->Viewport[1]*size[1] <= y)&&
      (this->Viewport[3]*size[1] >= y))
    {
    return 1;
    }

  return 0;
}

void vtkRenderer::UpdateViewRays()
{
  float xpos, ypos, zpos;
  float xinc, yinc;
  float *vr_ptr;

  float mag;
  float nx, ny, nz;

  float *aspect;

  int   	x, y;
  vtkMatrix4x4	mat;
  float		result[4];

  if( !this->ViewRays )
    {
    vtkErrorMacro(<< "No memory allocated to build viewing rays.");
    return;
    }

  // Loop through each pixel and compute viewing ray

  // get the perspective transformation from the active camera
  aspect = this->GetAspect();

  mat = this->ActiveCamera->GetPerspectiveTransform(aspect[0]/aspect[1],0,1);

  mat.Invert();
  mat.Transpose();

  vr_ptr = this->ViewRays;

  xinc = 2.0/(float)this->ViewRaysSize[0];
  yinc = 2.0/(float)this->ViewRaysSize[1];

  ypos = -1.0;
  zpos =  1.0;
 
  nx = ny = nz = 0.0;

  for( y=0; y<this->ViewRaysSize[1]; y++ )
  {
    xpos = -1.0;
    for( x=0; x<this->ViewRaysSize[0]; x++ )
    {
      result[0] = xpos;
      result[1] = ypos;
      result[2] = 1.0;
      result[3] = 1.0;

      mat.PointMultiply(result,result);

      // Normalize view ray
      mag = sqrt( (double)(result[0]*result[0] + 
			   result[1]*result[1] + result[2]*result[2]) );

      if( mag != 0.0 )
      {
        nx = result[0]/mag;
        ny = result[1]/mag;
        nz = result[2]/mag;
      }
      else
        nx = ny = nz = 0.0;

      *(vr_ptr++) = nx;
      *(vr_ptr++) = ny;
      *(vr_ptr++) = nz;

      xpos += xinc;
    }
    ypos += yinc;
  }
}

float *vtkRenderer::GetViewRays()
{
  int    size[2];
  int    *rwin_size;
  float  *vp_size;

  unsigned long  cam_mtime;

  int    update_rays = 0;

  // get physical window dimensions
  rwin_size = this->RenderWindow->GetSize();
  vp_size = this->GetViewport();

  size[0] = (int)(rwin_size[0]*(float)(vp_size[2] - vp_size[0]));
  size[1] = (int)(rwin_size[1]*(float)(vp_size[3] - vp_size[1]));

  // Allocate viewing rays memory if pixel size has changed
  if( (!this->ViewRays) ||
      (size[0] != this->ViewRaysSize[0]) ||
      (size[1] != this->ViewRaysSize[1]) )
  {
    if( this->ViewRays )
      free( this->ViewRays );

    this->ViewRaysSize[0] = size[0];
    this->ViewRaysSize[1] = size[1];

    this->ViewRays = new float[(size[0]*size[1]*3)];

    update_rays = 1;
  }

  // Check to see if camera mtime has changed
  cam_mtime = this->ActiveCamera->GetViewingRaysMTime();

  if( cam_mtime != this->ViewRaysCamMtime )
    {
    this->ViewRaysCamMtime = cam_mtime;
    update_rays = 1;
    }

  if( update_rays )
    this->UpdateViewRays();

  return( this->ViewRays );
}


// Description:
// Specify a function to be called before rendering process begins.
// Function will be called with argument provided.
void vtkRenderer::SetStartRenderMethod(void (*f)(void *), void *arg)
{
  if ( f != this->StartRenderMethod || arg != this->StartRenderMethodArg )
    {
    // delete the current arg if there is one and a delete meth
    if ((this->StartRenderMethodArg)&&(this->StartRenderMethodArgDelete))
      {
      (*this->StartRenderMethodArgDelete)(this->StartRenderMethodArg);
      }
    this->StartRenderMethod = f;
    this->StartRenderMethodArg = arg;
    this->Modified();
    }
}

// Description:
// Set the arg delete method. This is used to free user memory.
void vtkRenderer::SetStartRenderMethodArgDelete(void (*f)(void *))
{
  if ( f != this->StartRenderMethodArgDelete)
    {
    this->StartRenderMethodArgDelete = f;
    this->Modified();
    }
}

// Description:
// Set the arg delete method. This is used to free user memory.
void vtkRenderer::SetEndRenderMethodArgDelete(void (*f)(void *))
{
  if ( f != this->EndRenderMethodArgDelete)
    {
    this->EndRenderMethodArgDelete = f;
    this->Modified();
    }
}

// Description:
// Specify a function to be called when rendering process completes.
// Function will be called with argument provided.
void vtkRenderer::SetEndRenderMethod(void (*f)(void *), void *arg)
{
  if ( f != this->EndRenderMethod || arg != EndRenderMethodArg )
    {
    // delete the current arg if there is one and a delete meth
    if ((this->EndRenderMethodArg)&&(this->EndRenderMethodArgDelete))
      {
      (*this->EndRenderMethodArgDelete)(this->EndRenderMethodArg);
      }
    this->EndRenderMethod = f;
    this->EndRenderMethodArg = arg;
    this->Modified();
    }
}

void vtkRenderer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os,indent);

  os << indent << "Actors:\n";
  this->Actors.PrintSelf(os,indent.GetNextIndent());
  os << indent << "Ambient: (" << this->Ambient[0] << ", " 
    << this->Ambient[1] << ", " << this->Ambient[2] << ")\n";
  os << indent << "Aspect: (" << this->Aspect[0] << ", " 
    << this->Aspect[1] << ")\n";
  os << indent << "Background: (" << this->Background[0] << ", " 
    << this->Background[1] << ", "  << this->Background[2] << ")\n";

  os << indent << "DisplayPoint: ("  << this->DisplayPoint[0] << ", " 
    << this->DisplayPoint[1] << ", " << this->DisplayPoint[2] << ")\n";
  os << indent << "Lights:\n";
  this->Lights.PrintSelf(os,indent.GetNextIndent());

  os << indent << "ViewPoint: (" << this->ViewPoint[0] << ", " 
    << this->ViewPoint[1] << ", " << this->ViewPoint[2] << ")\n";
  os << indent << "Viewport: (" << this->Viewport[0] << ", " 
    << this->Viewport[1] << ", " << this->Viewport[2] << ", " 
      << this->Viewport[3] << ")\n";

  os << indent << "Two-sided Lighting: " 
     << (this->TwoSidedLighting ? "On\n" : "Off\n");

  if ( this->StartRenderMethod )
    {
    os << indent << "Start Render method defined.\n";
    }
  else
    {
    os << indent << "No Start Render method.\n";
    }

  if ( this->EndRenderMethod )
    {
    os << indent << "End Render method defined.\n";
    }
  else
    {
    os << indent << "No End Render method.\n";
    }

}

int vtkRenderer::VisibleActorCount()
{
  vtkActor *anActor;
  int count = 0;

  // loop through actors
  for (this->Actors.InitTraversal(); (anActor = this->Actors.GetNextItem()); )
    if (anActor->GetVisibility())
      count++;

  return count;
}

int vtkRenderer::VisibleVolumeCount()
{
  int count = 0;

  if( this->NewVolumeRenderer )
    count++;

  return count;

}
