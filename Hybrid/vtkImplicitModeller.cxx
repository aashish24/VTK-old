/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkImplicitModeller.h"

#include "vtkCell.h"
#include "vtkCellLocator.h"
#include "vtkClipPolyData.h"
#include "vtkCommand.h"
#include "vtkFloatArray.h"
#include "vtkGenericCell.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkMath.h"
#include "vtkMultiThreader.h"
#include "vtkMutexLock.h"
#include "vtkObjectFactory.h"
#include "vtkPlane.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkRectilinearGrid.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"

#include <math.h>

vtkCxxRevisionMacro(vtkImplicitModeller, "$Revision$");
vtkStandardNewMacro(vtkImplicitModeller);

struct vtkImplicitModellerAppendInfo
{
  vtkImplicitModeller *Modeller;
  vtkDataSet          **Input;
  double               MaximumDistance;
  vtkMutexLock        *ProgressMutex;
};

//----------------------------------------------------------------------------
// Construct with sample dimensions=(50,50,50), and so that model bounds are
// automatically computed from the input. Capping is turned on with CapValue
// equal to a large positive number.
vtkImplicitModeller::vtkImplicitModeller()
{
  this->MaximumDistance = 0.1;

  this->ModelBounds[0] = 0.0;
  this->ModelBounds[1] = 0.0;
  this->ModelBounds[2] = 0.0;
  this->ModelBounds[3] = 0.0;
  this->ModelBounds[4] = 0.0;
  this->ModelBounds[5] = 0.0;
  this->BoundsComputed = 0;

  this->SampleDimensions[0] = 50;
  this->SampleDimensions[1] = 50;
  this->SampleDimensions[2] = 50;

  this->Capping = 1;
  this->CapValue = sqrt(1.0e29) / 3.0;

  this->DataAppended = 0;
  this->AdjustBounds = 1;
  this->AdjustDistance = 0.0125;

  this->ProcessMode = VTK_CELL_MODE;
  this->LocatorMaxLevel = 5;

  this->Threader                     = vtkMultiThreader::New();
  this->NumberOfThreads              = this->Threader->GetNumberOfThreads();
}

vtkImplicitModeller::~vtkImplicitModeller()
{
  if (this->Threader)
    {
    this->Threader->Delete();
    }
}

//----------------------------------------------------------------------------

void vtkImplicitModeller::UpdateData(vtkDataObject *output)
{
  if (this->GetInput() == NULL)
    {
    // we do not want to release the data because user might
    // have called Append ...
    return;
    }

  this->vtkDataSetToImageFilter::UpdateData( output );
}

//----------------------------------------------------------------------------
// Initialize the filter for appending data. You must invoke the
// StartAppend() method before doing successive Appends(). It's also a
// good idea to manually specify the model bounds; otherwise the input
// bounds for the data will be used.
void vtkImplicitModeller::StartAppend()
{
  vtkIdType numPts;
  vtkIdType i;
  double maxDistance;

  // we must call execute information because we can't be sure that 
  // it has been called.
  this->ExecuteInformation();
  this->GetOutput()->SetUpdateExtent(this->GetOutput()->GetWholeExtent());
  
  vtkDebugMacro(<< "Initializing data");
  this->AllocateOutputData(this->GetOutput());
  vtkFloatArray *newScalars = vtkFloatArray::SafeDownCast(
    this->GetOutput()->GetPointData()->GetScalars());
  this->UpdateProgress(0.0);
  this->DataAppended = 1;

  numPts = this->SampleDimensions[0] * this->SampleDimensions[1] 
           * this->SampleDimensions[2];
  maxDistance = this->CapValue * this->CapValue;//sqrt taken later
  for (i=0; i<numPts; i++)
    {
    newScalars->SetComponent(i, 0, maxDistance);
    }
}

