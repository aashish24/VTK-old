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
#include <string.h>

#include "vtkRayCaster.h"
#include "vtkRenderWindow.h"
#include "vtkMath.h"
#include "vtkVoxel.h"
#include "vtkTimerLog.h"
#include "vtkVolumeRayCastMapper.h"

// Swap two values
#define VTK_SWAP_VALUES( A, B, T ) T=A; A=B; B=T

// Constructor for vtkRayCaster
vtkRayCaster::vtkRayCaster()
{
  int   i;
  float scale;

  this->Threader                     = vtkMultiThreader::New();
  this->NumberOfThreads              = this->Threader->GetNumberOfThreads();
  this->SelectedImageScaleIndex      = 0;
  this->AutomaticScaleAdjustment     = 1;
  this->AutomaticScaleLowerLimit     = 0.15;
  this->BilinearImageZoom            = 0;
  this->Renderer	             = NULL;
  this->ImageRenderTime[0]           = 0.0;
  this->ImageRenderTime[1]           = 0.0;
  this->StableImageScaleCounter      = 10;
  this->PreviousAllocatedTime        = 0.0;
  this->TotalRenderTime              = 0.0;
  scale = 1.0;
  for ( i = 0; i < VTK_MAX_VIEW_RAYS_LEVEL; i++ )
    {
    this->ViewRays[i] = vtkViewRays::New();
    this->ImageScale[i]         = scale;
    this->ViewRaysStepSize[i]   = 1.0;
    scale /= 2.0;
    }
  this->ViewRays[VTK_MAX_VIEW_RAYS_LEVEL] = vtkViewRays::New();

  this->ImageScale[VTK_MAX_VIEW_RAYS_LEVEL] = 0.5;

  this->ViewToWorldTransform = vtkTransform::New();

  for ( i = 0; i < VTK_MAX_THREADS; i++ )
    {
    this->NumberOfSamplesTaken[i] = 0;
    }
}

// Destructor for vtkRayCaster
vtkRayCaster::~vtkRayCaster()
{
  int i;

  if ( this->Threader )
    {
    this->Threader->Delete();
    }

  for ( i = 0; i <= VTK_MAX_VIEW_RAYS_LEVEL; i++ )
    {
    this->ViewRays[i]->Delete();
    this->ViewRays[i] = NULL;
    }

  if ( this->ViewToWorldTransform )
    {
    this->ViewToWorldTransform->Delete();
    }

  this->SetRenderer(NULL);	
}

// Zoom the image using nearest neighbor interpolation
void vtkRayCaster::NearestNeighborZoom(float *smallImage, 
				       float *largeImage,
				       int smallDims[2],
				       int largeDims[2] )
{
  int     i, j;
  float   *iptr1, *iptr2;
  float   xscale, yscale;
  int     yoffset, offset;

  iptr2 = largeImage;

  if ( smallDims[0] > largeDims[0] ||
       smallDims[1] > largeDims[1] )
    {
    vtkErrorMacro( << "Invalid dimensions to Nearest Neighbor Zoom:\n" <<
      smallDims[0] << " " << smallDims[1] << " " << 
      largeDims[0] << " " << largeDims[1] );
    return;
    }

  xscale = ((float)(smallDims[0]))/((float)(largeDims[0]));
  yscale = ((float)(smallDims[1]))/((float)(largeDims[1]));

  for ( j = 0; j < largeDims[1]; j++ )
    {
    yoffset = ((int)((float)(j) * yscale)) * smallDims[0] * 4;
    for ( i = 0; i < largeDims[0]; i++ )
      {
      offset = yoffset + 4*((int)((float)(i) * xscale));
      iptr1 = smallImage + offset;
      memcpy( iptr2, iptr1, 4*sizeof(float) );
      iptr2 += 4;
      iptr1 += 4;
      }
    }
}

// Zoom the image using bilinear interpolation
void vtkRayCaster::BilinearZoom(float *smallImage, 
				float *largeImage,
				int smallDims[2],
				int largeDims[2] )
{
  int     i, j;
  float   *iptr1, *iptr2;
  float   xscale, yscale;
  float   y_position, x_position;
  float   x_bilin_factor, y_bilin_factor;
  float   A_coeff, B_coeff, C_coeff, D_coeff;
  float   A, B, C, D;
  float   *A_ptr, *B_ptr, *C_ptr, *D_ptr;
  float   val;
  int     yoffset, offset;

  iptr2 = largeImage;

  if ( smallDims[0] < 2 || 
       smallDims[1] < 2 ||
       largeDims[0] < 2 ||
       largeDims[1] < 2 ||
       smallDims[0] > largeDims[0] ||
       smallDims[1] > largeDims[1] )
    {
    vtkErrorMacro( << "Invalid dimensions to Bilinear Zoom:\n" <<
      smallDims[0] << " " << smallDims[1] << " " << 
      largeDims[0] << " " << largeDims[1] );
    return;
    }

  xscale = ((float)(smallDims[0]-1))/((float)(largeDims[0]-1));
  yscale = ((float)(smallDims[1]-1))/((float)(largeDims[1]-1));

  for ( j = 0; j < largeDims[1]; j++ )
    {
    y_position = (float)(j) * yscale;
    y_bilin_factor = y_position - ((int)y_position);
    yoffset = ((int)y_position) * smallDims[0] * 4;
    for ( i = 0; i < largeDims[0]; i++ )
      {
      x_position = (float)(i) * xscale;
      x_bilin_factor = x_position - ((int)x_position);
      offset = yoffset + 4*((int)x_position);
      iptr1 = smallImage + offset;

      A_coeff = (1.0 - y_bilin_factor)*(1.0 - x_bilin_factor);
      B_coeff = (1.0 - y_bilin_factor)*(x_bilin_factor);
      C_coeff = (y_bilin_factor)*(1.0 - x_bilin_factor);
      D_coeff = (y_bilin_factor)*(x_bilin_factor);

      A_ptr = (iptr1);
      B_ptr = (iptr1 + 4);
      C_ptr = (iptr1 + 4 * smallDims[0]);
      D_ptr = (iptr1 + 4 * smallDims[0] + 4);

      // Bilinearly interpolate the red channel
      A = *(A_ptr++);
      B = *(B_ptr++);
      C = *(C_ptr++);
      D = *(D_ptr++);

      val = A*A_coeff + B*B_coeff + C*C_coeff + D*D_coeff;
      *(iptr2++) = val;

      // Bilinearly interpolate the green channel
      A = *(A_ptr++);
      B = *(B_ptr++);
      C = *(C_ptr++);
      D = *(D_ptr++);

      val = A*A_coeff + B*B_coeff + C*C_coeff + D*D_coeff;
      *(iptr2++) = val;

      // Bilinearly interpolate the blue channel
      A = *(A_ptr++);
      B = *(B_ptr++);
      C = *(C_ptr++);
      D = *(D_ptr++);

      val = A*A_coeff + B*B_coeff + C*C_coeff + D*D_coeff;
      *(iptr2++) = val;

      // Bilinearly interpolate the alpha channel
      A = *(A_ptr);
      B = *(B_ptr);
      C = *(C_ptr);
      D = *(D_ptr);

      val = A*A_coeff + B*B_coeff + C*C_coeff + D*D_coeff;
      *(iptr2++) = val;
      }
    }
}

