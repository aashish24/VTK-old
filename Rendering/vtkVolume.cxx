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

#include "vtkVolume.h"
#include "vtkVolumeCollection.h"
#include "vtkRenderer.h"
#include "vtkRayCaster.h"
#include "vtkVolumeRayCastMapper.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkVolume* vtkVolume::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkVolume");
  if(ret)
    {
    return (vtkVolume*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkVolume;
}




// Creates a Volume with the following defaults: origin(0,0,0) 
// position=(0,0,0) scale=1 visibility=1 pickable=1 dragable=1
// orientation=(0,0,0).
vtkVolume::vtkVolume()
{
  this->Mapper          	    = NULL;
  this->Property                    = NULL;
  this->ScalarOpacityArray          = NULL;
  this->RGBArray                    = NULL;
  this->GrayArray                   = NULL;
  this->CorrectedScalarOpacityArray = NULL;
  this->CorrectedStepSize           = -1;

  this->VolumeInfo         = new VTKRayCastVolumeInfo;
  this->VolumeInfo->Volume = this;

}

// Destruct a volume
vtkVolume::~vtkVolume()
{
  if (this->Property )
    {
    this->Property->UnRegister(this);
    }

  this->SetMapper(NULL);

  if ( this->ScalarOpacityArray )
    {
    delete [] this->ScalarOpacityArray;
    }

  if ( this->RGBArray )
    {
    delete [] this->RGBArray;
    }

  if ( this->GrayArray )
    {
    delete [] this->GrayArray;
    }

  if ( this->CorrectedScalarOpacityArray )
    {
    delete [] this->CorrectedScalarOpacityArray;
    }

  if ( this->VolumeInfo )
    {
    delete this->VolumeInfo;
    }

}

void vtkVolume::GetVolumes(vtkPropCollection *vc)
{
  vc->AddItem(this);
}

// Shallow copy of an volume.
void vtkVolume::ShallowCopy(vtkVolume *volume)
{
  this->vtkProp3D::ShallowCopy(volume);

  this->SetMapper(volume->GetMapper());
  this->SetProperty(volume->GetProperty());
}

void vtkVolume::SetMapper(vtkVolumeMapper *mapper)
{
  if (this->Mapper != mapper)
    {
    if (this->Mapper != NULL) {this->Mapper->UnRegister(this);}
    this->Mapper = mapper;
    if (this->Mapper != NULL) {this->Mapper->Register(this);}
    this->Modified();
    }
}

int vtkVolume::RequiresRayCasting()
{
  int               retval;

  retval = 0;

  if ( this->Mapper )
    {
    retval = this->Mapper->IsARayCastMapper();
    }

  return retval;
}

int vtkVolume::RequiresRenderingIntoImage()
{
  int               retval;

  retval = 0;

  if ( this->Mapper )
    {
    retval = this->Mapper->IsARenderIntoImageMapper();
    }

  return retval;
}


// Get the bounds for this Volume as (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax).
float *vtkVolume::GetBounds()
{
  int i,n;
  float *bounds, bbox[24], *fptr;
  float *result;
  vtkMatrix4x4 *matrix = vtkMatrix4x4::New();
  
  // get the bounds of the Mapper if we have one
  if (!this->Mapper)
    {
    matrix->Delete();
    return this->Bounds;
    }

  bounds = this->Mapper->GetBounds();

  // fill out vertices of a bounding box
  bbox[ 0] = bounds[1]; bbox[ 1] = bounds[3]; bbox[ 2] = bounds[5];
  bbox[ 3] = bounds[1]; bbox[ 4] = bounds[2]; bbox[ 5] = bounds[5];
  bbox[ 6] = bounds[0]; bbox[ 7] = bounds[2]; bbox[ 8] = bounds[5];
  bbox[ 9] = bounds[0]; bbox[10] = bounds[3]; bbox[11] = bounds[5];
  bbox[12] = bounds[1]; bbox[13] = bounds[3]; bbox[14] = bounds[4];
  bbox[15] = bounds[1]; bbox[16] = bounds[2]; bbox[17] = bounds[4];
  bbox[18] = bounds[0]; bbox[19] = bounds[2]; bbox[20] = bounds[4];
  bbox[21] = bounds[0]; bbox[22] = bounds[3]; bbox[23] = bounds[4];
  
  // save the old transform
  this->GetMatrix(matrix);
  this->Transform->Push();
  this->Transform->PostMultiply();
  this->Transform->Identity();
  this->Transform->Concatenate(matrix);

  // and transform into actors coordinates
  fptr = bbox;
  for (n = 0; n < 8; n++) 
    {
    this->Transform->TransformPoint(fptr,fptr);
    fptr += 3;
    }
  
  this->Transform->PreMultiply();
  this->Transform->Pop();  
  
  // now calc the new bounds
  this->Bounds[0] = this->Bounds[2] = this->Bounds[4] = VTK_LARGE_FLOAT;
  this->Bounds[1] = this->Bounds[3] = this->Bounds[5] = -VTK_LARGE_FLOAT;
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
    }

  matrix->Delete();
  return this->Bounds;
}

