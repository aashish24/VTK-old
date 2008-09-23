/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  
-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

#ifndef __vtkTypedArray_h
#define __vtkTypedArray_h

#include "vtkArray.h"
#include "vtkTypeTemplate.h"

class vtkArrayCoordinates;

// .NAME vtkTypedArray - Provides a type-specific interface to N-way arrays

// .SECTION Thanks
// Developed by Timothy M. Shead (tshead@sandia.gov) at Sandia National Laboratories.

template<typename T>
class vtkTypedArray : public vtkTypeTemplate<vtkTypedArray<T>, vtkArray>
{
public:
  void PrintSelf(ostream &os, vtkIndent indent);

  // Description:
  // Returns the value stored in the array at the given coordinates.
  // Note that the number of dimensions in the supplied coordinates must
  // match the number of dimensions in the array.
  inline const T& GetValue(vtkIdType i);
  inline const T& GetValue(vtkIdType i, vtkIdType j);
  inline const T& GetValue(vtkIdType i, vtkIdType j, vtkIdType k);
  virtual const T& GetValue(const vtkArrayCoordinates& coordinates) = 0;
  
  // Description:
  // Returns the n-th value stored in the array, where n is in the
  // range [0, GetNonNullSize()).  This is useful for efficiently
  // visiting every value in the array.  Note that the order in which
  // values are visited is undefined, but is guaranteed to match the
  // order used by vtkArray::GetCoordinatesN().
  virtual const T& GetValueN(const vtkIdType n) = 0;
  
  // Description:
  // Overwrites the value stored in the array at the given coordinates.
  // Note that the number of dimensions in the supplied coordinates must
  // match the number of dimensions in the array.
  inline void SetValue(vtkIdType i, const T& value);
  inline void SetValue(vtkIdType i, vtkIdType j, const T& value);
  inline void SetValue(vtkIdType i, vtkIdType j, vtkIdType k, const T& value);
  virtual void SetValue(const vtkArrayCoordinates& coordinates, const T& value) = 0;
  
  // Description:
  // Overwrites the n-th value stored in the array, where n is in the
  // range [0, GetNonNullSize()).  This is useful for efficiently
  // visiting every value in the array.  Note that the order in which
  // values are visited is undefined, but is guaranteed to match the
  // order used by vtkArray::GetCoordinatesN().
  virtual void SetValueN(const vtkIdType n, const T& value) = 0;

protected:
  vtkTypedArray() {}
  ~vtkTypedArray() {}

private:
  vtkTypedArray(const vtkTypedArray&); // Not implemented
  void operator=(const vtkTypedArray&); // Not implemented
};

#include "vtkTypedArray.txx"

#endif

