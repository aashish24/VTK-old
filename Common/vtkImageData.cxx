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
#include "vtkImageData.h"
#include "vtkVertex.h"
#include "vtkLine.h"
#include "vtkPixel.h"
#include "vtkVoxel.h"
#include "vtkImageToStructuredPoints.h"
#include "vtkStructuredExtent.h"
#include "vtkImageInformation.h"
#include "vtkObjectFactory.h"



//----------------------------------------------------------------------------
vtkImageData* vtkImageData::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageData");
  if (ret)
    {
    return (vtkImageData*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageData;
}




//----------------------------------------------------------------------------
vtkImageData::vtkImageData()
{
  int idx;
  
  this->Vertex = vtkVertex::New();
  this->Line = vtkLine::New();
  this->Pixel = vtkPixel::New();
  this->Voxel = vtkVoxel::New();
  
  // I do not like defaulting to one pixel, but it avoids
  // alot of special case checking.
  this->Dimensions[0] = 1;
  this->Dimensions[1] = 1;
  this->Dimensions[2] = 1;
  this->DataDescription = VTK_SINGLE_POINT;
  
  for (idx = 0; idx < 3; ++idx)
    {
    this->Extent[idx*2] = 0;
    this->Extent[idx*2+1] = 0;    
    this->Increments[idx] = 0;
    }
  
  // for automatic conversion
  this->ImageToStructuredPoints = NULL;

  // Delete the generic information object created by the superclass.
  this->Information->Delete();
  this->Information = vtkImageInformation::New();

  this->UpdateExtent->Delete();
  this->UpdateExtent = vtkStructuredExtent::New();  
}

//----------------------------------------------------------------------------
vtkImageData::vtkImageData(const vtkImageData& v) :
vtkDataSet(v)
{
  vtkImageInformation *vInfo;
  
  vInfo = (vtkImageInformation *)(v.Information);
  
  this->Dimensions[0] = v.Dimensions[0];
  this->Dimensions[1] = v.Dimensions[1];
  this->Dimensions[2] = v.Dimensions[2];

  this->Extent[0] = v.Extent[0];
  this->Extent[1] = v.Extent[1];
  this->Extent[2] = v.Extent[2];
  this->Extent[3] = v.Extent[3];
  this->Extent[4] = v.Extent[4];
  this->Extent[5] = v.Extent[5];

  this->DataDescription = v.DataDescription;
  
  this->SetSpacing(vInfo->GetSpacing());
  this->SetOrigin(vInfo->GetOrigin());
}

//----------------------------------------------------------------------------
vtkImageData::~vtkImageData()
{
  this->Vertex->Delete();
  this->Line->Delete();
  this->Pixel->Delete();
  this->Voxel->Delete();
}

//----------------------------------------------------------------------------
// Copy the geometric and topological structure of an input structured points 
// object.
void vtkImageData::CopyStructure(vtkDataSet *ds)
{
  vtkImageData *sPts=(vtkImageData *)ds;
  this->Initialize();

  for (int i=0; i<3; i++)
    {
    this->Extent[i] = sPts->Extent[i];
    this->Extent[i+3] = sPts->Extent[i+3];
    this->Dimensions[i] = sPts->Dimensions[i];
    }
  this->DataDescription = sPts->DataDescription;
  this->CopyInformation(sPts);
}


//----------------------------------------------------------------------------
vtkCell *vtkImageData::GetCell(int cellId)
{
  vtkCell *cell = NULL;
  int idx, loc[3], npts;
  int iMin, iMax, jMin, jMax, kMin, kMax;
  int d01 = this->Dimensions[0]*this->Dimensions[1];
  float x[3];
  float *origin = this->GetOrigin();
  float *spacing = this->GetSpacing();

  iMin = iMax = jMin = jMax = kMin = kMax = 0;
  
  switch (this->DataDescription)
    {
    case VTK_SINGLE_POINT: // cellId can only be = 0
      cell = this->Vertex;
      break;

    case VTK_X_LINE:
      iMin = cellId;
      iMax = cellId + 1;
      cell = this->Line;
      break;

    case VTK_Y_LINE:
      jMin = cellId;
      jMax = cellId + 1;
      cell = this->Line;
      break;

    case VTK_Z_LINE:
      kMin = cellId;
      kMax = cellId + 1;
      cell = this->Line;
      break;

    case VTK_XY_PLANE:
      iMin = cellId % (this->Dimensions[0]-1);
      iMax = iMin + 1;
      jMin = cellId / (this->Dimensions[0]-1);
      jMax = jMin + 1;
      cell = this->Pixel;
      break;

    case VTK_YZ_PLANE:
      jMin = cellId % (this->Dimensions[1]-1);
      jMax = jMin + 1;
      kMin = cellId / (this->Dimensions[1]-1);
      kMax = kMin + 1;
      cell = this->Pixel;
      break;

    case VTK_XZ_PLANE:
      iMin = cellId % (this->Dimensions[0]-1);
      iMax = iMin + 1;
      kMin = cellId / (this->Dimensions[0]-1);
      kMax = kMin + 1;
      cell = this->Pixel;
      break;

    case VTK_XYZ_GRID:
      iMin = cellId % (this->Dimensions[0] - 1);
      iMax = iMin + 1;
      jMin = (cellId / (this->Dimensions[0] - 1)) % (this->Dimensions[1] - 1);
      jMax = jMin + 1;
      kMin = cellId / ((this->Dimensions[0] - 1) * (this->Dimensions[1] - 1));
      kMax = kMin + 1;
      cell = this->Voxel;
      break;
    }

  // Extract point coordinates and point ids
  // Ids are relative to extent min.
  npts = 0;
  for (loc[2]=kMin; loc[2]<=kMax; loc[2]++)
    {
    x[2] = origin[2] + (loc[2]+this->Extent[4]) * spacing[2]; 
    for (loc[1]=jMin; loc[1]<=jMax; loc[1]++)
      {
      x[1] = origin[1] + (loc[1]+this->Extent[2]) * spacing[1]; 
      for (loc[0]=iMin; loc[0]<=iMax; loc[0]++)
        {
        x[0] = origin[0] + (loc[0]+this->Extent[0]) * spacing[0]; 

        idx = loc[0] + loc[1]*this->Dimensions[0] + loc[2]*d01;
        cell->PointIds->SetId(npts,idx);
        cell->Points->SetPoint(npts++,x);
        }
      }
    }

  return cell;
}





//----------------------------------------------------------------------------
int vtkImageData::ClipUpdateExtentWithWholeExtent()
{
  int valid = 1;
  int idx, minIdx, maxIdx;
  int uExt[6];
  int *wExt = this->GetImageInformation()->GetWholeExtent();
  
  this->GetStructuredUpdateExtent()->GetExtent(uExt);
  
  for (idx = 0; idx < 3; ++idx)
    {
    minIdx = 2*idx;
    maxIdx = 2*idx + 1;
    // make sure there is overlap!
    if (uExt[minIdx] > wExt[maxIdx])
      {
      valid = 0;
      vtkErrorMacro("UpdateExtent " << uExt[minIdx] 
		      << " -> " << uExt[maxIdx]  
		      << " does not overlap with WholeExtent "
		      << wExt[minIdx] << " -> " << wExt[maxIdx]);
      uExt[minIdx] = wExt[maxIdx];
      }
    if (uExt[maxIdx] < wExt[minIdx])
      {
      valid = 0;
      vtkErrorMacro("UpdateExtent " << uExt[minIdx] 
		      << " -> " << uExt[maxIdx]  
		      << " does not overlap with WholeExtent "
		      << wExt[minIdx] << " -> " << wExt[maxIdx]);
      uExt[maxIdx] = wExt[minIdx];
      }
    
    // typical intersection shift min up to whole min
    if (uExt[minIdx] < wExt[minIdx])
      {
      uExt[minIdx] = wExt[minIdx];
      }
    // typical intersection shift max down to whole max
    if (uExt[maxIdx] >= wExt[maxIdx])
      {
      uExt[maxIdx] = wExt[maxIdx];
      }
    }
  
  // Now check to see if the UpdateExtent is in the current Extent.
  
  // If the requested extent is not completely in the data structure already,
  // release the data to cause the source to execute.
  if (this->Extent[0] > uExt[0] || 
      this->Extent[1] < uExt[1] || 
      this->Extent[2] > uExt[2] || 
      this->Extent[3] < uExt[3] ||
      this->Extent[4] > uExt[4] ||
      this->Extent[5] < uExt[5])
    {
    this->ReleaseData();
    }
  
  this->GetStructuredUpdateExtent()->SetExtent(uExt);
  
  return valid;
}

//----------------------------------------------------------------------------
void vtkImageData::GetCell(int cellId, vtkGenericCell *cell)
{
  int npts, loc[3], idx;
  int iMin, iMax, jMin, jMax, kMin, kMax;
  int d01 = this->Dimensions[0]*this->Dimensions[1];
  float *origin = this->GetOrigin();
  float *spacing = this->GetSpacing();
  float x[3];

  iMin = iMax = jMin = jMax = kMin = kMax = 0;
  
  switch (this->DataDescription)
    {
    case VTK_SINGLE_POINT: // cellId can only be = 0
      cell->SetCellTypeToVertex();
      break;

    case VTK_X_LINE:
      iMin = cellId;
      iMax = cellId + 1;
      cell->SetCellTypeToLine();
      break;

    case VTK_Y_LINE:
      jMin = cellId;
      jMax = cellId + 1;
      cell->SetCellTypeToLine();
      break;

    case VTK_Z_LINE:
      kMin = cellId;
      kMax = cellId + 1;
      cell->SetCellTypeToLine();
      break;

    case VTK_XY_PLANE:
      iMin = cellId % (this->Dimensions[0]-1);
      iMax = iMin + 1;
      jMin = cellId / (this->Dimensions[0]-1);
      jMax = jMin + 1;
      cell->SetCellTypeToPixel();
      break;

    case VTK_YZ_PLANE:
      jMin = cellId % (this->Dimensions[1]-1);
      jMax = jMin + 1;
      kMin = cellId / (this->Dimensions[1]-1);
      kMax = kMin + 1;
      cell->SetCellTypeToPixel();
      break;

    case VTK_XZ_PLANE:
      iMin = cellId % (this->Dimensions[0]-1);
      iMax = iMin + 1;
      kMin = cellId / (this->Dimensions[0]-1);
      kMax = kMin + 1;
      cell->SetCellTypeToPixel();
      break;

    case VTK_XYZ_GRID:
      iMin = cellId % (this->Dimensions[0] - 1);
      iMax = iMin + 1;
      jMin = (cellId / (this->Dimensions[0] - 1)) % (this->Dimensions[1] - 1);
      jMax = jMin + 1;
      kMin = cellId / ((this->Dimensions[0] - 1) * (this->Dimensions[1] - 1));
      kMax = kMin + 1;
      cell->SetCellTypeToVoxel();
      break;
    }

  // Extract point coordinates and point ids
  for (npts=0,loc[2]=kMin; loc[2]<=kMax; loc[2]++)
    {
    x[2] = origin[2] + (loc[2]+this->Extent[4]) * spacing[2]; 
    for (loc[1]=jMin; loc[1]<=jMax; loc[1]++)
      {
      x[1] = origin[1] + (loc[1]+this->Extent[2]) * spacing[1]; 
      for (loc[0]=iMin; loc[0]<=iMax; loc[0]++)
        {
        x[0] = origin[0] + (loc[0]+this->Extent[0]) * spacing[0]; 

        idx = loc[0] + loc[1]*this->Dimensions[0] + loc[2]*d01;
        cell->PointIds->SetId(npts,idx);
        cell->Points->SetPoint(npts++,x);
        }
      }
    }
}


//----------------------------------------------------------------------------
// Fast implementation of GetCellBounds().  Bounds are calculated without
// constructing a cell.
void vtkImageData::GetCellBounds(int cellId, float bounds[6])
{
  int loc[3], iMin, iMax, jMin, jMax, kMin, kMax;
  float x[3];
  float *origin = this->GetOrigin();
  float *spacing = this->GetSpacing();

  iMin = iMax = jMin = jMax = kMin = kMax = 0;
  
  switch (this->DataDescription)
    {
    case VTK_SINGLE_POINT: // cellId can only be = 0
      break;

    case VTK_X_LINE:
      iMin = cellId;
      iMax = cellId + 1;
      break;

    case VTK_Y_LINE:
      jMin = cellId;
      jMax = cellId + 1;
      break;

    case VTK_Z_LINE:
      kMin = cellId;
      kMax = cellId + 1;
      break;

    case VTK_XY_PLANE:
      iMin = cellId % (this->Dimensions[0]-1);
      iMax = iMin + 1;
      jMin = cellId / (this->Dimensions[0]-1);
      jMax = jMin + 1;
      break;

    case VTK_YZ_PLANE:
      jMin = cellId % (this->Dimensions[1]-1);
      jMax = jMin + 1;
      kMin = cellId / (this->Dimensions[1]-1);
      kMax = kMin + 1;
      break;

    case VTK_XZ_PLANE:
      iMin = cellId % (this->Dimensions[0]-1);
      iMax = iMin + 1;
      kMin = cellId / (this->Dimensions[0]-1);
      kMax = kMin + 1;
      break;

    case VTK_XYZ_GRID:
      iMin = cellId % (this->Dimensions[0] - 1);
      iMax = iMin + 1;
      jMin = (cellId / (this->Dimensions[0] - 1)) % (this->Dimensions[1] - 1);
      jMax = jMin + 1;
      kMin = cellId / ((this->Dimensions[0] - 1) * (this->Dimensions[1] - 1));
      kMax = kMin + 1;
      break;
    }


  bounds[0] = bounds[2] = bounds[4] =  VTK_LARGE_FLOAT;
  bounds[1] = bounds[3] = bounds[5] = -VTK_LARGE_FLOAT;
  
  // Extract point coordinates
  for (loc[2]=kMin; loc[2]<=kMax; loc[2]++)
    {
    x[2] = origin[2] + (loc[2]+this->Extent[4]) * spacing[2]; 
    bounds[4] = (x[2] < bounds[4] ? x[2] : bounds[4]);
    bounds[5] = (x[2] > bounds[5] ? x[2] : bounds[5]);
    }
  for (loc[1]=jMin; loc[1]<=jMax; loc[1]++)
    {
    x[1] = origin[1] + (loc[1]+this->Extent[2]) * spacing[1]; 
    bounds[2] = (x[1] < bounds[2] ? x[1] : bounds[2]);
    bounds[3] = (x[1] > bounds[3] ? x[1] : bounds[3]);
    }
  for (loc[0]=iMin; loc[0]<=iMax; loc[0]++)
    {
    x[0] = origin[0] + (loc[0]+this->Extent[0]) * spacing[0]; 
    bounds[0] = (x[0] < bounds[0] ? x[0] : bounds[0]);
    bounds[1] = (x[0] > bounds[1] ? x[0] : bounds[1]);
    }
}

//----------------------------------------------------------------------------
float *vtkImageData::GetPoint(int ptId)
{
  static float x[3];
  int i, loc[3];
  float *origin = this->GetOrigin();
  float *spacing = this->GetSpacing();
  
  switch (this->DataDescription)
    {
    case VTK_SINGLE_POINT: 
      loc[0] = loc[1] = loc[2] = 0;
      break;

    case VTK_X_LINE:
      loc[1] = loc[2] = 0;
      loc[0] = ptId;
      break;

    case VTK_Y_LINE:
      loc[0] = loc[2] = 0;
      loc[1] = ptId;
      break;

    case VTK_Z_LINE:
      loc[0] = loc[1] = 0;
      loc[2] = ptId;
      break;

    case VTK_XY_PLANE:
      loc[2] = 0;
      loc[0] = ptId % this->Dimensions[0];
      loc[1] = ptId / this->Dimensions[0];
      break;

    case VTK_YZ_PLANE:
      loc[0] = 0;
      loc[1] = ptId % this->Dimensions[1];
      loc[2] = ptId / this->Dimensions[1];
      break;

    case VTK_XZ_PLANE:
      loc[1] = 0;
      loc[0] = ptId % this->Dimensions[0];
      loc[2] = ptId / this->Dimensions[0];
      break;

    case VTK_XYZ_GRID:
      loc[0] = ptId % this->Dimensions[0];
      loc[1] = (ptId / this->Dimensions[0]) % this->Dimensions[1];
      loc[2] = ptId / (this->Dimensions[0]*this->Dimensions[1]);
      break;
    }

  for (i=0; i<3; i++)
    {
    x[i] = origin[i] + (loc[i]+this->Extent[i*2]) * spacing[i];
    }

  return x;
}

//----------------------------------------------------------------------------
int vtkImageData::FindPoint(float x[3])
{
  int i, loc[3];
  float d;
  float *origin = this->GetOrigin();
  float *spacing = this->GetSpacing();

  //
  //  Compute the ijk location
  //
  for (i=0; i<3; i++) 
    {
    d = x[i] - origin[i];
    loc[i] = (int) ((d / spacing[i]) + 0.5);
    if ( loc[i] < this->Extent[i*2] || loc[i] > this->Extent[i*2+1] )
      {
      return -1;
      } 
    // since point id is relative to the first point actually stored
    loc[i] -= this->Extent[i*2];
    }
  //
  //  From this location get the point id
  //
  return loc[2]*this->Dimensions[0]*this->Dimensions[1] +
         loc[1]*this->Dimensions[0] + loc[0];
  
}

//----------------------------------------------------------------------------
int vtkImageData::FindCell(float x[3], vtkCell *vtkNotUsed(cell), 
				  vtkGenericCell *vtkNotUsed(gencell),
				  int vtkNotUsed(cellId), 
				  float vtkNotUsed(tol2), 
				  int& subId, float pcoords[3], 
				  float *weights)
{
  return
    this->FindCell( x, (vtkCell *)NULL, 0, 0.0, subId, pcoords, weights );
}

//----------------------------------------------------------------------------
int vtkImageData::FindCell(float x[3], vtkCell *vtkNotUsed(cell), 
                         int vtkNotUsed(cellId), float vtkNotUsed(tol2), 
                         int& subId, float pcoords[3], float *weights)
{
  int loc[3];

  if ( this->ComputeStructuredCoordinates(x, loc, pcoords) == 0 )
    {
    return -1;
    }

  vtkVoxel::InterpolationFunctions(pcoords,weights);

  //
  //  From this location get the cell id
  //
  subId = 0;
  return loc[2] * (this->Dimensions[0]-1)*(this->Dimensions[1]-1) +
         loc[1] * (this->Dimensions[0]-1) + loc[0];
}

//----------------------------------------------------------------------------
vtkCell *vtkImageData::FindAndGetCell(float x[3],
		vtkCell *vtkNotUsed(cell), int vtkNotUsed(cellId),
	        float vtkNotUsed(tol2), int& subId, 
                float pcoords[3], float *weights)
{
  int i, j, k, loc[3];
  int npts, idx;
  int d01 = this->Dimensions[0]*this->Dimensions[1];
  float xOut[3];
  int iMax, jMax, kMax;
  vtkCell *cell = NULL;
  float *origin = this->GetOrigin();
  float *spacing = this->GetSpacing();

  if ( this->ComputeStructuredCoordinates(x, loc, pcoords) == 0 )
    {
    return NULL;
    }

  //
  // Get the parametric coordinates and weights for interpolation
  //
  switch (this->DataDescription)
    {
    case VTK_SINGLE_POINT: // cellId can only be = 0
      vtkVertex::InterpolationFunctions(pcoords,weights);
      iMax = loc[0];
      jMax = loc[1];
      kMax = loc[2];
      cell = this->Vertex;
      break;

    case VTK_X_LINE:
      vtkLine::InterpolationFunctions(pcoords,weights);
      iMax = loc[0] + 1;
      jMax = loc[1];
      kMax = loc[2];
      cell = this->Line;
      break;

    case VTK_Y_LINE:
      vtkLine::InterpolationFunctions(pcoords,weights);
      iMax = loc[0];
      jMax = loc[1] + 1;
      kMax = loc[2];
      cell = this->Line;
      break;

    case VTK_Z_LINE:
      vtkLine::InterpolationFunctions(pcoords,weights);
      iMax = loc[0];
      jMax = loc[1];
      kMax = loc[2] + 1;
      cell = this->Line;
      break;

    case VTK_XY_PLANE:
      vtkPixel::InterpolationFunctions(pcoords,weights);
      iMax = loc[0] + 1;
      jMax = loc[1] + 1;
      kMax = loc[2];
      cell = this->Pixel;
      break;

    case VTK_YZ_PLANE:
      vtkPixel::InterpolationFunctions(pcoords,weights);
      iMax = loc[0];
      jMax = loc[1] + 1;
      kMax = loc[2] + 1;
      cell = this->Pixel;
      break;

    case VTK_XZ_PLANE:
      vtkPixel::InterpolationFunctions(pcoords,weights);
      iMax = loc[0] + 1;
      jMax = loc[1];
      kMax = loc[2] + 1;
      cell = this->Pixel;
      break;

    case VTK_XYZ_GRID:
      vtkVoxel::InterpolationFunctions(pcoords,weights);
      iMax = loc[0] + 1;
      jMax = loc[1] + 1;
      kMax = loc[2] + 1;
      cell = this->Voxel;
      break;
    }

  npts = 0;
  for (k = loc[2]; k <= kMax; k++)
    {
    xOut[2] = origin[2] + k * spacing[2]; 
    for (j = loc[1]; j <= jMax; j++)
      {
      xOut[1] = origin[1] + j * spacing[1]; 
      // make idx relative to the extent not the whole extent
      idx = loc[0]-this->Extent[0] + (j-this->Extent[2])*this->Dimensions[0]
	+ (k-this->Extent[4])*d01;
      for (i = loc[0]; i <= iMax; i++, idx++)
        {
        xOut[0] = origin[0] + i * spacing[0]; 

        cell->PointIds->SetId(npts,idx);
        cell->Points->SetPoint(npts++,xOut);
        }
      }
    }
  subId = 0;

  return cell;
}

//----------------------------------------------------------------------------
int vtkImageData::GetCellType(int vtkNotUsed(cellId))
{
  switch (this->DataDescription)
    {
    case VTK_SINGLE_POINT: 
      return VTK_VERTEX;

    case VTK_X_LINE: case VTK_Y_LINE: case VTK_Z_LINE:
      return VTK_LINE;

    case VTK_XY_PLANE: case VTK_YZ_PLANE: case VTK_XZ_PLANE:
      return VTK_PIXEL;

    case VTK_XYZ_GRID:
      return VTK_VOXEL;

    default:
      vtkErrorMacro(<<"Bad data description!");
      return VTK_EMPTY_CELL;
    }
}

//----------------------------------------------------------------------------
void vtkImageData::ComputeBounds()
{
  float *origin = this->GetOrigin();
  float *spacing = this->GetSpacing();
  
  this->Bounds[0] = origin[0] + (this->Extent[0] * spacing[0]);
  this->Bounds[2] = origin[1] + (this->Extent[2] * spacing[1]);
  this->Bounds[4] = origin[2] + (this->Extent[4] * spacing[2]);

  this->Bounds[1] = origin[0] + (this->Extent[1] * spacing[0]);
  this->Bounds[3] = origin[1] + (this->Extent[3] * spacing[1]);
  this->Bounds[5] = origin[2] + (this->Extent[5] * spacing[2]);
}

//----------------------------------------------------------------------------
// Given structured coordinates (i,j,k) for a voxel cell, compute the eight 
// gradient values for the voxel corners. The order in which the gradient
// vectors are arranged corresponds to the ordering of the voxel points. 
// Gradient vector is computed by central differences (except on edges of 
// volume where forward difference is used). The scalars s are the scalars
// from which the gradient is to be computed. This method will treat 
// only 3D structured point datasets (i.e., volumes).
void vtkImageData::GetVoxelGradient(int i, int j, int k, vtkScalars *s, 
                                          vtkVectors *g)
{
  float gv[3];
  int ii, jj, kk, idx=0;

  for ( kk=0; kk < 2; kk++)
    {
    for ( jj=0; jj < 2; jj++)
      {
      for ( ii=0; ii < 2; ii++)
        {
        this->GetPointGradient(i+ii, j+jj, k+kk, s, gv);
        g->SetVector(idx++, gv);
        }
      } 
    }
}

//----------------------------------------------------------------------------
// Given structured coordinates (i,j,k) for a point in a structured point 
// dataset, compute the gradient vector from the scalar data at that point. 
// The scalars s are the scalars from which the gradient is to be computed.
// This method will treat structured point datasets of any dimension.
void vtkImageData::GetPointGradient(int i,int j,int k, vtkScalars *s, 
                                          float g[3])
{
  int *dims=this->Dimensions;
  float *ar=this->GetSpacing();
  int ijsize=dims[0]*dims[1];
  float sp, sm;

  // x-direction
  if ( dims[0] == 1 )
    {
    g[0] = 0.0;
    }
  else if ( i == 0 )
    {
    sp = s->GetScalar(i+1 + j*dims[0] + k*ijsize);
    sm = s->GetScalar(i + j*dims[0] + k*ijsize);
    g[0] = (sm - sp) / ar[0];
    }
  else if ( i == (dims[0]-1) )
    {
    sp = s->GetScalar(i + j*dims[0] + k*ijsize);
    sm = s->GetScalar(i-1 + j*dims[0] + k*ijsize);
    g[0] = (sm - sp) / ar[0];
    }
  else
    {
    sp = s->GetScalar(i+1 + j*dims[0] + k*ijsize);
    sm = s->GetScalar(i-1 + j*dims[0] + k*ijsize);
    g[0] = 0.5 * (sm - sp) / ar[0];
    }

  // y-direction
  if ( dims[1] == 1 )
    {
    g[1] = 0.0;
    }
  else if ( j == 0 )
    {
    sp = s->GetScalar(i + (j+1)*dims[0] + k*ijsize);
    sm = s->GetScalar(i + j*dims[0] + k*ijsize);
    g[1] = (sm - sp) / ar[1];
    }
  else if ( j == (dims[1]-1) )
    {
    sp = s->GetScalar(i + j*dims[0] + k*ijsize);
    sm = s->GetScalar(i + (j-1)*dims[0] + k*ijsize);
    g[1] = (sm - sp) / ar[1];
    }
  else
    {
    sp = s->GetScalar(i + (j+1)*dims[0] + k*ijsize);
    sm = s->GetScalar(i + (j-1)*dims[0] + k*ijsize);
    g[1] = 0.5 * (sm - sp) / ar[1];
    }

  // z-direction
  if ( dims[2] == 1 )
    {
    g[2] = 0.0;
    }
  else if ( k == 0 )
    {
    sp = s->GetScalar(i + j*dims[0] + (k+1)*ijsize);
    sm = s->GetScalar(i + j*dims[0] + k*ijsize);
    g[2] = (sm - sp) / ar[2];
    }
  else if ( k == (dims[2]-1) )
    {
    sp = s->GetScalar(i + j*dims[0] + k*ijsize);
    sm = s->GetScalar(i + j*dims[0] + (k-1)*ijsize);
    g[2] = (sm - sp) / ar[2];
    }
  else
    {
    sp = s->GetScalar(i + j*dims[0] + (k+1)*ijsize);
    sm = s->GetScalar(i + j*dims[0] + (k-1)*ijsize);
    g[2] = 0.5 * (sm - sp) / ar[2];
    }
}

//----------------------------------------------------------------------------
// Set dimensions of structured points dataset.
void vtkImageData::SetDimensions(int i, int j, int k)
{
  this->SetExtent(0, i-1, 0, j-1, 0, k-1);
}

//----------------------------------------------------------------------------
// Set dimensions of structured points dataset.
void vtkImageData::SetDimensions(int dim[3])
{
  this->SetExtent(0, dim[0]-1, 0, dim[1]-1, 0, dim[2]-1);
}


// streaming change: ijk is in extent coordinate system.
//----------------------------------------------------------------------------
// Convenience function computes the structured coordinates for a point x[3].
// The voxel is specified by the array ijk[3], and the parametric coordinates
// in the cell are specified with pcoords[3]. The function returns a 0 if the
// point x is outside of the volume, and a 1 if inside the volume.
int vtkImageData::ComputeStructuredCoordinates(float x[3], int ijk[3], 
					       float pcoords[3])
{
  int i;
  float d, floatLoc;
  float *origin = this->GetOrigin();
  float *spacing = this->GetSpacing();
  
  //
  //  Compute the ijk location
  //
  for (i=0; i<3; i++) 
    {
    d = x[i] - origin[i];
    floatLoc = d / spacing[i];
    ijk[i] = (int) floatLoc;
    if ( ijk[i] >= this->Extent[i*2] && ijk[i] < this->Extent[i*2 + 1] )
      {
      pcoords[i] = floatLoc - (float)ijk[i];
      }

    else if ( ijk[i] < this->Extent[i*2] || ijk[i] > this->Extent[i*2+1] ) 
      {
      return 0;
      } 

    else //if ( ijk[i] == this->Extent[i*2+1] )
      {
      if (this->Dimensions[i] == 1)
        {
        pcoords[i] = 0.0;
        }
      else
        {
        ijk[i] -= 1;
        pcoords[i] = 1.0;
        }
      }

    }
  return 1;
}


//----------------------------------------------------------------------------
void vtkImageData::PrintSelf(ostream& os, vtkIndent indent)
{
  int idx;
  
  vtkDataSet::PrintSelf(os,indent);

  os << indent << "Dimensions: (" << this->Dimensions[0] << ", "
                                  << this->Dimensions[1] << ", "
                                  << this->Dimensions[2] << ")\n";

  os << indent << "Increments: (" << this->Increments[0];
  for (idx = 1; idx < 3; ++idx)
    {
    os << ", " << this->Increments[idx];
    }
  os << ")\n";

  os << indent << "Extent: (" << this->Extent[0];
  for (idx = 1; idx < 6; ++idx)
    {
    os << ", " << this->Extent[idx];
    }
  os << ")\n";

}

//----------------------------------------------------------------------------
void vtkImageData::SetSpacing(float spacing[3])
{
  this->GetImageInformation()->SetSpacing(spacing);
}
//----------------------------------------------------------------------------
void vtkImageData::SetSpacing(float x, float y, float z)
{
  this->GetImageInformation()->SetSpacing(x, y, z);
}
//----------------------------------------------------------------------------
float *vtkImageData::GetSpacing()
{
  return this->GetImageInformation()->GetSpacing();
}
//----------------------------------------------------------------------------
void vtkImageData::GetSpacing(float spacing[3])
{
  this->GetImageInformation()->GetSpacing(spacing);
}

//----------------------------------------------------------------------------
void vtkImageData::SetOrigin(float origin[3])
{
  this->GetImageInformation()->SetOrigin(origin);
}
//----------------------------------------------------------------------------
void vtkImageData::SetOrigin(float x, float y, float z)
{
  this->GetImageInformation()->SetOrigin(x, y, z);
}
//----------------------------------------------------------------------------
float *vtkImageData::GetOrigin()
{
  return this->GetImageInformation()->GetOrigin();
}
//----------------------------------------------------------------------------
void vtkImageData::GetOrigin(float origin[3])
{
  this->GetImageInformation()->GetOrigin(origin);
}



//----------------------------------------------------------------------------
void vtkImageData::SetWholeExtent(int extent[6])
{
  this->GetImageInformation()->SetWholeExtent(extent);
}
//----------------------------------------------------------------------------
void vtkImageData::SetWholeExtent(int xMin, int xMax,
				  int yMin, int yMax, int zMin, int zMax)
{
  int extent[6];

  extent[0] = xMin; extent[1] = xMax;
  extent[2] = yMin; extent[3] = yMax;
  extent[4] = zMin; extent[5] = zMax;
  this->SetWholeExtent(extent);
}

//----------------------------------------------------------------------------
int *vtkImageData::GetWholeExtent()
{
  return this->GetImageInformation()->GetWholeExtent();
}
//----------------------------------------------------------------------------
void vtkImageData::GetWholeExtent(int extent[6])
{
  this->GetImageInformation()->GetWholeExtent(extent);
}
//----------------------------------------------------------------------------
void vtkImageData::GetWholeExtent(int &xMin, int &xMax, int &yMin, int &yMax,
				  int &zMin, int &zMax)
{
  int *ext = this->GetImageInformation()->GetWholeExtent();
  
  xMin = ext[0];
  xMax = ext[1];
  yMin = ext[2];
  yMax = ext[3];
  zMin = ext[4];
  zMax = ext[5];
}

//----------------------------------------------------------------------------
void vtkImageData::SetUpdateExtent(int extent[6])
{
  this->GetStructuredUpdateExtent()->SetExtent(extent);
}

//----------------------------------------------------------------------------
void vtkImageData::SetUpdateExtent(int xMin, int xMax, int yMin, int yMax,
				   int zMin, int zMax)
{
  int extent[6];

  extent[0] = xMin; extent[1] = xMax;
  extent[2] = yMin; extent[3] = yMax;
  extent[4] = zMin; extent[5] = zMax;
  
  this->SetUpdateExtent(extent);
}

//----------------------------------------------------------------------------
// Should we split up cells, or just points.  It does not matter for now.
// Extent of structured data assumes points.
void vtkImageData::SetUpdateExtent(int piece, int numPieces)
{
  int ext[6], zdim, min, max;
  
  // Lets just divide up the z axis.
  this->GetWholeExtent(ext);
  zdim = ext[5] - ext[4] + 1;
  
  if (piece >= zdim)
    {
    // empty
    this->SetUpdateExtent(0, -1, 0, -1, 0, -1);
    return;
    }
  
  if (numPieces > zdim)
    {
    numPieces = zdim;
    }
  
  min = ext[4] + piece * zdim / numPieces;
  max = ext[4] + (piece+1) * zdim / numPieces - 1;
  
  ext[4] = min;
  ext[5] = max;

  this->SetUpdateExtent(ext);
}



//----------------------------------------------------------------------------
int *vtkImageData::GetUpdateExtent()
{
  return this->GetStructuredUpdateExtent()->GetExtent();
}

//----------------------------------------------------------------------------
void vtkImageData::GetUpdateExtent(int ext[6])
{
  int *tmp = this->GetStructuredUpdateExtent()->GetExtent();
  
  ext[0] = tmp[0];
  ext[1] = tmp[1];
  ext[2] = tmp[2];
  ext[3] = tmp[3];
  ext[4] = tmp[4];
  ext[5] = tmp[5];
}

//----------------------------------------------------------------------------
void vtkImageData::SetUpdateExtentToWholeExtent()
{
  this->UpdateInformation();
  this->SetUpdateExtent(this->GetWholeExtent());
}

//----------------------------------------------------------------------------
void vtkImageData::GetUpdateExtent(int &x1, int &x2, int &y1, int &y2, 
				   int &z1, int &z2)
{
  int *ext;
  ext = this->GetUpdateExtent();
  x1 = ext[0];
  x2 = ext[1];
  y1 = ext[2];
  y2 = ext[3];
  z1 = ext[4];
  z2 = ext[5];
}

//----------------------------------------------------------------------------
void vtkImageData::GetExtent(int &x1, int &x2, int &y1, int &y2, 
			     int &z1, int &z2)
{
  int *ext;
  ext = this->GetExtent();
  x1 = ext[0];
  x2 = ext[1];
  y1 = ext[2];
  y2 = ext[3];
  z1 = ext[4];
  z2 = ext[5];
}

//----------------------------------------------------------------------------
void vtkImageData::SetExtent(int x1, int x2, int y1, int y2, int z1, int z2)
{
  int ext[6];
  ext[0] = x1;
  ext[1] = x2;
  ext[2] = y1;
  ext[3] = y2;
  ext[4] = z1;
  ext[5] = z2;
  this->SetExtent(ext);
}

//----------------------------------------------------------------------------
void vtkImageData::SetExtent(int *extent)
{
  int description;

  description = vtkStructuredData::SetExtent(extent, this->Extent);
  if ( description < 0 ) //improperly specified
    {
    vtkErrorMacro (<< "Bad Extent, retaining previous values");
    }
  
  if (description == VTK_UNCHANGED)
    {
    return;
    }

  this->DataDescription = description;
  
  this->Modified();
  this->Dimensions[0] = extent[1] - extent[0] + 1;
  this->Dimensions[1] = extent[3] - extent[2] + 1;
  this->Dimensions[2] = extent[5] - extent[4] + 1;
  this->ComputeIncrements();
  // previously, we released data here, but vtkStructuredPoints
  // sources can setup the scalars before Dimensions are set ...
}

//----------------------------------------------------------------------------
void vtkImageData::SetNumberOfScalarComponents(int num)
{
  // What should we do if the number of scalar components do not match ...
  this->GetImageInformation()->SetNumberOfScalarComponents(num);
  this->ComputeIncrements();
}


//----------------------------------------------------------------------------
int vtkImageData::GetNumberOfScalarComponents()
{
  return this->GetImageInformation()->GetNumberOfScalarComponents();
}


int *vtkImageData::GetIncrements()
{
  // Make sure the increments are up to date. The filter bypass and update
  // mechanism make it tricky to update the increments anywhere other than here
  this->ComputeIncrements();

  return this->Increments;
}

void vtkImageData::GetIncrements(int &incX, int &incY, int &incZ)
{
  // Make sure the increments are up to date. The filter bypass and update
  // mechanism make it tricky to update the increments anywhere other than here
  this->ComputeIncrements();

  incX = this->Increments[0];
  incY = this->Increments[1];
  incZ = this->Increments[2];
}

void vtkImageData::GetIncrements(int inc[3])
{
  // Make sure the increments are up to date. The filter bypass and update
  // mechanism make it tricky to update the increments anywhere other than here
  this->ComputeIncrements();

  inc[0] = this->Increments[0];
  inc[1] = this->Increments[1];
  inc[2] = this->Increments[2];
}


//----------------------------------------------------------------------------
void vtkImageData::GetContinuousIncrements(int extent[6], int &incX,
					   int &incY, int &incZ)
{
  int e0, e1, e2, e3;
  
  incX = 0;

  e0 = extent[0];
  if (e0 < this->Extent[0])
    {
    e0 = this->Extent[0];
    }
  e1 = extent[1];
  if (e1 > this->Extent[1])
    {
    e1 = this->Extent[1];
    }
  e2 = extent[2];
  if (e2 < this->Extent[2])
    {
    e2 = this->Extent[2];
    }
  e3 = extent[3];
  if (e3 > this->Extent[3])
    {
    e3 = this->Extent[3];
    }

  // Make sure the increments are up to date
  this->ComputeIncrements();

  incY = this->Increments[1] - (e1 - e0 + 1)*this->Increments[0];
  incZ = this->Increments[2] - (e3 - e2 + 1)*this->Increments[1];
}


//----------------------------------------------------------------------------
// This method computes the increments from the MemoryOrder and the extent.
void vtkImageData::ComputeIncrements()
{
  int idx;
  int inc = this->GetNumberOfScalarComponents();

  for (idx = 0; idx < 3; ++idx)
    {
    this->Increments[idx] = inc;
    inc *= (this->Extent[idx*2+1] - this->Extent[idx*2] + 1);
    }
}




//----------------------------------------------------------------------------
float vtkImageData::GetScalarComponentAsFloat(int x, int y, int z, int comp)
{
  void *ptr;
  
  if (comp >= this->GetNumberOfScalarComponents() || comp < 0)
    {
    vtkErrorMacro("Bad component index " << comp);
    return 0.0;
    }
  
  ptr = this->GetScalarPointer(x, y, z);
  
  if (ptr == NULL)
    {
    // error message will be generated by get scalar pointer
    return 0.0;
    }
  
  switch (this->GetScalarType())
    {
    case VTK_FLOAT:
      return *(((float *)ptr) + comp);
    case VTK_DOUBLE:
      return *(((double *)ptr) + comp);
    case VTK_INT:
      return (float)(*(((int *)ptr) + comp));
    case VTK_UNSIGNED_INT:
      return (float)(*(((unsigned int *)ptr) + comp));
    case VTK_LONG:
      return (float)(*(((long *)ptr) + comp));
    case VTK_UNSIGNED_LONG:
      return (float)(*(((unsigned long *)ptr) + comp));
    case VTK_SHORT:
      return (float)(*(((short *)ptr) + comp));
    case VTK_UNSIGNED_SHORT:
      return (float)(*(((unsigned short *)ptr) + comp));
    case VTK_UNSIGNED_CHAR:
      return (float)(*(((unsigned char *)ptr) + comp));
    case VTK_CHAR:
      return (float)(*(((char *)ptr) + comp));
    }

  vtkErrorMacro("Unknown Scalar type");
  return 0.0;
}


//----------------------------------------------------------------------------
// This Method returns a pointer to a location in the vtkImageData.
// Coordinates are in pixel units and are relative to the whole
// image origin.
void *vtkImageData::GetScalarPointer(int x, int y, int z)
{
  int tmp[3];
  tmp[0] = x;
  tmp[1] = y;
  tmp[2] = z;
  return this->GetScalarPointer(tmp);
}

//----------------------------------------------------------------------------
// This Method returns a pointer to a location in the vtkImageData.
// Coordinates are in pixel units and are relative to the whole
// image origin.
void *vtkImageData::GetScalarPointerForExtent(int extent[6])
{
  int tmp[3];
  tmp[0] = extent[0];
  tmp[1] = extent[2];
  tmp[2] = extent[4];
  return this->GetScalarPointer(tmp);
}

//----------------------------------------------------------------------------
// This Method returns a pointer to a location in the vtkImageData.
// Coordinates are in pixel units and are relative to the whole
// image origin.
void *vtkImageData::GetScalarPointer(int coordinates[3])
{
  vtkScalars *scalars;
  int idx;
    
  // Make sure the scalars have been allocated.
  scalars = this->PointData->GetScalars();
  if (scalars == NULL)
    {
    vtkDebugMacro("Allocating scalars in ImageData");
    this->AllocateScalars();
    scalars = this->PointData->GetScalars();
    }
  
  // error checking: since most acceses will be from pointer arithmetic.
  // this should not waste much time.
  for (idx = 0; idx < 3; ++idx)
    {
    if (coordinates[idx] < this->Extent[idx*2] ||
	coordinates[idx] > this->Extent[idx*2+1])
      {
      vtkErrorMacro(<< "GetScalarPointer: Pixel (" << coordinates[0] << ", " 
      << coordinates[1] << ", "
      << coordinates[2] << ") not in memory.\n Current extent= ("
      << this->Extent[0] << ", " << this->Extent[1] << ", "
      << this->Extent[2] << ", " << this->Extent[3] << ", "
      << this->Extent[4] << ", " << this->Extent[5] << ")");
      return NULL;
      }
    }
  
  // compute the index of the vector.
  idx = ((coordinates[0] - this->Extent[0]) * this->Increments[0]
	 + (coordinates[1] - this->Extent[2]) * this->Increments[1]
	 + (coordinates[2] - this->Extent[4]) * this->Increments[2]);
  
  return scalars->GetVoidPointer(idx);
}


//----------------------------------------------------------------------------
// This Method returns a pointer to the origin of the vtkImageData.
void *vtkImageData::GetScalarPointer()
{
  if (this->PointData->GetScalars() == NULL)
    {
    vtkDebugMacro("Allocating scalars in ImageData");
    this->AllocateScalars();
    }
  return this->PointData->GetScalars()->GetVoidPointer(0);
}

//----------------------------------------------------------------------------
int vtkImageData::GetScalarType()
{
  vtkScalars *tmp;
  int type = this->GetImageInformation()->GetScalarType();
  
  // if we have scalars make sure the type matches our ivar
  tmp = this->GetPointData()->GetScalars();
  if (tmp && tmp->GetDataType() != type)
    {
    // this happens when filters are being bypassed.  Don't error...
    //vtkErrorMacro("ScalarType " << tmp->GetDataType() 
    //                 << " does not match current scalars of type " << type);
    }
  
  return type;
}

//----------------------------------------------------------------------------
void vtkImageData::SetScalarType(int t)
{
  vtkScalars *tmp;

  // if we have scalars make sure they match
  tmp = this->GetPointData()->GetScalars();
  if (tmp && tmp->GetDataType() != t)
    {
    // This happens during setup of default information
    //vtkWarningMacro("Setting ScalarType: Existing scalars do not match.");
    }
  
  this->GetImageInformation()->SetScalarType(t);
}


//----------------------------------------------------------------------------
void vtkImageData::AllocateScalars()
{
  vtkScalars *scalars;
  
  // if the scalar type has not been set then we have a problem
  if (this->GetScalarType() == VTK_VOID)
    {
    vtkErrorMacro("Attempt to allocate scalars before scalar type was set!.");
    return;
    }

  // if we currently have scalars then just adjust the size
  scalars = this->PointData->GetScalars();
  if (scalars && scalars->GetDataType() == this->GetScalarType()) 
    {
    scalars->SetNumberOfComponents(this->GetNumberOfScalarComponents());
    scalars->SetNumberOfScalars((this->Extent[1] - this->Extent[0] + 1)*
				(this->Extent[3] - this->Extent[2] + 1)*
				(this->Extent[5] - this->Extent[4] + 1));
    // Since the execute method will be modifying the scalars
    // directly.
    scalars->Modified();
    return;
    }
  
  // allocate the new scalars
  scalars = vtkScalars::New();
  scalars->SetDataType(this->GetScalarType());
  scalars->SetNumberOfComponents(this->GetNumberOfScalarComponents());
  this->PointData->SetScalars(scalars);
  scalars->Delete();
  
  // allocate enough memory
  this->PointData->GetScalars()->
    SetNumberOfScalars((this->Extent[1] - this->Extent[0] + 1)*
		       (this->Extent[3] - this->Extent[2] + 1)*
		       (this->Extent[5] - this->Extent[4] + 1));
}


//----------------------------------------------------------------------------
int vtkImageData::GetScalarSize()
{
  // allocate the new scalars
  switch (this->GetScalarType())
    {
    case VTK_FLOAT:
      return sizeof(float);
    case VTK_DOUBLE:
      return sizeof(double);
    case VTK_INT:
    case VTK_UNSIGNED_INT:
      return sizeof(int);
    case VTK_LONG:
    case VTK_UNSIGNED_LONG:
      return sizeof(long);
    case VTK_SHORT:
    case VTK_UNSIGNED_SHORT:
      return 2;
    case VTK_UNSIGNED_CHAR:
      return 1;
    case VTK_CHAR:
      return 1;
    }
  
  return 1;
}

//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
template <class IT, class OT>
static void vtkImageDataCastExecute(vtkImageData *inData, IT *inPtr,
				    vtkImageData *outData, OT *outPtr,
				    int outExt[6])
{
  int idxR, idxY, idxZ;
  int maxY, maxZ;
  int inIncX, inIncY, inIncZ;
  int outIncX, outIncY, outIncZ;
  int rowLength;

  // find the region to loop over
  rowLength = (outExt[1] - outExt[0]+1)*inData->GetNumberOfScalarComponents();
  maxY = outExt[3] - outExt[2]; 
  maxZ = outExt[5] - outExt[4];
  
  // Get increments to march through data 
  inData->GetContinuousIncrements(outExt, inIncX, inIncY, inIncZ);
  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);

  // Loop through ouput pixels
  for (idxZ = 0; idxZ <= maxZ; idxZ++)
    {
    for (idxY = 0; idxY <= maxY; idxY++)
      {
      for (idxR = 0; idxR < rowLength; idxR++)
	{
	// Pixel operation
	*outPtr = (OT)(*inPtr);
	outPtr++;
	inPtr++;
	}
      outPtr += outIncY;
      inPtr += inIncY;
      }
    outPtr += outIncZ;
    inPtr += inIncZ;
    }
}



