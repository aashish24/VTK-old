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
#include "vtkInputPort.h"

#include "vtkCommand.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiProcessController.h"
#include "vtkObjectFactory.h"
#include "vtkOutputPort.h"
#include "vtkPolyData.h"
#include "vtkRectilinearGrid.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkStructuredGrid.h"
#include "vtkStructuredPoints.h"
#include "vtkUnstructuredGrid.h"

vtkCxxRevisionMacro(vtkInputPort, "$Revision$");
vtkStandardNewMacro(vtkInputPort);

vtkCxxSetObjectMacro(vtkInputPort,Controller, vtkMultiProcessController);

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

  this->LastUpdatePiece = -1;
  this->LastUpdateNumberOfPieces = -1;
  this->LastUpdateGhostLevel = -1;

  this->LastUpdateExtent[0] = this->LastUpdateExtent[1] = 
    this->LastUpdateExtent[2] = this->LastUpdateExtent[3] = 
    this->LastUpdateExtent[4] = this->LastUpdateExtent[5] = 0;
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
  if (this->UpStreamMTime <= this->DataTime && ! output->GetDataReleased() &&
    !this->UpdateExtentIsOutsideOfTheExtent(output) )
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





int vtkInputPort::UpdateExtentIsOutsideOfTheExtent(vtkDataObject *output)
  {
  switch ( output->GetExtentType() )
    {
    case VTK_PIECES_EXTENT:
      if ( this->LastUpdatePiece != output->GetUpdatePiece() ||
           this->LastUpdateNumberOfPieces != output->GetUpdateNumberOfPieces() ||
           this->LastUpdateGhostLevel != output->GetUpdateGhostLevel())
        {
        return 1;
        }
      break;

    case VTK_3D_EXTENT:
      int extent[6];
      output->GetUpdateExtent(extent);
      if ( extent[0] < this->LastUpdateExtent[0] ||
           extent[1] > this->LastUpdateExtent[1] ||
           extent[2] < this->LastUpdateExtent[2] ||
           extent[3] > this->LastUpdateExtent[3] ||
           extent[4] < this->LastUpdateExtent[4] ||
           extent[5] > this->LastUpdateExtent[5] )
        {
        return 1;
        }
      break;

    // We should never have this case occur
    default:
      vtkErrorMacro( << "Internal error - invalid extent type!" );
      break;
    }
  return 0;

  }

