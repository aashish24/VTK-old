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
#include "vtkAxes.h"
#include "vtkFloatArray.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkAxes, "$Revision$");
vtkStandardNewMacro(vtkAxes);

// Construct with origin=(0,0,0) and scale factor=1.
vtkAxes::vtkAxes()
{
  this->Origin[0] = 0.0;  
  this->Origin[1] = 0.0;  
  this->Origin[2] = 0.0;

  this->ScaleFactor = 1.0;
  
  this->Symmetric = 0;
  this->ComputeNormals = 1;
}

void vtkAxes::Execute()
{
  int numPts=6, numLines=3;
  vtkPoints *newPts;
  vtkCellArray *newLines;
  vtkFloatArray *newScalars;
  vtkFloatArray *newNormals;
  float x[3], n[3];
  vtkIdType ptIds[2];
  vtkPolyData *output = this->GetOutput();
  
  vtkDebugMacro(<<"Creating x-y-z axes");

  newPts = vtkPoints::New();
  newPts->Allocate(numPts);
  newLines = vtkCellArray::New();
  newLines->Allocate(newLines->EstimateSize(numLines,2));
  newScalars = vtkFloatArray::New();
  newScalars->Allocate(numPts);
  newScalars->SetName("Axes");
  newNormals = vtkFloatArray::New();
  newNormals->SetNumberOfComponents(3);
  newNormals->Allocate(numPts);
  newNormals->SetName("Normals");
  
//
// Create axes
//
  x[0] = this->Origin[0];
  x[1] = this->Origin[1];
  x[2] = this->Origin[2];
  if (this->Symmetric)
    {
    x[0] = this->Origin[0] - this->ScaleFactor;
    }
  n[0] = 0.0; n[1] = 1.0; n[2] = 0.0; 
  ptIds[0] = newPts->InsertNextPoint(x);
  newScalars->InsertNextValue(0.0);
  newNormals->InsertNextTuple(n);

  x[0] = this->Origin[0] + this->ScaleFactor;
  x[1] = this->Origin[1];
  x[2] = this->Origin[2];
  ptIds[1] = newPts->InsertNextPoint(x);
  newLines->InsertNextCell(2,ptIds);
  newScalars->InsertNextValue(0.0);
  newNormals->InsertNextTuple(n);

  x[0] = this->Origin[0];
  x[1] = this->Origin[1];
  x[2] = this->Origin[2];
  if (this->Symmetric)
    {
    x[1] = this->Origin[1] - this->ScaleFactor;
    }
  n[0] = 0.0; n[1] = 0.0; n[2] = 1.0; 
  ptIds[0] = newPts->InsertNextPoint(x);
  newScalars->InsertNextValue(0.25);
  newNormals->InsertNextTuple(n);

  x[0] = this->Origin[0];
  x[1] = this->Origin[1] + this->ScaleFactor;
  x[2] = this->Origin[2];
  ptIds[1] = newPts->InsertNextPoint(x);
  newScalars->InsertNextValue(0.25);
  newNormals->InsertNextTuple(n);
  newLines->InsertNextCell(2,ptIds);

  x[0] = this->Origin[0];
  x[1] = this->Origin[1];
  x[2] = this->Origin[2];
  if (this->Symmetric)
    {
    x[2] = this->Origin[2] - this->ScaleFactor;
    }
  n[0] = 1.0; n[1] = 0.0; n[2] = 0.0; 
  ptIds[0] = newPts->InsertNextPoint(x);
  newScalars->InsertNextValue(0.5);
  newNormals->InsertNextTuple(n);

  x[0] = this->Origin[0];
  x[1] = this->Origin[1];
  x[2] = this->Origin[2] + this->ScaleFactor;
  ptIds[1] = newPts->InsertNextPoint(x);
  newScalars->InsertNextValue(0.5);
  newNormals->InsertNextTuple(n);
  newLines->InsertNextCell(2,ptIds);

  //
  // Update our output and release memory
  // 
  output->SetPoints(newPts);
  newPts->Delete();

  output->GetPointData()->SetScalars(newScalars);
  newScalars->Delete();

  if (this->ComputeNormals)
    {
    output->GetPointData()->SetNormals(newNormals);
    }
  newNormals->Delete();

  output->SetLines(newLines);
  newLines->Delete();
}

//----------------------------------------------------------------------------
// This source does not know how to generate pieces yet.
int vtkAxes::ComputeDivisionExtents(vtkDataObject *vtkNotUsed(output),
                                      int idx, int numDivisions)
{
  if (idx == 0 && numDivisions == 1)
    {
    // I will give you the whole thing
    return 1;
    }
  else
    {
    // I have nothing to give you for this piece.
    return 0;
    }
}

void vtkAxes::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Origin: (" << this->Origin[0] << ", "
               << this->Origin[1] << ", "
               << this->Origin[2] << ")\n";
  os << indent << "Scale Factor: " << this->ScaleFactor << "\n";
  os << indent << "Symmetric: " << this->Symmetric << "\n";
  os << indent << "ComputeNormals: " << this->ComputeNormals << "\n";
}
