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
#include "vtkCompositeDataPipeline.h"

#include "vtkAlgorithm.h"
#include "vtkAlgorithmOutput.h"
#include "vtkHierarchicalDataInformation.h"
#include "vtkHierarchicalDataSet.h"
#include "vtkInformation.h"
#include "vtkInformationDoubleKey.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationIntegerVectorKey.h"
#include "vtkInformationKey.h"
#include "vtkInformationObjectBaseKey.h"
#include "vtkInformationStringKey.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"

#include "vtkImageData.h"
#include "vtkRectilinearGrid.h"
#include "vtkStructuredGrid.h"
#include "vtkUniformGrid.h"

vtkCxxRevisionMacro(vtkCompositeDataPipeline, "$Revision$");
vtkStandardNewMacro(vtkCompositeDataPipeline);

vtkInformationKeyMacro(vtkCompositeDataPipeline,BEGIN_LOOP,Integer);
vtkInformationKeyMacro(vtkCompositeDataPipeline,END_LOOP,Integer);
vtkInformationKeyMacro(vtkCompositeDataPipeline,COMPOSITE_DATA_TYPE_NAME,String);
vtkInformationKeyMacro(vtkCompositeDataPipeline,COMPOSITE_DATA_INFORMATION,ObjectBase);
vtkInformationKeyMacro(vtkCompositeDataPipeline,UPDATE_COST,Double);
vtkInformationKeyMacro(vtkCompositeDataPipeline,MARKED_FOR_UPDATE,Integer);
vtkInformationKeyMacro(vtkCompositeDataPipeline,INPUT_REQUIRED_COMPOSITE_DATA_TYPE, String);

//----------------------------------------------------------------------------
vtkCompositeDataPipeline::vtkCompositeDataPipeline()
{
  this->InSubPass = 0;
}

//----------------------------------------------------------------------------
vtkCompositeDataPipeline::~vtkCompositeDataPipeline()
{
}

//----------------------------------------------------------------------------
int vtkCompositeDataPipeline::ProcessRequest(vtkInformation* request)
{

  if(this->Algorithm && request->Has(BEGIN_LOOP()))
    {
    this->InSubPass = 1;
    return 1;
    }

  if(this->Algorithm && request->Has(END_LOOP()))
    {
    this->InSubPass = 0;
    return 1;
    }

  if(this->Algorithm && request->Has(REQUEST_PIPELINE_MODIFIED_TIME()))
    {
    if (this->InSubPass)
      {
      // Update the time only if it is the beginning of the next
      // pass in the loop.
      if (request->Has(vtkCompositeDataSet::INDEX()))
        {
        this->SubPassTime.Modified();
        }
      // Set the pipeline mtime for all outputs.
      for(int j=0; j < this->Algorithm->GetNumberOfOutputPorts(); ++j)
        {
        vtkInformation* info = this->GetOutputInformation(j);
        info->Set(PIPELINE_MODIFIED_TIME(), this->SubPassTime);
        }
      return 1;
      }
    }

  if(this->Algorithm && request->Has(REQUEST_DATA_OBJECT()))
    {
    if (this->InSubPass)
      {
      int result = 1;
      if (this->SubPassTime > this->DataObjectTime.GetMTime())
        {
        // Request information from the algorithm.
        result = this->ExecuteDataObjectForBlock(request);
        }
      return result;
      }
    }

  if(this->Algorithm && request->Has(REQUEST_INFORMATION()))
    {
    // Make sure our output information is up-to-date.
    int result = 1;
    if (this->InSubPass)
      {
      if(this->SubPassTime > this->InformationTime.GetMTime())
        {
        result = this->ExecuteInformationForBlock(request);
        
        // Information is now up to date.
        this->InformationTime.Modified();
        }
      }
    else
      {
      int appendKey = 1;
      vtkInformationKey** keys = request->Get(vtkExecutive::KEYS_TO_COPY());
      if (keys)
        {
        int len = request->Length(vtkExecutive::KEYS_TO_COPY());
        for (int i=0; i<len; i++)
          {
          if (keys[i] == vtkCompositeDataPipeline::COMPOSITE_DATA_INFORMATION())
            {
            appendKey = 0;
            break;
            }
          }
        }
      if (appendKey)
        {
        request->Append(vtkExecutive::KEYS_TO_COPY(), 
                        vtkCompositeDataPipeline::COMPOSITE_DATA_INFORMATION());
        }
      
      result = this->Superclass::ProcessRequest(request);
      }
    return result;
    }

  if(request->Has(REQUEST_UPDATE_EXTENT()))
    {
    if (this->InSubPass)
      {
      return 1;
      }
    }

  if(this->Algorithm && request->Has(REQUEST_DATA()))
    {
    // Get the output port from which the request was made.
    int outputPort = -1;
    if(request->Has(FROM_OUTPUT_PORT()))
      {
      outputPort = request->Get(FROM_OUTPUT_PORT());
      }

    if(this->NeedToExecuteData(outputPort))
      {
      // We have to check whether an input is marked for update before
      // ExecuteData() is entered. When looping over blocks, the composite 
      // data producer up the pipeline will trick the intermediate simple
      // filters to execute by sending an update mtime. However, if none
      // of those filters are modified, this would cause unnecessary
      // execution of the whole intermediate pipeline. Here, NeedToExecuteData()
      // is called and the result cached so that the blocks that are up to date
      // can be skipped later.
 
      // Loop over all input ports.
      for(int i=0; i < this->Algorithm->GetNumberOfInputPorts(); ++i)
        {
        // Loop over all connections on this input port.
        int numInConnections = this->Algorithm->GetNumberOfInputConnections(i);
        for (int j=0; j<numInConnections; j++)
          {
          vtkInformation* inInfo = this->GetInputInformation(i, j);
          vtkDemandDrivenPipeline* ddp = 
            vtkDemandDrivenPipeline::SafeDownCast(
              inInfo->GetExecutive(vtkExecutive::PRODUCER()));
          inInfo->Remove(MARKED_FOR_UPDATE());
          if (ddp)
            {
            if (ddp->NeedToExecuteData(-1))
              {
              inInfo->Set(MARKED_FOR_UPDATE(), 1);
              }
            }
          }
        }
      }

    if (this->InSubPass)
      {
      return this->ExecuteDataForBlock(request);
      }
    }

  // Let the superclass handle other requests.
  return this->Superclass::ProcessRequest(request);
}