//----------------------------------------------------------------------------
// This is the multithreaded piece of the append when doing per voxel
// processing - it is called once pfor each thread, with each thread
// taking a different slab of the output to work on.
static VTK_THREAD_RETURN_TYPE vtkImplicitModeller_ThreadedAppend( void *arg )
{
  int                      thread_count;
  int                      thread_id;
  vtkImplicitModellerAppendInfo *userData;
  vtkImageData *output;
  double maxDistance;
  int i, j, k;
  double *bounds, adjBounds[6];
  double pcoords[3];
  vtkDataArray *newScalars;
  int idx, subId;
  vtkIdType cellId;
  int min[3], max[3];
  double x[3], prevDistance2, distance2;
  int jkFactor;
  double closestPoint[3], mDist;
  double *Spacing;
  double *origin;
  double *weights;
  double maxDistance2;
  int slabSize, slabMin, slabMax;
  vtkMutexLock *mutex;

  thread_id = ((vtkMultiThreader::ThreadInfo *)(arg))->ThreadID;
  thread_count = ((vtkMultiThreader::ThreadInfo *)(arg))->NumberOfThreads;
  userData = (vtkImplicitModellerAppendInfo *)
    (((vtkMultiThreader::ThreadInfo *)(arg))->UserData);
  mutex = userData->ProgressMutex;


  if (userData->Input[thread_id] == NULL)
    {
    return VTK_THREAD_RETURN_VALUE;
    }

  maxDistance = userData->MaximumDistance;
  maxDistance2 = maxDistance * maxDistance;

  output = userData->Modeller->GetOutput();
  Spacing = output->GetSpacing();
  origin = output->GetOrigin();

  int *sampleDimensions = userData->Modeller->GetSampleDimensions();
  if (!(newScalars = output->GetPointData()->GetScalars()))
    {
    vtkGenericWarningMacro("Sanity check failed.");
    return VTK_THREAD_RETURN_VALUE;
    }

  // break up into slabs based on thread_id and thread_count
  slabSize = sampleDimensions[2] / thread_count;
  if (slabSize == 0) // in case thread_count >  sampleDimensions[2]
    {
    slabSize = 1;
    }
  slabMin = thread_id * slabSize;
  if (slabMin >= sampleDimensions[2])
    {
    return VTK_THREAD_RETURN_VALUE;
    }
  slabMax = slabMin + slabSize - 1;
  if (thread_id == thread_count - 1)
    {
    slabMax = sampleDimensions[2] - 1;
    }


  bounds = userData->Input[thread_id]->GetBounds();
  for (i=0; i<3; i++)
    {
    adjBounds[2*i] = bounds[2*i] - maxDistance;
    adjBounds[2*i+1] = bounds[2*i+1] + maxDistance;
    }
  
  // compute dimensional bounds in data set
  for (i=0; i<3; i++)
    {
    min[i] = (int) ((double)(adjBounds[2*i] - origin[i]) / 
      Spacing[i]);
    max[i] = (int) ((double)(adjBounds[2*i+1] - origin[i]) / 
      Spacing[i]);
    if (min[i] < 0)
      {
      min[i] = 0;
      }
    if (max[i] >= sampleDimensions[i])
      {
      max[i] = sampleDimensions[i] - 1;
      }
    }

  // input not close enough to effect this slab
  if (min[2] > slabMax || max[2] < slabMin) 
    {
    return VTK_THREAD_RETURN_VALUE;
    }

  // adjust min/max to match slab
  if (min[2] < slabMin)
    {
    min[2] = slabMin;
    }
  if (max[2] > slabMax)
    {
    max[2] = slabMax;
    }

  
  // allocate weights for the EvaluatePosition
  weights = new double[userData->Input[thread_id]->GetMaxCellSize()];

  //
  // Traverse each voxel; using CellLocator to find the closest point
  //
  vtkGenericCell *cell = vtkGenericCell::New();
  vtkCellLocator *locator = vtkCellLocator::New();
  
  // Set up the cell locator.
  // If AutomaticOff, then NumberOfCellsPerBucket only used for allocating
  // memory.  If AutomaticOn, then NumberOfCellsPerBucket is used to guess
  // the depth for the uniform octree required to support
  // NumberOfCellsPerBucket (assuming uniform distribution of cells).
  locator->SetDataSet( userData->Input[thread_id] );
  locator->AutomaticOff();
  locator->SetMaxLevel( userData->Modeller->GetLocatorMaxLevel() );
  locator->SetNumberOfCellsPerBucket( 1 );  
  locator->CacheCellBoundsOn();
  locator->BuildLocator();
  
  // for pregress update, compute portion of final output for each sub-plane 
  // completed
  double progressUpdate = 
    (double)(slabMax - slabMin + 1) / (double)sampleDimensions[2]  // if did whole slab
    / (max[2] - min[2] + 1); // divided by portion of slab we actually do

  jkFactor = sampleDimensions[0]*sampleDimensions[1];
  for (k = min[2]; k <= max[2]; k++) 
    {
    x[2] = Spacing[2] * k + origin[2];
    for (j = min[1]; j <= max[1]; j++)
      {
      cellId = -1;
      x[1] = Spacing[1] * j + origin[1];
      for (i = min[0]; i <= max[0]; i++) 
        {
        x[0] = Spacing[0] * i + origin[0];
        idx = jkFactor*k + sampleDimensions[0]*j + i;
        prevDistance2 = newScalars->GetComponent(idx, 0);
        
        if (cellId != -1)
          {
          cell->EvaluatePosition(x, closestPoint, subId, pcoords,
            distance2, weights);
          if (distance2 <= maxDistance2 && distance2 < prevDistance2)
            {
            mDist = sqrt(distance2);
            newScalars->SetComponent(idx,0,distance2);
            }
          else if (prevDistance2 < maxDistance2)
            {
            mDist = sqrt(prevDistance2);
            }
          else
            {
            mDist = maxDistance;
            }
          }
        else if (prevDistance2 < maxDistance2)
          {
          mDist = sqrt(prevDistance2);
          }
        else
          {
          mDist = maxDistance;
          }
        
        if (locator->FindClosestPointWithinRadius(x, mDist,
          closestPoint, cell, cellId, subId, distance2) )
          {
          if(distance2 <= prevDistance2)
            {
            newScalars->SetComponent(idx,0,distance2);
            }
          }
        else
          {
          cellId = -1;
          }
        }
      }
    if (mutex)
      {
      mutex->Lock();
      }
    userData->Modeller->UpdateProgress(
      userData->Modeller->GetProgress() + progressUpdate);
    if (mutex)
      {
      mutex->Unlock();
      }
    }
  locator->Delete();
  cell->Delete();

  delete [] weights;
  return VTK_THREAD_RETURN_VALUE;
}



