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
#include "SGOutlF.hh"

void vlStructuredGridOutlineFilter::Execute()
{
  vlStructuredGrid *input;
  vlPoints *inPts;
  int i, j, k;
  int idx, gridIdx;
  vlPointData *pd;
  int *dim, *pts;
  vlFloatPoints *newPts;
  vlCellArray *newLines;

  vlDebugMacro(<< "Creating structured outline");
  this->Initialize();

  if ( (inPts=input->GetPoints()) == NULL )
    {
    vlErrorMacro("No input!");
    return;
    }
  pd = input->GetPointData();
  dim = input->GetDimensions();
//
//  Allocate storage for lines and points
//
  newPts = new vlFloatPoints(4*(dim[0]+dim[1]+dim[2]));
  newLines = new vlCellArray;
  newLines->Allocate(newLines->EstimateSize(4*((dim[0]-1)+(dim[1]-1)+(dim[2]-1)),2));
//
//  Load data
//  x-data
//
  for (idx=j=0; j<4; j++) 
    {
    if ( j == 0 )
      gridIdx = 0;
    else if ( j == 1)
      gridIdx = (dim[1] - 1)*dim[0];
    else if ( j == 2)
      gridIdx = (dim[1] - 1)*dim[0] + (dim[2] - 1)*dim[0]*dim[1];
    else
      gridIdx = (dim[2] - 1)*dim[0]*dim[1];

    for (i=0; i<dim[0]; i++)
      newPts->InsertNextPoint(inPts->GetPoint(gridIdx+i));

    }
//
//  y-data
//
  for (j=0; j<4; j++) 
    {
    if ( j == 0 )
      gridIdx = 0;
    else if ( j == 1)
      gridIdx = dim[0] - 1;
    else if ( j == 2)
      gridIdx = (dim[0] - 1) + (dim[2]-1)*dim[0]*dim[1];
    else
      gridIdx = (dim[2] - 1)*dim[0]*dim[1];

    for (i=0; i<dim[0]; i++)
      newPts->InsertNextPoint(inPts->GetPoint(gridIdx+i*dim[0]));

    }
//
//  z-data
//
  idx = dim[0]*dim[1];
  for (j=0; j<4; j++) 
    {
    if ( j == 0 )
      gridIdx = 0;
    else if ( j == 1)
      gridIdx = (dim[0] - 1);
    else if ( j == 2)
      gridIdx = (dim[0] - 1) + (dim[1]-1)*dim[0];
    else
      gridIdx = (dim[1] - 1)*dim[0];
        
    for (i=0; i<dim[0]; i++)
      newPts->InsertNextPoint(inPts->GetPoint(gridIdx+i*idx));

    }
//
// Create lines. Rely on the fact that x, then y, then z points have been 
// created.
//
  idx = 0;
  for (k=0; k < 3; k++)
    {
    for (i=0;i<4;i++)
      {
      idx++;
      for (j=0;j<(dim[k]-1);j++) 
        {
        pts[0] = idx++;
        pts[1] = idx;
        newLines->InsertNextCell(2,pts);
        }
      }
    }
//
// Update selves
//
  this->SetPoints(newPts);
  this->SetLines(newLines);

}