// Set the scale factor for a given level. This is used during multi-
// resolution interactive rendering
void vtkRayCaster::SetImageScale( int level, float scale )
{
  // Check if the level is out of range
  if ( level >= VTK_MAX_VIEW_RAYS_LEVEL || level < 1 )
    {
    vtkErrorMacro( << "Level: " << level << " is outside range: 1 to " << 
                   VTK_MAX_VIEW_RAYS_LEVEL-1 );
    }
  // Check if the scale is out of range
  else if ( scale < 0.01 || scale > 1.0 )
    {
    vtkErrorMacro( << "Scale: " << scale << " must be between 0.01 and 1.0" );
    }
  // Check if the scale is greater than or equal to the previous level scale
  else if ( scale >= this->ImageScale[level-1] )
    {
    vtkErrorMacro( << "Scale: " << scale << " is >= previous level scale" );
    }
  // Check if the scale is less than or equal to the next level scale
  else if ( level < VTK_MAX_VIEW_RAYS_LEVEL-1 &&
	    scale <= this->ImageScale[level+1] )
    {
    vtkErrorMacro( << "Scale: " << scale << " is <= next level scale" );
    }
  // Everything is ok - actually set it
  else
    {
    this->ImageScale[level] = scale;
    }
}

// Get the scale factor for a given level. This is used during multi-
// resolution interactive rendering
float vtkRayCaster::GetImageScale( int level )
{
  // Check for out of range level
  if ( level >= VTK_MAX_VIEW_RAYS_LEVEL || level < 0 )
    {
    vtkErrorMacro( << "Level: " << level << " is outside range: 0 to " << 
                   VTK_MAX_VIEW_RAYS_LEVEL-1 );
    return -1.0;
    }
  // Level is ok - return the ImageScale
  else
    {
    return this->ImageScale[level]; 
    }
}

// Turn the automatic scale adjustment on
void vtkRayCaster::AutomaticScaleAdjustmentOn( void )
{
  this->AutomaticScaleAdjustment = 1;
}

// Turn the automatic scale adjustment off
void vtkRayCaster::AutomaticScaleAdjustmentOff( void )
{
  // If we turn automatic scale adjustment off, we reset the selected
  // image scale index to 0 since we have been using this for other
  // purposes while automatic scale adjustment was on
  this->AutomaticScaleAdjustment = 0;
  this->SelectedImageScaleIndex  = 0;
}

void vtkRayCaster::SetViewRaysStepSize( int level, float scale )
 {
  // Check for out of range level
  if ( level >= VTK_MAX_VIEW_RAYS_LEVEL || level < 0 )
    {
    vtkErrorMacro( << "Level: " << level << " is outside range: 0 to " << 
                   VTK_MAX_VIEW_RAYS_LEVEL-1 );
    }
  else if ( scale < 0.01 || scale > 100.0 )
    {
    vtkErrorMacro( << "Scale: " << scale << 
                   " must be between 0.01 and 100.0" );
    }
  else
    {
    this->ViewRaysStepSize[level] = scale;
    }
}

float vtkRayCaster::GetViewRaysStepSize( int level )
{
  // Check for out of range level
  if ( level >= VTK_MAX_VIEW_RAYS_LEVEL || level < 0 )
    {
    vtkErrorMacro( << "Level: " << level << " is outside range: 0 to " << 
                   VTK_MAX_VIEW_RAYS_LEVEL-1 );
    return -1.0;
    }
  else
    {
    return this->ViewRaysStepSize[level]; 
    }
}

// Get the size in pixels of the view rays for the selected scale indexl
void vtkRayCaster::GetViewRaysSize(int size[2])
{
  int    *rwin_size;
  float  *vp_size;

  // get physical window dimensions
  rwin_size = this->Renderer->GetRenderWindow()->GetSize();
  vp_size = this->Renderer->GetViewport();

  size[0] = (int)(rwin_size[0]*(float)(vp_size[2] - vp_size[0]));
  size[1] = (int)(rwin_size[1]*(float)(vp_size[3] - vp_size[1]));

  size[0] = (int)((float)(size[0]) * 
		this->ImageScale[ this->SelectedImageScaleIndex ]);
  size[1] = (int)((float)(size[1]) * 
		this->ImageScale[ this->SelectedImageScaleIndex ]);
}

float *vtkRayCaster::GetPerspectiveViewRays(void)
{
  int    size[2];
  int    *rwin_size;
  float  *vp_size;

  // get physical window dimensions
  rwin_size = this->Renderer->GetRenderWindow()->GetSize();
  vp_size = this->Renderer->GetViewport();

  size[0] = (int)(rwin_size[0]*(float)(vp_size[2] - vp_size[0]));
  size[1] = (int)(rwin_size[1]*(float)(vp_size[3] - vp_size[1]));

  size[0] = (int)((float)(size[0]) * 
		this->ImageScale[ this->SelectedImageScaleIndex ]);
  size[1] = (int)((float)(size[1]) * 
		this->ImageScale[ this->SelectedImageScaleIndex ]);

  this->ViewRays[this->SelectedImageScaleIndex]->SetRenderer(this->Renderer);
  this->ViewRays[this->SelectedImageScaleIndex]->SetSize( size );

  return 
    this->ViewRays[this->SelectedImageScaleIndex]->GetPerspectiveViewRays();
}

float *vtkRayCaster::GetParallelStartPosition(void)
{
  int    size[2];
  int    *rwin_size;
  float  *vp_size; 
 
  // get physical window dimensions
  rwin_size = this->Renderer->GetRenderWindow()->GetSize();
  vp_size = this->Renderer->GetViewport();
 
  size[0] = (int)(rwin_size[0]*(float)(vp_size[2] - vp_size[0]));
  size[1] = (int)(rwin_size[1]*(float)(vp_size[3] - vp_size[1]));
 
  size[0] = (int)((float)(size[0]) * 
		this->ImageScale[ this->SelectedImageScaleIndex ]);
  size[1] = (int)((float)(size[1]) * 
		this->ImageScale[ this->SelectedImageScaleIndex ]);
 
  this->ViewRays[this->SelectedImageScaleIndex]->SetRenderer(this->Renderer);
  this->ViewRays[this->SelectedImageScaleIndex]->SetSize( size );

  return 
    this->ViewRays[this->SelectedImageScaleIndex]->GetParallelStartPosition();
}