// Get the minimum X bound
float vtkVolume::GetMinXBound( )
{
  this->GetBounds();
  return this->Bounds[0];
}

// Get the maximum X bound
float vtkVolume::GetMaxXBound( )
{
  this->GetBounds();
  return this->Bounds[1];
}

// Get the minimum Y bound
float vtkVolume::GetMinYBound( )
{
  this->GetBounds();
  return this->Bounds[2];
}

// Get the maximum Y bound
float vtkVolume::GetMaxYBound( )
{
  this->GetBounds();
  return this->Bounds[3];
}

// Get the minimum Z bound
float vtkVolume::GetMinZBound( )
{
  this->GetBounds();
  return this->Bounds[4];
}

// Get the maximum Z bound
float vtkVolume::GetMaxZBound( )
{
  this->GetBounds();
  return this->Bounds[5];
}

int vtkVolume::RenderIntoImage( vtkViewport *vp )
{
  int renderedSomething = 0;

  if ( !this->Mapper )
    {
    vtkErrorMacro( << "You must specify a mapper!\n" );
    return 0;
    }

  // Force the creation of a property
  if( !this->Property )
    {
    this->GetProperty();
    }

  if( !this->Property )
    {
    vtkErrorMacro( << "Error generating a property!\n" );
    return 0;
    }

  if ( this->Mapper->GetMapperType() == VTK_SOFTWAREBUFFER_VOLUME_MAPPER )
    {
    renderedSomething = 1;
    this->Mapper->Render( (vtkRenderer *)vp, this );
    }

  return renderedSomething;
}

float *vtkVolume::GetRGBAImage()
{
  return this->Mapper->GetRGBAPixelData();
}

float *vtkVolume::GetZImage()
{
  return NULL;
}

// If the volume mapper is of type VTK_FRAMEBUFFER_VOLUME_MAPPER, then
// this is its opportunity to render
int vtkVolume::RenderTranslucentGeometry( vtkViewport *vp )
{
  int renderedSomething = 0;

  this->Update();

  if ( !this->Mapper )
    {
    vtkErrorMacro( << "You must specify a mapper!\n" );
    return 0;
    }

  // Force the creation of a property
  if( !this->Property )
    {
    this->GetProperty();
    }

  if( !this->Property )
    {
    vtkErrorMacro( << "Error generating a property!\n" );
    return 0;
    }

  if ( this->Mapper->GetMapperType() == VTK_FRAMEBUFFER_VOLUME_MAPPER )
    {
    renderedSomething = 1;
    this->Mapper->Render( (vtkRenderer *)vp, this );
    this->EstimatedRenderTime += this->Mapper->GetTimeToDraw();
    }

  return renderedSomething;
}

int vtkVolume::InitializeRayCasting( vtkViewport *vp )
{
  float        interactionScale;
  float        sampleDistance;
  vtkRenderer  *ren;
  float        cameraPosition[3], *volumePosition;

  this->Update();

  ren = (vtkRenderer *)vp;

  this->UpdateTransferFunctions( ren );

  interactionScale = ren->GetRayCaster()->GetViewportStepSize();
  sampleDistance =  
    ((vtkVolumeRayCastMapper *)this->Mapper)->
    GetSampleDistance() * interactionScale;
  this->UpdateScalarOpacityforSampleSize( ren, sampleDistance );

  // Get the position of the camera for use in determining the
  // distance to the center of the volume
  ren->GetActiveCamera()->GetPosition( cameraPosition );
  volumePosition = this->GetCenter();
  this->VolumeInfo->CenterDistance = 
    sqrt( (double) 
	  ( ( cameraPosition[0] - volumePosition[0] ) *   
	    ( cameraPosition[0] - volumePosition[0] ) +
	    ( cameraPosition[1] - volumePosition[1] ) *
	    ( cameraPosition[1] - volumePosition[1] ) +
	    ( cameraPosition[2] - volumePosition[2] ) *
	    ( cameraPosition[2] - volumePosition[2] ) ) );


  ((vtkVolumeRayCastMapper *)this->Mapper)->
    InitializeRender( ren, this, this->VolumeInfo );

  return 1;
}

