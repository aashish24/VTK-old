/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.


     THIS CLASS IS PATENTED UNDER UNITED STATES PATENT NUMBER 5,542,036
     "Implicit Modeling of Swept Volumes and Swept Surfaces"
     Application of this software for commercial purposes requires 
     a license grant from GE. Contact:

         Carl B. Horton
         Sr. Counsel, Intellectual Property
         3000 N. Grandview Blvd., W-710
         Waukesha, WI  53188
         Phone:  (262) 513-4022
         E-Mail: Carl.Horton@med.ge.com

     for more information.

=========================================================================*/
#include "vtkSweptSurface.h"

#include "vtkDataArray.h"
#include "vtkImageData.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkTransformCollection.h"
#include "vtkVoxel.h"

vtkCxxRevisionMacro(vtkSweptSurface, "$Revision$");
vtkStandardNewMacro(vtkSweptSurface);

vtkCxxSetObjectMacro(vtkSweptSurface,Transforms, vtkTransformCollection);

// Description:
// Construct object with SampleDimensions = (50,50,50), FillValue = 
// VTK_FLOAT ModelBounds=(0,0,0,0,0,0) (i.e, bounds will be
// computed automatically), and Capping turned on.
vtkSweptSurface::vtkSweptSurface()
{
  this->ModelBounds[0] = 0.0; //compute automatically
  this->ModelBounds[1] = 0.0;
  this->ModelBounds[2] = 0.0;
  this->ModelBounds[3] = 0.0;
  this->ModelBounds[4] = 0.0;
  this->ModelBounds[5] = 0.0;

  this->SampleDimensions[0] = 50;
  this->SampleDimensions[1] = 50;
  this->SampleDimensions[2] = 50;

  this->NumberOfInterpolationSteps = 0;
  this->MaximumNumberOfInterpolationSteps = VTK_LARGE_INTEGER;
  this->FillValue = VTK_FLOAT_MAX;
  this->Transforms = NULL;
  this->Capping = 1;

  this->AdjustBounds = 1;
  this->AdjustDistance = 0.040;

  this->T = vtkTransform::New();
}

vtkSweptSurface::~vtkSweptSurface()
{
  this->T->Delete();
  this->SetTransforms(NULL);
}

void vtkSweptSurface::SetModelBounds(double xmin, double xmax, double ymin, 
                                     double ymax, double zmin, double zmax)
{
  double bounds[6];

  bounds[0] = xmin;
  bounds[1] = xmax;
  bounds[2] = ymin;
  bounds[3] = ymax;
  bounds[4] = zmin;
  bounds[5] = zmax;

  this->SetModelBounds(bounds);
}

//----------------------------------------------------------------------------
// Get ALL of the input.
void vtkSweptSurface::ComputeInputUpdateExtent(int inExt[6], 
                                               int *vtkNotUsed(outExt))
{
  int *wholeExtent;

  wholeExtent = this->GetInput()->GetWholeExtent();
  memcpy(inExt, wholeExtent, 6*sizeof(int));
}

void vtkSweptSurface::ExecuteInformation(vtkImageData *input, 
                                         vtkImageData *output)
{
  // make sure there is input
  if (input == NULL)
    {
    vtkErrorMacro(<<"Input is NULL");
    return;
    }
  
  // check that path is defined
  if ( this->Transforms == NULL )
    {
    vtkErrorMacro(<<"No path defined!");
    return;
    }

  if ( this->Transforms->GetNumberOfItems() < 2 )
    {
    vtkErrorMacro(<<"At least two transforms are required to define path!");
    return;
    }

  /*
   *
   */
  output->SetNumberOfScalarComponents(1);
  output->SetScalarType(input->GetScalarType());
  output->SetWholeExtent(0, this->SampleDimensions[0]-1,
                         0, this->SampleDimensions[1]-1,
                         0, this->SampleDimensions[2]-1);

  output->SetSpacing(input->GetSpacing());
  output->SetOrigin(output->GetOrigin());
}