float *vtkRayCaster::GetParallelIncrements(void)
{
  int    size[2];
  int    *rwin_size;
  float  *vp_size; 
 
  // get physical window dimensions
  rwin_size = this->Renderer->GetRenderWindow()->GetSize();
  vp_size = this->Renderer->GetViewport();
 
  size[0] = (int)(rwin_size[0]*(float)(vp_size[2] - vp_size[0]));
  size[1] = (int)(rwin_size[1]*(float)(vp_size[3] - vp_size[1]));
 
  size[0] = (int)((float)(size[0]) * 
		this->ImageScale[ this->SelectedImageScaleIndex ]);
  size[1] = (int)((float)(size[1]) * 
		this->ImageScale[ this->SelectedImageScaleIndex ]);
 
  this->ViewRays[this->SelectedImageScaleIndex]->SetRenderer(this->Renderer);
  this->ViewRays[this->SelectedImageScaleIndex]->SetSize( size );

  return 
    this->ViewRays[this->SelectedImageScaleIndex]->GetParallelIncrements();
}

// This method returns the scale that should be applied to the viewport
// for geometric rendering, and for the image in volume rendering. It 
// is either explicitly set (if AutomaticScaleAdjustment is off) or
// is adjusted automatically to get the desired frame rate.
//
// Note: IMPORTANT!!!! This should only be called once per render!!!
//
float vtkRayCaster::GetViewportScaleFactor( vtkRenderer *ren )
{
  float                time_to_render;
  float                estimated_time;
  int                  selected_level;
  int                  visible_volume;
  vtkVolumeCollection  *volumes;
  vtkVolume            *volume;
  float                estimated_scale;
  float                scale_diff;

  // loop through volumes looking for a visible one of the right type
  visible_volume = 0;
  volumes = ren->GetVolumes();

  for (volumes->InitTraversal(); (volume = volumes->GetNextItem()); )
    {
    if (volume->GetVisibility() &&
	( volume->GetVolumeMapper()->GetMapperType() == 
	  VTK_RAYCAST_VOLUME_MAPPER ||
	  volume->GetVolumeMapper()->GetMapperType() == 
	  VTK_SOFTWAREBUFFER_VOLUME_MAPPER) )
      {
      visible_volume = 1;
      break;
      }
    }

  // There's no visible volume so we shouldn't scale the image
  if ( !visible_volume )
    {
    this->SelectedImageScaleIndex = 0;
    return 1.0;
    }

  // If we aren't automatically adjusting, then just use the selected
  // level that was supplied in the SelectedImageScaleIndex variable
  if ( !this->AutomaticScaleAdjustment )
    {
    return this->ImageScale[ this->SelectedImageScaleIndex ];
    }

  // Otherwise, adjust the level to get the desired frame rate
  // First, figure out how must time we have to render ( a time of
  // 0.0 means take as long as you like )
  time_to_render = ren->GetAllocatedRenderTime();
  if ( time_to_render == 0.0 ) 
    {
    time_to_render = 10000.0;
    }

  if ( ( time_to_render - this->PreviousAllocatedTime ) >  0.05 ||
       ( time_to_render - this->PreviousAllocatedTime ) < -0.05 )
    {
    this->StableImageScaleCounter = 10;
    }

  this->PreviousAllocatedTime = time_to_render;

  // First test the full res level - is that ok?
  selected_level = 0;
  estimated_time = this->ImageRenderTime[0];

  if ( estimated_time > time_to_render )
    {
    // Full res would take too long - use the adjustable level that is
    // stored in ImageScale[VTK_MAX_VIEW_RAYS_LEVEL]
    selected_level = VTK_MAX_VIEW_RAYS_LEVEL;
    // Only allow the scale to be adjusted every 3 renders to avoid
    // trashing
    if ( this->StableImageScaleCounter > 3 )
      {
	// If we have no render time, estimate the scale from the full
	// res render time. If there is no full res render time (this
	// should not happen!) then just pick 0.1 as the scale as a
	// first guess since we have nothing to base a guess on
	if ( this->ImageRenderTime[1] == 0.0 )
	  {
	  if ( this->ImageRenderTime[0] != 0.0 )
	    {
	    estimated_scale = 
	      sqrt( (double)( time_to_render / this->ImageRenderTime[0] ) );
	    }
	  else
	    {
	    estimated_scale = 0.1;
	    }
	  }
	// There is a time for this scale - figure out how far off we
	// are from hitting our desired time
	else
	  {
	  estimated_scale = this->ImageScale[selected_level] *
	    sqrt( (double)( time_to_render / this->ImageRenderTime[1] ) );
	  }
	// Put some bounds on the scale
	if ( estimated_scale < this->AutomaticScaleLowerLimit ) 
	  {
	  estimated_scale = this->AutomaticScaleLowerLimit;
	  }
	if ( estimated_scale > 1.0 )
	  {
	  estimated_scale = 1.0;
	  }
	// How different is this from what we previously used?
	scale_diff = estimated_scale - this->ImageScale[selected_level];
	if ( scale_diff < 0 )
	  {
	  scale_diff = -scale_diff;
	  }
	// Make sure the difference is significant to avoid trashing
	if ( scale_diff > 0.02 )
	  {
	  this->ImageScale[selected_level] = estimated_scale;
	  // Reset the counter to 0 so that we have to wait 3 frames
	  // before we can adjust this scale again
	  this->StableImageScaleCounter = 0;
	  }
	else 
	  // Increment the counter since we didn't adjust the scale
	  {
	  this->StableImageScaleCounter++;
	  }
      }
    else 
      // Increment the counter since we didn't adjust the scale
      {
      this->StableImageScaleCounter++;
      }
    }
  else
    // We used the full res image so set the counter to a high number
    // so that next time we use the adjustable scale we can recompute
    // a new scale value immediately instead of having to wait 3 frames
    {
    this->StableImageScaleCounter = 10;
    }

  this->SelectedImageScaleIndex = selected_level;

  return this->ImageScale[ this->SelectedImageScaleIndex ];
}

