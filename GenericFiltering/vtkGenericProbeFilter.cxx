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
#include "vtkGenericProbeFilter.h"

#include "vtkCell.h"
#include "vtkIdTypeArray.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkGenericDataSet.h"
#include "vtkDoubleArray.h"
#include "vtkGenericCellIterator.h"
#include "vtkGenericAdaptorCell.h"
#include "vtkGenericAttribute.h"
#include "vtkGenericAttributeCollection.h"

vtkCxxRevisionMacro(vtkGenericProbeFilter, "$Revision$");
vtkStandardNewMacro(vtkGenericProbeFilter);

//----------------------------------------------------------------------------
vtkGenericProbeFilter::vtkGenericProbeFilter()
{
  this->SpatialMatch = 0;
  this->ValidPoints = vtkIdTypeArray::New();
}

//----------------------------------------------------------------------------
vtkGenericProbeFilter::~vtkGenericProbeFilter()
{
  this->ValidPoints->Delete();
  this->ValidPoints = NULL;
}


//----------------------------------------------------------------------------
void vtkGenericProbeFilter::SetSource(vtkGenericDataSet *input)
{
  this->vtkProcessObject::SetNthInput(1, input);
}

//----------------------------------------------------------------------------
vtkGenericDataSet *vtkGenericProbeFilter::GetSource()
{
  if (this->NumberOfInputs < 2)
    {
    return NULL;
    }
  
  return (vtkGenericDataSet *)(this->Inputs[1]);
}


//----------------------------------------------------------------------------
void vtkGenericProbeFilter::Execute()
{
  vtkIdType ptId, numPts;
  double x[3], tol2;
//  vtkCell *cell;
  vtkPointData *pd, *outPD;
  int subId;
  vtkGenericDataSet *source = this->GetSource();
  vtkDataSet *input = this->GetInput();
  vtkDataSet *output= this->GetOutput();
  double pcoords[3], *weights;
  double fastweights[256];

  vtkDebugMacro(<<"Probing data");

  if (source == NULL)
    {
    vtkErrorMacro (<< "Source is NULL.");
    return;
    }

//  pd = source->GetPointData();
  pd = vtkPointData::New();
  //pd = NULL;
  //int size = input->GetNumberOfPoints();
  
  // lets use a stack allocated array if possible for performance reasons
  int mcs = 255; //source->GetMaxCellSize();  //FIXME
  if (mcs<=256)
    {
    weights = fastweights;
    }
  else
    {
    weights = new double[mcs];
    }

  // First, copy the input to the output as a starting point
  output->CopyStructure( input );

  numPts = input->GetNumberOfPoints();
  this->ValidPoints->Allocate(numPts);

  // Allocate storage for output PointData
  //
  outPD = output->GetPointData();
  //outPD->InterpolateAllocate(pd, size, size);
  vtkDoubleArray *foobar = vtkDoubleArray::New();
  outPD->SetScalars( foobar );

  // Use tolerance as a function of size of source data
  //
//  tol2 = source->GetLength();
  tol2 = 1000;  //FIXME
  tol2 = tol2 ? tol2*tol2 / 1000.0 : 0.001;

  // Loop over all input points, interpolating source data
  //
  int abort=0;
  // Need to use source to create a cellIt since this class is virtual
  vtkGenericCellIterator *cellIt = source->NewCellIterator();

  vtkIdType progressInterval=numPts/20 + 1;
  for (ptId=0; ptId < numPts && !abort; ptId++)
    {
    if ( !(ptId % progressInterval) )
      {
      this->UpdateProgress((double)ptId/numPts);
      abort = GetAbortExecute();
      }

    // Get the xyz coordinate of the point in the input dataset
    input->GetPoint(ptId, x);

    // Find the cell that contains xyz and get it
    vtkIdType foo = source->FindCell(x,cellIt,tol2,subId,pcoords);
    if (foo >= 0)
      {
      // Interpolate the point data
      vtkGenericAdaptorCell *cellProbe = cellIt->GetCell();
      double s[3]; // FIXME: should be double *s=new double[source->GetAttributes()->GetNumberOfComponents()]
      //cellProbe->EvaluateShapeFunction(x,s);
      //source->GetAttributes()->EvaluateTuple(cellProbe, x,s);
      cellProbe->InterpolateTuple(source->GetAttributes(), x,s);
      foobar->InsertTuple( ptId, s);
      this->ValidPoints->InsertNextValue(ptId);
      }
    else
      {
      outPD->NullPoint(ptId);
      }
    }
  cellIt->Delete();

  // BUG FIX: JB.
  // Output gets setup from input, but when output is imagedata, scalartype
  // depends on source scalartype not input scalartype
  if (output->IsA("vtkImageData"))
    {
    vtkImageData *out = (vtkImageData*)output;
    vtkDataArray *s = outPD->GetScalars();
    out->SetScalarType(s->GetDataType());
    out->SetNumberOfScalarComponents(s->GetNumberOfComponents());
    }
  if (mcs>256)
    {
    delete [] weights;
    }
}

