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

#include "vtkCommand.h"
#include "vtkDenseArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkArrayNorm.h"

#include <vtkstd/stdexcept>

///////////////////////////////////////////////////////////////////////////////
// vtkArrayNorm

vtkCxxRevisionMacro(vtkArrayNorm, "$Revision$");
vtkStandardNewMacro(vtkArrayNorm);

vtkArrayNorm::vtkArrayNorm() :
  Dimension(0),
  L(2)
{
}

vtkArrayNorm::~vtkArrayNorm()
{
}

void vtkArrayNorm::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Dimension: " << this->Dimension << endl;
  os << indent << "L: " << this->L << endl;
}

void vtkArrayNorm::SetL(int value)
{
  if(value < 1)
    {
    vtkErrorMacro(<< "Cannot compute array norm for L < 1");
    return;
    }
  
  if(this->L == value)
    return;

  this->L = value;
  this->Modified();
}

int vtkArrayNorm::RequestData(
  vtkInformation*, 
  vtkInformationVector** inputVector, 
  vtkInformationVector* outputVector)
{
  try
    {
    // Test our preconditions ...
    vtkArrayData* const input_data = vtkArrayData::GetData(inputVector[0]);
    if(!input_data)
      throw vtkstd::runtime_error("Missing input vtkArrayData on port 0.");
    if(input_data->GetNumberOfArrays() != 1)
      throw vtkstd::runtime_error("Input vtkArrayData must contain exactly one array.");
    vtkTypedArray<double>* const input_array = vtkTypedArray<double>::SafeDownCast(input_data->GetArray(0));
    if(!input_array)
      throw vtkstd::runtime_error("Input array must be a vtkTypedArray<double>.");

    if(this->Dimension < 0 || this->Dimension >= input_array->GetDimensions())
      throw vtkstd::runtime_error("Dimension out-of-bounds.");

    const vtkIdType dimension_extents = input_array->GetExtents()[this->Dimension];

    // Setup our output ...
    vtkDenseArray<double>* const output_array = vtkDenseArray<double>::New();
    output_array->Resize(dimension_extents);
    output_array->Fill(0.0);

    vtkArrayData* const output = vtkArrayData::GetData(outputVector);
    output->ClearArrays();
    output->AddArray(output_array);
    output_array->Delete();

    // Make it happen ...
    vtkArrayCoordinates coordinates;
    const vtkIdType non_null_count = input_array->GetNonNullSize();
    for(vtkIdType n = 0; n != non_null_count; ++n)
      {
      input_array->GetCoordinatesN(n, coordinates);
      (*output_array)[vtkArrayCoordinates(coordinates[this->Dimension])] += pow(input_array->GetValueN(n), this->L);
      }

    for(vtkIdType n = 0; n != dimension_extents; ++n)
      {
      output_array->SetValueN(n, pow(output_array->GetValueN(n), 1.0 / this->L));
      }
    }
  catch(vtkstd::exception& e)
    {
    vtkErrorMacro(<< "unhandled exception: " << e.what());
    return 0;
    }
  catch(...)
    {
    vtkErrorMacro(<< "unknown exception");
    return 0;
    }

  return 1;
}