void vtkSweptSurface::ExecuteData(vtkDataObject *)
{
  vtkIdType i, numOutPts;
  vtkPointData *pd;
  vtkDataArray *inScalars, *newScalars;
  double inSpacing[3], inOrigin[3];
  int inDim[3];
  int numSteps, stepNum;
  int numTransforms, transNum;
  vtkTransform *actorTransform; //use an actor-like transform to do position/orientation stuff
  vtkTransform *transform1, *transform2, *t = vtkTransform::New();
  double time;
  // position2 is [4] for GetPoint() call in GetRelativePosition
  double position[3], position1[3], position2[4]; 
  double orient1[3], orient2[3], orientation[3];
  double origin[3], spacing[3], bbox[24];
  vtkImageData *input = this->GetInput();
  vtkImageData *output = this->GetOutput();
  output->SetExtent(output->GetWholeExtent());
  output->AllocateScalars();
  newScalars = output->GetPointData()->GetScalars();
  
  vtkDebugMacro(<<"Creating swept surface");

  // make sure there is input
  if (input == NULL)
    {
    vtkErrorMacro(<<"Input is NULL");
    return;
    }
  pd = input->GetPointData();
  
  if ( pd == NULL )
    {
    vtkErrorMacro(<<"No point data!");
    return;
    }

  inScalars = pd->GetScalars();
  if ( input->GetNumberOfPoints() < 1 ||
  inScalars == NULL )
    {
    vtkErrorMacro(<<"No input scalars defined!");
    t->Delete();
    return;
    }

  // check that path is defined
  if ( this->Transforms == NULL )
    {
    vtkErrorMacro(<<"No path defined!");
    t->Delete();
    return;
    }

  if ( (numTransforms=this->Transforms->GetNumberOfItems()) < 2 )
    {
    vtkErrorMacro(<<"At least two transforms are required to define path!");
    t->Delete();
    return;
    }

  /*
   *
   */
  this->ComputeBounds(origin, spacing, bbox);

  // Get/Set the origin for the actor... for handling case when the input
  // is not centered at 0,0,0
  actorTransform = vtkTransform::New();
  double *bounds = input->GetBounds();
  double actorOrigin[3];

  actorOrigin[0] = (bounds[0]+bounds[1])/2.0;
  actorOrigin[1] = (bounds[2]+bounds[3])/2.0;
  actorOrigin[2] = (bounds[4]+bounds[5])/2.0;

  input->GetDimensions(inDim);
  input->GetSpacing(inSpacing);
  input->GetOrigin(inOrigin);

  //
  // Allocate data.  Scalar "type" is same as input.
  //
  numOutPts = this->SampleDimensions[0] * this->SampleDimensions[1] * 
              this->SampleDimensions[2];
  newScalars->SetNumberOfTuples(numOutPts);
  for (i = 0; i < numOutPts; i++)
    {
    newScalars->SetComponent(i,0,this->FillValue);
    }
  //
  // Sample data at each point in path
  //
  vtkCollectionSimpleIterator sit;
  this->Transforms->InitTraversal(sit);
  transform2 = this->Transforms->GetNextTransform(sit);
  transform2->GetMatrix(t->GetMatrix());

  this->GetRelativePosition(*t,actorOrigin,position2);
  t->GetOrientation(orient2);
  
  for (transNum=0; transNum < (numTransforms-1); transNum++)
    {
    transform1 = transform2;
    transform2 = this->Transforms->GetNextTransform(sit);
    transform2->GetMatrix(t->GetMatrix());

    //
    // Loop over all points (i.e., voxels), 
    // transform into input coordinate system,
    // and obtain interpolated value. Then perform union operation.  
    //
    if ( this->NumberOfInterpolationSteps > 0 ) 
      {
      numSteps = this->NumberOfInterpolationSteps;
      }
    else if ( this->NumberOfInterpolationSteps < 0 ) 
      {
      numSteps = 1;
      }
    else 
      {
      numSteps = this->ComputeNumberOfSteps(transform1,transform2,bbox);
      }
    numSteps = (numSteps > this->MaximumNumberOfInterpolationSteps ?
                this->MaximumNumberOfInterpolationSteps : numSteps);

    // copy state2 to state1 (position and orientation)
    for (i=0; i < 3; i++)
      {
      position1[i] = position2[i];
      orient1[i] = orient2[i];
      }
    this->GetRelativePosition(*t,actorOrigin,position2);
    t->GetOrientation(orient2);

    vtkDebugMacro(<<"Injecting " << numSteps << " steps between transforms "
                  << transNum <<" and "<< transNum+1);
    for (stepNum=0; stepNum < numSteps; stepNum++)
      {
      // interpolate position and orientation
      time = (double) stepNum / numSteps;
      this->InterpolateStates(position1, position2, orient1, orient2, time,
                                       position, orientation); 
      this->SampleInput(this->GetActorMatrixPointer(*actorTransform,
                                                    actorOrigin, position,
                                                    orientation),
                        inDim, inOrigin, inSpacing, inScalars, newScalars);
      }
    }

  //finish off last step
  this->SampleInput(this->GetActorMatrixPointer(*actorTransform, actorOrigin,
                                                position2, orient2),
                    inDim, inOrigin, inSpacing, inScalars, newScalars);

  // Cap if requested
  if ( this->Capping )
    {
    this->Cap(newScalars);
    }

  // Update ourselves and release memory
  t->Delete();
  actorTransform->Delete();
}

