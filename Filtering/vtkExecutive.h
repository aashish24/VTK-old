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
// .NAME vtkExecutive - Superclass for all pipeline executives in VTK.
// .SECTION Description
// vtkExecutive is the superclass for all pipeline executives in VTK.
// A VTK executive is responsible for controlling one instance of
// vtkAlgorithm.  A pipeline consists of one or more executives that
// control data flow.  Every reader, source, writer, or data
// processing algorithm in the pipeline is implemented in an instance
// of vtkAlgorithm.

#ifndef __vtkExecutive_h
#define __vtkExecutive_h

#include "vtkObject.h"

class vtkAlgorithm;
class vtkAlgorithmOutput;
class vtkAlgorithmToExecutiveFriendship;
class vtkDataObject;
class vtkExecutiveInternals;
class vtkInformation;
class vtkInformationExecutiveKey;
class vtkInformationIntegerKey;
class vtkInformationVector;

class VTK_FILTERING_EXPORT vtkExecutive : public vtkObject
{
public:
  vtkTypeRevisionMacro(vtkExecutive,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the algorithm to which this executive has been assigned.
  vtkAlgorithm* GetAlgorithm();

  // Description:
  // Generalized interface for asking the executive to fullfill update
  // requests.
  virtual int ProcessRequest(vtkInformation* request);

  // Description:
  // Bring the algorithm's outputs up-to-date.  Returns 1 for success
  // and 0 for failure.
  virtual int Update();
  virtual int Update(int port);

  // Description:
  // Get the pipeline information object for the given output port.
  virtual vtkInformation* GetOutputInformation(int port);

  // Description:
  // Get the pipeline information for the given input connection.
  vtkInformation* GetInputInformation(int port, int connection);

  // Description:
  // Get the executive managing the given input connection.
  vtkExecutive* GetInputExecutive(int port, int connection);

  // Description:
  // Get/Set the data object for an output port of the algorithm.
  virtual vtkDataObject* GetOutputData(int port);
  virtual void SetOutputData(int port, vtkDataObject*);

  // Description:
  // Get the data object for an output port of the algorithm.
  virtual vtkDataObject* GetInputData(int port, int connection);

  // Description:
  // Get the output port that produces the given data object.
  virtual vtkAlgorithmOutput* GetProducerPort(vtkDataObject*);

  // Description:
  // Decrement the count of references to this object and participate
  // in garbage collection.
  virtual void UnRegister(vtkObjectBase* o);

  // Description:
  // Information key to store a pointer to an executive in an
  // information object.
  static vtkInformationExecutiveKey* EXECUTIVE();

  // Description:
  // Information key to store a port number in an information object.
  static vtkInformationIntegerKey* PORT_NUMBER();

  // Description:
  // Information key to store the output port number from which a
  // request is made.
  static vtkInformationIntegerKey* FROM_OUTPUT_PORT();

  // Description:
  // Keys to program vtkExecutive::ProcessRequest with the default
  // behavior for unknown requests.
  static vtkInformationIntegerKey* ALGORITHM_BEFORE_FORWARD();
  static vtkInformationIntegerKey* ALGORITHM_AFTER_FORWARD();
  static vtkInformationIntegerKey* ALGORITHM_DIRECTION();
  static vtkInformationIntegerKey* FORWARD_DIRECTION();
  //BTX
  enum { RequestUpstream, RequestDownstream };
  //ETX

protected:
  vtkExecutive();
  ~vtkExecutive();

  // Helper methods for subclasses.
  int InputPortIndexInRange(int port, const char* action);
  int OutputPortIndexInRange(int port, const char* action);

  // Get the number of input/output ports for the algorithm.  Returns
  // 0 if no algorithm is set.
  int GetNumberOfInputPorts();
  int GetNumberOfOutputPorts();

  // Access methods to arguments passed to vtkAlgorithm::ProcessRequest.
  vtkInformationVector** GetInputInformation();
  vtkInformationVector* GetOutputInformation();

  int CheckAlgorithm(const char* method);

  virtual int ForwardDownstream(vtkInformation* request);
  virtual int ForwardUpstream(vtkInformation* request);
  virtual void CopyDefaultInformation(vtkInformation* request, int direction);
  virtual int CallAlgorithm(vtkInformation* request, int direction);

  // Reset the pipeline update values in the given output information object.
  virtual void ResetPipelineInformation(int port, vtkInformation*)=0;

  // Bring the existence of output data objects up to date.
  virtual int UpdateDataObject()=0;

  // Bring the input information up to date with current connections.
  void UpdateInputInformationVector();

  // Garbage collection support.
  virtual void GarbageCollectionStarting();
  int GarbageCollecting;
  virtual void ReportReferences(vtkGarbageCollector*);
  virtual void RemoveReferences();

  virtual void SetAlgorithm(vtkAlgorithm* algorithm);

  // The algorithm managed by this executive.
  vtkAlgorithm* Algorithm;

  // Flag set when the algorithm is processing a request.
  int InAlgorithm;

private:
  vtkExecutiveInternals* ExecutiveInternal;

  //BTX
  friend class vtkAlgorithmToExecutiveFriendship;
  //ETX
private:
  vtkExecutive(const vtkExecutive&);  // Not implemented.
  void operator=(const vtkExecutive&);  // Not implemented.
};

#endif
