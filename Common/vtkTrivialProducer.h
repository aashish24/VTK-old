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
// .NAME vtkTrivialProducer - Producer for stand-alone data objects.
// .SECTION Description
// vtkTrivialProducer allows stand-alone data objects to be connected
// as inputs in a pipeline.  All data objects that are connected to a
// pipeline involving vtkAlgorithm must have a producer.  This trivial
// producer allows data objects that are hand-constructed in a program
// without another vtk producer to be connected.

#ifndef __vtkTrivialProducer_h
#define __vtkTrivialProducer_h

#include "vtkAlgorithm.h"

class vtkCallbackCommand;
class vtkDataObject;

class VTK_COMMON_EXPORT vtkTrivialProducer : public vtkAlgorithm
{
public:
  static vtkTrivialProducer *New();
  vtkTypeRevisionMacro(vtkTrivialProducer,vtkAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Process upstream/downstream requests trivially.  The associated
  // output data object is never modified, but it is queried to
  // fulfill requests.
  virtual int ProcessUpstreamRequest(vtkInformation*,
                                     vtkInformationVector*,
                                     vtkInformationVector*);
  virtual int ProcessDownstreamRequest(vtkInformation*,
                                       vtkInformationVector*,
                                       vtkInformationVector*);

  // Description:
  // Set the data object that is "produced" by this producer.  It is
  // never really modified.
  virtual void SetOutput(vtkDataObject* output);
protected:
  vtkTrivialProducer();
  ~vtkTrivialProducer();

  virtual int FillInputPortInformation(int, vtkInformation*);
  virtual int FillOutputPortInformation(int, vtkInformation*);
  virtual vtkExecutive* CreateDefaultExecutive();

  // The real data object.
  vtkDataObject* Output;

  // The observer to report when the internal object is modified.
  vtkCallbackCommand* ModifiedObserver;

  // Callback registered with the ModifiedObserver.
  static void ModifiedCallbackFunction(vtkObject*, unsigned long, void*,
                                       void*);
  // Modified callback from output data object.
  virtual void ModifiedCallback();

  virtual void ReportReferences(vtkGarbageCollector*);
  virtual void RemoveReferences();
private:
  vtkTrivialProducer(const vtkTrivialProducer&);  // Not implemented.
  void operator=(const vtkTrivialProducer&);  // Not implemented.
};

#endif
