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
#include "vtkHandleRepresentation.h"
#include "vtkCoordinate.h"
#include "vtkInteractorObserver.h"
#include "vtkObjectFactory.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"


vtkCxxRevisionMacro(vtkHandleRepresentation, "$Revision$");

//----------------------------------------------------------------------
vtkHandleRepresentation::vtkHandleRepresentation()
{
  // Positions are maintained via a vtkCoordinate
  this->DisplayPosition = vtkCoordinate::New();
  this->DisplayPosition->SetCoordinateSystemToDisplay();

  this->WorldPosition = vtkCoordinate::New();
  this->WorldPosition->SetCoordinateSystemToWorld();

  this->InteractionState = vtkHandleRepresentation::Outside;
  this->Tolerance = 15;
  this->ActiveRepresentation = 0;
  this->Constrained = 0;

  this->DisplayPositionTime.Modified();
  this->WorldPositionTime.Modified();
}

//----------------------------------------------------------------------
vtkHandleRepresentation::~vtkHandleRepresentation()
{
  this->DisplayPosition->Delete();
  this->WorldPosition->Delete();
}

//----------------------------------------------------------------------
void vtkHandleRepresentation::SetDisplayPosition(double pos[3])
{
  this->DisplayPosition->SetValue(pos);
  if ( this->Renderer )
    {
    double *p = this->DisplayPosition->GetComputedWorldValue(this->Renderer);
    this->WorldPosition->SetValue(p[0], p[1], p[2]);
    }
  this->DisplayPositionTime.Modified();
}

//----------------------------------------------------------------------
void vtkHandleRepresentation::GetDisplayPosition(double pos[3])
{
  // The world position maintains the position
  if ( this->Renderer && (this->WorldPositionTime > this->DisplayPositionTime) )
    {
    int *p = this->WorldPosition->GetComputedDisplayValue(this->Renderer);
    this->DisplayPosition->SetValue(p[0],p[1],0.0);
    }
  this->DisplayPosition->GetValue(pos);
}

//----------------------------------------------------------------------
double* vtkHandleRepresentation::GetDisplayPosition()
{
  return this->DisplayPosition->GetValue();
}

//----------------------------------------------------------------------
void vtkHandleRepresentation::SetWorldPosition(double pos[3])
{
  this->WorldPosition->SetValue(pos);
  this->WorldPositionTime.Modified();
}

//----------------------------------------------------------------------
void vtkHandleRepresentation::GetWorldPosition(double pos[3])
{
  this->WorldPosition->GetValue(pos);
}

double* vtkHandleRepresentation::GetWorldPosition()
{
  return this->WorldPosition->GetValue();
}

//----------------------------------------------------------------------
void vtkHandleRepresentation::SetRenderer(vtkRenderer *ren)
{
  this->DisplayPosition->SetViewport(ren);
  this->WorldPosition->SetViewport(ren);
  this->Superclass::SetRenderer(ren);
}

//----------------------------------------------------------------------
void vtkHandleRepresentation::ShallowCopy(vtkProp *prop)
{
  vtkHandleRepresentation *rep = vtkHandleRepresentation::SafeDownCast(prop);
  if ( rep )
    {
    this->SetTolerance(rep->GetTolerance());
    this->SetActiveRepresentation(rep->GetActiveRepresentation());
    this->SetConstrained(rep->GetConstrained());
    }
  this->Superclass::ShallowCopy(prop);
}

//----------------------------------------------------------------------
unsigned long vtkHandleRepresentation::GetMTime()
{
  unsigned long mTime=this->Superclass::GetMTime();
  unsigned long wMTime=this->WorldPosition->GetMTime();
  mTime = ( wMTime > mTime ? wMTime : mTime );
  unsigned long dMTime=this->DisplayPosition->GetMTime();
  mTime = ( dMTime > mTime ? dMTime : mTime );
  
  return mTime;
}

//----------------------------------------------------------------------
void vtkHandleRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os,indent);
  
  double p[3];
  this->GetDisplayPosition(p);
  os << indent << "Display Position: (" << p[0] << ", "
               << p[1] << ", " << p[2] << ")\n";

  this->GetWorldPosition(p);
  os << indent << "World Position: (" << p[0] << ", "
               << p[1] << ", " << p[2] << ")\n";

  os << indent << "Constrained: " 
     << (this->Constrained ? "On" : "Off") << "\n";

  os << indent << "Tolerance: " << this->Tolerance << "\n";

  os << indent << "Active Representation: " 
     << (this->ActiveRepresentation ? "On" : "Off") << "\n";
}
