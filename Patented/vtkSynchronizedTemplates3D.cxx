/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1996 Ken Martin, Will Schroeder, Bill Lorensen.

    THIS CLASS IS PATENT PENDING.

    Application of this software for commercial purposes requires 
    a license grant from Kitware. Contact:
        Ken Martin
        Kitware
        469 Clifton Corporate Parkway,
        Clifton Park, NY 12065
        Phone:1-518-371-3971 
    for more information.

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

#include <math.h>
#include "vtkStructuredPoints.h"
#include "vtkCharArray.h"
#include "vtkUnsignedCharArray.h"
#include "vtkShortArray.h"
#include "vtkUnsignedShortArray.h"
#include "vtkIntArray.h"
#include "vtkUnsignedIntArray.h"
#include "vtkLongArray.h"
#include "vtkUnsignedLongArray.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkSynchronizedTemplates3D.h"
#include "vtkMath.h"

//----------------------------------------------------------------------------
// Description:
// Construct object with initial scalar range (0,1) and single contour value
// of 0.0. The ImageRange are set to extract the first k-plane.
vtkSynchronizedTemplates3D::vtkSynchronizedTemplates3D()
{
  int idx;

  this->ContourValues = vtkContourValues::New();
  this->ComputeNormals = 1;
  this->ComputeGradients = 0;
  this->ComputeScalars = 1;

  this->MinimumPieceSize[0] = 1;
  this->MinimumPieceSize[1] = 1;
  this->MinimumPieceSize[2] = 1;

  this->ExecuteExtent[0] = this->ExecuteExtent[1] 
    = this->ExecuteExtent[2] = this->ExecuteExtent[3] 
    = this->ExecuteExtent[4] = this->ExecuteExtent[5] = 0;
  
  this->Threader = vtkMultiThreader::New();
  this->NumberOfThreads = this->Threader->GetNumberOfThreads();
  for (idx = 0; idx < VTK_MAX_THREADS; ++idx)
    {
    this->Threads[idx] = NULL;
    }
}

//----------------------------------------------------------------------------
vtkSynchronizedTemplates3D::~vtkSynchronizedTemplates3D()
{
  this->ContourValues->Delete();
  this->Threader->Delete();
}

//----------------------------------------------------------------------------
// Overload standard modified time function. If contour values are modified,
// then this object is modified as well.
unsigned long vtkSynchronizedTemplates3D::GetMTime()
{
  unsigned long mTime=this->vtkPolyDataSource::GetMTime();
  unsigned long mTime2=this->ContourValues->GetMTime();

  mTime = ( mTime2 > mTime ? mTime2 : mTime );
  return mTime;
}


//----------------------------------------------------------------------------
// Calculate the gradient using central difference.
template <class T>
static void vtkSTComputePointGradient(int i, int j, int k, T *s, int *wholeExt, 
				      int yInc, int zInc, float *spacing,
				      float n[3])
{
  float sp, sm;

  // x-direction
  if ( i == wholeExt[0] )
    {
    sp = *(s+1);
    sm = *s;
    n[0] = (sp - sm) / spacing[0];
    }
  else if ( i == wholeExt[1] )
    {
    sp = *s;
    sm = *(s-1);
    n[0] = (sp - sm) / spacing[0];
    }
  else
    {
    sp = *(s+1);
    sm = *(s-1);
    n[0] = 0.5 * (sp - sm) / spacing[0];
    }

  // y-direction
  if ( j == wholeExt[2] )
    {
    sp = *(s+yInc);
    sm = *s;
    n[1] = (sp - sm) / spacing[1];
    }
  else if ( j == wholeExt[3] )
    {
    sp = *s;
    sm = *(s-yInc);
    n[1] = (sp - sm) / spacing[1];
    }
  else
    {
    sp = *(s+yInc);
    sm = *(s-yInc);
    n[1] = 0.5 * (sp - sm) / spacing[1];
    }

  // z-direction
  if ( k == wholeExt[4] )
    {
    sp = *(s+zInc);
    sm = *s;
    n[2] = (sp - sm) / spacing[2];
    }
  else if ( k == wholeExt[5] )
    {
    sp = *s;
    sm = *(s-zInc);
    n[2] = (sp - sm) / spacing[2];
    }
  else
    {
    sp = *(s+zInc);
    sm = *(s-zInc);
    n[2] = 0.5 * (sp - sm) / spacing[2];
    }
}

