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
#include "vtkOutputPort.h"

#include "vtkCommand.h"
#include "vtkDataObject.h"
#include "vtkDemandDrivenPipeline.h"
#include "vtkInformation.h"
#include "vtkInputPort.h"
#include "vtkMultiProcessController.h"
#include "vtkMultiProcessController.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkOutputPort, "$Revision$");
vtkStandardNewMacro(vtkOutputPort);

vtkCxxSetObjectMacro(vtkOutputPort,Controller,vtkMultiProcessController);

//----------------------------------------------------------------------------
vtkOutputPort::vtkOutputPort()
{
  this->Tag = -1;
  
  // Controller keeps a reference to this object as well.
  this->Controller = NULL;
  this->SetController(vtkMultiProcessController::GetGlobalController());
  
  this->PipelineFlag = 0;
  this->ParameterMethod = NULL;
  this->ParameterMethodArgDelete = NULL;
  this->ParameterMethodArg = NULL;

  this->SetNumberOfInputPorts(1);
}

//----------------------------------------------------------------------------
// We need to have a "GetNetReferenceCount" to avoid memory leaks.
vtkOutputPort::~vtkOutputPort()
{
  this->SetController(0);

  if ((this->ParameterMethodArg)&&(this->ParameterMethodArgDelete))
    {
    (*this->ParameterMethodArgDelete)(this->ParameterMethodArg);
    }
}

//----------------------------------------------------------------------------
void vtkOutputPort::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Tag: " << this->Tag << endl;
  os << indent << "Controller: (" << this->Controller << ")\n";
  os << indent << "Pipeline Flag: " 
     << (this->PipelineFlag ? "On\n" : "Off\n");
}


//----------------------------------------------------------------------------
// Remote method call to UpdateInformation and send the information downstream.
// This should be a friend.
void vtkOutputPortUpdateInformationCallBack(void *arg, void *remoteArgs,
                                    int remoteArgsLength, int remoteProcessId)
{
  vtkOutputPort *self = (vtkOutputPort*)arg;
  
  remoteArgs = remoteArgs;
  remoteArgsLength = remoteArgsLength;
  // Just call a method
  self->TriggerUpdateInformation(remoteProcessId);
}
//----------------------------------------------------------------------------
void vtkOutputPort::TriggerUpdateInformation(int remoteProcessId)
{
  vtkDataObject *input = this->GetInput();
  
  // Handle no input gracefully.
  if ( input != NULL )
    {
    input->UpdateInformation();
    }
  
  // The MTime of the input should also be considered.
  // Important for pipeline parallelism.
  // Include it in the information for efficiency.
  unsigned long latestMTime;
  latestMTime = input->GetMTime();
  vtkDemandDrivenPipeline *ddp = 
    vtkDemandDrivenPipeline::SafeDownCast(this->GetExecutive());
  if (ddp)
    {
    ddp->UpdateInformation();
    unsigned long t2 = ddp->GetPipelineMTime();
    if (t2 > latestMTime)
      {
      latestMTime = t2;
      }
    }
  
  // Now just send the information downstream.
  // PipelineMTime is part of information, so downstream
  // port will make the time comparison, and call Update if necessary.
  int wholeInformation[8];
  input->GetWholeExtent( wholeInformation );
  
  this->Controller->Send( wholeInformation, 7,
                          remoteProcessId, vtkInputPort::INFORMATION_TRANSFER_TAG);
  
  this->Controller->Send( &latestMTime, 1,
                          remoteProcessId, vtkInputPort::INFORMATION_TRANSFER_TAG );

  int maxNumPieces = input->GetMaximumNumberOfPieces();
  this->Controller->Send( &maxNumPieces, 1,
                          remoteProcessId, vtkInputPort::INFORMATION_TRANSFER_TAG );
}


