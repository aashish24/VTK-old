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
#include "vtkWarpVector.h"

#include "vtkCellData.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPointSet.h"
#include "vtkPoints.h"

vtkCxxRevisionMacro(vtkWarpVector, "$Revision$");
vtkStandardNewMacro(vtkWarpVector);

vtkWarpVector::vtkWarpVector()
{
  this->ScaleFactor = 1.0;
  this->InputVectorsSelection = NULL;
}

vtkWarpVector::~vtkWarpVector()
{
  this->SetInputVectorsSelection(NULL);
}

template <class T1, class T2>
void vtkWarpVectorExecute2(vtkWarpVector *self, T1 *inPts, 
                           T1 *outPts, T2 *inVec, vtkIdType max)
{
  vtkIdType ptId;
  T1 scaleFactor = (T1)self->GetScaleFactor();
  
  // Loop over all points, adjusting locations
  for (ptId=0; ptId < max; ptId++)
    {
    if (!(ptId & 0xfff)) 
      {
      self->UpdateProgress ((double)ptId/(max+1));
      if (self->GetAbortExecute())
        {
        break;
        }
      }
    
    *outPts = *inPts + scaleFactor * (T1)(*inVec);
    outPts++; inPts++; inVec++;
    *outPts = *inPts + scaleFactor * (T1)(*inVec);
    outPts++; inPts++; inVec++;
    *outPts = *inPts + scaleFactor * (T1)(*inVec);
    outPts++; inPts++; inVec++;
    }
}
          
template <class T>
void vtkWarpVectorExecute(vtkWarpVector *self, 
                          T *inPts, 
                          T *outPts, 
                          vtkIdType max,
                          const char* vectorsSelection)
{
  void *inVec;
  vtkDataArray *vectors = self->GetInput()->GetPointData()->
                            GetVectors(vectorsSelection);

  if (vectors == NULL)
    {
    return;
    }
  inVec = vectors->GetVoidPointer(0);

  // call templated function
  switch (vectors->GetDataType())
    {
    vtkTemplateMacro5(vtkWarpVectorExecute2,self, inPts, outPts, 
                      (VTK_TT *)(inVec), max);
    default:
      break;
    }  
}

//----------------------------------------------------------------------------
void vtkWarpVector::Execute()
{
  vtkPointSet *input = this->GetInput();
  vtkPointSet *output = this->GetOutput();
  vtkPoints *points;
  vtkIdType numPts;

  // First, copy the input to the output as a starting point
  output->CopyStructure( input );

  if (input == NULL || input->GetPoints() == NULL)
    {
    return;
    }
  numPts = input->GetPoints()->GetNumberOfPoints();
  if ( !input->GetPointData()->GetVectors(this->InputVectorsSelection) 
          || !numPts)
    {
    vtkDebugMacro(<<"No input data");
    return;
    }

  // SETUP AND ALLOCATE THE OUTPUT
  numPts = input->GetNumberOfPoints();
  points = input->GetPoints()->NewInstance();
  points->SetDataType(input->GetPoints()->GetDataType());
  points->Allocate(numPts);
  points->SetNumberOfPoints(numPts);
  output->SetPoints(points);
  points->Delete();

  void *inPtr = input->GetPoints()->GetVoidPointer(0);
  void *outPtr = output->GetPoints()->GetVoidPointer(0);

  // call templated function
  switch (input->GetPoints()->GetDataType())
    {
    vtkTemplateMacro5(vtkWarpVectorExecute, this, 
                      (VTK_TT *)(inPtr), (VTK_TT *)(outPtr), numPts, this->InputVectorsSelection);
    default:
      break;
    }
  
  // now pass the data.
  output->GetPointData()->CopyNormalsOff(); // distorted geometry
  output->GetPointData()->PassData(input->GetPointData());
  output->GetCellData()->PassData(input->GetCellData());
}

void vtkWarpVector::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if (this->InputVectorsSelection)
    {
    os << indent << "InputVectorsSelection: " << this->InputVectorsSelection;
    } 
  os << indent << "Scale Factor: " << this->ScaleFactor << "\n";
}