//----------------------------------------------------------------------------
#define VTK_CSP3PA(i2,j2,k2,s) \
if (NeedGradients) \
{ \
  if (!g0) \
    { \
    vtkSTComputePointGradient(i, j, k, s0, wholeExt, yInc, zInc, spacing, n0); \
    g0 = 1; \
    } \
  vtkSTComputePointGradient(i2, j2, k2, s, wholeExt, yInc, zInc, spacing, n1); \
  for (jj=0; jj<3; jj++) \
    { \
    n[jj] = n0[jj] + t * (n1[jj] - n0[jj]); \
    } \
  if (ComputeGradients) \
    { \
    newGradients->InsertNextVector(n); \
    } \
  if (ComputeNormals) \
    { \
    vtkMath::Normalize(n); \
    n[0] = -n[0]; n[1] = -n[1]; n[2] = -n[2]; \
    newNormals->InsertNextNormal(n); \
    }   \
} \
if (ComputeScalars) \
{ \
  newScalars->InsertNextScalar(value); \
}

//----------------------------------------------------------------------------
//
// Contouring filter specialized for images
//
template <class T>
static void ContourImage(vtkSynchronizedTemplates3D *self, int *exExt,
			 vtkImageData *data, vtkPoints *newPts, 
			 vtkScalars *newScalars, vtkCellArray *polys, 
			 vtkNormals *newNormals, vtkVectors *newGradients, 
			 T *ptr)
{
  int xdim = exExt[1] - exExt[0] + 1;
  int ydim = exExt[3] - exExt[2] + 1;
  int zdim = exExt[5] - exExt[4] + 1;
  float *values = self->GetValues();
  int numContours = self->GetNumberOfContours();
  T *inPtrX, *inPtrY, *inPtrZ;
  T *s0, *s1, *s2, *s3;
  int xMin, xMax, yMin, yMax, zMin, zMax;
  int xInc, yInc, zInc;
  float *origin = data->GetOrigin();
  float *spacing = data->GetSpacing();
  int *itmp, *isect1Ptr, *isect2Ptr;
  float y, z, t;
  int i, j, k;
  int zstep, yisectstep;
  int offsets[12];
  int ComputeNormals = self->GetComputeNormals();
  int ComputeGradients = self->GetComputeGradients();
  int ComputeScalars = self->GetComputeScalars();
  int NeedGradients = ComputeGradients || ComputeNormals;
  float n[3], n0[3], n1[3], n2[3], n3[3];
  int jj, g0;
  int *tablePtr;
  int idx, vidx;
  float x[3], xz[3];
  int v0, v1, v2, v3;
  int ptIds[3];
  float value;
  int *wholeExt;
  
  // this is an exploded execute extent.
  xMin = exExt[0];
  xMax = exExt[1];
  yMin = exExt[2];
  yMax = exExt[3];
  zMin = exExt[4];
  zMax = exExt[5];
  
  // increments to move through scalars
  data->GetIncrements(xInc, yInc, zInc);
  wholeExt = self->GetInput()->GetWholeExtent();
  
  // Kens increments, probably to do with edge array
  zstep = xdim*ydim;
  yisectstep = xdim*3;
  // compute offsets probably how to get to the edges in the edge array.
  offsets[0] = -xdim*3;
  offsets[1] = -xdim*3 + 1;
  offsets[2] = -xdim*3 + 2;
  offsets[3] = -xdim*3 + 4;
  offsets[4] = -xdim*3 + 5;
  offsets[5] = 0;
  offsets[6] = 2;
  offsets[7] = 5;
  offsets[8] = (zstep - xdim)*3;
  offsets[9] = (zstep - xdim)*3 + 1;
  offsets[10] = (zstep - xdim)*3 + 4;
  offsets[11] = zstep*3;
    
  // allocate storage array
  int *isect1 = new int [xdim*ydim*3*2];
  // set impossible edges to -1
  for (i = 0; i < ydim; i++)
    {
    isect1[(i+1)*xdim*3-3] = -1;
    isect1[(i+1)*xdim*3*2-3] = -1;
    }
  for (i = 0; i < xdim; i++)
    {
    isect1[((ydim-1)*xdim + i)*3 + 1] = -1;
    isect1[((ydim-1)*xdim + i)*3*2 + 1] = -1;
    }

  // for each contour
  for (vidx = 0; vidx < numContours; vidx++)
    {
    value = values[vidx];
    inPtrZ = ptr;
    s2 = inPtrZ;
    v2 = (*s2 < value ? 0 : 1);

    //==================================================================
    for (k = zMin; k <= zMax; k++)
      {
      z = origin[2] + spacing[2]*k;
      x[2] = z;

      // swap the buffers
      if (k%2)
        {
	      offsets[8] = (zstep - xdim)*3;
        offsets[9] = (zstep - xdim)*3 + 1;
        offsets[10] = (zstep - xdim)*3 + 4;
        offsets[11] = zstep*3;
        isect1Ptr = isect1;
        isect2Ptr = isect1 + xdim*ydim*3;
        }
      else
        { 
        offsets[8] = (-zstep - xdim)*3;
        offsets[9] = (-zstep - xdim)*3 + 1;
        offsets[10] = (-zstep - xdim)*3 + 4;
        offsets[11] = -zstep*3;
        isect1Ptr = isect1 + xdim*ydim*3;
        isect2Ptr = isect1;
        }

      inPtrY = inPtrZ;
      for (j = yMin; j <= yMax; j++)
        {
        y = origin[1] + j*spacing[1];
        xz[1] = y;
        s1 = inPtrY;
        v1 = (*s1 < value ? 0 : 1);
	
	inPtrX = inPtrY;
        for (i = xMin; i <= xMax; i++)
	  {
	  s0 = s1;
	  v0 = v1;
          // this flag keeps up from computing gradient for grid point 0 twice.
	  g0 = 0;
	  if (i < xMax)
	    {
	    s1 = (inPtrX + 1);
	    v1 = (*s1 < value ? 0 : 1);
	    if (v0 ^ v1)
	      {
	      t = (value - (float)(*s0)) / ((float)(*s1) - (float)(*s0));
              x[0] = origin[0] + spacing[0]*(i+t);
              x[1] = y;
              *isect2Ptr = newPts->InsertNextPoint(x);
              VTK_CSP3PA(i+1,j,k,s1);
              }
            else
              {
              *isect2Ptr = -1;
              }
            }
          if (j < yMax)
	    {
	    s2 = (inPtrX + yInc);
	    v2 = (*s2 < value ? 0 : 1);
	    if (v0 ^ v2)
	      {
	      t = (value - (float)(*s0)) / ((float)(*s2) - (float)(*s0));
	      x[0] = origin[0] + spacing[0]*i;
	      x[1] = y + spacing[1]*t;
	      *(isect2Ptr + 1) = newPts->InsertNextPoint(x);
	      VTK_CSP3PA(i,j+1,k,s2);
	      }
	    else
	      {
	      *(isect2Ptr + 1) = -1;
	      }
	    }
	  if (k < zMax)
	    {
	    s3 = (inPtrX + zInc);
	    v3 = (*s3 < value ? 0 : 1);
	    if (v0 ^ v3)
	      {
	      t = (value - (float)(*s0)) / ((float)(*s3) - (float)(*s0));
	      xz[0] = origin[0] + spacing[0]*i;
	      xz[2] = z + spacing[2]*t;
	      *(isect2Ptr + 2) = newPts->InsertNextPoint(xz);
	      VTK_CSP3PA(i,j,k+1,s3);
	      }
	    else
	      {
	      *(isect2Ptr + 2) = -1;
	      }
	    }
	  
	  // now add any polys that need to be added
	  // basically look at the isect values, 
	  // form an index and lookup the polys
	  if (j > yMin && i < xMax && k > zMin)
	    {
	    idx = (v0 ? 4096 : 0);
	    idx = idx + (*(isect1Ptr - yisectstep) > -1 ? 2048 : 0);
	    idx = idx + (*(isect1Ptr -yisectstep +1) > -1 ? 1024 : 0);
	    idx = idx + (*(isect1Ptr -yisectstep +2) > -1 ? 512 : 0);
	    idx = idx + (*(isect1Ptr -yisectstep +4) > -1 ? 256 : 0);
	    idx = idx + (*(isect1Ptr -yisectstep +5) > -1 ? 128 : 0);
	    idx = idx + (*(isect1Ptr) > -1 ? 64 : 0);
	    idx = idx + (*(isect1Ptr + 2) > -1 ? 32 : 0);
	    idx = idx + (*(isect1Ptr + 5) > -1 ? 16 : 0);
	    idx = idx + (*(isect2Ptr -yisectstep) > -1 ? 8 : 0);
	    idx = idx + (*(isect2Ptr -yisectstep +1) > -1 ? 4 : 0);
	    idx = idx + (*(isect2Ptr -yisectstep +4) > -1 ? 2 : 0);
	    idx = idx + (*(isect2Ptr) > -1 ? 1 : 0);
	    
	    tablePtr = VTK_SYNCHONIZED_TEMPLATES_3D_TABLE_2 
	      + VTK_SYNCHONIZED_TEMPLATES_3D_TABLE_1[idx];
	    while (*tablePtr != -1)
	      {
	      ptIds[0] = *(isect1Ptr + offsets[*tablePtr]);
	      tablePtr++;
	      ptIds[1] = *(isect1Ptr + offsets[*tablePtr]);
	      tablePtr++;
	      ptIds[2] = *(isect1Ptr + offsets[*tablePtr]);
	      tablePtr++;
	      polys->InsertNextCell(3,ptIds);
	      }
	    }
	  ++inPtrX;
	  isect2Ptr += 3;
	  isect1Ptr += 3;
	  }
	inPtrY += yInc;
	}
      inPtrZ += zInc;
      }
    }
  delete [] isect1;
}




