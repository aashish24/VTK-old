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
// .NAME vtkInputPort - Receives data from another process. 
// .SECTION Description
// InputPort connects the pipeline in this process to one in another 
// processes.  It communicates all the pipeline protocol so that
// the fact you are running in multiple processes is transparent.
// An input port is used as a source (input to a process). 
// One is placed at the start of a pipeline, and has a single 
// corresponding output port in another process 
// (specified by RemoteProcessId).

// .SECTION see also
// vtkOutputPort vtkMultiProcessController

#ifndef __vtkInputPort_h
#define __vtkInputPort_h

#include "vtkSource.h"

class vtkPolyData;
class vtkUnstructuredGrid;
class vtkStructuredGrid;
class vtkRectilinearGrid;
class vtkStructuredPoints;
class vtkImageData;
class vtkMultiProcessController;

class VTK_PARALLEL_EXPORT vtkInputPort : public vtkSource
{
public:
  static vtkInputPort *New();
  vtkTypeRevisionMacro(vtkInputPort,vtkSource);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Note: You have to ask for the right type, and it has to match
  // the type of the up stream port input, or you will get an error.
  // We have to live with the fact that the error will not occur until
  // an update is called.
  vtkPolyData *GetPolyDataOutput();
  vtkUnstructuredGrid *GetUnstructuredGridOutput();
  vtkStructuredGrid *GetStructuredGridOutput();
  vtkRectilinearGrid *GetRectilinearGridOutput();
  vtkStructuredPoints *GetStructuredPointsOutput();
  vtkImageData *GetImageDataOutput();
  
  // Description:
  // The matching OutputPort is specified by the output port's process
  // and a tag.  There can be more than one output port per process.
  // THE TAG MUST BE EVEN BECAUSE TWO RMIs ARE CREATED FROM IT!!!
  vtkSetMacro(RemoteProcessId, int);
  vtkGetMacro(RemoteProcessId, int);
  vtkSetMacro(Tag, int);
  vtkGetMacro(Tag, int);

  // See the vtkAlgorithm for a desciption of what these do
  int ProcessUpstreamRequest(vtkInformation *, vtkInformationVector *, 
                              vtkInformationVector *);
  int ProcessDownstreamRequest(vtkInformation *, vtkInformationVector *, 
                                vtkInformationVector *);

#ifdef VTK_USE_EXECUTIVES
  // Description:
  // Return the MTime also considering the other process
  unsigned long GetMTime();

#else
  // Description:
  // Need to override to propagate across port.
  void UpdateInformation();

  // Description:
  // Need to override to propagate across port
  void UpdateData( vtkDataObject *out );

  // Description:
  // Need to override to propagate across port.
  void PropagateUpdateExtent(vtkDataObject *vtkNotUsed(output)) {};

  // Description:
  // Need to override to trigger the update across the port
  void TriggerAsynchronousUpdate();  
  
#endif
  
  // Description:
  // Access to the controller used for communication.  By default, the
  // global controller is used.
  vtkMultiProcessController *GetController() {return this->Controller;}
  virtual void SetController(vtkMultiProcessController*);
  
  // Description:
  // If DoUpdateInformation if false (it is true by default),
  // UpdateInformation is not performed during Update. This can
  // be used to avoid  unnecessary communication once the data
  // has been transferred. However, if the pipeline changes
  // upstream, DoUpdateInformation has to be set to true again.
  // Otherwise, Updata will not occur.
  vtkSetMacro(DoUpdateInformation, int);
  vtkGetMacro(DoUpdateInformation, int);
  
//BTX

// Arbitrary tags used by the ports for communication.
  enum Tags {
    DOWN_DATA_TIME_TAG = 98970,
    UPDATE_EXTENT_TAG = 98971,
    TRANSFER_NEEDED_TAG = 98972,
    INFORMATION_TRANSFER_TAG = 98973,
    DATA_TRANSFER_TAG = 98974,
    NEW_DATA_TIME_TAG = 98975
  };

//ETX

protected:
  vtkInputPort();
  ~vtkInputPort();  
  
  vtkMultiProcessController *Controller;
  int RemoteProcessId;
  int Tag;

  unsigned long DataTime;
  unsigned long UpStreamMTime;
  int TransferNeeded;
  int DoUpdateInformation;

  int LastUpdatePiece;
  int LastUpdateNumberOfPieces;
  int LastUpdateGhostLevel;

  vtkSetVector6Macro(LastUpdateExtent, int);
  int LastUpdateExtent[6];

  int UpdateExtentIsOutsideOfTheExtent(vtkDataObject *output);

#ifdef VTK_USE_EXECUTIVES
  void ExecuteInformation();
  void ExecuteData(vtkDataObject *output);

  // see the vtkAlgorithm for what these methods do
  virtual int FillOutputPortInformation(int port, vtkInformation* info);
#endif

private:
  vtkInputPort(const vtkInputPort&);  // Not implemented.
  void operator=(const vtkInputPort&);  // Not implemented.
};

#endif


