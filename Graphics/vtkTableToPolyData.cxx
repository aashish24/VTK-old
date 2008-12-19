/*=========================================================================

  Program:   ParaView
  Module:    $RCSfile$

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkTableToPolyData.h"

#include "vtkDoubleArray.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkTable.h"
#include "vtkCellArray.h"
#include "vtkInformation.h"

vtkStandardNewMacro(vtkTableToPolyData);
vtkCxxRevisionMacro(vtkTableToPolyData, "$Revision$");
//----------------------------------------------------------------------------
vtkTableToPolyData::vtkTableToPolyData()
{
  this->XColumn = 0;
  this->YColumn = 0;
  this->ZColumn = 0;
  this->XComponent = 0;
  this->YComponent = 0;
  this->ZComponent = 0;
}

//----------------------------------------------------------------------------
vtkTableToPolyData::~vtkTableToPolyData()
{
  this->SetXColumn(0);
  this->SetYColumn(0);
  this->SetZColumn(0);
}

//----------------------------------------------------------------------------
int vtkTableToPolyData::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkTable");
  return 1;
}

//----------------------------------------------------------------------------
int vtkTableToPolyData::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkTable* input = vtkTable::GetData(inputVector[0], 0);
  vtkPolyData* output = vtkPolyData::GetData(outputVector, 0);

  if (input->GetNumberOfRows() == 0)
    {
    // empty input.
    return 1;
    }

  vtkDataArray* xarray = vtkDataArray::SafeDownCast(
    input->GetColumnByName(this->XColumn));
  vtkDataArray* yarray = vtkDataArray::SafeDownCast(
    input->GetColumnByName(this->YColumn));
  vtkDataArray* zarray = vtkDataArray::SafeDownCast(
    input->GetColumnByName(this->ZColumn));
  if (!xarray || !yarray || !zarray)
    {
    vtkErrorMacro("Failed to locate  the columns to use for the point"
      " coordinates");
    return 0;
    }

  vtkPoints* newPoints = vtkPoints::New();

  if (xarray == yarray && yarray == zarray && 
    this->XComponent == 0 &&
    this->YComponent == 1 &&
    this->ZComponent == 2 &&
    xarray->GetNumberOfComponents() == 3)
    {
    newPoints->SetData(xarray);
    }
  else
    {
    // Ideally we determine the smallest data type that can contain the values
    // in all the 3 arrays. For now I am just going with doubles.
    vtkDoubleArray* newData =  vtkDoubleArray::New();
    newData->SetNumberOfComponents(3);
    newData->SetNumberOfTuples(input->GetNumberOfRows());
    vtkIdType numtuples = newData->GetNumberOfTuples();
    for (vtkIdType cc=0; cc < numtuples; cc++)
      {
      newData->SetComponent(cc, 0, xarray->GetComponent(cc, this->XComponent));
      newData->SetComponent(cc, 1, yarray->GetComponent(cc, this->YComponent));
      newData->SetComponent(cc, 2, zarray->GetComponent(cc, this->ZComponent));
      }
    newPoints->SetData(newData);
    newData->Delete();
    }

  output->SetPoints(newPoints);
  newPoints->Delete();

  // Now create a poly-vertex cell will all the points.
  vtkIdType numPts = newPoints->GetNumberOfPoints();
  vtkIdType *ptIds = new vtkIdType[numPts];
  for (vtkIdType cc=0; cc < numPts; cc++)
    {
    ptIds[cc] = cc;
    }
  output->Allocate(1);
  output->InsertNextCell(VTK_POLY_VERTEX, numPts, ptIds);
  delete [] ptIds;

  // Add all other columns as point data.
  for (int cc=0; cc < input->GetNumberOfColumns(); cc++)
    {
    vtkAbstractArray* arr = input->GetColumn(cc);
    if (arr != xarray && arr != yarray && arr != zarray)
      {
      output->GetPointData()->AddArray(arr);
      }
    }
  return 1;
}

//----------------------------------------------------------------------------
void vtkTableToPolyData::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "XColumn: " 
    << (this->XColumn? this->XColumn : "(none)") << endl;
  os << indent << "XComponent: " << this->XComponent << endl;
  os << indent << "YColumn: " 
    << (this->YColumn? this->YColumn : "(none)") << endl;
  os << indent << "YComponent: " << this->XComponent << endl;
  os << indent << "ZColumn: " 
    << (this->ZColumn? this->ZColumn : "(none)") << endl;
  os << indent << "ZComponent: " << this->XComponent << endl;
}