//----------------------------------------------------------------------------
void vtkSynchronizedTemplates3D::SetInputMemoryLimit(unsigned long limit)
{
  if (this->GetInput() == NULL)
    {
    vtkErrorMacro("Input not set");
    return;
    }
  this->GetInput()->SetMemoryLimit(limit);
}


//----------------------------------------------------------------------------
unsigned long vtkSynchronizedTemplates3D::GetInputMemoryLimit()
{
  if (this->GetInput() == NULL)
    {
    vtkErrorMacro("Input not set");
    return 0;
    }
  return this->GetInput()->GetMemoryLimit();
}



//----------------------------------------------------------------------------
//
// Contouring filter specialized for images (or slices from images)
//
void vtkSynchronizedTemplates3D::ThreadedExecute(vtkImageData *data,
						 int *exExt, int threadId)
{
  vtkPoints *newPts;
  vtkCellArray *newPolys;
  void *ptr;
  vtkPolyData *output = this->GetOutput();
  int estimatedSize;
  vtkScalars *newScalars;
  vtkNormals *newNormals;
  vtkVectors *newGradients;
  

  vtkDebugMacro(<< "Executing 3D structured contour");
  
  // The first thread always writes into the output.
  // Other threads must create a temporary polydata as output.
  if (threadId > 0)
    {
    output = this->Threads[threadId] = vtkPolyData::New();
    this->InitializeOutput(exExt, output);
    }

  vtkDebugMacro(<< "Executing 3D structured contour");
  
  if ( exExt[0] == exExt[1] || exExt[2] == exExt[3] || exExt[4] == exExt[5] )
    {
    vtkErrorMacro(<<"3D structured contours requires 3D data");
    return;
    }
  
  newPts = output->GetPoints();
  newPolys = output->GetPolys();
  newScalars = output->GetPointData()->GetScalars();
  newNormals = output->GetPointData()->GetNormals();
  newGradients = output->GetPointData()->GetVectors();


  //
  // Check data type and execute appropriate function
  //
  if (data->GetNumberOfScalarComponents() == 1 )
    {
    ptr = data->GetScalarPointerForExtent(exExt);
    switch (data->GetScalarType())
      {
      case VTK_CHAR:
        {
	ContourImage(this, exExt, data, newPts, newScalars, newPolys, 
		     newNormals, newGradients, (char*)ptr);
	}
      break;
      case VTK_UNSIGNED_CHAR:
        {
	ContourImage(this, exExt, data, newPts, newScalars, newPolys, 
		     newNormals, newGradients, (unsigned char*)ptr);
	}
      break;
      case VTK_SHORT:
        {
	ContourImage(this, exExt, data, newPts, newScalars, newPolys, 
		     newNormals, newGradients, (short*)ptr);
	}
      break;
      case VTK_UNSIGNED_SHORT:
        {
	ContourImage(this, exExt, data, newPts, newScalars, newPolys, 
		     newNormals, newGradients, (unsigned short*)ptr);
	}
      break;
      case VTK_INT:
        {
	ContourImage(this, exExt, data, newPts, newScalars, newPolys, 
		     newNormals, newGradients, (int*)ptr);
	}
      break;
      case VTK_UNSIGNED_INT:
        {
	ContourImage(this, exExt, data, newPts, newScalars, newPolys, 
		     newNormals, newGradients, (unsigned int*)ptr);
	}
      break;
      case VTK_LONG:
        {
	ContourImage(this, exExt, data, newPts, newScalars, newPolys, 
		     newNormals, newGradients, (long*)ptr);
	}
      break;
      case VTK_UNSIGNED_LONG:
        {
	ContourImage(this, exExt, data, newPts, newScalars, newPolys, 
		     newNormals, newGradients, (unsigned long*)ptr);
	}
      break;
      case VTK_FLOAT:
        {
	ContourImage(this, exExt, data, newPts, newScalars, newPolys, 
		     newNormals, newGradients, (float*)ptr);
	}
      break;
      case VTK_DOUBLE:
        {
	ContourImage(this, exExt, data, newPts, newScalars, newPolys, 
		     newNormals, newGradients, (double*)ptr);
	}
      break;
      }//switch
    }
  else //multiple components - have to convert
    {
    vtkErrorMacro("Cannot handle multiple components yet.");
    return;
    }
}

