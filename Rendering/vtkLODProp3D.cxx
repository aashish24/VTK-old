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
#include <stdlib.h>
#include <math.h>

#include "vtkLODProp3D.h"
#include "vtkActor.h"
#include "vtkVolume.h"

#define VTK_INDEX_NOT_IN_USE    -1

#define VTK_INVALID_LOD_INDEX   -2

#define VTK_LOD_ACTOR_TYPE       1
#define VTK_LOD_VOLUME_TYPE      2

// Construct a new vtkLODProp3D. Automatic LOD selection is on, there are
// no LODs.
vtkLODProp3D::vtkLODProp3D()
{
  this->NumberOfEntries               = 0;
  this->NumberOfLODs                  = 0;
  this->LODs                          = NULL;
  this->CurrentIndex                  = 1000;
  this->AutomaticLODSelection         = 1;
  this->SelectedLODID                 = 1000;
  this->SelectedLODIndex              = -1;
}

// Destruct the vtkLODProp3D. Delete the vtkProp3Ds that were created
// for the LODs. Delete the array of LODs.
vtkLODProp3D::~vtkLODProp3D()
{
  int i;

  // Delete the vtkProp3D objects that were created
  for ( i = 0; i < this->NumberOfEntries; i++ )
    {
    if ( this->LODs[i].ID != VTK_INDEX_NOT_IN_USE )
      {
      this->LODs[i].Prop3D->Delete();
      }
    }

  // Delete the array of LODs
  if ( this->NumberOfEntries > 0 )
    {
      delete [] this->LODs;
    }
}

int vtkLODProp3D::ConvertIDToIndex( int id )
{
  int    index=0;

  while ( index < this->NumberOfEntries && this->LODs[index].ID != id )
    {
    index++;
    }
  if ( index == this->NumberOfEntries )
    {
    vtkErrorMacro( << "Could not locate ID: " << id );
    index = VTK_INVALID_LOD_INDEX;
    }

  return index;
}


// Get the next available entry index
int vtkLODProp3D::GetNextEntryIndex()
{
  int                 index;
  int                 i;
  int                 amount;
  vtkLODProp3DEntry   *newLODs;

  // Search for an available index
  index = 0;
  while ( index < this->NumberOfEntries && this->LODs[index].ID != 
	  VTK_INDEX_NOT_IN_USE )
    {
    index++;
    }

  // If an available index was not found, we need more entries
  if ( index >= this->NumberOfEntries )
    {
    // If we have no entries, create 10. If we already have some, create
    // twice as many as we already have.
    amount = (this->NumberOfEntries)?(this->NumberOfEntries*2):(10);
    
    // Make the new array
    newLODs = new vtkLODProp3DEntry[amount];

    // Copy the old entries into the new array
    for ( i = 0; i < this->NumberOfEntries; i++ )
      {
      newLODs[i].Prop3D        = this->LODs[i].Prop3D;
      newLODs[i].Prop3DType    = this->LODs[i].Prop3DType;
      newLODs[i].ID            = this->LODs[i].ID;
      newLODs[i].EstimatedTime = this->LODs[i].EstimatedTime;
      }

    // This is the index that we will return - one past the old entries
    index = i;

    // Initialize the new entries to default values
    for ( ; i < amount; i++ )
      {
      newLODs[i].Prop3D = NULL;
      newLODs[i].ID     = VTK_INDEX_NOT_IN_USE;
      }

    // Delete the old array and set the pointer to the new one
    delete [] this->LODs;
    this->LODs = newLODs;

    // Set the new number of entries that we have
    this->NumberOfEntries = amount;
    }
  return index;
}

