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
#include "MaskPoly.hh"

vlMaskPolyData::vlMaskPolyData()
{
  this->OnRatio = 11;
  this->Offset = 0;
}

//
// Down sample polygonal data.  Don't down sample points (that is, use the
// original points, since usually not worth it.
//
void vlMaskPolyData::Execute()
{
  int numVerts, numLines, numPolys, numStrips;
  vlCellArray *inVerts,*inLines,*inPolys,*inStrips;
  int numNewVerts, numNewLines, numNewPolys, numNewStrips;
  vlCellArray *newVerts=NULL, *newLines=NULL;
  vlCellArray *newPolys=NULL, *newStrips=NULL;
  int id, interval;
  vlPointData *pd;
  int npts, *pts;
  vlPolyData *input=(vlPolyData *)this->Input;
//
// Check input / pass data through
//
  this->Initialize();

  inVerts = input->GetVerts();
  numVerts = inVerts->GetNumberOfCells();
  numNewVerts = numVerts / this->OnRatio;

  inLines = input->GetLines();
  numLines = inLines->GetNumberOfCells();
  numNewLines = numLines / this->OnRatio;

  inPolys = input->GetPolys();
  numPolys = inPolys->GetNumberOfCells();
  numNewPolys = numPolys / this->OnRatio;

  inStrips = input->GetStrips();
  numStrips = inStrips->GetNumberOfCells();
  numNewStrips = numStrips / this->OnRatio;

  if ( numNewVerts < 1 && numNewLines < 1 &&
  numNewPolys < 1 && numNewStrips < 1 )
    {
    vlErrorMacro (<<"No PolyData to mask!");
    return;
    }
//
// Allocate space
//
  if ( numNewVerts )
    newVerts = new vlCellArray(numNewVerts);

  if ( numNewLines )
    {
    newLines = new vlCellArray;
    newLines->Allocate(newLines->EstimateSize(numNewLines,2));
    }

  if ( numNewPolys )
    {
    newPolys = new vlCellArray;
    newPolys->Allocate(newPolys->EstimateSize(numNewPolys,4));
    }

  if ( numNewStrips )
    {
    newStrips = new vlCellArray;
    newStrips->Allocate(newStrips->EstimateSize(numNewStrips,6));
    }
//
// Traverse topological lists and traverse
//
  interval = this->Offset + this->OnRatio;
  if ( newVerts )
    {
    for (id=0, inVerts->InitTraversal(); inVerts->GetNextCell(npts,pts); id++)
      {
      if ( ! (id % interval) )
        {
        newVerts->InsertNextCell(npts,pts);
        }
      }
    }

  if ( newLines )
    {
    for (id=0, inLines->InitTraversal(); inLines->GetNextCell(npts,pts); id++)
      {
      if ( ! (id % interval) )
        {
        newLines->InsertNextCell(npts,pts);
        }
      }
    }

  if ( newPolys )
    {
    for (id=0, inPolys->InitTraversal(); inPolys->GetNextCell(npts,pts); id++)
      {
      if ( ! (id % interval) )
        {
        newPolys->InsertNextCell(npts,pts);
        }
      }
    }

  if ( newStrips )
    {
    for (id=0, inStrips->InitTraversal(); inStrips->GetNextCell(npts,pts); id++)
      {
      if ( ! (id % interval) )
        {
        newStrips->InsertNextCell(npts,pts);
        }
      }
    }
//
// Update ourselves
//
  // pass through points and point data
  this->SetPoints(input->GetPoints());
  pd = input->GetPointData();
  this->PointData = *pd;
  this->SetVerts(newVerts);
  this->SetLines(newLines);
  this->SetPolys(newPolys);
  this->SetStrips(newStrips);
  this->Squeeze();
}

void vlMaskPolyData::PrintSelf(ostream& os, vlIndent indent)
{
  vlPolyToPolyFilter::PrintSelf(os,indent);

  os << indent << "On Ratio: " << this->OnRatio << "\n";
  os << indent << "Offset: " << this->Offset << "\n";
}
