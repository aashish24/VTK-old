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
// .NAME vtkCollectPolyData - Collect distributed polydata.
// .DESCRIPTION
// This filter has code to collect polydat from across processes onto node 0.
// This collection can be controlled by the size of the data.  If the
// final data size will be above the threshold, then it will not be collected.


#ifndef __vtkCollectPolyData_h
#define __vtkCollectPolyData_h

#include "vtkPolyDataToPolyDataFilter.h"

class vtkMultiProcessController;
class vtkSocketController;

class VTK_PARALLEL_EXPORT vtkCollectPolyData : public vtkPolyDataToPolyDataFilter
{
public:
  static vtkCollectPolyData *New();
  vtkTypeRevisionMacro(vtkCollectPolyData, vtkPolyDataToPolyDataFilter);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // By defualt this filter uses the global controller,
  // but this method can be used to set another instead.
  virtual void SetController(vtkMultiProcessController*);
  vtkGetObjectMacro(Controller, vtkMultiProcessController);

  // Description:
  // When this filter is being used in client-server mode,
  // this is the controller used to communicate between
  // client and server.  Client should not set the other controller.
  virtual void SetSocketController(vtkSocketController*);
  vtkGetObjectMacro(SocketController, vtkSocketController);

  // Description:
  // Threshold that determines whether data will be collected.
  // If the total size of the data in kilobytes is less than this threshold, 
  // then the data remains distributed.
  vtkSetMacro(Threshold, unsigned long);
  vtkGetMacro(Threshold, unsigned long);
  
  // Description:
  // This flag is set based on whether the data was collected to process 0 or not.
  vtkGetMacro(Collected, int);

  // Description:
  // Gets the total memory size in kBytes.  This is the sum from all processes.
  vtkGetMacro(MemorySize, unsigned long);

protected:
  vtkCollectPolyData();
  ~vtkCollectPolyData();

  // Data generation method
  void ComputeInputUpdateExtents(vtkDataObject *output);
  void ExecuteData(vtkDataObject*);
  void ExecuteInformation();

  unsigned long Threshold;
  unsigned long MemorySize;
  int Collected;

  vtkMultiProcessController *Controller;
  vtkSocketController *SocketController;

private:
  vtkCollectPolyData(const vtkCollectPolyData&); // Not implemented
  void operator=(const vtkCollectPolyData&); // Not implemented
};

#endif
