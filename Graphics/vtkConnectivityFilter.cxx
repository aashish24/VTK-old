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
#include "vtkMath.h"

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

  this->ClosestPoint[0] = this->ClosestPoint[1] = this->ClosestPoint[2] = 0.0;

  this->CellScalars = vtkScalars::New(); 
  this->CellScalars->Allocate(8);

  this->NeighborCellPointIds = vtkIdList::New();
  this->NeighborCellPointIds->Allocate(8);
  
  this->Seeds = vtkIdList::New();
  this->SpecifiedRegionIds = vtkIdList::New();
}


vtkConnectivityFilter::~vtkConnectivityFilter()
{
  this->RegionSizes->Delete();
  this->CellScalars->Delete();
  this->NeighborCellPointIds->Delete();
  this->Seeds->Delete();
  this->SpecifiedRegionIds->Delete();
}

void vtkConnectivityFilter::Execute()
{
  int cellId, i, j, pt, newCellId;
  int numPts, numCells;
  vtkPoints *newPts;
  vtkIdList *cellIds, *ptIds;
  int id;
  int maxCellsInRegion;
  int largestRegionId = 0;
  vtkDataSet *input=(vtkDataSet *)this->Input;
  vtkUnstructuredGrid *output = this->GetOutput();
  vtkPointData *pd=input->GetPointData(), *outputPD=output->GetPointData();
  vtkCellData *cd=input->GetCellData(), *outputCD=output->GetCellData();
  
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
  this->InScalars = input->GetPointData()->GetScalars();
  if ( !this->ScalarConnectivity ) 
    {
    this->InScalars = NULL;
    }
  else
    {
    if ( this->ScalarRange[1] < this->ScalarRange[0] ) 
      {
      this->ScalarRange[1] = this->ScalarRange[0];
      }
    }
  //
  // Initialize.  Keep track of points and cells visited.
  //
  this->RegionSizes->Reset();
  this->Visited = new int[numCells];
  for ( i=0; i < numCells; i++ )
    {
    this->Visited[i] = -1;
    }
  this->PointMap = new int[numPts];  
  for ( i=0; i < numPts; i++ )
    {
    this->PointMap[i] = -1;
    }

  this->NewScalars = vtkScalars::New();
  this->NewScalars->SetNumberOfScalars(numPts);
  newPts = vtkPoints::New();
  newPts->Allocate(numPts);
  //
  // Traverse all cells marking those visited.  Each new search
  // starts a new connected region.  Note: have to truncate recursion
  // and keep track of seeds to start up again.
  //
  this->RecursionSeeds = vtkIdList::New();
  this->RecursionSeeds->Allocate(1000,10000);

  this->NumExceededMaxDepth = 0;
  this->PointNumber = 0;
  this->RegionNumber = 0;
  maxCellsInRegion = 0;

  cellIds = vtkIdList::New(); cellIds->Allocate(VTK_CELL_SIZE);
  ptIds = vtkIdList::New(); ptIds->Allocate(VTK_CELL_SIZE);

  if ( this->ExtractionMode != VTK_EXTRACT_POINT_SEEDED_REGIONS && 
  this->ExtractionMode != VTK_EXTRACT_CELL_SEEDED_REGIONS &&
  this->ExtractionMode != VTK_EXTRACT_CLOSEST_POINT_REGION ) 
    { //visit all cells marking with region number
    for (cellId=0; cellId < numCells; cellId++)
      {

      if ( cellId && !(cellId % 5000) )
	{
	this->UpdateProgress (0.1 + 0.8*cellId/numCells);
	}

      if ( this->Visited[cellId] < 0 ) 
        {
        this->NumCellsInRegion = 0;
        this->RecursionDepth = 0;
        this->TraverseAndMark (cellId);

        for (i=0; i < this->RecursionSeeds->GetNumberOfIds(); i++) 
          {
          this->RecursionDepth = 0;
          this->TraverseAndMark (this->RecursionSeeds->GetId(i));
          }

        if ( this->NumCellsInRegion > maxCellsInRegion )
          {
          maxCellsInRegion = this->NumCellsInRegion;
          largestRegionId = this->RegionNumber;
          }

        this->RegionSizes->InsertValue(this->RegionNumber++,this->NumCellsInRegion);
        this->RecursionSeeds->Reset();
        }
      }
    }
  else // regions have been seeded, everything considered in same region
    {
    this->NumCellsInRegion = 0;

    if ( this->ExtractionMode == VTK_EXTRACT_POINT_SEEDED_REGIONS )
      {
      for (i=0; i < this->Seeds->GetNumberOfIds(); i++) 
        {
        pt = this->Seeds->GetId(i);
        if ( pt >= 0 ) 
          {
          input->GetPointCells(pt,cellIds);
          for (j=0; j < cellIds->GetNumberOfIds(); j++) 
	    {
            this->RecursionSeeds->InsertNextId(cellIds->GetId(j));
	    }
          }
        }
      }
    else if ( this->ExtractionMode == VTK_EXTRACT_CELL_SEEDED_REGIONS )
      {
      for (i=0; i < this->Seeds->GetNumberOfIds(); i++) 
        {
        cellId = this->Seeds->GetId(i);
        if ( cellId >= 0 )
	  {
	  this->RecursionSeeds->InsertNextId(cellId);
	  }
        }
      }
    else if ( this->ExtractionMode == VTK_EXTRACT_CLOSEST_POINT_REGION )
      {//loop over points, find closest one
      float minDist2, dist2, x[3];
      int minId;
      for (minDist2=VTK_LARGE_FLOAT, i=0; i<numPts; i++)
	{
        input->GetPoint(i,x);
        dist2 = vtkMath::Distance2BetweenPoints(x,this->ClosestPoint);
        if ( dist2 < minDist2 )
	  {
	  minId = i;
          minDist2 = dist2;
	  }
	}
      input->GetPointCells(minId,cellIds);
      for (j=0; j < cellIds->GetNumberOfIds(); j++) 
	{
	this->RecursionSeeds->InsertNextId(cellIds->GetId(j));
	}
      }
    this->UpdateProgress (0.5);

    //mark all seeded regions
    for (i=0; i < this->RecursionSeeds->GetNumberOfIds(); i++) 
      {
      this->RecursionDepth = 0;
      this->TraverseAndMark (this->RecursionSeeds->GetId(i));
      }
    this->RegionSizes->InsertValue(this->RegionNumber,this->NumCellsInRegion);
    this->UpdateProgress (0.9);
    }

  vtkDebugMacro (<<"Extracted " << this->RegionNumber << " region(s)");
  vtkDebugMacro (<<"Exceeded recursion depth " << this->NumExceededMaxDepth 
                 << " times");

  this->RecursionSeeds->Delete();
//
// Now that points and cells have been marked, traverse these lists pulling
// everything that has been visited.
//
  //Pass through point data that has been visited
  if ( this->ColorRegions )
    {
    outputPD->CopyScalarsOff();
    }
  outputPD->CopyAllocate(pd);
  outputCD->CopyAllocate(cd);

  for (i=0; i < numPts; i++)
    {
    if ( this->PointMap[i] > -1 )
      {
      newPts->InsertPoint(this->PointMap[i],input->GetPoint(i));
      outputPD->CopyData(pd,i,this->PointMap[i]);
      }
    }

  // if coloring regions; send down new scalar data
  if ( this->ColorRegions )
    {
    outputPD->SetScalars(this->NewScalars);
    }
  this->NewScalars->Delete();

  output->SetPoints(newPts);
  newPts->Delete();
//
// Create output cells
//
  if ( this->ExtractionMode == VTK_EXTRACT_POINT_SEEDED_REGIONS ||
  this->ExtractionMode == VTK_EXTRACT_CELL_SEEDED_REGIONS ||
  this->ExtractionMode == VTK_EXTRACT_CLOSEST_POINT_REGION ||
  this->ExtractionMode == VTK_EXTRACT_ALL_REGIONS)
    { // extract any cell that's been visited
    for (cellId=0; cellId < numCells; cellId++)
      {
      if ( this->Visited[cellId] >= 0 )
        {
        input->GetCellPoints(cellId, ptIds);
        for (i=0; i < ptIds->GetNumberOfIds(); i++)
          {
          id = this->PointMap[ptIds->GetId(i)];
          ptIds->InsertId(i,id);
          }
        newCellId = output->InsertNextCell(input->GetCellType(cellId),ptIds);
	outputCD->CopyData(cd,cellId,newCellId);
        }
      }
    }
  else if ( this->ExtractionMode == VTK_EXTRACT_SPECIFIED_REGIONS )
    {
    for (cellId=0; cellId < numCells; cellId++)
      {
      int inReg, regionId;
      if ( (regionId=this->Visited[cellId]) >= 0 )
        {
        for (inReg=0,i=0; i<this->SpecifiedRegionIds->GetNumberOfIds(); i++)
          {
          if ( regionId == this->SpecifiedRegionIds->GetId(i) )
            {
            inReg = 1;
            break;
            }
          }
        if ( inReg )
          {
	  input->GetCellPoints(cellId, ptIds);
          for (i=0; i < ptIds->GetNumberOfIds(); i++)
            {
            id = this->PointMap[ptIds->GetId(i)];
            ptIds->InsertId(i,id);
            }
          newCellId =output->InsertNextCell(input->GetCellType(cellId),ptIds);
	  outputCD->CopyData(cd,cellId,newCellId);
          }
        }
      }
    }
  else //extract largest region
    {
    for (cellId=0; cellId < numCells; cellId++)
      {
      if ( this->Visited[cellId] == largestRegionId )
        {
        input->GetCellPoints(cellId, ptIds);
        for (i=0; i < ptIds->GetNumberOfIds(); i++)
          {
          id = this->PointMap[ptIds->GetId(i)];
          ptIds->InsertId(i,id);
          }
        newCellId = output->InsertNextCell(input->GetCellType(cellId),ptIds);
	outputCD->CopyData(cd,cellId,newCellId);
        }
      }
   }

  delete [] this->Visited;
  delete [] this->PointMap;
  output->Squeeze();
  ptIds->Delete();
  cellIds->Delete();

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
  vtkIdList *ptIds, *cellIds;
  vtkDataSet *input=(vtkDataSet *)this->Input;

  this->Visited[cellId] = this->RegionNumber;

  if ( this->RecursionDepth++ > this->MaxRecursionDepth ) 
    {
    this->RecursionSeeds->InsertNextId(cellId);
    this->NumExceededMaxDepth++;
    return;
    }

  cellIds = vtkIdList::New(); cellIds->Allocate(8, VTK_CELL_SIZE);
  ptIds = vtkIdList::New(); ptIds->Allocate(8, VTK_CELL_SIZE);

  this->NumCellsInRegion++;
  input->GetCellPoints(cellId, ptIds);

  numPts = ptIds->GetNumberOfIds();
  for (j=0; j < numPts; j++) 
    {
    if ( this->PointMap[ptId=ptIds->GetId(j)] < 0 )
      {
      this->PointMap[ptId] = this->PointNumber++;
      this->NewScalars->SetScalar(this->PointMap[ptId], this->RegionNumber);
      }
     
    input->GetPointCells(ptId,cellIds);

    // check connectivity criterion (geometric + scalar)
    numCells = cellIds->GetNumberOfIds();
    for (k=0; k < numCells; k++)
      {
      cellId = cellIds->GetId(k);
      if ( this->Visited[cellId] < 0 )
        {
        if ( this->InScalars )
          {
          int numScalars, ii;
          float s, range[2];

          input->GetCellPoints(cellId, this->NeighborCellPointIds);
          this->InScalars->GetScalars(this->NeighborCellPointIds,
				      this->CellScalars);
          numScalars = this->CellScalars->GetNumberOfScalars();
          range[0] = VTK_LARGE_FLOAT; range[1] = -VTK_LARGE_FLOAT;
          for (ii=0; ii < numScalars;  ii++)
            {
            s = this->CellScalars->GetScalar(ii);
            if ( s < range[0] )
	      {
	      range[0] = s;
	      }
            if ( s > range[1] )
	      {
	      range[1] = s;
	      }
            }
          if ( range[1] >= this->ScalarRange[0] && 
	       range[0] <= this->ScalarRange[1] )
            {
            this->TraverseAndMark (cellId);
            }
          }
        else
          {
          this->TraverseAndMark (cellId);
          }
        }
      }

    } // for all cells of this element

  ptIds->Delete();
  cellIds->Delete();

  this->RecursionDepth--;
  return;
}