// Get the bounds of this prop. This is just the max bounds of all LODs
float *vtkLODProp3D::GetBounds()
{
  float newBounds[6];
  int   i;
  int   first = 1;

  // Loop through all valid entries
  for ( i = 0; i < this->NumberOfEntries; i++ )
    {
    if ( this->LODs[i].ID != VTK_INDEX_NOT_IN_USE )
      {
      // Get the bounds of this entry
      this->LODs[i].Prop3D->GetBounds(newBounds);

      // If this is the first entry, this is the current bounds
      if ( first )
	{
	memcpy( this->Bounds, newBounds, 6*sizeof(float) );
	first = 0;
	}
      // If this is not the first entry, compare these bounds with the
      // current bounds expanding the current ones as necessary
      else
	{
	this->Bounds[0] = 
	  (newBounds[0] < this->Bounds[0])?(newBounds[0]):(this->Bounds[0]);
	this->Bounds[1] = 
	  (newBounds[1] > this->Bounds[1])?(newBounds[1]):(this->Bounds[1]);
	this->Bounds[2] = 
	  (newBounds[2] < this->Bounds[2])?(newBounds[2]):(this->Bounds[2]);
	this->Bounds[3] = 
	  (newBounds[3] > this->Bounds[3])?(newBounds[3]):(this->Bounds[3]);
	this->Bounds[4] = 
	  (newBounds[4] < this->Bounds[4])?(newBounds[4]):(this->Bounds[4]);
	this->Bounds[5] = 
	  (newBounds[5] > this->Bounds[5])?(newBounds[5]):(this->Bounds[5]);
	}
      }
    }

  return this->Bounds;
}

// Method to remove a LOD based on an ID
void vtkLODProp3D::RemoveLOD( int id )
{
  int index = this->ConvertIDToIndex( id );

  if ( index == VTK_INVALID_LOD_INDEX )
    {
    return;
    }

  this->LODs[index].Prop3D->Delete();
  this->LODs[index].ID = VTK_INDEX_NOT_IN_USE;
  this->NumberOfLODs--;
}


// Convenience method to get the ID of the LOD that was used
// during the last render
int vtkLODProp3D::GetLastRenderedLODID( )
{
  // Check if the selected index is in range
  if ( this->SelectedLODIndex < 0 ||
       this->SelectedLODIndex >= this->NumberOfEntries )
    {
    return -1;
    }

  // Check if the selected index is valid
  if ( this->LODs[this->SelectedLODIndex].ID == VTK_INDEX_NOT_IN_USE )
    {
    return -1;
    }

  return this->LODs[this->SelectedLODIndex].ID;
}

// Convenience method to get the estimated render time for a given LOD
// based on an ID (the number returned when the LOD was added)
float vtkLODProp3D::GetLODEstimatedRenderTime( int id )
{
  int index = this->ConvertIDToIndex( id );

  if ( index != VTK_INVALID_LOD_INDEX )
    {
    return GetLODIndexEstimatedRenderTime( index );
    }
  else
    {
    return 0.0;
    }
}

float vtkLODProp3D::GetLODIndexEstimatedRenderTime( int index )
{
  float  estimatedTime;

  // For stability, blend in the new time - 75% old + 25% new
  estimatedTime = this->LODs[index].Prop3D->GetEstimatedRenderTime();
  if ( this->LODs[index].EstimatedTime != 0.0 )
    {
    estimatedTime = 
      0.75 * this->LODs[index].EstimatedTime +
      0.25 * estimatedTime;
    }

  return estimatedTime;
}

// Convenience method to set an actor LOD without a texture.
// Needed from tcl (for example) where null pointers are not possible
int vtkLODProp3D::AddLOD( vtkMapper *m, vtkProperty *p, float time )
{
  return this->AddLOD( m, p, (vtkTexture *)NULL, time );
}

// Convenience method to set an actor LOD without a property.
// Needed from tcl (for example) where null pointers are not possible
int vtkLODProp3D::AddLOD( vtkMapper *m, vtkTexture *t, float time )
{
  return this->AddLOD( m, (vtkProperty *)NULL, time );
}

// Convenience method to set an actor LOD without a texture or a property.
// Needed from tcl (for example) where null pointers are not possible
int vtkLODProp3D::AddLOD( vtkMapper *m, float time )
{
  return this->AddLOD( m, (vtkProperty *)NULL, (vtkTexture *)NULL, time );
} 