//----------------------------------------------------------------------------
// Handle REQUEST_DATA_OBJECT
int vtkCompositeDataPipeline::ExecuteDataObject(vtkInformation* request)
{
  int result = this->ExecuteDataObjectForBlock(request);
  
  if (!result)
    {
    return result;
    }

  return this->Superclass::ExecuteDataObject(request);
}

//----------------------------------------------------------------------------
int vtkCompositeDataPipeline::ExecuteDataObjectForBlock(vtkInformation* request)
{
  int result = 1;

  vtkInformationVector* outputVector = this->GetOutputInformation();
  int numOut = outputVector->GetNumberOfInformationObjects();
  for (int i=0; i<numOut; i++)
    {
    vtkInformation* portInfo = 
      this->Algorithm->GetOutputPortInformation(0);
    if (portInfo->Has(COMPOSITE_DATA_TYPE_NAME()))
      {
      vtkInformation* info = outputVector->GetInformationObject(i);
    
      vtkDataObject* doOutput = 
        info->Get(vtkCompositeDataSet::COMPOSITE_DATA_SET());
      vtkCompositeDataSet* output = vtkCompositeDataSet::SafeDownCast(doOutput);

      vtkDataObject* simpleOutput = 0;

      if (output)
        {
        vtkDataObject* dobj;

        // If the index is specified, use it. Otherwise, use (0,0)
        if (request->Has(vtkCompositeDataSet::INDEX()))
          {
          dobj = output->GetDataSet(request);
          }
        else
          {
          vtkSmartPointer<vtkInformation> r = 
            vtkSmartPointer<vtkInformation>::New();
          r->Set(vtkHierarchicalDataSet::LEVEL(), 0);
          r->Set(vtkCompositeDataSet::INDEX(),    0);
          dobj = output->GetDataSet(r);
          }

        if (dobj)
          {
          this->DataObjectTime.Modified();

          vtkDataObject* currDObj = info->Get(vtkDataObject::DATA_OBJECT());

          if (currDObj && 
              strcmp(currDObj->GetClassName(), dobj->GetClassName()) == 0)
            {
            continue;
            }
          
          simpleOutput = dobj->NewInstance();
          if (!simpleOutput)
            {
            vtkErrorMacro("Could not create a copy of " << dobj->GetClassName()
                          << ".");
            return 0;
            }
          }
        }

      if (!simpleOutput)
        {
        // If there was no current data object in the composite data object
        // and there is already any data object in the output, move to the
        // next port
        if(info->Get(vtkDataObject::DATA_OBJECT()))
          {
          continue;
          }
        // Otherwise, simply create a polydata so that filters that
        // must have a data object (such as datasettodataset filters)
        // have a dummy dataset
        simpleOutput = vtkPolyData::New();
        }
      
      simpleOutput->SetPipelineInformation(info);
      simpleOutput->Delete();
      }
    }

  return result;
}