// Obtain the number of connected regions.
int vtkConnectivityFilter::GetNumberOfExtractedRegions()
{
  return this->RegionSizes->GetMaxId() + 1;
}

// Initialize list of point ids/cell ids used to seed regions.
void vtkConnectivityFilter::InitializeSeedList()
{
  this->Modified();
  this->Seeds->Reset();
}

// Add a seed id (point or cell id). Note: ids are 0-offset.
void vtkConnectivityFilter::AddSeed(int id)
{
  this->Modified();
  this->Seeds->InsertNextId(id);
}

// Delete a seed id (point or cell id). Note: ids are 0-offset.
void vtkConnectivityFilter::DeleteSeed(int id)
{
  this->Modified();
  this->Seeds->DeleteId(id);
}

// Initialize list of region ids to extract.
void vtkConnectivityFilter::InitializeSpecifiedRegionList()
{
  this->Modified();
  this->SpecifiedRegionIds->Reset();
}

// Add a region id to extract. Note: ids are 0-offset.
void vtkConnectivityFilter::AddSpecifiedRegion(int id)
{
  this->Modified();
  this->SpecifiedRegionIds->InsertNextId(id);
}

// Delete a region id to extract. Note: ids are 0-offset.
void vtkConnectivityFilter::DeleteSpecifiedRegion(int id)
{
  this->Modified();
  this->SpecifiedRegionIds->DeleteId(id);
}

void vtkConnectivityFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataSetToUnstructuredGridFilter::PrintSelf(os,indent);

  os << indent << "Extraction Mode: ";
  os << this->GetExtractionModeAsString() << "\n";

  os << indent << "Closest Point: (" << this->ClosestPoint[0] << ", " 
     << this->ClosestPoint[1] << ", " << this->ClosestPoint[2] << ")\n";

  os << indent << "Color Regions: " << (this->ColorRegions ? "On\n" : "Off\n");
  os << indent << "Maximum Recursion Depth: " << this->MaxRecursionDepth << "\n";

  os << indent << "Scalar Connectivity: " 
     << (this->ScalarConnectivity ? "On\n" : "Off\n");

  float *range = this->GetScalarRange();
  os << indent << "Scalar Range: (" << range[0] << ", " << range[1] << ")\n";
}

