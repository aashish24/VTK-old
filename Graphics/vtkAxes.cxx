/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Toolkit. No part of this file
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
#include "Axes.hh"
#include "FScalars.hh"
#include "FNormals.hh"

// Description:
// Construct with origin=(0,0,0) and scale factor=1.
vtkAxes::vtkAxes()
{
  this->Origin[0] = 0.0;  
  this->Origin[1] = 0.0;  
  this->Origin[2] = 0.0;

  this->ScaleFactor = 1.0;
}

void vtkAxes::Execute()
{
  int numPts=6, numLines=3;
  vtkFloatPoints *newPts;
  vtkCellArray *newLines;
  vtkFloatScalars *newScalars;
  vtkFloatNormals *newNormals;
  float x[3], n[3];
  int ptIds[2];

  this->Initialize();

  newPts = new vtkFloatPoints(numPts);
  newLines = new vtkCellArray();
  newLines->Allocate(newLines->EstimateSize(numLines,2));
  newScalars = new vtkFloatScalars(numPts);
  newNormals = new vtkFloatNormals(numPts);
//
// Create axes
//
  n[0] = 0.0; n[1] = 1.0; n[2] = 0.0; 
  ptIds[0] = newPts->InsertNextPoint(this->Origin);
  newScalars->InsertNextScalar(0.0);
  newNormals->InsertNextNormal(n);

  x[0] = this->Origin[0] + this->ScaleFactor;
  x[1] = this->Origin[1];
  x[2] = this->Origin[2];
  ptIds[1] = newPts->InsertNextPoint(x);
  newLines->InsertNextCell(2,ptIds);
  newScalars->InsertNextScalar(0.0);
  newNormals->InsertNextNormal(n);


  n[0] = 0.0; n[1] = 0.0; n[2] = 1.0; 
  ptIds[0] = newPts->InsertNextPoint(this->Origin);
  newScalars->InsertNextScalar(0.25);
  newNormals->InsertNextNormal(n);

  x[0] = this->Origin[0];
  x[1] = this->Origin[1] + this->ScaleFactor;
  x[2] = this->Origin[2];
  ptIds[1] = newPts->InsertNextPoint(x);
  newScalars->InsertNextScalar(0.25);
  newNormals->InsertNextNormal(n);
  newLines->InsertNextCell(2,ptIds);


  n[0] = 1.0; n[1] = 0.0; n[2] = 0.0; 
  ptIds[0] = newPts->InsertNextPoint(this->Origin);
  newScalars->InsertNextScalar(0.5);
  newNormals->InsertNextNormal(n);

  x[0] = this->Origin[0];
  x[1] = this->Origin[1];
  x[2] = this->Origin[2] + this->ScaleFactor;
  ptIds[1] = newPts->InsertNextPoint(x);
  newScalars->InsertNextScalar(0.5);
  newNormals->InsertNextNormal(n);
  newLines->InsertNextCell(2,ptIds);
//
// Update self and release memory
// 
  this->SetPoints(newPts);
  newPts->Delete();

  this->PointData.SetScalars(newScalars);
  newScalars->Delete();

  this->PointData.SetNormals(newNormals);
  newNormals->Delete();

  this->SetLines(newLines);
  newLines->Delete();
}

void vtkAxes::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolySource::PrintSelf(os,indent);

  os << indent << "Origin: (" << this->Origin[0] << ", "
               << this->Origin[1] << ", "
               << this->Origin[2] << ")\n";

  os << indent << "Scale Factor: " << this->ScaleFactor << "\n";
}
