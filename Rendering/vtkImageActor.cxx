/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkImageActor.h"
#include "vtkGraphicsFactory.h"
#include "vtkRenderer.h"

vtkCxxRevisionMacro(vtkImageActor, "$Revision$");

vtkImageActor* vtkImageActor::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkGraphicsFactory::CreateInstance("vtkImageActor");
  return (vtkImageActor*)ret;
}

vtkImageActor::vtkImageActor()
{
  this->Input = NULL;
  this->Interpolate = 1;
  this->DisplayExtent[0] = -1;
  this->DisplayExtent[1] = 0;
  this->DisplayExtent[2] = 0;
  this->DisplayExtent[3] = 0;
  this->DisplayExtent[4] = 0;
  this->DisplayExtent[5] = 0;  

  this->Bounds[0] = this->Bounds[2] = this->Bounds[4] = -1.0;
  this->Bounds[1] = this->Bounds[3] = this->Bounds[5] = 1.0;
}

vtkImageActor::~vtkImageActor()
{
  if (this->Input)
    {
    this->Input->RemoveConsumer(this);
    this->GetInput()->UnRegister(this);
    this->Input = NULL;
    }
}

int vtkImageActor::GetSliceNumber()
{
  // find the first axis with a one pixel extent and return
  // its value
  if (this->DisplayExtent[0] == this->DisplayExtent[1])
    {
    return this->DisplayExtent[0];
    }
  if (this->DisplayExtent[2] == this->DisplayExtent[3])
    {
    return this->DisplayExtent[2];
    }
  return this->DisplayExtent[4];
}

//----------------------------------------------------------------------------
void vtkImageActor::SetDisplayExtent(int extent[6])
{
  int idx, modified = 0;
  
  for (idx = 0; idx < 6; ++idx)
    {
    if (this->DisplayExtent[idx] != extent[idx])
      {
      this->DisplayExtent[idx] = extent[idx];
      modified = 1;
      }
    }

  if (modified)
    {
    this->Modified();
    }
}
//----------------------------------------------------------------------------
void vtkImageActor::SetDisplayExtent(int minX, int maxX, 
                                     int minY, int maxY,
                                     int minZ, int maxZ)
{
  int extent[6];
  
  extent[0] = minX;  extent[1] = maxX;
  extent[2] = minY;  extent[3] = maxY;
  extent[4] = minZ;  extent[5] = maxZ;
  this->SetDisplayExtent(extent);
}


//----------------------------------------------------------------------------
void vtkImageActor::GetDisplayExtent(int extent[6])
{
  int idx;
  
  for (idx = 0; idx < 6; ++idx)
    {
    extent[idx] = this->DisplayExtent[idx];
    }
}


// Renders an actor2D's property and then it's mapper.
int vtkImageActor::RenderOpaqueGeometry(vtkViewport* viewport)
{
  vtkDebugMacro(<< "vtkImageActor::RenderOpaqueGeometry");

  vtkImageData *input = this->GetInput();
  if (!input)
    {
    return 0;
    }
  // make sure the data is available
  input->UpdateInformation();

  // if the display extent has not been set, then compute one
  int *wExtent = input->GetWholeExtent();
  if (this->DisplayExtent[0] == -1)
    {
    this->DisplayExtent[0] = wExtent[0];
    this->DisplayExtent[1] = wExtent[1];
    this->DisplayExtent[2] = wExtent[2];
    this->DisplayExtent[3] = wExtent[3];
    this->DisplayExtent[4] = wExtent[4];
    this->DisplayExtent[5] = wExtent[4];
    }
  input->SetUpdateExtent(this->DisplayExtent);
  input->PropagateUpdateExtent();
  input->UpdateData();

  // render the texture map
  if ( input->GetScalarType() == VTK_UNSIGNED_CHAR )
    {
    this->Load(vtkRenderer::SafeDownCast(viewport));
    return 1;
    }
  else
    {
    vtkErrorMacro(<<"This filter requires unsigned char scalars as input");
    return 0;
    }
}

// Get the bounds for this Volume as (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax).
float *vtkImageActor::GetBounds()
{
  if (!this->Input)
    {
    return this->Bounds;
    }
  this->Input->UpdateInformation();
  float *spacing = this->Input->GetSpacing();
  float *origin = this->Input->GetOrigin();

  // if the display extent has not been set, then compute one
  int *wExtent = this->Input->GetWholeExtent();
  if (this->DisplayExtent[0] == -1)
    {
    this->DisplayExtent[0] = wExtent[0];
    this->DisplayExtent[1] = wExtent[1];
    this->DisplayExtent[2] = wExtent[2];
    this->DisplayExtent[3] = wExtent[3];
    this->DisplayExtent[4] = wExtent[4];
    this->DisplayExtent[5] = wExtent[4];
    }
  this->Bounds[0] = this->DisplayExtent[0]*spacing[0] + origin[0];
  this->Bounds[1] = this->DisplayExtent[1]*spacing[0] + origin[0];
  this->Bounds[2] = this->DisplayExtent[2]*spacing[1] + origin[1];
  this->Bounds[3] = this->DisplayExtent[3]*spacing[1] + origin[1];
  this->Bounds[4] = this->DisplayExtent[4]*spacing[2] + origin[2];
  this->Bounds[5] = this->DisplayExtent[5]*spacing[2] + origin[2];
  
  return this->Bounds;
}

// Get the bounds for this Prop3D as (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax).
void vtkImageActor::GetBounds(float bounds[6])
{
  this->GetBounds();
  for (int i=0; i<6; i++)
    {
    bounds[i] = this->Bounds[i];
    }
}

void vtkImageActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Input: " << this->Input << "\n";
  os << indent << "Interpolate: " << (this->Interpolate ? "On\n" : "Off\n");

  int idx;  
  os << indent << "DisplayExtent: (" << this->DisplayExtent[0];
  for (idx = 1; idx < 6; ++idx)
    {
    os << ", " << this->DisplayExtent[idx];
    }
  os << ")\n";
}

//----------------------------------------------------------------------------
int vtkImageActor::GetWholeZMin()
{
  int *extent;
  
  if ( ! this->GetInput())
    {
    return 0;
    }
  this->GetInput()->UpdateInformation();
  extent = this->GetInput()->GetWholeExtent();
  return extent[4];
}

//----------------------------------------------------------------------------
int vtkImageActor::GetWholeZMax()
{
  int *extent;
  
  if ( ! this->GetInput())
    {
    return 0;
    }
  this->GetInput()->UpdateInformation();
  extent = this->GetInput()->GetWholeExtent();
  return extent[5];
}

void vtkImageActor::SetInput(vtkImageData *args)
{
  vtkDebugMacro(<< this->GetClassName() << " (" << this         
  << "): setting Input to " << args );     
  if (this->Input != args)                                       
    {                                                           
    if (this->Input != NULL) 
      { 
      this->Input->RemoveConsumer(this);
      this->Input->UnRegister(this); 
      }   
    this->Input = args;                                          
    if (this->Input != NULL) 
      { 
      this->Input->Register(this); 
      this->Input->AddConsumer(this);
      }     
    this->Modified();                                           
    }                                                           
}