void vtkSweptSurface::SampleInput(vtkMatrix4x4 *m, int inDim[3], 
                                  double inOrigin[3], double inSpacing[3], 
                                  vtkDataArray *inScalars, 
                                  vtkDataArray *outScalars)
{
  int i, j, k;
  int inSliceSize=inDim[0]*inDim[1];
  int sliceSize=this->SampleDimensions[0]*this->SampleDimensions[1];
  double x[4], loc[4], newScalar, scalar;
  vtkIdType kOffset, idx;
  int jOffset, ijk[3];
  double weights[8];
  double *origin, *spacing;
  double locP1[4], locP2[4], t[3];
  double dxdi, dydi, dzdi, dxdj, dydj, dzdj, dxdk, dydk, dzdk;
  vtkMatrix4x4 *matrix;
  int indices[6];
  // Compute the index bounds of the workspace volume that will cover the
  // input volume

  this->ComputeFootprint (m, inDim, inOrigin, inSpacing, indices);

  m->Invert(m,m);
  this->T->SetMatrix(m);

  // Now concatenate the shift and scale to convert from world to voxel coordinates
  this->T->PostMultiply();
  this->T->Translate (-inOrigin[0], -inOrigin[1], -inOrigin[2]);
  this->T->Scale (1.0 / inSpacing[0], 1.0 / inSpacing[1], 1.0 / inSpacing[2]);
  this->T->PreMultiply();
  matrix = this->T->GetMatrix();


  x[3] = 1.0; //homogeneous coordinates

  origin = this->GetOutput()->GetOrigin();
  spacing = this->GetOutput()->GetSpacing();

  // Compute the change in voxel coordinates for each step change in world coordinates
  x[0] = origin[0];
  x[1] = origin[1];
  x[2] = origin[2];
  matrix->MultiplyPoint(x,locP1);

  x[0] += spacing[0];
  matrix->MultiplyPoint(x,locP2);

  dxdi = locP2[0] - locP1[0];
  dydi = locP2[1] - locP1[1];
  dzdi = locP2[2] - locP1[2];

  x[0] = origin[0];
  x[1] += spacing[1];
  matrix->MultiplyPoint(x,locP2);

  dxdj = locP2[0] - locP1[0];
  dydj = locP2[1] - locP1[1];
  dzdj = locP2[2] - locP1[2];

  x[1] = origin[1];
  x[2] += spacing[2];
  matrix->MultiplyPoint(x,locP2);

  dxdk = locP2[0] - locP1[0];
  dydk = locP2[1] - locP1[1];
  dzdk = locP2[2] - locP1[2];

  // Compute starting position that is one step before the first world
  // coordinate of each row
  x[0] = origin[0] - spacing[0];
  x[1] = origin[1];
  x[2] = origin[2];
  matrix->MultiplyPoint(x,locP1);

  for (k=indices[4]; k<indices[5]; k++)
    {
    kOffset = k*sliceSize;
    for (j=indices[2]; j<indices[3]; j++)
      {
      jOffset = j*this->SampleDimensions[0];
      loc[0] = locP1[0] + indices[0]*dxdi + j*dxdj + k*dxdk;
      loc[1] = locP1[1] + indices[0]*dydi + j*dydj + k*dydk;
      loc[2] = locP1[2] + indices[0]*dzdi + j*dzdj + k*dzdk;
      for (i=indices[0]; i<indices[1]; i++)
        {
        loc[0] += dxdi;
        loc[1] += dydi;
        loc[2] += dzdi;

        if (loc[0] < 0 || loc[1] < 0 || loc[2] < 0)
          {
          continue;
          }
        ijk[0] = (int)loc[0];
        ijk[1] = (int)loc[1];
        ijk[2] = (int)loc[2];

        //check and make sure point is inside
        if ( (ijk[0] < inDim[0] - 1) && 
             (ijk[1] < inDim[1] - 1) && 
             (ijk[2] < inDim[2] - 1))
          {

          //get scalar values
          t[0] = loc[0] - ijk[0];
          t[1] = loc[1] - ijk[1];
          t[2] = loc[2] - ijk[2];
          vtkVoxel::InterpolationFunctions(t,weights);

          //get scalar values
          idx = ijk[0] + ijk[1]*inDim[0] + ijk[2]*inSliceSize;
          newScalar = inScalars->GetComponent(idx,0) * weights[0];
          newScalar += inScalars->GetComponent(idx+1,0) * weights[1];
          newScalar += inScalars->GetComponent(idx + inDim[0],0) * weights[2];
          newScalar += inScalars->GetComponent(idx+1 + inDim[0],0) * weights[3];
          newScalar += inScalars->GetComponent(idx + inSliceSize,0) * weights[4] ;
          newScalar += inScalars->GetComponent(idx+1 + inSliceSize,0) * weights[5];
          newScalar += inScalars->GetComponent(idx + inDim[0] + inSliceSize,0) *
            weights[6];
          newScalar += inScalars->GetComponent(idx+1 + inDim[0] + inSliceSize,0) *
            weights[7];

          scalar = outScalars->GetComponent((idx=i+jOffset+kOffset),0);
          if ( newScalar < scalar )  //union operation
            {
            outScalars->SetComponent(idx,0,newScalar);
            }
          }
        }
      }
    }
}