float vtkRayCaster::GetViewportStepSize()
{
  if ( this->SelectedImageScaleIndex >= 0 &&
       this->SelectedImageScaleIndex < VTK_MAX_VIEW_RAYS_LEVEL )
    {
    return this->ViewRaysStepSize[this->SelectedImageScaleIndex];
    }
  else
    {
    return 1.0;
    }
}

// We have to keep a NumberOfSamplesTaken per thread id so that
// we don't try to increment the same location across different
// threads.
int vtkRayCaster::GetNumberOfSamplesTaken()
{
  int sum, i;

  sum = 0;
  for ( i = 0; i < VTK_MAX_THREADS; i++ )
    {
    sum += this->NumberOfSamplesTaken[i];
    }
  return sum;
}

// Initialize the buffers that we will need for rendering. If we have
// something in the frame buffer (either geometry or volumes drawn by 
// the graphics hardware) capture it for later use.
void vtkRayCaster::InitializeRenderBuffers(vtkRenderer *ren)
{
  float                *viewport;
  int                  *renWinSize;
  int                  something_in_framebuffer = 0;
  vtkActor             *anActor;
  vtkVolume            *aVolume;
  vtkVolumeCollection  *volumes;
  int                  lowerLeftCorner[2];

  // How big is this image?
  this->GetViewRaysSize( this->ImageSize );

  // How big is the viewport?
  viewport   =  ren->GetViewport();
  renWinSize = ren->GetRenderWindow()->GetSize();

  // The full image fills the viewport
  this->FullImageSize[0] = (int)((float)
				 renWinSize[0]*(viewport[2] - viewport[0]));
  this->FullImageSize[1] = (int)((float)
				 renWinSize[1]*(viewport[3] - viewport[1]));


  if ( ren->GetNumberOfPropsRenderedAsGeometry() > 0 )
    {
    something_in_framebuffer = 1;
    }

  // If we haven't found any actors in the frame buffer, check for
  // volumes that might be there
  if ( !something_in_framebuffer )
    {
    volumes = ren->GetVolumes();
    for (volumes->InitTraversal(); (aVolume = volumes->GetNextItem()); )
      {
      if (aVolume->GetVisibility() && 
	  aVolume->GetVolumeMapper()->GetMapperType() == 
	  VTK_FRAMEBUFFER_VOLUME_MAPPER )
	{
	something_in_framebuffer = 1;
	break;
	}
      }
    }

  // If we have something in the frame buffer, capture the color and z values
  if ( something_in_framebuffer )
    {
    lowerLeftCorner[0] = (int)((float)renWinSize[0]*viewport[0]);
    lowerLeftCorner[1] = (int)((float)renWinSize[1]*viewport[1]);

    this->RGBAImage =  ren->GetRenderWindow()->GetRGBAPixelData( 
			      lowerLeftCorner[0],
			      lowerLeftCorner[1],
			      lowerLeftCorner[0] + this->ImageSize[0] - 1,
			      lowerLeftCorner[1] + this->ImageSize[1] - 1,
			      0 );

    this->ZImage    =  ren->GetRenderWindow()->GetZbufferData( 
			      lowerLeftCorner[0],
			      lowerLeftCorner[1],
			      lowerLeftCorner[0] + this->ImageSize[0] - 1,
			      lowerLeftCorner[1] + this->ImageSize[1] - 1 );
    this->FirstBlend = 0;
    }
  // There is nothing in the frame buffer, so just create the color and z
  // buffers which are uninitialized. The FirstBlend variable keeps track of
  // the fact that we have not blended anything into these buffers yet.
  else
    {
    this->RGBAImage = new float [( 4 * 
				   this->ImageSize[0] * 
				   this->ImageSize[1] )];
    this->ZImage    = new float [( this->ImageSize[0] * 
				   this->ImageSize[1] )];  
    this->FirstBlend = 1;
    }
}

void vtkRayCaster::ComputeRowBounds( vtkRenderer *ren,
				     struct VolumeRayCastVolumeInfoStruct *volumeInfo )
{
  float     *bounds;
  int       i, j, k, indx, low, high;
  float     screenBounds[8][3];
  float     pointIn[4];
  int       edges[12][2] = {{0,1},{0,2},{0,4},{1,3},{1,5},{2,3},
                            {2,6},{3,7},{4,5},{4,6},{5,7},{6,7}};
  float     slope, x, x1, x2, y1, y2, dx, dy;
  float     *viewport;
  int       *renWinSize;

  bounds          = volumeInfo->Volume->GetBounds();
  renWinSize      = ren->GetRenderWindow()->GetSize();
  viewport        = ren->GetViewport();

  indx = 0;
  pointIn[3] = 1.0;
  for ( k = 0; k < 2; k++ )
    {
    pointIn[2] = bounds[4+k];
    for ( j = 0; j < 2; j++ )
      {
      pointIn[1] = bounds[2+j];
      for ( i = 0; i < 2; i++ )
	{
	pointIn[0] = bounds[i];
	ren->SetWorldPoint( pointIn );
	ren->WorldToDisplay();
	ren->GetDisplayPoint( screenBounds[indx] );
	screenBounds[indx][0] = 
	  ( (screenBounds[indx][0] - viewport[0]*(float)(renWinSize[0])) / 
	    this->FullImageSize[0] ) * this->ImageSize[0];
	screenBounds[indx][1] = 
	  ( (screenBounds[indx][1] - viewport[1]*(float)(renWinSize[1])) / 
	    this->FullImageSize[1] ) * this->ImageSize[1];
	indx++;
	}
      }
    }

  if ( volumeInfo->RowBoundsSize != this->ImageSize[1] )
    {
    if ( volumeInfo->RowBounds )
      {
      delete volumeInfo->RowBounds;
      }
    volumeInfo->RowBounds = new int [this->ImageSize[1]*2];
    volumeInfo->RowBoundsSize = this->ImageSize[1];
    }

  for ( i = 0; i < this->ImageSize[1]; i++ )
    {
    volumeInfo->RowBounds[i*2+0] = this->ImageSize[0] + 1;
    volumeInfo->RowBounds[i*2+1] = -1;
    }

