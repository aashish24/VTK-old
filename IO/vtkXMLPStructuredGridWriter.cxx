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
#include "vtkXMLPStructuredGridWriter.h"
#include "vtkObjectFactory.h"
#include "vtkXMLStructuredGridWriter.h"
#include "vtkErrorCode.h"
#include "vtkStructuredGrid.h"

vtkCxxRevisionMacro(vtkXMLPStructuredGridWriter, "$Revision$");
vtkStandardNewMacro(vtkXMLPStructuredGridWriter);

//----------------------------------------------------------------------------
vtkXMLPStructuredGridWriter::vtkXMLPStructuredGridWriter()
{
}

//----------------------------------------------------------------------------
vtkXMLPStructuredGridWriter::~vtkXMLPStructuredGridWriter()
{
}

//----------------------------------------------------------------------------
void vtkXMLPStructuredGridWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkXMLPStructuredGridWriter::SetInput(vtkStructuredGrid* input)
{
  this->vtkProcessObject::SetNthInput(0, input);
}

//----------------------------------------------------------------------------
vtkStructuredGrid* vtkXMLPStructuredGridWriter::GetInput()
{
  if(this->NumberOfInputs < 1)
    {
    return 0;
    }
  
  return static_cast<vtkStructuredGrid*>(this->Inputs[0]);
}

//----------------------------------------------------------------------------
const char* vtkXMLPStructuredGridWriter::GetDataSetName()
{
  return "PStructuredGrid";
}

//----------------------------------------------------------------------------
const char* vtkXMLPStructuredGridWriter::GetDefaultFileExtension()
{
  return "pvts";
}

//----------------------------------------------------------------------------
vtkXMLStructuredDataWriter*
vtkXMLPStructuredGridWriter::CreateStructuredPieceWriter()
{  
  // Create the writer for the piece.
  vtkXMLStructuredGridWriter* pWriter = vtkXMLStructuredGridWriter::New();
  pWriter->SetInput(this->GetInput());
  return pWriter;
}

//----------------------------------------------------------------------------
void vtkXMLPStructuredGridWriter::WritePData(vtkIndent indent)
{
  this->Superclass::WritePData(indent);
  if (this->ErrorCode == vtkErrorCode::OutOfDiskSpaceError)
    {
    return;
    }
  vtkStructuredGrid* input = this->GetInput();  
  this->WritePPoints(input->GetPoints(), indent);
}