//----------------------------------------------------------------------------
/*void vtkGenericProbeFilter::ExecuteInformation()
{
  if (this->GetInput() == NULL || this->GetSource() == NULL)
    {
    vtkErrorMacro("Missing input or source");
    return;
    }

  // Copy whole extent ...
  this->vtkSource::ExecuteInformation();

  // Special case for ParaView.
  if (this->SpatialMatch == 2)
    {
    this->GetOutput()->SetMaximumNumberOfPieces(this->GetSource()->GetMaximumNumberOfPieces());
    }

  if (this->SpatialMatch == 1)
    {
    int m1 = this->GetInput()->GetMaximumNumberOfPieces();
    int m2 = this->GetSource()->GetMaximumNumberOfPieces();
    if (m1 < 0 && m2 < 0)
      {
      this->GetOutput()->SetMaximumNumberOfPieces(-1);
      }
    else
      {
      if (m1 < -1)
        {
        m1 = VTK_LARGE_INTEGER;
        }
      if (m2 < -1)
        {
        m2 = VTK_LARGE_INTEGER;
        }
      if (m2 < m1)
        {
        m1 = m2;
        }
      this->GetOutput()->SetMaximumNumberOfPieces(m1);
      }
    }
}*/


//----------------------------------------------------------------------------
/*void vtkGenericProbeFilter::ComputeInputUpdateExtents( vtkDataObject *output )
{
  vtkDataObject *input = this->GetInput();
  vtkDataObject *source = this->GetSource();
  int usePiece = 0;
  
  if (input == NULL || source == NULL)
    {
    vtkErrorMacro("Missing input or source.");
    return;
    }

  // What ever happend to CopyUpdateExtent in vtkDataObject?
  // Copying both piece and extent could be bad.  Setting the piece
  // of a structured data set will affect the extent.
  if (output->IsA("vtkUnstructuredGrid") || output->IsA("vtkPolyData"))
    {
    usePiece = 1;
    }
  
  input->RequestExactExtentOn();
  
  if ( ! this->SpatialMatch)
    {
    source->SetUpdateExtent(0, 1, 0);
    }
  else if (this->SpatialMatch == 1)
    {
    if (usePiece)
      {
      // Request an extra ghost level because the probe
      // gets external values with computation prescision problems.
      // I think the probe should be changed to have an epsilon ...
      source->SetUpdateExtent(output->GetUpdatePiece(), 
                              output->GetUpdateNumberOfPieces(),
                              output->GetUpdateGhostLevel()+1);
      }
    else
      {
      source->SetUpdateExtent(output->GetUpdateExtent()); 
      }
    }
  
  if (usePiece)
    {
    input->SetUpdateExtent(output->GetUpdatePiece(), 
                           output->GetUpdateNumberOfPieces(),
                           output->GetUpdateGhostLevel());
    }
  else
    {
    input->SetUpdateExtent(output->GetUpdateExtent()); 
    }
  
  // Use the whole input in all processes, and use the requested update
  // extent of the output to divide up the source.
  if (this->SpatialMatch == 2)
    {
    input->SetUpdateExtent(0, 1, 0);
    source->SetUpdateExtent(output->GetUpdatePiece(),
                            output->GetUpdateNumberOfPieces(),
                            output->GetUpdateGhostLevel());
    }
}*/

//----------------------------------------------------------------------------
void vtkGenericProbeFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkGenericDataSet *source = this->GetSource();

  this->Superclass::PrintSelf(os,indent);
  os << indent << "Source: " << source << "\n";
  if (this->SpatialMatch)
    {
    os << indent << "SpatialMatchOn\n";
    }
  else
    {
    os << indent << "SpatialMatchOff\n";
    }
  os << indent << "ValidPoints: " << this->ValidPoints << "\n";
}
