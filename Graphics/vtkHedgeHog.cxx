/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Library. No part of this file
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
#include "HedgeHog.hh"

void vlHedgeHog::Execute()
{
  vlDataSet *input=this->Input;
  int numPts;
  vlFloatPoints *newPts;
  vlPointData *pd;
  vlVectors *inVectors;
  int i, ptId, pts[2];
  vlCellArray *newLines;
  float *x, *v;
  float newX[3];
//
// Initialize
//
  this->Initialize();

  numPts = input->GetNumberOfPoints();
  pd = input->GetPointData();
  inVectors = pd->GetVectors();
  if ( !inVectors || numPts < 1 )
    {
    vlErrorMacro(<<"No input data");
    return;
    }
  this->PointData.CopyAllocate(pd, 2*numPts);

  newPts = new vlFloatPoints(2*numPts);
  newLines = new vlCellArray;
  newLines->Allocate(newLines->EstimateSize(numPts,2));
//
// Loop over all points, creating oriented line
//
  for (ptId=0; ptId < numPts; ptId++)
    {
    x = input->GetPoint(ptId);
    v = inVectors->GetVector(ptId);
    for (i=0; i<3; i++)
      {
      newX[i] = x[i] + this->ScaleFactor * v[i];
      }

    pts[0] = ptId;
    pts[1] = ptId + numPts;;

    newPts->SetPoint(pts[0], x);
    newPts->SetPoint(pts[1], newX);

    newLines->InsertNextCell(2,pts);

    this->PointData.CopyData(pd,ptId,pts[0]);
    this->PointData.CopyData(pd,ptId,pts[1]);
    }
//
// Update ourselves
//
  this->SetPoints(newPts);
  this->SetLines(newLines);
}

void vlHedgeHog::PrintSelf(ostream& os, vlIndent indent)
{
  vlDataSetToPolyFilter::PrintSelf(os,indent);

  os << indent << "Scale Factor: " << this->ScaleFactor << "\n";
}
