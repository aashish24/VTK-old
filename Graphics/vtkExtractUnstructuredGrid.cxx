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
#include "vtkExtractUnstructuredGrid.h"
#include "vtkMergePoints.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkExtractUnstructuredGrid* vtkExtractUnstructuredGrid::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkExtractUnstructuredGrid");
  if(ret)
    {
    return (vtkExtractUnstructuredGrid*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkExtractUnstructuredGrid;
}




// Construct with all types of clipping turned off.
vtkExtractUnstructuredGrid::vtkExtractUnstructuredGrid()
{
  this->PointMinimum = 0;
  this->PointMaximum = VTK_LARGE_INTEGER;

  this->CellMinimum = 0;
  this->CellMaximum = VTK_LARGE_INTEGER;

  this->Extent[0] = -VTK_LARGE_FLOAT;
  this->Extent[1] = VTK_LARGE_FLOAT;
  this->Extent[2] = -VTK_LARGE_FLOAT;
  this->Extent[3] = VTK_LARGE_FLOAT;
  this->Extent[4] = -VTK_LARGE_FLOAT;
  this->Extent[5] = VTK_LARGE_FLOAT;

  this->PointClipping = 0;
  this->CellClipping = 0;
  this->ExtentClipping = 0;
}

// Specify a (xmin,xmax, ymin,ymax, zmin,zmax) bounding box to clip data.
void vtkExtractUnstructuredGrid::SetExtent(float xMin,float xMax, float yMin,
                                     float yMax, float zMin, float zMax)
{
  float extent[6];

  extent[0] = xMin;
  extent[1] = xMax;
  extent[2] = yMin;
  extent[3] = yMax;
  extent[4] = zMin;
  extent[5] = zMax;

  this->SetExtent(extent);
}

// Specify a (xmin,xmax, ymin,ymax, zmin,zmax) bounding box to clip data.
void vtkExtractUnstructuredGrid::SetExtent(float *extent)
{
  int i;

  if ( extent[0] != this->Extent[0] || extent[1] != this->Extent[1] ||
  extent[2] != this->Extent[2] || extent[3] != this->Extent[3] ||
  extent[4] != this->Extent[4] || extent[5] != this->Extent[5] )
    {
    this->Modified();
    for (i=0; i<3; i++)
      {
      if ( extent[2*i+1] < extent[2*i] )
	{
	extent[2*i+1] = extent[2*i];
	}
      this->Extent[2*i] = extent[2*i];
      this->Extent[2*i+1] = extent[2*i+1];
      }
    }
}

// Extract cells and pass points and point data through. Also handles
// cell data.
void vtkExtractUnstructuredGrid::Execute()
{
  int cellId, i, newCellId, newPtId;
  vtkUnstructuredGrid *input=this->GetInput();
  int numPts=input->GetNumberOfPoints();
  int numCells=input->GetNumberOfCells();
  vtkPoints *inPts=input->GetPoints(), *newPts;
  char *cellVis;
  vtkCell *cell;
  float *x;
  vtkIdList *ptIds, *cellIds=vtkIdList::New();
  int ptId;
  vtkPointData *pd = input->GetPointData();
  vtkCellData *cd = input->GetCellData();
  int allVisible, numIds;
  vtkUnstructuredGrid *output = this->GetOutput();
  vtkPointData *outputPD = output->GetPointData();
  vtkCellData *outputCD = output->GetCellData();
  int *pointMap;
  
  vtkDebugMacro(<<"Executing geometry filter");

  if ( numPts < 1 || numCells < 1 || !inPts )
    {
    vtkErrorMacro(<<"No data to extract!");
    return;
    }

  if ( (!this->CellClipping) && (!this->PointClipping) && 
  (!this->ExtentClipping) )
    {
    allVisible = 1;
    cellVis = NULL;
    }
  else
    {
    allVisible = 0;
    cellVis = new char[numCells];
    }

  // Mark cells as being visible or not
  if ( ! allVisible )
    {
    for(cellId=0; cellId < numCells; cellId++)
      {
      if ( this->CellClipping && cellId < this->CellMinimum ||
      cellId > this->CellMaximum )
        {
        cellVis[cellId] = 0;
        }
      else
        {
        cell = input->GetCell(cellId);
        ptIds = cell->GetPointIds();
        numIds = ptIds->GetNumberOfIds();
        for (i=0; i < numIds; i++) 
          {
          ptId = ptIds->GetId(i);
          x = input->GetPoint(ptId);

          if ( (this->PointClipping && (ptId < this->PointMinimum ||
          ptId > this->PointMaximum) ) ||
          (this->ExtentClipping && 
          (x[0] < this->Extent[0] || x[0] > this->Extent[1] ||
          x[1] < this->Extent[2] || x[1] > this->Extent[3] ||
          x[2] < this->Extent[4] || x[2] > this->Extent[5] )) )
            {
            cellVis[cellId] = 0;
            break;
            }
          }
        if ( i >= numIds )
	  {
	  cellVis[cellId] = 1;
	  }
        }
      }
    }

  // Allocate
  newPts = vtkPoints::New();
  newPts->Allocate(numPts);
  output->Allocate(numCells);
  outputPD->CopyAllocate(pd,numPts,numPts/2);
  outputCD->CopyAllocate(cd,numCells,numCells/2);
  pointMap = new int[numPts];
  for (i=0; i<numPts; i++)
    {
    pointMap[i] = (-1); //initialize as unused
    }

  // Traverse cells to extract geometry
  for(cellId=0; cellId < numCells; cellId++)
    {
    if ( allVisible || cellVis[cellId] )
      {
      cell = input->GetCell(cellId);
      numIds = cell->PointIds->GetNumberOfIds();
      cellIds->Reset();
      for (i=0; i < numIds; i++)
	{
	ptId = cell->PointIds->GetId(i);
	if ( pointMap[ptId] < 0 )
	  {
	  pointMap[ptId] = newPtId 
	    = newPts->InsertNextPoint(inPts->GetPoint(ptId));
	  outputPD->CopyData(pd, ptId, newPtId);
	  }
	cellIds->InsertNextId(pointMap[ptId]);
	}

      newCellId = output->InsertNextCell(input->GetCellType(cellId), cellIds);
      outputCD->CopyData(cd, cellId, newCellId);
	
      } //if visible
    } //for all cells

  vtkDebugMacro(<<"Extracted " << output->GetNumberOfPoints() << " points,"
                << output->GetNumberOfCells() << " cells.");

  // Update ourselves and release memory
  output->SetPoints(newPts);
  newPts->Delete();

  output->Squeeze();

  delete [] pointMap;
  if ( cellVis )
    {
    delete [] cellVis;
    }
  cellIds->Delete();
}

void vtkExtractUnstructuredGrid::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkUnstructuredGridToUnstructuredGridFilter::PrintSelf(os,indent);

  os << indent << "Point Minimum : " << this->PointMinimum << "\n";
  os << indent << "Point Maximum : " << this->PointMaximum << "\n";

  os << indent << "Cell Minimum : " << this->CellMinimum << "\n";
  os << indent << "Cell Maximum : " << this->CellMaximum << "\n";

  os << indent << "Extent: \n";
  os << indent << "  Xmin,Xmax: (" << this->Extent[0] << ", " << this->Extent[1] << ")\n";
  os << indent << "  Ymin,Ymax: (" << this->Extent[2] << ", " << this->Extent[3] << ")\n";
  os << indent << "  Zmin,Zmax: (" << this->Extent[4] << ", " << this->Extent[5] << ")\n";

  os << indent << "PointClipping: " << (this->PointClipping ? "On\n" : "Off\n");
  os << indent << "CellClipping: " << (this->CellClipping ? "On\n" : "Off\n");
  os << indent << "ExtentClipping: " << (this->ExtentClipping ? "On\n" : "Off\n");

}