void vtkVolume::InitializeTextureMapping( vtkViewport *vp, 
	float sampleDistance )
{
  vtkRenderer  *ren;

  this->Update();

  ren = (vtkRenderer *)vp;

  this->UpdateTransferFunctions( ren );

  this->UpdateScalarOpacityforSampleSize( ren, sampleDistance );
}

int vtkVolume::CastViewRay( VTKRayCastRayInfo *rayInfo )
{
  ((vtkVolumeRayCastMapper *)this->Mapper)->
    CastViewRay( rayInfo, this->VolumeInfo );

  return 1;
}

void vtkVolume::ReleaseGraphicsResources(vtkWindow *win)
{
  // pass this information onto the mapper
  if (this->Mapper)
    {
    this->Mapper->ReleaseGraphicsResources(win);
    }
}

void vtkVolume::Update()
{
  if ( this->Mapper )
    {
    this->Mapper->Update();
    }
}

void vtkVolume::SetProperty(vtkVolumeProperty *property)
{
  if( this->Property != property )
    {
    if (this->Property != NULL) {this->Property->UnRegister(this);}
    this->Property = property;
    if (this->Property != NULL) 
      {
      this->Property->Register(this);
      this->Property->UpdateMTimes();
      }
    this->Modified();
    }
}

vtkVolumeProperty *vtkVolume::GetProperty()
{
  if( this->Property == NULL )
    {
    this->Property = vtkVolumeProperty::New();
    this->Property->Register(this);
    this->Property->Delete();
    }
  return this->Property;
}

unsigned long int vtkVolume::GetMTime()
{
  unsigned long mTime=this->vtkObject::GetMTime();
  unsigned long time;

  if ( this->Property != NULL )
    {
    time = this->Property->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }

  if ( this->UserMatrix != NULL )
    {
    time = this->UserMatrix->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }

  if ( this->UserTransform != NULL )
    {
    time = this->UserTransform->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }

  return mTime;
}

unsigned long int vtkVolume::GetRedrawMTime()
{
  unsigned long mTime=this->GetMTime();
  unsigned long time;

  if ( this->Mapper != NULL )
    {
    time = this->Mapper->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    if (this->GetMapper()->GetInput() != NULL)
      {
      this->GetMapper()->GetInput()->Update();
      time = this->Mapper->GetInput()->GetMTime();
      mTime = ( time > mTime ? time : mTime );
      }
    }

  if ( this->Property != NULL )
    {
    time = this->Property->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    if ( this->Property->GetGrayTransferFunction() != NULL )
      {
      time = this->Property->GetGrayTransferFunction()->GetMTime();
      mTime = ( time > mTime ? time : mTime );
      }
    if ( this->Property->GetRGBTransferFunction() != NULL )
      {
      time = this->Property->GetRGBTransferFunction()->GetMTime();
      mTime = ( time > mTime ? time : mTime );
      }
    if ( this->Property->GetScalarOpacity() != NULL )
      {
      time = this->Property->GetScalarOpacity()->GetMTime();
      mTime = ( time > mTime ? time : mTime );
      }
    if ( this->Property->GetGradientOpacity() != NULL )
      {
      time = this->Property->GetGradientOpacity()->GetMTime();
      mTime = ( time > mTime ? time : mTime );
      }
    }

  return mTime;
}

