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
//
// Methods for Line generator
//
#include <math.h>
#include "LineSrc.hh"
#include "FPoints.hh"
#include "FTCoords.hh"

vlLineSource::vlLineSource(int res)
{
  this->Pt1[0] = -0.5;
  this->Pt1[1] =  0.0;
  this->Pt1[2] =  0.0;

  this->Pt2[0] =  0.5;
  this->Pt2[1] =  0.0;
  this->Pt2[2] =  0.0;

  this->Resolution = (res < 1 ? 1 : res);
}

void vlLineSource::Execute()
{
  int numLines=this->Resolution;
  int numPts=this->Resolution+1;
  float x[3], tc[2], v[3];
  int i, j;
  int pts[2];
  vlFloatPoints *newPoints; 
  vlFloatTCoords *newTCoords; 
  vlCellArray *newLines;
//
// Set things up; allocate memory
//
  this->Initialize();

  newPoints = new vlFloatPoints(numPts);
  newTCoords = new vlFloatTCoords(numPts,2);

  newLines = new vlCellArray;
  newLines->Allocate(newLines->EstimateSize(numLines,2));
//
// Generate points and texture coordinates
//
  for (i=0; i<3; i++) v[i] = this->Pt2[i] - this->Pt1[i];

  tc[1] = 0.0;
  for (i=0; i<numPts; i++) 
    {
    tc[0] = ((float)i/this->Resolution);
    for (j=0; j<3; j++) x[j] = this->Pt1[j] + tc[0]*v[j];
    newPoints->InsertPoint(i,x);
    newTCoords->InsertTCoord(i,tc);
    }
//
//  Generate lines
//
  for (i=0; i < numLines; i++) 
    {
    pts[0] = i;
    pts[1] = i+1;
    newLines->InsertNextCell(2,pts);
    }
//
// Update ourselves
//
  this->SetPoints(newPoints);
  this->PointData.SetTCoords(newTCoords);
  this->SetLines(newLines);
}

void vlLineSource::PrintSelf(ostream& os, vlIndent indent)
{
  vlPolySource::PrintSelf(os,indent);

  os << indent << "Resolution: " << this->Resolution << "\n";

  os << indent << "Point 1: (" << this->Pt1[0] << ", "
                                << this->Pt1[1] << ", "
                                << this->Pt1[2] << ")\n";

  os << indent << "Point 2: (" << this->Pt2[0] << ", "
                                << this->Pt2[1] << ", "
                                << this->Pt2[2] << ")\n";
}
