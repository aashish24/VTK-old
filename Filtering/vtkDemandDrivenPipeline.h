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
// .NAME vtkDemandDrivenPipeline -
// .SECTION Description
// vtkDemandDrivenPipeline

#ifndef __vtkDemandDrivenPipeline_h
#define __vtkDemandDrivenPipeline_h

#include "vtkExecutive.h"

class vtkDataArray;
class vtkDataSetAttributes;
class vtkDemandDrivenPipelineInternals;
class vtkFieldData;
class vtkInformation;
class vtkInformationIntegerKey;
class vtkInformationVector;
class vtkInformationKeyVectorKey;
class vtkInformationUnsignedLongKey;

class VTK_FILTERING_EXPORT vtkDemandDrivenPipeline : public vtkExecutive
{
public:
  static vtkDemandDrivenPipeline* New();
  vtkTypeRevisionMacro(vtkDemandDrivenPipeline,vtkExecutive);
  void PrintSelf(ostream& os, vtkIndent indent);

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
  // Get the PipelineMTime for this exective.
  vtkGetMacro(PipelineMTime, unsigned long);

  // Description:
  // Set whether the given output port releases data when it is
  // consumed.  Returns 1 if the the value changes and 0 otherwise.
  virtual int SetReleaseDataFlag(int port, int n);

  // Description:
  // Get whether the given output port releases data when it is consumed.
  virtual int GetReleaseDataFlag(int port);

  static vtkInformationIntegerKey* REQUEST_DATA_OBJECT();
  static vtkInformationIntegerKey* REQUEST_INFORMATION();
  static vtkInformationIntegerKey* REQUEST_DATA();
  static vtkInformationIntegerKey* RELEASE_DATA();
  static vtkInformationIntegerKey* REQUEST_PIPELINE_MODIFIED_TIME();
  static vtkInformationUnsignedLongKey* PIPELINE_MODIFIED_TIME();

  virtual int UpdatePipelineMTime();
  virtual int UpdateDataObject();
  virtual int UpdateInformation();
  virtual int UpdateData(int outputPort);

  vtkDemandDrivenPipeline* GetConnectedInputExecutive(int port, int index);
  vtkInformation* GetConnectedInputInformation(int port, int index);

protected:
  vtkDemandDrivenPipeline();
  ~vtkDemandDrivenPipeline();

  virtual int ExecuteDataObject();
  virtual int ExecuteInformation();
  virtual int ExecuteData(int outputPort);

  // Copy information for the given request.
  virtual void CopyDefaultInformation(vtkInformation* request, int direction);

  // Reset the pipeline update values in the given output information object.
  virtual void ResetPipelineInformation(int port, vtkInformation*);

  virtual int CheckDataObject(int port);

  // Input connection validity checkers.
  int InputCountIsValid();
  int InputCountIsValid(int port);
  int InputTypeIsValid();
  int InputTypeIsValid(int port);
  int InputTypeIsValid(int port, int index);
  int InputFieldsAreValid();
  int InputFieldsAreValid(int port);
  int InputFieldsAreValid(int port, int index);

  // Field existence checkers.
  int DataSetAttributeExists(vtkDataSetAttributes* dsa, vtkInformation* field);
  int FieldArrayExists(vtkFieldData* data, vtkInformation* field);
  int ArrayIsValid(vtkDataArray* array, vtkInformation* field);

  // Input port information checkers.
  int InputIsOptional(int port);
  int InputIsRepeatable(int port);

  vtkDataObject* NewDataObject(const char* type);

  // Decide whether the output data need to be generated.
  virtual int NeedToExecuteData(int outputPort);

  // Largest MTime of any algorithm on this executive or preceding
  // executives.
  unsigned long PipelineMTime;

  // Time when information or data were last generated.
  vtkTimeStamp DataObjectTime;
  vtkTimeStamp InformationTime;
  vtkTimeStamp DataTime;

private:
  vtkDemandDrivenPipeline(const vtkDemandDrivenPipeline&);  // Not implemented.
  void operator=(const vtkDemandDrivenPipeline&);  // Not implemented.
};

#endif