//----------------------------------------------------------------------------
// Handle REQUEST_INFORMATION
int vtkCompositeDataPipeline::ExecuteInformation(vtkInformation* request)
{
  if(this->PipelineMTime > this->InformationTime.GetMTime())
    {
    // Make sure a valid data object exists for all output ports.
    for(int i=0; i < this->Algorithm->GetNumberOfOutputPorts(); ++i)
      {
      if (!this->CheckCompositeData(i))
        {
        return 0;
        }
      }
    }

  return this->Superclass::ExecuteInformation(request);
}

//----------------------------------------------------------------------------
int vtkCompositeDataPipeline::ExecuteInformationForBlock(vtkInformation* request)
{
  vtkInformationVector* outputVector = this->GetOutputInformation();
  int numOut = outputVector->GetNumberOfInformationObjects();
  for (int i=0; i<numOut; i++)
    {
    vtkInformation* info = outputVector->GetInformationObject(i);
    
    vtkDataObject* doOutput = 
      info->Get(vtkCompositeDataSet::COMPOSITE_DATA_SET());
    vtkCompositeDataSet* output = vtkCompositeDataSet::SafeDownCast(doOutput);

    if (output)
      {
      vtkDataObject* dobj = output->GetDataSet(request);
      vtkDataObject* dobjCopy = 
        info->Get(vtkDataObject::DATA_OBJECT());
      if (dobj && dobjCopy)
        {
        dobjCopy->ShallowCopy(dobj);
        }
      }
    }

  return 1;
}


