/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkFixedPointVolumeRayCastMapper.h"

#include "vtkCamera.h"
#include "vtkColorTransferFunction.h"
#include "vtkEncodedGradientShader.h"
#include "vtkFiniteDifferenceGradientEstimator.h"
#include "vtkImageData.h"
#include "vtkCommand.h"
#include "vtkGraphicsFactory.h"
#include "vtkSphericalDirectionEncoder.h"
#include "vtkFixedPointVolumeRayCastCompositeGOHelper.h"
#include "vtkFixedPointVolumeRayCastCompositeGOShadeHelper.h"
#include "vtkFixedPointVolumeRayCastCompositeHelper.h"
#include "vtkFixedPointVolumeRayCastCompositeShadeHelper.h"
#include "vtkFixedPointVolumeRayCastMIPHelper.h"
#include "vtkMath.h"
#include "vtkMultiThreader.h"
#include "vtkObjectFactory.h"
#include "vtkPiecewiseFunction.h"
#include "vtkPlaneCollection.h"
#include "vtkPointData.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkTimerLog.h"
#include "vtkTransform.h"
#include "vtkVolumeProperty.h"
#include "vtkRayCastImageDisplayHelper.h"

#include <math.h>

vtkCxxRevisionMacro(vtkFixedPointVolumeRayCastMapper, "$Revision$");
vtkStandardNewMacro(vtkFixedPointVolumeRayCastMapper); 

// Macro for tri-linear interpolation - do four linear interpolations on
// edges, two linear interpolations between pairs of edges, then a final
// interpolation between faces
#define vtkTrilinFuncMacro(v,x,y,z,a,b,c,d,e,f,g,h)         \
        t00 =   a + (x)*(b-a);      \
        t01 =   c + (x)*(d-c);      \
        t10 =   e + (x)*(f-e);      \
        t11 =   g + (x)*(h-g);      \
        t0  = t00 + (y)*(t01-t00);  \
        t1  = t10 + (y)*(t11-t10);  \
        v   =  t0 + (z)*(t1-t0);


#define vtkVRCMultiplyPointMacro( A, B, M ) \
  B[0] = A[0]*M[0]  + A[1]*M[1]  + A[2]*M[2]  + M[3]; \
  B[1] = A[0]*M[4]  + A[1]*M[5]  + A[2]*M[6]  + M[7]; \
  B[2] = A[0]*M[8]  + A[1]*M[9]  + A[2]*M[10] + M[11]; \
  B[3] = A[0]*M[12] + A[1]*M[13] + A[2]*M[14] + M[15]; \
  if ( B[3] != 1.0 ) { B[0] /= B[3]; B[1] /= B[3]; B[2] /= B[3]; }

#define vtkVRCMultiplyViewPointMacro( A, B, M ) \
  B[0] = A[0]*M[0]  + A[1]*M[1]  + A[2]*M[2]  + M[3]; \
  B[1] = A[0]*M[4]  + A[1]*M[5]  + A[2]*M[6]  + M[7]; \
  B[3] = A[0]*M[12] + A[1]*M[13] + A[2]*M[14] + M[15]; \
  if ( B[3] != 1.0 ) { B[0] /= B[3]; B[1] /= B[3]; }

#define vtkVRCMultiplyNormalMacro( A, B, M ) \
  B[0] = A[0]*M[0]  + A[1]*M[4]  + A[2]*M[8]; \
  B[1] = A[0]*M[1]  + A[1]*M[5]  + A[2]*M[9]; \
  B[2] = A[0]*M[2]  + A[1]*M[6]  + A[2]*M[10]


template <class T>
void vtkFixedPointVolumeRayCastMapperFillInMinMaxVolume( T *dataPtr, unsigned short *minMaxVolume,
                                                         int fullDim[3], int smallDim[4],
                                                         int independent, int components,
                                                         float *shift, float *scale )
{
  int i, j, k, c;
  int sx1, sx2, sy1, sy2, sz1, sz2;
  int x, y, z;
  
  T *dptr = dataPtr;
  
  for ( k = 0; k < fullDim[2]; k++ )
    {
    sz1 = (k < 1)?(0):(static_cast<int>((k-1)/4));
    sz2 =              static_cast<int>((k  )/4);
    for ( j = 0; j < fullDim[1]; j++ )
      {      
      sy1 = (j < 1)?(0):(static_cast<int>((j-1)/4));
      sy2 =              static_cast<int>((j  )/4);
      for ( i = 0; i < fullDim[0]; i++ )
        {
        sx1 = (i < 1)?(0):(static_cast<int>((i-1)/4));
        sx2 =              static_cast<int>((i  )/4);
        
        for ( c = 0; c < smallDim[3]; c++ )
          {
          unsigned short val;
          if ( independent )
            {
            val = static_cast<unsigned short>((*dptr + shift[c]) * scale[c]);
            dptr++;
            }
          else
            {
            val = static_cast<unsigned short>((*(dptr+components-1) + 
                   shift[components-1]) * scale[components-1]);
            dptr += components;
            }
          
          for ( z = sz1; z <= sz2; z++ )
            {
            for ( y = sy1; y <= sy2; y++ )
              {
              for ( x = sx1; x <= sx2; x++ )
                {
                unsigned short *tmpPtr = minMaxVolume +
                  3*( z*smallDim[0]*smallDim[1]*smallDim[3] +
                      y*smallDim[0]*smallDim[3] +
                      x*smallDim[3] + c);
                
                tmpPtr[0] = (val<tmpPtr[0])?(val):(tmpPtr[0]);
                tmpPtr[1] = (val>tmpPtr[1])?(val):(tmpPtr[1]);
                }
              }
            }
          } 
        }
      }
    }
}

template <class T>
void vtkFixedPointVolumeRayCastMapperComputeGradients( T *dataPtr,
                                               int dim[3], 
                                               double spacing[3],
                                               int components,
                                               int independent,
                                               double scalarRange[4][2],
                                               unsigned short **gradientNormal,
                                               unsigned char  **gradientMagnitude,
                                               vtkDirectionEncoder *directionEncoder,
                                               vtkFixedPointVolumeRayCastMapper *me )
{
  int                 x, y, z, c;
  int                 x_start, x_limit;
  int                 y_start, y_limit;
  int                 z_start, z_limit;
  T                   *dptr, *cdptr;
  float               n[3], t;
  float               gvalue=0;
  int                 xlow, xhigh;
  double              aspect[3];
  int                 xstep, ystep, zstep;
  float               scale[4];
  unsigned short      *dirPtr, *cdirPtr;
  unsigned char       *magPtr, *cmagPtr;
  

  me->InvokeEvent( vtkCommand::StartEvent, NULL );

  double avgSpacing = (spacing[0]+spacing[1]+spacing[2])/3.0;
  
  // adjust the aspect
  aspect[0] = spacing[0] * 2.0 / avgSpacing;
  aspect[1] = spacing[1] * 2.0 / avgSpacing;
  aspect[2] = spacing[2] * 2.0 / avgSpacing;
  
  // Compute steps through the volume in x, y, and z
  xstep = components;
  ystep = components*dim[0];
  zstep = components*dim[0] * dim[1];

  if ( !independent )
    {
    scale[0] = 255.0 / (0.25*(scalarRange[components-1][1] - scalarRange[components-1][0]));
    }
  else
    {
    for (c = 0; c < components; c++ )
      {
      scale[c] = 255.0 / (0.25*(scalarRange[c][1] - scalarRange[c][0]));
      }
    }
  
  
  int thread_id = 0;
  int thread_count = 1;
  
  x_start = 0;
  x_limit = dim[0];
  y_start = 0;
  y_limit = dim[1];
  z_start = (int)(( (float)thread_id / (float)thread_count ) *
                  dim[2] );
  z_limit = (int)(( (float)(thread_id + 1) / (float)thread_count ) *
                  dim[2] );

  // Do final error checking on limits - make sure they are all within bounds
  // of the scalar input
  
  x_start = (x_start<0)?(0):(x_start);
  y_start = (y_start<0)?(0):(y_start);
  z_start = (z_start<0)?(0):(z_start);
  
  x_limit = (x_limit>dim[0])?(dim[0]):(x_limit);
  y_limit = (y_limit>dim[1])?(dim[1]):(y_limit);
  z_limit = (z_limit>dim[2])?(dim[2]):(z_limit);


  int increment = (independent)?(components):(1);

  float tolerance[4];
  for ( c = 0; c < components; c++ )
    {
    tolerance[c] = .00001 * (scalarRange[c][1] - scalarRange[c][0]);
    }
  
  // Loop through all the data and compute the encoded normal and
  // gradient magnitude for each scalar location
  for ( z = z_start; z < z_limit; z++ )
    {
    unsigned short *gradientDirPtr = gradientNormal[z];
    unsigned char *gradientMagPtr = gradientMagnitude[z];
  
    for ( y = y_start; y < y_limit; y++ )
      {
      xlow = x_start;
      xhigh = x_limit;
      
      dptr = dataPtr + components*(z * dim[0] * dim[1] + y * dim[0] + xlow);
      
      dirPtr  = gradientDirPtr    + (y * dim[0] + xlow)*increment; 
      magPtr  = gradientMagPtr    + (y * dim[0] + xlow)*increment; 
      
      for ( x = xlow; x < xhigh; x++ )
        {
        for ( c = 0; ( independent && c < components ) || c == 0; c++ )
          {
          cdptr   = dptr   + ((independent)?(c):(components-1));
          cdirPtr = dirPtr + ((independent)?(c):(0));
          cmagPtr = magPtr + ((independent)?(c):(0));

          // Allow up to 3 tries to find the gadient - looking out at a distance of
          // 1, 2, and 3 units.
          int foundGradient = 0;
          for ( int d = 1; d <= 3 && !foundGradient; d++ )
            {
            // Use a central difference method if possible,
            // otherwise use a forward or backward difference if
            // we are on the edge
            // Compute the X component
            if ( x < 2 ) 
              {
              n[0] = 2.0*((float)*(cdptr) - (float)*(cdptr+d*xstep));
              }
            else if ( x >= dim[0] - d )
              {
              n[0] = 2.0*((float)*(cdptr-d*xstep) - (float)*(cdptr));
              }
            else
              {
            n[0] = (float)*(cdptr-d*xstep) - (float)*(cdptr+d*xstep); 
              }
            
            // Compute the Y component
            if ( y < d )
              {
              n[1] = 2.0*((float)*(cdptr) - (float)*(cdptr+d*ystep)); 
              }
            else if ( y >= dim[1] - d )
              {
              n[1] = 2.0*((float)*(cdptr-d*ystep) - (float)*(cdptr)); 
              }
            else
              {
              n[1] = (float)*(cdptr-d*ystep) - (float)*(cdptr+d*ystep); 
              }
            
            // Compute the Z component
            if ( z < d )
              {
              n[2] = 2.0*((float)*(cdptr) - (float)*(cdptr+d*zstep)); 
              }
            else if ( z >= dim[2] - d )
              {
              n[2] = 2.0*((float)*(cdptr-d*zstep) - (float)*(cdptr)); 
              }
            else
              {
              n[2] = (float)*(cdptr-d*zstep) - (float)*(cdptr+d*zstep); 
              }
            
            // Take care of the aspect ratio of the data
            // Scaling in the vtkVolume is isotropic, so this is the
            // only place we have to worry about non-isotropic scaling.
            n[0] /= d*aspect[0];
            n[1] /= d*aspect[1];
            n[2] /= d*aspect[2];
            
            // Compute the gradient magnitude
            t = sqrt( (double)( n[0]*n[0] + 
                                n[1]*n[1] + 
                                n[2]*n[2] ) );
            
            
            // Encode this into an 8 bit value 
            gvalue = t * scale[c]; 

            if ( d > 1 )
              {
              gvalue = 0;
              }
            
            gvalue = (gvalue<0.0)?(0.0):(gvalue);
            gvalue = (gvalue>255.0)?(255.0):(gvalue);
            
            // Normalize the gradient direction
            if ( t > tolerance[c] )
              {
              n[0] /= t;
              n[1] /= t;
              n[2] /= t;
              foundGradient = 1;
              }
            else
              {
              n[0] = n[1] = n[2] = 0.0;
            }
          }
        
          
          *cmagPtr = static_cast<unsigned char>(gvalue + 0.5);
          *cdirPtr = directionEncoder->GetEncodedDirection( n );
          }
        
        dptr    +=   components;
        dirPtr  +=   increment;
        magPtr  +=   increment;
        }
      }
    if ( z%8 == 7 )
      {
      float args[1];
      args[0] = 
        static_cast<float>(z - z_start) / 
        static_cast<float>(z_limit - z_start - 1);
      me->InvokeEvent( vtkCommand::ProgressEvent, args );
      }
    }
  
  me->InvokeEvent( vtkCommand::EndEvent, NULL );  
}

