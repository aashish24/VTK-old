/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Toolkit. No part of this file
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
#include "SGGeomF.hh"

// Description:
// Construct with initial extent (0,100, 0,100, 0,0) (i.e., a plane).
vtkStructuredGridGeometryFilter::vtkStructuredGridGeometryFilter()
{
  this->Extent[0] = 0;
  this->Extent[1] = 100;
  this->Extent[2] = 0;
  this->Extent[3] = 100;
  this->Extent[4] = 0;
  this->Extent[5] = 0;
}

void vtkStructuredGridGeometryFilter::Execute()
{
  vtkPointData *pd;
  int *dims, dimension, dir[3], diff[3];
  int i, j, k, extent[6];
  int ptIds[4], idx, startIdx;
  vtkFloatPoints *newPts=0;
  vtkCellArray *newVerts=0;
  vtkCellArray *newLines=0;
  vtkCellArray *newPolys=0;
  int totPoints, numPolys;
  int offset[3], pos;
  float *x;
  vtkStructuredGrid *input=(vtkStructuredGrid *)this->Input;

  vtkDebugMacro(<< "Extracting structured points geometry");
  this->Initialize();

  pd = input->GetPointData();
  this->PointData.CopyNormalsOff();
  dims = input->GetDimensions();
//
// Based on the dimensions of the structured data, and the extent of the geometry,
// compute the combined extent plus the dimensionality of the data
//
  for (dimension=3, i=0; i<3; i++)
    {
    extent[2*i] = this->Extent[2*i] < 0 ? 0 : this->Extent[2*i];
    extent[2*i] = this->Extent[2*i] >= dims[i] ? dims[i]-1 : this->Extent[2*i];
    extent[2*i+1] = this->Extent[2*i+1] >= dims[i] ? dims[i]-1 : this->Extent[2*i+1];
    if ( extent[2*i+1] < extent[2*i] ) extent[2*i+1] = extent[2*i];
    if ( (extent[2*i+1] - extent[2*i]) == 0 ) dimension--;
    }
//
// Now create polygonal data based on dimension of data
//
  startIdx = extent[0] + extent[2]*dims[0] + extent[4]*dims[0]*dims[1];

  switch (dimension) 
    {
    default:
      break;

    case 0: // --------------------- build point -----------------------

      if ( input->IsPointVisible(startIdx) )
        {
        newPts = new vtkFloatPoints(1);
        newVerts = new vtkCellArray;
        newVerts->Allocate(newVerts->EstimateSize(1,1));
        this->PointData.CopyAllocate(pd,1);

        ptIds[0] = newPts->InsertNextPoint(input->GetPoint(startIdx));
        this->PointData.CopyData(pd,startIdx,ptIds[0]);
        newVerts->InsertNextCell(1,ptIds);
        }
      break;

    case 1: // --------------------- build line -----------------------

      for (dir[0]=dir[1]=dir[2]=totPoints=0, i=0; i<3; i++)
        {
        if ( (diff[i] = extent[2*i+1] - extent[2*i]) > 0 ) 
          {
          dir[0] = i;
          totPoints = diff[i] + 1;
          break;
          }
        }
      newPts = new vtkFloatPoints(totPoints);
      newLines = new vtkCellArray;
      newLines->Allocate(newLines->EstimateSize(totPoints-1,2));
      this->PointData.CopyAllocate(pd,totPoints);
//
//  Load data
//
      if ( dir[0] == 0 ) 
        offset[0] = 1;
      else if (dir[0] == 1)
        offset[0] = dims[0];
      else
        offset[0] = dims[0]*dims[1];

      for (i=0; i<totPoints; i++) 
        {
        idx = startIdx + i*offset[0];
        x = input->GetPoint(idx);
        ptIds[0] = newPts->InsertNextPoint(x);
        this->PointData.CopyData(pd,idx,ptIds[0]);
        }

      for (idx=0,i=0; i<(totPoints-1); i++) 
        {
        if ( input->IsPointVisible(idx) || input->IsPointVisible(idx+offset[0]) )
          {
          ptIds[0] = i;
          ptIds[1] = i + 1;
          newLines->InsertNextCell(2,ptIds);
          }
        }
      break;

    case 2: // --------------------- build plane -----------------------
//
//  Create the data objects
//
      for (dir[0]=dir[1]=dir[2]=idx=0,i=0; i<3; i++)
        {
        if ( (diff[i] = extent[2*i+1] - extent[2*i]) != 0 )
          dir[idx++] = i;
        else
          dir[2] = i;
        }

      totPoints = (diff[dir[0]]+1) * (diff[dir[1]]+1);
      numPolys = diff[dir[0]]  * diff[dir[1]];

      newPts = new vtkFloatPoints(totPoints);
      newPolys = new vtkCellArray;
      newPolys->Allocate(newLines->EstimateSize(numPolys,4));
      this->PointData.CopyAllocate(pd,totPoints);
//
//  Create polygons
//
      for (i=0; i<2; i++) 
        {
        if ( dir[i] == 0 )
          offset[i] = 1;
        else if ( dir[i] == 1 )
          offset[i] = dims[0];
        else if ( dir[i] == 2 )
          offset[i] = dims[0]*dims[1];
        }

      // create points whether visible or not.  Makes coding easier but generates
      // extra data.
      for (pos=startIdx, j=0; j < (diff[dir[1]]+1); j++) 
        {
        for (i=0; i < (diff[dir[0]]+1); i++) 
          {
          idx = pos + i*offset[0];
          x = input->GetPoint(idx);
          ptIds[0] = newPts->InsertNextPoint(x);
          this->PointData.CopyData(pd,idx,ptIds[0]);
          }
        pos += offset[1];
        }

      // create any polygon who has a visible vertex.  To turn off a polygon, all 
      // vertices have to be blanked.
      for (pos=startIdx, j=0; j < diff[dir[1]]; j++) 
        {
        for (i=0; i < diff[dir[0]]; i++) 
          {
          if (input->IsPointVisible(pos+i*offset[0])
          || input->IsPointVisible(pos+(i+1)*offset[0])
          || input->IsPointVisible(pos+i*offset[0]+offset[1]) 
          || input->IsPointVisible(pos+(i+1)*offset[0]+offset[1]) ) 
            {
            ptIds[0] = i + j*(diff[dir[0]]+1);
            ptIds[1] = ptIds[0] + 1;
            ptIds[2] = ptIds[1] + diff[dir[0]] + 1;
            ptIds[3] = ptIds[2] - 1;
            newPolys->InsertNextCell(4,ptIds);
            }
          }
        pos += offset[1];
        }
      break;

    case 3: // ------------------- grab points in volume  --------------

//
// Create data objects
//
      for (i=0; i<3; i++) diff[i] = extent[2*i+1] - extent[2*i];

      totPoints = (diff[0]+1) * (diff[1]+1) * (diff[2]+1);

      newPts = new vtkFloatPoints(totPoints);
      newVerts = new vtkCellArray;
      newVerts->Allocate(newVerts->EstimateSize(totPoints,1));
      this->PointData.CopyAllocate(pd,totPoints);
//
// Create vertices
//
      offset[0] = dims[0];
      offset[1] = dims[0]*dims[1];

      for (pos=startIdx, k=0; k < (diff[2]+1); k++) 
        {
        for (j=0; j < (diff[1]+1); j++) 
          {
          pos = startIdx + j*offset[0] + k*offset[1];
          for (i=0; i < (diff[0]+1); i++) 
            {
            if ( input->IsPointVisible(pos+i) ) 
              {
              x = input->GetPoint(pos+i);
              ptIds[0] = newPts->InsertNextPoint(x);
              this->PointData.CopyData(pd,idx,ptIds[0]);
              newVerts->InsertNextCell(1,ptIds);
              }
            }
          }
        }
        break; /* end this case */

    } // switch
//
// Update self and release memory
//
  if (newPts)
    {
    this->SetPoints(newPts);
    newPts->Delete();
    }

  if (newVerts)
    {
    this->SetVerts(newVerts);
    newVerts->Delete();
    }

  if (newLines)
    {
    this->SetLines(newLines);
    newLines->Delete();
    }

  if (newPolys)
    {
    this->SetPolys(newPolys);
    newPolys->Delete();
    }
}

void vtkStructuredGridGeometryFilter::SetExtent(int iMin, int iMax, int jMin, int jMax, 
                                   int kMin, int kMax)
{
  int extent[6];

  extent[0] = iMin;
  extent[1] = iMax;
  extent[2] = jMin;
  extent[3] = jMax;
  extent[4] = kMin;
  extent[5] = kMax;

  this->SetExtent(extent);
}

// Description:
// Specify (imin,imax, jmin,jmax, kmin,kmax) indices.
void vtkStructuredGridGeometryFilter::SetExtent(int *extent)
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

void vtkStructuredGridGeometryFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkStructuredGridToPolyFilter::PrintSelf(os,indent);

  os << indent << "Extent: \n";
  os << indent << "  Imin,Imax: (" << this->Extent[0] << ", " << this->Extent[1] << ")\n";
  os << indent << "  Jmin,Jmax: (" << this->Extent[2] << ", " << this->Extent[3] << ")\n";
  os << indent << "  Kmin,Kmax: (" << this->Extent[4] << ", " << this->Extent[5] << ")\n";
}