//----------------------------------------------------------------------------
template <class T>
static void vtkImageDataCastExecute(vtkImageData *inData, T *inPtr,
				    vtkImageData *outData, int outExt[6])
{
  void *outPtr = outData->GetScalarPointerForExtent(outExt);

  switch (outData->GetScalarType())
    {
    case VTK_DOUBLE:
      vtkImageDataCastExecute(inData, (T *)(inPtr), 
			      outData, (double *)(outPtr),outExt);
      break;
    case VTK_FLOAT:
      vtkImageDataCastExecute(inData, (T *)(inPtr), 
			      outData, (float *)(outPtr),outExt);
      break;
    case VTK_LONG:
      vtkImageDataCastExecute(inData, (T *)(inPtr), 
			      outData, (long *)(outPtr),outExt); 
      break;
    case VTK_UNSIGNED_LONG:
      vtkImageDataCastExecute(inData, (T *)(inPtr), 
			      outData, (unsigned long *)(outPtr),outExt); 
      break;
    case VTK_INT:
      vtkImageDataCastExecute(inData, (T *)(inPtr), 
			      outData, (int *)(outPtr),outExt); 
      break;
    case VTK_UNSIGNED_INT:
      vtkImageDataCastExecute(inData, (T *)(inPtr), 
			      outData, (unsigned int *)(outPtr),outExt); 
      break;
    case VTK_SHORT:
      vtkImageDataCastExecute(inData, (T *)(inPtr), 
			      outData, (short *)(outPtr),outExt);
      break;
    case VTK_UNSIGNED_SHORT:
      vtkImageDataCastExecute(inData, (T *)(inPtr), 
			      outData, (unsigned short *)(outPtr),outExt); 
      break;
    case VTK_CHAR:
      vtkImageDataCastExecute(inData, (T *)(inPtr), 
			      outData, (char *)(outPtr),outExt); 
      break;
    case VTK_UNSIGNED_CHAR:
      vtkImageDataCastExecute(inData, (T *)(inPtr), 
			      outData, (unsigned char *)(outPtr),outExt); 
      break;
    default:
      vtkGenericWarningMacro("Execute: Unknown output ScalarType");
      return;
    }
}