// Construct a new vtkFixedPointVolumeRayCastMapper with default values
vtkFixedPointVolumeRayCastMapper::vtkFixedPointVolumeRayCastMapper()
{
  this->SampleDistance             =  1.0;
  this->ImageSampleDistance        =  1.0;
  this->MinimumImageSampleDistance =  1.0;
  this->MaximumImageSampleDistance = 10.0;
  this->AutoAdjustSampleDistances  =  1;
  
  this->PerspectiveMatrix      = vtkMatrix4x4::New();
  this->ViewToWorldMatrix      = vtkMatrix4x4::New();
  this->ViewToVoxelsMatrix     = vtkMatrix4x4::New();
  this->VoxelsToViewMatrix     = vtkMatrix4x4::New();
  this->WorldToVoxelsMatrix    = vtkMatrix4x4::New();
  this->VoxelsToWorldMatrix    = vtkMatrix4x4::New();

  this->VolumeMatrix           = vtkMatrix4x4::New();
  
  this->PerspectiveTransform   = vtkTransform::New();
  this->VoxelsTransform        = vtkTransform::New();
  this->VoxelsToViewTransform  = vtkTransform::New();
  
  this->ImageViewportSize[0] = this->ImageViewportSize[1] = 0;
  this->ImageMemorySize[0]   = this->ImageMemorySize[1]   = 0;
  this->ImageInUseSize[0]    = this->ImageInUseSize[1]    = 0;

  this->Threader               = vtkMultiThreader::New();
  this->NumberOfThreads        = this->Threader->GetNumberOfThreads();
  
  this->Image                  = NULL;
  this->RowBounds              = NULL;
  this->OldRowBounds           = NULL;
  
  this->RenderTimeTable        = NULL;
  this->RenderVolumeTable      = NULL;
  this->RenderRendererTable    = NULL;
  this->RenderTableSize        = 0;  
  this->RenderTableEntries     = 0;

  this->ZBuffer                = NULL;
  this->ZBufferSize[0]         = 0;
  this->ZBufferSize[1]         = 0;
  this->ZBufferOrigin[0]       = 0;
  this->ZBufferOrigin[1]       = 0;
  
  this->RenderWindow           = NULL;
  
  this->MIPHelper              = vtkFixedPointVolumeRayCastMIPHelper::New();
  this->CompositeHelper        = vtkFixedPointVolumeRayCastCompositeHelper::New();
  this->CompositeGOHelper      = vtkFixedPointVolumeRayCastCompositeGOHelper::New();
  this->CompositeShadeHelper   = vtkFixedPointVolumeRayCastCompositeShadeHelper::New();
  this->CompositeGOShadeHelper = vtkFixedPointVolumeRayCastCompositeGOShadeHelper::New();
  
  this->IntermixIntersectingGeometry = 1;
  
  int i;
  for ( i = 0; i < 4; i++ )
    {
    this->SavedRGBFunction[i]             = NULL;
    this->SavedGrayFunction[i]            = NULL;
    this->SavedScalarOpacityFunction[i]   = NULL;
    this->SavedGradientOpacityFunction[i] = NULL;
    this->SavedColorChannels[i]           = 0;
    this->SavedScalarOpacityDistance[i]   = 0;
    }
  
  this->SavedSampleDistance          = 0;
  this->SavedBlendMode               = -1;
  
  this->SavedGradientsInput          = NULL;
  this->SavedParametersInput         = NULL;

  this->NumberOfGradientSlices       = 0;
  this->GradientNormal               = NULL;
  this->GradientMagnitude            = NULL;
  this->ContiguousGradientNormal     = NULL;
  this->ContiguousGradientMagnitude  = NULL;
  
  this->DirectionEncoder             = vtkSphericalDirectionEncoder::New();
  this->GradientShader               = vtkEncodedGradientShader::New();
  this->GradientEstimator            = vtkFiniteDifferenceGradientEstimator::New();
  
  this->GradientEstimator->SetDirectionEncoder( this->DirectionEncoder );
  
  this->ShadingRequired              = 0;
  this->GradientOpacityRequired      = 0;
  
  this->CroppingRegionMask[0] = 1;
  for ( i = 1; i < 27; i++ )
    {
    this->CroppingRegionMask[i] = this->CroppingRegionMask[i-1]*2;
    }
  
  this->NumTransformedClippingPlanes = 0;
  this->TransformedClippingPlanes    = NULL;

  this->ImageDisplayHelper  = vtkRayCastImageDisplayHelper::New();
  this->ImageDisplayHelper->PreMultipliedColorsOn();
  
  // This is the min max volume used for space leaping. Each 4x4x4 cell from
  // the original input volume has three values per component - a minimum scalar 
  // index, maximum scalar index, and a values used for both the maximum gradient
  // magnitude and a flag. The flag is used to indicate for the
  // current transfer function whether any non-zero opacity exists between the
  // minimum and maximum scalar values and up to the maximum gradient magnitude
  this->MinMaxVolume = NULL;
  this->MinMaxVolumeSize[0] = 0;
  this->MinMaxVolumeSize[1] = 0;
  this->MinMaxVolumeSize[2] = 0;
  this->MinMaxVolumeSize[3] = 0;
  this->SavedMinMaxInput = NULL;
  
  this->Volume = NULL;
  
  
}

// Destruct a vtkFixedPointVolumeRayCastMapper - clean up any memory used
vtkFixedPointVolumeRayCastMapper::~vtkFixedPointVolumeRayCastMapper()
{
  this->PerspectiveMatrix->Delete();
  this->ViewToWorldMatrix->Delete();
  this->ViewToVoxelsMatrix->Delete();
  this->VoxelsToViewMatrix->Delete();
  this->WorldToVoxelsMatrix->Delete();
  this->VoxelsToWorldMatrix->Delete();

  this->VolumeMatrix->Delete();
  
  this->VoxelsTransform->Delete();
  this->VoxelsToViewTransform->Delete();
  this->PerspectiveTransform->Delete();
  
  this->Threader->Delete();
  
  this->MIPHelper->Delete();
  this->CompositeHelper->Delete();
  this->CompositeGOHelper->Delete();
  this->CompositeShadeHelper->Delete();
  this->CompositeGOShadeHelper->Delete();
  
  delete [] this->Image;
    
  delete [] this->RenderTimeTable;
  delete [] this->RenderVolumeTable;
  delete [] this->RenderRendererTable;
  
  delete [] this->RowBounds;
  delete [] this->OldRowBounds;

  int i;
  if ( this->GradientNormal )
    {
    // Contiguous? Delete in one chunk otherwise delete slice by slice
    if ( this->ContiguousGradientNormal )
      {
      delete [] this->ContiguousGradientNormal;
      this->ContiguousGradientNormal = NULL;
      }
    else
      {
      for ( i = 0; i < this->NumberOfGradientSlices; i++ )
        {
        delete [] this->GradientNormal[i];
        }
      }
    delete [] this->GradientNormal;
    this->GradientNormal = NULL;
    }
 
  if ( this->GradientMagnitude )
    {
    // Contiguous? Delete in one chunk otherwise delete slice by slice
    if ( this->ContiguousGradientMagnitude )
      {
      delete [] this->ContiguousGradientMagnitude;
      this->ContiguousGradientMagnitude = NULL;
      }
    else
      {
      for ( i = 0; i < this->NumberOfGradientSlices; i++ )
        {
        delete [] this->GradientMagnitude[i];
        }
      }
    delete [] this->GradientMagnitude;
    this->GradientMagnitude = NULL;
    }
  
  this->DirectionEncoder->Delete();
  this->GradientShader->Delete();
  this->GradientEstimator->Delete();
  
  delete [] this->TransformedClippingPlanes;

  this->ImageDisplayHelper->Delete();
  
  // Delete storage used by min/max volume
  delete [] this->MinMaxVolume;
}

float vtkFixedPointVolumeRayCastMapper::ComputeRequiredImageSampleDistance( float desiredTime,
                                                                    vtkRenderer *ren )
{
  return this->ComputeRequiredImageSampleDistance( desiredTime, ren, NULL );
}

float vtkFixedPointVolumeRayCastMapper::ComputeRequiredImageSampleDistance( float desiredTime,
                                                                    vtkRenderer *ren,
                                                                    vtkVolume *vol )
{
  float result;
  
  float oldTime;
  
  if ( vol )
    {
    oldTime = this->RetrieveRenderTime( ren, vol );
    }
  else
    {
    oldTime = this->RetrieveRenderTime( ren );
    }
  
  float newTime = desiredTime;
  
  if ( oldTime == 0.0 )
    {
    if ( newTime > 10 )
      {
      result = this->MinimumImageSampleDistance;
      }
    else
      {
      result = this->MaximumImageSampleDistance / 2.0;
      }
    }
  else
    {
    oldTime /= (this->ImageSampleDistance * this->ImageSampleDistance);
    result = this->ImageSampleDistance * sqrt(oldTime / newTime);
    result = (result > this->MaximumImageSampleDistance)?
      (this->MaximumImageSampleDistance):(result);
    result = 
      (result<this->MinimumImageSampleDistance)?
      (this->MinimumImageSampleDistance):(result);
    }
  
  return result;
}

float vtkFixedPointVolumeRayCastMapper::RetrieveRenderTime( vtkRenderer *ren, 
                                                    vtkVolume   *vol )
{
  int i;
  
  for ( i = 0; i < this->RenderTableEntries; i++ )
    {
    if ( this->RenderVolumeTable[i] == vol &&
         this->RenderRendererTable[i] == ren )
      {
      return this->RenderTimeTable[i];
      }
    }
  
  return 0.0;
}

float vtkFixedPointVolumeRayCastMapper::RetrieveRenderTime( vtkRenderer *ren ) 
{
  int i;
  
  for ( i = 0; i < this->RenderTableEntries; i++ )
    {
    if ( this->RenderRendererTable[i] == ren )
      {
      return this->RenderTimeTable[i];
      }
    }
  
  return 0.0;
}

void vtkFixedPointVolumeRayCastMapper::StoreRenderTime( vtkRenderer *ren, 
                                                vtkVolume   *vol, 
                                                float       time )
{
  int i;
  for ( i = 0; i < this->RenderTableEntries; i++ )
    {
    if ( this->RenderVolumeTable[i] == vol &&
         this->RenderRendererTable[i] == ren )
      {
      this->RenderTimeTable[i] = time;
      return;
      }
    }
  
  
  // Need to increase size
  if ( this->RenderTableEntries >= this->RenderTableSize )
    {
    if ( this->RenderTableSize == 0 )
      {
      this->RenderTableSize = 10;
      }
    else
      {
      this->RenderTableSize *= 2;
      }
    
    float        *oldTimePtr     = this->RenderTimeTable;
    vtkVolume   **oldVolumePtr   = this->RenderVolumeTable;
    vtkRenderer **oldRendererPtr = this->RenderRendererTable;
    
    this->RenderTimeTable     = new float [this->RenderTableSize];
    this->RenderVolumeTable   = new vtkVolume *[this->RenderTableSize];
    this->RenderRendererTable = new vtkRenderer *[this->RenderTableSize];
    
    for (i = 0; i < this->RenderTableEntries; i++ )
      {
      this->RenderTimeTable[i] = oldTimePtr[i];
      this->RenderVolumeTable[i] = oldVolumePtr[i];
      this->RenderRendererTable[i] = oldRendererPtr[i];
      }
    
    delete [] oldTimePtr;
    delete [] oldVolumePtr;
    delete [] oldRendererPtr;
    }
  
  this->RenderTimeTable[this->RenderTableEntries] = time;
  this->RenderVolumeTable[this->RenderTableEntries] = vol;
  this->RenderRendererTable[this->RenderTableEntries] = ren;
  
  this->RenderTableEntries++;
}

void vtkFixedPointVolumeRayCastMapper::SetNumberOfThreads( int num )
{
  this->Threader->SetNumberOfThreads( num );
}

void vtkFixedPointVolumeRayCastMapper::FillInMaxGradientMagnitudes( int fullDim[3],
                                                                    int smallDim[4] )
{
  int i, j, k, c;
  int sx1, sx2, sy1, sy2, sz1, sz2;
  int x, y, z;
  
  
  for ( k = 0; k < fullDim[2]; k++ )
    {
    sz1 = (k < 1)?(0):(static_cast<int>((k-1)/4));
    sz2 =              static_cast<int>((k  )/4);
    
    unsigned char *dptr = this->GradientMagnitude[k];
    
    for ( j = 0; j < fullDim[1]; j++ )
      {      
      sy1 = (j < 1)?(0):(static_cast<int>((j-1)/4));
      sy2 =              static_cast<int>((j  )/4);
      for ( i = 0; i < fullDim[0]; i++ )
        {
        sx1 = (i < 1)?(0):(static_cast<int>((i-1)/4));
        sx2 =              static_cast<int>((i  )/4);
        
        for ( c = 0; c < smallDim[3]; c++ )
          {
          unsigned char val;
          val = *dptr;
          dptr++;
          
          for ( z = sz1; z <= sz2; z++ )
            {
            for ( y = sy1; y <= sy2; y++ )
              {
              for ( x = sx1; x <= sx2; x++ )
                {
                unsigned short *tmpPtr = this->MinMaxVolume +
                  3*( z*smallDim[0]*smallDim[1]*smallDim[3] +
                      y*smallDim[0]*smallDim[3] +
                      x*smallDim[3] + c);

                // Need to keep track of max gradient magnitude in upper
                // eight bits. No need to preserve lower eight (the flag)
                // since we will be recomputing this.
                tmpPtr[2] = (val>(tmpPtr[2]>>8))?(val<<8):(tmpPtr[2]);
                }
              }
            }
          } 
        }
      }
    }
}

