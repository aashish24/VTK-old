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

#include "vtkArrayRange.h"

#include <vtkstd/algorithm> // for vtkstd::max()

vtkArrayRange::vtkArrayRange() :
  Begin(0),
  End(0)
{
}

vtkArrayRange::vtkArrayRange(vtkIdType index) :
  Begin(index),
  End(index + 1)
{
}

vtkArrayRange::vtkArrayRange(vtkIdType begin, vtkIdType end) :
  Begin(begin),
  End(vtkstd::max(begin, end))
{
}

vtkIdType vtkArrayRange::GetBegin() const
{
  return this->Begin;
}

vtkIdType vtkArrayRange::GetEnd() const
{
  return this->End;
}

vtkIdType vtkArrayRange::GetExtent() const
{
  return this->End - this->Begin;
}

ostream& operator<<(ostream& stream, const vtkArrayRange& rhs)
{
  stream << "[" << rhs.Begin << ", " << rhs.End << ")";
  return stream;
}

