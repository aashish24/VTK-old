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
// .NAME vtkImageQuantizeRGBToIndex - generalized histograms up to 4 dimensions
// .SECTION Description
// vtkImageQuantizeRGBToIndex takes a 3 component RGB image as
// input and produces a one component index image as output, along with
// a lookup table that contains the color definitions for the index values.
// This filter works on the entire input extent - it does not perform
// streaming, and it does not supported threaded execution (because it has
// to process the entire image).
//
// To use this filter, you typically set the number of colors 
// (between 2 and 65536), execute it, and then retrieve the lookup table.
// The colors can then be using the lookup table and the image index.

#ifndef __vtkImageQuantizeRGBToIndex_h
#define __vtkImageQuantizeRGBToIndex_h

#include "vtkImageToImageFilter.h"

class vtkLookupTable;

class VTK_IMAGING_EXPORT vtkImageQuantizeRGBToIndex : public vtkImageToImageFilter
{
public:
  static vtkImageQuantizeRGBToIndex *New();
  vtkTypeRevisionMacro(vtkImageQuantizeRGBToIndex,vtkImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set / Get the number of color index values to produce - must be 
  // a number between 2 and 65536.
  vtkSetClampMacro( NumberOfColors, int, 2, 65536 );
  vtkGetMacro( NumberOfColors, int );

  // Description:
  // Get the resulting lookup table that contains the color definitions
  // corresponding to the index values in the output image.
  vtkGetObjectMacro( LookupTable, vtkLookupTable );

  vtkGetMacro( InitializeExecuteTime, float );
  vtkGetMacro( BuildTreeExecuteTime, float );
  vtkGetMacro( LookupIndexExecuteTime, float );

//BTX
  // Description: 
  // For internal use only - get the type of the image
  vtkGetMacro( InputType, int );

  // Description: 
  // For internal use only - set the times for execution
  vtkSetMacro( InitializeExecuteTime, float );
  vtkSetMacro( BuildTreeExecuteTime, float );
  vtkSetMacro( LookupIndexExecuteTime, float );
//ETX

protected:
  vtkImageQuantizeRGBToIndex();
  ~vtkImageQuantizeRGBToIndex();

  vtkLookupTable  *LookupTable;
  int             NumberOfColors;
  int             InputType;

  float           InitializeExecuteTime;
  float           BuildTreeExecuteTime;
  float           LookupIndexExecuteTime;

  void ExecuteInformation(vtkImageData *inData, vtkImageData *outData);
  void ComputeInputUpdateExtent(int inExt[6], int outExt[6]);
  void ExecuteInformation(){this->vtkImageToImageFilter::ExecuteInformation();};

  virtual void ExecuteData(vtkDataObject *out);
private:
  vtkImageQuantizeRGBToIndex(const vtkImageQuantizeRGBToIndex&);  // Not implemented.
  void operator=(const vtkImageQuantizeRGBToIndex&);  // Not implemented.
};

#endif