//----------------------------------------------------------------------------
// This method is passed a input and output region, and executes the filter
// algorithm to fill the output from the input.
// It just executes a switch statement to call the correct function for
// the regions data types.
void vtkImageData::CopyAndCastFrom(vtkImageData *inData, int extent[6])
{
  void *inPtr = inData->GetScalarPointerForExtent(extent);
  
  switch (inData->GetScalarType())
    {
    case VTK_DOUBLE:
      vtkImageDataCastExecute(inData, (double *)(inPtr), 
			      this, extent);
      break;
    case VTK_FLOAT:
      vtkImageDataCastExecute(inData, (float *)(inPtr), 
			      this, extent);
      break;
    case VTK_LONG:
      vtkImageDataCastExecute(inData, (long *)(inPtr), 
			      this, extent);
      break;
    case VTK_UNSIGNED_LONG:
      vtkImageDataCastExecute(inData, (unsigned long *)(inPtr), 
			      this, extent);
      break;
    case VTK_INT:
      vtkImageDataCastExecute(inData, (int *)(inPtr), 
			      this, extent);
      break;
    case VTK_UNSIGNED_INT:
      vtkImageDataCastExecute(inData, (unsigned int *)(inPtr), 
			      this, extent);
      break;
    case VTK_SHORT:
      vtkImageDataCastExecute(inData, (short *)(inPtr), 
			      this, extent);
      break;
    case VTK_UNSIGNED_SHORT:
      vtkImageDataCastExecute(inData, (unsigned short *)(inPtr), 
			      this, extent);
      break;
    case VTK_CHAR:
      vtkImageDataCastExecute(inData, (char *)(inPtr), 
			      this, extent);
      break;
    case VTK_UNSIGNED_CHAR:
      vtkImageDataCastExecute(inData, (unsigned char *)(inPtr), 
			      this, extent);
      break;
    default:
      vtkErrorMacro(<< "Execute: Unknown input ScalarType");
      return;
    }
}




