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
// Methods for cube generator
//
#include <math.h>
#include "CubeSrc.hh"
#include "FPoints.hh"
#include "FNormals.hh"

vtkCubeSource::vtkCubeSource(float xL, float yL, float zL)
{
  this->XLength = fabs(xL);
  this->YLength = fabs(yL);
  this->ZLength = fabs(zL);

  this->Center[0] = 0.0;
  this->Center[1] = 0.0;
  this->Center[2] = 0.0;
}

void vtkCubeSource::Execute()
{
  float x[3], n[3];
  int numPolys=6, numPts=24;
  int i, j, k;
  int pts[4];
  vtkFloatPoints *newPoints; 
  vtkFloatNormals *newNormals;
  vtkCellArray *newPolys;
//
// Set things up; allocate memory
//
  this->Initialize();

  newPoints = new vtkFloatPoints(numPts);
  newNormals = new vtkFloatNormals(numPts);

  newPolys = new vtkCellArray;
  newPolys->Allocate(newPolys->EstimateSize(numPolys,4));
//
// Generate points and normals
//
  numPts = 0;

  for (x[0]=Center[0]-this->XLength/2.0, n[0]=(-1.0), n[1]=n[2]=0.0, i=0; 
  i<2; i++, x[0]+=this->XLength, n[0]+=2.0)
    {
    for (x[1]=Center[1]-this->YLength/2.0, j=0; j<2; 
    j++, x[1]+=this->YLength)
      {
      for (x[2]=Center[2]-this->ZLength/2.0, k=0; k<2; 
      k++, x[2]+=this->ZLength)
        {
        newPoints->InsertNextPoint(x);
        newNormals->InsertNextNormal(n);
        }
      }
    }
  pts[0] = 0; pts[1] = 1; pts[2] = 3; pts[3] = 2; 
  newPolys->InsertNextCell(4,pts);
  pts[0] += 4; pts[1] +=4; pts[2] +=4; pts[3] += 4; 
  newPolys->InsertNextCell(4,pts);

  for (x[1]=Center[1]-this->YLength/2.0, n[1]=(-1.0), n[0]=n[2]=0.0, i=0; 
  i<2; i++, x[1]+=this->YLength, n[1]+=2.0)
    {
    for (x[0]=Center[0]-this->XLength/2.0, j=0; j<2; 
    j++, x[0]+=this->XLength)
      {
      for (x[2]=Center[2]-this->ZLength/2.0, k=0; k<2; 
      k++, x[2]+=this->ZLength)
        {
        newPoints->InsertNextPoint(x);
        newNormals->InsertNextNormal(n);
        }
      }
    }
  pts[0] += 4; pts[1] +=4; pts[2] +=4; pts[3] += 4; 
  newPolys->InsertNextCell(4,pts);
  pts[0] += 4; pts[1] +=4; pts[2] +=4; pts[3] += 4; 
  newPolys->InsertNextCell(4,pts);

  for (x[2]=Center[2]-this->ZLength/2.0, n[2]=(-1.0), n[0]=n[1]=0.0, i=0; 
  i<2; i++, x[2]+=this->ZLength, n[2]+=2.0)
    {
    for (x[1]=Center[1]-this->YLength/2.0, j=0; j<2; 
    j++, x[1]+=this->YLength)
      {
      for (x[0]=Center[0]-this->XLength/2.0, k=0; k<2; 
      k++, x[0]+=this->XLength)
        {
        newPoints->InsertNextPoint(x);
        newNormals->InsertNextNormal(n);
        }
      }
    }
  pts[0] += 4; pts[1] +=4; pts[2] +=4; pts[3] += 4; 
  newPolys->InsertNextCell(4,pts);
  pts[0] += 4; pts[1] +=4; pts[2] +=4; pts[3] += 4; 
  newPolys->InsertNextCell(4,pts);
//
// Update ourselves and release memory
//
  this->SetPoints(newPoints);
  newPoints->Delete();

  this->PointData.SetNormals(newNormals);
  newNormals->Delete();

  newPolys->Squeeze(); // since we've estimated size; reclaim some space
  this->SetPolys(newPolys);
  newPolys->Delete();
}

void vtkCubeSource::SetBounds(float bounds[6])
{
  this->SetXLength(bounds[1]-bounds[0]);
  this->SetYLength(bounds[3]-bounds[2]);
  this->SetZLength(bounds[5]-bounds[4]);

  this->SetCenter((bounds[1]+bounds[0])/2.0, (bounds[3]+bounds[2])/2.0, 
                  (bounds[5]+bounds[4])/2.0);
}

// Description:
// Convenience method allows creation of cube by specifying bounding box.
void vtkCubeSource::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolySource::PrintSelf(os,indent);

  os << indent << "X Length: " << this->XLength << "\n";
  os << indent << "Y Length: " << this->YLength << "\n";
  os << indent << "Z Length: " << this->ZLength << "\n";
  os << indent << "Center: (" << this->Center[0] << ", " 
               << this->Center[1] << ", " << this->Center[2] << ")\n";
}

