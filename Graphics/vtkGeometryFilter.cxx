/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1995 Ken Martin, Will Schroeder, Bill Lorensen.

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
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.


=========================================================================*/
#include "GeomF.hh"

// Description:
// Construct with all types of clipping turned off.
vtkGeometryFilter::vtkGeometryFilter()
{
  this->PointMinimum = 0;
  this->PointMaximum = LARGE_INTEGER;

  this->CellMinimum = 0;
  this->CellMaximum = LARGE_INTEGER;

  this->Extent[0] = -LARGE_FLOAT;
  this->Extent[1] = LARGE_FLOAT;
  this->Extent[2] = -LARGE_FLOAT;
  this->Extent[3] = LARGE_FLOAT;
  this->Extent[4] = -LARGE_FLOAT;
  this->Extent[5] = LARGE_FLOAT;

  this->PointClipping = 0;
  this->CellClipping = 0;
  this->ExtentClipping = 0;
}

void vtkGeometryFilter::SetExtent(float xMin, float xMax, float yMin,
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

// Description:
// Specify a (xmin,xmax, ymin,ymax, zmin,zmax) bounding box to clip data.
void vtkGeometryFilter::SetExtent(float *extent)
{
  int i;

  if ( extent[0] != this->Extent[0] || extent[1] != this->Extent[1] ||
  extent[2] != this->Extent[2] || extent[3] != this->Extent[3] ||
  extent[4] != this->Extent[4] || extent[5] != this->Extent[5] )
    {
    this->Modified();
    for (i=0; i<3; i++)
      {
      if ( extent[2*i] < 0 ) extent[2*i] = 0;
      if ( extent[2*i+1] < extent[2*i] ) extent[2*i+1] = extent[2*i];
      this->Extent[2*i] = extent[2*i];
      this->Extent[2*i+1] = extent[2*i+1];
      }
    }
}

void vtkGeometryFilter::Execute()
{
  int cellId, i, j;
  int numPts=this->Input->GetNumberOfPoints();
  int numCells=this->Input->GetNumberOfCells();
  char *cellVis;
  vtkCell *cell, *face, *cellCopy;
  float *x;
  vtkIdList *ptIds;
  static vtkIdList cellIds(MAX_CELL_SIZE);
  vtkFloatPoints *newPts;
  int ptId;
  int npts, pts[MAX_CELL_SIZE];
  vtkPointData *pd = this->Input->GetPointData();
  int allVisible;

  this->Initialize();

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
//
// Mark cells as being visible or not
//
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
        cell = this->Input->GetCell(cellId);
        ptIds = cell->GetPointIds();
        for (i=0; i < ptIds->GetNumberOfIds(); i++) 
          {
          ptId = ptIds->GetId(i);
          x = this->Input->GetPoint(ptId);

          if ( (this->PointClipping && (ptId < this->PointMinimum ||
          ptId > this->PointMaximum) ) &&
          (this->ExtentClipping && 
          (x[0] < this->Extent[0] || x[0] > this->Extent[1] ||
          x[1] < this->Extent[2] || x[1] > this->Extent[3] ||
          x[2] < this->Extent[4] || x[2] > this->Extent[5] )) )
            {
            cellVis[cellId] = 0;
            break;
            }
          }
        if ( i >= ptIds->GetNumberOfIds() ) cellVis[cellId] = 1;
        }
      }
    }
//
// Allocate
//
  newPts = new vtkFloatPoints(numPts,numPts/2);
  this->Allocate(4*numCells,numCells/2);
  this->PointData.CopyAllocate(pd,numPts,numPts/2);
//
// Traverse cells to extract geometry
//
  for(cellId=0; cellId < numCells; cellId++)
    {
    if ( allVisible || cellVis[cellId] )
      {
      cell = this->Input->GetCell(cellId);
      switch (cell->GetCellDimension())
        {
        // create new points and then cell
        case 0: case 1: case 2:
          
          npts = cell->GetNumberOfPoints();
          for ( i=0; i < npts; i++)
            {
            ptId = cell->GetPointId(i);
            x = this->Input->GetPoint(ptId);
            pts[i] = newPts->InsertNextPoint(x);
            this->PointData.CopyData(pd,ptId,pts[i]);
            }
          this->InsertNextCell(cell->GetCellType(), npts, pts);
          break;

        case 3:
          cellCopy = cell->MakeObject();
          for (j=0; j < cellCopy->GetNumberOfFaces(); j++)
            {
            face = cellCopy->GetFace(j);
            this->Input->GetCellNeighbors(cellId, face->PointIds, cellIds);
            if ( cellIds.GetNumberOfIds() <= 0 || 
            (!allVisible && !cellVis[cellIds.GetId(0)]) )
              {
              npts = face->GetNumberOfPoints();
              for ( i=0; i < npts; i++)
                {
                ptId = face->GetPointId(i);
                x = this->Input->GetPoint(ptId);
                pts[i] = newPts->InsertNextPoint(x);
                this->PointData.CopyData(pd,ptId,pts[i]);
                }
              this->InsertNextCell(face->GetCellType(), npts, pts);
              }
            }
            cellCopy->Delete();
          break;

        } //switch
      } //if visible
    } //for all cells
//
// Update ourselves and release memory
//
  this->SetPoints(newPts);
  newPts->Delete();

  this->Squeeze();

  if ( cellVis ) delete [] cellVis;
}

void vtkGeometryFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataSetToPolyFilter::PrintSelf(os,indent);

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