// The real method for adding an actor LOD.
int vtkLODProp3D::AddLOD( vtkMapper *m, vtkProperty *p, 
			  vtkTexture *t, float time )
{
  int       index;
  vtkActor  *actor;

  index = this->GetNextEntryIndex();

  actor = vtkActor::New();
  actor->SetMapper( m );
  if ( p ) 
    {
    actor->SetProperty( p );
    }

  if ( t )
    {
    actor->SetTexture( t );
    }

  this->LODs[index].Prop3D        = (vtkProp3D *)actor;
  this->LODs[index].Prop3DType    = VTK_LOD_ACTOR_TYPE;
  this->LODs[index].ID            = this->CurrentIndex++;
  this->LODs[index].EstimatedTime = time;

  this->NumberOfLODs++;

  return this->LODs[index].ID;
}

// Convenience method to set a volume LOD without a property.
// Needed from tcl (for example) where null pointers are not possible
int vtkLODProp3D::AddLOD( vtkVolumeMapper *m, float time )
{
  return this->AddLOD( m, (vtkVolumeProperty *)NULL, time );
}

// The real method for adding a volume LOD.
int vtkLODProp3D::AddLOD( vtkVolumeMapper *m, vtkVolumeProperty *p, 
			  float time )
{
  int       index;
  vtkVolume  *volume;

  index = this->GetNextEntryIndex();

  volume = vtkVolume::New();
  volume->SetMapper( m );
  if ( p ) 
    {
    volume->SetProperty( p );
    }

  this->LODs[index].Prop3D        = (vtkProp3D *)volume;
  this->LODs[index].Prop3DType    = VTK_LOD_VOLUME_TYPE;
  this->LODs[index].ID            = this->CurrentIndex++;
  this->LODs[index].EstimatedTime = time;

  this->NumberOfLODs++;

  return this->LODs[index].ID;
}

// Set the mapper for an LOD that is an actor
void vtkLODProp3D::SetLODMapper( int id, vtkMapper *m )
{
  int index = this->ConvertIDToIndex( id );

  if ( index == VTK_INVALID_LOD_INDEX )
    {
    return;
    }

  if ( this->LODs[index].Prop3DType != VTK_LOD_ACTOR_TYPE )
    {
    vtkErrorMacro( << "Error: Cannot set an actor mapper on a non-actor!");
    return;
    }

  ((vtkActor *)this->LODs[index].Prop3D)->SetMapper( m );
}

// Get the mapper for an LOD that is an actor
void vtkLODProp3D::GetLODMapper( int id, vtkMapper *m )
{
  int index = this->ConvertIDToIndex( id );

  if ( index == VTK_INVALID_LOD_INDEX )
    {
    return;
    }

  if ( this->LODs[index].Prop3DType != VTK_LOD_ACTOR_TYPE )
    {
    vtkErrorMacro( << "Error: Cannot get an actor mapper on a non-actor!");
    return;
    }

  m = ((vtkActor *)this->LODs[index].Prop3D)->GetMapper();
}

// Set the mapper for an LOD that is a volume
void vtkLODProp3D::SetLODMapper( int id, vtkVolumeMapper *m )
{
  int index = this->ConvertIDToIndex( id );

  if ( index == VTK_INVALID_LOD_INDEX )
    {
    return;
    }

  if ( this->LODs[index].Prop3DType != VTK_LOD_VOLUME_TYPE )
    {
    vtkErrorMacro( << "Error: Cannot set a volume mapper on a non-volume!");
    return;
    }

  ((vtkVolume *)this->LODs[index].Prop3D)->SetMapper( m );
}

// Get the mapper for an LOD that is an actor
void vtkLODProp3D::GetLODMapper( int id, vtkVolumeMapper *m )
{
  int index = this->ConvertIDToIndex( id );

  if ( index == VTK_INVALID_LOD_INDEX )
    {
    return;
    }

  if ( this->LODs[index].Prop3DType != VTK_LOD_VOLUME_TYPE )
    {
    vtkErrorMacro( << "Error: Cannot get a volume mapper on a non-volume!");
    return;
    }

  m = ((vtkVolume *)this->LODs[index].Prop3D)->GetMapper();
}

