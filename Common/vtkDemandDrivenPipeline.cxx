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

//----------------------------------------------------------------------------
class vtkDemandDrivenPipelineInternals
{
public:
  vtkSmartPointer<vtkInformationVector> OutputInformation;
  vtkSmartPointer<vtkInformationVector> InputInformation;
  vtkSmartPointer<vtkInformation> RequestInformation;

  vtkDemandDrivenPipelineInternals()
    {
    this->OutputInformation = vtkSmartPointer<vtkInformationVector>::New();
    this->InputInformation = vtkSmartPointer<vtkInformationVector>::New();
    }
};

//----------------------------------------------------------------------------
vtkDemandDrivenPipeline::vtkDemandDrivenPipeline()
{
  this->DemandDrivenInternal = new vtkDemandDrivenPipelineInternals;
  this->InProcessDownstreamRequest = 0;
  this->InProcessUpstreamRequest = 0;
}

//----------------------------------------------------------------------------
vtkDemandDrivenPipeline::~vtkDemandDrivenPipeline()
{
  delete this->DemandDrivenInternal;
}

//----------------------------------------------------------------------------
void vtkDemandDrivenPipeline::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkInformationIntegerKey* vtkDemandDrivenPipeline::REQUEST_INFORMATION()
{
  static vtkInformationIntegerKey instance("REQUEST_INFORMATION",
                                           "vtkDemandDrivenPipeline");
  return &instance;
}

//----------------------------------------------------------------------------
vtkInformationIntegerKey* vtkDemandDrivenPipeline::REQUEST_DATA()
{
  static vtkInformationIntegerKey instance("REQUEST_DATA",
                                           "vtkDemandDrivenPipeline");
  return &instance;
}

//----------------------------------------------------------------------------
vtkInformationIntegerKey* vtkDemandDrivenPipeline::FROM_OUTPUT_PORT()
{
  static vtkInformationIntegerKey instance("FROM_OUTPUT_PORT",
                                           "vtkDemandDrivenPipeline");
  return &instance;
}