void vtkSweptSurface::ComputeFootprint (vtkMatrix4x4 *m, int inDim[3], 
                                        double inOrigin[3], double inSpacing[3],
                                        int indices[6])
{
  int i, ii, n;
  double bounds[6], bbox[24], *fptr, workBounds[6];
  double x[4], xTrans[4];
  double *origin, *spacing;

  this->T->SetMatrix(m);
  for (ii = 0 ; ii < 3; ii++)
    {
    bounds[2 * ii]     = inOrigin[ii];
    bounds[2 * ii + 1] = inOrigin[ii] + (inDim[ii] - 1) * inSpacing[ii];
    }

  bbox[ 0] = bounds[1]; bbox[ 1] = bounds[3]; bbox[ 2] = bounds[5];
  bbox[ 3] = bounds[1]; bbox[ 4] = bounds[2]; bbox[ 5] = bounds[5];
  bbox[ 6] = bounds[0]; bbox[ 7] = bounds[2]; bbox[ 8] = bounds[5];
  bbox[ 9] = bounds[0]; bbox[10] = bounds[3]; bbox[11] = bounds[5];
  bbox[12] = bounds[1]; bbox[13] = bounds[3]; bbox[14] = bounds[4];
  bbox[15] = bounds[1]; bbox[16] = bounds[2]; bbox[17] = bounds[4];
  bbox[18] = bounds[0]; bbox[19] = bounds[2]; bbox[20] = bounds[4];
  bbox[21] = bounds[0]; bbox[22] = bounds[3]; bbox[23] = bounds[4];

  // and transform into work space coordinates
  fptr = bbox;
  x[3] = 1.0;
  for (n = 0; n < 8; n++) 
    {
    x[0] = fptr[0]; x[1] = fptr[1]; x[2] = fptr[2];
    this->T->MultiplyPoint(x,xTrans);

    fptr[0] = xTrans[0];
    fptr[1] = xTrans[1];
    fptr[2] = xTrans[2];
    fptr += 3;
    }

  // now calc the new bounds carefully
  workBounds[0] = bbox[0];
  workBounds[1] = bbox[0];
  workBounds[2] = bbox[1];
  workBounds[3] = bbox[1];
  workBounds[4] = bbox[2];
  workBounds[5] = bbox[2];
  for (i = 0; i < 8; i++)
    {
    for (n = 0; n < 3; n++)
      {
      if (bbox[i*3+n] < workBounds[n*2]) 
        {
        workBounds[n*2] = bbox[i*3+n];
        }
      if (bbox[i*3+n] > workBounds[n*2+1]) 
        {
        workBounds[n*2+1] = bbox[i*3+n];
        }
      }
    }
  origin = this->GetOutput()->GetOrigin();
  spacing = this->GetOutput()->GetSpacing();

  // Compute the footprint of the input in the workspace volume
  for (ii = 0; ii < 3; ii++) 
    {
    indices[2 * ii] = (int) ((workBounds[2 * ii] - origin[ii]) / spacing[ii]);
    indices[2 * ii + 1] = (int) ((workBounds[2 * ii + 1] - origin[ii]) /
                                 spacing[ii]) + 1;
    }
}