  for ( i = 0; i < 12; i++ )
    {
    x1 = screenBounds[edges[i][0]][0];  
    y1 = screenBounds[edges[i][0]][1];  
    x2 = screenBounds[edges[i][1]][0];  
    y2 = screenBounds[edges[i][1]][1];  
    
    if ( y2 < y1 ) 
      {
      low = y2;
      high = y1;
      }
    else
      {
      low = y1;
      high = y2;
      }

    if ( low < 0 )
      {
      low = 0;
      }

    if ( high > (this->ImageSize[1] - 1) )
      {
      high = this->ImageSize[1] - 1;
      }

    dx = x1-x2;
    dy = y1-y2;
    if ( dy )
      {
      if ( dx )
	{
	slope = dy/dx;
	for ( j = low; j <= high; j++ )
	  {
	  x = x1 - (y1-j)/slope;
	  if ( ((int)x - 1) < volumeInfo->RowBounds[2*j+0] )
	    {
	    volumeInfo->RowBounds[2*j+0] = (int)x - 1;
	    }
	  if ( ((int)x + 1) > volumeInfo->RowBounds[2*j+1] )
	    {
	    volumeInfo->RowBounds[2*j+1] = (int)x + 1;
	    }
	  }
	}
      else
	{
	for ( j = low; j <= high; j++ )
	  {
	  if ( ((int)x1 - 1) < volumeInfo->RowBounds[2*j+0] )
	    {
	    volumeInfo->RowBounds[2*j+0] = (int)x1 - 1;
	    }
	  if ( ((int)x1 + 1) > volumeInfo->RowBounds[2*j+1] )
	    {
	    volumeInfo->RowBounds[2*j+1] = (int)x1 + 1;
	    }
	  }
	}
      }
    }
  
}



// Perform the initialization for ray casting. Create the temporary structures
// necessary for storing information and quick access.
void vtkRayCaster::InitializeRayCasting(vtkRenderer *ren)
{
  vtkVolumeCollection   *volumes;
  vtkVolume             *aVolume;
  int                   i, j;
  vtkTransform          *transform;
  vtkMatrix4x4          *matrix;
  float                 aspect;
  float                 ren_aspect[2];
  float                 cameraPosition[3], *volumePosition;
 
  // Get the position of the camera for use in determining the
  // distance to the center of each volume
  ren->GetActiveCamera()->GetPosition( cameraPosition );

  // Create a pointer to each volume for speedy access
  this->RayCastVolumes = new vtkVolume *[this->RayCastVolumeCount];

  // Create the volume info structure for each volume which stores
  // ray casting information that is independent of pixel but dependent on
  // the volume such as the world to volume coordinate conversion matrix.
  this->VolumeInfo = 
    new struct VolumeRayCastVolumeInfoStruct [this->RayCastVolumeCount];

  // Get the pointer to each of the ray cast volume, and initialize the
  // mapper, passing in the info structure so that information can be
  // stored there
  i = 0;
  volumes = ren->GetVolumes();
  for ( volumes->InitTraversal(); (aVolume = volumes->GetNextItem()); )
    {
    // Check visibility of volume 
    if( aVolume->GetVisibility() &&
	aVolume->GetVolumeMapper()->GetMapperType() == 
	VTK_RAYCAST_VOLUME_MAPPER )
	{
	this->RayCastVolumes[i] = aVolume;
	this->VolumeInfo[i].Volume = aVolume;
        volumePosition = aVolume->GetCenter();
	this->VolumeInfo[i].CenterDistance = sqrt( (double) 
	       ( ( cameraPosition[0] - volumePosition[0] ) *   
		 ( cameraPosition[0] - volumePosition[0] ) +
		 ( cameraPosition[1] - volumePosition[1] ) *
		 ( cameraPosition[1] - volumePosition[1] ) +
		 ( cameraPosition[2] - volumePosition[2] ) *
		 ( cameraPosition[2] - volumePosition[2] ) ) );
	((vtkVolumeRayCastMapper *)
	 (aVolume->GetVolumeMapper()))->InitializeRender(ren, aVolume,
							 &this->VolumeInfo[i]);
	this->VolumeInfo[i].RowBounds = NULL;
	this->VolumeInfo[i].RowBoundsSize = 0;
	this->ComputeRowBounds( ren, &(this->VolumeInfo[i]) );

	i++;
	}
    }

  // Store the view to world transformation matrix for later use. This is the
  // inverse of the camera's view transform. Copy it into an array for 
  // faster processing
  this->ViewToWorldTransform->SetMatrix( 
	    ren->GetActiveCamera()->GetViewTransform() );
  this->ViewToWorldTransform->Inverse();
  for ( j = 0; j < 4; j++ )
    {
    for ( i = 0; i < 4; i++ )
      {
      this->ViewToWorldMatrix[j][i] = 
	this->ViewToWorldTransform->GetMatrixPointer()->Element[j][i];
      }
    }

  // Get the clipping range of the active camera. This will be used
  // for clipping the rays
  ren->GetActiveCamera()->GetClippingRange( this->CameraClippingRange );

  // Get the aspect ratio of the renderer
  ren->GetAspect( ren_aspect );
  aspect = ren_aspect[0]/ren_aspect[1];
  
  // Create the perspective matrix for the camera.  This will be used
  // to decode z values, so we will need to invert it
  transform       = vtkTransform::New();
  transform->SetMatrix(
     *ren->GetActiveCamera()->GetPerspectiveTransformMatrix( aspect, -1, 1 ) );
  transform->Inverse();

  // To speed things up, we pull the matrix out of the transform. 
  // This way, we can decode z values faster since we know which elements
  // of the matrix are important, and which are zero.
  matrix = transform->GetMatrixPointer();

  // Do initialization for orthographic view - we need to 
  // know the starting position of the first ray, and the increments
  // in x and y to move to each of the next rays. Also, check our assumptions
  // about the inverse camera matrix.
  if( ren->GetActiveCamera()->GetParallelProjection() )
    {
    this->ParallelProjection = 1;

    // Just checking that our assumptions are correct. 
    if( this->Debug )
      {
      if (  matrix->Element[3][0] || matrix->Element[3][1]  ||
	    matrix->Element[3][2] || (matrix->Element[3][3] != 1.0) )
	{
	vtkErrorMacro( << 
           "Assumption incorrect: cannot correctly decode z values");
	}
      }

    this->ParallelStartPosition = this->GetParallelStartPosition();
    this->ParallelIncrements = this->GetParallelIncrements();
    }
  // Check our assumptions about the inverse camera matrix for a 
  // perspective projection. No need to compute anything about the rays
  // since that info is all stored with the view rays.
  else
    {
    this->ParallelProjection = 0;

    this->SelectedViewRays = this->GetPerspectiveViewRays();

    // Just checking that our assumptions are correct. 
    if( this->Debug )
      {
      if ( matrix->Element[2][0] || matrix->Element[2][1]  ||
	   matrix->Element[3][0] || matrix->Element[3][1]  ||
	   matrix->Element[2][2] )
	{
	vtkErrorMacro( << 
           "Assumption incorrect: cannot correctly decode z values");
	}
      }
    }

    // These are the important elements of the matrix. 
    this->CameraInverse22   = matrix->Element[2][2];
    this->CameraInverse23   = matrix->Element[2][3];
    this->CameraInverse32   = matrix->Element[3][2];
    this->CameraInverse33   = matrix->Element[3][3];

    // Delete the object we created
    transform->Delete();

    // Zero out all the number of samples taken values
    for ( i = 0; i < VTK_MAX_THREADS; i++ )
      {
      this->NumberOfSamplesTaken[i] = 0;
      }
}