// Append a data set to the existing output. To use this function,
// you'll have to invoke the StartAppend() method before doing
// successive appends. It's also a good idea to specify the model
// bounds; otherwise the input model bounds is used. When you've
// finished appending, use the EndAppend() method.
void vtkImplicitModeller::Append(vtkDataSet *input)
{
  double *Spacing, *origin;

  vtkDebugMacro(<< "Appending data");

  vtkImageData *output = this->GetOutput();

  if ( !this->BoundsComputed )
    {
    this->ComputeModelBounds(input);
    }

  Spacing = output->GetSpacing();
  origin = output->GetOrigin();

  if (this->ProcessMode == VTK_CELL_MODE)
    {
    int i, j, k, updateTime;
    vtkIdType cellNum;
    double *bounds, adjBounds[6];
    double pcoords[3];
    vtkDataArray *newScalars;
    int idx;
    int min[3], max[3];
    double x[3], prevDistance2, distance2;
    int jkFactor, subId;
    double closestPoint[3];
    double *weights=new double[input->GetMaxCellSize()];
    double maxDistance2;
    // Get the output scalars
    if (!(newScalars = output->GetPointData()->GetScalars()))
      {
      vtkErrorMacro("Sanity check failed.");
      return;
      }
    
    maxDistance2 = this->InternalMaxDistance * this->InternalMaxDistance;
    
    //
    // Traverse all cells; computing distance function on volume points.
    //
    vtkCell *cell;
    updateTime = input->GetNumberOfCells() / 50;  // update every 2%
    if (updateTime < 1)
      {
      updateTime = 1;
      }
    for (cellNum=0; cellNum < input->GetNumberOfCells(); cellNum++)
      {
      cell = input->GetCell(cellNum);
      bounds = cell->GetBounds();
      for (i=0; i<3; i++)
        {
        adjBounds[2*i] = bounds[2*i] - this->InternalMaxDistance;
        adjBounds[2*i+1] = bounds[2*i+1] + this->InternalMaxDistance;
        }
      
      // compute dimensional bounds in data set
      for (i=0; i<3; i++)
        {
        min[i] = (int) ((double)(adjBounds[2*i] - origin[i]) / 
          Spacing[i]);
        max[i] = (int) ((double)(adjBounds[2*i+1] - origin[i]) / 
          Spacing[i]);
        if (min[i] < 0)
          {
          min[i] = 0;
          }
        if (max[i] >= this->SampleDimensions[i])
          {
          max[i] = this->SampleDimensions[i] - 1;
          }
        }
      
      jkFactor = this->SampleDimensions[0]*this->SampleDimensions[1];
      for (k = min[2]; k <= max[2]; k++) 
        {
        x[2] = Spacing[2] * k + origin[2];
        for (j = min[1]; j <= max[1]; j++)
          {
          x[1] = Spacing[1] * j + origin[1];
          for (i = min[0]; i <= max[0]; i++) 
            {
            x[0] = Spacing[0] * i + origin[0];
            idx = jkFactor*k + this->SampleDimensions[0]*j + i;
            prevDistance2 = newScalars->GetComponent(idx, 0);
            
            // union combination of distances
            if ( cell->EvaluatePosition(x, closestPoint, subId, pcoords, 
              distance2, weights) != -1 && distance2 < prevDistance2 &&
              distance2 <= maxDistance2 ) 
              {
              newScalars->SetComponent(idx,0,distance2);
              }
            }
          }
        }

      if (cellNum % updateTime == 0)
        {
        this->UpdateProgress(double(cellNum + 1) / input->GetNumberOfCells());
        }
      }
    delete [] weights;
    }
  else
    {
    vtkImplicitModellerAppendInfo info;
    double minZ, maxZ;
    int slabMin, slabMax, slabSize, i;
    vtkClipPolyData **minClipper = NULL, **maxClipper = NULL; 
    vtkPlane ** minPlane = NULL, **maxPlane = NULL;
    
    // Use a MultiThreader here, splitting the volume into slabs to be processed
    // by the separate threads
    
    // Set the number of threads to use,
    // then set the execution method and do it.
    this->Threader->SetNumberOfThreads( this->NumberOfThreads );
    
      // set up the info object for the thread
    info.Modeller = this;
    info.MaximumDistance = this->InternalMaxDistance;

    info.Input = new vtkDataSet* [this->NumberOfThreads];
    if (this->NumberOfThreads == 1)
      {
      info.Input[0] = input;
      info.ProgressMutex = NULL;
      }
    else
      {
      // if not PolyData, then copy the input for each thread
      if ( input->GetDataObjectType() != VTK_POLY_DATA )
        {
        for (i = 0; i < this->NumberOfThreads; i++)
          {
          switch( input->GetDataObjectType() )
            {
            case VTK_STRUCTURED_GRID:
              info.Input[i] = vtkStructuredGrid::New();
              break;
            case VTK_IMAGE_DATA:
              info.Input[i] = vtkImageData::New();
              break;
            case VTK_UNSTRUCTURED_GRID:
              info.Input[i] = vtkUnstructuredGrid::New();
              break;
            case VTK_RECTILINEAR_GRID:
              info.Input[i] = vtkRectilinearGrid::New();
              break;
            default:
              vtkErrorMacro(<<"Unexpected DataSet type!");
              return;
            }
          info.Input[i]->CopyStructure(input);
          }
        }
      else // break up the input data into slabs to help ensure thread safety

        {
        minClipper = new vtkClipPolyData* [this->NumberOfThreads];
        maxClipper = new vtkClipPolyData* [this->NumberOfThreads];
        minPlane = new vtkPlane* [this->NumberOfThreads];
        maxPlane = new vtkPlane* [this->NumberOfThreads];
        
        slabSize = this->SampleDimensions[2] / this->NumberOfThreads;
        if (slabSize == 0) // in case thread_count >  SampleDimensions[2]
          {
          slabSize = 1;
          }

        for (i = 0; i < this->NumberOfThreads; i++)
          {
          //////////////////////////////////////////////////
          // do the 1st clip
          slabMin = i * slabSize;
          if (slabMin >= this->SampleDimensions[2])
            {
            break;
            }

          // get/clip input cells in this slab + maxDistance+ 
          minZ = Spacing[2] * slabMin + origin[2] - this->InternalMaxDistance*1.00001;
          if (minZ < this->ModelBounds[4])
            {
            minZ = this->ModelBounds[4];
            }

          minPlane[i] = vtkPlane::New();
          minPlane[i]->SetNormal(0.0, 0.0, -1.0);
          minPlane[i]->SetOrigin(0.0, 0.0, minZ);

          minClipper[i] = vtkClipPolyData::New();
          minClipper[i]->SetInput((vtkPolyData *)input);
          minClipper[i]->SetClipFunction(minPlane[i]);
            minClipper[i]->SetValue( 0.0 );
                minClipper[i]->InsideOutOn();
          minClipper[i]->Update();

          if ( minClipper[i]->GetOutput()->GetNumberOfCells() == 0 )
            {
            info.Input[i] = NULL;
            maxPlane[i] = NULL;
            continue;
            }
          minClipper[i]->ReleaseDataFlagOn();

          //////////////////////////////////////////////////
          // do the 2nd clip
          slabMax = slabMin + slabSize - 1;
          if (i == this->NumberOfThreads - 1)
            {
            slabMax = this->SampleDimensions[2] - 1;
            }
          
          maxZ = Spacing[2] * slabMax + origin[2] + this->InternalMaxDistance*1.00001;
          if (maxZ > this->ModelBounds[5])
            {
            maxZ = this->ModelBounds[5];
            }
          maxPlane[i] = vtkPlane::New();
          maxPlane[i]->SetNormal(0.0, 0.0, 1.0);
          maxPlane[i]->SetOrigin(0.0, 0.0, maxZ);

          maxClipper[i] = vtkClipPolyData::New();
          maxClipper[i]->SetInput(minClipper[i]->GetOutput());
          maxClipper[i]->SetClipFunction(maxPlane[i]);
            maxClipper[i]->SetValue( 0.0 );
                maxClipper[i]->InsideOutOn();
          maxClipper[i]->Update();

          if ( maxClipper[i]->GetOutput()->GetNumberOfCells() == 0 )
            {
            info.Input[i] = NULL;
            }
          else
            {
            info.Input[i] = maxClipper[i]->GetOutput();
            }
          }
        }
      if (this->HasObserver(vtkCommand::ProgressEvent))
        {
        info.ProgressMutex = vtkMutexLock::New();
        }
      else
        {
        info.ProgressMutex = NULL;
        }
      }

    this->Threader->SetSingleMethod( vtkImplicitModeller_ThreadedAppend, 
      (void *)&info);
    this->Threader->SingleMethodExecute();

    // cleanup
    if (this->NumberOfThreads > 1)
      {
      if (info.ProgressMutex)
        {
        info.ProgressMutex->Delete();
        }
      if ( input->GetDataObjectType() != VTK_POLY_DATA )
        {
        for (i = 0; i < this->NumberOfThreads; i++)
          {
          info.Input[i]->Delete();
          }
        }
      else
        {
        for (i = 0; i < this->NumberOfThreads; i++)
          {
          minPlane[i]->Delete();
          minClipper[i]->Delete();
          if (maxPlane[i])
            {
            maxPlane[i]->Delete();
            maxClipper[i]->Delete();
            }
          }
        delete [] minPlane;
        delete [] maxPlane;
        delete [] minClipper;
        delete [] maxClipper;
        }
      }
    delete [] info.Input;
    }
}