//----------------------------------------------------------------------------
void vtkSynchronizedTemplates3D::ExecuteInformation()
{
  vtkImageData *input = this->GetInput();
  int *ext, dims[3];
  long numPts, numTris;
  long sizePt, sizeTri;

  // swag at the output memory size
  // Outside surface.
  ext = input->GetWholeExtent();
  dims[0] = ext[1] - ext[0] + 1;
  dims[1] = ext[3] - ext[2] + 1;
  dims[2] = ext[5] - ext[4] + 1;
  numPts = 2 * (dims[0]*dims[1] + dims[0]*dims[2] + dims[1]*dims[2]);
  numTris = numPts * 2;
  // Determine the memory for each point and triangle.
  sizeTri = 4 * sizeof(int);
  sizePt = 3 * sizeof(float);
  if (this->ComputeNormals)
    {
    sizePt += 3 * sizeof(float);
    }
  if (this->ComputeGradients)
    {
    sizePt += 3 * sizeof(float);
    }
  if (this->ComputeScalars)
    {
    sizePt += sizeof(float);
    }
  // Set the whole output estimated memory size in kBytes.
  // be careful not to overflow.
  numTris = numTris / 1000;
  if (numTris == 0)
    {
    numTris = 1;
    }
  numPts = numPts / 1000;
  if (numPts == 0)
    {
    numPts = 1;
    }
  this->GetOutput()->SetEstimatedWholeMemorySize(
    numTris*sizeTri + numPts*sizePt);
  
  // Lets determine how many piece we can divide the output into.
  // OK lets be picky and compute this exactly.
  
  numPts = (dims[0]-1) / this->MinimumPieceSize[0];
  if (numPts <= 0) {numPts = 1;}
  numTris = numPts;
  numPts = (dims[1]-1) / this->MinimumPieceSize[1];
  if (numPts <= 0) {numPts = 1;}
  numTris *= numPts;
  numPts = (dims[2]-1) / this->MinimumPieceSize[2];
  if (numPts <= 0) {numPts = 1;}
  numTris *= numPts;
  this->GetOutput()->SetMaximumNumberOfPieces(numTris);
}

