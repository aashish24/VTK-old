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
#include "vtkPropAssembly.h"
#include "vtkViewport.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkPropAssembly* vtkPropAssembly::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkPropAssembly");
  if(ret)
    {
    return (vtkPropAssembly*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkPropAssembly;
}




// Construct object with no children.
vtkPropAssembly::vtkPropAssembly()
{
  this->Parts = vtkPropCollection::New();
}

vtkPropAssembly::~vtkPropAssembly()
{
  this->Parts->Delete();
  this->Parts = NULL;
}

// Add a part to the list of Parts.
void vtkPropAssembly::AddPart(vtkProp *prop)
{
  if ( ! this->Parts->IsItemPresent(prop) )
    {
    this->Parts->AddItem(prop);
    this->Modified();
    } 
}

// Remove a part from the list of parts,
void vtkPropAssembly::RemovePart(vtkProp *prop)
{
  if ( this->Parts->IsItemPresent(prop) )
    {
    this->Parts->RemoveItem(prop);
    this->Modified();
    } 
}

// Get the list of parts for this prop assembly.
vtkPropCollection *vtkPropAssembly::GetParts() 
{
  return this->Parts;
}

// Render this assembly and all of its Parts. The rendering process is recursive.
int vtkPropAssembly::RenderTranslucentGeometry(vtkViewport *ren)
{
  vtkProp *part;
  float fraction;
  int renderedSomething=0;

  fraction = this->AllocatedRenderTime / 
             (float)this->Parts->GetNumberOfItems();
  
  // render the Paths
  for ( this->Parts->InitTraversal(); (part=this->Parts->GetNextProp()); )
    {
    if ( part->GetVisibility() )
      {
      part->SetAllocatedRenderTime(fraction);
      renderedSomething |= part->RenderTranslucentGeometry(ren);
      }
    }

  return renderedSomething;
}

// Render this assembly and all its parts. The rendering process is recursive.
int vtkPropAssembly::RenderOpaqueGeometry(vtkViewport *ren)
{
  vtkProp *part;
  float fraction;
  int   renderedSomething=0;

  fraction = this->AllocatedRenderTime / 
             (float)this->Parts->GetNumberOfItems();
  
  // render the Paths
  for ( this->Parts->InitTraversal(); (part=this->Parts->GetNextProp()); )
    {
    if ( part->GetVisibility() )
      {
      part->SetAllocatedRenderTime(fraction);
      renderedSomething |= part->RenderOpaqueGeometry(ren);
      }
    }

  return renderedSomething;
}

// Render this assembly and all its parts. The rendering process is recursive.
int vtkPropAssembly::RenderOverlay(vtkViewport *ren)
{
  vtkProp *part;
  float fraction;
  int   renderedSomething=0;

  fraction = this->AllocatedRenderTime / 
             (float)this->Parts->GetNumberOfItems();
  
  // render the Paths
  for ( this->Parts->InitTraversal(); (part=this->Parts->GetNextProp()); )
    {
    if ( part->GetVisibility() )
      {
      part->SetAllocatedRenderTime(fraction);
      renderedSomething |= part->RenderOverlay(ren);
      }
    }

  return renderedSomething;
}

// Render this assembly and all its parts. The rendering process is recursive.
int vtkPropAssembly::InitializeRayCasting(vtkViewport *ren)
{
  vtkProp *part;
  int needsToCast=0;

  // render the Paths
  for ( this->Parts->InitTraversal(); (part=this->Parts->GetNextProp()); )
    {
    if ( part->GetVisibility() )
      {
      needsToCast |= part->InitializeRayCasting(ren);
      }
    }

  return needsToCast;
}

int vtkPropAssembly::CastViewRay(VTKRayCastRayInfo *ray)
{
  vtkProp *part;
  int rayHit=0;

  // render the Paths
  for ( this->Parts->InitTraversal(); (part=this->Parts->GetNextProp()); )
    {
    if ( part->GetVisibility() )
      {
      rayHit |= part->CastViewRay(ray);
      }
    }

  return rayHit;
}

int vtkPropAssembly::RenderIntoImage(vtkViewport *ren)
{
  vtkProp *part;
  int success=0;

  // render the Paths
  for ( this->Parts->InitTraversal(); (part=this->Parts->GetNextProp()); )
    {
    if ( part->GetVisibility() )
      {
      success |= part->RenderIntoImage(ren);
      }
    }

  return success;
}

void vtkPropAssembly::ReleaseGraphicsResources(vtkWindow *renWin)
{
  vtkProp *part;

  vtkProp::ReleaseGraphicsResources(renWin);

  // broadcast the message down the Parts
  for ( this->Parts->InitTraversal(); (part=this->Parts->GetNextProp()); )
    {
    part->ReleaseGraphicsResources(renWin);
    }
}

// Get the bounds for the assembly as (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax).
float *vtkPropAssembly::GetBounds()
{
  vtkProp *part;
  int i, n;
  float *bounds, bbox[24];
  int partVisible=0;

  this->Bounds[0] = this->Bounds[2] = this->Bounds[4] = VTK_LARGE_FLOAT;
  this->Bounds[1] = this->Bounds[3] = this->Bounds[5] = -VTK_LARGE_FLOAT;
    
  for ( this->Parts->InitTraversal(); (part=this->Parts->GetNextProp()); )
    {
    if ( part->GetVisibility() )
      {
      bounds = part->GetBounds();

      if ( bounds != NULL )
        {
        //  For the purposes of GetBounds, an object is visisble only if
        //  its visibility is on and it has visible parts.
        partVisible = 1;

        // fill out vertices of a bounding box
        bbox[ 0] = bounds[1]; bbox[ 1] = bounds[3]; bbox[ 2] = bounds[5];
        bbox[ 3] = bounds[1]; bbox[ 4] = bounds[2]; bbox[ 5] = bounds[5];
        bbox[ 6] = bounds[0]; bbox[ 7] = bounds[2]; bbox[ 8] = bounds[5];
        bbox[ 9] = bounds[0]; bbox[10] = bounds[3]; bbox[11] = bounds[5];
        bbox[12] = bounds[1]; bbox[13] = bounds[3]; bbox[14] = bounds[4];
        bbox[15] = bounds[1]; bbox[16] = bounds[2]; bbox[17] = bounds[4];
        bbox[18] = bounds[0]; bbox[19] = bounds[2]; bbox[20] = bounds[4];
        bbox[21] = bounds[0]; bbox[22] = bounds[3]; bbox[23] = bounds[4];

        for (i = 0; i < 8; i++)
          {
          for (n = 0; n < 3; n++)
            {
            if (bbox[i*3+n] < this->Bounds[n*2])
              {
              this->Bounds[n*2] = bbox[i*3+n];
              }
            if (bbox[i*3+n] > this->Bounds[n*2+1])
              {
              this->Bounds[n*2+1] = bbox[i*3+n];
              }
            }
          }//for each point of box
        }//if bounds
      }//for each part
    }//for each part

  if ( ! partVisible )
    {
    return NULL;
    }
  else
    {
    return this->Bounds;
    }
}

unsigned long int vtkPropAssembly::GetMTime()
{
  unsigned long mTime=this->vtkProp::GetMTime();
  unsigned long time;
  vtkProp *part;

  for (this->Parts->InitTraversal(); (part=this->Parts->GetNextProp()); )
    {
    time = part->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }

  return mTime;
}

// Shallow copy another vtkPropAssembly.
void vtkPropAssembly::ShallowCopy(vtkPropAssembly *propAssembly)
{
  this->vtkProp::ShallowCopy(propAssembly);
  
  this->Parts->RemoveAllItems();
  propAssembly->Parts->InitTraversal();
  for (int i=0; i<0; i++)
    {
    this->AddPart(propAssembly->Parts->GetNextProp());
    }
}

void vtkPropAssembly::PrintSelf(vtkOstream& os, vtkIndent indent)
{
  vtkProp::PrintSelf(os,indent);

  os << indent << "There are: " << this->Parts->GetNumberOfItems()
     << " parts in this assembly\n";
}