//----------------------------------------------------------------------------
// Method completes the append process.
void vtkImplicitModeller::EndAppend()
{
  vtkDataArray *newScalars;
  vtkIdType i, numPts;
  double distance2;

  vtkDebugMacro(<< "End append");
  
  if (!(newScalars =this->GetOutput()->GetPointData()->GetScalars()))
    {
    vtkErrorMacro("Sanity check failed.");
    return;
    }
  numPts = newScalars->GetNumberOfTuples();
  //
  // Run through scalars and take square root
  //
  for (i=0; i<numPts; i++)
    {
    distance2 = newScalars->GetComponent(i,0);
    newScalars->SetComponent(i,0,sqrt(distance2));
    }
  //
  // If capping is turned on, set the distances of 
  // the outside faces of the volume
  // to the CapValue.
  //
  if ( this->Capping )
    {
    this->Cap(newScalars);
    }
  this->UpdateProgress(1.0);
}


//----------------------------------------------------------------------------
void vtkImplicitModeller::ExecuteInformation()
{
  int i;
  double ar[3], origin[3];
  vtkImageData *output = this->GetOutput();
  
  output->SetScalarType(VTK_FLOAT);
  output->SetNumberOfScalarComponents(1);
  
  output->SetWholeExtent(0, this->SampleDimensions[0]-1,
                         0, this->SampleDimensions[1]-1,
                         0, this->SampleDimensions[2]-1);

  for (i=0; i < 3; i++)
    {
    origin[i] = this->ModelBounds[2*i];
    if ( this->SampleDimensions[i] <= 1 )
      {
      ar[i] = 1;
      }
    else
      {
      ar[i] = (this->ModelBounds[2*i+1] - this->ModelBounds[2*i])
              / (this->SampleDimensions[i] - 1);
      }
    }
  output->SetOrigin(origin);
  output->SetSpacing(ar);
}