//----------------------------------------------------------------------------
void vtkSynchronizedTemplates3D::StreamExecuteStart()
{
  vtkPolyData *output = this->GetOutput();
  vtkImageData *input = this->GetInput();
  int *ext = input->GetWholeExtent();

  this->InitializeOutput(ext, output);
}

//----------------------------------------------------------------------------
void vtkSynchronizedTemplates3D::StreamExecuteEnd()
{
  vtkPolyData *output = this->GetOutput();
  // reclaim unused space.
  output->Squeeze();
}


//----------------------------------------------------------------------------
void vtkSynchronizedTemplates3D::InitializeOutput(int *ext,vtkPolyData *o)
{
  vtkPoints *newPts;
  vtkCellArray *newPolys;
  vtkScalars *newScalars;
  vtkNormals *newNormals;
  vtkVectors *newGradients;
  long estimatedSize;

  estimatedSize = (int) pow ((double) 
      ((ext[1]-ext[0]+1)*(ext[3]-ext[2]+1)*(ext[5]-ext[4]+1)), .75);
  if (estimatedSize < 1024)
    {
    estimatedSize = 1024;
    }
  newPts = vtkPoints::New();
  newPts->Allocate(estimatedSize,estimatedSize);
  newPolys = vtkCellArray::New();
  newPolys->Allocate(newPolys->EstimateSize(estimatedSize,3));

  if (this->ComputeNormals)
    {
    newNormals = vtkNormals::New();
    newNormals->Allocate(estimatedSize,estimatedSize/2);
    }
  else
    {
    newNormals = NULL;
    }
  
  if (this->ComputeGradients)
    {
    newGradients = vtkVectors::New();
    newGradients->Allocate(estimatedSize,estimatedSize/2);
    }
  else
    {
    newGradients = NULL;
    }

  if (this->ComputeScalars)
    {
    newScalars = vtkScalars::New();
    newScalars->Allocate(estimatedSize,estimatedSize/2);
    }
  else
    {
    newScalars = NULL;
    }

  o->SetPoints(newPts);
  newPts->Delete();

  o->SetPolys(newPolys);
  newPolys->Delete();

  if (newScalars)
    {
    o->GetPointData()->SetScalars(newScalars);
    newScalars->Delete();
    }
  if (newGradients)
    {
    o->GetPointData()->SetVectors(newGradients);
    newGradients->Delete();
    }
  if (newNormals)
    {
    o->GetPointData()->SetNormals(newNormals);
    newNormals->Delete();
    }
}

