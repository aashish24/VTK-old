/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkInputPort.h"
#include "vtkOutputPort.h"
#include "vtkMultiProcessController.h"
#include "vtkPolyData.h"
#include "vtkUnstructuredGrid.h"
#include "vtkStructuredGrid.h"
#include "vtkRectilinearGrid.h"
#include "vtkStructuredPoints.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

vtkCxxRevisionMacro(vtkInputPort, "$Revision$");
vtkStandardNewMacro(vtkInputPort);

//----------------------------------------------------------------------------
vtkInputPort::vtkInputPort()
{
  this->RemoteProcessId = 0;
  this->Tag = 0;
  
  // Controller keeps a reference to this object as well.
  this->Controller = NULL;
  this->SetController(vtkMultiProcessController::GetGlobalController());

  // State variables.
  this->TransferNeeded = 0;
  this->DataTime = 0;

  this->DoUpdateInformation = 1;
}

//----------------------------------------------------------------------------
// We need to have a "GetNetReferenceCount" to avoid memory leaks.
vtkInputPort::~vtkInputPort()
{
  this->SetController(0);
}

//----------------------------------------------------------------------------
void vtkInputPort::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "RemoteProcessId: " << this->RemoteProcessId << endl;
  os << indent << "Tag: " << this->Tag << endl;
  os << indent << "Controller: (" << this->Controller << ")\n";
  os << indent << "DataTime: " << this->DataTime << endl;
  os << indent << "TransferNeeded: " << this->TransferNeeded << endl;  
  os << indent << "DoUpdateInformation: " << this->DoUpdateInformation << endl;
}

//----------------------------------------------------------------------------
// Maybe we can come up with a way to check the type of the upstream port's
// input here.  While we are at it, we could automatically generate a tag.
vtkPolyData *vtkInputPort::GetPolyDataOutput()
{
  vtkDataObject *output = NULL;
  
  // If there is already an output, I hope it is a vtkPolyData.
  if (this->Outputs)
    {
    output = this->Outputs[0];
    }
  if (output)
    {
    if (output->GetDataObjectType() == VTK_POLY_DATA)
      {
      return (vtkPolyData*)(output);
      }
//      else
//        {
//        vtkWarningMacro("vtkInputPort: Changing data type of output.");
//        }
    }
  
  output = vtkPolyData::New();
  output->ReleaseData();
  this->vtkSource::SetNthOutput(0, output);
  output->Delete();
  return (vtkPolyData*)(output);
}