//----------------------------------------------------------------------------
void vtkImplicitModeller::ExecuteData(vtkDataObject *)
{
  vtkDebugMacro(<< "Executing implicit model");

  if (this->GetInput() == NULL)
    {
    // we do not want to release the data because user might
    // have called Append ...
    return;
    }

  this->StartAppend();
  this->Append(this->GetInput());
  this->EndAppend();
}

// Compute ModelBounds from input geometry.
double vtkImplicitModeller::ComputeModelBounds(vtkDataSet *input)
{
  double *bounds, maxDist;
  int i;
  vtkImageData *output=this->GetOutput();
  double tempf[3];
  
  // compute model bounds if not set previously
  if ( this->ModelBounds[0] >= this->ModelBounds[1] ||
       this->ModelBounds[2] >= this->ModelBounds[3] ||
       this->ModelBounds[4] >= this->ModelBounds[5] )
    {
    if (input != NULL)
      {
      bounds = input->GetBounds();
      }
    else
      {
      if (this->GetInput() != NULL)
        {
        bounds = this->GetInput()->GetBounds();
        }
      else
        {
        vtkErrorMacro( << "An input must be specified to Compute the model bounds.");
        return VTK_FLOAT_MAX;
        }
      }
    }
  else
    {
    bounds = this->ModelBounds;
    }

  for (maxDist=0.0, i=0; i<3; i++)
    {
    if ( (bounds[2*i+1] - bounds[2*i]) > maxDist )
      {
      maxDist = bounds[2*i+1] - bounds[2*i];
      }
    }

  // adjust bounds so model fits strictly inside (only if not set previously)
  if ( this->AdjustBounds )
    {
    for (i=0; i<3; i++)
      {
      this->ModelBounds[2*i] = bounds[2*i] - maxDist*this->AdjustDistance;
      this->ModelBounds[2*i+1] = bounds[2*i+1] + maxDist*this->AdjustDistance;
      }
    }
  else  // to handle problem case where bounds not specified and AdjustBounds
    //  not on; will be setting ModelBounds to self if previosusly set
    {
    for (i=0; i<3; i++)
      {
      this->ModelBounds[2*i] = bounds[2*i];
      this->ModelBounds[2*i+1] = bounds[2*i+1];
      }
    }

  maxDist *= this->MaximumDistance;

  // Set volume origin and data spacing
  output->SetOrigin(this->ModelBounds[0],this->ModelBounds[2],
                    this->ModelBounds[4]);
  
  for (i=0; i<3; i++)
    {
    tempf[i] = (this->ModelBounds[2*i+1] - this->ModelBounds[2*i])
      / (this->SampleDimensions[i] - 1);
    }
  output->SetSpacing(tempf);

  this->BoundsComputed = 1;
  this->InternalMaxDistance = maxDist;
  
  return maxDist;  
}

