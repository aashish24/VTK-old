/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1998 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
#include "vtkConnectivityFilter.h"

// Description:
// Construct with default extraction mode to extract largest regions.
vtkConnectivityFilter::vtkConnectivityFilter()
{
  this->RegionSizes = vtkIntArray::New();
  this->ExtractionMode = VTK_EXTRACT_LARGEST_REGION;
  this->ColorRegions = 0;
  this->MaxRecursionDepth = 10000;

  this->ScalarConnectivity = 0;
  this->ScalarRange[0] = 0.0;
  this->ScalarRange[1] = 1.0;

  this->CellScalars = vtkScalars::New(); 
  this->CellScalars->Allocate(8);

  this->NeighborCellPointIds = vtkIdList::New();
  this->NeighborCellPointIds->Allocate(8);
}


vtkConnectivityFilter::~vtkConnectivityFilter()
{
  this->RegionSizes->Delete();
  this->CellScalars->Delete();
  this->NeighborCellPointIds->Delete();
}


static int NumExceededMaxDepth;
static int *Visited, *PointMap;
static vtkScalars *NewScalars;
static int RecursionDepth;
static int RegionNumber, PointNumber;    
static int NumCellsInRegion;
static vtkIdList *RecursionSeeds;
static vtkScalars *InScalars;

