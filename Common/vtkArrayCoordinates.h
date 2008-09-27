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

#ifndef __vtkArrayCoordinates_h
#define __vtkArrayCoordinates_h

#include "vtkSystemIncludes.h"
#include <vtksys/stl/vector>

// .NAME vtkArrayCoordinates - Used to store N-way array coordinates.

// .SECTION Description
// Provides a collection of coordinates for accessing values in an N-way array.
// Convenience constructors are provided for working with 1, 2, and 3-way data.
// For higher dimensions, use SetDimensions() and operator[].

// .SECTION Thanks
// Developed by Timothy M. Shead (tshead@sandia.gov) at Sandia National Laboratories.

class VTK_COMMON_EXPORT vtkArrayCoordinates
{
public:
  // Description:
  // Create an empty set of coordinates.  Use SetDimensions() and operator[]
  // to populate the coordinates.
  vtkArrayCoordinates();
  
  // Description:
  // Create coordinates for a one-dimensional array.
  explicit vtkArrayCoordinates(vtkIdType i);
  
  // Description:
  // Create coordinates for a two-dimensional array.
  vtkArrayCoordinates(vtkIdType i, vtkIdType j);
  
  // Description:
  // Create coordinates for a three-dimensional array.
  vtkArrayCoordinates(vtkIdType i, vtkIdType j, vtkIdType k);

  // Description:
  // Return the number of dimensions contained in the coordinates.
  vtkIdType GetDimensions() const;

  // Description:
  // Set the number of dimensions.  Resets each coordinate to zero.
  void SetDimensions(vtkIdType dimensions);
  
  // Description:
  // Returns the index of the i-th dimension.
  vtkIdType& operator[](vtkIdType i);
  
  // Description:
  // Returns the index of the i-th dimension.
  const vtkIdType& operator[](vtkIdType i) const;  

  VTK_COMMON_EXPORT friend ostream& operator<<(ostream& stream, const vtkArrayCoordinates& rhs);
  
private:
  vtkstd::vector<vtkIdType> Storage;
};

#endif