//----------------------------------------------------------------------------
// Set the i-j-k dimensions on which to sample the distance function.
void vtkImplicitModeller::SetSampleDimensions(int i, int j, int k)
{
  int dim[3];

  dim[0] = i;
  dim[1] = j;
  dim[2] = k;

  this->SetSampleDimensions(dim);
}

//----------------------------------------------------------------------------
void vtkImplicitModeller::SetSampleDimensions(int dim[3])
{
  int dataDim, i;

  vtkDebugMacro(<< " setting SampleDimensions to (" << dim[0] << "," << dim[1] << "," << dim[2] << ")");

  if ( dim[0] != this->SampleDimensions[0] ||
       dim[1] != this->SampleDimensions[1] ||
       dim[2] != this->SampleDimensions[2] )
    {
    if ( dim[0]<1 || dim[1]<1 || dim[2]<1 )
      {
      vtkErrorMacro (<< "Bad Sample Dimensions, retaining previous values");
      return;
      }

    for (dataDim=0, i=0; i<3 ; i++)
      {
      if (dim[i] > 1)
        {
        dataDim++;
        }
      }

    if ( dataDim  < 3 )
      {
      vtkErrorMacro(<<"Sample dimensions must define a volume!");
      return;
      }

    for ( i=0; i<3; i++)
      {
      this->SampleDimensions[i] = dim[i];
      }

    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkImplicitModeller::Cap(vtkDataArray *s)
{
  int i,j,k;
  int idx;
  int d01=this->SampleDimensions[0]*this->SampleDimensions[1];

// i-j planes
  k = 0;
  for (j=0; j<this->SampleDimensions[1]; j++)
    {
    for (i=0; i<this->SampleDimensions[0]; i++)
      {
      s->SetComponent(i+j*this->SampleDimensions[0],0, this->CapValue);
      }
    }
  k = this->SampleDimensions[2] - 1;
  idx = k*d01;
  for (j=0; j<this->SampleDimensions[1]; j++)
    {
    for (i=0; i<this->SampleDimensions[0]; i++)
      {
      s->SetComponent(idx+i+j*this->SampleDimensions[0], 0, this->CapValue);
      }
    }
  // j-k planes
  i = 0;
  for (k=0; k<this->SampleDimensions[2]; k++)
    {
    for (j=0; j<this->SampleDimensions[1]; j++)
      {
      s->SetComponent(j*this->SampleDimensions[0]+k*d01,0,this->CapValue);
      }
    }
  i = this->SampleDimensions[0] - 1;
  for (k=0; k<this->SampleDimensions[2]; k++)
    {
    for (j=0; j<this->SampleDimensions[1]; j++)
      {
      s->SetComponent(i+j*this->SampleDimensions[0]+k*d01,0, this->CapValue);
      }
    }
  // i-k planes
  j = 0;
  for (k=0; k<this->SampleDimensions[2]; k++)
    {
    for (i=0; i<this->SampleDimensions[0]; i++)
      {
      s->SetComponent(i+k*d01,0, this->CapValue);
      }
    }
  j = this->SampleDimensions[1] - 1;
  idx = j*this->SampleDimensions[0];
  for (k=0; k<this->SampleDimensions[2]; k++)
    {
    for (i=0; i<this->SampleDimensions[0]; i++)
      {
      s->SetComponent(idx+i+k*d01,0, this->CapValue);
      }
    }
}

//----------------------------------------------------------------------------
const char *vtkImplicitModeller::GetProcessModeAsString()
{
  if (this->ProcessMode == VTK_CELL_MODE)
    {
    return "PerCell";
    }
  else
    {
    return "PerVoxel";
    }
}

//----------------------------------------------------------------------------
int
vtkImplicitModeller::FillInputPortInformation(int port, vtkInformation* info)
{
  if(!this->Superclass::FillInputPortInformation(port, info))
    {
    return 0;
    }
  info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
  return 1;
}

//----------------------------------------------------------------------------
int vtkImplicitModeller::ProcessRequest(vtkInformation* request,
                                        vtkInformationVector* inputVector,
                                        vtkInformationVector* outputVector)
{
  // should we generate the data?
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA()))
    {
    if (this->GetInput() == NULL)
      {
      // we do not want to release the data because user might
      // have called Append ...
      return 1;
      }
    }
  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}

