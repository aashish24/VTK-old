/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkPolyDataConnectivityFilter.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkPolyDataConnectivityFilter* vtkPolyDataConnectivityFilter::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkPolyDataConnectivityFilter");
  if(ret)
    {
    return (vtkPolyDataConnectivityFilter*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkPolyDataConnectivityFilter;
}




// Construct with default extraction mode to extract largest regions.
vtkPolyDataConnectivityFilter::vtkPolyDataConnectivityFilter()
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

vtkPolyDataConnectivityFilter::~vtkPolyDataConnectivityFilter()
{
  this->RegionSizes->Delete();
  this->CellScalars->Delete();
  this->NeighborCellPointIds->Delete();
  this->Seeds->Delete();
  this->SpecifiedRegionIds->Delete();
}

void vtkPolyDataConnectivityFilter::Execute()
{
  int cellId, i, j, pt, newCellId;
  int numPts, numCells;
  vtkPoints *inPts;
  vtkPoints *newPts;
  vtkIdList *cellIds, *ptIds;
  int id, npts, *pts, *cells, n;
  unsigned short ncells;
  int maxCellsInRegion;
  int largestRegionId = 0;
  vtkPolyData *input = this->GetInput();
  vtkPolyData *output = this->GetOutput();
  vtkPointData *pd=input->GetPointData(), *outputPD=output->GetPointData();
  vtkCellData *cd=input->GetCellData(), *outputCD=output->GetCellData();
  
  vtkDebugMacro(<<"Executing polygon connectivity filter.");
  //
  //  Check input/allocate storage
  //
  inPts = input->GetPoints();

  if (inPts == NULL)
    {
    vtkErrorMacro("No points!");
    return;
    }

  numPts = inPts->GetNumberOfPoints();
  numCells = input->GetNumberOfCells();

  if ( numPts < 1 || numCells < 1 )
    {
    vtkDebugMacro(<<"No data to connect!");
    return;
    }
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
  // Build cell structure
  //
  this->Mesh = vtkPolyData::New();
  this->Mesh->CopyStructure(input);
  this->Mesh->BuildLinks();
  this->UpdateProgress(0.10);

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
          this->Mesh->GetPointCells(pt,ncells,cells);
          for (j=0; j < ncells; j++) 
	    {
            this->RecursionSeeds->InsertNextId(cells[j]);
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
        inPts->GetPoint(i,x);
        dist2 = vtkMath::Distance2BetweenPoints(x,this->ClosestPoint);
        if ( dist2 < minDist2 )
	  {
	  minId = i;
          minDist2 = dist2;
	  }
	}
      this->Mesh->GetPointCells(minId,ncells,cells);
      for (j=0; j < ncells; j++) 
	{
        this->RecursionSeeds->InsertNextId(cells[j]);
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

    }//else extracted seeded cells

  vtkDebugMacro (<<"Extracted " << this->RegionNumber << " region(s)");
  vtkDebugMacro (<<"Exceeded recursion depth " << this->NumExceededMaxDepth 
                 << " times");

  this->RecursionSeeds->Delete();
  //
  // Now that points and cells have been marked, traverse these lists pulling
  // everything that has been visited.
  //
  //Pass through point data that has been visited
  pd = input->GetPointData();
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
      newPts->InsertPoint(this->PointMap[i],inPts->GetPoint(i));
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
  // Create output cells. Have to allocate storage first.
  //
  if ( (n=input->GetVerts()->GetNumberOfCells()) > 0 )
    {
    vtkCellArray *newVerts = vtkCellArray::New();
    newVerts->Allocate(n,n);
    output->SetVerts(newVerts);
    newVerts->Delete();
    }
  if ( (n=input->GetLines()->GetNumberOfCells()) > 0 )
    {
    vtkCellArray *newLines = vtkCellArray::New();
    newLines->Allocate(2*n,n);
    output->SetLines(newLines);
    newLines->Delete();
    }
  if ( (n=input->GetPolys()->GetNumberOfCells()) > 0 )
    {
    vtkCellArray *newPolys = vtkCellArray::New();
    newPolys->Allocate(3*n,n);
    output->SetPolys(newPolys);
    newPolys->Delete();
    }
  if ( (n=input->GetStrips()->GetNumberOfCells()) > 0 )
    {
    vtkCellArray *newStrips = vtkCellArray::New();
    newStrips->Allocate(5*n,n);
    output->SetStrips(newStrips);
    newStrips->Delete();
    }
  
  cellIds = vtkIdList::New();
  cellIds->Allocate(VTK_CELL_SIZE);
  ptIds = vtkIdList::New();
  ptIds->Allocate(VTK_CELL_SIZE);

  if ( this->ExtractionMode == VTK_EXTRACT_POINT_SEEDED_REGIONS ||
  this->ExtractionMode == VTK_EXTRACT_CELL_SEEDED_REGIONS ||
  this->ExtractionMode == VTK_EXTRACT_CLOSEST_POINT_REGION ||
  this->ExtractionMode == VTK_EXTRACT_ALL_REGIONS)
    { // extract any cell that's been visited
    for (cellId=0; cellId < numCells; cellId++)
      {
      if ( this->Visited[cellId] >= 0 )
        {
        this->Mesh->GetCellPoints(cellId, npts, pts);
        ptIds->Reset();
        for (i=0; i < npts; i++)
          {
          id = this->PointMap[pts[i]];
          ptIds->InsertId(i,id);
          }
        newCellId = output->InsertNextCell(this->Mesh->GetCellType(cellId),
					   ptIds);
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
          this->Mesh->GetCellPoints(cellId, npts, pts);
          ptIds->Reset ();
          for (i=0; i < npts; i++)
            {
            id = this->PointMap[pts[i]];
            ptIds->InsertId(i,id);
            }
          newCellId = output->InsertNextCell(this->Mesh->GetCellType(cellId),
					     ptIds);
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
        this->Mesh->GetCellPoints(cellId, npts, pts);
        ptIds->Reset ();
        for (i=0; i < npts; i++)
          {
          id = this->PointMap[pts[i]];
          ptIds->InsertId(i,id);
          }
        newCellId = output->InsertNextCell(this->Mesh->GetCellType(cellId),
					   ptIds);
	outputCD->CopyData(cd,cellId,newCellId);
        }
      }
   }

  delete [] this->Visited;
  delete [] this->PointMap;
  this->Mesh->Delete();
  output->Squeeze();
  cellIds->Delete();
  ptIds->Delete();

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
void vtkPolyDataConnectivityFilter::TraverseAndMark (int cellId)
{
  int j, k, ptId, npts, *pts, *cells;
  unsigned short ncells;

  this->Visited[cellId] = this->RegionNumber;
  if ( this->RecursionDepth++ > this->MaxRecursionDepth ) 
    {
    this->RecursionSeeds->InsertNextId(cellId);
    this->NumExceededMaxDepth++;
    return;
    }

  this->NumCellsInRegion++;

  this->Mesh->GetCellPoints(cellId, npts, pts);

  for (j=0; j < npts; j++) 
    {
    if ( this->PointMap[ptId=pts[j]] < 0 )
      {
      this->PointMap[ptId] = this->PointNumber++;
      this->NewScalars->SetScalar(this->PointMap[ptId], this->RegionNumber);
      }
     
    this->Mesh->GetPointCells(ptId,ncells,cells);

    // check connectivity criterion (geometric + scalar)
    for (k=0; k < ncells; k++)
      {
      cellId = cells[k];
      if ( this->Visited[cellId] < 0 )
        {
        if ( this->InScalars )
          {
          int numScalars, ii;
          float s, range[2];

          this->Mesh->GetCellPoints(cellId, this->NeighborCellPointIds);
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

  this->RecursionDepth--;
  return;
}

// Obtain the number of connected regions.
int vtkPolyDataConnectivityFilter::GetNumberOfExtractedRegions()
{
  return this->RegionSizes->GetMaxId() + 1;
}

// Initialize list of point ids/cell ids used to seed regions.
void vtkPolyDataConnectivityFilter::InitializeSeedList()
{
  this->Modified();
  this->Seeds->Reset();
}

// Add a seed id (point or cell id). Note: ids are 0-offset.
void vtkPolyDataConnectivityFilter::AddSeed(int id)
{
  this->Modified();
  this->Seeds->InsertNextId(id);
}

// Delete a seed id (point or cell id). Note: ids are 0-offset.
void vtkPolyDataConnectivityFilter::DeleteSeed(int id)
{
  this->Modified();
  this->Seeds->DeleteId(id);
}

// Initialize list of region ids to extract.
void vtkPolyDataConnectivityFilter::InitializeSpecifiedRegionList()
{
  this->Modified();
  this->SpecifiedRegionIds->Reset();
}

// Add a region id to extract. Note: ids are 0-offset.
void vtkPolyDataConnectivityFilter::AddSpecifiedRegion(int id)
{
  this->Modified();
  this->SpecifiedRegionIds->InsertNextId(id);
}

// Delete a region id to extract. Note: ids are 0-offset.
void vtkPolyDataConnectivityFilter::DeleteSpecifiedRegion(int id)
{
  this->Modified();
  this->SpecifiedRegionIds->DeleteId(id);
}

void vtkPolyDataConnectivityFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolyDataToPolyDataFilter::PrintSelf(os,indent);

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

