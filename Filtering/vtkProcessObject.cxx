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
#include "vtkProcessObject.h"

#include "vtkAlgorithmOutput.h"
#include "vtkCommand.h"
#include "vtkDataObject.h"
#include "vtkGarbageCollector.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkSource.h"
#include "vtkInformation.h"

#include "vtkDebugLeaks.h"

vtkCxxRevisionMacro(vtkProcessObject, "$Revision$");

//----------------------------------------------------------------------------

// Fake data object type used to represent NULL connections for the
// compatibility layer.
class vtkProcessObjectDummyData: public vtkDataObject
{
public:
  vtkTypeMacro(vtkProcessObjectDummyData, vtkDataObject);
  static vtkProcessObjectDummyData* New()
    {
#ifdef VTK_DEBUG_LEAKS
    vtkDebugLeaks::ConstructClass("vtkProcessObjectDummyData");
#endif
    return new vtkProcessObjectDummyData;
    }
protected:
  vtkProcessObjectDummyData() {}; 
  virtual ~vtkProcessObjectDummyData() {}; 
private:
  vtkProcessObjectDummyData(const vtkProcessObjectDummyData&);
  void operator=(const vtkProcessObjectDummyData&);  // Not implemented.
};

//----------------------------------------------------------------------------
vtkProcessObject::vtkProcessObject()
{
  this->NumberOfInputs = 0;
  this->NumberOfRequiredInputs = 0;
  this->Inputs = NULL;

  this->SetNumberOfInputPorts(1);
}

// Destructor for the vtkProcessObject class
vtkProcessObject::~vtkProcessObject()
{
  int idx;

  for (idx = 0; idx < this->NumberOfInputs; ++idx)
    {
    if (this->Inputs[idx])
      {
      this->Inputs[idx]->UnRegister(this);
      this->Inputs[idx] = NULL;
      }
    }
  if (this->Inputs)
    {
    delete [] this->Inputs;
    this->Inputs = NULL;
    this->NumberOfInputs = 0;
    }
}

//----------------------------------------------------------------------------
vtkDataObject** vtkProcessObject::GetInputs()
{
  return this->Inputs;
}

//----------------------------------------------------------------------------
int vtkProcessObject::GetNumberOfInputs()
{
  return this->NumberOfInputs;
}

//----------------------------------------------------------------------------
void vtkProcessObject::SetNumberOfInputs(int)
{
  // Input array size management is automatic.  Do nothing.
}

//----------------------------------------------------------------------------
void vtkProcessObject::AddInput(vtkDataObject* input)
{
  this->AddInputInternal(input);
}

//----------------------------------------------------------------------------
void vtkProcessObject::RemoveInput(vtkDataObject* input)
{
  this->RemoveInputInternal(input);
}

//----------------------------------------------------------------------------
void vtkProcessObject::SqueezeInputArray()
{
  // Array is always squeezed.  Do nothing.
}

//----------------------------------------------------------------------------
void vtkProcessObject::SetNthInput(int idx, vtkDataObject* input)
{
  int num = idx;
  // Check whether anything will change.
  if(num >= 0 && num < this->GetNumberOfInputConnections(0))
    {
    if(this->Inputs[num] == input)
      {
      return;
      }
    }
  else if(num < 0)
    {
    vtkErrorMacro("SetNthInput cannot set input index " << num << ".");
    return;
    }

  if(input && num > this->GetNumberOfInputConnections(0))
    {
    // Avoid creating holes in input array.  Use dummy data to fill in
    // the missing connections.
    for(int i=this->GetNumberOfInputConnections(0); i < num; ++i)
      {
      vtkProcessObjectDummyData* d = vtkProcessObjectDummyData::New();
      this->AddInputInternal(d);
      d->Delete();
      }

    // Now add the real input.
    this->AddInputInternal(input);
    }
  else if(!input && num < this->GetNumberOfInputConnections(0)-1)
    {
    vtkErrorMacro("SetNthInput cannot set input index " << num
                  << " to NULL because there are "
                  << this->GetNumberOfInputConnections(0)
                  << " connections and NULL connections are not allowed.");
    }
  else if(input && num == this->GetNumberOfInputConnections(0))
    {
    this->AddInputInternal(input);
    }
  else if(!input && num == this->GetNumberOfInputConnections(0)-1)
    {
    // need to get the current algorithm input if there is one
    vtkAlgorithmOutput *inputToBeRemoved = this->GetInputConnection(0,num);
    this->RemoveInputConnection(0, inputToBeRemoved);
    }
  else if(input && num < this->GetNumberOfInputConnections(0))
    {
    this->SetNthInputConnection(0, num, input->GetProducerPort());
    }
}

