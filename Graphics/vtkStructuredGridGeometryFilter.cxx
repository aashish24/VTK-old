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
#include "vtkStructuredGridGeometryFilter.h"

// Construct with initial extent of all the data
vtkStructuredGridGeometryFilter::vtkStructuredGridGeometryFilter()
{
  this->Extent[0] = 0;
  this->Extent[1] = VTK_LARGE_INTEGER;
  this->Extent[2] = 0;
  this->Extent[3] = VTK_LARGE_INTEGER;
  this->Extent[4] = 0;
  this->Extent[5] = VTK_LARGE_INTEGER;
}

void vtkStructuredGridGeometryFilter::Execute()
{
  int *dims, dimension, dir[3], diff[3];
  int i, j, k, extent[6];
  int ptIds[4], idx, startIdx, startCellIdx;
  int cellId;
  vtkPoints *newPts=0;
  vtkCellArray *newVerts=0;
  vtkCellArray *newLines=0;
  vtkCellArray *newPolys=0;
  int totPoints, numPolys;
  int offset[3], pos;
  float *x;
  vtkPointData *pd, *outPD;
  vtkCellData *cd, *outCD;
  vtkStructuredGrid *input=(vtkStructuredGrid *)this->Input;
  vtkPolyData *output=(vtkPolyData *)this->Output;

  vtkDebugMacro(<< "Extracting structured points geometry");

  if ( input->GetPoints() == NULL)
    {
    vtkErrorMacro(<<"No data to extract");
    return;
    }

  pd = input->GetPointData();
  outPD = output->GetPointData();
  outPD->CopyNormalsOff();
  cd = input->GetCellData();
  outCD = output->GetCellData();
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
    if ( extent[2*i+1] < extent[2*i] )
      {
      extent[2*i+1] = extent[2*i];
      }
    if ( (extent[2*i+1] - extent[2*i]) == 0 )
      {
      dimension--;
      }
    }
//
// Now create polygonal data based on dimension of data
//
// First compute starting index of the point and cell
  startIdx = extent[0] + extent[2]*dims[0] + extent[4]*dims[0]*dims[1];

