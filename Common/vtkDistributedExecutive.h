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
// .NAME vtkDistributedExecutive - Superclass for distributed executives.
// .SECTION Description
// vtkDistributedExecutive is the superclass for VTK's distributed
// executives.  Some pipeline architectures are more easily maintained
// with an executive instance per algorithm (vtkAlgorithm instance).
// This class references algorithms in a distributed manner so
// subclasses can focus on their pipeline update designs.

#ifndef __vtkDistributedExecutive_h
#define __vtkDistributedExecutive_h

#include "vtkExecutive.h"

class VTK_COMMON_EXPORT vtkDistributedExecutive : public vtkExecutive
{
public:
  static vtkDistributedExecutive* New();
  vtkTypeRevisionMacro(vtkDistributedExecutive,vtkExecutive);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Distributed executives have a one-to-one correspondence with
  // their algorithms.  Get the algorithm to which this executive has
  // been assigned.
  vtkAlgorithm* GetAlgorithm();

  // Description:
  // Bring the given algorithm's outputs up-to-date.  The algorithm
  // must already be managed by this executive.  Returns 1 for success
  // and 0 for failure.
  virtual int Update();
  virtual int Update(int port);
  virtual int Update(vtkAlgorithm* algorithm);
  virtual int Update(vtkAlgorithm* algorithm, int port);

  // Description:
  // Get the information object for an output port of an algorithm.
  virtual vtkInformation* GetOutputInformation(int port);
  virtual vtkInformation* GetOutputInformation(vtkAlgorithm* algorithm,
                                               int port);

  // Description:
  // Get the data object for an output port of an algorithm.
  virtual vtkDataObject* GetOutputData(int port);
  virtual vtkDataObject* GetOutputData(vtkAlgorithm* algorithm, int port);
protected:
  vtkDistributedExecutive();
  ~vtkDistributedExecutive();

  // Helper methods for subclasses.
  int InputPortIndexInRange(int port, const char* action);
  int OutputPortIndexInRange(int port, const char* action);

  // Implement methods required by superclass.
  virtual void AddAlgorithm(vtkAlgorithm* algorithm);
  virtual void RemoveAlgorithm(vtkAlgorithm* algorithm);
  virtual void ReportReferences(vtkGarbageCollector*);
  virtual void RemoveReferences();

  // A distributed executive manages at most one algorithm.
  virtual void SetAlgorithm(vtkAlgorithm* algorithm);
  vtkAlgorithm* Algorithm;

  // Get/Set the output data for an output port on an algorithm.
  virtual void SetOutputDataInternal(vtkAlgorithm* algorithm, int port,
                                     vtkDataObject* output);
  virtual vtkDataObject* GetOutputDataInternal(vtkAlgorithm* algorithm,
                                               int port);
private:
  vtkDistributedExecutive(const vtkDistributedExecutive&);  // Not implemented.
  void operator=(const vtkDistributedExecutive&);  // Not implemented.
};

#endif