//----------------------------------------------------------------------------
void vtkProcessObject::RemoveAllInputs()
{
  this->SetInputConnection(0, 0);
}

//----------------------------------------------------------------------------
int vtkProcessObject::FillInputPortInformation(int, vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
  if(this->NumberOfRequiredInputs == 0)
    {
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkProcessObject::FillOutputPortInformation(int, vtkInformation*)
{
  return 1;
}

//----------------------------------------------------------------------------
void vtkProcessObject::ReportReferences(vtkGarbageCollector* collector)
{
  this->Superclass::ReportReferences(collector);
  for(int i=0; i < this->NumberOfInputs; ++i)
    {
    vtkGarbageCollectorReport(collector, this->Inputs[i], "Inputs");
    }
}

//----------------------------------------------------------------------------
void vtkProcessObject::SetInputConnection(int port, vtkAlgorithmOutput* input)
{
  this->Superclass::SetInputConnection(port, input);
  this->SetupInputs();
}

//----------------------------------------------------------------------------
void vtkProcessObject::AddInputConnection(int port, vtkAlgorithmOutput* input)
{
  this->Superclass::AddInputConnection(port, input);
  this->SetupInputs();
}

//----------------------------------------------------------------------------
void vtkProcessObject::RemoveInputConnection(int port, vtkAlgorithmOutput* input)
{
  this->Superclass::RemoveInputConnection(port, input);
  this->SetupInputs();
}

//----------------------------------------------------------------------------
void vtkProcessObject::AddInputInternal(vtkDataObject* input)
{
  if(input)
    {
    this->AddInputConnection(0, input->GetProducerPort());
    }
}

//----------------------------------------------------------------------------
void vtkProcessObject::RemoveInputInternal(vtkDataObject* input)
{
  if(input)
    {
    this->RemoveInputConnection(0, input->GetProducerPort());
    }
}

//----------------------------------------------------------------------------
void vtkProcessObject::SetupInputs()
{
  // Construct a new array of input data objects using connections
  // from input port 0.
  typedef vtkDataObject* vtkDataObjectPointer;
  vtkDataObject** newInputs = 0;
  int newNumberOfInputs = this->GetNumberOfInputConnections(0);
  if(newNumberOfInputs > 0)
    {
    newInputs = new vtkDataObjectPointer[newNumberOfInputs];
    int count=0;
    for(int i=0; i < this->GetNumberOfInputConnections(0); ++i)
      {
      vtkAlgorithmOutput* ic = this->GetInputConnection(0, i);
      newInputs[count] = ic->GetProducer()->GetOutputDataObject(ic->GetIndex());
      if(newInputs[count])
        {
        // If the connection has dummy data, set a NULL input.
        if(newInputs[count]->IsA("vtkProcessObjectDummyData"))
          {
          newInputs[count] = 0;
          }
        else
          {
          // If the data object was already an input, avoid the
          // Register/UnRegister cycle.
          int found = 0;
          for(int j=0; !found && j < this->NumberOfInputs; ++j)
            {
            if(newInputs[count] == this->Inputs[j])
              {
              this->Inputs[j] = 0;
              found = 1;
              }
            }
          if(!found)
            {
            newInputs[count]->Register(this);
            }
          }
        ++count;
        }
      }
    newNumberOfInputs = count;
    }

  // Remove the old array of input data objects.
  if(this->NumberOfInputs)
    {
    for(int i=0; i < this->NumberOfInputs; ++i)
      {
      if(this->Inputs[i])
        {
        this->Inputs[i]->UnRegister(this);
        }
      }
    delete [] this->Inputs;
    }

  // Save the new array of input data objects.
  this->NumberOfInputs = newNumberOfInputs;
  this->Inputs = newInputs;
}

//----------------------------------------------------------------------------
void vtkProcessObject::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Number Of Required Inputs: "
     << this->NumberOfRequiredInputs << endl;

  if ( this->NumberOfInputs)
    {
    int idx;
    for (idx = 0; idx < this->NumberOfInputs; ++idx)
      {
      os << indent << "Input " << idx << ": (" << this->Inputs[idx] << ")\n";
      }
    }
  else
    {
    os << indent <<"No Inputs\n";
    }
  
}
