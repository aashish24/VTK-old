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
// .NAME vtkInformationIntegerVectorKey - Key for integer vector values.
// .SECTION Description
// vtkInformationIntegerVectorKey is used to represent keys for integer
// vector values in vtkInformation.h

#ifndef __vtkInformationIntegerVectorKey_h
#define __vtkInformationIntegerVectorKey_h

#include "vtkInformationKey.h"

class VTK_COMMON_EXPORT vtkInformationIntegerVectorKey : public vtkInformationKey
{
public:
  vtkTypeRevisionMacro(vtkInformationIntegerVectorKey,vtkInformationKey);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkInformationIntegerVectorKey(const char* name, const char* location);
  ~vtkInformationIntegerVectorKey();

  // Description:
  // Get/Set the value associated with this key in the given
  // information object.
  void Set(vtkInformation* info, int* value, int length);
  int* Get(vtkInformation* info);
  void Get(vtkInformation* info, int* value);
  int Length(vtkInformation* info);
  int Has(vtkInformation* info);

private:
  vtkInformationIntegerVectorKey(const vtkInformationIntegerVectorKey&);  // Not implemented.
  void operator=(const vtkInformationIntegerVectorKey&);  // Not implemented.
};

#endif