//----------------------------------------------------------------------------
// Handle REQUEST_DATA
int vtkCompositeDataPipeline::ExecuteData(vtkInformation* request)
{
  int result = 1;

  int outputPort = request->Get(FROM_OUTPUT_PORT());

  int i, j;

  // Loop over all input ports.
  for(i=0; i < this->Algorithm->GetNumberOfInputPorts(); ++i)
    {
    // Loop over all connections on this input port.
    int numInConnections = this->Algorithm->GetNumberOfInputConnections(i);
    for (j=0; j<numInConnections; j++)
      {
      vtkInformation* inInfo = this->GetInputInformation(i, j);

      vtkHierarchicalDataInformation* dataInf = 
        vtkHierarchicalDataInformation::SafeDownCast(
          inInfo->Get(vtkCompositeDataPipeline::COMPOSITE_DATA_INFORMATION()));

      if (!dataInf)
        {
        continue;
        }

      vtkCompositeDataSet* input = vtkCompositeDataSet::SafeDownCast(
        inInfo->Get(vtkCompositeDataSet::COMPOSITE_DATA_SET()));

      // There is a composite input, no need to loop
      if (input)
        {
        continue;
        }

      // If the input is up-to-date (not MARKED_FOR_UPDATE() by ProcessRequest ->
      // REQUEST_DATA()), there is no need to loop the input. If the input is
      // looped, it will cause unnecessary execution(s)
      if (!inInfo->Has(MARKED_FOR_UPDATE()))
        {
        continue;
        }

      // If the input requires a composite dataset and one is not available,
      // create it. This means that there are "simple" filters between the
      // composite data producer and this filter
      if (!input)
        {
        vtkInformation* inPortInfo = 
          this->Algorithm->GetInputPortInformation(i);
        const char* dt = 
          inPortInfo->Get(INPUT_REQUIRED_COMPOSITE_DATA_TYPE());
        if (dt)
          {
          if (strcmp(dt, "vtkCompositeDataSet") == 0)
            {
            // If vtkCompositeDataSet is specified, the algorithm
            // will work with all sub-classes. Create a vtkHierarchicalDataSet
            dt = "vtkHierarchicalDataSet";
            }
          // If the composite data input to the algorithm is not
          // set, create and assign it. This happens when the producer
          // of the input data actually produces a simple data object
          // (in a loop)
          vtkDataObject* dobj = this->NewDataObject(dt);
          if (dobj)
            {
            dobj->SetPipelineInformation(inInfo);
            input = vtkCompositeDataSet::SafeDownCast(dobj);
            dobj->Delete();
            }
          else
            {
            vtkErrorMacro("Cannot instantiate " << dt
                          << ". The INPUT_REQUIRED_COMPOSITE_DATA_TYPE() of "
                          << this->Algorithm->GetClassName()
                          << " is not set properly.");
            
            }
          }
        }
            
      // Tell the producer upstream that looping is about to start
      vtkSmartPointer<vtkInformation> rb = 
        vtkSmartPointer<vtkInformation>::New();
      rb->Set(BEGIN_LOOP(), 1);
        
      // The request is forwarded upstream through the pipeline.
      rb->Set(vtkExecutive::FORWARD_DIRECTION(), 
              vtkExecutive::RequestUpstream);
        
      // Send the request.
      if (!this->ForwardUpstream(i, j, rb))
        {
        return 0;
        }

      // Execute the streaming demand driven pipeline for each block
      unsigned int numLevels = dataInf->GetNumberOfLevels();
      for (unsigned int k=0; k<numLevels; k++)
        {
        unsigned int numDataSets = dataInf->GetNumberOfDataSets(k);
        for (unsigned l=0; l<numDataSets; l++)
          {
          if (dataInf)
            {
            vtkInformation* partInf = dataInf->GetInformation(k, l);
            if (!partInf->Has(MARKED_FOR_UPDATE()))
              {
              cout << k << "," << l << "  not marked for update" << endl;
              continue;
              }
            }
          // First pipeline mtime
              
          // Setup the request for pipeline modification time.
          vtkSmartPointer<vtkInformation> r1 = 
            vtkSmartPointer<vtkInformation>::New();
          r1->Set(REQUEST_PIPELINE_MODIFIED_TIME(), 1);
              
          r1->Set(vtkHierarchicalDataSet::LEVEL(), k);
          r1->Set(vtkCompositeDataSet::INDEX(), l);
              
          // The request is forwarded upstream through the pipeline.
          r1->Set(vtkExecutive::FORWARD_DIRECTION(), 
                  vtkExecutive::RequestUpstream);
              
          // Send the request.
          if (!this->ForwardUpstream(i, j, r1))
            {
            return 0;
            }
            
          // Do the data-object creation pass before the information pass.
              
          // Setup the request for data object creation.
          vtkSmartPointer<vtkInformation> r1_5 = 
            vtkSmartPointer<vtkInformation>::New();
          r1_5->Set(REQUEST_DATA_OBJECT(), 1);
              
          // The request is forwarded upstream through the pipeline.
          r1_5->Set(vtkExecutive::FORWARD_DIRECTION(), 
                    vtkExecutive::RequestUpstream);
              
          // Algorithms process this request after it is forwarded.
          r1_5->Set(vtkExecutive::ALGORITHM_AFTER_FORWARD(), 1);
              
          r1_5->Set(vtkHierarchicalDataSet::LEVEL(), k);
          r1_5->Set(vtkCompositeDataSet::INDEX(), l);
            
          // Send the request.
          if (!this->ForwardUpstream(i, j, r1_5))
            {
            return 0;
            }
              
          // Setup the request for information.
          vtkSmartPointer<vtkInformation> r2 = 
            vtkSmartPointer<vtkInformation>::New();
          r2->Set(REQUEST_INFORMATION(), 1);
              
          r2->Set(vtkHierarchicalDataSet::LEVEL(), k);
          r2->Set(vtkCompositeDataSet::INDEX(), l);
            
          // The request is forwarded upstream through the pipeline.
          r2->Set(
            vtkExecutive::FORWARD_DIRECTION(), vtkExecutive::RequestUpstream);
              
          // Algorithms process this request after it is forwarded.
          r2->Set(vtkExecutive::ALGORITHM_AFTER_FORWARD(), 1);
              
          // Send the request.
          if (!this->ForwardUpstream(i, j, r2))
            {
            return 0;
            }
            
          // Update the whole thing
          // TODO: This might change
          if (inInfo->Has(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()))
            {
            int extent[6] = {0,-1,0,-1,0,-1};
            inInfo->Get(
              vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),extent);
            inInfo->Set(
              vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), extent, 6);
            inInfo->Set(
              vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT_INITIALIZED(), 1);
            inInfo->Set(
              vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES(), 1);
            inInfo->Set(
              vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER(), 0);
            }
            
          // Setup the request for update extent propagation.
          vtkSmartPointer<vtkInformation> r3 = 
            vtkSmartPointer<vtkInformation>::New();
          r3->Set(REQUEST_UPDATE_EXTENT(), 1);
          r3->Set(FROM_OUTPUT_PORT(), outputPort);
            
          r3->Set(vtkHierarchicalDataSet::LEVEL(), k);
          r3->Set(vtkCompositeDataSet::INDEX(), l);
            
          // The request is forwarded upstream through the pipeline.
          r3->Set(
            vtkExecutive::FORWARD_DIRECTION(), vtkExecutive::RequestUpstream);
              
          // Algorithms process this request before it is forwarded.
          r3->Set(vtkExecutive::ALGORITHM_BEFORE_FORWARD(), 1);
              
          // Send the request.
          if (!this->ForwardUpstream(i, j, r3))
            {
            return 0;
            }
            
          // Setup the request for data.
          vtkSmartPointer<vtkInformation> r4 = 
            vtkSmartPointer<vtkInformation>::New();
          r4->Set(REQUEST_DATA(), 1);
          r4->Set(FROM_OUTPUT_PORT(), outputPort);
              
          r4->Set(vtkHierarchicalDataSet::LEVEL(), k);
          r4->Set(vtkCompositeDataSet::INDEX(), l);
            
          // The request is forwarded upstream through the pipeline.
          r4->Set(
            vtkExecutive::FORWARD_DIRECTION(), vtkExecutive::RequestUpstream);
              
          // Algorithms process this request after it is forwarded.
          r4->Set(vtkExecutive::ALGORITHM_AFTER_FORWARD(), 1);
              
          // Send the request.
          if (!this->ForwardUpstream(i, j, r4))
            {
            return 0;
            }
              
          vtkDataObject* block = inInfo->Get(vtkDataObject::DATA_OBJECT());
          if (block && input)
            {
            vtkDataObject* blockCopy = block->NewInstance();
            blockCopy->ShallowCopy(block);
            input->AddDataSet(r4, blockCopy);
            blockCopy->Delete();
            }
          }
        }

      // Tell the producer upstream that looping is over
      vtkSmartPointer<vtkInformation> re = 
        vtkSmartPointer<vtkInformation>::New();
      re->Set(END_LOOP(), 1);
      
      // The request is forwarded upstream through the pipeline.
      re->Set(vtkExecutive::FORWARD_DIRECTION(), 
              vtkExecutive::RequestUpstream);
      
      // Send the request.
      if (!this->ForwardUpstream(i, j, re))
        {
        return 0;
        }
      }
    }

    

  if (result)
    {
    int inputPortComposite = 0;
    int inputComposite = 0;
    int compositePort = -1;
    // Loop over all input ports.
    for(i=0; i < this->Algorithm->GetNumberOfInputPorts(); ++i)
      {
      vtkInformation* inPortInfo = 
        this->Algorithm->GetInputPortInformation(i);
      if (inPortInfo->Has(INPUT_REQUIRED_COMPOSITE_DATA_TYPE()))
        {
        inputPortComposite = 1;
        }
      int numInConnections = this->Algorithm->GetNumberOfInputConnections(i);
      if (numInConnections > 0)
        {
        vtkInformation* inInfo = this->GetInputInformation(i, 0);
        if (inInfo->Has(vtkCompositeDataSet::COMPOSITE_DATA_SET()))
          {
          inputComposite = 1;
          compositePort = i;
          }
        }
      }

    if (inputComposite && !inputPortComposite)
      {
      this->ExecuteDataStart(request);

      vtkCompositeDataSet* output = 0;
      vtkInformation* outInfo = 0;
      vtkDataObject* prevOutput = 0;
      if (this->GetNumberOfOutputPorts() > 0)
        {
        outInfo = this->GetOutputInformation(0);

        // This assumes that the filter has one output
        vtkDataObject* doOutput = 
          outInfo->Get(vtkCompositeDataSet::COMPOSITE_DATA_SET());
        output = vtkCompositeDataSet::SafeDownCast(doOutput);
        if (!output)
          {
          output = vtkHierarchicalDataSet::New();
          output->SetPipelineInformation(outInfo);
          output->Delete();
          }

        prevOutput = outInfo->Get(vtkDataObject::DATA_OBJECT());
        }

      // Loop using the first input on the first port.
      // This might not be valid for all cases but it is a decent
      // assumption to start with.
      vtkInformation* inInfo = this->GetInputInformation(compositePort, 0);
      vtkCompositeDataSet* input = vtkCompositeDataSet::SafeDownCast(
        inInfo->Get(vtkCompositeDataSet::COMPOSITE_DATA_SET()));

      vtkHierarchicalDataInformation* dataInf = 
        vtkHierarchicalDataInformation::SafeDownCast(
          inInfo->Get(vtkCompositeDataPipeline::COMPOSITE_DATA_INFORMATION()));

      if (input && dataInf)
        {
        vtkSmartPointer<vtkInformation> r = 
          vtkSmartPointer<vtkInformation>::New();

        r->Set(FROM_OUTPUT_PORT(), inInfo->GetPort(PRODUCER()));

        // The request is forwarded upstream through the pipeline.
        r->Set(vtkExecutive::FORWARD_DIRECTION(), vtkExecutive::RequestUpstream);
        
        // Algorithms process this request after it is forwarded.
        r->Set(vtkExecutive::ALGORITHM_AFTER_FORWARD(), 1);

        unsigned int numLevels = dataInf->GetNumberOfLevels();
        vtkDataObject* prevInput = inInfo->Get(vtkDataObject::DATA_OBJECT());
        prevInput->Register(this);

        cout << "EXECUTING: " << this->Algorithm->GetClassName() << endl;
        for (unsigned int k=0; k<numLevels; k++)
          {
          unsigned int numDataSets = dataInf->GetNumberOfDataSets(k);
          for (unsigned l=0; l<numDataSets; l++)
            {
            vtkInformation* partInf = dataInf->GetInformation(k, l);
            if (partInf->Has(MARKED_FOR_UPDATE()))
              {
              r->Set(vtkHierarchicalDataSet::LEVEL(), k);
              r->Set(vtkCompositeDataSet::INDEX(),    l);
              vtkDataObject* dobj = input->GetDataSet(r);
              // There must be a bug somehwere. If this Remove()
              // is not called, the following Set() has the effect
              // of removing (!) the key.
              inInfo->Remove(vtkDataObject::DATA_OBJECT());
              inInfo->Set(vtkDataObject::DATA_OBJECT(), dobj);

              this->CopyFromDataToInformation(dobj, inInfo);

              r->Set(REQUEST_DATA_OBJECT(), 1);
              this->Superclass::ExecuteDataObject(r);
              r->Remove(REQUEST_DATA_OBJECT());

              r->Set(REQUEST_INFORMATION(), 1);
              this->Superclass::ExecuteInformation(r);
              r->Remove(REQUEST_INFORMATION());

              r->Set(REQUEST_DATA(), 1);
              this->Superclass::ExecuteData(r);
              r->Remove(REQUEST_DATA());
              if (output && outInfo)
                {
                vtkDataObject* tmpOutput = 
                  outInfo->Get(vtkDataObject::DATA_OBJECT());
                vtkDataObject* outputCopy = tmpOutput->NewInstance();
                outputCopy->ShallowCopy(tmpOutput);
                output->AddDataSet(r, outputCopy);
                outputCopy->Delete();
                }
              }
            }
          }
        vtkDataObject* curInput = inInfo->Get(vtkDataObject::DATA_OBJECT());
        if (curInput != prevInput)
          {
          inInfo->Remove(vtkDataObject::DATA_OBJECT());
          inInfo->Set(vtkDataObject::DATA_OBJECT(), prevInput);
          }
        if (prevInput)
          {
          prevInput->UnRegister(this);
          }
        vtkDataObject* curOutput = outInfo->Get(vtkDataObject::DATA_OBJECT());
        if (curOutput != prevOutput)
           {
           prevOutput->SetPipelineInformation(outInfo);
           }
        }
      this->ExecuteDataEnd(request);
      }
    else
      {
      result = this->Superclass::ExecuteData(request);
      }
    }

  return result;
}

