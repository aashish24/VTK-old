/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen 
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
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkConnectivityFilter.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"

//--------------------------------------------------------------------------
vtkConnectivityFilter* vtkConnectivityFilter::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkConnectivityFilter");
  if(ret)
    {
    return (vtkConnectivityFilter*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkConnectivityFilter;
}

// Construct with default extraction mode to extract largest regions.
vtkConnectivityFilter::vtkConnectivityFilter()
{
  this->RegionSizes = vtkIntArray::New();
  this->ExtractionMode = VTK_EXTRACT_LARGEST_REGION;
  this->ColorRegions = 0;

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
  vtkIdType numPts, numCells, cellId, newCellId, i, j, pt;
  vtkPoints *newPts;
  int id;
  int maxCellsInRegion;
  int largestRegionId = 0;
  vtkDataSet *input= this->GetInput();
  vtkUnstructuredGrid *output = this->GetOutput();
  vtkPointData *pd=input->GetPointData(), *outputPD=output->GetPointData();
  vtkCellData *cd=input->GetCellData(), *outputCD=output->GetCellData();
  
  vtkDebugMacro(<<"Executing connectivity filter.");

  //  Check input/allocate storage
  //
  numCells=input->GetNumberOfCells();
  if ( (numPts=input->GetNumberOfPoints()) < 1 || numCells < 1 )
    {
    vtkDebugMacro(<<"No data to connect!");
    return;
    }
  output->Allocate(numCells,numCells);

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

  // Initialize.  Keep track of points and cells visited.
  //
  this->RegionSizes->Reset();
  this->Visited = new vtkIdType[numCells];
  for ( i=0; i < numCells; i++ )
    {
    this->Visited[i] = -1;
    }
  this->PointMap = new vtkIdType[numPts];  
  for ( i=0; i < numPts; i++ )
    {
    this->PointMap[i] = -1;
    }

  this->NewScalars = vtkScalars::New();
  this->NewScalars->SetNumberOfScalars(numPts);
  newPts = vtkPoints::New();
  newPts->Allocate(numPts);

  // Traverse all cells marking those visited.  Each new search
  // starts a new connected region. Connected region grows 
  // using a connected wave propagation.
  //
  this->Wave = vtkIdList::New();
  this->Wave->Allocate(numPts/4+1,numPts);
  this->Wave2 = vtkIdList::New();
  this->Wave2->Allocate(numPts/4+1,numPts);

  this->PointNumber = 0;
  this->RegionNumber = 0;
  maxCellsInRegion = 0;

  this->CellIds = vtkIdList::New(); 
  this->CellIds->Allocate(8, VTK_CELL_SIZE);
  this->PointIds = vtkIdList::New(); 
  this->PointIds->Allocate(8, VTK_CELL_SIZE);

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
        this->Wave->InsertNextId(cellId);
        this->TraverseAndMark ();

        if ( this->NumCellsInRegion > maxCellsInRegion )
          {
          maxCellsInRegion = this->NumCellsInRegion;
          largestRegionId = this->RegionNumber;
          }

        this->RegionSizes->InsertValue(this->RegionNumber++,
                                       this->NumCellsInRegion);
        this->Wave->Reset();
        this->Wave2->Reset();
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
          input->GetPointCells(pt,this->CellIds);
          for (j=0; j < this->CellIds->GetNumberOfIds(); j++) 
            {
            this->Wave->InsertNextId(this->CellIds->GetId(j));
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
          this->Wave->InsertNextId(cellId);
          }
        }
      }
    else if ( this->ExtractionMode == VTK_EXTRACT_CLOSEST_POINT_REGION )
      {//loop over points, find closest one
      float minDist2, dist2, x[3];
      vtkIdType minId = 0;
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
      input->GetPointCells(minId,this->CellIds);
      for (j=0; j < this->CellIds->GetNumberOfIds(); j++) 
        {
        this->Wave->InsertNextId(this->CellIds->GetId(j));
        }
      }
    this->UpdateProgress (0.5);

    //mark all seeded regions
    this->TraverseAndMark ();
    this->RegionSizes->InsertValue(this->RegionNumber,this->NumCellsInRegion);
    this->UpdateProgress (0.9);
    }

  vtkDebugMacro (<<"Extracted " << this->RegionNumber << " region(s)");
  this->Wave->Delete();
  this->Wave2->Delete();

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
        input->GetCellPoints(cellId, this->PointIds);
        for (i=0; i < this->PointIds->GetNumberOfIds(); i++)
          {
          id = this->PointMap[this->PointIds->GetId(i)];
          this->PointIds->InsertId(i,id);
          }
        newCellId = output->InsertNextCell(input->GetCellType(cellId),
                                           this->PointIds);
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
          input->GetCellPoints(cellId, this->PointIds);
          for (i=0; i < this->PointIds->GetNumberOfIds(); i++)
            {
            id = this->PointMap[this->PointIds->GetId(i)];
            this->PointIds->InsertId(i,id);
            }
          newCellId =output->InsertNextCell(input->GetCellType(cellId),
                                            this->PointIds);
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
        input->GetCellPoints(cellId, this->PointIds);
        for (i=0; i < this->PointIds->GetNumberOfIds(); i++)
          {
          id = this->PointMap[this->PointIds->GetId(i)];
          this->PointIds->InsertId(i,id);
          }
        newCellId = output->InsertNextCell(input->GetCellType(cellId),
                                           this->PointIds);
        outputCD->CopyData(cd,cellId,newCellId);
        }
      }
   }

  delete [] this->Visited;
  delete [] this->PointMap;
  this->PointIds->Delete();
  this->CellIds->Delete();
  output->Squeeze();

  int num = this->GetNumberOfExtractedRegions();
  int count = 0;

  for (int ii = 0; ii < num; ii++)
    {
    count += this->RegionSizes->GetValue (ii);
    }
  vtkDebugMacro (<< "Total # of cells accounted for: " << count);
  vtkDebugMacro (<< "Extracted " << output->GetNumberOfCells() << " cells");

  return;
}