unsigned long int vtkSweptSurface::GetMTime()
{
  unsigned long mtime=this->Superclass::GetMTime();
  unsigned long int transMtime;
  vtkTransform *t;
  vtkCollectionSimpleIterator sit;

  if (this->Transforms != NULL)
    {
    for (this->Transforms->InitTraversal(sit); 
         (t = this->Transforms->GetNextTransform(sit)); )
      {
      transMtime = t->GetMTime();
      if ( transMtime > mtime )
        {
        mtime = transMtime;
        }
      }
    }
  
  return mtime;
}

// compute model bounds from geometry and path
void vtkSweptSurface::ComputeBounds(double origin[3], double spacing[3],
                                    double bbox[24])
{
  int i, j, k, ii, idx, dim;
  double *bounds;
  double xmin[3], xmax[3], x[4], xTrans[4], h;
  vtkImageData *input = this->GetInput();

  // Compute eight points of bounding box (used later)
  bounds = input->GetBounds();

  for (idx=0, k=4; k<6; k++) 
    {
    for (j=2; j<4; j++) 
      {
      for (i=0; i<2; i++) 
        {
        bbox[idx++] = bounds[i];
        bbox[idx++] = bounds[j];
        bbox[idx++] = bounds[k];
        }
      }
    }

  // if bounds are not specified, compute bounds from path
  if (this->ModelBounds[0] >= this->ModelBounds[1] ||
      this->ModelBounds[2] >= this->ModelBounds[3] ||
      this->ModelBounds[4] >= this->ModelBounds[5])
    {
    int numTransforms, transNum;
    double position[3], orientation[3], position1[3], orient1[3];
    // position2 is [4] for GetPoint() call in GetRelativePosition
    double position2[4], orient2[3];
    vtkTransform *actorTransform = vtkTransform::New();
    vtkTransform *t, *t2, *transform2;

    t = vtkTransform::New();
    t2 = vtkTransform::New();

    double actorOrigin[3];

    actorOrigin[0] = (bounds[0]+bounds[1])/2.0;
    actorOrigin[1] = (bounds[2]+bounds[3])/2.0;
    actorOrigin[2] = (bounds[4]+bounds[5])/2.0;
    
    double tmpBnds[6];
    vtkMath::UninitializeBounds(tmpBnds);
    xmin[0] = tmpBnds[0];
    xmax[0] = tmpBnds[1];
    xmin[1] = tmpBnds[2];
    xmax[1] = tmpBnds[3];
    xmin[2] = tmpBnds[4];
    xmax[2] = tmpBnds[5];

    numTransforms = this->Transforms->GetNumberOfItems();

    // I don not know how to handle this "correctly"
    // but NULL Pointers should be handled correctly (CLAW)
    if (this->Transforms == NULL)
      {
      vtkErrorMacro("Transforms is NULL");
      return;
      }
        
    vtkCollectionSimpleIterator sit;
    this->Transforms->InitTraversal(sit);
    transform2 = this->Transforms->GetNextTransform(sit);
    transform2->GetMatrix(t->GetMatrix());
    this->GetRelativePosition(*t,actorOrigin,position2);
    t->GetOrientation(orient2);
    // Initialize process with initial transformed position of input
    x[3] = 1.0;
    for (i=0; i<8; i++)
      {
      x[0] = bbox[i*3]; x[1] = bbox[i*3+1]; x[2] = bbox[i*3+2]; 
      t->MultiplyPoint(x,xTrans);
      if ( xTrans[3] != 0.0 )
        {
        for (ii=0; ii<3; ii++)
          {
          xTrans[ii] /= xTrans[3];
          }
        }
      for (j=0; j<3; j++)
        {
        if (xTrans[j] < xmin[j])
          {
          xmin[j] = xTrans[j];
          }
        if (xTrans[j] > xmax[j])
          {
          xmax[j] = xTrans[j];
          }
        }
      }

    for (transNum=0; transNum < (numTransforms-1); transNum++)
      {
      transform2 = this->Transforms->GetNextTransform(sit);
      transform2->GetMatrix(t->GetMatrix());
      for (i = 0; i < 3; i++)
        {
        position1[i] = position2[i];
        orient1[i] = orient2[i];
        }
      this->GetRelativePosition(*t,actorOrigin,position2);
      t->GetOrientation(orient2);

      // Sample inbetween matrices to compute better bounds. 
      // Use 4 steps (arbitrary),
      h = 0.25;
      for (k=1; k <= 4; k++ ) 
        {
        this->InterpolateStates(position1, position2, orient1, orient2, k*h,
                                       position, orientation); 
        t2->SetMatrix(this->GetActorMatrixPointer(*actorTransform, actorOrigin,
                                                  position,orientation));

        for (i=0; i<8; i++) //loop over eight corners of bounding box
          {
          x[0] = bbox[i*3]; x[1] = bbox[i*3+1]; x[2] = bbox[i*3+2]; 
          t2->MultiplyPoint(x,xTrans);
          if ( xTrans[3] != 0.0 ) 
            {
            for (ii=0; ii<3; ii++)
              {
              xTrans[ii] /= xTrans[3];
              }
            }
          for (j=0; j<3; j++)
            {
            if (xTrans[j] < xmin[j])
              {
              xmin[j] = xTrans[j];
              }
            if (xTrans[j] > xmax[j])
              {
              xmax[j] = xTrans[j];
              }
            }
          }
        }
      }
    t->Delete();
    t2->Delete();
    actorTransform->Delete();
    } //if compute model bounds

  else // else use model bounds specified
    {
    for (i=0; i<3; i++)
      {
      origin[i] = this->ModelBounds[2*i];
      spacing[i] = (this->ModelBounds[2*i+1] - this->ModelBounds[2*i]);
      }
    }

  // Assumption (or forced?) that won't/can't specify bounds and adjust
  // bounds?
  if ( this->AdjustBounds )
    {
    // adjust bounds larger to make sure data lies within volume
    for (i=0; i<3; i++)
      {
      spacing[i] = (xmax[i]-xmin[i]);
      h = this->AdjustDistance * spacing[i];
      xmin[i] -= h;
      xmax[i] += h;
      spacing[i] = xmax[i] - xmin[i];
      }
    }

  vtkDebugMacro(<<"Computed model bounds as (" << xmin[0] << "," << xmax[0]
                << ", " << xmin[1] << "," << xmax[1]
                << ", " << xmin[2] << "," << xmax[2] << ")");

  // set output
  for (i=0; i<3; i++)
    {
    origin[i] = xmin[i];
    if ( (dim=this->SampleDimensions[i]) <= 1 )
      {
      dim=2;
      }
    if ( (spacing[i]=spacing[i]/(dim-1)) == 0.0 )
      {
      spacing[i] = 1.0;
      }
    }

  this->GetOutput()->SetOrigin(origin);
  this->GetOutput()->SetSpacing(spacing);
}

