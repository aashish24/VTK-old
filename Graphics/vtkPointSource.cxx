/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Library. No part of this file or its 
contents may be copied, reproduced or altered in any way without the express
written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
#include <math.h>
#include "PointSrc.hh"
#include "vlMath.hh"

vlPointSource::vlPointSource(int numPts)
{
  this->NumberOfPoints = (numPts > 0 ? numPts : 10);

  this->Center[0] = 0.0;
  this->Center[1] = 0.0;
  this->Center[2] = 0.0;

  this->Radius = 0.5;
}

void vlPointSource::Execute()
{
  int i;
  float radius, theta, phi, x[3], rho;
  vlFloatPoints *newPoints;
  vlCellArray *newVerts;
  vlMath math;
  int pts[1];

  this->Initialize();

  newPoints = new vlFloatPoints(this->NumberOfPoints);
  newVerts = new vlCellArray;
  newVerts->Initialize(newVerts->EstimateSize(1,this->NumberOfPoints));

  newVerts->InsertNextCell(this->NumberOfPoints);
  for (i=0; i<this->NumberOfPoints; i++)
    {
    phi = math.Pi() * math.Random();
    rho = this->Radius * math.Random();
    radius = rho * sin((double)phi);
    theta = 2.0*math.Pi() * math.Random();
    x[0] = this->Center[0] + radius * cos((double)theta);
    x[1] = this->Center[1] + radius * sin((double)theta);
    x[2] = this->Center[2] + rho * cos((double)phi);
    newVerts->InsertCellPoint(newPoints->InsertNextPoint(x));
    }
//
// Update ourselves
//
  this->SetPoints(newPoints);
  this->SetVerts(newVerts);
}

void vlPointSource::PrintSelf(ostream& os, vlIndent indent)
{
  if (this->ShouldIPrint(vlPointSource::GetClassName()))
    {
    vlPolySource::PrintSelf(os,indent);

    os << indent << "Number Of Points: " << this->NumberOfPoints << "\n";
    os << indent << "Radius: " << this->Radius << "\n";
    os << indent << "Center: (" << this->Center[0] << ", "
                                  << this->Center[1] << ", "
                                  << this->Center[2] << ")\n";

    }
}