void vtkConnectivityFilter::Execute()
{
  int cellId, i, j, pt;
  int numPts, numCells;
  vtkPoints *newPts;
  vtkIdList cellIds(VTK_CELL_SIZE), ptIds(VTK_CELL_SIZE);
  vtkPointData *pd;
  int id;
  int maxCellsInRegion;
  int largestRegionId = 0;
  vtkDataSet *input=(vtkDataSet *)this->Input;
  vtkUnstructuredGrid *output = this->GetOutput();
  vtkPointData *outputPD = output->GetPointData();
  
  vtkDebugMacro(<<"Executing connectivity filter.");
  //
  //  Check input/allocate storage
  //
  numCells=input->GetNumberOfCells();
  if ( (numPts=input->GetNumberOfPoints()) < 1 || numCells < 1 )
    {
    vtkDebugMacro(<<"No data to connect!");
    return;
    }
  output->Allocate(numCells,numCells);
  //
  // See whether to consider scalar connectivity
  //
  InScalars = input->GetPointData()->GetScalars();
  if ( !this->ScalarConnectivity ) 
    InScalars = NULL;
  else
    {
    if ( this->ScalarRange[1] < this->ScalarRange[0] ) 
      this->ScalarRange[1] = this->ScalarRange[0];
    }
  //
  // Initialize.  Keep track of points and cells visited.
  //
  this->RegionSizes->Reset();
  Visited = new int[numCells];
  for ( i=0; i < numCells; i++ ) Visited[i] = -1;
  PointMap = new int[numPts];  
  for ( i=0; i < numPts; i++ ) PointMap[i] = -1;

  NewScalars = vtkScalars::New();
  NewScalars->SetNumberOfScalars(numPts);
  newPts = vtkPoints::New();
  newPts->Allocate(numPts);
  //
  // Traverse all cells marking those visited.  Each new search
  // starts a new connected region.  Note: have to truncate recursion
  // and keep track of seeds to start up again.
  //
  RecursionSeeds = vtkIdList::New();
  RecursionSeeds->Allocate(1000,10000);

  NumExceededMaxDepth = 0;
  PointNumber = 0;
  RegionNumber = 0;
  maxCellsInRegion = 0;

  if ( this->ExtractionMode != VTK_EXTRACT_POINT_SEEDED_REGIONS && 
  this->ExtractionMode != VTK_EXTRACT_CELL_SEEDED_REGIONS ) 
    { //visit all cells marking with region number
    for (cellId=0; cellId < numCells; cellId++)
      {

      if ( cellId && !(cellId % 5000) )
	{
	this->UpdateProgress (0.1 + 0.8*cellId/numCells);
	}

      if ( Visited[cellId] < 0 ) 
        {
        NumCellsInRegion = 0;
        RecursionDepth = 0;
        this->TraverseAndMark (cellId);

        for (i=0; i < RecursionSeeds->GetNumberOfIds(); i++) 
          {
          RecursionDepth = 0;
          this->TraverseAndMark (RecursionSeeds->GetId(i));
          }

        if ( NumCellsInRegion > maxCellsInRegion )
          {
          maxCellsInRegion = NumCellsInRegion;
          largestRegionId = RegionNumber;
          }

        this->RegionSizes->InsertValue(RegionNumber++,NumCellsInRegion);
        RecursionSeeds->Reset();
        }
      }
    }
  else // regions have been seeded, everything considered in same region
    {
    NumCellsInRegion = 0;

    if ( this->ExtractionMode == VTK_EXTRACT_POINT_SEEDED_REGIONS )
      {
      for (i=0; i < this->Seeds.GetNumberOfIds(); i++) 
        {
        pt = this->Seeds.GetId(i);
        if ( pt >= 0 ) 
          {
          input->GetPointCells(pt,cellIds);
          for (j=0; j < cellIds.GetNumberOfIds(); j++) 
            RecursionSeeds->InsertNextId(cellIds.GetId(j));
          }
        }
      }
    else if ( this->ExtractionMode == VTK_EXTRACT_CELL_SEEDED_REGIONS )
      {
      for (i=0; i < this->Seeds.GetNumberOfIds(); i++) 
        {
        cellId = this->Seeds.GetId(i);
        if ( cellId >= 0 ) RecursionSeeds->InsertNextId(cellId);
        }
      }
    this->UpdateProgress (0.5);

    //mark all seeded regions
    for (i=0; i < RecursionSeeds->GetNumberOfIds(); i++) 
      {
      RecursionDepth = 0;
      this->TraverseAndMark (RecursionSeeds->GetId(i));
      }
    this->RegionSizes->InsertValue(RegionNumber,NumCellsInRegion);
    this->UpdateProgress (0.9);
    }

  vtkDebugMacro (<<"Extracted " << RegionNumber << " region(s)");
  vtkDebugMacro (<<"Exceeded recursion depth " << NumExceededMaxDepth 
                 << " times");

  RecursionSeeds->Delete();
//
// Now that points and cells have been marked, traverse these lists pulling
// everything that has been visited.
//
  //Pass through point data that has been visited
  pd = input->GetPointData();
  if ( this->ColorRegions ) outputPD->CopyScalarsOff();
  outputPD->CopyAllocate(pd);

  for (i=0; i < numPts; i++)
    {
    if ( PointMap[i] > -1 )
      {
      newPts->InsertPoint(PointMap[i],input->GetPoint(i));
      outputPD->CopyData(pd,i,PointMap[i]);
      }
    }

  // if coloring regions; send down new scalar data
  if ( this->ColorRegions ) outputPD->SetScalars(NewScalars);
  NewScalars->Delete();

  output->SetPoints(newPts);
  newPts->Delete();
//
// Create output cells
//
  if ( this->ExtractionMode == VTK_EXTRACT_POINT_SEEDED_REGIONS ||
  this->ExtractionMode == VTK_EXTRACT_CELL_SEEDED_REGIONS ||
  this->ExtractionMode == VTK_EXTRACT_ALL_REGIONS)
    { // extract any cell that's been visited
    for (cellId=0; cellId < numCells; cellId++)
      {
      if ( Visited[cellId] >= 0 )
        {
        input->GetCellPoints(cellId, ptIds);
        for (i=0; i < ptIds.GetNumberOfIds(); i++)
          {
          id = PointMap[ptIds.GetId(i)];
          ptIds.InsertId(i,id);
          }
        output->InsertNextCell(input->GetCellType(cellId),ptIds);
        }
      }
    }
  else if ( this->ExtractionMode == VTK_EXTRACT_SPECIFIED_REGIONS )
    {
    for (cellId=0; cellId < numCells; cellId++)
      {
      int inReg, regionId;
      if ( (regionId=Visited[cellId]) >= 0 )
        {
        for (inReg=0,i=0; i<this->SpecifiedRegionIds.GetNumberOfIds(); i++)
          {
          if ( regionId == this->SpecifiedRegionIds.GetId(i) )
            {
            inReg = 1;
            break;
            }
          }
        if ( inReg )
          {
          input->GetCellPoints(cellId, ptIds);
          for (i=0; i < ptIds.GetNumberOfIds(); i++)
            {
            id = PointMap[ptIds.GetId(i)];
            ptIds.InsertId(i,id);
            }
          output->InsertNextCell(input->GetCellType(cellId),ptIds);
          }
        }
      }
    }
  else //extract largest region
    {
    for (cellId=0; cellId < numCells; cellId++)
      {
      if ( Visited[cellId] == largestRegionId )
        {
        input->GetCellPoints(cellId, ptIds);
        for (i=0; i < ptIds.GetNumberOfIds(); i++)
          {
          id = PointMap[ptIds.GetId(i)];
          ptIds.InsertId(i,id);
          }
        output->InsertNextCell(input->GetCellType(cellId),ptIds);
        }
      }
   }

  delete [] Visited;
  delete [] PointMap;
  output->Squeeze();

  int num = this->GetNumberOfExtractedRegions();
  int count = 0;

  for (int ii = 0; ii < num; ii++)
    {
    count += this->RegionSizes->GetValue (ii);
    }
  vtkDebugMacro (<< "Total # of cells accounted for: " << count);
  vtkDebugMacro (<<"Extracted " << output->GetNumberOfCells() << " cells");

  return;
}

