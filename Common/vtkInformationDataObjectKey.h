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
// .NAME vtkInformationDataObjectKey - Key for vtkDataObject values.
// .SECTION Description
// vtkInformationDataObjectKey is used to represent keys in
// vtkInformation for values that are vtkDataObject instances.

#ifndef __vtkInformationDataObjectKey_h
#define __vtkInformationDataObjectKey_h

#include "vtkInformationKey.h"

class vtkDataObject;

class VTK_COMMON_EXPORT vtkInformationDataObjectKey : public vtkInformationKey
{
public:
  vtkTypeRevisionMacro(vtkInformationDataObjectKey,vtkInformationKey);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkInformationDataObjectKey(const char* name, const char* location);
  ~vtkInformationDataObjectKey();

  // Description:
  // Get/Set the value associated with this key in the given
  // information object.
  void Set(vtkInformation* info, vtkDataObject*);
  vtkDataObject* Get(vtkInformation* info);
  int Has(vtkInformation* info);

private:
  vtkInformationDataObjectKey(const vtkInformationDataObjectKey&);  // Not implemented.
  void operator=(const vtkInformationDataObjectKey&);  // Not implemented.
};

#endif
