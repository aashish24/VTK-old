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

#ifndef __vtkArrayRange_h
#define __vtkArrayRange_h

#include "vtkSystemIncludes.h"

// .NAME vtkArrayRange - Stores a half-open range of coordinates for a
// single dimension of a vtkArraySlice.

// .SECTION Thanks
// Developed by Timothy M. Shead (tshead@sandia.gov) at Sandia National Laboratories.

class VTK_COMMON_EXPORT vtkArrayRange
{
public:
  // Description:
  // Creates an empty range.
  vtkArrayRange();
  
  // Description:
  // Creates a range containing a single value.
  vtkArrayRange(vtkIdType index);
  
  // Description:
  // Creates a half-open range [begin, end).  Note that begin must be <= end,
  // if not, creates the empty range [begin, begin).
  vtkArrayRange(vtkIdType begin, vtkIdType end);

  // Description:
  // Returns the beginning of the range
  const vtkIdType GetBegin() const;
  
  // Description:
  // Returns one-past-the-end of the range
  const vtkIdType GetEnd() const;

  // Description:
  // Returns the extent of the range (the distance End - Begin).
  const vtkIdType GetExtent() const;

  // Description:
  // Serialization  
  friend ostream& operator<<(ostream& stream, const vtkArrayRange& rhs);

private:
  // Description:
  // Stores the beginning of the range.
  vtkIdType Begin;
  
  // Description:
  // Stores one-past-the-end of the range.
  vtkIdType End;
};

#endif

