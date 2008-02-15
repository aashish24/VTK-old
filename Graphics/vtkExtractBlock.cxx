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
#include "vtkExtractBlock.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkDataSet.h"
#include "vtkCompositeDataIterator.h"
#include "vtkInformationIntegerKey.h"
#include "vtkMultiPieceDataSet.h"

#include <vtkstd/set>

class vtkExtractBlock::vtkSet : public vtkstd::set<unsigned int>
{
};

vtkStandardNewMacro(vtkExtractBlock);
vtkCxxRevisionMacro(vtkExtractBlock, "$Revision$");
vtkInformationKeyMacro(vtkExtractBlock, DONT_PRUNE, Integer);
//----------------------------------------------------------------------------
vtkExtractBlock::vtkExtractBlock()
{
  this->Indices = new vtkExtractBlock::vtkSet();
  this->PruneOutput = 1;
}

//----------------------------------------------------------------------------
vtkExtractBlock::~vtkExtractBlock()
{
  delete this->Indices;
}

//----------------------------------------------------------------------------
void vtkExtractBlock::AddIndex(unsigned int index)
{
  this->Indices->insert(index);
  this->Modified();
}


//----------------------------------------------------------------------------
void vtkExtractBlock::RemoveIndex(unsigned int index)
{
  this->Indices->erase(index);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkExtractBlock::RemoveAllIndices()
{
  this->Indices->clear();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkExtractBlock::CopySubTree(vtkCompositeDataIterator* loc, 
  vtkMultiBlockDataSet* output, vtkMultiBlockDataSet* input)
{
  vtkDataObject* inputNode = input->GetDataSet(loc);
  if (!inputNode->IsA("vtkCompositeDataSet"))
    {
    vtkDataObject* clone = inputNode->NewInstance();
    clone->ShallowCopy(inputNode);
    output->SetDataSet(loc, clone);
    clone->Delete();
    }
  else
    {
    vtkCompositeDataSet* cinput = vtkCompositeDataSet::SafeDownCast(inputNode);
    vtkCompositeDataSet* coutput = vtkCompositeDataSet::SafeDownCast(
      output->GetDataSet(loc));
    vtkCompositeDataIterator* iter = cinput->NewIterator();
    for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
      {
      vtkDataObject* curNode = iter->GetCurrentDataObject();
      vtkDataObject* clone = curNode->NewInstance();
      clone->ShallowCopy(curNode);
      coutput->SetDataSet(iter, clone);
      clone->Delete();
      }
    iter->Delete();
    }
}

//----------------------------------------------------------------------------
int vtkExtractBlock::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  vtkMultiBlockDataSet *input = vtkMultiBlockDataSet::GetData(inputVector[0], 0);
  vtkMultiBlockDataSet *output = vtkMultiBlockDataSet::GetData(outputVector, 0);

  if (this->Indices->find(0) != this->Indices->end())
    {
    // trivial case.
    output->ShallowCopy(input);
    return 1;
    }

  output->CopyStructure(input);

  // Copy selected blocks over to the output.
  vtkCompositeDataIterator* iter = input->NewIterator();
  iter->VisitOnlyLeavesOff();

  for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
    {
    if (this->Indices->find(iter->GetCurrentFlatIndex()) != this->Indices->end())
      {
      this->CopySubTree(iter, output, input);
      // TODO: avoid copying if subtree has already been copied over.
      }
    }
  iter->Delete();

  if (!this->PruneOutput)
    {
    return 1;
    }

  // Now prune the output tree.

  // Since in case multiple processes are involved, this process may have some
  // data-set pointers NULL. Hence, pruning cannot simply trim NULL ptrs, since
  // in that case we may end up with different structures on different
  // processess, which is a big NO-NO. Hence, we first flag nodes based on
  // whether they are being pruned or not.

  iter = output->NewIterator();
  iter->VisitOnlyLeavesOff();
  iter->SkipEmptyNodesOff();
  for (iter->InitTraversal(); !iter->IsDoneWithTraversal(); iter->GoToNextItem())
    {
    if (this->Indices->find(iter->GetCurrentFlatIndex()) != this->Indices->end())
      {
      iter->GetCurrentMetaData()->Set(DONT_PRUNE(), 1);
      }
    }
  iter->Delete();

  // Do the actual pruning. Only those branches are pruned which don't have
  // DON_PRUNE flag set.
  this->Prune(output);
  return 1;
}

//----------------------------------------------------------------------------
bool vtkExtractBlock::Prune(vtkDataObject* branch)
{
  if (branch->IsA("vtkMultiBlockDataSet"))
    {
    return this->Prune(vtkMultiBlockDataSet::SafeDownCast(branch));
    }
  else if (branch->IsA("vtkMultiPieceDataSet"))
    {
    return this->Prune(vtkMultiPieceDataSet::SafeDownCast(branch));
    }

  return true;
}

//----------------------------------------------------------------------------
bool vtkExtractBlock::Prune(vtkMultiPieceDataSet* mpiece)
{
  // * Remove any children on mpiece that don't have DONT_PRUNE set.
  vtkMultiPieceDataSet* clone = vtkMultiPieceDataSet::New();
  unsigned int index=0;
  unsigned int numChildren = mpiece->GetNumberOfPieces();
  for (unsigned int cc=0; cc<numChildren; cc++)
    {
    if (mpiece->HasMetaData(cc) && mpiece->GetMetaData(cc)->Has(DONT_PRUNE()))
      {
      clone->SetPiece(index, mpiece->GetPiece(cc));
      clone->GetMetaData(index)->Copy(mpiece->GetMetaData(cc));
      index++;
      }
    }
  mpiece->ShallowCopy(clone);
  clone->Delete();

  // tell caller to prune mpiece away if num of pieces is 0.
  return (mpiece->GetNumberOfPieces() == 0);
}

//----------------------------------------------------------------------------
bool vtkExtractBlock::Prune(vtkMultiBlockDataSet* mblock)
{
  vtkMultiBlockDataSet* clone = vtkMultiBlockDataSet::New();
  unsigned int index=0;
  unsigned int numChildren = mblock->GetNumberOfBlocks();
  for (unsigned int cc=0; cc < numChildren; cc++)
    {
    vtkDataObject* block = mblock->GetBlock(cc);
    if (mblock->HasMetaData(cc) && mblock->GetMetaData(cc)->Has(DONT_PRUNE()))
      {
      clone->SetBlock(index, block);
      clone->GetMetaData(index)->Copy(mblock->GetMetaData(cc));
      index++;
      }
    else if (block)
      {
      bool prune = this->Prune(block);
      if (!prune)
        {
        vtkMultiBlockDataSet* prunedBlock = vtkMultiBlockDataSet::SafeDownCast(block);
        if (prunedBlock && prunedBlock->GetNumberOfBlocks()==1)
          {
          // shrink redundant branches.
          clone->SetBlock(index, prunedBlock->GetBlock(0));
          if (prunedBlock->HasMetaData(static_cast<unsigned int>(0)))
            {
            clone->GetMetaData(index)->Copy(prunedBlock->GetMetaData(cc));
            }
          }
        else
          {
          clone->SetBlock(index, block);
          if (mblock->HasMetaData(cc))
            {
            clone->GetMetaData(index)->Copy(mblock->GetMetaData(cc));
            }
          }
        index++;
        }
      }
    }
  mblock->ShallowCopy(clone);
  clone->Delete();
  return (mblock->GetNumberOfBlocks() == 0);
}

//----------------------------------------------------------------------------
void vtkExtractBlock::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "PruneOutput: " << this->PruneOutput << endl;
}