//----------------------------------------------------------------------------
double vtkImageData::GetScalarTypeMin()
{
  switch (this->GetScalarType())
    {
    case VTK_DOUBLE:
      return (double)(VTK_DOUBLE_MIN);
    case VTK_FLOAT:
      return (double)(VTK_FLOAT_MIN);
    case VTK_LONG:
      return (double)(VTK_LONG);
    case VTK_UNSIGNED_LONG:
      return (double)(VTK_UNSIGNED_LONG);
    case VTK_INT:
      return (double)(VTK_INT_MIN);
    case VTK_UNSIGNED_INT:
      return (double)(VTK_UNSIGNED_INT_MIN);
    case VTK_SHORT:
      return (double)(VTK_SHORT_MIN);
    case VTK_UNSIGNED_SHORT:
      return (double)(0.0);
    case VTK_CHAR:
      return (double)(VTK_CHAR_MIN);
    case VTK_UNSIGNED_CHAR:
      return (double)(0.0);
    default:
      vtkErrorMacro("Cannot handle scalar type " << this->GetScalarType());
      return 0.0;
    }
}


//----------------------------------------------------------------------------
double vtkImageData::GetScalarTypeMax()
{
  switch (this->GetScalarType())
    {
    case VTK_DOUBLE:
      return (double)(VTK_DOUBLE_MAX);
    case VTK_FLOAT:
      return (double)(VTK_FLOAT_MAX);
    case VTK_LONG:
      return (double)(VTK_LONG_MAX);
    case VTK_UNSIGNED_LONG:
      return (double)(VTK_UNSIGNED_LONG_MAX);
    case VTK_INT:
      return (double)(VTK_INT_MAX);
    case VTK_UNSIGNED_INT:
      return (double)(VTK_UNSIGNED_INT_MAX);
    case VTK_SHORT:
      return (double)(VTK_SHORT_MAX);
    case VTK_UNSIGNED_SHORT:
      return (double)(VTK_UNSIGNED_SHORT_MAX);
    case VTK_CHAR:
      return (double)(VTK_CHAR_MAX);
    case VTK_UNSIGNED_CHAR:
      return (double)(VTK_UNSIGNED_CHAR_MAX);
    default:
      vtkErrorMacro("Cannot handle scalar type " << this->GetScalarType());
      return 0.0;
    }
}