// Set the property for an LOD that is an actor
void vtkLODProp3D::SetLODProperty( int id, vtkProperty *p )
{
  int index = this->ConvertIDToIndex( id );

  if ( index == VTK_INVALID_LOD_INDEX )
    {
    return;
    }

  if ( this->LODs[index].Prop3DType != VTK_LOD_ACTOR_TYPE )
    {
    vtkErrorMacro( << "Error: Cannot set an actor property on a non-actor!");
    return;
    }

  ((vtkActor *)this->LODs[index].Prop3D)->SetProperty( p );
}

// Get the property for an LOD that is an actor
void vtkLODProp3D::GetLODProperty( int id, vtkProperty *p )
{
  int index = this->ConvertIDToIndex( id );

  if ( index == VTK_INVALID_LOD_INDEX )
    {
    return;
    }

  if ( this->LODs[index].Prop3DType != VTK_LOD_ACTOR_TYPE )
    {
    vtkErrorMacro( << "Error: Cannot get an actor property on a non-actor!");
    return;
    }

  p = ((vtkActor *)this->LODs[index].Prop3D)->GetProperty();
}

// Set the property for an LOD that is a volume
void vtkLODProp3D::SetLODProperty( int id, vtkVolumeProperty *p )
{
  int index = this->ConvertIDToIndex( id );

  if ( index == VTK_INVALID_LOD_INDEX )
    {
    return;
    }

  if ( this->LODs[index].Prop3DType != VTK_LOD_VOLUME_TYPE )
    {
    vtkErrorMacro( << "Error: Cannot set a volume property on a non-volume!");
    return;
    }

  ((vtkVolume *)this->LODs[index].Prop3D)->SetProperty( p );
}

// Get the property for an LOD that is an actor
void vtkLODProp3D::GetLODProperty( int id, vtkVolumeProperty *p )
{
  int index = this->ConvertIDToIndex( id );

  if ( index == VTK_INVALID_LOD_INDEX )
    {
    return;
    }

  if ( this->LODs[index].Prop3DType != VTK_LOD_VOLUME_TYPE )
    {
    vtkErrorMacro( << "Error: Cannot get a volume property on a non-volume!");
    return;
    }

  p = ((vtkVolume *)this->LODs[index].Prop3D)->GetProperty();
}

// Set the texture for an LOD that is an actor
void vtkLODProp3D::SetLODTexture( int id, vtkTexture *t )
{
  int index = this->ConvertIDToIndex( id );

  if ( index == VTK_INVALID_LOD_INDEX )
    {
    return;
    }

  if ( this->LODs[index].Prop3DType != VTK_LOD_ACTOR_TYPE )
    {
    vtkErrorMacro( << "Error: Cannot set an actor texture on a non-actor!");
    return;
    }

  ((vtkActor *)this->LODs[index].Prop3D)->SetTexture( t );
}

// Get the texture for an LOD that is an actor
void vtkLODProp3D::GetLODTexture( int id, vtkTexture *t )
{
  int index = this->ConvertIDToIndex( id );

  if ( index == VTK_INVALID_LOD_INDEX )
    {
    return;
    }

  if ( this->LODs[index].Prop3DType != VTK_LOD_ACTOR_TYPE )
    {
    vtkErrorMacro( << "Error: Cannot get an actor texture on a non-actor!");
    return;
    }

  t = ((vtkActor *)this->LODs[index].Prop3D)->GetTexture();
}

// Release any graphics resources that any of the LODs might be using
// for a particular window (such as display lists). 
void vtkLODProp3D::ReleaseGraphicsResources(vtkWindow *w)
{
  int i;

  // Loop through all LODs and pass this message along
  for ( i = 0; i < this->NumberOfEntries; i++ )
    {
    if ( this->LODs[i].ID != VTK_INDEX_NOT_IN_USE )
      {
      this->LODs[i].Prop3D->ReleaseGraphicsResources( w );
      }
    }
}

// Does the selected LOD need ray casting?
int vtkLODProp3D::RequiresRayCasting( )
{
  // Check if the selected index is in range
  if ( this->SelectedLODIndex < 0 ||
       this->SelectedLODIndex >= this->NumberOfEntries )
    {
    vtkErrorMacro( << "Index out of range!" );
    return 0;
    }

  // Check if the selected index is valid
  if ( this->LODs[this->SelectedLODIndex].ID == VTK_INDEX_NOT_IN_USE )
    {
    vtkErrorMacro( << "Index not valid!" );
    return 0;
    }

  // Actually ask the question
  return this->LODs[this->SelectedLODIndex].Prop3D->RequiresRayCasting();
}