void vtkImplicitModeller::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Maximum Distance: " << this->MaximumDistance << "\n";
  os << indent << "Sample Dimensions: (" << this->SampleDimensions[0] << ", "
               << this->SampleDimensions[1] << ", "
               << this->SampleDimensions[2] << ")\n";
  os << indent << "ModelBounds: \n";
  os << indent << "  Xmin,Xmax: (" << this->ModelBounds[0] << ", " 
     << this->ModelBounds[1] << ")\n";
  os << indent << "  Ymin,Ymax: (" << this->ModelBounds[2] << ", " 
     << this->ModelBounds[3] << ")\n";
  os << indent << "  Zmin,Zmax: (" << this->ModelBounds[4] << ", " 
     << this->ModelBounds[5] << ")\n";

  os << indent << "AdjustBounds: " << (this->AdjustBounds ? "On\n" : "Off\n");
  os << indent << "Adjust Distance: " << this->AdjustDistance << "\n";
  os << indent << "Process Mode: " << this->ProcessMode << "\n";
  os << indent << "Locator Max Level: " << this->LocatorMaxLevel << "\n";

  os << indent << "Capping: " << (this->Capping ? "On\n" : "Off\n");
  os << indent << "Cap Value: " << this->CapValue << "\n";
  os << indent << "Process Mode: " << this->GetProcessModeAsString() << endl;
  os << indent << "Number Of Threads (for PerVoxel mode): " << this->NumberOfThreads << endl;
}
