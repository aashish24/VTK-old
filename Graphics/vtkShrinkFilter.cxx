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
// Methods for shrink filter
//
#include "ShrinkF.hh"

//
// Shrink cells towards their centroid
//
void vlShrinkFilter::Execute()
{
  vlIdList ptIds(MAX_CELL_SIZE), newPtIds(MAX_CELL_SIZE);
  vlFloatPoints *newPts;
  int i, j, cellId, numCells, numPts;
  int oldId, newId;
  float center[3], *p, pt[3];
  vlPointData *pd;

  vlDebugMacro(<<"Shrinking cells");
  this->Initialize();

  if ( (numCells=this->Input->GetNumberOfCells()) < 1 ||
  (numPts = this->Input->GetNumberOfPoints()) < 1 )
    {
    vlErrorMacro(<<"No data to shrink!");
    return;
    }

  newPts = new vlFloatPoints(numPts*8,numPts);
  pd = this->Input->GetPointData();
  this->PointData.CopyAllocate(pd,numPts*8,numPts);
//
// Traverse all cells, obtaining node coordinates.  Compute "center" of cell,
// then create new vertices shrunk towards center.
//
  for (cellId=0; cellId < numCells; i++)
    {
    this->Input->GetCellPoints(cellId,ptIds);

    // get the center of the cell
    center[0] = center[1] = center[2] = 0.0;
    for (i=0; i < ptIds.GetNumberOfIds(); i++)
      {
      p = this->Input->GetPoint(i);
      for (j=0; j < 3; j++) center[j] += p[j];
      }
    for (j=0; j<3; j++) center[j] /= ptIds.GetNumberOfIds();

    // Create new points and cells
    for (i=0; i < ptIds.GetNumberOfIds(); i++)
      {
      p = this->Input->GetPoint(ptIds.GetId(i));
      for (j=0; j < 3; j++)
        pt[j] = center[j] + this->ShrinkFactor*(p[j] - center[j]);

      oldId = ptIds.GetId(i);
      newId = newPts->InsertNextPoint(pt);
      newPtIds.SetId(i,newId);

      this->InsertNextCell(this->Input->GetCellType(cellId), newPtIds);
      
      this->PointData.CopyData(pd, oldId, newId);
      }
    }
//
// Update ourselves
//
  
  newPts->Squeeze();
  this->SetPoints(newPts);

  this->PointData.Squeeze();
}

void vlShrinkFilter::PrintSelf(ostream& os, vlIndent indent)
{
  if (this->ShouldIPrint(vlShrinkFilter::GetClassName()))
    {
    vlDataSetToUnstructuredGridFilter::PrintSelf(os,indent);

    os << indent << "Shrink Factor: " << this->ShrinkFactor << "\n";
    }
}