//----------------------------------------------------------------------------
// Assumes UpdateInformation was called first.
int vtkSynchronizedTemplates3D::GetNumberOfStreamDivisions()
{
  vtkImageData *input = this->GetInput();
  vtkPolyData *output = this->GetOutput();
  int numPieces;
  int *ext, max;
  long memSize, memLimit;
  int num;
  
  // we should really take into consider overlap (later).

  // Hack to get around getting a single piece
  input->SetUpdateExtent(input->GetWholeExtent());
  memSize = input->GetEstimatedUpdateMemorySize();
  // relative to what we are generating
  memSize = memSize / output->GetUpdateNumberOfPieces();
  memLimit = input->GetMemoryLimit();
  // determine the number of divisions needed for the whole data set.
  num = (int)(ceil((float)(memSize) / (float)(memLimit)));

  // lets restrict ourselves to spliting up the z axis
  ext = input->GetWholeExtent();
  max = (ext[5] - ext[4]);
  
  if (num > max)
    {
    num = max;
    }
  
  // if input is already splitting up the output, consider this
  // There must be a better way of doing this.
  numPieces = output->GetUpdateNumberOfPieces();
  num = num / numPieces;

  // At least 1 (truncation)
  if (num == 0)
    {
    num = 1;
    }

  return num;
}


