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
// .NAME vtkCompositeDataPipeline - Executive supporting composite datasets.
// .SECTION Description
// vtkCompositeDataPipeline is an executive that supports the processing of
// composite dataset. It supports algorithms that are aware of composite
// dataset as well as those that are not. Portions of the pipeline that are
// note composite dataset-aware are looped by the next consumer that is
// composite dataset-aware. Type checking is performed at run
// time. Algorithms that are not composite dataset-aware have to support
// all dataset types contained in the composite dataset. The pipeline
// execution can be summarized as follows: 
//
// * REQUEST_INFORMATION: The producers have to provide information about
// the contents of the composite dataset in this pass. This is accomplished
// by creating and populating a vtkHierarchicalDataInformation and setting
// it using the COMPOSITE_DATA_INFORMATION() key in the output information
// vector. Sources that can produce more than one piece (note that a piece is
// different than a block; each piece consistes of 0 or more blocks) should
// set MAXIMUM_NUMBER_OF_PIECES to -1.
//
// * REQUEST_UPDATE_EXTENT: This pass is identical to the one implemented
// in vtkStreamingDemandDrivenPipeline
//
// * BEGIN_LOOP: The source is told that looping is about to start. 
// The source has to perform  "extent translation". This is the process 
// by which the piece request is converted to a block request. 
// This is done by adding a MARKED_FOR_UPDATE() key to the appropriate blocks 
// in UPDATE_BLOCKS().
//
// * REQUEST_DATA: This is where the algorithms execute. If a composite
// data algorithm is consuming the output of a simple data algorithm, the
// executive will execute the streaming demand driven pipeline passes for
// each block:
// @verbatim
// for each block
//    REQUEST_PIPELINE_MODIFIED_TIME()
//    REQUEST_DATA_OBJECT()
//    REQUEST_INFORMATION()
//    REQUEST_DATA()
// @endverbatim
// The request passed to these passes will contain a LEVEL() and INDEX() key
// of each block to be updated. 
// Shallow copies of individual blocks are added to the composite input
// of the algorithm. Finally, the request is passed to the algorithm.
// If the algorithm it points to is simple, the executive will also call
// it on each block and collect the results as the output.
// Furthermore, if the vtkCompositeDataPipeline is assigned to a simple filter, 
// it will invoke the  vtkStreamingDemandDrivenPipeline passes in a loop, 
// passing a different block each time and will collect the results in a 
// composite dataset (vtkHierarchicalDataSet).
// .SECTION See also
//  vtkHierarchicalDataInformation vtkCompositeDataSet vtkHierarchicalDataSet

#ifndef __vtkCompositeDataPipeline_h
#define __vtkCompositeDataPipeline_h

#include "vtkStreamingDemandDrivenPipeline.h"

class vtkInformationDoubleKey;
class vtkInformationIntegerVectorKey;
class vtkInformationObjectBaseKey;
class vtkInformationStringKey;
class vtkInformationDataObjectKey;

class VTK_FILTERING_EXPORT vtkCompositeDataPipeline : public vtkStreamingDemandDrivenPipeline
{
public:
  static vtkCompositeDataPipeline* New();
  vtkTypeRevisionMacro(vtkCompositeDataPipeline,vtkStreamingDemandDrivenPipeline);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Generalized interface for asking the executive to fullfill update
  // requests.
  virtual int ProcessRequest(vtkInformation* request);

  // Description:
  // Returns the data object stored with the COMPOSITE_DATA_SET() in the
  // output port
  vtkDataObject* GetCompositeOutputData(int port);

  // Description:
  // vtkCompositeDataPipeline specific keys
  static vtkInformationIntegerKey*       BEGIN_LOOP();
  static vtkInformationIntegerKey*       END_LOOP();
  static vtkInformationStringKey*        COMPOSITE_DATA_TYPE_NAME();
  static vtkInformationObjectBaseKey*    COMPOSITE_DATA_INFORMATION();
  static vtkInformationIntegerKey*       MARKED_FOR_UPDATE();
  static vtkInformationStringKey*        INPUT_REQUIRED_COMPOSITE_DATA_TYPE();
  static vtkInformationObjectBaseKey*    UPDATE_BLOCKS();

protected:
  vtkCompositeDataPipeline();
  ~vtkCompositeDataPipeline();

  virtual int ForwardUpstream(vtkInformation* request);
  virtual int ForwardUpstream(int i, int j, vtkInformation* request);

  // Copy information for the given request.
  virtual void CopyDefaultInformation(vtkInformation* request, int direction);

  virtual void CopyFromDataToInformation(
    vtkDataObject* dobj, vtkInformation* inInfo);
  virtual void PushInformation(vtkInformation*);
  virtual void PopInformation (vtkInformation*);

  // Composite data pipeline times. Similar to superclass'
  vtkTimeStamp SubPassTime;

  // If true, the producer is being driven in a loop (dumb filters
  // between composite consumer and producer)
  int InSubPass;

  virtual int ExecuteDataObjectForBlock(vtkInformation* request);
  virtual int ExecuteDataObject(vtkInformation* request);

  virtual int ExecuteInformationForBlock(vtkInformation* request);
  virtual int ExecuteInformation(vtkInformation* request);

  virtual int ExecuteDataForBlock(vtkInformation* request);
  virtual int ExecuteData(vtkInformation* request);

  int CheckCompositeData(int port);
  int SendEndLoop(int i, int j);

  vtkInformation* InformationCache;

private:
  vtkCompositeDataPipeline(const vtkCompositeDataPipeline&);  // Not implemented.
  void operator=(const vtkCompositeDataPipeline&);  // Not implemented.
};

#endif