// based on both path and bounding box of input, compute the number of 
// steps between the specified transforms
int vtkSweptSurface::ComputeNumberOfSteps(vtkTransform *t1, vtkTransform *t2, 
                                          double bbox[24])
{
  double x[4], xTrans1[4], xTrans2[4];
  double dist2, maxDist2;
  double h, *spacing;
  int numSteps, i, j;
  
  // Compute maximum distance between points.
  x[3] = 1.0;
  for (maxDist2=0.0, i=0; i<8; i++)
    {
    for (j=0; j<3; j++)
      {
      x[j] = bbox[3*i+j];
      }
    t1->MultiplyPoint(x,xTrans1);
    if ( xTrans1[3] != 0.0 )
      {
      for (j=0; j<3; j++)
        {
        xTrans1[j] /= xTrans1[3];
        }
      }
    t2->MultiplyPoint(x,xTrans2);
    if ( xTrans2[3] != 0.0 )
      {
      for (j=0; j<3; j++)
        {
        xTrans2[j] /= xTrans2[3];
        }
      }
    dist2 = vtkMath::Distance2BetweenPoints((double *)xTrans1,(double *)xTrans2);
    if ( dist2 > maxDist2 )
      {
      maxDist2 = dist2;
      }
    }

  // use magic factor to convert to number of steps. Takes into account
  // rotation (assuming maximum 90 degrees), data spacing of output, and 
  // effective size of voxel.
  spacing = this->GetOutput()->GetSpacing();
  h = sqrt(spacing[0]*spacing[0] + spacing[1]*spacing[1] + 
           spacing[2]*spacing[2]) / 2.0;
  numSteps = (int) ((double)(1.414 * sqrt((double)maxDist2)) / h);

  if ( numSteps <= 0 )
    {
    return 1;
    }
  else
    {
    return numSteps;
    }
}

