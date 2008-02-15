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
#include "vtkXMLHierarchicalBoxDataWriter.h"

#include "vtkAMRBox.h"
#include "vtkCompositeDataIterator.h"
#include "vtkHierarchicalBoxDataSet.h"
#include "vtkInformation.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"
#include "vtkXMLDataElement.h"
#include "vtkUniformGrid.h"

vtkStandardNewMacro(vtkXMLHierarchicalBoxDataWriter);
vtkCxxRevisionMacro(vtkXMLHierarchicalBoxDataWriter, "$Revision$");
//----------------------------------------------------------------------------
vtkXMLHierarchicalBoxDataWriter::vtkXMLHierarchicalBoxDataWriter()
{
}

//----------------------------------------------------------------------------
vtkXMLHierarchicalBoxDataWriter::~vtkXMLHierarchicalBoxDataWriter()
{
}

//----------------------------------------------------------------------------
int vtkXMLHierarchicalBoxDataWriter::FillInputPortInformation(
  int vtkNotUsed(port), vtkInformation* info)
{
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkHierarchicalBoxDataSet");
  return 1;
}

//----------------------------------------------------------------------------
int vtkXMLHierarchicalBoxDataWriter::WriteComposite(vtkCompositeDataSet* compositeData, 
    vtkXMLDataElement* parent, int &writerIdx)
{
  vtkHierarchicalBoxDataSet* hboxData = vtkHierarchicalBoxDataSet::SafeDownCast(compositeData);

  unsigned int numLevels = hboxData->GetNumberOfLevels();
  // Iterate over each level.
  for (unsigned int level=0; level < numLevels; level++)
    {
    vtkSmartPointer<vtkXMLDataElement> block = vtkSmartPointer<vtkXMLDataElement>::New();
    block->SetName("Block");
    block->SetIntAttribute("level", level);
    block->SetIntAttribute("refinement_ratio", hboxData->GetRefinementRatio(level));
    unsigned int numDS = hboxData->GetNumberOfDataSets(level);
    for (unsigned int cc=0; cc < numDS; cc++)
      {
      vtkAMRBox box;
      int vec_box[6];
      vtkUniformGrid* ug = hboxData->GetDataSet(level, cc, box);
      vtkSmartPointer<vtkXMLDataElement> datasetXML = 
        vtkSmartPointer<vtkXMLDataElement>::New();
      datasetXML->SetName("DataSet");
      datasetXML->SetIntAttribute("index", cc);
      vec_box[0] = box.LoCorner[0];
      vec_box[1] = box.HiCorner[0];
      vec_box[2] = box.LoCorner[1];
      vec_box[3] = box.HiCorner[1];
      vec_box[4] = box.LoCorner[2];
      vec_box[5] = box.HiCorner[2];
      datasetXML->SetVectorAttribute("amr_box", 6, vec_box);
      if (!this->WriteNonCompositeData(ug, datasetXML, writerIdx))
        {
        return 0;
        } 
      block->AddNestedElement(datasetXML);
      }
    parent->AddNestedElement(block);
    }
  return 1;
}


//----------------------------------------------------------------------------
void vtkXMLHierarchicalBoxDataWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