//----------------------------------------------------------------------------
void vtkImageData::SetAxisUpdateExtent(int idx, int min, int max)
{
  int modified = 0;
  int ext[6];
  
  this->GetStructuredUpdateExtent()->GetExtent(ext);
  
  if (idx > 2)
    {
    vtkWarningMacro("illegal axis!");
    return;
    }
  
  if (ext[idx*2] != min)
    {
    modified = 1;
    ext[idx*2] = min;
    }
  if (ext[idx*2+1] != max)
    {
    modified = 1;
    ext[idx*2+1] = max;
    }

  if (modified)
    {
    this->Modified();
    }
  
  this->GetStructuredUpdateExtent()->SetExtent(ext);
}

//----------------------------------------------------------------------------
void vtkImageData::GetAxisUpdateExtent(int idx, int &min, int &max)
{
  int *ext = this->GetStructuredUpdateExtent()->GetExtent();
  
  if (idx > 2)
    {
    vtkWarningMacro("illegal axis!");
    return;
    }

  min = ext[idx*2];
  max = ext[idx*2+1];
}

//----------------------------------------------------------------------------
void vtkImageData::UpdateInformation()
{
  if (this->Source)
    {
    this->Source->UpdateInformation();
    // It is important the this not change our modify time.
    this->ComputeEstimatedWholeMemorySize();
    }
}