#ifdef VTK_USE_EXECUTIVES
unsigned long vtkInputPort::GetMTime()
{
  if (this->DoUpdateInformation)
    {
    vtkDataObject *output;
    unsigned long pmt = 0;
    
    if (this->Outputs == NULL || this->Outputs[0] == NULL)
      {
      vtkErrorMacro("No output.");
      return this->Superclass::GetMTime();
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
    int maxNumPieces = 0;
    this->Controller->Receive( &maxNumPieces, 1, 
                               this->RemoteProcessId,
                               vtkInputPort::INFORMATION_TRANSFER_TAG);

    // Save the upstream PMT for execute check (this may not be necessary)
    this->UpStreamMTime = pmt;

    // !!! Make sure that Update is called if data is released. !!!
    if (pmt > this->DataTime || output->GetDataReleased())
      {
      // Our data is out of data.  We will need a transfer.
      // This Modified call will ensure Update will get called.
      this->Modified();
      }
    }
  
  return this->Superclass::GetMTime();
}

void vtkInputPort::ExecuteInformation()
{
  vtkDataObject *output;
  unsigned long pmt = 0;
  
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
  int maxNumPieces = 0;
  this->Controller->Receive( &maxNumPieces, 1, 
                             this->RemoteProcessId,
                             vtkInputPort::INFORMATION_TRANSFER_TAG);
  output->SetMaximumNumberOfPieces(maxNumPieces);
  output->SetWholeExtent( wholeInformation );
    
  // Locality has to be changed too.
  output->SetLocality(1.0);  
}

#else

//----------------------------------------------------------------------------
void vtkInputPort::UpdateData(vtkDataObject *output)
{

  if (this->UpStreamMTime <= this->DataTime && ! output->GetDataReleased() &&
    !this->UpdateExtentIsOutsideOfTheExtent(output) )
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

  this->LastUpdatePiece = output->GetUpdatePiece();
  this->LastUpdateNumberOfPieces = output->GetUpdateNumberOfPieces();
  this->LastUpdateGhostLevel = output->GetUpdateGhostLevel();

  this->SetLastUpdateExtent( output->GetUpdateExtent() );
}

//----------------------------------------------------------------------------
// The only tricky thing here is the translation of the PipelineMTime
// into a value meaningful to this process.
void vtkInputPort::UpdateInformation()
{
  vtkDataObject *output;
  unsigned long pmt = 0;
  
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
  int maxNumPieces = 0;
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
#endif


#ifdef VTK_USE_EXECUTIVES
int vtkInputPort::FillOutputPortInformation(
  int port, vtkInformation* info)
{
  // invoke super first
  int retVal = this->Superclass::FillOutputPortInformation(port, info);
  
  // now add our info
  info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkDataSet");
  
  return retVal;
}
#endif

int vtkInputPort::ProcessUpstreamRequest(
  vtkInformation *request, 
  vtkInformationVector *inputVector, 
  vtkInformationVector *outputVector)
{
#ifdef VTK_USE_EXECUTIVES
  if(request->Has(vtkStreamingDemandDrivenPipeline::REQUEST_UPDATE_EXTENT()))
    {
    // we must set the extent on the input
    vtkInformation* outInfo = outputVector->GetInformationObject(0);
    
    // Send the UpdateExtent request. The first 6 ints are the 3d extent, the
    // next to are the pieces extent (we don't know which type it is - just
    // send both)
    int extent[9];
    vtkDataObject* dataObject = outInfo->Get(vtkDataObject::DATA_OBJECT());
    vtkInformation* dataInfo = dataObject->GetInformation();
    if(dataInfo->Get(vtkDataObject::DATA_EXTENT_TYPE()) == VTK_PIECES_EXTENT)
      {
      extent[6] = outInfo->Get(
        vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER());
      extent[7] = outInfo->Get(
        vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES());
      extent[8]= outInfo->Get(
        vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS());
      }
    else if(dataInfo->Get(vtkDataObject::DATA_EXTENT_TYPE()) == VTK_3D_EXTENT)
      {
      outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), extent);
      }
    this->Controller->Send( extent, 9, this->RemoteProcessId, 
                            vtkInputPort::UPDATE_EXTENT_TAG);
    
    return 1;
    }
#else
  (void)request;
#endif
  return 0;
}

int vtkInputPort::ProcessDownstreamRequest(
  vtkInformation *request, 
  vtkInformationVector *inputVector, 
  vtkInformationVector *outputVector)
{
#ifdef VTK_USE_EXECUTIVES
  
  // this is basically execute information
  if(request->Has(vtkDemandDrivenPipeline::REQUEST_INFORMATION()))
    {
    // Ask the subclass to fill in the information for the outputs.
    this->InvokeEvent(vtkCommand::ExecuteInformationEvent, NULL);
    
    vtkDataObject *output;
    unsigned long pmt = 0;
  
    if (!this->DoUpdateInformation)
      {
      return 1;
      }

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
    int maxNumPieces = 0;
    this->Controller->Receive( &maxNumPieces, 1, 
                               this->RemoteProcessId,
                               vtkInputPort::INFORMATION_TRANSFER_TAG);

    
    if (this->Outputs == NULL || this->Outputs[0] == NULL)
      {
      vtkErrorMacro("No output.");
      return 0;
      }

    output = this->Outputs[0];
  

    output->SetMaximumNumberOfPieces(maxNumPieces);
    output->SetWholeExtent( wholeInformation );
    
    // Locality has to be changed too.
    output->SetLocality(1.0);  

  

    // information, we just need to change any that should be different from
    // the input
    vtkInformation* outInfo = outputVector->GetInformationObject(0);

    // make sure the output is there
//    vtkDataObject *output = 
//      vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
    
    if (!output)
      {
      output = vtkImageData::New();
      outInfo->Set(vtkDataObject::DATA_OBJECT(), output);
      output->Delete();
      }
    return 1;
    }

  
  // generate the data
  else if(request->Has(vtkDemandDrivenPipeline::REQUEST_DATA()))
    {
    // get the output data object
    vtkInformation* outInfo = outputVector->GetInformationObject(0);
    vtkImageData *output = 
      vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));


    return 1;
    }
  return 0;
#else
  return this->Superclass::ProcessDownstreamRequest(request, inputVector,
                                                    outputVector);
#endif
}