void vtkVolume::UpdateTransferFunctions( vtkRenderer *vtkNotUsed(ren) )
{
  int                       data_type;
  vtkPiecewiseFunction      *scalar_opacity_transfer_function;
  vtkPiecewiseFunction      *gradient_opacity_transfer_function;
  vtkPiecewiseFunction      *gray_transfer_function;
  vtkColorTransferFunction  *rgb_transfer_function;
  int                       color_channels;
  int                       scalar_opacity_tf_needs_updating = 0;
  int                       gradient_opacity_tf_needs_updating = 0;
  int                       rgb_tf_needs_updating = 0;
  int                       gray_tf_needs_updating = 0;

  // Update the ScalarOpacityArray if necessary.  This is necessary if
  // the ScalarOpacityArray does not exist, or the transfer function has
  // been modified more recently than the ScalarOpacityArray has.
  scalar_opacity_transfer_function   = this->Property->GetScalarOpacity();
  gradient_opacity_transfer_function = this->Property->GetGradientOpacity();
  rgb_transfer_function              = this->Property->GetRGBTransferFunction();
  gray_transfer_function             = this->Property->GetGrayTransferFunction();
  color_channels                     = this->Property->GetColorChannels();

  if ( ((vtkStructuredPoints *)this->Mapper->GetInput())->
       GetPointData()->GetScalars() == NULL )
    {
    vtkErrorMacro(<<"Need scalar data to volume render");
    return;
    }
    
  data_type = ((vtkStructuredPoints *)this->Mapper->GetInput())->
    GetPointData()->GetScalars()->GetDataType();

  if ( scalar_opacity_transfer_function == NULL )
    {
    vtkErrorMacro( << "Error: no transfer function!" );
    return;
    }
  else if ( this->ScalarOpacityArray == NULL ||
	    scalar_opacity_transfer_function->GetMTime() >
	    this->ScalarOpacityArrayMTime ||
	    this->Property->GetScalarOpacityMTime() >
	    this->ScalarOpacityArrayMTime )
    {
    scalar_opacity_tf_needs_updating = 1;
    }

  if ( gradient_opacity_transfer_function == NULL )
    {
    vtkErrorMacro( << "Error: no gradient magnitude opacity function!" );
    return;
    }
  else if ( gradient_opacity_transfer_function->GetMTime() >
	    this->GradientOpacityArrayMTime ||
	    this->Property->GetGradientOpacityMTime() >
	    this->GradientOpacityArrayMTime )
    {
    gradient_opacity_tf_needs_updating = 1;
    }
  
  switch ( color_channels )
    {
    case 1:
      if ( gray_transfer_function == NULL )
	{
	vtkErrorMacro( << "Error: no gray transfer function!" );
	}
      else if ( this->GrayArray == NULL ||
		gray_transfer_function->GetMTime() >
		this->GrayArrayMTime ||
		this->Property->GetGrayTransferFunctionMTime() >
		this->GrayArrayMTime )
	{
	gray_tf_needs_updating = 1;
	}
      break;
    case 3:
      if ( rgb_transfer_function == NULL )
	{
	vtkErrorMacro( << "Error: no color transfer function!" );
	}
      else if ( this->RGBArray == NULL ||
		rgb_transfer_function->GetMTime() >
		this->RGBArrayMTime ||
		this->Property->GetRGBTransferFunctionMTime() >
		this->RGBArrayMTime )
	{
	rgb_tf_needs_updating = 1;
	}
      break;
    }

  if ( gradient_opacity_tf_needs_updating )
    {
    // Get values 0-255 (256 values)
    gradient_opacity_transfer_function->GetTable(
					  (float)(0x00),
					  (float)(0xff),  
					  (int)(0x100), 
					  this->GradientOpacityArray );
    if ( !strcmp(gradient_opacity_transfer_function->GetType(), "Constant") )
      {
      this->GradientOpacityConstant = this->GradientOpacityArray[128];
      }
    else
      {
      this->GradientOpacityConstant = -1.0;
      }

    this->GradientOpacityArrayMTime.Modified();
    }


  if (data_type == VTK_UNSIGNED_CHAR)
    {
    this->ArraySize = (int)(0x100);

    if ( scalar_opacity_tf_needs_updating )
      {
      // Get values 0-255 (256 values)
      if ( this->ScalarOpacityArray )
	{
	delete [] this->ScalarOpacityArray;
	}

      this->ScalarOpacityArray = new float[(int)(0x100)];
      scalar_opacity_transfer_function->GetTable( (float)(0x00),
					   (float)(0xff),  
					   (int)(0x100), 
					   this->ScalarOpacityArray );
      this->ScalarOpacityArrayMTime.Modified();
      }

    if ( gray_tf_needs_updating )
      {
      if ( this->GrayArray )
	{
	delete [] this->GrayArray;
	}

      this->GrayArray = new float[(int)(0x100)];
      gray_transfer_function->GetTable( (float)(0x00),
					(float)(0xff),  
					(int)(0x100), 
					this->GrayArray );
      this->GrayArrayMTime.Modified();
      }

    if ( rgb_tf_needs_updating )
      {
      if ( this->RGBArray )
	{
	delete [] this->RGBArray;
	}
      
      this->RGBArray = new float[3 * (int)(0x100)];
      rgb_transfer_function->GetTable( (float)(0x00),
				       (float)(0xff),  
				       (int)(0x100), 
				       this->RGBArray );
      this->RGBArrayMTime.Modified();
      }
    }
  else if ( data_type == VTK_UNSIGNED_SHORT )
    {
    this->ArraySize = (int)(0x10000);

    if ( scalar_opacity_tf_needs_updating )
      {
      // Get values 0-65535 (65536 values)
      if ( this->ScalarOpacityArray )
	{
	delete [] this->ScalarOpacityArray;
	}

      this->ScalarOpacityArray = new float[(int)(0x10000)];
      scalar_opacity_transfer_function->GetTable( (float)(0x0000),
					   (float)(0xffff),  
					   (int)(0x10000), 
					   this->ScalarOpacityArray );
      this->ScalarOpacityArrayMTime.Modified();
      }

    if ( gray_tf_needs_updating )
      {
      if ( this->GrayArray )
	{
	delete [] this->GrayArray;
	}

      this->GrayArray = new float[(int)(0x10000)];
      gray_transfer_function->GetTable( (float)(0x0000),
					(float)(0xffff),  
					(int)(0x10000), 
					this->GrayArray );
      this->GrayArrayMTime.Modified();
      }

    if ( rgb_tf_needs_updating )
      {
      if ( this->RGBArray )
	{
	delete [] this->RGBArray;
	}
      
      this->RGBArray = new float[3 * (int)(0x10000)];
      rgb_transfer_function->GetTable( (float)(0x0000),
				       (float)(0xffff),  
				       (int)(0x10000), 
				       this->RGBArray );
      this->RGBArrayMTime.Modified();
      }
    }

  // check that the corrected scalar opacity transfer function
  // is update to date with the current step size.
  // Update CorrectedScalarOpacityArray if it is required.

  if ( scalar_opacity_tf_needs_updating )
    {
    if ( this->CorrectedScalarOpacityArray )
      {
      delete [] this->CorrectedScalarOpacityArray;
      }

    this->CorrectedScalarOpacityArray = new float[this->ArraySize];
    }

}

