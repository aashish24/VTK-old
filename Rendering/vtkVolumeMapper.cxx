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
#include "vtkVolumeMapper.h"

#include "vtkGarbageCollector.h"
#include "vtkImageClip.h"
#include "vtkImageData.h"
#include "vtkDataSet.h"

vtkCxxRevisionMacro(vtkVolumeMapper, "$Revision$");

// Construct a vtkVolumeMapper with empty scalar input and clipping off.
vtkVolumeMapper::vtkVolumeMapper()
{
  int i;

  this->Cropping = 0;
  for ( i = 0; i < 3; i++ )
    {
    this->CroppingRegionPlanes[2*i    ]      = 0;
    this->CroppingRegionPlanes[2*i + 1]      = 1;
    this->VoxelCroppingRegionPlanes[2*i]     = 0;
    this->VoxelCroppingRegionPlanes[2*i + 1] = 1;    
    }
  this->CroppingRegionFlags = VTK_CROP_SUBVOLUME;

  this->UseImageClipper = 1;
  this->ImageClipper = vtkImageClip::New();
  this->ImageClipper->ClipDataOn();
}

vtkVolumeMapper::~vtkVolumeMapper()
{  
  if (this->ImageClipper)
    {
    this->ImageClipper->Delete();
    this->ImageClipper = 0;
    }
}

void vtkVolumeMapper::ConvertCroppingRegionPlanesToVoxels()
{
  double *spacing    = this->GetInput()->GetSpacing();
  int   *dimensions = this->GetInput()->GetDimensions();
  double origin[3];
  double *bds = this->GetInput()->GetBounds();
  origin[0] = bds[0];
  origin[1] = bds[2];
  origin[2] = bds[4];
  
  for ( int i = 0; i < 6; i++ )
    {
    this->VoxelCroppingRegionPlanes[i] =
      (this->CroppingRegionPlanes[i] - origin[i/2]) / spacing[i/2];
    
    this->VoxelCroppingRegionPlanes[i] = 
      ( this->VoxelCroppingRegionPlanes[i] < 0 ) ?
      ( 0 ) : ( this->VoxelCroppingRegionPlanes[i] );

    this->VoxelCroppingRegionPlanes[i] = 
      ( this->VoxelCroppingRegionPlanes[i] > dimensions[i/2]-1 ) ?
      ( dimensions[i/2]-1 ) : ( this->VoxelCroppingRegionPlanes[i] );
    }
}

void vtkVolumeMapper::SetUseImageClipper(int arg)
{
  if (this->UseImageClipper == arg)
    {
    return;
    }

  this->UseImageClipper = arg;
  this->Modified();

  // Force a change of the input to reconnect the pipeline correctly

  vtkImageData *input = this->GetInput();
  if (input)
    {
    input->Register(this);
    }
  this->SetInput((vtkImageData *)NULL);
  if (input)
    {
    this->SetInput(input);
    input->UnRegister(this);
    }
}

void vtkVolumeMapper::SetInput( vtkDataSet *genericInput )
{
  vtkImageData *input = 
    vtkImageData::SafeDownCast( genericInput );
  
  if ( input )
    {
    this->SetInput( input );
    }
  else
    {
    vtkErrorMacro("The SetInput method of this mapper requires vtkImageData as input");
    }
}

void vtkVolumeMapper::SetInput( vtkImageData *input )
{
  if (this->UseImageClipper)
    {
    this->ImageClipper->SetInput(input);
    this->vtkProcessObject::SetNthInput(0, this->ImageClipper->GetOutput());
    }
  else
    {
    this->vtkProcessObject::SetNthInput(0, input);
    }
}

vtkImageData *vtkVolumeMapper::GetInput()
{
  if (this->NumberOfInputs < 1)
    {
    return NULL;
    }
  return (vtkImageData *)this->Inputs[0];
}


// Print the vtkVolumeMapper
void vtkVolumeMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Cropping: " << (this->Cropping ? "On\n" : "Off\n");

  os << indent << "Cropping Region Planes: " << endl 
     << indent << "  In X: " << this->CroppingRegionPlanes[0] 
     << " to " << this->CroppingRegionPlanes[1] << endl 
     << indent << "  In Y: " << this->CroppingRegionPlanes[2] 
     << " to " << this->CroppingRegionPlanes[3] << endl 
     << indent << "  In Z: " << this->CroppingRegionPlanes[4] 
     << " to " << this->CroppingRegionPlanes[5] << endl;
 
  os << indent << "Cropping Region Flags: " 
     << this->CroppingRegionFlags << endl;

  // Don't print this->VoxelCroppingRegionPlanes
  
  os << indent << "UseImageClipper: " 
     << (this->UseImageClipper ? "On\n" : "Off\n");
}

//----------------------------------------------------------------------------
void vtkVolumeMapper::ReportReferences(vtkGarbageCollector* collector)
{
  this->Superclass::ReportReferences(collector);
#ifdef VTK_USE_EXECUTIVES
  // These filters share our input and are therefore involved in a
  // reference loop.
  if (this->UseImageClipper)
    {
    collector->ReportReference(this->ImageClipper, "ImageClipper");
    }
#endif
}

//----------------------------------------------------------------------------
void vtkVolumeMapper::RemoveReferences()
{
#ifdef VTK_USE_EXECUTIVES
  if(this->ImageClipper)
    {
    this->ImageClipper->Delete();
    this->ImageClipper = 0;
    }
#endif
  this->Superclass::RemoveReferences();
}
