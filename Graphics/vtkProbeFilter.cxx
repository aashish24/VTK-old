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
#include "ProbeF.hh"

vlProbeFilter::vlProbeFilter()
{
  this->Source = NULL;
}

void vlProbeFilter::Execute()
{
  int cellId, ptId;
  float *x, tol2;
  vlCell *cell;
  vlPointData *pd;
  int numPts, subId;
  float pcoords[3], weights[MAX_CELL_SIZE];

  vlDebugMacro(<<"Probing data");
  this->Initialize();

  pd = this->Input->GetPointData();
  numPts = this->Input->GetNumberOfPoints();
//
// Allocate storage for output PointData
//
  this->PointData.InterpolateAllocate(pd);
//
// Use tolerance as a function of size of input data
//
  tol2 = this->Input->GetLength();
  tol2 = tol2*tol2 / 1000.0;
//
// Loop over all source points, interpolating input data
//
  for (ptId=0; ptId < this->Source->GetNumberOfPoints(); ptId++)
    {
    x = this->Source->GetPoint(ptId);
    cellId = this->Input->FindCell(x,NULL,tol2,subId,pcoords,weights);
    if ( cellId >= 0 )
      {
      cell = this->Input->GetCell(cellId);
      this->PointData.InterpolatePoint(pd,ptId,&(cell->PointIds),weights);
      }
    else
      {
      this->PointData.NullPoint(ptId);
      }
    }
}

void vlProbeFilter::Initialize()
{
  this->Modified();
  if ( this->Source )
    {
    if (this->DataSet) delete this->DataSet;
    // copies SOURCE geometry to internal data set
    this->DataSet = this->Source->MakeObject(); 
    }
}

// Description:
// Override update method because execution can branch two ways (Input 
// and Source)
void vlProbeFilter::Update()
{
  // make sure input is available
  if ( this->Input == NULL || this->Source == NULL )
    {
    vlErrorMacro(<< "No input!");
    return;
    }

  // prevent chasing our tail
  if (this->Updating) return;

  this->Updating = 1;
  this->Input->Update();
  this->Source->Update();
  this->Updating = 0;

  if (this->Input->GetMTime() > this->GetMTime() || 
  this->Source->GetMTime() > this->GetMTime() || 
  this->GetMTime() > this->ExecuteTime || this->GetDataReleased() )
    {
    if ( this->StartMethod ) (*this->StartMethod)(this->StartMethodArg);
    this->Execute();
    this->ExecuteTime.Modified();
    this->SetDataReleased(0);
    if ( this->EndMethod ) (*this->EndMethod)(this->EndMethodArg);
    }

  if ( this->Input->ShouldIReleaseData() ) this->Input->ReleaseData();
  if ( this->Source->ShouldIReleaseData() ) this->Source->ReleaseData();
}

void vlProbeFilter::PrintSelf(ostream& os, vlIndent indent)
{
  vlDataSetToDataSetFilter::PrintSelf(os,indent);

  os << indent << "Source: " << this->Source << "\n";
}