// This is the multithreaded piece of the rendering - it is called once 
// for each thread, and the scan lines are divided among processors with a
// simple interleaving method
VTK_THREAD_RETURN_TYPE RayCast_RenderImage( void *arg )
{
  int                          i, j, k, q;
  float                        *ray_ptr;
  float                        *iptr;
  float                        *zptr;
  int                          thread_id;
  int                          thread_count;
  float                        nearclip, farclip, farplane;
  int                          noAbort = 1;
  vtkRenderWindow              *renWin;
  vtkRayCaster                 *raycaster;
  vtkVolumeRayCastMapper       **mapper;
  float                        zm22, zm23, zm32, zm33;
  float                        *red, *green, *blue, *alpha, *depth;
  float                        tmp;
  float                        r, g, b, a, remaining_a;
  int                          icount;
  struct VolumeRayCastRayInfoStruct  rayInfo;
  int                          numSamples = 0;

  // Get the info out of the input structure
  thread_id    = ((ThreadInfoStruct *)(arg))->ThreadID;
  thread_count = ((ThreadInfoStruct *)(arg))->NumberOfThreads;
  raycaster    = (vtkRayCaster *)((ThreadInfoStruct *)arg)->UserData;

  ray_ptr = raycaster->SelectedViewRays;
  iptr    = raycaster->RGBAImage;
  zptr    = raycaster->ZImage;

  renWin = raycaster->Renderer->GetRenderWindow();

  // The clipping range of the camera is used to clip the rays
  nearclip = raycaster->CameraClippingRange[0];
  farclip = raycaster->CameraClippingRange[1];

  zm23 = raycaster->CameraInverse23;
  zm22 = raycaster->CameraInverse22;
  zm32 = raycaster->CameraInverse32;
  zm33 = raycaster->CameraInverse33;

  // We need an rgba and z value for each possible volume intersection
  // along the ray, plus one for the information that is (possibly) already
  // store in the rgba and z buffers (from hardware rendering)
  red   = new float [raycaster->RayCastVolumeCount+1];
  green = new float [raycaster->RayCastVolumeCount+1];
  blue  = new float [raycaster->RayCastVolumeCount+1];
  alpha = new float [raycaster->RayCastVolumeCount+1];
  depth = new float [raycaster->RayCastVolumeCount+1];

  // In an orthographic projection, the direction is constant
  if ( raycaster->ParallelProjection )
    {
    rayInfo.RayDirection[0] =  0.0;
    rayInfo.RayDirection[1] =  0.0;
    rayInfo.RayDirection[2] = -1.0;
    }
  // In a perspective projection, the origin in constant
  else
    {
    rayInfo.RayOrigin[0] = 0.0;
    rayInfo.RayOrigin[1] = 0.0;
    rayInfo.RayOrigin[2] = 0.0;
    }
  
  // We need to know the width of the image down in the mapper to 
  // decode z values from the ray bounder
  rayInfo.ImageWidth = raycaster->ImageSize[0];

  // Keep a pointer to each mapper for faster access
  mapper = new vtkVolumeRayCastMapper *[raycaster->RayCastVolumeCount];
  for ( k = 0; k < raycaster->RayCastVolumeCount; k++ )
    {
    mapper[k] = (vtkVolumeRayCastMapper *)
      raycaster->RayCastVolumes[k]->GetVolumeMapper();
    }

  // Loop through all rows of the image
  for ( j = 0; j < raycaster->ImageSize[1]; j++ )
    {
    // If we want this row to be computed by this thread_id
    // also need to check on abort status
    if (!thread_id)
      {
      if (noAbort && renWin->CheckAbortStatus())
	{
	noAbort = 0;
	}
      }
    else
      {
      noAbort = !(renWin->GetAbortRender());
      }
    if (noAbort && (( j % thread_count ) == thread_id ))
      {
      // Loop through each pixel in this row
      for ( i = 0; i < raycaster->ImageSize[0]; i++ )
	{
	// If we have nothing in the rgba and z buffers, we want to
	// cast the ray all the way until the far clipping plane
	if ( raycaster->FirstBlend )
	  {
	  farplane = farclip;
	  red[0]   = raycaster->Background[0];
	  green[0] = raycaster->Background[1];
	  blue[0]  = raycaster->Background[2];
	  alpha[0] = 1.0;
	  }
	// If we have something in the rgba and z buffers, then we only
	// want to cast the ray until it reaches the z value stored in
	// the z buffer, and the rgba values stored in the color buffer are
	// considered the farthest sample.
	else
	  {
	  if ( raycaster->ParallelProjection )
	    {
	    farplane = -(((*zptr)*2.0 -1.0)*zm22 + zm23);
	    }
	  else
	    {
	    farplane = -zm23 / (((*zptr)*2.0 -1.0)*zm32 + zm33 );
	    }

	  red[0]   = *iptr;
	  green[0] = *(iptr+1);
	  blue[0]  = *(iptr+2);
	  alpha[0] = 1.0;
	  }

	// This far sample is at the far plane
	depth[0] = farplane;

	// If we have an orthographic projection, the origin must be computed.
	if ( raycaster->ParallelProjection )
	  {
	  rayInfo.RayOrigin[0] = raycaster->ParallelStartPosition[0] + 
	    i * raycaster->ParallelIncrements[0];
	  rayInfo.RayOrigin[1] = raycaster->ParallelStartPosition[1] + 
	    j * raycaster->ParallelIncrements[1];
	  rayInfo.RayOrigin[2] = 0.0;
	  }
	// If we have a perspective projection, then copy the ray direction of
	// of the view rays
	else
	  {
	  memcpy( rayInfo.RayDirection, ray_ptr, 3*sizeof(float) );	
	  }

	// We need to tell the mapper which pixel this is in case there is
	// some ray bounding going on in the mapper
	rayInfo.RayPixel[1] = j;
	rayInfo.RayPixel[0] = i;

	icount = 1;

	// For each volume we need to cast this ray
	for ( k = 0; k < raycaster->RayCastVolumeCount; k++ )
	  {
	  if ( i >= raycaster->VolumeInfo[k].RowBounds[2*j+0] &&
	       i <= raycaster->VolumeInfo[k].RowBounds[2*j+1] )
	    {
	    // In an orthographic projection, near and far remain unchanged
	    if ( raycaster->ParallelProjection )
	      {
	      rayInfo.RayNearClip = nearclip;
	      rayInfo.RayFarClip  = farplane;
	      }
	    // In a perspective projection we must divide near and far by the
	    // z component of the view ray to account for the fact that this
	    // distance is a distance to a plane and we want the distance to
	    // the view origin
	    else
	      {
	      rayInfo.RayNearClip = nearclip / -ray_ptr[2];
	      rayInfo.RayFarClip  = farplane / -ray_ptr[2];
	      }

	    // Cast the ray and gather the resulting values.
	    mapper[k]->CastViewRay( &rayInfo, &raycaster->VolumeInfo[k] );
	    red[icount]   = rayInfo.RayColor[0];
	    green[icount] = rayInfo.RayColor[1];
	    blue[icount]  = rayInfo.RayColor[2];
	    alpha[icount] = rayInfo.RayColor[3];
	    numSamples += rayInfo.VolumeRayStepsTaken;

	    if ( raycaster->ParallelProjection )
	      {
	      depth[icount] = rayInfo.RayDepth + nearclip;
	      }
	    else
	      {
	      depth[icount] = rayInfo.RayDepth + nearclip / -ray_ptr[2];
	      }
	  
	    // Bubble it down - we want to have the samples ordered from
	    // farthest to closest
	    q = icount;
	    while ( q > 1 && depth[q] > depth[q-1] )
	      {
	      VTK_SWAP_VALUES(   red[q],   red[q-1], tmp );
	      VTK_SWAP_VALUES( green[q], green[q-1], tmp );
	      VTK_SWAP_VALUES(  blue[q],  blue[q-1], tmp );
	      VTK_SWAP_VALUES( alpha[q], alpha[q-1], tmp );
	      VTK_SWAP_VALUES( depth[q], depth[q-1], tmp );
	      q--;
	      }
	    icount++;
	    }
	  }

	// Merge the colors together in back to front order. 
	r = red[0];
	g = green[0];
	b = blue[0];	
	remaining_a = alpha[0];

	for ( k = 1; k < icount; k++ )
	  {
	  r = red[k]   + (1.0 - alpha[k]) * r;
	  g = green[k] + (1.0 - alpha[k]) * g;
	  b = blue[k]  + (1.0 - alpha[k]) * b; 
	  remaining_a *= 1.0 - alpha[k];
	  }

	a = 1.0 - remaining_a;

	// Check the upper bounds
	if ( r > 1.0 )
	  {
	  r = 1.0;
	  }
	if ( g > 1.0 )
	  {
	  g = 1.0;
	  }
	if ( b > 1.0 )
	  {
	  b = 1.0;
	  }
	if ( a > 1.0 )
	  {
	  a = 1.0;
	  }

	// Set the pixel value
	*(iptr++) = r;
	*(iptr++) = g;
	*(iptr++) = b;
	*(iptr++) = a;

	// Increment the pointers
	ray_ptr += 3;
	zptr++;
	}
      } // End of for each pixel loop
    else  // This is the wrong thread to compute this row of the image
      {
      ray_ptr     += 3 * raycaster->ImageSize[0];
      iptr        += 4 * raycaster->ImageSize[0];
      zptr        +=     raycaster->ImageSize[0];
      }
    } // End of for each row loop

  // Delete the temporary stuff we created
  delete mapper;
  delete red;
  delete green;
  delete blue;
  delete alpha;
  delete depth;

  raycaster->NumberOfSamplesTaken[thread_id] = numSamples;

  return VTK_THREAD_RETURN_VALUE;

}