// Does the selected LOD need to be rendered into an image?
int vtkLODProp3D::RequiresRenderingIntoImage( )
{
  // Check if the selected index is in range
  if ( this->SelectedLODIndex < 0 ||
       this->SelectedLODIndex >= this->NumberOfEntries )
    {
    vtkErrorMacro( << "Index out of range!" );
    return 0;
    }

  // Check if the selected index is valid
  if ( this->LODs[this->SelectedLODIndex].ID == VTK_INDEX_NOT_IN_USE )
    {
    vtkErrorMacro( << "Index not valid!" );
    return 0;
    }

  // Actually ask the question
  return 
    this->LODs[this->SelectedLODIndex].Prop3D->RequiresRenderingIntoImage();
}

// Standard render method - render any opaque geometry in the selected LOD
int vtkLODProp3D::RenderOpaqueGeometry(vtkViewport *viewport)
{
  int retval;

  // Check if the selected index is in range
  if ( this->SelectedLODIndex < 0 ||
       this->SelectedLODIndex >= this->NumberOfEntries )
    {
    vtkErrorMacro( << "Index out of range!" );
    return 0;
    }

  // Check if the selected index is valid
  if ( this->LODs[this->SelectedLODIndex].ID == VTK_INDEX_NOT_IN_USE )
    {
    vtkErrorMacro( << "Index not valid!" );
    return 0;
    }

  // Actually do the rendering
  retval = 
    this->LODs[this->SelectedLODIndex].Prop3D->RenderOpaqueGeometry(viewport);

  this->EstimatedRenderTime += 
    this->LODs[this->SelectedLODIndex].Prop3D->GetEstimatedRenderTime();

  return retval;
}

// Standard render method - render any translucent geometry in the selected LOD
int vtkLODProp3D::RenderTranslucentGeometry(vtkViewport *viewport)
{
  int retval;

  // Check if the selected index is in range
  if ( this->SelectedLODIndex < 0 ||
       this->SelectedLODIndex >= this->NumberOfEntries )
    {
    vtkErrorMacro( << "Index out of range!" );
    return 0;
    }

  // Check if the selected index is valid
  if ( this->LODs[this->SelectedLODIndex].ID == VTK_INDEX_NOT_IN_USE )
    {
    vtkErrorMacro( << "Index not valid!" );
    return 0;
    }

  // Actually do the rendering
  retval = this->LODs[this->SelectedLODIndex].Prop3D->
    RenderTranslucentGeometry(viewport);

  this->EstimatedRenderTime += 
    this->LODs[this->SelectedLODIndex].Prop3D->GetEstimatedRenderTime();
  
  return retval;
} 


// Standard render method - render the selected LOD into an image
int vtkLODProp3D::RenderIntoImage(vtkViewport *viewport)
{
  // Check if the selected index is in range
  if ( this->SelectedLODIndex < 0 ||
       this->SelectedLODIndex >= this->NumberOfEntries )
    {
    vtkErrorMacro( << "Index out of range!" );
    return 0;
    }

  // Check if the selected index is valid
  if ( this->LODs[this->SelectedLODIndex].ID == VTK_INDEX_NOT_IN_USE )
    {
    vtkErrorMacro( << "Index not valid!" );
    return 0;
    }

  // Actually do the rendering
  return this->LODs[this->SelectedLODIndex].Prop3D->RenderIntoImage(viewport);
}

// Standard render method - cast a view ray for the selected LOD
int vtkLODProp3D::CastViewRay( VTKRayCastRayInfo *rayInfo )
{
  // Don't do any error checking - assume this won't be called unless
  // RequiresRayCasting() return 1 - error checking was performed there
  return this->LODs[this->SelectedLODIndex].Prop3D->CastViewRay(rayInfo);
}

// Standard render method - initialize ray casting for the selected LOD
int vtkLODProp3D::InitializeRayCasting( vtkViewport *viewport)
{
  // Don't do any error checking - assume this won't be called unless
  // RequiresRayCasting() return 1 - error checking was performed there
  return 
    this->LODs[this->SelectedLODIndex].Prop3D->InitializeRayCasting(viewport);
}

