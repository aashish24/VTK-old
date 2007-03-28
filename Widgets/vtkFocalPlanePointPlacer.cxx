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
#include "vtkFocalPlanePointPlacer.h"

#include "vtkCamera.h"
#include "vtkObjectFactory.h"
#include "vtkMath.h"
#include "vtkPlane.h"
#include "vtkPlanes.h"
#include "vtkPlaneCollection.h"
#include "vtkRenderer.h"

vtkCxxRevisionMacro(vtkFocalPlanePointPlacer, "$Revision$");
vtkStandardNewMacro(vtkFocalPlanePointPlacer);


//----------------------------------------------------------------------
vtkFocalPlanePointPlacer::vtkFocalPlanePointPlacer()
{
  this->PointBounds[0] = this->PointBounds[2] = this->PointBounds[4] = 0;
  this->PointBounds[1] = this->PointBounds[3] = this->PointBounds[5] = -1;
}

//----------------------------------------------------------------------
vtkFocalPlanePointPlacer::~vtkFocalPlanePointPlacer()
{
}

//----------------------------------------------------------------------
int vtkFocalPlanePointPlacer::ComputeWorldPosition( vtkRenderer *ren,
                                                    double displayPos[2],
                                                    double worldPos[3],
                                                    double worldOrient[9] )
{
  double fp[4];
  ren->GetActiveCamera()->GetFocalPoint(fp);
  fp[3] = 1.0;
  ren->SetWorldPoint(fp);
  ren->WorldToDisplay();
  ren->GetDisplayPoint(fp);
  
  double tmp[4];
  tmp[0] = displayPos[0];
  tmp[1] = displayPos[1];
  tmp[2] = fp[2];
  ren->SetDisplayPoint(tmp);
  ren->DisplayToWorld();
  ren->GetWorldPoint(tmp);
  
  double tolerance[3] = { 1e-12, 1e-12, 1e-12 };
  if ( this->PointBounds[0] < this->PointBounds[1] && 
      !(vtkMath::PointIsWithinBounds( tmp, this->PointBounds, tolerance )))
    {
    return 0;
    }

  worldPos[0] = tmp[0];
  worldPos[1] = tmp[1];
  worldPos[2] = tmp[2];
  
  this->GetCurrentOrientation( worldOrient );
  
  return 1;
}

//----------------------------------------------------------------------
int vtkFocalPlanePointPlacer::ComputeWorldPosition( vtkRenderer *ren,
                                                    double displayPos[2],
                                                    double refWorldPos[3],
                                                    double worldPos[3],
                                                    double worldOrient[9] )
{
  double tmp[4];
  tmp[0] = refWorldPos[0];
  tmp[1] = refWorldPos[1];
  tmp[2] = refWorldPos[2];
  tmp[3] = 1.0;
  ren->SetWorldPoint(tmp);
  ren->WorldToDisplay();
  ren->GetDisplayPoint(tmp);
  
  tmp[0] = displayPos[0];
  tmp[1] = displayPos[1];
  tmp[3] = 1.0;
  ren->SetDisplayPoint(tmp);
  ren->DisplayToWorld();
  ren->GetWorldPoint(tmp);
  
  double tolerance[3] = { 1e-12, 1e-12, 1e-12 };
  if ( this->PointBounds[0] < this->PointBounds[1] && 
      !(vtkMath::PointIsWithinBounds( tmp, this->PointBounds, tolerance )))
    {
    return 0;
    }

  worldPos[0] = tmp[0];
  worldPos[1] = tmp[1];
  worldPos[2] = tmp[2];

  this->GetCurrentOrientation( worldOrient );
  
  return 1;
}

//----------------------------------------------------------------------
int vtkFocalPlanePointPlacer::ValidateWorldPosition( double* worldPos )
{
  double tolerance[3] = { 1e-12, 1e-12, 1e-12 };
  if ( this->PointBounds[0] < this->PointBounds[1] && 
    !(vtkMath::PointIsWithinBounds( worldPos, this->PointBounds, tolerance )))
    {
    return 0;
    }

  return 1;
}

//----------------------------------------------------------------------
int vtkFocalPlanePointPlacer::ValidateWorldPosition( double* worldPos,
                                                     double* vtkNotUsed(worldOrient) )
{
  double tolerance[3] = { 1e-12, 1e-12, 1e-12 };
  if ( this->PointBounds[0] < this->PointBounds[1] && 
    !(vtkMath::PointIsWithinBounds( worldPos, this->PointBounds, tolerance )))
    {
    return 0;
    }

  return 1;
}

//----------------------------------------------------------------------
void vtkFocalPlanePointPlacer::GetCurrentOrientation( double worldOrient[9] )
{
  double *x = worldOrient;
  double *y = worldOrient+3;
  double *z = worldOrient+6;
  
  x[0] = 1.0;
  x[1] = 0.0;
  x[2] = 0.0;

  y[0] = 0.0;
  y[1] = 1.0;
  y[2] = 0.0;

  z[0] = 0.0;
  z[1] = 0.0;
  z[2] = 1.0;
}

//----------------------------------------------------------------------
void vtkFocalPlanePointPlacer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "PointBounds: \n";
  os << indent << "  Xmin,Xmax: (" << 
    this->PointBounds[0] << ", " << this->PointBounds[1] << ")\n";
  os << indent << "  Ymin,Ymax: (" << 
    this->PointBounds[2] << ", " << this->PointBounds[3] << ")\n";
  os << indent << "  Zmin,Zmax: (" <<
    this->PointBounds[4] << ", " << this->PointBounds[5] << ")\n";
}