//----------------------------------------------------------------------------
// Estimated memory size is implicit in the other image information.
// It is computed automatically (superclass) during UpdateInformation.
void vtkImageData::ComputeEstimatedWholeMemorySize()
{
  double size = (float)(this->GetNumberOfScalarComponents());
  int idx;
  int *wExt = this->GetWholeExtent();
  
  switch (this->GetScalarType())
    {
    case VTK_FLOAT:
      size *= sizeof(float);
      break;
    case VTK_DOUBLE:
      size *= sizeof(double);
      break;
    case VTK_INT:
      size *= sizeof(int);
      break;
    case VTK_UNSIGNED_INT:
      size *= sizeof(unsigned int);
      break;
    case VTK_LONG:
      size *= sizeof(long);
      break;
    case VTK_UNSIGNED_LONG:
      size *= sizeof(unsigned long);
      break;
    case VTK_SHORT:
      size *= sizeof(short);
      break;
    case VTK_UNSIGNED_SHORT:
      size *= sizeof(unsigned short);
      break;
    case VTK_UNSIGNED_CHAR:
      size *= sizeof(unsigned char);
      break;
    case VTK_CHAR:
      size *= sizeof(char);
      break;
    case VTK_BIT:
      size = size / 8;
      break;
    default:
      vtkWarningMacro(<< "GetExtentMemorySize: "
        << "Cannot determine input scalar type");
    }  

  // Compute the number of scalars.
  for (idx = 0; idx < 3; ++idx)
    {
    size = size*(wExt[idx*2+1] - wExt[idx*2] + 1);
    }

  // In case the extent is set improperly
  // Now Improperly might mean the filter will update no memory,
  // (multiple input filters) so do not give an error.
  if (size < 0)
    {
    this->Information->SetEstimatedWholeMemorySize((unsigned long)(0));
    }

  long lsize = (long)(size / 1000.0);
  
  this->Information->SetEstimatedWholeMemorySize(lsize);
}

