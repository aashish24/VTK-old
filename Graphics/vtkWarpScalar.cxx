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
#include "WarpScal.hh"

void vtkWarpScalar::Execute()
{
  vtkPoints *inPts;
  vtkNormals *inNormals;
  vtkScalars *inScalars;
  vtkFloatPoints *newPts;
  vtkPointData *pd;
  int i, ptId;
  float *x, *n, s, newX[3];
  vtkPointSet *input=(vtkPointSet *)this->Input;
  
  vtkDebugMacro(<<"Warping data with scalars");
  this->Initialize();

  inPts = input->GetPoints();
  pd = input->GetPointData();
  inNormals = pd->GetNormals();
  inScalars = pd->GetScalars();

  if ( !inNormals || !inPts || !inScalars )
    {
    vtkErrorMacro(<<"No input data");
    return;
    }

  newPts = new vtkFloatPoints(inPts->GetNumberOfPoints());
//
// Loop over all points, adjusting locations
//
  for (ptId=0; ptId < inPts->GetNumberOfPoints(); ptId++)
    {
    x = inPts->GetPoint(ptId);
    n = inNormals->GetNormal(ptId);
    s = inScalars->GetScalar(ptId);
    for (i=0; i<3; i++)
      {
      newX[i] = x[i] + this->ScaleFactor * s * n[i];
      }
    newPts->SetPoint(ptId, newX);
    }
//
// Update ourselves
//
  this->PointData.CopyNormalsOff(); // distorted geometry - normals are bad
  this->PointData.PassData(input->GetPointData());

  this->SetPoints(newPts);
}

void vtkWarpScalar::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPointSetToPointSetFilter::PrintSelf(os,indent);

  os << indent << "Scale Factor: " << this->ScaleFactor << "\n";
}
