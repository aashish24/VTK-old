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
/*-------------------------------------------------------------------------
  Copyright (c) 2008, 2009 by SCI Institute, University of Utah.
  
  This is part of the Parallel Dataflow System originally developed by
  Huy T. Vo and Claudio T. Silva. For more information, see:

  "Parallel Dataflow Scheme for Streaming (Un)Structured Data" by Huy
  T. Vo, Daniel K. Osmari, Brian Summa, Joao L.D. Comba, Valerio
  Pascucci and Claudio T. Silva, SCI Institute, University of Utah,
  Technical Report #UUSCI-2009-004, 2009.

  "Multi-Threaded Streaming Pipeline For VTK" by Huy T. Vo and Claudio
  T. Silva, SCI Institute, University of Utah, Technical Report
  #UUSCI-2009-005, 2009.
-------------------------------------------------------------------------*/
// .NAME vtkThreadedStreamingPipeline - Executive supporting multi-threads
// .SECTION Description
// vtkThreadeStreamingDemandDrivenPipeline is an executive that supports
// updating input ports based on the number of threads available.

#ifndef __vtkThreadedStreamingPipeline_h
#define __vtkThreadedStreamingPipeline_h

#include "vtkCompositeDataPipeline.h"
#include "vtkSmartPointer.h"
#include "vtkThreadedStreamingTypes.i"

class vtkDoubleArray;
class vtkComputingResources;
class vtkExecutionScheduler;
class vtkFloatArray;
class vtkIntArray;
class vtkMutexLock;
class vtkMultiThreader;
class vtkThreadMessager;

class VTK_FILTERING_EXPORT vtkThreadedStreamingPipeline : public vtkCompositeDataPipeline
{
public:
  static vtkThreadedStreamingPipeline* New();
  vtkTypeRevisionMacro(vtkThreadedStreamingPipeline,vtkCompositeDataPipeline);

 // Description:
  // Key to store the priority of a task
  static vtkInformationIntegerKey* AUTO_PROPAGATE();
  static vtkInformationObjectBaseKey* EXTRA_INFORMATION();

  // Description:
  // Definition of different types of processing units an algorithm
  // can be executed
  //BTX
  enum
  {
    PROCESSING_UNIT_NONE = 0,
    PROCESSING_UNIT_CPU = 1,
    PROCESSING_UNIT_GPU = 2
  };
  //ETX

  // Description:
  // Enable/Disable Multi-Threaded updating mechanism
  static void SetMultiThreadedEnabled(bool enabled);

  // Description:
  // Enable/Disable automatic propagation of Push events
  static void SetAutoPropagatePush(bool enabled);

  //BTX
  // Convinient definitions of vector/set of vtkExecutive
  class vtkExecutiveHasher {
  public:
    size_t operator()(const vtkExecutive* e) const {
      return (size_t)e;
    };
  };
  typedef vtksys::hash_set<vtkExecutive*, vtkExecutiveHasher> vtkExecutiveSet;
  typedef vtkstd::vector<vtkExecutive*>                       vtkExecutiveVector;
  
  // Description:
  // Trigger the updates on certain execs and asking all of its
  // upstream modules to be updated as well (propagate up)
  static void Pull(const vtkExecutiveVector &execs, vtkInformation *info=NULL);
  
  // Description:
  // Trigger the updates on certain execs and asking all of its
  // downstream modules to be updated as well (propagate down)  
  static void Push(const vtkExecutiveVector &execs, vtkInformation *info=NULL);
  //ETX
  
  // Description:
  // A simplified version of the above and also allow wrapping
  static void Pull(vtkExecutive *exec, vtkInformation *info=NULL);
  static void Push(vtkExecutive *exec, vtkInformation *info=NULL);

  // Description:  
  // Pull/Push on a module triggers upstream/downsteram modules to
  // update but not includingitself  
  void Pull(vtkInformation *info=NULL);
  void Push(vtkInformation *info=NULL);
  
  // Description:
  // Release all the locks for input ports living upstream 
  void ReleaseInputs();

  // Description:
  // Generalized interface for asking the executive to fullfill update
  // requests.
  virtual int ProcessRequest(vtkInformation* request,
                             vtkInformationVector** inInfo,
                             vtkInformationVector* outInfo);

  // Description:
  // Send a direct REQUEST_DATA (on all ports) to this executive
  int ForceUpdateData(int processingUnit, vtkInformation *info);

  // Description:
  // Update the LastDataRequestTimeFromSource using its upstream time
  void UpdateRequestDataTimeFromSource();

  // Description:
  // Return the scheduling for this executive
  vtkComputingResources *GetResources();

  float                  LastDataRequestTime;
  float                  LastDataRequestTimeFromSource;
  vtkInformation        *ForceDataRequest;
  vtkComputingResources *Resources;
  vtkExecutionScheduler *Scheduler;
  
protected:
  vtkThreadedStreamingPipeline();
  ~vtkThreadedStreamingPipeline();

  virtual int ForwardUpstream(vtkInformation* request);
  
private:
  vtkThreadedStreamingPipeline(const vtkThreadedStreamingPipeline&);  // Not implemented.
  void operator=(const vtkThreadedStreamingPipeline&);  // Not implemented.
};

#endif