// This method should be called after UpdateColorTables since it
// relies on some information (shift and scale) computed in that method,
// as well as the last built time for the color tables.
void vtkFixedPointVolumeRayCastMapper::UpdateMinMaxVolume( vtkVolume *vol )
{
  int i, j, k, c;
  
  // A three bit variable:
  //   first bit indicates need to update flags
  //   second bit indicates need to update scalars
  //   third bit indicates need to update gradient magnitudes
  int needToUpdate = 0;
  
  // Get the image data
  vtkImageData *input = this->GetInput();

  // We'll need this info later
  int components   = input->GetNumberOfScalarComponents();
  int independent  = vol->GetProperty()->GetIndependentComponents();
  int dim[3];    
  input->GetDimensions( dim );  

  // Has the data itself changed?
  if ( input != this->SavedMinMaxInput ||
       input->GetMTime() > this->SavedMinMaxBuildTime.GetMTime() )
    {
    needToUpdate |= 0x03;
    }
  
  // Do the gradient magnitudes need to be filled in?
  if ( this->GradientOpacityRequired &&
       ( needToUpdate&0x02 || 
         this->SavedGradientsMTime.GetMTime() > 
         this->SavedMinMaxBuildTime.GetMTime() ) )
    {
    needToUpdate |= 0x05;
    }
  
  // Have the parameters changed which means the flags need
  // to be recomputed. Actually, we could be checking just
  // a subset of these parameters (we don't need to recompute
  // the flags if the colors change, but unless these seems
  // like a significant performance problem, I'd rather not
  // complicate the code)
  if ( !(needToUpdate&0x01) &&
       this->SavedParametersMTime.GetMTime() >
       this->SavedMinMaxFlagTime.GetMTime() )
    {
    needToUpdate |= 0x01;
    }
  

  if ( !needToUpdate )
    {
    return;
    }
  
  // Regenerate the min max values if necessary
  if ( needToUpdate&0x02 )
    {
    // How big should the min/max volume be?
    int targetSize[4];
    
    for ( i = 0; i < 3; i++ )
      {
      // We group four cells (which require 5 samples) into one element in the min/max tree
      targetSize[i] =
        (dim[i] < 2) ? (1) : ( 1 + static_cast<int>((dim[i] - 2)/4));
      }

    // This fourth dimension is the number of independent components for which we
    // need to keep track of min/max
    targetSize[3] = (independent)?(components):(1);
    
    if ( this->MinMaxVolumeSize[0] != targetSize[0] ||
         this->MinMaxVolumeSize[1] != targetSize[1] ||
         this->MinMaxVolumeSize[2] != targetSize[2] ||
         this->MinMaxVolumeSize[3] != targetSize[3] )
      {
      delete [] this->MinMaxVolume;
      
      // One entry for min, one for max, one shared by max gradient 
      // magnitude, and a flag set based on opacity transfer functions
      this->MinMaxVolume = new unsigned short [3 * ( targetSize[0] *
                                                     targetSize[1] *
                                                     targetSize[2] *
                                                     targetSize[3] ) ];
      
      // Don't really do anything about it - but reporting this error may
      // save some debugging time later...
      if ( !this->MinMaxVolume )
        {
        vtkErrorMacro( "Problem allocating min/max volume" );
        this->MinMaxVolumeSize[0] = 0;
        this->MinMaxVolumeSize[1] = 0;
        this->MinMaxVolumeSize[2] = 0;
        this->MinMaxVolumeSize[3] = 0;
        return;
        }
      
      this->MinMaxVolumeSize[0] = targetSize[0];
      this->MinMaxVolumeSize[1] = targetSize[1];
      this->MinMaxVolumeSize[2] = targetSize[2];
      this->MinMaxVolumeSize[3] = targetSize[3];
      
      // Initialize the structure
      unsigned short *tmpPtr = this->MinMaxVolume;
      for ( i = 0; i < targetSize[0] * targetSize[1] * targetSize[2]; i++ )
        {
        for ( j = 0; j < targetSize[3]; j++ )
          {
          *(tmpPtr++) = 0xffff;  // Min Scalar
          *(tmpPtr++) = 0;       // Max Scalar
          *(tmpPtr++) = 0;       // Max Gradient Magnitude and
          }                      // Flag computed from transfer functions
        }
      
      // Now put the scalar data values into the structure
      int scalarType   = input->GetScalarType();
      void *dataPtr = input->GetScalarPointer();
      switch ( scalarType )
        {
        vtkTemplateMacro8( vtkFixedPointVolumeRayCastMapperFillInMinMaxVolume,
                           (VTK_TT *)(dataPtr), this->MinMaxVolume, dim, targetSize, 
                           independent, components, this->TableShift, this->TableScale );
        }
      }
    
    this->SavedMinMaxInput = input;
    this->SavedMinMaxBuildTime.Modified();
    }
  
  if ( needToUpdate&0x04 )
    {
    // Now put the gradient magnitude values into the structure
    this->FillInMaxGradientMagnitudes( dim, this->MinMaxVolumeSize );
    
    // It is OK to use this same variable for scalars and gradient magnitudes - either
    // we just rebuilt the min max volume from the scalars, or the MTime on the input
    // is already less than this build time so updating it again won't matter for
    // future checks
    this->SavedMinMaxBuildTime.Modified();
    }
  
  // Update the flags now
  unsigned short *minNonZeroScalarIndex = new unsigned short [this->MinMaxVolumeSize[3]];
  for ( c = 0; c < this->MinMaxVolumeSize[3]; c++ )
    {
    for ( i = 0; i < this->TableSize[c]; i++ )
      {
      if ( this->ScalarOpacityTable[c][i] )
        {
        break;
        }
      }
    minNonZeroScalarIndex[c] = i;
    }
  
  unsigned char *minNonZeroGradientMagnitudeIndex = new unsigned char [this->MinMaxVolumeSize[3]];
  for ( c = 0; c < this->MinMaxVolumeSize[3]; c++ )
    {
    for ( i = 0; i < 256; i++ )
      {
      if ( this->GradientOpacityTable[c][i] )
        {
        break;
        }
      }
    minNonZeroGradientMagnitudeIndex[c] = i;
    }

  unsigned short *tmpPtr = this->MinMaxVolume;  
  int zero = 0;
  int nonZero = 0;
  
  for ( k = 0; k < this->MinMaxVolumeSize[2]; k++ )
    {
    for ( j = 0; j < this->MinMaxVolumeSize[1]; j++ )
      {
      for ( i = 0; i < this->MinMaxVolumeSize[0]; i++ )
        {
        for ( c = 0; c < this->MinMaxVolumeSize[3]; c++ )
          {
          // We definite have 0 opacity because our maximum scalar value in
          // this region is below the minimum scalar value with non-zero opacity
          // for this component
          if ( tmpPtr[1] < minNonZeroScalarIndex[c] )
            {
            tmpPtr[2] &= 0xff00;
            zero++;
            }
          // We have 0 opacity because we are using gradient magnitudes and
          // the maximum gradient magnitude in this area is below the minimum
          // gradient magnitude with non-zero opacity for this component
          else if ( this->GradientOpacityRequired &&
                    (tmpPtr[2]>>8) < minNonZeroGradientMagnitudeIndex[c] )
            {
            tmpPtr[2] &= 0xff00;
            zero++;
            }
          // We definitely have non-zero opacity because our minimum scalar
          // value is lower than our first scalar with non-zero opacity, and
          // the maximum scalar value is greater than this threshold - so
          // we must encounter scalars with opacity in between
          else if ( tmpPtr[0] < minNonZeroScalarIndex[c] )
            {
            tmpPtr[2] &= 0xff00;
            tmpPtr[2] |= 0x0001;
            nonZero++;
            }
          // We have to search between min scalar value and the
          // max scalar stored in the minmax volume to look for non-zero
          // opacity since both values must be above our first non-zero
          // threshold so we don't have information in this area
          else
            {
            int loop;
            for ( loop = tmpPtr[0]; loop <= tmpPtr[1]; loop++ )
              {
              if ( this->ScalarOpacityTable[c][loop] )
                {
                break;
                }
              }
            if ( loop <= tmpPtr[1] )
              {
              tmpPtr[2] &= 0xff00;
              tmpPtr[2] |= 0x0001;
              nonZero++;
              }
            else
              {
              tmpPtr[2] &= 0xff00;
              zero++;
              }
            }
          tmpPtr += 3;
          }
        }      
      }
    }
  
  this->SavedMinMaxFlagTime.Modified();
  
}

void vtkFixedPointVolumeRayCastMapper::UpdateCroppingRegions()
{
  int i;
  
  for ( i = 0; i < 6; i++ )
    {
    this->FixedPointCroppingRegionPlanes[i] = 
      this->ToFixedPointPosition( this->VoxelCroppingRegionPlanes[i] );
    }
  
}

void vtkFixedPointVolumeRayCastMapper::Render( vtkRenderer *ren, vtkVolume *vol )
{
  // make sure that we have scalar input and update the scalar input
  if ( this->GetInput() == NULL ) 
    {
    vtkErrorMacro(<< "No Input!");
    return;
    }
  else
    {
    this->GetInput()->UpdateInformation();
    this->GetInput()->SetUpdateExtentToWholeExtent();
    this->GetInput()->Update();
    } 

  // Start timing now. We didn't want to capture the update of the
  // input data in the times
  this->Timer->StartTimer();
  
  // This is the input of this mapper
  vtkImageData *input = this->GetInput();
  
  this->ConvertCroppingRegionPlanesToVoxels();
  
  // Compute some matrices from voxels to view and vice versa based 
  // on the whole input
  this->ComputeMatrices( input, ren, vol );
  
  // How big is the viewport in pixels?
  double *viewport   =  ren->GetViewport();
  int *renWinSize   =  ren->GetRenderWindow()->GetSize();

  // Save this so that we can restore it if the image is cancelled
  float oldImageSampleDistance = this->ImageSampleDistance;
  float oldSampleDistance      = this->SampleDistance;
  
  this->UseShortCuts = 0;
  
  // If we are using the fast interactive method, set the UseShortCuts flag

  // If we are automatically adjusting the size to achieve a desired frame
  // rate, then do that adjustment here. Base the new image sample distance 
  // on the previous one and the previous render time. Don't let
  // the adjusted image sample distance be less than the minimum image sample 
  // distance or more than the maximum image sample distance.
  if ( this->AutoAdjustSampleDistances )
    {
    this->ImageSampleDistance =
      this->ComputeRequiredImageSampleDistance( vol->GetAllocatedRenderTime(), ren, vol );
  
    // If this is an interactive render (faster than 1 frame per second) then we'll
    // increase the sample distance along the ray to improve performance
    if ( vol->GetAllocatedRenderTime() < 1.0 )
      {
      this->SampleDistance = 3 * oldSampleDistance;
      this->UseShortCuts = 1;
      }
    }

  
  this->RenderWindow = ren->GetRenderWindow();
  this->Volume = vol;
  
  if ( this->RenderWindow->CheckAbortStatus() )
    {
    this->ImageSampleDistance = oldImageSampleDistance;
    this->SampleDistance = oldSampleDistance;
    return;
    }
  
  this->UpdateColorTable( vol );
  this->UpdateGradients( vol );
  this->UpdateShadingTable( ren, vol );
  this->UpdateCroppingRegions();
  this->UpdateMinMaxVolume( vol );
  
  if ( this->RenderWindow->CheckAbortStatus() )
    {
    this->ImageSampleDistance = oldImageSampleDistance;
    this->SampleDistance = oldSampleDistance;
    return;
    }
  
  // The full image fills the viewport. First, compute the actual viewport
  // size, then divide by the ImageSampleDistance to find the full image
  // size in pixels
  int width, height;
  ren->GetTiledSize(&width, &height);
  this->ImageViewportSize[0] = 
    static_cast<int>(width/this->ImageSampleDistance);
  this->ImageViewportSize[1] = 
    static_cast<int>(height/this->ImageSampleDistance);
  
  // Compute row bounds. This will also compute the size of the image to
  // render, allocate the space if necessary, and clear the image where
  // required. If no rays need to be cast, restore the old image sample 
  // distance and return
  if ( !this->ComputeRowBounds( vol, ren )  )
    {
    this->ImageSampleDistance = oldImageSampleDistance;
    this->SampleDistance = oldSampleDistance;
    return;
    }
  
  // Do we need to capture the z buffer to intermix intersecting
  // geometry? If so, do it here
  if ( this->IntermixIntersectingGeometry && 
       ren->GetNumberOfPropsRendered() )
    {
    int x1, x2, y1, y2;
      
    // turn this->ImageOrigin into (x1,y1) in window (not viewport!)
    // coordinates. 
    x1 = static_cast<int> (
      viewport[0] * static_cast<float>(renWinSize[0]) +
      static_cast<float>(this->ImageOrigin[0]) * this->ImageSampleDistance );
    y1 = static_cast<int> (
      viewport[1] * static_cast<float>(renWinSize[1]) +
      static_cast<float>(this->ImageOrigin[1]) * this->ImageSampleDistance);
      
    // compute z buffer size
    this->ZBufferSize[0] = static_cast<int>(
      static_cast<float>(this->ImageInUseSize[0]) * this->ImageSampleDistance);
    this->ZBufferSize[1] = static_cast<int>(
      static_cast<float>(this->ImageInUseSize[1]) * this->ImageSampleDistance);
      
    // Use the size to compute (x2,y2) in window coordinates
    x2 = x1 + this->ZBufferSize[0] - 1;
    y2 = y1 + this->ZBufferSize[1] - 1;
      
    // This is the z buffer origin (in viewport coordinates)
    this->ZBufferOrigin[0] = static_cast<int>(
      static_cast<float>(this->ImageOrigin[0]) * this->ImageSampleDistance);
    this->ZBufferOrigin[1] = static_cast<int>(
      static_cast<float>(this->ImageOrigin[1]) * this->ImageSampleDistance);
      
    // Capture the z buffer
    this->ZBuffer = ren->GetRenderWindow()->GetZbufferData(x1,y1,x2,y2);
    }
    
  
  this->InitializeRayInfo( vol );
  
  if ( this->RenderWindow->GetAbortRender() )
    {
    this->ImageSampleDistance = oldImageSampleDistance;
    this->SampleDistance = oldSampleDistance;
    return;
    }

  // Set the number of threads to use for ray casting,
  // then set the execution method and do it.
  this->Threader->SetNumberOfThreads( this->NumberOfThreads );
  this->Threader->SetSingleMethod( FixedPointVolumeRayCastMapper_CastRays,
                                  (void *)this);
  this->Threader->SingleMethodExecute();
  
  if ( !ren->GetRenderWindow()->GetAbortRender() )
    {
      float depth;
      if ( this->IntermixIntersectingGeometry )
        {
        depth = this->MinimumViewDistance;
        }
      else
        {
        depth = -1;
        }
    
      this->ImageDisplayHelper->
        RenderTexture( vol, ren,
                       this->ImageMemorySize,
                       this->ImageViewportSize,
                       this->ImageInUseSize,
                       this->ImageOrigin,
                       depth,
                       this->Image );
    this->Timer->StopTimer();
    this->TimeToDraw = this->Timer->GetElapsedTime();
    // If we've increased the sample distance, account for that in the stored time. Since we
    // don't get linear performance improvement, use a factor of .66
    this->StoreRenderTime( ren, vol, 
                           this->TimeToDraw * this->ImageSampleDistance * this->ImageSampleDistance *
                           ( 1.0 + 0.66*(this->SampleDistance - oldSampleDistance) / oldSampleDistance ) );
    
//    cout << "Took " << this->TimeToDraw << " for " << this->ImageSampleDistance << " (" <<
//      this->ImageInUseSize[0] << "," << this->ImageInUseSize[1] << ")" << endl;
    }
  // Restore the image sample distance so that automatic adjustment
  // will work correctly
  else
    {
    this->ImageSampleDistance = oldImageSampleDistance;
    }
    
  this->SampleDistance = oldSampleDistance;
  if ( this->UseShortCuts )
    {
    this->ImageSampleDistance = oldImageSampleDistance;
    }
  
  if ( this->ZBuffer )
    {
    delete [] this->ZBuffer;
    this->ZBuffer = NULL;
    }
}