//----------------------------------------------------------------------------
// Assumes UpdateInformation was called first.
int vtkSynchronizedTemplates3D::SplitExtent(int piece, int numPieces,
                                                int *ext)
{
  int numPiecesInFirstHalf;
  int size[3], mid, splitAxis;

  // keep splitting until we have only one piece.
  // piece and numPieces will always be relative to the current ext. 
  while (numPieces > 1)
    {
    // Get the dimensions for each axis.
    size[0] = ext[1]-ext[0];
    size[1] = ext[3]-ext[2];
    size[2] = ext[5]-ext[4];
    // choose the biggest axis
    if (size[2] >= size[1] && size[2] >= size[0] && 
	size[2]/2 >= this->MinimumPieceSize[2])
      {
      splitAxis = 2;
      }
    else if (size[1] >= size[0] && size[1]/2 >= this->MinimumPieceSize[1])
      {
      splitAxis = 1;
      }
    else if (size[0]/2 >= this->MinimumPieceSize[0])
      {
      splitAxis = 0;
      }
    else
      {
      // signal no more splits possible
      splitAxis = -1;
      }

    if (splitAxis == -1)
      {
      // can not split any more.
      if (piece == 0)
        {
        // just return the remaining piece
        numPieces = 1;
        }
      else
        {
        // the rest must be empty
        return 0;
        }
      }
    else
      {
      // split the chosen axis into two pieces.
      numPiecesInFirstHalf = (numPieces / 2);
      mid = (size[splitAxis] * numPiecesInFirstHalf / numPieces) 
	+ ext[splitAxis*2];
      if (piece < numPiecesInFirstHalf)
        {
        // piece is in the first half
        // set extent to the first half of the previous value.
        ext[splitAxis*2+1] = mid;
        // piece must adjust.
        numPieces = numPiecesInFirstHalf;
        }
      else
        {
        // piece is in the second half.
        // set the extent to be the second half. (two halves share points)
        ext[splitAxis*2] = mid;
        // piece must adjust
        numPieces = numPieces - numPiecesInFirstHalf;
        piece -= numPiecesInFirstHalf;
        }
      }
    } // end of while

  return 1;
}

//----------------------------------------------------------------------------
VTK_THREAD_RETURN_TYPE vtkSyncTempThreadedExecute( void *arg )
{
  vtkSynchronizedTemplates3D *self;
  int threadId, threadCount;
  int ext[6], *tmp;
  
  threadId = ((ThreadInfoStruct *)(arg))->ThreadID;
  threadCount = ((ThreadInfoStruct *)(arg))->NumberOfThreads;
  self = (vtkSynchronizedTemplates3D *)
            (((ThreadInfoStruct *)(arg))->UserData);


  // we need to breakup the ExecuteExtent based on the threadId/Count
  tmp = self->GetExecuteExtent();
  ext[0] = tmp[0];
  ext[1] = tmp[1];
  ext[2] = tmp[2];
  ext[3] = tmp[3];
  ext[4] = tmp[4];
  ext[5] = tmp[5];
  if (self->SplitExtent(threadId, threadCount, ext))
    {
    self->ThreadedExecute(self->GetInput(), ext, threadId);
    }
  
  return VTK_THREAD_RETURN_VALUE;
}

//----------------------------------------------------------------------------
void vtkSynchronizedTemplates3D::Execute()
{
  int idx, offset, num, ptIdx, newIdx, numCellPts, *cellPts, newCellPts[3];
  vtkPolyData *output = this->GetOutput();
  vtkPoints *outPts = output->GetPoints();
  vtkPointData *outPD = output->GetPointData();
  vtkCellArray *outTris = output->GetPolys();
  vtkScalars *outScalars = outPD->GetScalars();
  vtkVectors *outGrads = outPD->GetVectors();
  vtkNormals *outNormals = outPD->GetNormals();
  vtkPolyData *threadOut;
  vtkPointData *threadPD;
  vtkCellArray *threadTris;
  vtkScalars *threadScalars;
  vtkVectors *threadGrads;
  vtkNormals *threadNormals;

  if (this->NumberOfThreads == 1)
    {
    // just call the threaded execute directly.
    this->ThreadedExecute(this->GetInput(), this->ExecuteExtent, 0);
    return;
    }

  this->Threader->SetNumberOfThreads(this->NumberOfThreads);

  // Setup threading and the invoke threadedExecute
  this->Threader->SetSingleMethod(vtkSyncTempThreadedExecute, this);
  this->Threader->SingleMethodExecute();

  // Collect all the data into the output.  Now I cannot use append filter
  // because this filter might be streaming.  (Maybe I could if thread
  // 0 wrote to output, and I copied output to a temp polyData...)
  for (idx = 1; idx < this->NumberOfThreads; ++idx)
    {
    threadOut = this->Threads[idx];
    if (threadOut != NULL)
      {
      offset = output->GetNumberOfPoints();
      threadPD = threadOut->GetPointData();
      threadScalars = threadPD->GetScalars();
      threadGrads = threadPD->GetVectors();
      threadNormals = threadPD->GetNormals();
      num = threadOut->GetNumberOfPoints();
      for (ptIdx = 0; ptIdx < num; ++ptIdx)
        {
        newIdx = ptIdx + offset;
        outPts->InsertPoint(newIdx, threadOut->GetPoint(ptIdx));
        if (outScalars)
          {
          outScalars->InsertScalar(newIdx, threadScalars->GetScalar(ptIdx));
          }
        if (outGrads)
          {
          outGrads->InsertVector(newIdx, threadGrads->GetVector(ptIdx));
          }
        if (outNormals)
          {
          outNormals->InsertNormal(newIdx, threadNormals->GetNormal(ptIdx));
          }
        }
      // copy the triangles.
      threadTris = threadOut->GetPolys();
      threadTris->InitTraversal();
      while (threadTris->GetNextCell(numCellPts, cellPts))
        {
        // copy and translate
        if (numCellPts == 3)
          {
          newCellPts[0] = cellPts[0] + offset;
          newCellPts[1] = cellPts[1] + offset;
          newCellPts[2] = cellPts[2] + offset;
          outTris->InsertNextCell(3, newCellPts); 
          }
        }
      threadOut->Delete();
      threadOut = this->Threads[idx] = NULL;         
      }
    }
}

