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
// .NAME vtkPStreamTracer - Parallel streamline generator
// .SECTION Description
// .SECTION See Also
// vtkStreamTracer

#ifndef __vtkPStreamTracer_h
#define __vtkPStreamTracer_h

#include "vtkStreamTracer.h"

class vtkInterpolatedVelocityField;
class vtkMultiProcessController;

class VTK_GRAPHICS_EXPORT vtkPStreamTracer : public vtkStreamTracer
{
public:
  vtkTypeRevisionMacro(vtkPStreamTracer,vtkStreamTracer);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  static vtkPStreamTracer *New();

  // Description:
  // Set/Get the controller use in compositing (set to
  // the global controller by default)
  // If not using the default, this must be called before any
  // other methods.
  void SetController(vtkMultiProcessController* controller);
  vtkGetObjectMacro(Controller, vtkMultiProcessController);
  
protected:

  vtkPStreamTracer();
  ~vtkPStreamTracer();

  void Execute();
  void ExecuteInformation();
  void ComputeInputUpdateExtents( vtkDataObject *output );

  vtkMultiProcessController* Controller;

  vtkInterpolatedVelocityField* Interpolator;
  void SetInterpolator(vtkInterpolatedVelocityField*);

  void ForwardTask(float seed[3], 
                   int direction, int isNewSeed, int lastid, int currentLine);
  int ProcessTask(float seed[3], 
                  int direction, int isNewSeed, int lastid, int currentLine);
  int ReceiveAndProcessTask();

  vtkDataArray* Seeds;
  vtkIdList* SeedIds;
  vtkIntArray* IntegrationDirections;

private:
  vtkPStreamTracer(const vtkPStreamTracer&);  // Not implemented.
  void operator=(const vtkPStreamTracer&);  // Not implemented.
};


#endif