VTK_THREAD_RETURN_TYPE FixedPointVolumeRayCastMapper_CastRays( void *arg )
{
  // Get the info out of the input structure
  int threadID    = ((vtkMultiThreader::ThreadInfo *)(arg))->ThreadID;
  int threadCount = ((vtkMultiThreader::ThreadInfo *)(arg))->NumberOfThreads;

  vtkFixedPointVolumeRayCastMapper *me = (vtkFixedPointVolumeRayCastMapper *)(((vtkMultiThreader::ThreadInfo *)arg)->UserData);
  
  if ( !me )
    {
    vtkGenericWarningMacro("Irrecoverable error: no mapper specified");
    return VTK_THREAD_RETURN_VALUE;
    }

  vtkVolume *vol = me->GetVolume();
  if ( me->GetBlendMode() == vtkVolumeMapper::MAXIMUM_INTENSITY_BLEND )
    {
    me->GetMIPHelper()->GenerateImage( threadID, threadCount, vol, me );
    }
  else
    {
    if ( me->GetShadingRequired() == 0 )
      {
      if ( me->GetGradientOpacityRequired() == 0 )
        {
        me->GetCompositeHelper()->GenerateImage( threadID, threadCount, vol, me );
        }
      else
        {
        me->GetCompositeGOHelper()->GenerateImage( threadID, threadCount, vol, me );
        }
      }
    else
      {
      if ( me->GetGradientOpacityRequired() == 0 )
        {
        me->GetCompositeShadeHelper()->GenerateImage( threadID, threadCount, vol, me );
        }
      else
        {
        me->GetCompositeGOShadeHelper()->GenerateImage( threadID, threadCount, vol, me );
        }
      }
    }
  
  return VTK_THREAD_RETURN_VALUE;
}

float vtkFixedPointVolumeRayCastMapper::GetZBufferValue(int x, int y)
{
  int xPos, yPos;
  
  xPos = static_cast<int>(static_cast<float>(x) * this->ImageSampleDistance);
  yPos = static_cast<int>(static_cast<float>(y) * this->ImageSampleDistance);
  
  xPos = (xPos >= this->ZBufferSize[0])?(this->ZBufferSize[0]-1):(xPos);
  yPos = (yPos >= this->ZBufferSize[1])?(this->ZBufferSize[1]-1):(yPos);
  
  return *(this->ZBuffer + yPos*this->ZBufferSize[0] + xPos);
}

void vtkFixedPointVolumeRayCastMapper::ComputeRayInfo( int x, int y, unsigned int pos[3],
                                                       unsigned int dir[3],
                                                       unsigned int *numSteps )
{
  float viewRay[3];
  float rayDirection[3];
  float rayStart[4], rayEnd[4];
  
  float offsetX = 1.0 / static_cast<float>(this->ImageViewportSize[0]);
  float offsetY = 1.0 / static_cast<float>(this->ImageViewportSize[1]);


  // compute the view point y value for this row. Do this by 
  // taking our pixel position, adding the image origin then dividing
  // by the full image size to get a number from 0 to 1-1/fullSize. Then,
  // multiply by two and subtract one to get a number from 
  // -1 to 1 - 2/fullSize. Then add offsetX (which is 1/fullSize) to 
  // center it.
  viewRay[1] = ((static_cast<float>(y) + 
                 static_cast<float>(this->ImageOrigin[1])) /
                this->ImageViewportSize[1]) * 2.0 - 1.0 + offsetY;
  
  // compute the view point x value for this pixel. Do this by 
  // taking our pixel position, adding the image origin then dividing
  // by the full image size to get a number from 0 to 1-1/fullSize. Then,
  // multiply by two and subtract one to get a number from 
  // -1 to 1 - 2/fullSize. Then add offsetX (which is 1/fullSize) to 
  // center it.
  viewRay[0] = ((static_cast<float>(x) + 
                 static_cast<float>(this->ImageOrigin[0])) /
                this->ImageViewportSize[0]) * 2.0 - 1.0 + offsetX;
      
  // Now transform this point with a z value of 0 for the ray start, and
  // a z value of 1 for the ray end. This corresponds to the near and far
  // plane locations. If IntermixIntersectingGeometry is on, then use
  // the zbuffer value instead of 1.0
  viewRay[2] = 0.0;
  vtkVRCMultiplyPointMacro( viewRay, rayStart,
                            this->ViewToVoxelsArray );

  viewRay[2] = 
    (this->ZBuffer)?(this->GetZBufferValue(x,y)):(1.0);
  
  vtkVRCMultiplyPointMacro( viewRay, rayEnd,
                            this->ViewToVoxelsArray );

  rayDirection[0] = rayEnd[0] - rayStart[0];
  rayDirection[1] = rayEnd[1] - rayStart[1];
  rayDirection[2] = rayEnd[2] - rayStart[2];

  float originalRayStart[3];
  originalRayStart[0] = rayStart[0];
  originalRayStart[1] = rayStart[1];
  originalRayStart[2] = rayStart[2];
  
  
  // Initialize with 0, fill in with actual number of steps
  // if necessary
  *numSteps = 0;
      
  if ( this->ClipRayAgainstVolume( rayStart,
                                   rayEnd,
                                   rayDirection,
                                   this->CroppingBounds ) &&
       ( this->NumTransformedClippingPlanes == 0 ||
         this->ClipRayAgainstClippingPlanes( rayStart, 
                                             rayEnd,
                                             this->NumTransformedClippingPlanes,
                                             this->TransformedClippingPlanes ) ) )
    {
    double worldRayDirection[3];
    worldRayDirection[0] = rayDirection[0]*this->SavedSpacing[0];
    worldRayDirection[1] = rayDirection[1]*this->SavedSpacing[1];
    worldRayDirection[2] = rayDirection[2]*this->SavedSpacing[2];
    double worldLength = 
      vtkMath::Normalize( worldRayDirection ) / this->SampleDistance;
        
    rayDirection[0] /= worldLength;
    rayDirection[1] /= worldLength;
    rayDirection[2] /= worldLength;
      
    float diff[3];
    diff[0] = (rayStart[0] - originalRayStart[0])*((rayDirection[0]<0)?(-1):(1));
    diff[1] = (rayStart[1] - originalRayStart[1])*((rayDirection[1]<0)?(-1):(1));
    diff[2] = (rayStart[2] - originalRayStart[2])*((rayDirection[2]<0)?(-1):(1));

    int steps = -1;
        
    if ( diff[0] >= diff[1] && diff[0] >= diff[2] && rayDirection[0])
      {
      steps = 1 + static_cast<int>( diff[0] /
                                    ((rayDirection[0]<0)?(-rayDirection[0]):(rayDirection[0])) );
      }
    
    if ( diff[1] >= diff[0] && diff[1] >= diff[2] && rayDirection[2])
      {
      steps = 1 + static_cast<int>( diff[1] /
                                    ((rayDirection[1]<0)?(-rayDirection[1]):(rayDirection[1])) );
      }
    
    if ( diff[2] >= diff[0] && diff[2] >= diff[1] && rayDirection[2])
      {
      steps = 1 + static_cast<int>( diff[2] /
                                    ((rayDirection[2]<0)?(-rayDirection[2]):(rayDirection[2])) );
      }
    
    if ( steps > 0 )
      {
      rayStart[0] = originalRayStart[0] + steps*rayDirection[0];
      rayStart[1] = originalRayStart[1] + steps*rayDirection[1];
      rayStart[2] = originalRayStart[2] + steps*rayDirection[2];
      }

    if ( rayStart[0] > 0.0 && rayStart[1] > 0.0 && rayStart[2] > 0.0 )
      {
      pos[0] = this->ToFixedPointPosition(rayStart[0]);        
      pos[1] = this->ToFixedPointPosition(rayStart[1]);        
      pos[2] = this->ToFixedPointPosition(rayStart[2]);        
      dir[0] = this->ToFixedPointDirection(rayDirection[0]);        
      dir[1] = this->ToFixedPointDirection(rayDirection[1]);        
      dir[2] = this->ToFixedPointDirection(rayDirection[2]);        
        
      unsigned int biggestVal, biggestIndex;
      biggestIndex = 0;
      biggestVal = dir[0];
      if ( (dir[1]&0x7fffffff) > (biggestVal&0x7fffffff) )
        {
        biggestIndex = 1;
        biggestVal = dir[1];
        }
      if ( (dir[2]&0x7fffffff) > (biggestVal&0x7fffffff) )
        {
        biggestIndex = 2;
        biggestVal = dir[2];
        }
      
      
      unsigned int endVal = this->ToFixedPointPosition(rayEnd[biggestIndex]);
        
      if ( biggestVal&0x80000000 )
        {
        if ( endVal > pos[biggestIndex] )
          {
          *numSteps = 1 + (endVal - pos[biggestIndex])/(biggestVal&0x7fffffff);
          }
        else
          {
          *numSteps = 0;
          }
        }
      else
        {
        if ( pos[biggestIndex] > endVal )
          {
          *numSteps = 1 + (pos[biggestIndex]- endVal)/biggestVal;
          }
        else
          {
          *numSteps = 0;
          }
        }
      }
    }
}