// Override the method from vtkProp - add to both this prop and the prop of
// the selected LOD
void vtkLODProp3D::AddEstimatedRenderTime( float t )
{
  // Add to this prop's estimated render time
  this->EstimatedRenderTime += t;

  // Check if the selected index is in range
  if ( this->SelectedLODIndex < 0 ||
       this->SelectedLODIndex >= this->NumberOfEntries )
    {
    vtkErrorMacro( << "Index out of range!" );
    return;
    }

  // Check if the selected index is valid
  if ( this->LODs[this->SelectedLODIndex].ID == VTK_INDEX_NOT_IN_USE )
    {
    vtkErrorMacro( << "Index not valid!" );
    return;
    }
  
  // Now that error checking is done, add to the estimated render time
  // of the selected LOD
  this->LODs[this->SelectedLODIndex].Prop3D->AddEstimatedRenderTime(t);
}

// Set the allocated render time - this is where the decision is made
// as to which LOD to select
void vtkLODProp3D::SetAllocatedRenderTime( float t )
{
  int    i;
  int    index = -1;
  float  bestTime;
  float  targetTime;
  float  estimatedTime;
  float  factor;


  if ( this->AutomaticLODSelection )
    {
    bestTime = -1.0;

    targetTime = t;

    for ( i = 0; i < this->NumberOfEntries; i++ )
      {
      if ( this->LODs[i].ID != VTK_INDEX_NOT_IN_USE )
	{
	// Gather some information
	estimatedTime = this->GetLODIndexEstimatedRenderTime(i);
	
	// If we've never rendered this LOD and we have no info on it,
	// then try it out
	if ( estimatedTime == 0.0 )
	  {
	  index = i;
	  bestTime = 0.0;
	  break;
	  }
	
	// If we do have at least a guess as to the render time, and
	// this seems like the best we have so far, pick it.
	// It is the best we have if 
	//
	// 1) our estimated time is less than what we are looking for, 
	//    but greater than any we have selected so far. 
	//
	// 2) we have not selected anything else yet 
	//    (regardless of what the estimated time is)
	//
	// 3) it is less than the time of the currently selected LOD 
	//    if that LOD's time is greater than the time we are targeting.
	//
	if ( estimatedTime > 0.0 && 
	     ( ( estimatedTime > bestTime && estimatedTime < targetTime ) ||
	       ( bestTime == -1.0 ) ||
	       ( estimatedTime < bestTime && bestTime > targetTime ) ) )
	  {
	  index = i;
	  bestTime = estimatedTime;
	  }
	}
      }
    }
  else
    {
    index = 0;
    while ( index < this->NumberOfEntries && this->LODs[index].ID != 
	    this->SelectedLODID )
      {
      index++;
      }
    if ( index == this->NumberOfEntries )
      {
      vtkErrorMacro( << "Could not render selected LOD ID: " << 
                     this->SelectedLODID );
      index = 0;
      while ( index < this->NumberOfEntries && this->LODs[index].ID != 
	      VTK_INDEX_NOT_IN_USE )
	{
	index++;
	}
      }
    
    }

  this->SelectedLODIndex = index;
  this->LODs[this->SelectedLODIndex].Prop3D->SetAllocatedRenderTime( t );
  this->EstimatedRenderTime = 0.0;
  this->AllocatedRenderTime = t;

  // Push the matrix down into the selected LOD
  vtkMatrix4x4 *matrix = vtkMatrix4x4::New();
  this->GetMatrix( matrix );
  this->LODs[this->SelectedLODIndex].Prop3D->SetUserMatrix( matrix );
  matrix->Delete();

}

void vtkLODProp3D::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkProp3D::PrintSelf(os,indent);

  os << indent << "Number Of LODs: " << this->NumberOfLODs << endl;

  os << indent << "Selected LOD ID: " << this->SelectedLODID << endl;

  os << indent << "AutomaticLODSelection: " 
     << (this->AutomaticLODSelection ? "On\n" : "Off\n");

}

