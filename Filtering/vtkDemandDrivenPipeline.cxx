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
#include "vtkDemandDrivenPipeline.h"

#include "vtkAlgorithm.h"
#include "vtkAlgorithmOutput.h"
#include "vtkCellData.h"
#include "vtkCommand.h"
#include "vtkDataObject.h"
#include "vtkDataSet.h"
#include "vtkGarbageCollector.h"
#include "vtkInformation.h"
#include "vtkInformationIntegerKey.h"
#include "vtkInformationKeyVectorKey.h"
#include "vtkInformationUnsignedLongKey.h"
#include "vtkInformationVector.h"
#include "vtkInstantiator.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkSmartPointer.h"

#include "vtkImageData.h"
#include "vtkPolyData.h"
#include "vtkRectilinearGrid.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"

#include <vtkstd/vector>

vtkCxxRevisionMacro(vtkDemandDrivenPipeline, "$Revision$");
vtkStandardNewMacro(vtkDemandDrivenPipeline);

vtkInformationKeyMacro(vtkDemandDrivenPipeline, REQUEST_DATA_OBJECT, Integer);
vtkInformationKeyMacro(vtkDemandDrivenPipeline, REQUEST_INFORMATION, Integer);
vtkInformationKeyMacro(vtkDemandDrivenPipeline, REQUEST_DATA, Integer);
vtkInformationKeyMacro(vtkDemandDrivenPipeline, RELEASE_DATA, Integer);
vtkInformationKeyMacro(vtkDemandDrivenPipeline, REQUEST_PIPELINE_MODIFIED_TIME, Integer);
vtkInformationKeyMacro(vtkDemandDrivenPipeline, PIPELINE_MODIFIED_TIME, UnsignedLong);

//----------------------------------------------------------------------------
vtkDemandDrivenPipeline::vtkDemandDrivenPipeline()
{
}

//----------------------------------------------------------------------------
vtkDemandDrivenPipeline::~vtkDemandDrivenPipeline()
{
}

//----------------------------------------------------------------------------
void vtkDemandDrivenPipeline::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "PipelineMTime: " << this->PipelineMTime << "\n";
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::ProcessRequest(vtkInformation* request)
{
  // The algorithm should not invoke anything on the executive.
  if(!this->CheckAlgorithm("ProcessRequest"))
    {
    return 0;
    }

  // Look for specially supported requests.
  if(this->Algorithm && request->Has(REQUEST_PIPELINE_MODIFIED_TIME()))
    {
    // Update inputs first.
    if(!this->ForwardUpstream(request))
      {
      return 0;
      }

    // The pipeline's MTime starts with this algorithm's MTime.
    this->PipelineMTime = this->Algorithm->GetMTime();

    // We want the maximum PipelineMTime of all inputs.
    for(int i=0; i < this->Algorithm->GetNumberOfInputPorts(); ++i)
      {
      for(int j=0; j < this->Algorithm->GetNumberOfInputConnections(i); ++j)
        {
        vtkInformation* info = this->GetInputInformation(i, j);
        unsigned long mtime = info->Get(PIPELINE_MODIFIED_TIME());
        if(mtime > this->PipelineMTime)
          {
          this->PipelineMTime = mtime;
          }
        }
      }

    // Set the pipeline mtime for all outputs.
    for(int j=0; j < this->Algorithm->GetNumberOfOutputPorts(); ++j)
      {
      vtkInformation* info = this->GetOutputInformation(j);
      info->Set(PIPELINE_MODIFIED_TIME(), this->PipelineMTime);
      }

    return 1;
    }

  if(this->Algorithm && request->Has(REQUEST_DATA_OBJECT()))
    {
    // Update inputs first.
    if(!this->ForwardUpstream(request))
      {
      return 0;
      }

    // Make sure our output data type is up-to-date.
    int result = 1;
    if(this->PipelineMTime > this->DataObjectTime.GetMTime())
      {
      // Request data type from the algorithm.
      result = this->ExecuteDataObject();

      // Make sure the data object exists for all output ports.
      for(int i=0;
          result && i < this->Algorithm->GetNumberOfOutputPorts(); ++i)
        {
        vtkInformation* info = this->GetOutputInformation(i);
        if(!info->Get(vtkDataObject::DATA_OBJECT()))
          {
          result = 0;
          }
        }

      if(result)
        {
        // Data object is now up to date.
        this->DataObjectTime.Modified();
        }
      }

    return result;
    }

  if(this->Algorithm && request->Has(REQUEST_INFORMATION()))
    {
    // Update inputs first.
    if(!this->ForwardUpstream(request))
      {
      return 0;
      }

    // Make sure our output information is up-to-date.
    int result = 1;
    if(this->PipelineMTime > this->InformationTime.GetMTime())
      {
      // Make sure input types are valid before algorithm does anything.
      if(!this->InputCountIsValid() || !this->InputTypeIsValid())
        {
        return 0;
        }

      // Request information from the algorithm.
      result = this->ExecuteInformation();

      // Information is now up to date.
      this->InformationTime.Modified();
      }

    return result;
    }

  if(this->Algorithm && request->Has(REQUEST_DATA()))
    {
    // Get the output port from which the request was made.
    int outputPort = -1;
    if(request->Has(FROM_OUTPUT_PORT()))
      {
      outputPort = request->Get(FROM_OUTPUT_PORT());
      }

    // Make sure our outputs are up-to-date.
    int result = 1;
    if(this->NeedToExecuteData(outputPort))
      {
      // Update inputs first.
      if(!this->ForwardUpstream(request))
        {
        return 0;
        }

      // Make sure inputs are valid before algorithm does anything.
      if(!this->InputCountIsValid() || !this->InputTypeIsValid() ||
         !this->InputFieldsAreValid())
        {
        return 0;
        }

      // Request data from the algorithm.
      result = this->ExecuteData(outputPort);

      // Release input data if requested.
      for(int i=0; i < this->Algorithm->GetNumberOfInputPorts(); ++i)
        {
        for(int j=0; j < this->Algorithm->GetNumberOfInputConnections(i); ++j)
          {
          vtkInformation* inInfo = this->GetInputInformation(i, j);
          vtkDataObject* dataObject = inInfo->Get(vtkDataObject::DATA_OBJECT());
          if(dataObject && (dataObject->GetGlobalReleaseDataFlag() ||
                            inInfo->Get(RELEASE_DATA())))
            {
            dataObject->ReleaseData();
            }
          }
        }

      // Data are now up to date.
      this->DataTime.Modified();
      }
    return result;
    }

  // Let the superclass handle other requests.
  return this->Superclass::ProcessRequest(request);
}