void vtkFixedPointVolumeRayCastMapper::InitializeRayInfo( vtkVolume   *vol )
{
  if ( !vol )
    {
    return;
    }
  
  // Copy the viewToVoxels matrix to 16 floats
  int i, j;
  for ( j = 0; j < 4; j++ )
    {
    for ( i = 0; i < 4; i++ )
      {
      this->ViewToVoxelsArray[j*4+i] = 
        static_cast<float>(this->ViewToVoxelsMatrix->GetElement(j,i));
      }
    }
  
  // Copy the worldToVoxels matrix to 16 floats
  for ( j = 0; j < 4; j++ )
    {
    for ( i = 0; i < 4; i++ )
      {
      this->WorldToVoxelsArray[j*4+i] = 
        static_cast<float>(this->WorldToVoxelsMatrix->GetElement(j,i));
      }
    }
  
  // Copy the voxelsToWorld matrix to 16 floats
  for ( j = 0; j < 4; j++ )
    {
    for ( i = 0; i < 4; i++ )
      {
      this->VoxelsToWorldArray[j*4+i] = 
        static_cast<float>(this->VoxelsToWorldMatrix->GetElement(j,i));
      }
    }
  
  int dim[3];
  this->GetInput()->GetDimensions(dim);
  this->CroppingBounds[0] = this->CroppingBounds[2] = this->CroppingBounds[4] = 0.0;
  this->CroppingBounds[1] = dim[0]-1;
  this->CroppingBounds[3] = dim[1]-1;
  this->CroppingBounds[5] = dim[2]-1;

  
  // Do some initialization of the clipping planes
  this->NumTransformedClippingPlanes = (this->ClippingPlanes)?(this->ClippingPlanes->GetNumberOfItems()):(0);
  
  // Clear out old clipping planes
  delete [] this->TransformedClippingPlanes;
  this->TransformedClippingPlanes = NULL;

  // Do we have any clipping planes
  if ( this->NumTransformedClippingPlanes > 0 )
    {
    // Allocate some space to store the plane equations
    this->TransformedClippingPlanes = new float [4*this->NumTransformedClippingPlanes];
  
    // loop through all the clipping planes
    for ( i = 0; i < this->NumTransformedClippingPlanes; i++ )
      {
      // Convert plane into voxel coordinate system
      double worldNormal[3], worldOrigin[3];
      double volumeOrigin[4];
      vtkPlane *onePlane = (vtkPlane *)this->ClippingPlanes->GetItemAsObject(i);
      onePlane->GetNormal(worldNormal);
      onePlane->GetOrigin(worldOrigin);
      float *planePtr = this->TransformedClippingPlanes + 4*i;
      vtkVRCMultiplyNormalMacro( worldNormal, 
                                 planePtr,
                                 this->VoxelsToWorldArray );
      vtkVRCMultiplyPointMacro( worldOrigin, volumeOrigin,
                                this->WorldToVoxelsArray );
      
      float t = sqrt( planePtr[0]*planePtr[0] +
                      planePtr[1]*planePtr[1] +
                      planePtr[2]*planePtr[2] );
      if ( t )
        {
        planePtr[0] /= t;
        planePtr[1] /= t;
        planePtr[2] /= t;
        }
      
      planePtr[3] = -(planePtr[0]*volumeOrigin[0] + 
                      planePtr[1]*volumeOrigin[1] + 
                      planePtr[2]*volumeOrigin[2]);
      }
    }

  // If we have a simple crop box then we can tighten the bounds
  if ( this->Cropping && this->CroppingRegionFlags == 0x2000 )
    {
    this->CroppingBounds[0] = this->VoxelCroppingRegionPlanes[0];
    this->CroppingBounds[1] = this->VoxelCroppingRegionPlanes[1];
    this->CroppingBounds[2] = this->VoxelCroppingRegionPlanes[2];
    this->CroppingBounds[3] = this->VoxelCroppingRegionPlanes[3];
    this->CroppingBounds[4] = this->VoxelCroppingRegionPlanes[4];
    this->CroppingBounds[5] = this->VoxelCroppingRegionPlanes[5];
    }

  this->CroppingBounds[0] = (this->CroppingBounds[0] < 0)?(0):(this->CroppingBounds[0]);
  this->CroppingBounds[0] = (this->CroppingBounds[0] > dim[0]-1)?(dim[0]-1):(this->CroppingBounds[0]);
  this->CroppingBounds[1] = (this->CroppingBounds[1] < 0)?(0):(this->CroppingBounds[1]);
  this->CroppingBounds[1] = (this->CroppingBounds[1] > dim[0]-1)?(dim[0]-1):(this->CroppingBounds[1]);
  this->CroppingBounds[2] = (this->CroppingBounds[2] < 0)?(0):(this->CroppingBounds[2]);
  this->CroppingBounds[2] = (this->CroppingBounds[2] > dim[1]-1)?(dim[1]-1):(this->CroppingBounds[2]);
  this->CroppingBounds[3] = (this->CroppingBounds[3] < 0)?(0):(this->CroppingBounds[3]);
  this->CroppingBounds[3] = (this->CroppingBounds[3] > dim[1]-1)?(dim[1]-1):(this->CroppingBounds[3]);
  this->CroppingBounds[4] = (this->CroppingBounds[4] < 0)?(0):(this->CroppingBounds[4]);
  this->CroppingBounds[4] = (this->CroppingBounds[4] > dim[2]-1)?(dim[2]-1):(this->CroppingBounds[4]);
  this->CroppingBounds[5] = (this->CroppingBounds[5] < 0)?(0):(this->CroppingBounds[5]);
  this->CroppingBounds[5] = (this->CroppingBounds[5] > dim[2]-1)?(dim[2]-1):(this->CroppingBounds[5]);  
  
  // Save spacing because for some reason this call is really really slow!
  this->GetInput()->GetSpacing(this->SavedSpacing);
}

int vtkFixedPointVolumeRayCastMapper::ComputeRowBounds(vtkVolume   *vol,
                                               vtkRenderer *ren)
{
  float voxelPoint[3];
  float viewPoint[8][4];
  int i, j, k;
  unsigned char *ucptr;
  float minX, minY, maxX, maxY, minZ, maxZ;

  minX =  1.0;
  minY =  1.0;
  maxX = -1.0;
  maxY = -1.0;
  minZ =  1.0;
  maxZ =  0.0;
  
  float bounds[6];
  int dim[3];
  
  this->GetInput()->GetDimensions(dim);
  bounds[0] = bounds[2] = bounds[4] = 0.0;
  bounds[1] = static_cast<float>(dim[0]-1);
  bounds[3] = static_cast<float>(dim[1]-1);
  bounds[5] = static_cast<float>(dim[2]-1);
  
  double camPos[3];
  double worldBounds[6];
  vol->GetBounds( worldBounds );
  int insideFlag = 0;
  ren->GetActiveCamera()->GetPosition( camPos );
  if ( camPos[0] >= worldBounds[0] &&
       camPos[0] <= worldBounds[1] &&
       camPos[1] >= worldBounds[2] &&
       camPos[1] <= worldBounds[3] &&
       camPos[2] >= worldBounds[4] &&
       camPos[2] <= worldBounds[5] )
    {
    insideFlag = 1;
    }
  
  
  // If we have a simple crop box then we can tighten the bounds
  if ( this->Cropping && this->CroppingRegionFlags == 0x2000 )
    {
    bounds[0] = this->VoxelCroppingRegionPlanes[0];
    bounds[1] = this->VoxelCroppingRegionPlanes[1];
    bounds[2] = this->VoxelCroppingRegionPlanes[2];
    bounds[3] = this->VoxelCroppingRegionPlanes[3];
    bounds[4] = this->VoxelCroppingRegionPlanes[4];
    bounds[5] = this->VoxelCroppingRegionPlanes[5];
    }
  
  
  // Copy the voxelsToView matrix to 16 floats
  float voxelsToViewMatrix[16];
  for ( j = 0; j < 4; j++ )
    {
    for ( i = 0; i < 4; i++ )
      {
      voxelsToViewMatrix[j*4+i] = 
        static_cast<float>(this->VoxelsToViewMatrix->GetElement(j,i));
      }
    }
  
  // Convert the voxel bounds to view coordinates to find out the
  // size and location of the image we need to generate. 
  int idx = 0;
  if ( insideFlag )
    {
    minX = -1.0;
    maxX =  1.0;
    minY = -1.0;
    maxY =  1.0;
    minZ =  0.001;
    maxZ =  0.001;
    }
  else
    {
    for ( k = 0; k < 2; k++ )
      {
      voxelPoint[2] = bounds[4+k];
      for ( j = 0; j < 2; j++ )
        {
        voxelPoint[1] = bounds[2+j];
        for ( i = 0; i < 2; i++ )
          {
          voxelPoint[0] = bounds[i];
          vtkVRCMultiplyPointMacro( voxelPoint, viewPoint[idx],
                                   voxelsToViewMatrix );
          
          minX = (viewPoint[idx][0]<minX)?(viewPoint[idx][0]):(minX);
          minY = (viewPoint[idx][1]<minY)?(viewPoint[idx][1]):(minY);        
          maxX = (viewPoint[idx][0]>maxX)?(viewPoint[idx][0]):(maxX);
          maxY = (viewPoint[idx][1]>maxY)?(viewPoint[idx][1]):(maxY);
          minZ = (viewPoint[idx][2]<minZ)?(viewPoint[idx][2]):(minZ);
          maxZ = (viewPoint[idx][2]>maxZ)?(viewPoint[idx][2]):(maxZ);
          idx++;
          }
        }
      }
    }
  
  if ( minZ < 0.001 || maxZ > 0.9999 )
    {
    minX = -1.0;
    maxX =  1.0;
    minY = -1.0;
    maxY =  1.0;
    insideFlag = 1;
    }
  
  this->MinimumViewDistance = 
    (minZ<0.001)?(0.001):((minZ>0.999)?(0.999):(minZ));
  
  // We have min/max values from -1.0 to 1.0 now - we want to convert 
  // these to pixel locations. Give a couple of pixels of breathing room
  // on each side if possible
  minX = ( minX + 1.0 ) * 0.5 * this->ImageViewportSize[0] - 2; 
  minY = ( minY + 1.0 ) * 0.5 * this->ImageViewportSize[1] - 2; 
  maxX = ( maxX + 1.0 ) * 0.5 * this->ImageViewportSize[0] + 2; 
  maxY = ( maxY + 1.0 ) * 0.5 * this->ImageViewportSize[1] + 2;

  // If we are outside the view frustum return 0 - there is no need
  // to render anything
  if ( ( minX < 0 && maxX < 0 ) ||
       ( minY < 0 && maxY < 0 ) ||
       ( minX > this->ImageViewportSize[0]-1 &&
         maxX > this->ImageViewportSize[0]-1 ) ||
       ( minX > this->ImageViewportSize[0]-1 &&
         maxX > this->ImageViewportSize[0]-1 ) )
    {
    return 0;
    }

  int oldImageMemorySize[2];
  oldImageMemorySize[0] = this->ImageMemorySize[0];
  oldImageMemorySize[1] = this->ImageMemorySize[1];
  
  // Swap the row bounds
  int *tmpptr;
  tmpptr = this->RowBounds;
  this->RowBounds = this->OldRowBounds;
  this->OldRowBounds = tmpptr;
  
  // Check the bounds - the volume might project outside of the 
  // viewing box / frustum so clip it if necessary
  minX = (minX<0)?(0):(minX);
  minY = (minY<0)?(0):(minY);
  maxX = (maxX>this->ImageViewportSize[0]-1)?
    (this->ImageViewportSize[0]-1):(maxX);
  maxY = (maxY>this->ImageViewportSize[1]-1)?
    (this->ImageViewportSize[1]-1):(maxY);

  // Create the new image, and set its size and position
  this->ImageInUseSize[0] = static_cast<int>(maxX - minX + 1.0);
  this->ImageInUseSize[1] = static_cast<int>(maxY - minY + 1.0);

  // What is a power of 2 size big enough to fit this image?
  this->ImageMemorySize[0] = 32;
  this->ImageMemorySize[1] = 32;
  while ( this->ImageMemorySize[0] < this->ImageInUseSize[0] )
    {
    this->ImageMemorySize[0] *= 2;
    }
  while ( this->ImageMemorySize[1] < this->ImageInUseSize[1] )
    {
    this->ImageMemorySize[1] *= 2;
    }
  
  this->ImageOrigin[0] = static_cast<int>(minX);
  this->ImageOrigin[1] = static_cast<int>(minY);

  // If the old image size is much too big (more than twice in
  // either direction) then set the old width to 0 which will
  // cause the image to be recreated
  if ( oldImageMemorySize[0] > 4*this->ImageMemorySize[0] ||
       oldImageMemorySize[1] > 4*this->ImageMemorySize[1] )
    {
    oldImageMemorySize[0] = 0;
    }
  
  // If the old image is big enough (but not too big - we handled
  // that above) then we'll bump up our required size to the
  // previous one. This will keep us from thrashing.
  if ( oldImageMemorySize[0] >= this->ImageMemorySize[0] &&
       oldImageMemorySize[1] >= this->ImageMemorySize[1] )
    {
    this->ImageMemorySize[0] = oldImageMemorySize[0];
    this->ImageMemorySize[1] = oldImageMemorySize[1];
    }
  
  // Do we already have a texture big enough? If not, create a new one and
  // clear it.
  if ( !this->Image ||
       this->ImageMemorySize[0] > oldImageMemorySize[0] ||
       this->ImageMemorySize[1] > oldImageMemorySize[1] )
    {
    delete [] this->Image;
    delete [] this->RowBounds;
    delete [] this->OldRowBounds;
    
    this->Image = new unsigned char[(this->ImageMemorySize[0] *
                                     this->ImageMemorySize[1] * 4)];

    // Create the row bounds array. This will store the start / stop pixel
    // for each row. This helps eleminate work in areas outside the bounding
    // hexahedron since a bounding box is not very tight. We keep the old ones
    // too to help with only clearing where required
    this->RowBounds = new int [2*this->ImageMemorySize[1]];
    this->OldRowBounds = new int [2*this->ImageMemorySize[1]];

    for ( i = 0; i < this->ImageMemorySize[1]; i++ )
      {
      this->RowBounds[i*2]      = this->ImageMemorySize[0];
      this->RowBounds[i*2+1]    = -1;
      this->OldRowBounds[i*2]   = this->ImageMemorySize[0];
      this->OldRowBounds[i*2+1] = -1;
      }

    if ( this->RenderWindow->CheckAbortStatus() )
      {
      return 0;
      }
    
    ucptr = this->Image;
    
    for ( i = 0; i < this->ImageMemorySize[0]*this->ImageMemorySize[1]; i++ )
      {
      *(ucptr++) = 0;
      *(ucptr++) = 0;
      *(ucptr++) = 0;
      *(ucptr++) = 0;      
      }
    }

  if ( this->RenderWindow->CheckAbortStatus() )
    {
    return 0;
    }
    
  // If we are inside the volume our row bounds indicate every ray must be
  // cast - we don't need to intersect with the 12 lines
  if ( insideFlag )
    {
    for ( j = 0; j < this->ImageInUseSize[1]; j++ )
      {
      this->RowBounds[j*2] = 0;
      this->RowBounds[j*2+1] = this->ImageInUseSize[0] - 1;
      }
    }
  else
    {
    // create an array of lines where the y value of the first vertex is less
    // than or equal to the y value of the second vertex. There are 12 lines,
    // each containing x1, y1, x2, y2 values.
    float lines[12][4];
    float x1, y1, x2, y2;
    int xlow, xhigh;
    int lineIndex[12][2] = {{0,1}, {2,3}, {4,5}, {6,7}, 
                            {0,2}, {1,3} ,{4,6}, {5,7},
                            {0,4}, {1,5}, {2,6}, {3,7}};
  
    for ( i = 0; i < 12; i++ )
      {
      x1 = (viewPoint[lineIndex[i][0]][0]+1.0) * 
        0.5*this->ImageViewportSize[0] - this->ImageOrigin[0];
      
      y1 = (viewPoint[lineIndex[i][0]][1]+1.0) * 
        0.5*this->ImageViewportSize[1] - this->ImageOrigin[1];
      
      x2 = (viewPoint[lineIndex[i][1]][0]+1.0) * 
        0.5*this->ImageViewportSize[0] - this->ImageOrigin[0];
      
      y2 = (viewPoint[lineIndex[i][1]][1]+1.0) * 
        0.5*this->ImageViewportSize[1] - this->ImageOrigin[1];
      
      if ( y1 < y2 )
        {
        lines[i][0] = x1;
        lines[i][1] = y1;
        lines[i][2] = x2;
        lines[i][3] = y2;
        }
      else
        {
        lines[i][0] = x2;
        lines[i][1] = y2;
        lines[i][2] = x1;
        lines[i][3] = y1;
        }
      }

    // Now for each row in the image, find out the start / stop pixel
    // If min > max, then no intersection occurred
    for ( j = 0; j < this->ImageInUseSize[1]; j++ )
      {
      this->RowBounds[j*2] = this->ImageMemorySize[0];
      this->RowBounds[j*2+1] = -1;
      for ( i = 0; i < 12; i++ )
        {
        if ( j >= lines[i][1] && j <= lines[i][3] &&
             ( lines[i][1] != lines[i][3] ) )
          {
          x1 = lines[i][0] +
            (static_cast<float>(j) - lines[i][1])/(lines[i][3] - lines[i][1]) *
            (lines[i][2] - lines[i][0] );

          xlow  = static_cast<int>(x1 + 1.5);
          xhigh = static_cast<int>(x1 - 1.0);
          
          xlow = (xlow<0)?(0):(xlow);
          xlow = (xlow>this->ImageInUseSize[0]-1)?
            (this->ImageInUseSize[0]-1):(xlow);

          xhigh = (xhigh<0)?(0):(xhigh);
          xhigh = (xhigh>this->ImageInUseSize[0]-1)?(
            this->ImageInUseSize[0]-1):(xhigh);

          if ( xlow < this->RowBounds[j*2] )
            {
            this->RowBounds[j*2] = xlow;
            }
          if ( xhigh > this->RowBounds[j*2+1] )
            {
            this->RowBounds[j*2+1] = xhigh;
            }
          }
        }
      // If they are the same this is either a point on the cube or
      // all lines were out of bounds (all on one side or the other)
      // It is safe to ignore the point (since the ray isn't likely
      // to travel through it enough to actually take a sample) and it
      // must be ignored in the case where all lines are out of range
      if ( this->RowBounds[j*2] == this->RowBounds[j*2+1] )
        {
        this->RowBounds[j*2] = this->ImageMemorySize[0];
        this->RowBounds[j*2+1] = -1;
        }
      }
    }
  
  for ( j = this->ImageInUseSize[1]; j < this->ImageMemorySize[1]; j++ )
    {
    this->RowBounds[j*2] = this->ImageMemorySize[0];
    this->RowBounds[j*2+1] = -1;
    }

  for ( j = 0; j < this->ImageMemorySize[1]; j++ )
    {
    if ( j%64 == 1 && this->RenderWindow->CheckAbortStatus() )
      {
      return 0;
      }

    // New bounds are not overlapping with old bounds - clear between
    // old bounds only
    if ( this->RowBounds[j*2+1] < this->OldRowBounds[j*2] ||
         this->RowBounds[j*2]   > this->OldRowBounds[j*2+1] )
      {
      ucptr = this->Image + 4*( j*this->ImageMemorySize[0] + 
                                this->OldRowBounds[j*2] );
      for ( i = 0; 
            i <= (this->OldRowBounds[j*2+1] - this->OldRowBounds[j*2]);
            i++ )
        {
        *(ucptr++) = 0;
        *(ucptr++) = 0;
        *(ucptr++) = 0;
        *(ucptr++) = 0;
        }
      }
    // New bounds do overlap with old bounds
    else
      {
      // Clear from old min to new min
      ucptr = this->Image + 4*( j*this->ImageMemorySize[0] + 
                                this->OldRowBounds[j*2] );
      for ( i = 0; 
            i < (this->RowBounds[j*2] - this->OldRowBounds[j*2]);
            i++ )
        {
        *(ucptr++) = 0;
        *(ucptr++) = 0;
        *(ucptr++) = 0;
        *(ucptr++) = 0;
        }
      
      // Clear from new max to old max
      ucptr = this->Image + 4*( j*this->ImageMemorySize[0] + 
                                this->RowBounds[j*2+1]+1 );
      for ( i = 0; 
            i < (this->OldRowBounds[j*2+1] - this->RowBounds[j*2+1]);
            i++ )
        {
        *(ucptr++) = 0;        
        *(ucptr++) = 0;        
        *(ucptr++) = 0;        
        *(ucptr++) = 0;        
        }

      }
    }
  
  return 1;
}