//----------------------------------------------------------------------------
unsigned long vtkImageData::GetEstimatedUpdateMemorySize()
{
  int idx, *uExt = this->GetStructuredUpdateExtent()->GetExtent();
  int *wExt = this->GetWholeExtent();
  unsigned long wholeSize, updateSize;
  
  // Compute the sizes
  wholeSize = updateSize = 1;
  for (idx = 0; idx < 3; ++idx)
    {
    wholeSize *= (wExt[idx*2+1] - wExt[idx*2] + 1);
    updateSize *= (uExt[idx*2+1] - uExt[idx*2] +1);
    }

  updateSize = updateSize * this->Information->GetEstimatedWholeMemorySize() 
    / wholeSize;
  if (updateSize < 1)
    {
    return 1;
    }
  return updateSize;
}

//----------------------------------------------------------------------------
// This method is used translparently by the "SetInput(vtkImageCache *)"
// method to connect the image pipeline to the visualization pipeline.
vtkImageToStructuredPoints *vtkImageData::MakeImageToStructuredPoints()
{
  if ( ! this->ImageToStructuredPoints)
    {
    this->ImageToStructuredPoints = vtkImageToStructuredPoints::New();
    this->ImageToStructuredPoints->SetInput(this);
    }
  else
    {
    // we must up the ref count because this is a Make method
    // it will be matched by a Delete
    this->ImageToStructuredPoints->Register(this);
    }
  return this->ImageToStructuredPoints;
}

//----------------------------------------------------------------------------
unsigned long vtkImageData::GetActualMemorySize()
{
  return this->vtkDataSet::GetActualMemorySize();
}