// The cell index is a bit more complicated at the boundaries
  startCellIdx =  (extent[0] < dims[0] - 1) ? extent[0]
                                            : extent[0]-1;
  startCellIdx += (extent[2] < dims[1] - 1) ? extent[2]*(dims[0]-1)
                                            : (extent[2]-1)*(dims[0]-1);
  startCellIdx += (extent[4] < dims[2] - 1) ? extent[4]*(dims[0]-1)*(dims[1]-1)
                                            : (extent[4]-1)*(dims[0]-1)*(dims[1]-1);

  switch (dimension) 
    {
    default:
      break;

    case 0: // --------------------- build point -----------------------

      if ( input->IsPointVisible(startIdx) )
        {
        newPts = vtkPoints::New();
        newPts->Allocate(1);
        newVerts = vtkCellArray::New();
        newVerts->Allocate(newVerts->EstimateSize(1,1));
        outPD->CopyAllocate(pd,1);
        outCD->CopyAllocate(cd,1);

        ptIds[0] = newPts->InsertNextPoint(input->GetPoint(startIdx));
        outPD->CopyData(pd,startIdx,ptIds[0]);

        cellId = newVerts->InsertNextCell(1,ptIds);
        outCD->CopyData(cd,startIdx,cellId);
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
      newPts = vtkPoints::New();
      newPts->Allocate(totPoints);
      newLines = vtkCellArray::New();
      newLines->Allocate(newLines->EstimateSize(totPoints-1,2));
      outPD->CopyAllocate(pd,totPoints);
      outCD->CopyAllocate(cd,totPoints - 1);
//
//  Load data
//
      if ( dir[0] == 0 ) 
	{
        offset[0] = 1;
	}
      else if (dir[0] == 1)
	{
        offset[0] = dims[0];
	}
      else
	{
        offset[0] = dims[0]*dims[1];
	}

      for (i=0; i<totPoints; i++) 
        {
        idx = startIdx + i*offset[0];
        x = input->GetPoint(idx);
        ptIds[0] = newPts->InsertNextPoint(x);
        outPD->CopyData(pd,idx,ptIds[0]);
        }

      if ( dir[0] == 0 ) 
	{
        offset[0] = 1;
	}
      else if (dir[0] == 1)
	{
        offset[0] = dims[0] - 1;
	}
      else
	{
        offset[0] = (dims[0] - 1) * (dims[1] - 1);
	}

      for (i=0; i<(totPoints-1); i++) 
        {
        if ( input->IsPointVisible(idx) || input->IsPointVisible(idx+offset[0]) )
          {
          idx = startIdx + i*offset[0];
          ptIds[0] = i;
          ptIds[1] = i + 1;
          cellId = newLines->InsertNextCell(2,ptIds);
          outCD->CopyData(cd,idx,cellId);
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
	  {
          dir[idx++] = i;
	  }
        else
	  {
          dir[2] = i;
	  }
        }

      totPoints = (diff[dir[0]]+1) * (diff[dir[1]]+1);
      numPolys = diff[dir[0]]  * diff[dir[1]];

      newPts = vtkPoints::New();
      newPts->Allocate(totPoints);
      newPolys = vtkCellArray::New();
      newPolys->Allocate(newLines->EstimateSize(numPolys,4));
      outPD->CopyAllocate(pd,totPoints);
      outCD->CopyAllocate(cd,numPolys);
//
//  Create polygons
//
      for (i=0; i<2; i++) 
        {
        if ( dir[i] == 0 )
	  {
          offset[i] = 1;
	  }
        else if ( dir[i] == 1 )
	  {
          offset[i] = dims[0];
	  }
        else if ( dir[i] == 2 )
	  {
          offset[i] = dims[0]*dims[1];
	  }
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
          outPD->CopyData(pd,idx,ptIds[0]);
          }
        pos += offset[1];
        }

      // create any polygon who has a visible vertex.  To turn off a polygon, all 
      // vertices have to be blanked.
      for (i=0; i<2; i++) 
        {
        if ( dir[i] == 0 )
	  {
          offset[i] = 1;
	  }
        else if ( dir[i] == 1 )
	  {
          offset[i] = (dims[0] - 1);
	  }
        else if ( dir[i] == 2 )
	  {
          offset[i] = (dims[0] - 1) * (dims[1] - 1);
	  }
        }

      for (pos=startCellIdx, j=0; j < diff[dir[1]]; j++) 
        {
        for (i=0; i < diff[dir[0]]; i++) 
          {
          if (input->IsPointVisible(pos+i*offset[0])
          || input->IsPointVisible(pos+(i+1)*offset[0])
          || input->IsPointVisible(pos+i*offset[0]+offset[1]) 
          || input->IsPointVisible(pos+(i+1)*offset[0]+offset[1]) ) 
            {
            idx = pos + i*offset[0];
            ptIds[0] = i + j*(diff[dir[0]]+1);
            ptIds[1] = ptIds[0] + 1;
            ptIds[2] = ptIds[1] + diff[dir[0]] + 1;
            ptIds[3] = ptIds[2] - 1;
            cellId = newPolys->InsertNextCell(4,ptIds);
            outCD->CopyData(cd,idx,cellId);
            }
          }
        pos += offset[1];
        }
      break;

    case 3: // ------------------- grab points in volume  --------------

//
// Create data objects
//
      for (i=0; i<3; i++)
	{
	diff[i] = extent[2*i+1] - extent[2*i];
	}

      totPoints = (diff[0]+1) * (diff[1]+1) * (diff[2]+1);

      newPts = vtkPoints::New();
      newPts->Allocate(totPoints);
      newVerts = vtkCellArray::New();
      newVerts->Allocate(newVerts->EstimateSize(totPoints,1));
      outPD->CopyAllocate(pd,totPoints);
      outCD->CopyAllocate(cd,totPoints);
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
              outPD->CopyData(pd,pos+i,ptIds[0]);
              cellId = newVerts->InsertNextCell(1,ptIds);
              outCD->CopyData(cd,pos+i,cellId);
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
    output->SetPoints(newPts);
    newPts->Delete();
    }

  if (newVerts)
    {
    output->SetVerts(newVerts);
    newVerts->Delete();
    }

  if (newLines)
    {
    output->SetLines(newLines);
    newLines->Delete();
    }

  if (newPolys)
    {
    output->SetPolys(newPolys);
    newPolys->Delete();
    }
}

// Specify (imin,imax, jmin,jmax, kmin,kmax) indices.
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

// Specify (imin,imax, jmin,jmax, kmin,kmax) indices in array form.
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
      if ( extent[2*i] < 0 )
	{
	extent[2*i] = 0;
	}
      if ( extent[2*i+1] < extent[2*i] )
	{
	extent[2*i+1] = extent[2*i];
	}
      this->Extent[2*i] = extent[2*i];
      this->Extent[2*i+1] = extent[2*i+1];
      }
    }
}

void vtkStructuredGridGeometryFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkStructuredGridToPolyDataFilter::PrintSelf(os,indent);

  os << indent << "Extent: \n";
  os << indent << "  Imin,Imax: (" << this->Extent[0] << ", " << this->Extent[1] << ")\n";
  os << indent << "  Jmin,Jmax: (" << this->Extent[2] << ", " << this->Extent[3] << ")\n";
  os << indent << "  Kmin,Kmax: (" << this->Extent[4] << ", " << this->Extent[5] << ")\n";
}
