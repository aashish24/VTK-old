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
// .NAME vtkImageAlgorithm - Generic algorithm superclass for image algs
// .SECTION Description
// vtkImageToImageAlgorithm is a filter superclass that hides much of the 
// pipeline  complexity. It handles breaking the pipeline execution 
// into smaller extents so that the vtkImageData limits are observed. It 
// also provides support for multithreading. If you don't need any of this
// functionality, consider using vtkSimpleImageToImageAlgorithm instead.
// .SECTION See also
// vtkSimpleImageToImageAlgorithm

#ifndef __vtkImageAlgorithm_h
#define __vtkImageAlgorithm_h

#include "vtkAlgorithm.h"
#include "vtkImageData.h" // makes things a bit easier

class vtkDataSet;
class vtkImageData;

class VTK_FILTERING_EXPORT vtkImageAlgorithm : public vtkAlgorithm
{
public:
  vtkTypeRevisionMacro(vtkImageAlgorithm,vtkAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the output data object for a port on this algorithm.
  vtkImageData* GetOutput();
  vtkImageData* GetOutput(int);

  // Description:
  // see vtkAlgorithm for details
  virtual int ProcessRequest(vtkInformation*,
                             vtkInformationVector**,
                             vtkInformationVector*);

  // Description:
  // Set an input of this algorithm.
  virtual void SetInput(vtkDataObject *);
  virtual void SetInput(int, vtkDataObject*);

  // Description:
  // Add an input of this algorithm.
  virtual void AddInput(vtkDataObject *);
  virtual void AddInput(int, vtkDataObject*);

protected:
  vtkImageAlgorithm();
  ~vtkImageAlgorithm();

  // convinience method
  virtual void ExecuteInformation(vtkInformation* request,
                                  vtkInformationVector** inputVector,
                                  vtkInformationVector* outputVector);
  virtual void RequestUpdateExtent(vtkInformation*,
                                   vtkInformationVector**,
                                   vtkInformationVector*);

  // This is called by the superclass.
  // This is the method you should override.
  virtual void RequestData(vtkInformation *request,
                           vtkInformationVector** inputVector,
                           vtkInformationVector* outputVector);

  // Description:
  // This method is the old style execute method
  virtual void ExecuteData(vtkDataObject *output);
  virtual void Execute();

  // just allocate the output data
  virtual void AllocateOutputData(vtkImageData *out, 
                                  int *uExtent);
  virtual vtkImageData *AllocateOutputData(vtkDataObject *out);

  // copy the other point and cell data
  virtual void CopyAttributeData(vtkImageData *in, vtkImageData *out);
  
  // see algorithm for more info
  virtual int FillOutputPortInformation(int port, vtkInformation* info);
  virtual int FillInputPortInformation(int port, vtkInformation* info);
  
  char *InputScalarsSelection;
  vtkSetStringMacro(InputScalarsSelection);

private:
  vtkImageAlgorithm(const vtkImageAlgorithm&);  // Not implemented.
  void operator=(const vtkImageAlgorithm&);  // Not implemented.
};

#endif







