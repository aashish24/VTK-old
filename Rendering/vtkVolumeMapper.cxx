/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen.

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
#include "vtkVolumeMapper.h"

// Construct a vtkVolumeMapper with empty scalar input and clipping off.
vtkVolumeMapper::vtkVolumeMapper()
{
  int i;

  this->Cropping = 0;
  for ( i = 0; i < 3; i++ )
    {
    this->CroppingRegionPlanes[2*i    ] = 0;
    this->CroppingRegionPlanes[2*i + 1] = 1;
    }
  this->Bounds[0] = this->Bounds[2] = this->Bounds[4] = -1.0;
  this->Bounds[1] = this->Bounds[3] = this->Bounds[5] = 1.0;
  this->Center[0] = this->Center[1] = this->Center[2] = 0.0;
  this->CroppingRegionFlags = 0x02000;
}

vtkVolumeMapper::~vtkVolumeMapper()
{
}

void vtkVolumeMapper::Update()
{
  if ( this->GetInput() )
    {
    this->GetInput()->Update();
    }

  if ( this->GetRGBTextureInput() )
    {
    this->GetRGBTextureInput()->Update();
    }
}

// Get the bounds for the input of this mapper as 
// (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax).
float *vtkVolumeMapper::GetBounds()
{
  static float bounds[] = {-1.0,1.0, -1.0,1.0, -1.0,1.0};

  if ( ! this->GetInput() ) 
    {
    return bounds;
    }
  else
    {
    this->GetInput()->Update();
    this->GetInput()->GetBounds(this->Bounds);
    return this->Bounds;
    }
}

void vtkVolumeMapper::SetInput( vtkStructuredPoints *input )
{
  this->vtkProcessObject::SetNthInput(0, input);
}

vtkStructuredPoints *vtkVolumeMapper::GetInput()
{
  if (this->NumberOfInputs < 1)
    {
    return NULL;
    }
  return (vtkStructuredPoints*)(this->Inputs[0]);
}


void vtkVolumeMapper::SetRGBTextureInput( vtkStructuredPoints *rgbTexture )
{
  vtkPointData    *pd;
  vtkScalars      *scalars;

  if ( rgbTexture )
    {
    rgbTexture->Update();
    pd = rgbTexture->GetPointData();
    if ( !pd )
      {
      vtkErrorMacro( << "No PointData in texture!" );
      return;
      }
    scalars = pd->GetScalars();
    if ( !scalars )
      {
      vtkErrorMacro( << "No scalars in texture!" );
      return;
      }
    if ( scalars->GetDataType() != VTK_UNSIGNED_CHAR )
      {
      vtkErrorMacro( << "Scalars in texture must be unsigned char!" );
      return;      
      }
    if ( scalars->GetNumberOfComponents() != 3 )
      {
      vtkErrorMacro( << "Scalars must have 3 components (r, g, and b)" );
      return;      
      }
    }
  
  this->vtkProcessObject::SetNthInput(1, rgbTexture);

}

vtkStructuredPoints *vtkVolumeMapper::GetRGBTextureInput()
{
  if (this->NumberOfInputs < 2)
    {
    return NULL;
    }
  return (vtkStructuredPoints *)(this->Inputs[1]);
}


// Print the vtkVolumeMapper
void vtkVolumeMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkAbstractMapper3D::PrintSelf(os,indent);

  if ( this->GetRGBTextureInput() )
    {
    os << indent << "RGBTextureInput: (" << this->GetRGBTextureInput() 
       << ")\n";
    }
  else
    {
    os << indent << "RGBTextureInput: (none)\n";
    }

  os << indent << "Cropping: " << (this->Cropping ? "On\n" : "Off\n");

  os << indent << "Cropping Region Planes: " << endl 
     << indent << "  In X: " << this->CroppingRegionPlanes[0] 
     << " to " << this->CroppingRegionPlanes[1] << endl 
     << indent << "  In Y: " << this->CroppingRegionPlanes[2] 
     << " to " << this->CroppingRegionPlanes[3] << endl 
     << indent << "  In Z: " << this->CroppingRegionPlanes[4] 
     << " to " << this->CroppingRegionPlanes[5] << endl;
 
  os << indent << "Cropping Region Flags: " << this->CroppingRegionFlags << endl;

  os << indent << "Build Time: " <<this->BuildTime.GetMTime() << "\n";
}