void vtkFixedPointVolumeRayCastMapper::ComputeMatrices( vtkImageData *data,
                                                vtkRenderer *ren,
                                                vtkVolume *vol )
{
  // Get the camera from the renderer
  vtkCamera *cam = ren->GetActiveCamera();
  
  // Get the aspect ratio from the renderer. This is needed for the
  // computation of the perspective matrix
  ren->ComputeAspect();
  double *aspect = ren->GetAspect();

  // Keep track of the projection matrix - we'll need it in a couple of places
  // Get the projection matrix. The method is called perspective, but
  // the matrix is valid for perspective and parallel viewing transforms.
  // Don't replace this with the GetCompositePerspectiveTransformMatrix 
  // because that turns off stereo rendering!!!
  this->PerspectiveTransform->Identity();
  this->PerspectiveTransform->
    Concatenate(cam->GetPerspectiveTransformMatrix(aspect[0]/aspect[1], 
                                                   0.0, 1.0 ));
  this->PerspectiveTransform->Concatenate(cam->GetViewTransformMatrix());
  this->PerspectiveMatrix->DeepCopy(this->PerspectiveTransform->GetMatrix());


  // Get the data spacing. This scaling is not accounted for in
  // the volume's matrix, so we must add it in.
  double volumeSpacing[3];
  data->GetSpacing( volumeSpacing );
  
  // Get the origin of the data.  This translation is not accounted for in
  // the volume's matrix, so we must add it in.
  double volumeOrigin[3];
  data->GetOrigin( volumeOrigin );

  // Get the dimensions of the data.
  int volumeDimensions[3];
  data->GetDimensions( volumeDimensions );

  int extent[6];
  data->GetExtent( extent );
  volumeOrigin[0] += extent[0]*volumeSpacing[0];
  volumeOrigin[1] += extent[2]*volumeSpacing[1];
  volumeOrigin[2] += extent[4]*volumeSpacing[2];
  
  vtkTransform *voxelsTransform = this->VoxelsTransform;
  vtkTransform *voxelsToViewTransform = this->VoxelsToViewTransform;
  
  // Get the volume matrix. This is a volume to world matrix right now. 
  // We'll need to invert it, translate by the origin and scale by the 
  // spacing to change it to a world to voxels matrix.
  this->VolumeMatrix->DeepCopy( vol->GetMatrix() );
  
  voxelsToViewTransform->SetMatrix( this->VolumeMatrix );

  // Create a transform that will account for the scaling and translation of
  // the scalar data. The is the volume to voxels matrix.
  voxelsTransform->Identity();
  voxelsTransform->Translate(volumeOrigin[0], 
                             volumeOrigin[1], 
                             volumeOrigin[2] );
  
  voxelsTransform->Scale( volumeSpacing[0],
                          volumeSpacing[1],
                          volumeSpacing[2] );
  
  // Now concatenate the volume's matrix with this scalar data matrix
  voxelsToViewTransform->PreMultiply();
  voxelsToViewTransform->Concatenate( voxelsTransform->GetMatrix() );

  // Now we actually have the world to voxels matrix - copy it out
  this->WorldToVoxelsMatrix->DeepCopy( voxelsToViewTransform->GetMatrix() );
  this->WorldToVoxelsMatrix->Invert();
  
  // We also want to invert this to get voxels to world
  this->VoxelsToWorldMatrix->DeepCopy( voxelsToViewTransform->GetMatrix() );
  
  // Compute the voxels to view transform by concatenating the
  // voxels to world matrix with the projection matrix (world to view)
  voxelsToViewTransform->PostMultiply();
  voxelsToViewTransform->Concatenate( this->PerspectiveMatrix );
  
  this->VoxelsToViewMatrix->DeepCopy( voxelsToViewTransform->GetMatrix() );
  
  this->ViewToVoxelsMatrix->DeepCopy( this->VoxelsToViewMatrix );
  this->ViewToVoxelsMatrix->Invert();  
}

int vtkFixedPointVolumeRayCastMapper::ClipRayAgainstClippingPlanes( float rayStart[3],
                                                            float rayEnd[3],
                                                            int numClippingPlanes,
                                                            float *clippingPlanes )
{

  float    *planePtr;
  int      i;
  float    t, point[3], dp;
  float    rayDir[3];
  
  rayDir[0] = rayEnd[0] - rayStart[0];
  rayDir[1] = rayEnd[1] - rayStart[1];
  rayDir[2] = rayEnd[2] - rayStart[2];

  // loop through all the clipping planes
  for ( i = 0; i < numClippingPlanes; i++ )
    {
    planePtr = clippingPlanes + 4*i;

    dp = 
      planePtr[0]*rayDir[0] + 
      planePtr[1]*rayDir[1] + 
      planePtr[2]*rayDir[2];

    if ( dp != 0.0 )
      {
      t = 
        -( planePtr[0]*rayStart[0] + 
           planePtr[1]*rayStart[1] + 
           planePtr[2]*rayStart[2] + planePtr[3]) / dp; 

      if ( t > 0.0 && t < 1.0 )
        {
        point[0] = rayStart[0] + t*rayDir[0];
        point[1] = rayStart[1] + t*rayDir[1];
        point[2] = rayStart[2] + t*rayDir[2];
        
        if ( dp > 0.0 )
          {
          rayStart[0] = point[0];
          rayStart[1] = point[1];
          rayStart[2] = point[2];
          }
        else
          {
          rayEnd[0] = point[0];
          rayEnd[1] = point[1];
          rayEnd[2] = point[2];
          }

        rayDir[0] = rayEnd[0] - rayStart[0];
        rayDir[1] = rayEnd[1] - rayStart[1];
        rayDir[2] = rayEnd[2] - rayStart[2];

        }
      // If the clipping plane is outside the ray segment, then
      // figure out if that means the ray segment goes to zero (if so
      // return 0) or doesn't affect it (if so do nothing)
      else
        {
        if ( dp >= 0.0 && t >= 1.0 )
          {
          return 0;
          }
        if ( dp <= 0.0 && t <= 0.0 )
          {
          return 0;
          }
        }
      }
    }

  return 1;
}