// Render any ray cast or software buffer volumes and put the resulting
// image in the frame buffer
void vtkRayCaster::Render(vtkRenderer *ren, int raycastCount, 
			  int softwareCount )
{
  int                  deleteDisabled = 0;
  vtkVolume            *aVolume;
  vtkVolumeCollection  *volumes;
  vtkTimerLog          *timer;
  float                *nextImage, *ptr1, *ptr2;
  float                alpha;
  int                  i, j;

  // We need a timer to know how long the ray casting and software
  // buffer rendering takes. This will be used to determine what
  // size to make the image next time - the size of the image shrinks
  // to achieve a desired update render rate.
  timer = vtkTimerLog::New();
  timer->StartTimer();

  // Grab the counts that were passed in - these were computed up
  // in vtkRenderer::UpdateVolumes()
  this->RayCastVolumeCount = raycastCount;
  this->SoftwareBufferVolumeCount = softwareCount;

  // Create the RGBA and Z buffers necessary to store the
  // render image data.
  this->InitializeRenderBuffers( ren );
 
  // If we don't have any geometry then we won't capture the color buffer
  // after geometry rendering, and therefore we will not correctly account 
  // for the background color. Check to see if this is the case so we
  // can handle it.
  this->NeedBackgroundBlend = 0;
  if ( this->FirstBlend )
    {
    ren->GetBackground( this->Background );
    if ( this->Background[0] != 0.0 ||
	 this->Background[1] != 0.0 ||
	 this->Background[2] != 0.0 )
      {
      this->NeedBackgroundBlend = 1;
      }
    }

  // If we have some volumes with ray cast volume mappers, then
  // render them
  if ( this->RayCastVolumeCount )
    {
    // Do any necessary initialization
    this->InitializeRayCasting( ren );

    // Set the number of threads to use for ray casting,
    // then set the execution method and do it.
    this->Threader->SetNumberOfThreads( this->NumberOfThreads );
    this->Threader->SetSingleMethod( RayCast_RenderImage, 
				     (void *)this);
    this->Threader->SingleMethodExecute();

    // Once we've ray casted we now have something in the image
    // so the next thing rendered will not be the first blend.
    this->FirstBlend = 0;
    this->NeedBackgroundBlend = 0;

    // Delete the structures that we created during
    // ray casting.
    for ( i = 0; i < this->RayCastVolumeCount; i++ )
      {
      if ( this->VolumeInfo[i].RowBounds )
	{
	delete this->VolumeInfo[i].RowBounds;
	}
      }

    delete this->RayCastVolumes;
    delete this->VolumeInfo;
    }

  // If we have any volumes with software buffer mappers, render them
  if ( this->SoftwareBufferVolumeCount )
    {
    // For speed - treat the cast where we have no geometry, no ray cast
    // volumes, and only one software buffer volume as a special cast
    if ( this->SoftwareBufferVolumeCount == 1 && this->FirstBlend )
      {
      // We will use the image returned by the mapper, so we don't need
      // the ones we created, and we don't want to delete this other one
      // later
      deleteDisabled = 1;
      delete this->RGBAImage;
      delete this->ZImage;

      // Find that first software buffer volume
      volumes = ren->GetVolumes();
      for ( volumes->InitTraversal(); (aVolume = volumes->GetNextItem()); )
	{
	if( aVolume->GetVisibility() &&
	    aVolume->GetVolumeMapper()->GetMapperType() == 
	    VTK_SOFTWAREBUFFER_VOLUME_MAPPER )
	  {
	  break;
	  }
	}
      
      // Render it and get the resulting image
      aVolume->Render( ren );
      aVolume->GetVolumeMapper()->Render( ren, aVolume );
      this->RGBAImage = 
          aVolume->GetVolumeMapper()->GetRGBAPixelData();
      }
    // Otherwise we have geometry, ray cast volumes, or more than one software
    // buffer volumes
    else
      {     
      // Render each volume of the right type
      volumes = ren->GetVolumes();
      for ( volumes->InitTraversal(); (aVolume = volumes->GetNextItem()); )
	{
	if( aVolume->GetVisibility() &&
	    aVolume->GetVolumeMapper()->GetMapperType() == 
	    VTK_SOFTWAREBUFFER_VOLUME_MAPPER )
	  {
	  // Render the volume and get the resulting image
	  aVolume->Render( ren );
	  aVolume->GetVolumeMapper()->Render( ren, aVolume );
	  nextImage = 
	    aVolume->GetVolumeMapper()->GetRGBAPixelData();

	  // Blend this image with what we have already
	  ptr1 = this->RGBAImage;
	  ptr2 = nextImage;
	  for ( j = 0; j < this->ImageSize[1]; j++ )
	    {
	    for ( i = 0; i < this->ImageSize[0]; i++ )
	      {
	      if ( this->FirstBlend )
		{
		*(ptr1++) = *(ptr2++);
		*(ptr1++) = *(ptr2++);
		*(ptr1++) = *(ptr2++);
		ptr1++;
		ptr2++;
		}
	      else
		{
		alpha = *(ptr2+3);
		*ptr1 = *ptr2 + *ptr1 * alpha;
		ptr1++;
		ptr2++;
		*ptr1 = *ptr2 + *ptr1 * alpha;
		ptr1++;
		ptr2++;
		*ptr1 = *ptr2 + *ptr1 * alpha;
		ptr1+=2;
		ptr2+=2;
		}
	      }
	    }
	  }
	}
      
      }
    }

  // If we still haven't blended the background color into the
  // image yet, then we need to do it here. This could happen
  // if we have one software buffer volume.
  if ( this->NeedBackgroundBlend )
    {
    ptr1 = this->RGBAImage;
    for ( j = 0; j < this->ImageSize[1]; j++ )
      {
      for ( i = 0; i < this->ImageSize[0]; i++ )
	{
	alpha = 1.0 - *(ptr1+3);
	*(ptr1++) += alpha * this->Background[0];
	*(ptr1++) += alpha * this->Background[1];
	*(ptr1++) += alpha * this->Background[2];
	ptr1++;
	}
      }
    }
  
  // If the full image size and the volume rendered image size are not 
  // the same, then we are going to need to rescale the image before
  // writing it into the render window
  if ( this->ImageSize[0] != this->FullImageSize[0] ||
       this->ImageSize[1] != this->FullImageSize[1] )
    {
    // Rescale it.  This also writes it to the render window's 
    // output.
    this->RescaleImage();
    }
  else
    {
    // Place final image into frame buffer if necessary - it is the
    // full resolution size so it doesn't need to be rescaled
    ren->GetRenderWindow()->SetRGBAPixelData( 
	     0, 0, this->ImageSize[0]-1, this->ImageSize[1]-1, 
	     this->RGBAImage, 0 );
    }

  // If we need to delete the image, do so. If we have only one
  // software buffer volume and no geometry then we never created
  // these buffers in the first place
  if ( !deleteDisabled )
    {
    delete this->RGBAImage;
    delete this->ZImage;
    }

  // Stop the timer and record the results
  // If we are rendering a full size image store the results in
  // this->ImageRenderTime[0] otherwise store it in
  // this->ImageRenderTime[1]. This way we can keep track of both
  // how long it takes to do the full size image and how long it
  // took to do the last reduced size image that we rendered.
  timer->StopTimer();
  this->TotalRenderTime = timer->GetElapsedTime();
  if ( this->AutomaticScaleAdjustment )
    {
    if ( this->SelectedImageScaleIndex == 0 )
      {
      this->ImageRenderTime[0] = this->TotalRenderTime;
      }
    else
      {
      this->ImageRenderTime[1] = this->TotalRenderTime;
      }
    }
  timer->Delete();

}