// Mark current cell as visited and assign region number.  Note:
// traversal occurs across shared vertices.
//
void vtkConnectivityFilter::TraverseAndMark ()
{
  int i, j, k, cellId, numIds, ptId, numPts, numCells;
  vtkIdList *tmpWave;
  vtkDataSet *input= this->GetInput();

  while ( (numIds=this->Wave->GetNumberOfIds()) > 0 )
    {
    for ( i=0; i < numIds; i++ )
      {
      cellId = this->Wave->GetId(i);
      if ( this->Visited[cellId] < 0 )
        {
        this->Visited[cellId] = this->RegionNumber;
        this->NumCellsInRegion++;
        input->GetCellPoints(cellId, this->PointIds);

        numPts = this->PointIds->GetNumberOfIds();
        for (j=0; j < numPts; j++) 
          {
          if ( this->PointMap[ptId=this->PointIds->GetId(j)] < 0 )
            {
            this->PointMap[ptId] = this->PointNumber++;
            this->NewScalars->SetScalar(this->PointMap[ptId],
                                        this->RegionNumber);
            }

          input->GetPointCells(ptId,this->CellIds);

          // check connectivity criterion (geometric + scalar)
          numCells = this->CellIds->GetNumberOfIds();
          for (k=0; k < numCells; k++)
            {
            cellId = this->CellIds->GetId(k);
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
                this->Wave2->InsertNextId(cellId);
                }
              }
            else
              {
              this->Wave2->InsertNextId(cellId);
              }
            }//for all cells using this point
          }//for all points of this cell
        }//if cell not yet visited
      }//for all cells in this wave

    tmpWave = this->Wave;
    this->Wave = this->Wave2;
    this->Wave2 = tmpWave;
    tmpWave->Reset();
    } //while wave is not empty

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
void vtkConnectivityFilter::AddSeed(vtkIdType id)
{
  this->Modified();
  this->Seeds->InsertNextId(id);
}

// Delete a seed id (point or cell id). Note: ids are 0-offset.
void vtkConnectivityFilter::DeleteSeed(vtkIdType id)
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

  os << indent << "Scalar Connectivity: " 
     << (this->ScalarConnectivity ? "On\n" : "Off\n");

  float *range = this->GetScalarRange();
  os << indent << "Scalar Range: (" << range[0] << ", " << range[1] << ")\n";
}