// This method computes the corrected alpha blending for a given
// step size.  The ScalarOpacityArray reflects step size 1.
// The CorrectedScalarOpacityArray reflects step size CorrectedStepSize.
void vtkVolume::UpdateScalarOpacityforSampleSize( vtkRenderer *vtkNotUsed(ren),
						  float sample_distance )
{
  int i;
  int needsRecomputing;
  float originalAlpha,correctedAlpha;
  float ray_scale;

  ray_scale = sample_distance;

  // step size changed
  needsRecomputing =  
      this->CorrectedStepSize-ray_scale >  0.0001;
  
  needsRecomputing = needsRecomputing || 
      this->CorrectedStepSize-ray_scale < -0.0001;

  if (!needsRecomputing)
    {
    // updated scalar opacity xfer function
    needsRecomputing = needsRecomputing || 
	this->ScalarOpacityArrayMTime > this->CorrectedScalarOpacityArrayMTime;
    }
  if (needsRecomputing)
    {
    this->CorrectedScalarOpacityArrayMTime.Modified();
    this->CorrectedStepSize = ray_scale;

    for (i = 0;i < this->ArraySize;i++)
      {
      originalAlpha = *(this->ScalarOpacityArray+i);

      // this test is to accelerate the Transfer function correction

      if (originalAlpha > 0.0001)
	{
	correctedAlpha = 
	  1.0-pow((double)(1.0-originalAlpha),double(this->CorrectedStepSize));
	}
      else
	{
	correctedAlpha = originalAlpha;
	}
      *(this->CorrectedScalarOpacityArray+i) = correctedAlpha;
      }
    }
}


void vtkVolume::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkProp3D::PrintSelf(os,indent);

  if( this->Property )
    {
    os << indent << "Property:\n";
    this->Property->PrintSelf(os,indent.GetNextIndent());
    }
  else
    {
    os << indent << "Property: (not defined)\n";
    }

  if( this->Mapper )
    {
    os << indent << "Mapper:\n";
    this->Mapper->PrintSelf(os,indent.GetNextIndent());
    }
  else
    {
    os << indent << "Mapper: (not defined)\n";
    }

  // make sure our bounds are up to date
  if ( this->Mapper )
    {
      this->GetBounds();
      os << indent << "Bounds: (" << this->Bounds[0] << ", " 
	 << this->Bounds[1] << ") (" << this->Bounds[2] << ") ("
	 << this->Bounds[3] << ") (" << this->Bounds[4] << ") ("
	 << this->Bounds[5] << ")\n";
    }
  else
    {
    os << indent << "Bounds: (not defined)\n";
    }
}

