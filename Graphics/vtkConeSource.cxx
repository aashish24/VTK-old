/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Toolkit. No part of this file or its 
contents may be copied, reproduced or altered in any way without the express
written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
//
// Methods for Cone generator
//
#include <math.h>
#include "ConeSrc.hh"

vtkConeSource::vtkConeSource(int res)
{
  res = (res < 0 ? 0 : res);
  this->Resolution = res;
  this->Height = 1.0;
  this->Radius = 0.5;
  this->Capping = 1;
}

void vtkConeSource::Execute()
{
  float angle= 2.0*3.141592654/this->Resolution;
  int numLines, numPolys, numPts;
  float x[3], xbot;
  int i;
  int pts[MAX_CELL_SIZE];
  vtkFloatPoints *newPoints; 
  vtkCellArray *newLines=0;
  vtkCellArray *newPolys=0;
//
// Set things up; allocate memory
//
  this->Initialize();

  switch ( this->Resolution )
  {
  case 0:
    numPts = 2;
    numLines =  1;
    newLines = new vtkCellArray;
    newLines->Allocate(newLines->EstimateSize(numLines,numPts));
  
  case 1: case 2:
    numPts = 2*this->Resolution + 1;
    numPolys = this->Resolution;
    newPolys = new vtkCellArray;
    newPolys->Allocate(newPolys->EstimateSize(numPolys,3));
    break;

  default:
    numPts = this->Resolution + 1;
    numPolys = this->Resolution + 1;
    newPolys = new vtkCellArray;
    newPolys->Allocate(newPolys->EstimateSize(numPolys,this->Resolution));
    break;
  }
  newPoints = new vtkFloatPoints(numPts);
//
// Create cone
//
  x[0] = this->Height / 2.0; // zero-centered
  x[1] = 0.0;
  x[2] = 0.0;
  pts[0] = newPoints->InsertNextPoint(x);

  xbot = -this->Height / 2.0;

  switch (this->Resolution) 
  {
  case 0:
    x[0] = xbot;
    x[1] = 0.0;
    x[2] = 0.0;
    pts[1] = newPoints->InsertNextPoint(x);
    newLines->InsertNextCell(2,pts);
    break;

  case 2:  // fall through this case to use the code in case 1
    x[0] = xbot;
    x[1] = 0.0;
    x[2] = -this->Radius;
    pts[1] = newPoints->InsertNextPoint(x);
    x[0] = xbot;
    x[1] = 0.0;
    x[2] = this->Radius;
    pts[2] = newPoints->InsertNextPoint(x);
 
    newPolys->InsertNextCell(3,pts);

  case 1:
    x[0] = xbot;
    x[1] = -this->Radius;
    x[2] = 0.0;
    pts[1] = newPoints->InsertNextPoint(x);
    x[0] = xbot;
    x[1] = this->Radius;
    x[2] = 0.0;
    pts[2] = newPoints->InsertNextPoint(x);

    newPolys->InsertNextCell(3,pts);

    break;

  default: // General case: create Resolution triangles and single cap

    for (i=0; i<this->Resolution; i++) 
      {
      x[0] = xbot;
      x[1] = this->Radius * cos ((double)i*angle);
      x[2] = this->Radius * sin ((double)i*angle);
      pts[1] = newPoints->InsertNextPoint(x);
      pts[2] = (pts[1] % this->Resolution) + 1;
      newPolys->InsertNextCell(3,pts);
      }
//
// If capping, create last polygon
//
    if ( this->Capping )
      {
      for (i=0; i<this->Resolution; i++) pts[i] = i+1;
      newPolys->InsertNextCell(this->Resolution,pts);
      }
  } //switch
//
// Update ourselves
//
  this->SetPoints(newPoints);

  if ( newPolys )
    {
    newPolys->Squeeze(); // we may have estimated size; reclaim some space
    this->SetPolys(newPolys);
    }
  else
    {
    this->SetLines(newLines);
    }
}

void vtkConeSource::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolySource::PrintSelf(os,indent);

  os << indent << "Resolution: " << this->Resolution << "\n";
  os << indent << "Height: " << this->Height << "\n";
  os << indent << "Radius: " << this->Radius << "\n";
  os << indent << "Capping: " << (this->Capping ? "On\n" : "Off\n");
}