//
// Mark current cell as visited and assign region number.  Note:
// traversal occurs across shared vertices.
//
void vtkConnectivityFilter::TraverseAndMark (int cellId)
{
  int j, k, ptId, numPts, numCells;
  vtkIdList ptIds(8,VTK_CELL_SIZE), cellIds(8,VTK_CELL_SIZE);
  vtkDataSet *input=(vtkDataSet *)this->Input;

  Visited[cellId] = RegionNumber;

  if ( RecursionDepth++ > this->MaxRecursionDepth ) 
    {
    RecursionSeeds->InsertNextId(cellId);
    NumExceededMaxDepth++;
    return;
    }

  NumCellsInRegion++;
  input->GetCellPoints(cellId, ptIds);

  numPts = ptIds.GetNumberOfIds();
  for (j=0; j < numPts; j++) 
    {
    if ( PointMap[ptId=ptIds.GetId(j)] < 0 )
      {
      PointMap[ptId] = PointNumber++;
      NewScalars->SetScalar(PointMap[ptId], RegionNumber);
      }
     
    input->GetPointCells(ptId,cellIds);

    // check connectivity criterion (geometric + scalar)
    numCells = cellIds.GetNumberOfIds();
    for (k=0; k < numCells; k++)
      {
      cellId = cellIds.GetId(k);
      if ( Visited[cellId] < 0 )
        {
        if ( InScalars )
          {
          int numScalars, ii;
          float s, range[2];

          input->GetCellPoints(cellId,*this->NeighborCellPointIds);
          InScalars->GetScalars(*this->NeighborCellPointIds,*this->CellScalars);
          numScalars = this->CellScalars->GetNumberOfScalars();
          range[0] = VTK_LARGE_FLOAT; range[1] = -VTK_LARGE_FLOAT;
          for (ii=0; ii < numScalars;  ii++)
            {
            s = this->CellScalars->GetScalar(ii);
            if ( s < range[0] ) range[0] = s;
            if ( s > range[1] ) range[1] = s;
            }
          if ( range[1] >= this->ScalarRange[0] && 
          range[0] <= this->ScalarRange[1] )
            {
            TraverseAndMark (cellId);
            }
          }
        else
          {
          TraverseAndMark (cellId);
          }
        }
      }

    } // for all cells of this element

  RecursionDepth--;
  return;
}

// Description:
// Obtain the number of connected regions.
int vtkConnectivityFilter::GetNumberOfExtractedRegions()
{
  return this->RegionSizes->GetMaxId() + 1;
}

// Description:
// Initialize list of point ids/cell ids used to seed regions.
void vtkConnectivityFilter::InitializeSeedList()
{
  this->Modified();
  this->Seeds.Reset();
}

// Description:
// Add a seed id (point or cell id). Note: ids are 0-offset.
void vtkConnectivityFilter::AddSeed(int id)
{
  this->Modified();
  this->Seeds.InsertNextId(id);
}

// Description:
// Delete a seed id (point or cell id). Note: ids are 0-offset.
void vtkConnectivityFilter::DeleteSeed(int id)
{
  this->Modified();
  this->Seeds.DeleteId(id);
}

// Description:
// Initialize list of region ids to extract.
void vtkConnectivityFilter::InitializeSpecifiedRegionList()
{
  this->Modified();
  this->SpecifiedRegionIds.Reset();
}

// Description:
// Add a region id to extract. Note: ids are 0-offset.
void vtkConnectivityFilter::AddSpecifiedRegion(int id)
{
  this->Modified();
  this->SpecifiedRegionIds.InsertNextId(id);
}

// Description:
// Delete a region id to extract. Note: ids are 0-offset.
void vtkConnectivityFilter::DeleteSpecifiedRegion(int id)
{
  this->Modified();
  this->SpecifiedRegionIds.DeleteId(id);
}

void vtkConnectivityFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataSetToUnstructuredGridFilter::PrintSelf(os,indent);

  os << indent << "Extraction Mode: ";
  switch (this->ExtractionMode)
    {
    case VTK_EXTRACT_POINT_SEEDED_REGIONS:
      os << "(Extract point seeded regions)\n";
      break;
    case VTK_EXTRACT_CELL_SEEDED_REGIONS:
      os << "(Extract cell seeded regions)\n";
      break;
    case VTK_EXTRACT_SPECIFIED_REGIONS:
      os << "(Extract specified regions)\n";
      break;
    case VTK_EXTRACT_LARGEST_REGION:
      os << "(Extract largest region)\n";
      break;
    }

  os << indent << "Color Regions: " << (this->ColorRegions ? "On\n" : "Off\n");
  os << indent << "Maximum Recursion Depth: " << this->MaxRecursionDepth << "\n";

  os << indent << "Scalar Connectivity: " 
     << (this->ScalarConnectivity ? "On\n" : "Off\n");

  float *range = this->GetScalarRange();
  os << indent << "Scalar Range: (" << range[0] << ", " << range[1] << ")\n";
}

