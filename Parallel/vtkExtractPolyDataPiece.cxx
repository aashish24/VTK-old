/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkExtractPolyDataPiece.h"

#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkGenericCell.h"
#include "vtkIdList.h"
#include "vtkOBBDicer.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkUnsignedCharArray.h"

vtkCxxRevisionMacro(vtkExtractPolyDataPiece, "$Revision$");
vtkStandardNewMacro(vtkExtractPolyDataPiece);

//=============================================================================
vtkExtractPolyDataPiece::vtkExtractPolyDataPiece()
{
  this->CreateGhostCells = 1;
}

//=============================================================================
void vtkExtractPolyDataPiece::ComputeInputUpdateExtents(vtkDataObject *out)
{
  vtkPolyData *input = this->GetInput();
  
  if (this->GetInput() == NULL)
    {
    vtkErrorMacro("Missing input");
    return;
    }

  out = out;
  input->SetUpdateExtent(0, 1, 0);
}

//=============================================================================
void vtkExtractPolyDataPiece::ExecuteInformation()
{
  if (this->GetInput() == NULL)
    {
    vtkErrorMacro("Missing input");
    return;
    }
  this->GetOutput()->SetMaximumNumberOfPieces(-1);
}

  
//=============================================================================
void vtkExtractPolyDataPiece::ComputeCellTags(vtkIntArray *tags, 
                                              vtkIdList *pointOwnership,
                                              int piece, int numPieces)
{
  vtkPolyData *input;
  vtkIdType idx, j, numCells, ptId;
  vtkIdList *cellPtIds;

  input = this->GetInput();
  numCells = input->GetNumberOfCells();
  
  cellPtIds = vtkIdList::New();
  // Clear Point ownership.
  for (idx = 0; idx < input->GetNumberOfPoints(); ++idx)
    {
    pointOwnership->SetId(idx, -1);
    }

  // Brute force division.
  // The first N cells go to piece 0 ...
  for (idx = 0; idx < numCells; ++idx)
    {
    if ((idx * numPieces / numCells) == piece)
      {
      tags->SetValue(idx, 0);
      }
    else
      {
      tags->SetValue(idx, -1);
      }
    // Fill in point ownership mapping.
    input->GetCellPoints(idx, cellPtIds);
    for (j = 0; j < cellPtIds->GetNumberOfIds(); ++j)
      {
      ptId = cellPtIds->GetId(j);
      if (pointOwnership->GetId(ptId) == -1)
        {
        pointOwnership->SetId(ptId, idx);
        }  
      }
    }

  cellPtIds->Delete(); 
  
  //dicer->SetInput(input);
  //dicer->SetDiceModeToSpecifiedNumberOfPieces();
  //dicer->SetNumberOfPieces(numPieces);
  //dicer->Update();
  
  //intermediate->ShallowCopy(dicer->GetOutput());
  //intermediate->BuildLinks();
  //pointScalars = intermediate->GetPointData()->GetScalars();
}