void vtkSweptSurface::Cap(vtkDataArray *s)
{
  int i,j,k;
  vtkIdType idx;
  int d01=this->SampleDimensions[0]*this->SampleDimensions[1];

// i-j planes
  k = 0;
  for (j=0; j<this->SampleDimensions[1]; j++)
    {
    for (i=0; i<this->SampleDimensions[0]; i++)
      {
      s->SetComponent(i+j*this->SampleDimensions[0], 0, this->FillValue);
      }
    }
  k = this->SampleDimensions[2] - 1;
  idx = k*d01;
  for (j=0; j<this->SampleDimensions[1]; j++)
    {
    for (i=0; i<this->SampleDimensions[0]; i++)
      {
      s->SetComponent(idx+i+j*this->SampleDimensions[0], 0, this->FillValue);
      }
    }
// j-k planes
  i = 0;
  for (k=0; k<this->SampleDimensions[2]; k++)
    {
    for (j=0; j<this->SampleDimensions[1]; j++)
      {
      s->SetComponent(j*this->SampleDimensions[0]+k*d01, 0, this->FillValue);
      }
    }
  i = this->SampleDimensions[0] - 1;
  for (k=0; k<this->SampleDimensions[2]; k++)
    {
    for (j=0; j<this->SampleDimensions[1]; j++)
      {
      s->SetComponent(i+j*this->SampleDimensions[0]+k*d01, 0, this->FillValue);
      }
    }
// i-k planes
  j = 0;
  for (k=0; k<this->SampleDimensions[2]; k++)
    {
    for (i=0; i<this->SampleDimensions[0]; i++)
      {
      s->SetComponent(i+k*d01, 0, this->FillValue);
      }
    }
  j = this->SampleDimensions[1] - 1;
  idx = j*this->SampleDimensions[0];
  for (k=0; k<this->SampleDimensions[2]; k++)
    {
    for (i=0; i<this->SampleDimensions[0]; i++)
      {
      s->SetComponent(idx+i+k*d01, 0, this->FillValue);
      }
    }
}