int vtkFixedPointVolumeRayCastMapper::ClipRayAgainstVolume( float rayStart[3],
                                                    float rayEnd[3],
                                                    float rayDirection[3],
                                                    double bounds[6] )
{
  int    loop;
  float  diff;
  float  t;

  if ( rayStart[0] >= bounds[1] ||
       rayStart[1] >= bounds[3] ||
       rayStart[2] >= bounds[5] ||
       rayStart[0] < bounds[0] || 
       rayStart[1] < bounds[2] || 
       rayStart[2] < bounds[4] )
    {
    for ( loop = 0; loop < 3; loop++ )
      {
      diff = 0;

      if ( rayStart[loop] < (bounds[2*loop]+0.01) )
        {
        diff = (bounds[2*loop]+0.01) - rayStart[loop];
        }
      else if ( rayStart[loop] > (bounds[2*loop+1]-0.01) )
        {
        diff = (bounds[2*loop+1]-0.01) - rayStart[loop];
        }
      
      if ( diff )
        {
        if ( rayDirection[loop] != 0.0 ) 
          {
          t = diff / rayDirection[loop];
          }
        else
          {
          t = -1.0;
          }
        
        if ( t > 0.0 )
          {
          rayStart[0] += rayDirection[0] * t;
          rayStart[1] += rayDirection[1] * t;
          rayStart[2] += rayDirection[2] * t;             
          }
        }
      }
    }

  // If the voxel still isn't inside the volume, then this ray
  // doesn't really intersect the volume
          
  if ( rayStart[0] >= bounds[1] ||
       rayStart[1] >= bounds[3] ||
       rayStart[2] >= bounds[5] ||
       rayStart[0] < bounds[0] || 
       rayStart[1] < bounds[2] || 
       rayStart[2] < bounds[4] )
    {
    return 0;
    }

  // The ray does intersect the volume, and we have a starting
  // position that is inside the volume
  if ( rayEnd[0] >= bounds[1] ||
       rayEnd[1] >= bounds[3] ||
       rayEnd[2] >= bounds[5] ||
       rayEnd[0] < bounds[0] || 
       rayEnd[1] < bounds[2] || 
       rayEnd[2] < bounds[4] )
    {
    for ( loop = 0; loop < 3; loop++ )
      {
      diff = 0;
      
      if ( rayEnd[loop] < (bounds[2*loop]+0.01) )
        {
        diff = (bounds[2*loop]+0.01) - rayEnd[loop];
        }
      else if ( rayEnd[loop] > (bounds[2*loop+1]-0.01) )
        {
        diff = (bounds[2*loop+1]-0.01) - rayEnd[loop];
        }
      
      if ( diff )
        {
        if ( rayDirection[loop] != 0.0 ) 
          {
          t = diff / rayDirection[loop];
          }
        else
          {
          t = 1.0;
          }
        
        if ( t < 0.0 )
          {
          rayEnd[0] += rayDirection[0] * t;
          rayEnd[1] += rayDirection[1] * t;
          rayEnd[2] += rayDirection[2] * t;
          }
        }
      }
    }
  
  // To be absolutely certain our ray remains inside the volume,
  // recompute the ray direction (since it has changed - it is not
  // normalized and therefore changes when start/end change) and move 
  // the start/end points in by 1/1000th of the distance.
  float offset;
  offset = (rayEnd[0] - rayStart[0])*0.001;
  rayStart[0] += offset;
  rayEnd[0]   -= offset;

  offset = (rayEnd[1] - rayStart[1])*0.001;
  rayStart[1] += offset;
  rayEnd[1]   -= offset;

  offset = (rayEnd[2] - rayStart[2])*0.001;
  rayStart[2] += offset;
  rayEnd[2]   -= offset;
  
  if ( rayEnd[0] >= bounds[1] ||
       rayEnd[1] >= bounds[3] ||
       rayEnd[2] >= bounds[5] ||
       rayEnd[0] < bounds[0] || 
       rayEnd[1] < bounds[2] || 
       rayEnd[2] < bounds[4] )
    {
      return 0;
    }

  if ( (rayEnd[0]-rayStart[0])*rayDirection[0] < 0.0 ||
       (rayEnd[1]-rayStart[1])*rayDirection[1] < 0.0 ||
       (rayEnd[2]-rayStart[2])*rayDirection[2] < 0.0 )
    {
    return 0;
    }
  
  return 1;
}


void vtkFixedPointVolumeRayCastMapper::ComputeGradients( vtkVolume *vol )
{
  vtkImageData *input = this->GetInput();
  
  void *dataPtr = input->GetScalarPointer();
 
 int scalarType   = input->GetScalarType();
 int components   = input->GetNumberOfScalarComponents();
 int independent  = vol->GetProperty()->GetIndependentComponents();
 
 int dim[3];
 double spacing[3];
 input->GetDimensions(dim);  
 input->GetSpacing(spacing);
 
 // Find the scalar range
 double scalarRange[4][2];
 int c;
 for ( c = 0; c < components; c++ )
   {
   input->GetPointData()->GetScalars()->GetRange(scalarRange[c], c);
   }
 
 int sliceSize = dim[0]*dim[1]*((independent)?(components):(1));
 int numSlices = dim[2];

 int i;
 
 // Delete the prior gradient normal information
 if ( this->GradientNormal )
   {
   // Contiguous? Delete in one chunk otherwise delete slice by slice
   if ( this->ContiguousGradientNormal )
     {
     delete [] this->ContiguousGradientNormal;
     this->ContiguousGradientNormal = NULL;
     }
   else
     {
     for ( i = 0; i < this->NumberOfGradientSlices; i++ )
       {
       delete [] this->GradientNormal[i];
       }
     }
   delete [] this->GradientNormal;
   this->GradientNormal = NULL;
   }
 
 // Delete the prior gradient magnitude information
 if ( this->GradientMagnitude )
   {
   // Contiguous? Delete in one chunk otherwise delete slice by slice
   if ( this->ContiguousGradientMagnitude )
     {
     delete [] this->ContiguousGradientMagnitude;
     this->ContiguousGradientMagnitude = NULL;
     }
   else
     {
     for ( i = 0; i < this->NumberOfGradientSlices; i++ )
       {
       delete [] this->GradientMagnitude[i];
       }
     }
   delete [] this->GradientMagnitude;
   this->GradientMagnitude = NULL;
   }
 
 this->NumberOfGradientSlices = numSlices;
 this->GradientNormal  = new unsigned short *[numSlices];
 this->GradientMagnitude = new unsigned char *[numSlices];

 // first, attempt contiguous memory. If this fails, then go
 // for non-contiguous
 this->ContiguousGradientNormal = new unsigned short [numSlices * sliceSize];
 this->ContiguousGradientMagnitude = new unsigned char [numSlices * sliceSize];
 
 if ( this->ContiguousGradientNormal )
   {
   // We were able to allocate contiguous space - we just need to set the
   // slice pointers here
   for ( i = 0; i < numSlices; i++ )
     {
     this->GradientNormal[i]  = this->ContiguousGradientNormal + i*sliceSize;
     }
   }
 else
   {
   // We were not able to allocate contigous space - allocate it slice by slice
   for ( i = 0; i < numSlices; i++ )
     {
     this->GradientNormal[i]  = new unsigned short [sliceSize];
     }
   }
 
 if ( this->ContiguousGradientMagnitude )
   {
   // We were able to allocate contiguous space - we just need to set the
   // slice pointers here
   for ( i = 0; i < numSlices; i++ )
     {
     this->GradientMagnitude[i]  = this->ContiguousGradientMagnitude + i*sliceSize;
     }
   }
 else
   {
   // We were not able to allocate contigous space - allocate it slice by slice
   for ( i = 0; i < numSlices; i++ )
     {
     this->GradientMagnitude[i] = new unsigned char [sliceSize];
     }
   }
 
 
 
 switch ( scalarType )
   {
   vtkTemplateMacro10( vtkFixedPointVolumeRayCastMapperComputeGradients, 
                      (VTK_TT *)(dataPtr), dim, spacing, components,
                      independent, scalarRange,
                      this->GradientNormal,
                      this->GradientMagnitude,
                      this->DirectionEncoder,
                      this );
   }
}

int vtkFixedPointVolumeRayCastMapper::UpdateShadingTable( vtkRenderer *ren,
                                                          vtkVolume *vol )
{
  if ( this->ShadingRequired == 0 )
    {
    return 0;
    }

  // How many components?
  int components = this->GetInput()->GetNumberOfScalarComponents();
  
  int c;
  for ( c = 0; c < ((vol->GetProperty()->GetIndependentComponents())?(components):(1)); c++ )
    {
    this->GradientShader->SetActiveComponent( c );
    this->GradientShader->UpdateShadingTable( ren, vol, this->GradientEstimator );
  
    float *r = this->GradientShader->GetRedDiffuseShadingTable(vol);
    float *g = this->GradientShader->GetGreenDiffuseShadingTable(vol);
    float *b = this->GradientShader->GetBlueDiffuseShadingTable(vol);
  
    float *rptr = r;
    float *gptr = g;
    float *bptr = b;
  
    unsigned short *tablePtr = this->DiffuseShadingTable[c];
  
    int i;
    for ( i = 0; i < this->DirectionEncoder->GetNumberOfEncodedDirections(); i++ )
      {
      *(tablePtr++) = static_cast<unsigned short>((*(rptr++))*VTKKW_FP_SCALE + 0.5);
      *(tablePtr++) = static_cast<unsigned short>((*(gptr++))*VTKKW_FP_SCALE + 0.5);
      *(tablePtr++) = static_cast<unsigned short>((*(bptr++))*VTKKW_FP_SCALE + 0.5);
      }

    r = this->GradientShader->GetRedSpecularShadingTable(vol);
    g = this->GradientShader->GetGreenSpecularShadingTable(vol);
    b = this->GradientShader->GetBlueSpecularShadingTable(vol);
  
    rptr = r;
    gptr = g;
    bptr = b;
  
    tablePtr = this->SpecularShadingTable[c];
  
 
    for ( i = 0; i < this->DirectionEncoder->GetNumberOfEncodedDirections(); i++ )
      {
      *(tablePtr++) = static_cast<unsigned short>((*(rptr++))*VTKKW_FP_SCALE + 0.5);
      *(tablePtr++) = static_cast<unsigned short>((*(gptr++))*VTKKW_FP_SCALE + 0.5);
      *(tablePtr++) = static_cast<unsigned short>((*(bptr++))*VTKKW_FP_SCALE + 0.5);
      }
    }
  
  return 1;
}

int vtkFixedPointVolumeRayCastMapper::UpdateGradients( vtkVolume *vol )
{
  int needToUpdate = 0;

  this->GradientOpacityRequired = 0;
  this->ShadingRequired         = 0;
  
  // Get the image data
  vtkImageData *input = this->GetInput();
  
  if ( vol->GetProperty()->GetShade() )
    {
    needToUpdate = 1;
    this->ShadingRequired = 1;
    }
  
  for ( int c = 0; c < input->GetNumberOfScalarComponents(); c++ )
    {
    vtkPiecewiseFunction *f = vol->GetProperty()->GetGradientOpacity(c);
    if ( strcmp(f->GetType(), "Constant") || f->GetValue(0.0) != 1.0 )
      {
      needToUpdate = 1;
      this->GradientOpacityRequired = 1;
      }
    }
  
  if ( !needToUpdate )
    {
    return 0;
    }

  // Check if the input has changed 
  if ( input == this->SavedGradientsInput &&
       input->GetMTime() < this->SavedGradientsMTime.GetMTime() )
    {
    return 0;
    }
  
  this->ComputeGradients( vol );
  
  // Time to save the input used to update the tabes
  this->SavedGradientsInput = this->GetInput();
  this->SavedGradientsMTime.Modified();

  return 1;
}

