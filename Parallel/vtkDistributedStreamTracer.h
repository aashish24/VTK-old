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
// .NAME vtkDistributedStreamTracer - Distributed streamline generator
// .SECTION Description
// This filter integrates streamlines on a distributed dataset. It is
// essentially a serial algorithm: only one process is active at one
// time and it is not more efficient than a single process integration.
// It is useful when the data is too large to be on one process and
// has to be kept distributed. See vtkMPIStreamTracer for a distributed
// parallel implementation. 
// .SECTION See Also
// vtkStreamTracer vtkPStreamTracer vtkMPIStreamTracer

#ifndef __vtkDistributedStreamTracer_h
#define __vtkDistributedStreamTracer_h

#include "vtkPStreamTracer.h"

class VTK_PARALLEL_EXPORT vtkDistributedStreamTracer : public vtkPStreamTracer
{
public:
  vtkTypeRevisionMacro(vtkDistributedStreamTracer,vtkPStreamTracer);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  static vtkDistributedStreamTracer *New();

protected:

  vtkDistributedStreamTracer();
  ~vtkDistributedStreamTracer();

  void ForwardTask(float seed[3], 
                   int direction, 
                   int isNewSeed, 
                   int lastid, 
                   int lastCellId,
                   int currentLine,
                   float* firstNormal);
  int ProcessTask(float seed[3], 
                  int direction, 
                  int isNewSeed, 
                  int lastid, 
                  int lastCellId,
                  int currentLine,
                  float* firstNormal);
  int ProcessNextLine(int currentLine);
  int ReceiveAndProcessTask();

  virtual void ParallelIntegrate();

private:
  vtkDistributedStreamTracer(const vtkDistributedStreamTracer&);  // Not implemented.
  void operator=(const vtkDistributedStreamTracer&);  // Not implemented.
};


#endif