//----------------------------------------------------------------------------
// Maybe we can come up with a way to check the type of the upstream port's
// input here.  While we are at it, we could automatically generate a tag.
vtkUnstructuredGrid *vtkInputPort::GetUnstructuredGridOutput()
{
  vtkDataObject *output = NULL;
  
  // If there is already an output, I hope it is a vtkPolyData.
  if (this->Outputs)
    {
    output = this->Outputs[0];
    }
  if (output)
    {
    if (output->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
      {
      return (vtkUnstructuredGrid*)(output);
      }
//      else
//        {
//        vtkWarningMacro("vtkInputPort: Changing data type of output.");
//        }
    }
  
  output = vtkUnstructuredGrid::New();
  output->ReleaseData();
  this->vtkSource::SetNthOutput(0, output);
  output->Delete();
  return (vtkUnstructuredGrid*)(output);
}

//----------------------------------------------------------------------------
// Maybe we can come up with a way to check the type of the upstream port's
// input here.  While we are at it, we could automatically generate a tag.
vtkStructuredGrid *vtkInputPort::GetStructuredGridOutput()
{
  vtkDataObject *output = NULL;
  
  // If there is already an output, I hope it is a vtkPolyData.
  if (this->Outputs)
    {
    output = this->Outputs[0];
    }
  if (output)
    {
    if (output->GetDataObjectType() == VTK_STRUCTURED_GRID)
      {
      return (vtkStructuredGrid*)(output);
      }
//      else
//        {
//        vtkWarningMacro("vtkInputPort: Changing data type of output.");
//        }
    }
  
  output = vtkStructuredGrid::New();
  output->ReleaseData();
  this->vtkSource::SetNthOutput(0, output);
  output->Delete();
  return (vtkStructuredGrid*)(output);
}


//----------------------------------------------------------------------------
// Maybe we can come up with a way to check the type of the upstream port's
// input here.  While we are at it, we could automatically generate a tag.
vtkRectilinearGrid *vtkInputPort::GetRectilinearGridOutput()
{
  vtkDataObject *output = NULL;
  
  // If there is already an output, I hope it is a vtkPolyData.
  if (this->Outputs)
    {
    output = this->Outputs[0];
    }
  if (output)
    {
    if (output->GetDataObjectType() == VTK_RECTILINEAR_GRID)
      {
      return (vtkRectilinearGrid*)(output);
      }
//      else
//        {
//        vtkWarningMacro("vtkInputPort: Changing data type of output.");
//        }
    }
  
  output = vtkRectilinearGrid::New();
  output->ReleaseData();
  this->vtkSource::SetNthOutput(0, output);
  output->Delete();
  return (vtkRectilinearGrid*)(output);
}


//----------------------------------------------------------------------------
// Maybe we can come up with a way to check the type of the upstream port's
// input here.  While we are at it, we could automatically generate a tag.
vtkStructuredPoints *vtkInputPort::GetStructuredPointsOutput()
{
  vtkDataObject *output = NULL;
  
  // If there is already an output, I hope it is a vtkPolyData.
  if (this->Outputs)
    {
    output = this->Outputs[0];
    }
  if (output)
    {
    if (output->GetDataObjectType() == VTK_STRUCTURED_POINTS)
      {
      return (vtkStructuredPoints*)(output);
      }
//      else
//        {
//        vtkWarningMacro("vtkInputPort: Changing data type of output.");
//        }
    }
  
  output = vtkStructuredPoints::New();
  output->ReleaseData();
  this->vtkSource::SetNthOutput(0, output);
  output->Delete();
  return (vtkStructuredPoints*)(output);
}


//----------------------------------------------------------------------------
// Maybe we can come up with a way to check the type of the upstream port's
// input here.  While we are at it, we could automatically generate a tag.
vtkImageData *vtkInputPort::GetImageDataOutput()
{
  vtkDataObject *output = NULL;
  
  // If there is already an output, I hope it is a vtkImageData.
  if (this->Outputs)
    {
    output = this->Outputs[0];
    }
  if (output)
    {
    if (output->GetDataObjectType() == VTK_IMAGE_DATA)
      {
      return (vtkImageData*)(output);
      }
//      else
//        {
//        vtkWarningMacro("vtkInputPort: Changing data type of output.");
//        }
    }
  
  output = vtkImageData::New();
  output->ReleaseData();
  this->vtkSource::SetNthOutput(0, output);
  output->Delete();
  return (vtkImageData*)(output);
}


//----------------------------------------------------------------------------
// The only tricky thing here is the translation of the PipelineMTime
// into a value meaningful to this process.
void vtkInputPort::UpdateInformation()
{
  vtkDataObject *output;
  unsigned long pmt;
  
  if (!this->DoUpdateInformation)
    {
    return;
    }

  if (this->Outputs == NULL || this->Outputs[0] == NULL)
    {
    vtkErrorMacro("No output.");
    return;
    }

  output = this->Outputs[0];
  
  // Trigger UpdateInformation in remotePort.
  // Up-stream port should have the same tag.
  this->Controller->TriggerRMI(this->RemoteProcessId, this->Tag);
  
  // Now receive the information
  int wholeInformation[7];
  this->Controller->Receive( wholeInformation, 7, 
                             this->RemoteProcessId,
                             vtkInputPort::INFORMATION_TRANSFER_TAG);

  this->Controller->Receive( &pmt, 1, 
                             this->RemoteProcessId,
                             vtkInputPort::INFORMATION_TRANSFER_TAG);
  int maxNumPieces;
  this->Controller->Receive( &maxNumPieces, 1, 
                             this->RemoteProcessId,
                             vtkInputPort::INFORMATION_TRANSFER_TAG);
  output->SetMaximumNumberOfPieces(maxNumPieces);

  output->SetWholeExtent( wholeInformation );
    
  // Save the upstream PMT for execute check (this may not be necessary)
  this->UpStreamMTime = pmt;

  // !!! Make sure that Update is called if data is released. !!!
  if (pmt > this->DataTime || output->GetDataReleased())
    {
    // Our data is out of data.  We will need a transfer.
    // This Modified call will ensure Update will get called.
    this->Modified();
    }
  output->SetPipelineMTime(this->GetMTime());
  // Locality has to be changed too.
  output->SetLocality(1.0);
  
}


//----------------------------------------------------------------------------
// The only tricky thing here is the translation of the PipelineMTime
// into a value meaningful to this process.
void vtkInputPort::TriggerAsynchronousUpdate()
{
  // This should be cleared by this point.
  // UpdateInformation and Update calls need to be made in pairs.
  if (this->TransferNeeded)
    {
    vtkWarningMacro("Transfer should have been received.");
    return;
    }

  vtkDataObject *output = this->Outputs[0];

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  // This would normally be done in the Update method, but since
  // we want task parallelism with multiple input filters, 
  // it needs to be here.

  // Do we need to update?
  // !!! I am uneasy about the Released check.  Although a new update extent
  // will cause the data to be released,  released data does not imply 
  // Update will be called !!!!
  if (this->UpStreamMTime <= this->DataTime && ! output->GetDataReleased())
    { 
    // No, we do not need to update.
    return;
    }

  // Trigger Update in remotePort.
  // remotePort should have the same tag.
  this->Controller->TriggerRMI(this->RemoteProcessId, this->Tag+1);
  
  // Send the UpdateExtent request. The first 6 ints are the 3d extent, the next
  // to are the pieces extent (we don't know which type it is - just send both)
  int extent[9];
  output->GetUpdateExtent( extent );
  extent[6] = output->GetUpdatePiece();
  extent[7] = output->GetUpdateNumberOfPieces();  
  extent[8] = output->GetUpdateGhostLevel();  
  this->Controller->Send( extent, 9, this->RemoteProcessId, 
                          vtkInputPort::UPDATE_EXTENT_TAG);

  // This is for pipeline parallism.
  // The Upstream port may or may not promote its data (execute).
  // It needs the data time of our output to compare to the mtime
  // of its input to determine if it should send the data (execute).
  this->Controller->Send( &(this->DataTime), 1, this->RemoteProcessId,
                          vtkInputPort::NEW_DATA_TIME_TAG);
  
  // This automatically causes to remotePort to send the data.
  // Tell the update method to receive the data.
  this->TransferNeeded = 1;
}


//----------------------------------------------------------------------------
void vtkInputPort::UpdateData(vtkDataObject *output)
{

  if (this->UpStreamMTime <= this->DataTime && ! output->GetDataReleased())
    { 
    // No, we do not need to update.
    return;
    }

  if ( ! this->TransferNeeded)
    {
    // If something unexpected happened, let me know.
    vtkWarningMacro("UpdateData was called when no data was needed.");
    return;
    }
  
  this->InvokeEvent(vtkCommand::StartEvent,NULL);

  // Well here is a bit of a hack.
  // Since the reader will overwrite whole extents, we need to save the whole
  // extent and reset it.
  int wholeExtent[6];
  output->GetWholeExtent(wholeExtent);
  // receive the data

  this->Controller->Receive(output, this->RemoteProcessId,
                            vtkInputPort::DATA_TRANSFER_TAG);

  output->SetWholeExtent( wholeExtent );

  this->InvokeEvent(vtkCommand::EndEvent,NULL);

  // Receive the data time
  this->Controller->Receive( &(this->DataTime), 1, this->RemoteProcessId,
                            vtkInputPort::NEW_DATA_TIME_TAG);
     
  this->TransferNeeded = 0;
}






















