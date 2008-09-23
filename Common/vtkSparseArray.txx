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

#ifndef __vtkSparseArray_txx
#define __vtkSparseArray_txx

template<typename T>
vtkSparseArray<T>* vtkSparseArray<T>::New()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance(typeid(ThisT).name());
  if(ret)
    {
    return static_cast<ThisT*>(ret);
    }
  return new ThisT();
}

template<typename T>
void vtkSparseArray<T>::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkSparseArray<T>::Superclass::PrintSelf(os, indent);
}

template<typename T>
vtkArrayExtents vtkSparseArray<T>::GetExtents()
{
  return this->Extents;
}

template<typename T>
vtkIdType vtkSparseArray<T>::GetNonNullSize()
{
  return this->Values.size();
}

template<typename T>
void vtkSparseArray<T>::GetCoordinatesN(const vtkIdType n, vtkArrayCoordinates& coordinates)
{
  coordinates.SetDimensions(this->GetDimensions());
  for(vtkIdType i = 0; i != this->GetDimensions(); ++i)
    coordinates[i] = this->Coordinates[(n * this->GetDimensions()) + i];
}

template<typename T>
vtkArray* vtkSparseArray<T>::DeepCopy()
{
  ThisT* const copy = ThisT::New();

  copy->Extents = this->Extents;
  copy->DimensionLabels = this->DimensionLabels;
  copy->Coordinates = this->Coordinates;
  copy->Values = this->Values;

  return copy;
}

template<typename T>
const T& vtkSparseArray<T>::GetValue(const vtkArrayCoordinates& coordinates)
{
  static T temp;

  if(coordinates.GetDimensions() != this->GetDimensions())
    {
    vtkErrorMacro(<< "Index-array dimension mismatch.");
    return temp;
    }

  // Do a naive linear-search for the time-being ... 
  for(vtkIdType row = 0; row != this->Values.size(); ++row)
    {
    for(vtkIdType column = 0; column != this->GetDimensions(); ++column)
      {
      if(coordinates[column] != this->Coordinates[(row * this->GetDimensions()) + column])
        break;

      if(column + 1 == this->GetDimensions())
        return this->Values[row];
      }
    }
  
  return temp;
}

template<typename T>
const T& vtkSparseArray<T>::GetValueN(const vtkIdType n)
{
  return this->Values[n];
}

template<typename T>
void vtkSparseArray<T>::SetValue(const vtkArrayCoordinates& coordinates, const T& value)
{
  if(coordinates.GetDimensions() != this->GetDimensions())
    {
    vtkErrorMacro(<< "Index-array dimension mismatch.");
    return;
    }

  // Do a naive linear-search for the time-being ... 
  for(vtkIdType row = 0; row != this->Values.size(); ++row)
    {
    for(vtkIdType column = 0; column != this->GetDimensions(); ++column)
      {
      if(coordinates[column] != this->Coordinates[(row * this->GetDimensions()) + column])
        break;

      if(column + 1 == this->GetDimensions())
        {
        this->Values[row] = value;
        return;
        }
      }
    }

  // Element doesn't already exist, so add it to the end of the list ...
  this->AddValue(coordinates, value);
}

template<typename T>
void vtkSparseArray<T>::SetValueN(const vtkIdType n, const T& value)
{
  this->Values[n] = value;
}

template<typename T>
void vtkSparseArray<T>::Clear()
{
  this->Coordinates.clear();
  this->Values.clear();
}

template<typename T>
const vtkIdType* vtkSparseArray<T>::GetCoordinateStorage() const
{
  return &this->Coordinates[0];
}

template<typename T>
vtkIdType* vtkSparseArray<T>::GetCoordinateStorage()
{
  return &this->Coordinates[0];
}

template<typename T>
const T* vtkSparseArray<T>::GetValueStorage() const
{
  return &(this->Values[0]);
}

template<typename T>
T* vtkSparseArray<T>::GetValueStorage()
{
  return &this->Values[0];
}

template<typename T>
void vtkSparseArray<T>::ResizeToContents()
{
  vtkArrayExtents new_extents = this->Extents;

  vtkIdType row_begin = 0;
  vtkIdType row_end = row_begin + this->Values.size();
  for(vtkIdType row = row_begin; row != row_end; ++row)
    {
    for(vtkIdType column = 0; column != this->GetDimensions(); ++column)
      {
      new_extents[column] = vtkstd::max(new_extents[column], this->Coordinates[(row * this->GetDimensions()) + column] + 1);
      }
    }

  this->Extents = new_extents;
}

template<typename T>
void vtkSparseArray<T>::AddValue(const vtkArrayCoordinates& coordinates, const T& Value)
{
  if(coordinates.GetDimensions() != this->GetDimensions())
    {
    vtkErrorMacro(<< "Index-array dimension mismatch.");
    return;
    }
 
  this->Values.push_back(Value);

  for(vtkIdType i = 0; i != coordinates.GetDimensions(); ++i)
    this->Coordinates.push_back(coordinates[i]);
}

template<typename T>
vtkSparseArray<T>::vtkSparseArray()
{
}

template<typename T>
vtkSparseArray<T>::~vtkSparseArray()
{
}

template<typename T>
void vtkSparseArray<T>::InternalResize(const vtkArrayExtents& extents)
{
  this->Extents = extents;
  this->DimensionLabels.resize(extents.GetDimensions(), vtkStdString());
  this->Values.resize(0);
  this->Coordinates.resize(0);
}

template<typename T>
void vtkSparseArray<T>::InternalSetDimensionLabel(vtkIdType i, const vtkStdString& label)
{
  this->DimensionLabels[i] = label;
}

template<typename T>
vtkStdString vtkSparseArray<T>::InternalGetDimensionLabel(vtkIdType i)
{
  return this->DimensionLabels[i];
}

#endif

