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
#include "vtkMultiBlockDataGroupFilter.h"

#include "vtkCellData.h"
#include "vtkDataSet.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"

vtkCxxRevisionMacro(vtkMultiBlockDataGroupFilter, "$Revision$");
vtkStandardNewMacro(vtkMultiBlockDataGroupFilter);
//-----------------------------------------------------------------------------
vtkMultiBlockDataGroupFilter::vtkMultiBlockDataGroupFilter()
{
}

//-----------------------------------------------------------------------------
vtkMultiBlockDataGroupFilter::~vtkMultiBlockDataGroupFilter()
{
}

//-----------------------------------------------------------------------------
int vtkMultiBlockDataGroupFilter::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  vtkInformation* info = outputVector->GetInformationObject(0);
  vtkMultiBlockDataSet* output = vtkMultiBlockDataSet::SafeDownCast(
    info->Get(vtkDataObject::DATA_OBJECT()));
  if (!output) 
    {
    return 0;
    }

  /*
  unsigned int updatePiece = static_cast<unsigned int>(
    info->Get(vtkStreamingDemandDrivenPipeline::UPDATE_PIECE_NUMBER()));
  unsigned int updateNumPieces =  static_cast<unsigned int>(
    info->Get(vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_PIECES()));
  */

  int numInputs = inputVector[0]->GetNumberOfInformationObjects();
  output->SetNumberOfBlocks(numInputs);
  for (int idx = 0; idx < numInputs; ++idx)
    {
    /*
    // This can be a vtkMultiPieceDataSet if we ever support it.
    vtkMultiBlockDataSet* block = vtkMultiBlockDataSet::New();
    block->SetNumberOfBlocks(updateNumPieces);
    block->Delete();
    */

    vtkDataObject* input = 0;
    vtkInformation* inInfo = inputVector[0]->GetInformationObject(idx);
    if (inInfo)
      {
      input = inInfo->Get(vtkDataObject::DATA_OBJECT());
      }
    if (input)
      {
      vtkDataObject* dsCopy = input->NewInstance();
      dsCopy->ShallowCopy(input);
      output->SetBlock(idx, dsCopy);
      dsCopy->Delete();
      }
    else
      {
      output->SetBlock(idx, 0);
      }
    }

  return 1;
}

//-----------------------------------------------------------------------------
void vtkMultiBlockDataGroupFilter::AddInput(vtkDataObject* input)
{
  this->AddInput(0, input);
}

//-----------------------------------------------------------------------------
void vtkMultiBlockDataGroupFilter::AddInput(int index, vtkDataObject* input)
{
  if(input)
    {
    this->AddInputConnection(index, input->GetProducerPort());
    }
}

//-----------------------------------------------------------------------------
int vtkMultiBlockDataGroupFilter::FillInputPortInformation(
  int, vtkInformation *info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataObject");
  info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
  info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
  return 1;
}

//-----------------------------------------------------------------------------
void vtkMultiBlockDataGroupFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
