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
// .NAME vtkAlgorithm - Superclass for all sources, filters, and sinks in VTK.
// .SECTION Description
// vtkAlgorithm is the superclass for all sources, filters, and sinks
// in VTK.  It defines a generalized interface for executing data
// processing algorithms.
//
// Instances may be used independently or within pipelines with a
// variety of architectures and update mechanisms.  Pipelines are
// controlled by instances of vtkExecutive.  Every vtkAlgorithm
// instance has an associated vtkExecutive when it is used in a
// pipeline.  The executive is responsible for data flow.

#ifndef __vtkAlgorithm_h
#define __vtkAlgorithm_h

#include "vtkObject.h"

class vtkAlgorithmInternals;
class vtkAlgorithmOutput;
class vtkDataObject;
class vtkExecutive;
class vtkInformation;
class vtkInformationInformationVectorKey;
class vtkInformationIntegerKey;
class vtkInformationStringKey;
class vtkInformationVector;

class VTK_FILTERING_EXPORT vtkAlgorithm : public vtkObject
{
public:
  static vtkAlgorithm *New();
  vtkTypeRevisionMacro(vtkAlgorithm,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Check whether this algorithm has an assigned executive.  This
  // will NOT create a default executive.
  int HasExecutive();

  // Description:
  // Get this algorithm's executive.  If it has none, a default
  // executive will be created.
  vtkExecutive* GetExecutive();

  // Description:
  // Set this algorithm's executive.  This algorithm is removed from
  // any executive to which it has previously been assigned and then
  // assigned to the given executive.
  virtual void SetExecutive(vtkExecutive* executive);

  // Description:
  // Upstream/Downstream requests form the generalized interface
  // through which executives invoke a algorithm's functionality.
  // Upstream requests correspond to information flow from the
  // algorithm's outputs to its inputs.  Downstream requests
  // correspond to information flow from the algorithm's inputs to its
  // outputs.
  //
  // A downstream request is defined by the contents of the request
  // information object.  The input to the request is stored in the
  // input information vector passed to ProcessRequest.  The results
  // of an downstream request are stored in the output information
  // vector passed to ProcessRequest.
  //
  // An upstream request is defined by the contents of the request
  // information object.  The input to the request is stored in the
  // output information vector passed to ProcessRequest.  The results
  // of an upstream request are stored in the input information vector
  // passed to ProcessRequest.
  virtual int ProcessRequest(vtkInformation* request,
                             vtkInformationVector** inInfo,
                             vtkInformationVector* outInfo);

  // Description:
  // Get the information object associated with an input port.  There
  // is one input port per kind of input to the algorithm.  Each input
  // port tells executives what kind of data and downstream requests
  // this algorithm can handle for that input.
  vtkInformation* GetInputPortInformation(int port);

  // Description:
  // Get the information object associated with an output port.  There
  // is one output port per output from the algorithm.  Each output
  // port tells executives what kind of upstream requests this
  // algorithm can handle for that output.
  vtkInformation* GetOutputPortInformation(int port);

  // Description:
  // Set/Get the information object associated with this algorithm.
  vtkGetObjectMacro(Information, vtkInformation);
  virtual void SetInformation(vtkInformation*);

  // Description:
  // Remove all the input data.
  void RemoveAllInputs();

  // Description:
  // Get the data object that will contain the algorithm output for
  // the given port.
  vtkDataObject* GetOutputDataObject(int port);

  // Description:
  // Set the connection for the given input port index.  Removes
  // any other connections.
  virtual void SetInputConnection(int port, vtkAlgorithmOutput* input);

  // Description:
  // Add a connection to the given input port index.
  virtual void AddInputConnection(int port, vtkAlgorithmOutput* input);

  // Description:
  // Remove a connection from the given input port index.
  virtual void RemoveInputConnection(int port, vtkAlgorithmOutput* input);

  // Description:
  // Get a proxy object corresponding to the given output port of this
  // algorithm.  The proxy object can be passed to another algorithm's
  // InputConnection methods to modify pipeline connectivity.
  vtkAlgorithmOutput* GetOutputPort(int index);

  // Description:
  // Get the number of input ports used by the algorithm.
  int GetNumberOfInputPorts();

  // Description:
  // Get the number of output ports provided by the algorithm.
  int GetNumberOfOutputPorts();

  // Description:
  // Get the number of input currently connected to a port.
  int GetNumberOfInputConnections(int port);

  // Description:
  // Get the total number of inputs for this algorithm
  int GetTotalNumberOfInputConnections();

  // Description:
  // Get the algorithm output port connected to an input port.
  vtkAlgorithmOutput* GetInputConnection(int port, int index);

  // Description:
  // Bring this algorithm's outputs up-to-date.
  virtual void Update();

  // Description:
  // Backward compatibility method to invoke UpdateInformation on executive.
  virtual void UpdateInformation();

  // Description:
  // Bring this algorithm's outputs up-to-date.
  virtual void UpdateWholeExtent();

  // Description:
  // Participate in garbage collection.
  virtual void Register(vtkObjectBase* o);
  virtual void UnRegister(vtkObjectBase* o);

  // Description:
  // Set/Get the AbortExecute flag for the process object. Process objects
  // may handle premature termination of execution in different ways.
  vtkSetMacro(AbortExecute,int);
  vtkGetMacro(AbortExecute,int);
  vtkBooleanMacro(AbortExecute,int);

  // Description:
  // Set/Get the execution progress of a process object.
  vtkSetClampMacro(Progress,double,0.0,1.0);
  vtkGetMacro(Progress,double);

  // Description:
  // Update the progress of the process object. If a ProgressMethod exists,
  // executes it.  Then set the Progress ivar to amount. The parameter amount
  // should range between (0,1).
  void UpdateProgress(double amount);

  // Description:
  // Set the current text message associated with the progress state.
  // This may be used by a calling process/GUI.
  vtkSetStringMacro(ProgressText);
  vtkGetStringMacro(ProgressText);

  // Description:
  // The error code contains a possible error that occured while
  // reading or writing the file.
  vtkGetMacro( ErrorCode, unsigned long );

  // left public for performance since it is used in inner loops
  int AbortExecute;

  // Description:
  // Keys used to specify input port requirements.
  static vtkInformationIntegerKey* INPUT_IS_OPTIONAL();
  static vtkInformationIntegerKey* INPUT_IS_REPEATABLE();
  static vtkInformationInformationVectorKey* INPUT_REQUIRED_FIELDS();
  static vtkInformationStringKey* INPUT_REQUIRED_DATA_TYPE();

  // Description:
  // Conviniance routine to convert from a linrar ordering of input
  // connections to a port, connecction pair
  void ConvertTotalInputToPortConnection(int ind, int &port, int &conn);
  

  //======================================================================
  //The followling block of code is to support old style VTK applications. If
  //you are using these calls there are better ways to do it in the new
  //pipeline
  //======================================================================
  
  // Description:
  // Turn release data flag on or off for all output ports.
  virtual void SetReleaseDataFlag(int);
  virtual int GetReleaseDataFlag();
  void ReleaseDataFlagOn();
  void ReleaseDataFlagOff();

  //========================================================================
  
protected:
  vtkAlgorithm();
  ~vtkAlgorithm();

  // Arbitrary extra information associated with this algorithm
  vtkInformation* Information;

  // Description:
  // Fill the input port information objects for this algorithm.  This
  // is invoked by the first call to GetInputPortInformation for each
  // port so subclasses can specify what they can handle.
  virtual int FillInputPortInformation(int port, vtkInformation* info);

  // Description:
  // Fill the output port information objects for this algorithm.
  // This is invoked by the first call to GetOutputPortInformation for
  // each port so subclasses can specify what they can handle.
  virtual int FillOutputPortInformation(int port, vtkInformation* info);

  // Description:
  // Set the number of input ports used by the algorithm.
  virtual void SetNumberOfInputPorts(int n);

  // Description:
  // Set the number of output ports provided by the algorithm.
  virtual void SetNumberOfOutputPorts(int n);

  // Helper methods to check input/output port index ranges.
  int InputPortIndexInRange(int index, const char* action);
  int OutputPortIndexInRange(int index, const char* action);

  // Description:
  // Replace the Nth connection on the given input port.  This should
  // not be used to remove an input, so the pointer cannot be NULL.
  // For use only by subclasses.
  void SetNthInputConnection(int port, int index, vtkAlgorithmOutput* input);

  // Create a default executive.
  virtual vtkExecutive* CreateDefaultExecutive();

  // Description:
  // The error code contains a possible error that occured while
  // reading or writing the file.
  vtkSetMacro( ErrorCode, unsigned long );
  unsigned long ErrorCode;

  // Progress/Update handling
  double Progress;
  char  *ProgressText;

  // Garbage collection support.
  virtual void ReportReferences(vtkGarbageCollector*);
private:
  vtkAlgorithmInternals* AlgorithmInternal;
  static void ConnectionAdd(vtkAlgorithm* producer, int producerPort,
                            vtkAlgorithm* consumer, int consumerPort);
  static void ConnectionRemove(vtkAlgorithm* producer, int producerPort,
                               vtkAlgorithm* consumer, int consumerPort);
  static void ConnectionRemoveAllInput(vtkAlgorithm* consumer, int port);
  static void ConnectionRemoveAllOutput(vtkAlgorithm* producer, int port);

private:
  vtkAlgorithm(const vtkAlgorithm&);  // Not implemented.
  void operator=(const vtkAlgorithm&);  // Not implemented.
};

#endif