void vtkSweptSurface::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Sample Dimensions: (" << this->SampleDimensions[0] << ", "
               << this->SampleDimensions[1] << ", "
               << this->SampleDimensions[2] << ")\n";
  os << indent << "ModelBounds: \n";
  os << indent << "  Xmin,Xmax: (" << this->ModelBounds[0] << ", " << this->ModelBounds[1] << ")\n";
  os << indent << "  Ymin,Ymax: (" << this->ModelBounds[2] << ", " << this->ModelBounds[3] << ")\n";
  os << indent << "  Zmin,Zmax: (" << this->ModelBounds[4] << ", " << this->ModelBounds[5] << ")\n";

  os << indent << "Fill Value:" << this->FillValue << "\n";
  os << indent << "Capping: " << (this->Capping ? "On\n" : "Off\n");

  os << indent << "Adjust Bounds: " << (this->AdjustBounds ? "On\n" : "Off\n");
  os << indent << "Adjust Distance: " << this->AdjustDistance << "\n";

  os << indent << "Interpolation Steps: " << this->NumberOfInterpolationSteps << "\n";
  os << indent << "Max Interp. Steps: " << this->MaximumNumberOfInterpolationSteps << "\n";

  if ( this->Transforms )
    {
    os << indent << "Number of Transforms: " << this->Transforms->GetNumberOfItems() << "\n";
    }
  else
    {
    os << indent << "No transform defined!\n";
    }
}

void vtkSweptSurface::GetRelativePosition(vtkTransform &t,double *origin,
                                          double *position)
{
  // get position relative to the origin (of the geometry)
  t.TransformPoint(origin,position);
  position[0] -= origin[0];
  position[1] -= origin[1];
  position[2] -= origin[2];
}

void vtkSweptSurface::InterpolateStates(double *pos1, double *pos2, 
                                        double *euler1, double *euler2, double t,
                                        double *posOut, double *eulerOut)
{
  for (int i=0; i < 3; i++)
    {
    posOut[i] = pos1[i] + t*(pos2[i] - pos1[i]);
    eulerOut[i] = euler1[i] + t*(euler2[i] - euler1[i]);
    }
}

// Simulate an actor's transform without all of the baggage of an actor
vtkMatrix4x4* vtkSweptSurface::GetActorMatrixPointer(vtkTransform &t,
                                                     double origin[3],
                                                     double position[3],
                                                     double orientation[3])
{
  t.Identity();  
  t.PostMultiply();  
    
  // shift back to actor's origin
  t.Translate(-origin[0],
              -origin[1],
              -origin[2]);

  // rotate
  t.RotateY(orientation[1]);
  t.RotateX(orientation[0]);
  t.RotateZ(orientation[2]);
    
  // move back from origin and translate
  t.Translate(origin[0] + position[0],
              origin[1] + position[1],
              origin[2] + position[2]);

  return t.GetMatrix();
}