//----------------------------------------------------------------------------
int vtkSynchronizedTemplates3D::ComputeDivisionExtents(vtkDataObject *out,
					       int idx, int numDivisions)
{
  vtkImageData *input = this->GetInput();
  vtkPolyData *output = (vtkPolyData *)out;
  int piece, numPieces;
  int *wholeExt = input->GetWholeExtent();
  int ext[6];

  // Get request from output
  output->GetUpdateExtent(piece, numPieces);

  // Divide this up.
  numPieces *= numDivisions;
  piece = piece * numDivisions + idx;

  // Start with the whole grid.
  input->GetWholeExtent(ext);  

  // get the extent associated with the piece.
  this->SplitExtent(piece, numPieces, ext);

  // As a side product of this call, ExecuteExtent is set.
  // (This may or may not be specific for this filter.)
  this->ExecuteExtent[0] = ext[0];
  this->ExecuteExtent[1] = ext[1];
  this->ExecuteExtent[2] = ext[2];
  this->ExecuteExtent[3] = ext[3];
  this->ExecuteExtent[4] = ext[4];
  this->ExecuteExtent[5] = ext[5];

  // expand if we need to compute gradients
  if (this->ComputeGradients || this->ComputeNormals)
    {
    ext[0] -= 1;
    if (ext[0] < wholeExt[0])
      {
      ext[0] = wholeExt[0];
      }
    ext[1] += 1;
    if (ext[1] > wholeExt[1])
      {
      ext[1] = wholeExt[1];
      }

    ext[2] -= 1;
    if (ext[2] < wholeExt[2])
      {
      ext[2] = wholeExt[2];
      }
    ext[3] += 1;
    if (ext[3] > wholeExt[3])
      {
      ext[3] = wholeExt[3];
      }

    ext[4] -= 1;
    if (ext[4] < wholeExt[4])
      {
      ext[4] = wholeExt[4];
      }
    ext[5] += 1;
    if (ext[5] > wholeExt[5])
      {
      ext[5] = wholeExt[5];
      }
    }

  // Set the update extent of the input.
  input->SetUpdateExtent(ext);
  return 1;
}

//----------------------------------------------------------------------------
void vtkSynchronizedTemplates3D::SetInput(vtkImageData *input)
{
  this->vtkProcessObject::SetInput(0, input);
}

//----------------------------------------------------------------------------
vtkImageData *vtkSynchronizedTemplates3D::GetInput()
{
  if (this->NumberOfInputs < 1)
    {
    return NULL;
    }
  
  return (vtkImageData *)(this->Inputs[0]);
}

  




//----------------------------------------------------------------------------
void vtkSynchronizedTemplates3D::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolyDataSource::PrintSelf(os,indent);

  this->ContourValues->PrintSelf(os,indent);

  os << indent << "Compute Normals: " << (this->ComputeNormals ? "On\n" : "Off\n");
  os << indent << "Compute Gradients: " << (this->ComputeGradients ? "On\n" : "Off\n");
  os << indent << "Compute Scalars: " << (this->ComputeScalars ? "On\n" : "Off\n");
  os << indent << "Number Of Threads: " << this->NumberOfThreads << "\n";
}