//=============================================================================
void vtkExtractPolyDataPiece::Execute()
{
  vtkPolyData *input = this->GetInput();
  vtkPolyData *output = this->GetOutput();
  vtkPointData *pd=input->GetPointData(), *outPD=output->GetPointData();
  vtkCellData *cd=input->GetCellData(), *outCD=output->GetCellData();
  vtkIntArray *cellTags;
  int ghostLevel, piece, numPieces;
  vtkIdType cellId, newCellId;
  vtkIdList *cellPts, *pointMap;
  vtkIdList *newCellPts = vtkIdList::New();
  vtkIdList *pointOwnership;
  vtkCell *cell;
  vtkPoints *newPoints;
  vtkUnsignedCharArray* cellGhostLevels = 0;
  vtkUnsignedCharArray* pointGhostLevels = 0;
  vtkIdType ptId=0, newId, numPts, i;
  int numCellPts;
  double *x=NULL;

  // Pipeline update piece will tell us what to generate.
  ghostLevel = output->GetUpdateGhostLevel();
  piece = output->GetUpdatePiece();
  numPieces = output->GetUpdateNumberOfPieces();
  
  outPD->CopyAllocate(pd);
  outCD->CopyAllocate(cd);

  if (ghostLevel > 0 && this->CreateGhostCells)
    {
    cellGhostLevels = vtkUnsignedCharArray::New();
    pointGhostLevels = vtkUnsignedCharArray::New();
    cellGhostLevels->Allocate(input->GetNumberOfCells());
    pointGhostLevels->Allocate(input->GetNumberOfPoints());
    }
    
  // Break up cells based on which piece they belong to.
  cellTags = vtkIntArray::New();
  cellTags->Allocate(input->GetNumberOfCells(), 1000);
  pointOwnership = vtkIdList::New();
  pointOwnership->Allocate(input->GetNumberOfPoints());
  // Cell tags end up being 0 for cells in piece and -1 for all others.
  // Point ownership is the cell that owns the point.
  this->ComputeCellTags(cellTags, pointOwnership, piece, numPieces);
  
  // Find the layers of ghost cells.
  if (this->CreateGhostCells)
    {
    for (i = 0; i < ghostLevel; i++)
      {
      this->AddGhostLevel(input, cellTags, i+1);
      }
    }
  
  // Filter the cells.

  numPts = input->GetNumberOfPoints();
  output->Allocate(input->GetNumberOfCells());
  newPoints = vtkPoints::New();
  newPoints->Allocate(numPts);

  pointMap = vtkIdList::New(); //maps old point ids into new
  pointMap->SetNumberOfIds(numPts);
  for (i=0; i < numPts; i++)
    {
    pointMap->SetId(i,-1);
    }

  // Filter the cells
  for (cellId=0; cellId < input->GetNumberOfCells(); cellId++)
    {
    if ( cellTags->GetValue(cellId) != -1) // satisfied thresholding
      {
      if (cellGhostLevels)
        {                
        cellGhostLevels->InsertNextValue(
          (unsigned char)(cellTags->GetValue(cellId)));
        }
 
      cell = input->GetCell(cellId);
      cellPts = cell->GetPointIds();
      numCellPts = cell->GetNumberOfPoints();
    
      for (i=0; i < numCellPts; i++)
        {
        ptId = cellPts->GetId(i);
        if ( (newId = pointMap->GetId(ptId)) < 0 )
          {
          x = input->GetPoint(ptId);
          newId = newPoints->InsertNextPoint(x);
          if (pointGhostLevels)
            {
            pointGhostLevels->InsertNextValue(
              cellTags->GetValue(pointOwnership->GetId(ptId)));
            }
          pointMap->SetId(ptId,newId);
          outPD->CopyData(pd,ptId,newId);
          }
        newCellPts->InsertId(i,newId);
        }
      newCellId = output->InsertNextCell(cell->GetCellType(),newCellPts);
      outCD->CopyData(cd,cellId,newCellId);
      newCellPts->Reset();
      } // satisfied thresholding
    } // for all cells


  // Split up points that are not used by cells, 
  // and have not been assigned to any piece. 
  // Count the number of unassigned points.  This is an extra pass through
  // the points, but the pieces will be better load balanced and
  // more spatially coherent.
  vtkIdType count = 0;
  vtkIdType idx;
  for (idx = 0; idx < input->GetNumberOfPoints(); ++idx)
    {
    if (pointOwnership->GetId(idx) == -1)
      {
      ++count;
      }
    }
  vtkIdType count2 = 0;
  for (idx = 0; idx < input->GetNumberOfPoints(); ++idx)
    {
    if (pointOwnership->GetId(idx) == -1)
      {
      if ((count2 * numPieces / count) == piece)
      x = input->GetPoint(ptId);
      newId = newPoints->InsertNextPoint(x);
      if (pointGhostLevels)
        {
        pointGhostLevels->InsertNextValue(0);
        }
      }
    }

  vtkDebugMacro(<< "Extracted " << output->GetNumberOfCells() 
                << " number of cells.");

  // now clean up / update ourselves
  pointMap->Delete();
  newCellPts->Delete();
  
  if (cellGhostLevels)
    {
    cellGhostLevels->SetName("vtkGhostLevels");
    output->GetCellData()->AddArray(cellGhostLevels);
    cellGhostLevels->Delete();
    cellGhostLevels = 0;
     }
  if (pointGhostLevels)
    {
    pointGhostLevels->SetName("vtkGhostLevels");
    output->GetPointData()->AddArray(pointGhostLevels);
    pointGhostLevels->Delete();
    pointGhostLevels = 0;
    }
  output->SetPoints(newPoints);
  newPoints->Delete();

  output->Squeeze();
  cellTags->Delete();
  pointOwnership->Delete();

}

//=============================================================================
void vtkExtractPolyDataPiece::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "Create Ghost Cells: " << (this->CreateGhostCells ? "On\n" : "Off\n");
}

//=============================================================================
void vtkExtractPolyDataPiece::AddGhostLevel(vtkPolyData *input,
                                            vtkIntArray *cellTags, 
                                            int level)
{
  vtkIdType numCells, pointId, cellId, i;
  int j, k;
  vtkGenericCell *cell1 = vtkGenericCell::New();
  vtkGenericCell *cell2 = vtkGenericCell::New();
  vtkIdList *cellIds = vtkIdList::New();
  
  numCells = input->GetNumberOfCells();
  
  for (i = 0; i < numCells; i++)
    {
    if (cellTags->GetValue(i) == level - 1)
      {
      input->GetCell(i, cell1);
      for (j = 0; j < cell1->GetNumberOfPoints(); j++)
        {
        pointId = cell1->GetPointId(j);
        input->GetPointCells(pointId, cellIds);
        for (k = 0; k < cellIds->GetNumberOfIds(); k++)
          {
          cellId = cellIds->GetId(k);
          if (cellTags->GetValue(cellId) == -1)
            {
            input->GetCell(cellId, cell2);
            cellTags->SetValue(cellId, level);
            }
          }
        }
      }
    }
  cell1->Delete();
  cell2->Delete();
  cellIds->Delete();
}


