//----------------------------------------------------------------------------
vtkDemandDrivenPipeline*
vtkDemandDrivenPipeline::GetConnectedInputExecutive(int port, int index)
{
  if(vtkAlgorithmOutput* input =
     this->Algorithm->GetInputConnection(port, index))
    {
    if(vtkDemandDrivenPipeline* executive =
       vtkDemandDrivenPipeline::SafeDownCast(
         input->GetProducer()->GetExecutive()))
      {
      return executive;
      }
    else
      {
      vtkErrorMacro("Algorithm "
                    << input->GetProducer()->GetClassName()
                    << "(" << input->GetProducer()
                    << ") producing input for connection index " << index
                    << " on port index " << port
                    << " to algorithm "
                    << this->Algorithm->GetClassName() << "("
                    << this->Algorithm << ") is not managed by a"
                    << " vtkDemandDrivenPipeline.");
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
vtkInformation*
vtkDemandDrivenPipeline::GetConnectedInputInformation(int port, int index)
{
  if(vtkDemandDrivenPipeline* executive =
     this->GetConnectedInputExecutive(port, index))
    {
    vtkAlgorithmOutput* input =
      this->Algorithm->GetInputConnection(port, index);
    return executive->GetOutputInformation(input->GetIndex());
    }
  return 0;
}

//----------------------------------------------------------------------------
vtkInformation* vtkDemandDrivenPipeline::GetRequestInformation()
{
  if(!this->DemandDrivenInternal->RequestInformation)
    {
    this->DemandDrivenInternal->RequestInformation =
      vtkSmartPointer<vtkInformation>::New();
    }
  return this->DemandDrivenInternal->RequestInformation.GetPointer();
}

//----------------------------------------------------------------------------
vtkInformationVector* vtkDemandDrivenPipeline::GetInputInformation()
{
  if(!this->DemandDrivenInternal->InputInformation)
    {
    this->DemandDrivenInternal->InputInformation =
      vtkSmartPointer<vtkInformationVector>::New();
    }
  this->DemandDrivenInternal->InputInformation
    ->SetNumberOfInformationObjects(this->Algorithm->GetNumberOfInputPorts());
  return this->DemandDrivenInternal->InputInformation.GetPointer();
}

//----------------------------------------------------------------------------
vtkInformation* vtkDemandDrivenPipeline::GetInputInformation(int port)
{
  return this->GetInputInformation()->GetInformationObject(port);
}

//----------------------------------------------------------------------------
vtkInformationVector* vtkDemandDrivenPipeline::GetOutputInformation()
{
  if(!this->DemandDrivenInternal->OutputInformation)
    {
    this->DemandDrivenInternal->OutputInformation =
      vtkSmartPointer<vtkInformationVector>::New();
    }
  this->DemandDrivenInternal->OutputInformation
    ->SetNumberOfInformationObjects(this->Algorithm->GetNumberOfOutputPorts());
  return this->DemandDrivenInternal->OutputInformation.GetPointer();
}

//----------------------------------------------------------------------------
vtkInformation* vtkDemandDrivenPipeline::GetOutputInformation(int port)
{
  return this->GetOutputInformation()->GetInformationObject(port);
}

//----------------------------------------------------------------------------
vtkInformation*
vtkDemandDrivenPipeline::GetOutputInformation(vtkAlgorithm* algorithm,
                                              int port)
{
  return this->Superclass::GetOutputInformation(algorithm, port);
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::Update(vtkAlgorithm* algorithm)
{
  return this->Superclass::Update(algorithm);
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::Update()
{
  if(!this->UpdateInformation())
    {
    return 0;
    }
  if(this->Algorithm->GetNumberOfOutputPorts() > 0)
    {
    return this->UpdateData(0);
    }
  else
    {
    return 1;
    }
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::UpdateInformation()
{
  // Avoid infinite recursion.
  if(this->InProcessDownstreamRequest)
    {
    vtkErrorMacro("UpdateInformation invoked during a downstream request.  "
                  "Returning failure to algorithm "
                  << this->Algorithm->GetClassName() << "("
                  << this->Algorithm << ").");
    return 0;
    }

  // The pipeline's MTime starts with this algorithm's MTime.
  this->PipelineMTime = this->Algorithm->GetMTime();

  // Get the pipeline MTime for all the inputs.
  for(int i=0; i < this->Algorithm->GetNumberOfInputPorts(); ++i)
    {
    for(int j=0; j < this->Algorithm->GetNumberOfInputConnections(i); ++j)
      {
      if(vtkDemandDrivenPipeline* e = this->GetConnectedInputExecutive(i, j))
        {
        // Propagate the UpdateInformation call
        if(!e->UpdateInformation())
          {
          return 0;
          }

        // We want the maximum PipelineMTime of all inputs.
        if(e->PipelineMTime > this->PipelineMTime)
          {
          this->PipelineMTime = e->PipelineMTime;
          }
        }
      }
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

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::UpdateData(int outputPort)
{
  // Avoid infinite recursion.
  if(this->InProcessDownstreamRequest)
    {
    vtkErrorMacro("UpdateData invoked during a downstream request.  "
                  "Returning failure to algorithm "
                  << this->Algorithm->GetClassName() << "("
                  << this->Algorithm << ").");
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

  // Make sure everything on which we might rely is up-to-date.
  for(int i=0; i < this->Algorithm->GetNumberOfInputPorts(); ++i)
    {
    for(int j=0; j < this->Algorithm->GetNumberOfInputConnections(i); ++j)
      {
      if(vtkDemandDrivenPipeline* e = this->GetConnectedInputExecutive(i, j))
        {
        if(!e->UpdateData(this->Algorithm->GetInputConnection(i, j)->GetIndex()))
          {
          return 0;
          }
        }
      }
    }

  // Make sure our outputs are up-to-date.
  int result = 1;
  if(this->PipelineMTime > this->DataTime.GetMTime())
    {
    // Make sure inputs are valid before algorithm does anything.
    if(!this->InputCountIsValid() || !this->InputTypeIsValid() ||
       !this->InputFieldsAreValid())
      {
      return 0;
      }

#if 0
    // TODO: This condition gives the default behavior if the user
    // asks for a piece that cannot be generated by the source.  Just
    // ignore the request and return empty.
    if (output && output->GetMaximumNumberOfPieces() > 0 &&
        output->GetUpdatePiece() >= output->GetMaximumNumberOfPieces())
      {
      skipExecute = 1;
      }
#endif

    // Request data from the algorithm.
    result = this->ExecuteData(outputPort);

    // Data are now up to date.
    this->DataTime.Modified();
    }
  return result;
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::ExecuteInformation()
{
  this->PrepareDownstreamRequest(REQUEST_INFORMATION());
  this->InProcessDownstreamRequest = 1;
  int result = this->Algorithm->ProcessDownstreamRequest(
    this->GetRequestInformation(), this->GetInputInformation(),
    this->GetOutputInformation());
  this->InProcessDownstreamRequest = 0;
  return result;
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::ExecuteData(int outputPort)
{
  this->PrepareDownstreamRequest(REQUEST_DATA());
  this->GetRequestInformation()->Set(FROM_OUTPUT_PORT(), outputPort);
  this->InProcessDownstreamRequest = 1;
  int result = this->Algorithm->ProcessDownstreamRequest(
    this->GetRequestInformation(), this->GetInputInformation(),
    this->GetOutputInformation());
  this->InProcessDownstreamRequest = 0;
  return result;
}

//----------------------------------------------------------------------------
vtkDataObject* vtkDemandDrivenPipeline::GetOutputData(int port)
{
  if(!this->OutputPortIndexInRange(port, "get data for"))
    {
    return 0;
    }

  // Make sure the output data object exists.
  vtkInformation* info = this->GetOutputInformation(port);
  vtkDataObject* data = this->GetOutputDataInternal(this->Algorithm, port);

  // If the algorithm specifies the output type, make sure an instance
  // of the specified type is set as the output.
  vtkInformation* portInfo = this->Algorithm->GetOutputPortInformation(port);
  if(const char* dt = portInfo->Get(vtkInformation::OUTPUT_DATA_TYPE()))
    {
    if(!(data && data->IsA(dt)))
      {
      // Try to create an instance of the correct type.
      data = this->NewDataObject(dt);
      this->SetOutputDataInternal(this->Algorithm, port, data);
      if(data)
        {
        data->Delete();
        }
      }
    }
  else
    {
    // The algorithm does not specify the output type, so we cannot
    // trust that the current output is of the correct type.
    data = 0;
    }

  // We may still not have data in the following situations:
  //  - The algorithm did not specify the output type.
  //  - The specified output type is abstract (like vtkDataSet).
  //  - NewDataObject() could not create an instance of the specified type.
  if(!data)
    {
    // We must update information on the pipeline to have the
    // algorithm create the output.
    if(this->UpdateInformation())
      {
      data = info->Get(vtkInformation::DATA_OBJECT());
      if(!data)
        {
        // The algorithm has a bug and did not create the data object.
        vtkErrorMacro("Algorithm " << this->Algorithm->GetClassName() << "("
                      << this->Algorithm
                      << ") did not create output for port " << port
                      << " when asked by REQUEST_INFORMATION.");
        }
      }
    else
      {
      // There was an error in pipeline connectivity.
      vtkErrorMacro("Output data for port " << port << " on algorithm "
                    << this->Algorithm->GetClassName() << "("
                    << this->Algorithm << ") does not exist "
                    "and UpdateInformation failed.");
      }
    }
  return data;
}

//----------------------------------------------------------------------------
vtkDataObject* vtkDemandDrivenPipeline::GetOutputData(vtkAlgorithm* algorithm,
                                                      int port)
{
  return this->Superclass::GetOutputData(algorithm, port);
}

//----------------------------------------------------------------------------
vtkDataObject* vtkDemandDrivenPipeline::GetInputData(int port, int index)
{
  if(vtkDemandDrivenPipeline* e = this->GetConnectedInputExecutive(port, index))
    {
    vtkAlgorithmOutput* input =
      this->Algorithm->GetInputConnection(port, index);
    return e->GetOutputData(input->GetIndex());
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
void
vtkDemandDrivenPipeline
::PrepareDownstreamRequest(vtkInformationIntegerKey* rkey)
{
  // Setup request information for this request.
  vtkInformation* request = this->GetRequestInformation();
  request->Clear();
  request->Set(rkey, 1);

  // Put all the input data objects into the input information.
  vtkInformationVector* inputVector = this->GetInputInformation();
  for(int ip=0; ip < this->Algorithm->GetNumberOfInputPorts(); ++ip)
    {
    vtkInformation* info = inputVector->GetInformationObject(ip);
    int numConnections = this->Algorithm->GetNumberOfInputConnections(ip);
    vtkInformationVector* connInfo =
      info->Get(vtkInformation::INPUT_CONNECTION_INFORMATION());
    if(!connInfo)
      {
      connInfo = vtkInformationVector::New();
      info->Set(vtkInformation::INPUT_CONNECTION_INFORMATION(), connInfo);
      connInfo->Delete();
      }
    connInfo->SetNumberOfInformationObjects(numConnections);
    if(numConnections > 0)
      {
      for(int i=0; i < numConnections; ++i)
        {
        connInfo->SetInformationObject(
          i, this->GetConnectedInputInformation(ip, i));
        }
      }
    }
}

//----------------------------------------------------------------------------
void
vtkDemandDrivenPipeline
::PrepareUpstreamRequest(vtkInformationIntegerKey* rkey)
{
  this->PrepareDownstreamRequest(rkey);
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

  // Enforce required type, if any.
  if(const char* dt = info->Get(vtkInformation::INPUT_REQUIRED_DATA_TYPE()))
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
    info->Get(vtkInformation::INPUT_REQUIRED_FIELDS());

  // If there are no required fields, there is nothing to check.
  if(!fields)
    {
    return 1;
    }

  // Check availability of each required field.
  int result = 1;
  for(int i=0; i < fields->GetNumberOfInformationObjects(); ++i)
    {
    vtkDataObject* input = this->GetInputData(port, index);
    vtkInformation* field = fields->GetInformationObject(i);

    // Decide which kinds of fields to check.
    int checkPoints = 1;
    int checkCells = 1;
    int checkFields = 1;
    if(field->Has(vtkInformation::FIELD_ASSOCIATION()))
      {
      switch(field->Get(vtkInformation::FIELD_ASSOCIATION()))
        {
        case vtkInformation::FIELD_ASSOCIATION_POINTS:
          checkCells = 0; checkFields = 0; break;
        case vtkInformation::FIELD_ASSOCIATION_CELLS:
          checkPoints = 0; checkFields = 0; break;
        case vtkInformation::FIELD_ASSOCIATION_NONE:
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
  if(field->Has(vtkInformation::FIELD_ATTRIBUTE_TYPE()))
    {
    // A specific attribute must match the requirements.
    int attrType = field->Get(vtkInformation::FIELD_ATTRIBUTE_TYPE());
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
  if(const char* name = field->Get(vtkInformation::FIELD_NAME()))
    {
    if(!array->GetName() || (strcmp(name, array->GetName()) != 0))
      {
      return 0;
      }
    }

  // Enforce component type for the array.
  if(field->Has(vtkInformation::FIELD_ARRAY_TYPE()))
    {
    int arrayType = field->Get(vtkInformation::FIELD_ARRAY_TYPE());
    if(array->GetDataType() != arrayType)
      {
      return 0;
      }
    }

  // Enforce number of components for the array.
  if(field->Has(vtkInformation::FIELD_NUMBER_OF_COMPONENTS()))
    {
    int arrayNumComponents =
      field->Get(vtkInformation::FIELD_NUMBER_OF_COMPONENTS());
    if(array->GetNumberOfComponents() != arrayNumComponents)
      {
      return 0;
      }
    }

  // Enforce number of tuples.  This should really only be used for
  // field data (not point or cell data).
  if(field->Has(vtkInformation::FIELD_NUMBER_OF_TUPLES()))
    {
    int arrayNumTuples = field->Get(vtkInformation::FIELD_NUMBER_OF_TUPLES());
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
    return info->Get(vtkInformation::INPUT_IS_OPTIONAL());
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkDemandDrivenPipeline::InputIsRepeatable(int port)
{
  if(vtkInformation* info = this->Algorithm->GetInputPortInformation(port))
    {
    return info->Get(vtkInformation::INPUT_IS_REPEATABLE());
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
void vtkDemandDrivenPipeline::ReportReferences(vtkGarbageCollector* collector)
{
  this->Superclass::ReportReferences(collector);
  for(int i=0; i < this->Algorithm->GetNumberOfOutputPorts(); ++i)
    {
    collector->ReportReference(
      this->GetOutputInformation(i)->Get(vtkInformation::DATA_OBJECT()),
      "AlgorithmOutput");
    }
}

//----------------------------------------------------------------------------
void vtkDemandDrivenPipeline::RemoveReferences()
{
  for(int i=0; i < this->Algorithm->GetNumberOfOutputPorts(); ++i)
    {
    this->GetOutputInformation(i)->Remove(vtkInformation::DATA_OBJECT());
    }
  this->Superclass::RemoveReferences();
}