void vtkRayCaster::RescaleImage( )
{

  int *renWinSize;
  float *viewport;
  int window_size[2];
  float *outputFloat;
  vtkRenderer *ren;

  
  // Is the rendering at full resolution?  If so,
  // Then just copy to the output window

  ren = this->Renderer;

  renWinSize = ren->GetRenderWindow()->GetSize();
  viewport = ren->GetViewport();
  window_size[0] =  (int)(renWinSize[0]*(float)(viewport[2] - viewport[0]));
  window_size[1] =  (int)(renWinSize[1]*(float)(viewport[3] - viewport[1]));

  outputFloat = new float[window_size[0]*window_size[1]*4];

  if( this->BilinearImageZoom )
    {
    this->BilinearZoom( this->RGBAImage, outputFloat, this->ImageSize, window_size );
    }
  else
    {
    this->NearestNeighborZoom( this->RGBAImage, outputFloat, this->ImageSize, window_size );
    }

  ren->GetRenderWindow()->SetRGBAPixelData(0,0,
    window_size[0]-1,window_size[1]-1,outputFloat,0);

  delete[] outputFloat;
}

void vtkRayCaster::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os,indent);

  os << indent << "Renderer: " << this->Renderer << "\n";

  os << indent << "Selected Image Scale Index: " << 
     this->SelectedImageScaleIndex << "\n";

  os << indent << "Automatic Scale Adjustment: " << 
     this->AutomaticScaleAdjustment << "\n";

  os << indent << "Automatic Scale Lower Limit: " << 
     this->AutomaticScaleLowerLimit << "\n";

  os << indent << "Bilinear Image Zoom: " << this->BilinearImageZoom << "\n";

  os << indent << "Total Render Time: " << this->TotalRenderTime << "\n";

  os << indent << "Number Of Samples Taken: " << 
     this->GetNumberOfSamplesTaken() << "\n";

  os << indent << "Number Of Threads: " << this->NumberOfThreads << "\n";
}

