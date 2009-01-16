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

// .NAME vtkArray - Abstract interface for N-dimensional arrays.
//
// .SECTION Description
// vtkArray is the root of a hierarchy of arrays that can be used to store data with
// any number of dimensions.  It provides an abstract interface for retrieving and
// setting array attributes that are independent of the type of values stored in the
// array - such as the number of dimensions, extents along each dimension, and number
// of values stored in the array.
//
// To get and set array values, the vtkTypedArray template class derives from vtkArray
// and provides type-specific methods for retrieval and update.
//
// Two concrete derivatives of vtkTypedArray are provided at the moment: vtkDenseArray
// and vtkSparseArray, which provide dense and sparse storage for arbitrary-dimension
// data, respectively.  Toolkit users can create their own concrete derivatives that
// implement alternative storage strategies, such as compressed-sparse-row, etc.  You
// could also create an array that provided read-only access to 'virtual' data, such
// as an array that returned a Fibonacci sequence, etc.
//
// .SECTION See Also
// vtkTypedArray, vtkDenseArray, vtkSparseArray
//
// .SECTION Thanks
// Developed by Timothy M. Shead (tshead@sandia.gov) at Sandia National Laboratories.

#ifndef __vtkArray_h
#define __vtkArray_h

#include "vtkArrayExtents.h"
#include "vtkObject.h"
#include "vtkStdString.h"

class vtkArrayCoordinates;

class VTK_COMMON_EXPORT vtkArray : public vtkObject
{
public:
  vtkTypeRevisionMacro(vtkArray, vtkObject);
  void PrintSelf(ostream &os, vtkIndent indent);

//BTX
  enum
  {
    /// Used with CreateArray() to create dense arrays
    DENSE = 0,
    /// Used with CreateArray() to create sparse arrays
    SPARSE = 1
  };
//ETX

  // Description:
  // Creates a new array where StorageType is one of vtkArray::DENSE or vtkArray::SPARSE, and
  // ValueType is one of VTK_CHAR, VTK_UNSIGNED_CHAR, VTK_SHORT, VTK_UNSIGNED_SHORT,
  // VTK_INT, VTK_UNSIGNED_INT, VTK_LONG, VTK_UNSIGNED_LONG, VTK_DOUBLE, VTK_ID_TYPE,
  // or VTK_STRING.  The caller is responsible for the lifetime of the returned object.
  static vtkArray* CreateArray(int StorageType, int ValueType);

//BTX
  // Description:
  // Resizes the array to the given extents (number of dimensions and size of each dimension).
  // Note that concrete implementations of vtkArray may place constraints on the the extents
  // that they will store, so you cannot assume that GetExtents() will always return the same
  // value passed to Resize().
  //
  // The contents of the array are undefined after calling Resize() - you should
  // initialize its contents accordingly.  In particular, dimension-labels will be
  // undefined, dense array values will be undefined, and sparse arrays will be
  // empty.
  void Resize(vtkIdType i);
  void Resize(vtkIdType i, vtkIdType j);
  void Resize(vtkIdType i, vtkIdType j, vtkIdType k);
  void Resize(const vtkArrayExtents& extents);

  // Description:
  // Returns the extents (the number of dimensions and size along each dimension) of the array.
  virtual vtkArrayExtents GetExtents() = 0;
//ETX

  // Description:
  // Returns the number of dimensions stored in the array.  Note that this is the same as
  // calling GetExtents().GetDimensions().
  vtkIdType GetDimensions();
  
  // Description:
  // Returns the number of values stored in the array.  Note that this is the same as calling
  // GetExtents().GetSize(), and represents the maximum number of values that could ever
  // be stored using the current extents.  This is equal to the number of values stored in a
  // dense array, but may be larger than the number of values stored in a sparse array.
  vtkIdType GetSize();
  
  // Description:
  // Returns the number of non-null values stored in the array.  Note that this
  // value will equal GetSize() for dense arrays, and will be less-than-or-equal
  // to GetSize() for sparse arrays.
  virtual vtkIdType GetNonNullSize() = 0;

  // Description:
  // Sets the label for the i-th array dimension.
  void SetDimensionLabel(vtkIdType i, const vtkStdString& label);
  
  // Description:
  // Returns the label for the i-th array dimension.
  vtkStdString GetDimensionLabel(vtkIdType i);

  //BTX
  // Description:
  // Returns the coordinates of the n-th value in the array, where n is in the
  // range [0, GetNonNullSize()).  Note that the order in which coordinates are visited
  // is undefined, but is guaranteed to match the order in which values are visited using
  // vtkTypedArray::GetValueN() and vtkTypedArray::SetValueN().
  virtual void GetCoordinatesN(const vtkIdType n, vtkArrayCoordinates& coordinates) = 0;
  //ETX

  // Description:
  // Returns a new array that is a deep copy of this array.
  virtual vtkArray* DeepCopy() = 0;

protected:
  vtkArray();
  ~vtkArray();

private:
  vtkArray(const vtkArray&); // Not implemented
  void operator=(const vtkArray&); // Not implemented

  // Description:
  // Implemented in concrete derivatives to update their storage
  // when the array is resized.
  virtual void InternalResize(const vtkArrayExtents&) = 0;
  
  // Description:
  // Implemented in concrete derivatives to set dimension labels.
  virtual void InternalSetDimensionLabel(vtkIdType i, const vtkStdString& label) = 0;
  
  // Description:
  // Implemented in concrete derivatives to get dimension labels.
  virtual vtkStdString InternalGetDimensionLabel(vtkIdType i) = 0;
};

#endif

