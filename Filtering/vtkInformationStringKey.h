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
// .NAME vtkInformationStringKey - Key for string values in vtkInformation.
// .SECTION Description
// vtkInformationStringKey is used to represent keys for string values
// in vtkInformation.

#ifndef __vtkInformationStringKey_h
#define __vtkInformationStringKey_h

#include "vtkInformationKey.h"

class VTK_FILTERING_EXPORT vtkInformationStringKey : public vtkInformationKey
{
public:
  vtkTypeRevisionMacro(vtkInformationStringKey,vtkInformationKey);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkInformationStringKey(const char* name, const char* location);
  ~vtkInformationStringKey();

  // Description:
  // Get/Set the value associated with this key in the given
  // information object.
  void Set(vtkInformation* info, const char*);
  const char* Get(vtkInformation* info);
  int Has(vtkInformation* info);

  // Description:
  // Copy the entry associated with this key from one information
  // object to another.  If there is no entry in the first information
  // object for this key, the value is removed from the second.
  virtual void Copy(vtkInformation* from, vtkInformation* to);
private:
  vtkInformationStringKey(const vtkInformationStringKey&);  // Not implemented.
  void operator=(const vtkInformationStringKey&);  // Not implemented.
};

#endif