//----------------------------------------------------------------------------
int vtkCompositeDataPipeline::ExecuteDataForBlock(vtkInformation* request)
{
  vtkInformationVector* outputVector = this->GetOutputInformation();
  int numOut = outputVector->GetNumberOfInformationObjects();
  for (int i=0; i<numOut; i++)
    {
    vtkInformation* info = outputVector->GetInformationObject(i);
    
    vtkDataObject* doOutput = 
      info->Get(vtkCompositeDataSet::COMPOSITE_DATA_SET());
    vtkCompositeDataSet* output = vtkCompositeDataSet::SafeDownCast(doOutput);
    
    if (output)
      {
      vtkDataObject* dobj = output->GetDataSet(request);
      if (dobj)
        {
        vtkDataObject* dobjCopy = 
          info->Get(vtkDataObject::DATA_OBJECT());
        
        if (dobj && dobjCopy)
          {
          dobjCopy->ShallowCopy(dobj);
          }
        }
      }
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkCompositeDataPipeline::ForwardUpstream(vtkInformation* request)
{
  return this->Superclass::ForwardUpstream(request);
}

//----------------------------------------------------------------------------
int vtkCompositeDataPipeline::ForwardUpstream(
  int i, int j, vtkInformation* request)
{
  int result = 1;
  if(vtkExecutive* e = this->GetInputExecutive(i, j))
    {
    vtkAlgorithmOutput* input = this->Algorithm->GetInputConnection(i, j);
    int port = request->Get(FROM_OUTPUT_PORT());
    request->Set(FROM_OUTPUT_PORT(), input->GetIndex());
    if(!e->ProcessRequest(request))
      {
      result = 0;
      }
    request->Set(FROM_OUTPUT_PORT(), port);
    }
  return result;
}

//----------------------------------------------------------------------------
void vtkCompositeDataPipeline::CopyFromDataToInformation(
  vtkDataObject* dobj, vtkInformation* inInfo)
{
  if (dobj->IsA("vtkImageData"))
    {
    inInfo->Set(
      WHOLE_EXTENT(), static_cast<vtkImageData*>(dobj)->GetExtent(), 6);
    }
  else if (dobj->IsA("vtkStructuredGrid"))
    {
    inInfo->Set(
      WHOLE_EXTENT(), static_cast<vtkStructuredGrid*>(dobj)->GetExtent(), 6);
    }
  else if (dobj->IsA("vtkRectilinearGrid"))
    {
    inInfo->Set(
      WHOLE_EXTENT(), static_cast<vtkRectilinearGrid*>(dobj)->GetExtent(), 6);
    }
  else if (dobj->IsA("vtkUniformGrid"))
    {
    inInfo->Set(
      WHOLE_EXTENT(), static_cast<vtkUniformGrid*>(dobj)->GetExtent(), 6);
    }
  else
    {
    inInfo->Set(MAXIMUM_NUMBER_OF_PIECES(), 1);
    }
}

//----------------------------------------------------------------------------
int vtkCompositeDataPipeline::CheckCompositeData(int port)
{
  // Check that the given output port has a valid data object.
  vtkInformation* outInfo =
    this->GetOutputInformation()->GetInformationObject(port);
  vtkDataObject* data = 
    outInfo->Get(vtkCompositeDataSet::COMPOSITE_DATA_SET());
  vtkInformation* portInfo = this->Algorithm->GetOutputPortInformation(port);
  if (const char* dt = portInfo->Get(COMPOSITE_DATA_TYPE_NAME()))
    {
    if(!data || !data->IsA(dt))
      {
      // Try to create an instance of the correct type.
      data = this->NewDataObject(dt);
      data->SetPipelineInformation(outInfo);
      if(data)
        {
        data->Delete();
        }
      }
    }

  return 1;
}


//----------------------------------------------------------------------------
vtkDataObject* vtkCompositeDataPipeline::GetCompositeOutputData(int port)
{
  if(!this->OutputPortIndexInRange(port, "get data for"))
    {
    return 0;
    }

  this->CheckCompositeData(port);

  // Return the data object.
  if(vtkInformation* info = this->GetOutputInformation(port))
    {
    return info->Get(vtkCompositeDataSet::COMPOSITE_DATA_SET());
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkCompositeDataPipeline::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