//----------------------------------------------------------------------------
void
vtkDemandDrivenPipeline
::CopyDefaultInformation(vtkInformation* request, int direction)
{
  // Let the superclass copy first.
  this->Superclass::CopyDefaultInformation(request, direction);

  if(request->Has(REQUEST_INFORMATION()))
    {
    if(this->GetNumberOfInputPorts() > 0)
      {
      // Copy information from the first input to all outputs.
      if(vtkInformation* inInfo = this->GetInputInformation(0, 0))
        {
        for(int i=0; i < this->Algorithm->GetNumberOfOutputPorts(); ++i)
          {
          vtkInformation* outInfo = this->GetOutputInformation(i);
          outInfo->CopyEntry(inInfo, vtkDataObject::SCALAR_TYPE());
          outInfo->CopyEntry(inInfo, vtkDataObject::SCALAR_NUMBER_OF_COMPONENTS());
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkDemandDrivenPipeline::ResetPipelineInformation(int,
                                                       vtkInformation* info)
{
  info->Remove(RELEASE_DATA());
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::Update()
{
  return this->Superclass::Update();
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::Update(int port)
{
  if(!this->UpdateInformation())
    {
    return 0;
    }
  if(port >= -1 && port < this->Algorithm->GetNumberOfOutputPorts())
    {
    return this->UpdateData(port);
    }
  else
    {
    return 1;
    }
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::UpdatePipelineMTime()
{
  // The algorithm should not invoke anything on the executive.
  if(!this->CheckAlgorithm("UpdatePipelineMTime"))
    {
    return 0;
    }

  // Setup the request for pipeline modification time.
  vtkSmartPointer<vtkInformation> r = vtkSmartPointer<vtkInformation>::New();
  r->Set(REQUEST_PIPELINE_MODIFIED_TIME(), 1);

  // The request is forwarded upstream through the pipeline.
  r->Set(vtkExecutive::FORWARD_DIRECTION(), vtkExecutive::RequestUpstream);

  // Send the request.
  return this->ProcessRequest(r);
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::UpdateDataObject()
{
  // The algorithm should not invoke anything on the executive.
  if(!this->CheckAlgorithm("UpdateDataObject"))
    {
    return 0;
    }

  // Update the pipeline mtime first.
  if(!this->UpdatePipelineMTime())
    {
    return 0;
    }

  // Setup the request for data object creation.
  vtkSmartPointer<vtkInformation> r = vtkSmartPointer<vtkInformation>::New();
  r->Set(REQUEST_DATA_OBJECT(), 1);

  // The request is forwarded upstream through the pipeline.
  r->Set(vtkExecutive::FORWARD_DIRECTION(), vtkExecutive::RequestUpstream);

  // Algorithms process this request after it is forwarded.
  r->Set(vtkExecutive::ALGORITHM_AFTER_FORWARD(), 1);

  // Send the request.
  return this->ProcessRequest(r);
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::UpdateInformation()
{
  // The algorithm should not invoke anything on the executive.
  if(!this->CheckAlgorithm("UpdateInformation"))
    {
    return 0;
    }

  // Do the data-object creation pass before the information pass.
  if(!this->UpdateDataObject())
    {
    return 0;
    }

  // Setup the request for information.
  vtkSmartPointer<vtkInformation> r = vtkSmartPointer<vtkInformation>::New();
  r->Set(REQUEST_INFORMATION(), 1);

  // The request is forwarded upstream through the pipeline.
  r->Set(vtkExecutive::FORWARD_DIRECTION(), vtkExecutive::RequestUpstream);

  // Algorithms process this request after it is forwarded.
  r->Set(vtkExecutive::ALGORITHM_AFTER_FORWARD(), 1);

  // Send the request.
  return this->ProcessRequest(r);
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::UpdateData(int outputPort)
{
  // The algorithm should not invoke anything on the executive.
  if(!this->CheckAlgorithm("UpdateData"))
    {
    return 0;
    }

  // Range check.
  if(outputPort < -1 ||
     outputPort >= this->Algorithm->GetNumberOfOutputPorts())
    {
    vtkErrorMacro("UpdateData given output port index "
                  << outputPort << " on an algorithm with "
                  << this->Algorithm->GetNumberOfOutputPorts()
                  << " output ports.");
    return 0;
    }

  // Setup the request for data.
  vtkSmartPointer<vtkInformation> r = vtkSmartPointer<vtkInformation>::New();
  r->Set(REQUEST_DATA(), 1);
  r->Set(FROM_OUTPUT_PORT(), outputPort);

  // The request is forwarded upstream through the pipeline.
  r->Set(vtkExecutive::FORWARD_DIRECTION(), vtkExecutive::RequestUpstream);

  // Algorithms process this request after it is forwarded.
  r->Set(vtkExecutive::ALGORITHM_AFTER_FORWARD(), 1);

  // Send the request.
  return this->ProcessRequest(r);
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::ExecuteDataObject()
{
  // Invoke the request on the algorithm.
  vtkSmartPointer<vtkInformation> r = vtkSmartPointer<vtkInformation>::New();
  r->Set(REQUEST_DATA_OBJECT(), 1);
  
  // execute the algorithm
  int result = this->CallAlgorithm(r, vtkExecutive::RequestDownstream);

  
  
  // Make sure a valid data object exists for all output ports.
  for(int i=0; result && i < this->Algorithm->GetNumberOfOutputPorts(); ++i)
    {
    result = this->CheckDataObject(i);
    }

  return result;
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::ExecuteInformation()
{
  // Invoke the request on the algorithm.
  vtkSmartPointer<vtkInformation> r = vtkSmartPointer<vtkInformation>::New();
  r->Set(REQUEST_INFORMATION(), 1);
  return this->CallAlgorithm(r, vtkExecutive::RequestDownstream);
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::ExecuteData(int outputPort)
{
  // Tell observers the algorithm is about to execute.
  this->Algorithm->InvokeEvent(vtkCommand::StartEvent,NULL);

  // The algorithm has not yet made any progress.
  this->Algorithm->SetAbortExecute(0);
  this->Algorithm->UpdateProgress(0.0);

  // Invoke the request on the algorithm.
  vtkSmartPointer<vtkInformation> r = vtkSmartPointer<vtkInformation>::New();
  r->Set(REQUEST_DATA(), 1);
  r->Set(FROM_OUTPUT_PORT(), outputPort);
  int result = this->CallAlgorithm(r, vtkExecutive::RequestDownstream);

  // The algorithm has either finished or aborted.
  if(!this->Algorithm->GetAbortExecute())
    {
    this->Algorithm->UpdateProgress(1.0);
    }

  // Tell observers the algorithm is done executing.
  this->Algorithm->InvokeEvent(vtkCommand::EndEvent,NULL);

  return result;
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::CheckDataObject(int port)
{
  // Check that the given output port has a valid data object.
  vtkInformation* outInfo =
    this->GetOutputInformation()->GetInformationObject(port);
  vtkDataObject* data = outInfo->Get(vtkDataObject::DATA_OBJECT());
  vtkInformation* portInfo = this->Algorithm->GetOutputPortInformation(port);
  if(const char* dt = portInfo->Get(vtkDataObject::DATA_TYPE_NAME()))
    {
    // The output port specifies a data type.  Make sure the data
    // object exists and is of the right type.
    if(!data || !data->IsA(dt))
      {
      // Try to create an instance of the correct type.
      data = this->NewDataObject(dt);
      this->SetOutputData(port, data);
      if(data)
        {
        data->Delete();
        }
      }
    if(!data)
      {
      // The algorithm has a bug and did not create the data object.
      vtkErrorMacro("Algorithm " << this->Algorithm->GetClassName() << "("
                    << this->Algorithm
                    << ") did not create output for port " << port
                    << " when asked by REQUEST_DATA_OBJECT and does not"
                    << " specify a concrete DATA_TYPE_NAME.");
      return 0;
      }
    return 1;
    }
  else if(data)
    {
    // The algorithm did not specify its output data type.  Just assume
    // the data object is of the correct type.
    return 1;
    }
  else
    {
    // The algorithm did not specify its output data type and no
    // object exists.
    vtkErrorMacro("Algorithm " << this->Algorithm->GetClassName() << "("
                  << this->Algorithm
                  << ") did not create output for port " << port
                  << " when asked by REQUEST_DATA_OBJECT and does not"
                  << " specify any DATA_TYPE_NAME.");
    return 0;
    }
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::InputCountIsValid()
{
  // Check the number of connections for each port.
  int result = 1;
  for(int p=0; p < this->Algorithm->GetNumberOfInputPorts(); ++p)
    {
    if(!this->InputCountIsValid(p))
      {
      result = 0;
      }
    }
  return result;
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::InputCountIsValid(int port)
{
  // Get the number of connections for this port.
  int connections = this->Algorithm->GetNumberOfInputConnections(port);

  // If the input port is optional, there may be less than one connection.
  if(!this->InputIsOptional(port) && (connections < 1))
    {
    vtkErrorMacro("Input port " << port << " of algorithm "
                  << this->Algorithm->GetClassName()
                  << "(" << this->Algorithm << ") has " << connections
                  << " connections but is not optional.");
    return 0;
    }

  // If the input port is repeatable, there may be more than one connection.
  if(!this->InputIsRepeatable(port) && (connections > 1))
    {
    vtkErrorMacro("Input port " << port << " of algorithm "
                  << this->Algorithm->GetClassName()
                  << "(" << this->Algorithm << ") has " << connections
                  << " connections but is not repeatable.");
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::InputTypeIsValid()
{
  // Check the connection types for each port.
  int result = 1;
  for(int p=0; p < this->Algorithm->GetNumberOfInputPorts(); ++p)
    {
    if(!this->InputTypeIsValid(p))
      {
      result = 0;
      }
    }
  return result;
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::InputTypeIsValid(int port)
{
  // Check the type of each connection on this port.
  int result = 1;
  for(int i=0; i < this->Algorithm->GetNumberOfInputConnections(port); ++i)
    {
    if(!this->InputTypeIsValid(port, i))
      {
      result = 0;
      }
    }
  return result;
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::InputTypeIsValid(int port, int index)
{
  vtkInformation* info = this->Algorithm->GetInputPortInformation(port);
  vtkDataObject* input = this->GetInputData(port, index);

  // Special case for compatibility layer to support NULL inputs.
  if(this->Algorithm->IsA("vtkProcessObject") &&
     input->IsA("vtkProcessObjectDummyData"))
    {
    return 1;
    }

  // Enforce required type, if any.
  if(const char* dt = info->Get(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE()))
    {
    // The input cannot be NULL.
    if(!input)
      {
      vtkErrorMacro("Input for connection index " << index
                    << " on input port index " << port
                    << " for algorithm " << this->Algorithm->GetClassName()
                    << "(" << this->Algorithm << ") is NULL, but a " << dt
                    << " is required.");
      return 0;
      }

    // The input must be of required type.
    if(!input->IsA(dt))
      {
      vtkErrorMacro("Input for connection index " << index
                    << " on input port index " << port
                    << " for algorithm " << this->Algorithm->GetClassName()
                    << "(" << this->Algorithm << ") is of type "
                    << input->GetClassName() << ", but a " << dt
                    << " is required.");
      return 0;
      }
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::InputFieldsAreValid()
{
  // Check the fields for each port.
  int result = 1;
  for(int p=0; p < this->Algorithm->GetNumberOfInputPorts(); ++p)
    {
    if(!this->InputFieldsAreValid(p))
      {
      result = 0;
      }
    }
  return result;
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::InputFieldsAreValid(int port)
{
  // Check the fields for each connection on this port.
  int result = 1;
  for(int i=0; i < this->Algorithm->GetNumberOfInputConnections(port); ++i)
    {
    if(!this->InputFieldsAreValid(port, i))
      {
      result = 0;
      }
    }
  return result;
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::InputFieldsAreValid(int port, int index)
{
  vtkInformation* info = this->Algorithm->GetInputPortInformation(port);
  vtkInformationVector* fields =
    info->Get(vtkAlgorithm::INPUT_REQUIRED_FIELDS());

  // If there are no required fields, there is nothing to check.
  if(!fields)
    {
    return 1;
    }
  vtkDataObject* input = this->GetInputData(port, index);

  // Special case for compatibility layer to support NULL inputs.
  if(this->Algorithm->IsA("vtkProcessObject") &&
     input->IsA("vtkProcessObjectDummyData"))
    {
    return 1;
    }

  // Check availability of each required field.
  int result = 1;
  for(int i=0; i < fields->GetNumberOfInformationObjects(); ++i)
    {
    vtkInformation* field = fields->GetInformationObject(i);

    // Decide which kinds of fields to check.
    int checkPoints = 1;
    int checkCells = 1;
    int checkFields = 1;
    if(field->Has(vtkDataObject::FIELD_ASSOCIATION()))
      {
      switch(field->Get(vtkDataObject::FIELD_ASSOCIATION()))
        {
        case vtkDataObject::FIELD_ASSOCIATION_POINTS:
          checkCells = 0; checkFields = 0; break;
        case vtkDataObject::FIELD_ASSOCIATION_CELLS:
          checkPoints = 0; checkFields = 0; break;
        case vtkDataObject::FIELD_ASSOCIATION_NONE:
          checkPoints = 0; checkCells = 0; break;
        }
      }

    // Point and cell data arrays only exist in vtkDataSet instances.
    vtkDataSet* dataSet = vtkDataSet::SafeDownCast(input);

    // Look for a point data, cell data, or field data array matching
    // the requirements.
    if(!(checkPoints && dataSet && dataSet->GetPointData() &&
         this->DataSetAttributeExists(dataSet->GetPointData(), field)) &&
       !(checkCells && dataSet && dataSet->GetCellData() &&
         this->DataSetAttributeExists(dataSet->GetCellData(), field)) &&
       !(checkFields && input && input->GetFieldData() &&
         this->FieldArrayExists(input->GetFieldData(), field)))
      {
      /* TODO: Construct more descriptive error message from field
         requirements. */
      vtkErrorMacro("Required field not found in input.");
      result = 0;
      }
    }

  return result;
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::DataSetAttributeExists(vtkDataSetAttributes* dsa,
                                                    vtkInformation* field)
{
  if(field->Has(vtkDataObject::FIELD_ATTRIBUTE_TYPE()))
    {
    // A specific attribute must match the requirements.
    int attrType = field->Get(vtkDataObject::FIELD_ATTRIBUTE_TYPE());
    return this->ArrayIsValid(dsa->GetAttribute(attrType), field);
    }
  else
    {
    // Search for an array matching the requirements.
    return this->FieldArrayExists(dsa, field);
    }
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::FieldArrayExists(vtkFieldData* data,
                                              vtkInformation* field)
{
  // Search the field data instance for an array matching the requirements.
  for(int a=0; a < data->GetNumberOfArrays(); ++a)
    {
    if(this->ArrayIsValid(data->GetArray(a), field))
      {
      return 1;
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::ArrayIsValid(vtkDataArray* array,
                                          vtkInformation* field)
{
  // Enforce existence of the array.
  if(!array)
    {
    return 0;
    }

  // Enforce name of the array.  This should really only be used for
  // field data (not point or cell data).
  if(const char* name = field->Get(vtkDataObject::FIELD_NAME()))
    {
    if(!array->GetName() || (strcmp(name, array->GetName()) != 0))
      {
      return 0;
      }
    }

  // Enforce component type for the array.
  if(field->Has(vtkDataObject::FIELD_ARRAY_TYPE()))
    {
    int arrayType = field->Get(vtkDataObject::FIELD_ARRAY_TYPE());
    if(array->GetDataType() != arrayType)
      {
      return 0;
      }
    }

  // Enforce number of components for the array.
  if(field->Has(vtkDataObject::FIELD_NUMBER_OF_COMPONENTS()))
    {
    int arrayNumComponents =
      field->Get(vtkDataObject::FIELD_NUMBER_OF_COMPONENTS());
    if(array->GetNumberOfComponents() != arrayNumComponents)
      {
      return 0;
      }
    }

  // Enforce number of tuples.  This should really only be used for
  // field data (not point or cell data).
  if(field->Has(vtkDataObject::FIELD_NUMBER_OF_TUPLES()))
    {
    int arrayNumTuples = field->Get(vtkDataObject::FIELD_NUMBER_OF_TUPLES());
    if(array->GetNumberOfTuples() != arrayNumTuples)
      {
      return 0;
      }
    }

  return 1;
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::InputIsOptional(int port)
{
  if(vtkInformation* info = this->Algorithm->GetInputPortInformation(port))
    {
    return info->Get(vtkAlgorithm::INPUT_IS_OPTIONAL());
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::InputIsRepeatable(int port)
{
  if(vtkInformation* info = this->Algorithm->GetInputPortInformation(port))
    {
    return info->Get(vtkAlgorithm::INPUT_IS_REPEATABLE());
    }
  return 0;
}

//----------------------------------------------------------------------------
vtkDataObject* vtkDemandDrivenPipeline::NewDataObject(const char* type)
{
  // Check for some standard types and then try the instantiator.
  if(strcmp(type, "vtkImageData") == 0)
    {
    return vtkImageData::New();
    }
  else if(strcmp(type, "vtkPolyData") == 0)
    {
    return vtkPolyData::New();
    }
  else if(strcmp(type, "vtkRectilinearGrid") == 0)
    {
    return vtkRectilinearGrid::New();
    }
  else if(strcmp(type, "vtkStructuredGrid") == 0)
    {
    return vtkStructuredGrid::New();
    }
  else if(strcmp(type, "vtkUnstructuredGrid") == 0)
    {
    return vtkUnstructuredGrid::New();
    }
  else if(vtkObject* obj = vtkInstantiator::CreateInstance(type))
    {
    vtkDataObject* data = vtkDataObject::SafeDownCast(obj);
    if(!data)
      {
      obj->Delete();
      }
    return data;
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::NeedToExecuteData(int outputPort)
{
  // If the data are out of date, we need to execute.
  if(this->PipelineMTime > this->DataTime.GetMTime())
    {
    return 1;
    }

  // If no port is specified, check all ports.  Subclass
  // implementations might use the port number.
  if(outputPort < 0)
    {
    for(int i=0; i < this->Algorithm->GetNumberOfOutputPorts(); ++i)
      {
      if(this->NeedToExecuteData(i))
        {
        return 1;
        }
      }
    }

  // We do not need to execute.
  return 0;
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::SetReleaseDataFlag(int port, int n)
{
  if(!this->OutputPortIndexInRange(port, "set release data flag on"))
    {
    return 0;
    }
  vtkInformation* info = this->GetOutputInformation(port);
  if(this->GetReleaseDataFlag(port) != n)
    {
    info->Set(RELEASE_DATA(), n);
    return 1;
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::GetReleaseDataFlag(int port)
{
  if(!this->OutputPortIndexInRange(port, "get release data flag from"))
    {
    return 0;
    }
  vtkInformation* info = this->GetOutputInformation(port);
  if(!info->Has(RELEASE_DATA()))
    {
    info->Set(RELEASE_DATA(), 0);
    }
  return info->Get(RELEASE_DATA());
}