int vtkFixedPointVolumeRayCastMapper::UpdateColorTable( vtkVolume *vol )
{
  int needToUpdate = 0;

  // Get the image data
  vtkImageData *input = this->GetInput();

  // Has the data itself changed?
  if ( input != this->SavedParametersInput ||
       input->GetMTime() > this->SavedParametersMTime.GetMTime() )
    {
    needToUpdate = 1;
    }

  // What is the blending mode?
  int blendMode = this->GetBlendMode();
  if ( blendMode != this->SavedBlendMode )
    {
    needToUpdate = 1;
    }
  
  // How many components?
  int components = input->GetNumberOfScalarComponents();

  // Has the sample distance changed?
  if ( this->SavedSampleDistance != this->SampleDistance )
    {
    needToUpdate = 1;
    }

  vtkColorTransferFunction *rgbFunc[4];
  vtkPiecewiseFunction     *grayFunc[4];
  vtkPiecewiseFunction     *scalarOpacityFunc[4];
  vtkPiecewiseFunction     *gradientOpacityFunc[4];
  int                       colorChannels[4];
  float                     scalarOpacityDistance[4];
  
  int c;
      
  for ( c = 0; c < ((vol->GetProperty()->GetIndependentComponents())?(components):(1)); c++ )
    {
    rgbFunc[c]               = vol->GetProperty()->GetRGBTransferFunction(c);
    grayFunc[c]              = vol->GetProperty()->GetGrayTransferFunction(c);
    scalarOpacityFunc[c]     = vol->GetProperty()->GetScalarOpacity(c);
    gradientOpacityFunc[c]   = vol->GetProperty()->GetGradientOpacity(c);
    colorChannels[c]         = vol->GetProperty()->GetColorChannels(c);
    scalarOpacityDistance[c] = vol->GetProperty()->GetScalarOpacityUnitDistance(c);
    
    // Has the number of color channels changed?
    if ( this->SavedColorChannels[c] != colorChannels[c] )
      {
      needToUpdate = 1;
      }

    // Has the color transfer function changed in some way,
    // and we are using it?
    if ( colorChannels[c] == 3 )
      {
      if ( this->SavedRGBFunction[c] != rgbFunc[c] ||
           this->SavedParametersMTime.GetMTime() < rgbFunc[c]->GetMTime() )
        {
        needToUpdate = 1;
        }
      }

    // Has the gray transfer function changed in some way,
    // and we are using it?
    if ( colorChannels[c] == 1 )
      {
      if ( this->SavedGrayFunction[c] != grayFunc[c] ||
           this->SavedParametersMTime.GetMTime() < grayFunc[c]->GetMTime() )
        {
        needToUpdate = 1;
        }
      }
  
    // Has the scalar opacity transfer function changed in some way?
    if ( this->SavedScalarOpacityFunction[c] != scalarOpacityFunc[c] ||
         this->SavedParametersMTime.GetMTime() < scalarOpacityFunc[c]->GetMTime() )
      {
      needToUpdate = 1;
      }

    // Has the gradient opacity transfer function changed in some way?
    if ( this->SavedGradientOpacityFunction[c] != gradientOpacityFunc[c] ||
         this->SavedParametersMTime.GetMTime() < gradientOpacityFunc[c]->GetMTime() )
      {
      needToUpdate = 1;
      }
    
    // Has the distance over which the scalar opacity function is defined changed?
    if ( this->SavedScalarOpacityDistance[c] != scalarOpacityDistance[c] )
      {
      needToUpdate = 1;
      }
    }
  
  // If we have not found any need to update, return now
  if ( !needToUpdate )
    {
    return 0;
    }


  for ( c = 0; c < ((vol->GetProperty()->GetIndependentComponents())?(components):(1)); c++ )
    {
    this->SavedRGBFunction[c]             = rgbFunc[c];
    this->SavedGrayFunction[c]            = grayFunc[c];
    this->SavedScalarOpacityFunction[c]   = scalarOpacityFunc[c];
    this->SavedGradientOpacityFunction[c] = gradientOpacityFunc[c];
    this->SavedColorChannels[c]           = colorChannels[c];
    this->SavedScalarOpacityDistance[c]   = scalarOpacityDistance[c];
    }
  
  this->SavedSampleDistance          = this->SampleDistance;
  this->SavedBlendMode               = blendMode;
  this->SavedParametersInput         = input;
  
  this->SavedParametersMTime.Modified();

  int scalarType = input->GetScalarType();
  
  int i;
  float tmpArray[3*32768];
  
  // Find the scalar range
  double scalarRange[4][2];
  for ( c = 0; c < components; c++ )
    {
    input->GetPointData()->GetScalars()->GetRange(scalarRange[c], c);
    
    // Is the difference between max and min less than 32768? If so, and if
    // the data is not of float or double type, use a simple offset mapping.
    // If the difference between max and min is 32768 or greater, or the data
    // is of type float or double, we must use an offset / scaling mapping.
    // In this case, the array size will be 32768 - we need to figure out the 
    // offset and scale factor.
    float offset;
    float scale;
    
    int arraySizeNeeded;
    
    if ( scalarType == VTK_FLOAT ||
         scalarType == VTK_DOUBLE ||
         scalarRange[c][1] - scalarRange[c][0] > 32767 )
      {
      arraySizeNeeded = 32768;
      offset          = -scalarRange[c][0];
      scale           = 32767.0 / (scalarRange[c][1] - scalarRange[c][0]);
      }
    else
      {
      arraySizeNeeded = (int)(scalarRange[c][1] - scalarRange[c][0] + 1);
      offset          = -scalarRange[c][0]; 
      scale           = 1.0;
      }
    
    this->TableSize[c]   = arraySizeNeeded;
    this->TableShift[c]  = offset;
    this->TableScale[c]  = scale;
    }
  
  if ( vol->GetProperty()->GetIndependentComponents() )
    {
    for ( c = 0; c < components; c++ )
      {
      // Sample the transfer functions between the min and max.
      if ( colorChannels[c] == 1 )
        {
        float tmpArray2[32768];
        grayFunc[c]->GetTable( scalarRange[c][0], scalarRange[c][1], 
                               this->TableSize[c], tmpArray2 );
        for ( int index = 0; index < this->TableSize[c]; index++ )
          {
          tmpArray[3*index  ] = tmpArray2[index];
          tmpArray[3*index+1] = tmpArray2[index];
          tmpArray[3*index+2] = tmpArray2[index];
          }
        }
      else
        {
        rgbFunc[c]->GetTable( scalarRange[c][0], scalarRange[c][1], 
                              this->TableSize[c], tmpArray );
        }
      // Convert color to short format
      for ( i = 0; i < this->TableSize[c]; i++ )
        {
        this->ColorTable[c][3*i  ] = 
          static_cast<unsigned short>(tmpArray[3*i  ]*VTKKW_FP_SCALE + 0.5);
        this->ColorTable[c][3*i+1] = 
          static_cast<unsigned short>(tmpArray[3*i+1]*VTKKW_FP_SCALE + 0.5);
        this->ColorTable[c][3*i+2] = 
          static_cast<unsigned short>(tmpArray[3*i+2]*VTKKW_FP_SCALE + 0.5);
        }
      
      scalarOpacityFunc[c]->GetTable( scalarRange[c][0], scalarRange[c][1], 
                                   this->TableSize[c], tmpArray );
      
      // Correct the opacity array for the spacing between the planes if we are
      // using a composite blending operation
      if ( this->BlendMode == vtkVolumeMapper::COMPOSITE_BLEND )
        {
        float *ptr = tmpArray;    
        double factor = this->SampleDistance / vol->GetProperty()->GetScalarOpacityUnitDistance(c);
        for ( i = 0; i < this->TableSize[c]; i++ )
          {
          if ( *ptr > 0.0001 )
            {
            *ptr =  1.0-pow((double)(1.0-(*ptr)),factor);
            }
          ptr++;
          }
        }
      
      // Convert tables to short format
      for ( i = 0; i < this->TableSize[c]; i++ )
        {
        this->ScalarOpacityTable[c][i] = 
          static_cast<unsigned short>(tmpArray[i]*VTKKW_FP_SCALE + 0.5);
        }
      
      gradientOpacityFunc[c]->GetTable( 0, 
                                        (scalarRange[c][1] - scalarRange[c][0])*0.25, 
                                        256, tmpArray );
      
      for ( i = 0; i < 256; i++ )
        {
        this->GradientOpacityTable[c][i] = 
          static_cast<unsigned short>(tmpArray[i]*VTKKW_FP_SCALE + 0.5);
        }    
      }
    }
  else 
    {
    if ( components ==  2 )
      {
      // Sample the transfer functions between the min and max.
      if ( colorChannels[0] == 1 )
        {
        float tmpArray2[32768];
        grayFunc[0]->GetTable( scalarRange[0][0], scalarRange[0][1], 
                               this->TableSize[0], tmpArray2 );
        for ( int index = 0; index < this->TableSize[0]; index++ )
          {
          tmpArray[3*index  ] = tmpArray2[index];
          tmpArray[3*index+1] = tmpArray2[index];
          tmpArray[3*index+2] = tmpArray2[index];
          }
        }
      else
        {
        rgbFunc[0]->GetTable( scalarRange[0][0], scalarRange[0][1], 
                           this->TableSize[0], tmpArray );
        }
      
      // Convert color to short format
      for ( i = 0; i < this->TableSize[0]; i++ )
        {
        this->ColorTable[0][3*i  ] = 
          static_cast<unsigned short>(tmpArray[3*i  ]*VTKKW_FP_SCALE + 0.5);
        this->ColorTable[0][3*i+1] = 
          static_cast<unsigned short>(tmpArray[3*i+1]*VTKKW_FP_SCALE + 0.5);
        this->ColorTable[0][3*i+2] = 
          static_cast<unsigned short>(tmpArray[3*i+2]*VTKKW_FP_SCALE + 0.5);
        }
      }
    
    // The opacity table is indexed with the last component
    scalarOpacityFunc[0]->GetTable( scalarRange[components-1][0], scalarRange[components-1][1], 
                                    this->TableSize[components-1], tmpArray );
      
    // Correct the opacity array for the spacing between the planes if we are
    // using a composite blending operation
    if ( this->BlendMode == vtkVolumeMapper::COMPOSITE_BLEND )
      {
      float *ptr = tmpArray; 
      double factor = 
        this->SampleDistance / vol->GetProperty()->GetScalarOpacityUnitDistance(); 
      for ( i = 0; i < this->TableSize[components-1]; i++ )
        {
        if ( *ptr > 0.0001 )
          {
          *ptr =  1.0-pow((double)(1.0-(*ptr)),factor);
          }
        ptr++;
        }
      }
      
      // Convert tables to short format
      for ( i = 0; i < this->TableSize[components-1]; i++ )
        {
        this->ScalarOpacityTable[0][i] = 
          static_cast<unsigned short>(tmpArray[i]*VTKKW_FP_SCALE + 0.5);
        }
      
      gradientOpacityFunc[0]->GetTable( 0, (scalarRange[components-1][1] - scalarRange[components-1][0])*0.25, 
                                        256, tmpArray );
      
      for ( i = 0; i < 256; i++ )
        {
        this->GradientOpacityTable[0][i] = 
          static_cast<unsigned short>(tmpArray[i]*VTKKW_FP_SCALE + 0.5);
        }    
    }
  
  return 1;
}


int vtkFixedPointVolumeRayCastMapper::ShouldUseNearestNeighborInterpolation( vtkVolume *vol )
{
//  return ( this->UseShortCuts ||
//           vol->GetProperty()->GetInterpolationType() == VTK_NEAREST_INTERPOLATION );
  return ( vol->GetProperty()->GetInterpolationType() == VTK_NEAREST_INTERPOLATION );
}

// Print method for vtkFixedPointVolumeRayCastMapper
void vtkFixedPointVolumeRayCastMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Sample Distance: " << this->SampleDistance << "\n";
  os << indent << "Image Sample Distance: " 
     << this->ImageSampleDistance << "\n";
  os << indent << "Minimum Image Sample Distance: " 
     << this->MinimumImageSampleDistance << "\n";
  os << indent << "Maximum Image Sample Distance: " 
     << this->MaximumImageSampleDistance << "\n";
  os << indent << "Auto Adjust Sample Distances: " 
     << this->AutoAdjustSampleDistances << "\n";
  os << indent << "Intermix Intersecting Geometry: "
    << (this->IntermixIntersectingGeometry ? "On\n" : "Off\n");
  
  os << indent << "Number Of Threads: " << this->NumberOfThreads << "\n";
  
  os << indent << "ShadingRequired: " << this->ShadingRequired << endl;
  os << indent << "GradientOpacityRequired: " << this->GradientOpacityRequired
     << endl;
  
  os << indent << "ImageInUseSize: " << this->ImageInUseSize[0] << " "
     << this->ImageInUseSize[1] << endl;
  os << indent << "ImageMemorySize: " << this->ImageMemorySize[0] << " "
     << this->ImageMemorySize[1] << endl;
  os << indent << "ImageViewportSize: " << this->ImageViewportSize[0] << " "
     << this->ImageViewportSize[1] << endl;
  os << indent << "ImageOrigin: " << this->ImageOrigin[0] << " "
     << this->ImageOrigin[1] << endl;
  
  os << indent << "RenderWindow: " << this->RenderWindow << endl;
  
  os << indent << "CompositeHelper: " << this->CompositeHelper << endl;
  os << indent << "CompositeShadeHelper: " << this->CompositeShadeHelper
     << endl;
  os << indent << "CompositeGOHelper: " << this->CompositeGOHelper << endl;
  os << indent << "CompositeGOShadeHelper: " << this->CompositeGOShadeHelper
     << endl;
  os << indent << "MIPHelper: " << this->MIPHelper << endl;
  
  os << indent << "TableShift: " << this->TableShift[0] << " "
     << this->TableShift[1] << " " << this->TableShift[2] << " "
     << this->TableShift[3] << endl;
  os << indent << "TableScale: " << this->TableScale[0] << " "
     << this->TableScale[1] << " " << this->TableScale[2] << " "
     << this->TableScale[3] << endl;
}