//----------------------------------------------------------------------------
// Remote method call to Update and send data downstream.
// This should be a friend.
void vtkOutputPortUpdateCallBack(void *arg, void *remoteArgs, 
                                 int remoteArgsLength, int remoteProcessId)  
{
  vtkOutputPort *self = (vtkOutputPort*)arg;
  
  remoteArgs = remoteArgs;
  remoteArgsLength = remoteArgsLength;  
  // Just call a method
  self->TriggerUpdate(remoteProcessId);
}
//----------------------------------------------------------------------------
void vtkOutputPort::TriggerUpdate(int remoteProcessId)
{
  unsigned long downDataTime;
  vtkDataObject *input = this->GetInput();
  
  // First get the update extent requested.
  int extent[9];
  this->Controller->Receive( extent, 9, remoteProcessId, 
                            vtkInputPort::UPDATE_EXTENT_TAG);
  input->SetUpdateExtent( extent );
  input->SetUpdatePiece( extent[6] );
  input->SetUpdateNumberOfPieces( extent[7] );
  input->SetUpdateGhostLevel( extent[8] );
  
  // Note:  Receiving DataTime was the start of a more intelligent promotion
  // for pipeline parallism.  Unfortunately there was no way (I knew of)
  // for us to not promote on the first Update.  I backed off, and am 
  // requiring either 1: Filters handle Update Not returning correct data,
  // or 2: Pipeline parallelism must be primed with Updates on the Output
  // ports (giving correct data).  This Receive can be removed.
  
  // This is for pipeline parallism.
  // This Output port may or may not promote our data (execute).
  // We need the data time of the last transfer to compare to the mtime
  // of our input to determine if it should send the data (execute).
  this->Controller->Receive( &(downDataTime), 1, remoteProcessId,
                             vtkInputPort::NEW_DATA_TIME_TAG);

  // What was the idea of using relesed data here?  It caused a bug for multiple updates.
  // if ( input != NULL && input->GetDataReleased())
  
  // Postpone the update if we want pipeline parallism.
  // Handle no input gracefully. (Not true: Later we will send a NULL input.)
  if ( input != NULL && this->PipelineFlag == 0)
    {
    input->UpdateInformation();
    input->PropagateUpdateExtent();
    input->TriggerAsynchronousUpdate();
    input->UpdateData();
    }

  // Did the input change?
  // If it did then we should execute (i.e. we should send the data).
  // Note: We may need some logic to catch the case where the down port
  // has released its data.
  //if (downDataTime < input->GetMTime())
  if (input->GetDataReleased() == 0)
    {
    this->InvokeEvent(vtkCommand::StartEvent,NULL);
    // First transfer the new data.
    this->Controller->Send( input, remoteProcessId,
                            vtkInputPort::DATA_TRANSFER_TAG);
    this->InvokeEvent(vtkCommand::EndEvent,NULL);
    
    // Since this time has to be local to downstream process
    // and we have no data, we have to create a time here.
    // (The output data usually does this.) 
    this->UpdateTime.Modified();
  
    // Since this OutputPort can have multiple InputPorts
    // and the InputPort makes the update-descision time comparison,
    // the InputPort has to store this time.
    downDataTime = this->UpdateTime.GetMTime();
    this->Controller->Send( &downDataTime, 1, remoteProcessId,
                            vtkInputPort::NEW_DATA_TIME_TAG);
    }
  else
    {  // Nothing to send.  We have to signal somehow.
    vtkDebugMacro("Promoting NULL (" << input << ") to process " 
                  << remoteProcessId);
    this->Controller->Send( (vtkDataObject*)(NULL), remoteProcessId,
                            vtkInputPort::DATA_TRANSFER_TAG);
    
    // Go through the motions of sending the data time,
    // but just send the same data time back. (nothing changed).
    this->Controller->Send( &downDataTime, 1, remoteProcessId,
                            vtkInputPort::NEW_DATA_TIME_TAG);
    }
  
  // Postpone the update if we want pipeline parallism.
  // Handle no input gracefully. (Not true: Later we will send a NULL input.)
  if (this->PipelineFlag)
    {
    // change any parameters if the user wants to.
    if ( this->ParameterMethod )
      {
      (*this->ParameterMethod)(this->ParameterMethodArg);
      input->UpdateInformation();
      }
    
    // Update to anticipate the next request.
    if ( input != NULL )
      {
      input->UpdateInformation();
      input->PropagateUpdateExtent();
      input->TriggerAsynchronousUpdate();
      input->UpdateData();
      }
    }
}




//----------------------------------------------------------------------------
void vtkOutputPort::SetInput(vtkDataObject *input)
{
  if(input)
    {
    this->SetInputConnection(0, input->GetProducerPort());
    }
  else
    {
    // Setting a NULL input removes the connection.
    this->SetInputConnection(0, 0);
    }
}

//----------------------------------------------------------------------------
vtkDataObject *vtkOutputPort::GetInput()
{
  if (this->GetNumberOfInputConnections(0) < 1)
    {
    return 0;
    }
  return this->GetExecutive()->GetInputData(0, 0);
}


//----------------------------------------------------------------------------
// We need to create two RMIs when the tag is set.
// This means we must generate two tags form this ports tag.
// The ports tag should be even. 
// (I do not like this, but is there another solution?)
void vtkOutputPort::SetTag(int tag)
{
  if (this->Tag == tag)
    {
    return;
    }
  
  this->Modified();
  
  // remove old RMI.
  if (this->Tag != -1)
    {
//    this->Controller->RemoveRMI(vtkOutputPortUpdateInformationCallBack, 
//                                (void *)this, this->Tag);
//    this->Controller->RemoveRMI(vtkOutputPortUpdateCallBack, 
//                                (void *)this, this->Tag + 1);
    }
  
  this->Tag = tag;
  this->Controller->AddRMI(vtkOutputPortUpdateInformationCallBack, 
                           (void *)this, tag);
  this->Controller->AddRMI(vtkOutputPortUpdateCallBack, 
                           (void *)this, tag+1);
}


//----------------------------------------------------------------------------
void vtkOutputPort::SetParameterMethod(void (*f)(void *), void *arg)
{
  if ( f != this->ParameterMethod || arg != this->ParameterMethodArg )
    {
    // delete the current arg if there is one and a delete meth
    if ((this->ParameterMethodArg)&&(this->ParameterMethodArgDelete))
      {
      (*this->ParameterMethodArgDelete)(this->ParameterMethodArg);
      }
    this->ParameterMethod = f;
    this->ParameterMethodArg = arg;
    this->Modified();
    }
}


//----------------------------------------------------------------------------
void vtkOutputPort::SetParameterMethodArgDelete(void (*f)(void *))
{
  if ( f != this->ParameterMethodArgDelete)
    {
    this->ParameterMethodArgDelete = f;
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkOutputPort::WaitForUpdate() 
{
  this->Controller->ProcessRMIs();
}

//----------------------------------------------------------------------------
int vtkOutputPort
::FillInputPortInformation(int vtkNotUsed( port ), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}
