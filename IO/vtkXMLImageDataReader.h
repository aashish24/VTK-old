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
// .NAME vtkXMLImageDataReader - Read VTK XML ImageData files.
// .SECTION Description
// vtkXMLImageDataReader reads the VTK XML ImageData file format.  One
// image data file can be read to produce one output.  Streaming is
// supported.  The standard extension for this reader's file format is
// "vti".  This reader is also used to read a single piece of the
// parallel file format.

// .SECTION See Also
// vtkXMLPImageDataReader

#ifndef __vtkXMLImageDataReader_h
#define __vtkXMLImageDataReader_h

#include "vtkXMLStructuredDataReader.h"

class vtkImageData;

class VTK_IO_EXPORT vtkXMLImageDataReader : public vtkXMLStructuredDataReader
{
public:
  vtkTypeRevisionMacro(vtkXMLImageDataReader,vtkXMLStructuredDataReader);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkXMLImageDataReader *New();
  
  // Description:
  // Get/Set the reader's output.
  void SetOutput(vtkImageData *output);
  vtkImageData *GetOutput();
  vtkImageData *GetOutput(int idx);
  
protected:
  vtkXMLImageDataReader();
  ~vtkXMLImageDataReader();  
  
  float Origin[3];
  float Spacing[3];
  
  const char* GetDataSetName();
  void SetOutputExtent(int* extent);
  int ReadPrimaryElement(vtkXMLDataElement* ePrimary);
  void SetupOutputInformation();
  
private:
  vtkXMLImageDataReader(const vtkXMLImageDataReader&);  // Not implemented.
  void operator=(const vtkXMLImageDataReader&);  // Not implemented.
};

#endif
