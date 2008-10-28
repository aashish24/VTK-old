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

#include "vtkQuadraturePointInterpolator.h"
#include "vtkQuadratureSchemeDefinition.h"

#include "vtkUnstructuredGrid.h"
#include "vtkPolyData.h"
#include "vtkPoints.h"
#include "vtkType.h"
#include "vtkDoubleArray.h"
#include "vtkDataArray.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkCellArray.h"
#include "vtkIdTypeArray.h"
#include "vtkIntArray.h"
#include "vtkUnstructuredGridAlgorithm.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkInformationQuadratureSchemeDefinitionVectorKey.h"
#include "vtkObjectFactory.h"

#include "vtksys/ios/sstream"
using vtksys_ios::ostringstream;

#include "vtkQuadraturePointsUtilities.hxx"

vtkCxxRevisionMacro(vtkQuadraturePointInterpolator, "$Revision$");
vtkStandardNewMacro(vtkQuadraturePointInterpolator);

//-----------------------------------------------------------------------------
vtkQuadraturePointInterpolator::vtkQuadraturePointInterpolator()
{
  this->Clear();
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

//-----------------------------------------------------------------------------
vtkQuadraturePointInterpolator::~vtkQuadraturePointInterpolator()
{
  this->Clear();
}

//-----------------------------------------------------------------------------
int vtkQuadraturePointInterpolator::FillInputPortInformation(
        int port,
        vtkInformation *info)
{
  switch (port)
    {
    case 0:
      info->Set(vtkDataObject::DATA_TYPE_NAME(),"vtkUnstructuredGrid");
      break;
    }
  return 1;
}

//-----------------------------------------------------------------------------
int vtkQuadraturePointInterpolator::FillOutputPortInformation(
        int port,
        vtkInformation *info)
{
  switch (port)
    {
    case 0:
      info->Set(vtkDataObject::DATA_TYPE_NAME(),"vtkUnstructuredGrid");
      break;
    }
  return 1;
}

//-----------------------------------------------------------------------------
int vtkQuadraturePointInterpolator::RequestData(
        vtkInformation *,
        vtkInformationVector **input,
        vtkInformationVector *output)
{
  vtkDataObject *tmpDataObj;
  // Get the inputs
  tmpDataObj
    = input[0]->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT());
  vtkUnstructuredGrid *usgIn
    = vtkUnstructuredGrid::SafeDownCast(tmpDataObj);
  // Get the outputs
  tmpDataObj
    = output->GetInformationObject(0)->Get(vtkDataObject::DATA_OBJECT());
  vtkUnstructuredGrid *usgOut
    = vtkUnstructuredGrid::SafeDownCast(tmpDataObj);

  // Quick sanity check.
  if (usgIn==NULL || usgOut==NULL
     || usgIn->GetNumberOfPoints()==0
     || usgIn->GetPointData()->GetNumberOfArrays()==0)
    {
    vtkWarningMacro("Filter data has not been configured correctly. Aborting.");
    return 1;
    }

  // Copy the unstructured grid on the input
  usgOut->ShallowCopy(usgIn);

  // Interpolate the data arrays, but no points. Results
  // are stored in field data arrays.
  this->InterpolateFields(usgOut);

  return 1;
}

//-----------------------------------------------------------------------------
void vtkQuadraturePointInterpolator::Clear()
{
  // Nothing to do
}

//-----------------------------------------------------------------------------
int vtkQuadraturePointInterpolator::InterpolateFields(
        vtkUnstructuredGrid *usgOut)
{
  // Extract info we need for all cells.
  vtkIdType nCells=usgOut->GetNumberOfCells();

  // For each array we interpolate scalar data to the 
  // integration point location. Results are in associated
  // field data arrays.
  int nArrays
    = usgOut->GetPointData()->GetNumberOfArrays();
  for (int arrayId=0; arrayId<nArrays; ++arrayId)
    {
    // Grab the next array, proccess it only if we have floating
    // point data.
    vtkDataArray *V=usgOut->GetPointData()->GetArray(arrayId);
    int V_type=V->GetDataType();
    if (! ((V_type==VTK_FLOAT)||(V_type==VTK_DOUBLE)))
      {
      continue;
      }
    // Use two arrays, one with the interpolated values,
    // the other with offsets to the start of each cell's 
    // interpolated values.
    vtkIdTypeArray *indexes=vtkIdTypeArray::New();
    indexes->SetNumberOfComponents(1);
    indexes->SetNumberOfTuples(nCells);
    ostringstream indexesName;
    indexesName << V->GetName() << "_QP_Indexes";
    indexes->SetName(indexesName.str().c_str());
    usgOut->GetFieldData()->AddArray(indexes);
    indexes->Delete();
    vtkIdType *pIndexes=indexes->GetPointer(0);
    //
    int nComps=V->GetNumberOfComponents();
    vtkDoubleArray *interpolated=vtkDoubleArray::New();
    interpolated->SetNumberOfComponents(nComps);
    interpolated->Allocate(nComps*nCells); // at least one qp per cell
    ostringstream interpolatedName;
    interpolatedName << V->GetName() << "_QP_Interpolated";
    interpolated->SetName(interpolatedName.str().c_str());
    usgOut->GetFieldData()->AddArray(interpolated);
    interpolated->Delete();

    // Get the dictionary associated with this array.We are going 
    // to make a copy for efficiency.
    vtkInformation *info=V->GetInformation();
    vtkInformationQuadratureSchemeDefinitionVectorKey *key=vtkQuadratureSchemeDefinition::DICTIONARY();
    if (!key->Has(info))
      {
      vtkWarningMacro("Dictionary is not present in "
                       << V->GetName() << ".  Skipping.");
      continue;
      }
    int dictSize= key->Size(info);
    vtkQuadratureSchemeDefinition **dict=new vtkQuadratureSchemeDefinition *[dictSize];
    key->GetRange(info,dict,0,0,dictSize);

    // For all cells interpolate.
    switch (V_type)
      {
      case VTK_DOUBLE:
        {
        vtkDoubleArray *V_d=static_cast<vtkDoubleArray *>(V);
        double *pV_d=V_d->GetPointer(0);
        if (!Interpolate(usgOut,nCells,V_d,pV_d,nComps,dict,interpolated,pIndexes))
          {
          vtkWarningMacro("Failed to interpolate fields "
                          "to quadrature points. Aborting.");
          return 0;
          }
        break;
        }
      case VTK_FLOAT:
        {
        vtkFloatArray *V_f=static_cast<vtkFloatArray *>(V);
        float *pV_f=V_f->GetPointer(0);
        if (!Interpolate(usgOut,nCells,V_f,pV_f,nComps,dict,interpolated,pIndexes))
          {
          vtkWarningMacro("Failed to interpolate fields "
                          "to quadrature points. Aborting.");
          return 0;
          }
        break;
        }
      }

    delete [] dict;
    }

  return 1;
}

//-----------------------------------------------------------------------------
void vtkQuadraturePointInterpolator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "No state." << endl;
}

