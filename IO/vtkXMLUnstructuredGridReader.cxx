/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkXMLUnstructuredGridReader.h"

#include "vtkCellArray.h"
#include "vtkIntArray.h"
#include "vtkObjectFactory.h"
#include "vtkUnsignedCharArray.h"
#include "vtkUnstructuredGrid.h"
#include "vtkXMLDataElement.h"

vtkCxxRevisionMacro(vtkXMLUnstructuredGridReader, "$Revision$");
vtkStandardNewMacro(vtkXMLUnstructuredGridReader);

//----------------------------------------------------------------------------
vtkXMLUnstructuredGridReader::vtkXMLUnstructuredGridReader()
{
  // Copied from vtkUnstructuredGridReader constructor:
  this->SetOutput(vtkUnstructuredGrid::New());
  // Releasing data for pipeline parallism.
  // Filters will know it is empty. 
  this->Outputs[0]->ReleaseData();
  this->Outputs[0]->Delete();
  
  this->CellElements = 0;
  this->TotalNumberOfCells = 0;
}

//----------------------------------------------------------------------------
vtkXMLUnstructuredGridReader::~vtkXMLUnstructuredGridReader()
{
  if(this->NumberOfPieces) { this->DestroyPieces(); }
}

//----------------------------------------------------------------------------
void vtkXMLUnstructuredGridReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkXMLUnstructuredGridReader::SetOutput(vtkUnstructuredGrid *output)
{
  this->Superclass::SetNthOutput(0, output);
}

//----------------------------------------------------------------------------
vtkUnstructuredGrid* vtkXMLUnstructuredGridReader::GetOutput()
{
  if(this->NumberOfOutputs < 1)
    {
    return 0;
    }
  return static_cast<vtkUnstructuredGrid*>(this->Outputs[0]);
}

//----------------------------------------------------------------------------
const char* vtkXMLUnstructuredGridReader::GetDataSetName()
{
  return "UnstructuredGrid";
}

//----------------------------------------------------------------------------
void vtkXMLUnstructuredGridReader::GetOutputUpdateExtent(int& piece,
                                                         int& numberOfPieces,
                                                         int& ghostLevel)
{
  this->GetOutput()->GetUpdateExtent(piece, numberOfPieces, ghostLevel);
}

//----------------------------------------------------------------------------
void vtkXMLUnstructuredGridReader::SetupOutputTotals()
{
  this->Superclass::SetupOutputTotals();
  // Find the total size of the output.
  int i;
  this->TotalNumberOfCells = 0;
  for(i=this->StartPiece; i < this->EndPiece; ++i)
    {
    this->TotalNumberOfCells += this->NumberOfCells[i];
    }
  
  // Data reading will start at the beginning of the output.
  this->StartCell = 0;
}

//----------------------------------------------------------------------------
void vtkXMLUnstructuredGridReader::SetupPieces(int numPieces)
{
  this->Superclass::SetupPieces(numPieces);
  this->NumberOfCells = new vtkIdType[numPieces];
  this->CellElements = new vtkXMLDataElement*[numPieces];
  int i;
  for(i=0;i < numPieces; ++i)
    {
    this->CellElements[i] = 0;
    }
}

//----------------------------------------------------------------------------
void vtkXMLUnstructuredGridReader::DestroyPieces()
{
  delete [] this->CellElements;
  delete [] this->NumberOfCells;
  this->Superclass::DestroyPieces();
}

//----------------------------------------------------------------------------
void vtkXMLUnstructuredGridReader::SetupOutputData()
{
  this->Superclass::SetupOutputData();
  
  vtkUnstructuredGrid* output = this->GetOutput();
  
  // Setup the output's cell arrays.
  vtkUnsignedCharArray* cellTypes = vtkUnsignedCharArray::New();
  cellTypes->SetNumberOfTuples(this->GetNumberOfCells());
  vtkCellArray* outCells = vtkCellArray::New();
  
  vtkIntArray* locations = vtkIntArray::New();
  locations->SetNumberOfTuples(this->GetNumberOfCells());
  
  output->SetCells(cellTypes, locations, outCells);
  
  locations->Delete();
  outCells->Delete();
  cellTypes->Delete();
}

//----------------------------------------------------------------------------
int vtkXMLUnstructuredGridReader::ReadPiece(vtkXMLDataElement* ePiece)
{
  if(!this->Superclass::ReadPiece(ePiece)) { return 0; }
  int i;
  
  if(!ePiece->GetScalarAttribute("NumberOfCells",
                                 this->NumberOfCells[this->Piece]))
    {
    vtkErrorMacro("Piece " << this->Piece
                  << " is missing its NumberOfCells attribute.");
    this->NumberOfCells[this->Piece] = 0;
    return 0;
    }  
  
  // Find the Cells element in the piece.
  this->CellElements[this->Piece] = 0;
  for(i=0; i < ePiece->GetNumberOfNestedElements(); ++i)
    {
    vtkXMLDataElement* eNested = ePiece->GetNestedElement(i);
    if((strcmp(eNested->GetName(), "Cells") == 0)
       && (eNested->GetNumberOfNestedElements() > 0))
      {
      this->CellElements[this->Piece] = eNested;
      }
    }
  
  if(!this->CellElements[this->Piece])
    {
    vtkErrorMacro("A piece is missing its Cells element.");
    return 0;
    }
  
  return 1;
}

//----------------------------------------------------------------------------
void vtkXMLUnstructuredGridReader::SetupNextPiece()
{
  this->Superclass::SetupNextPiece();  
  this->StartCell += this->NumberOfCells[this->Piece];
}

//----------------------------------------------------------------------------
int vtkXMLUnstructuredGridReader::ReadPieceData()
{
  if(!this->Superclass::ReadPieceData()) { return 0; }
  
  vtkUnstructuredGrid* output = this->GetOutput();
  
  // Save the start location where the new cell connectivity will be
  // appended.
  vtkIdType startLoc = 0;
  if(output->GetCells()->GetData())
    {
    startLoc = output->GetCells()->GetData()->GetNumberOfTuples();
    }
  
  // Read the Cells.
  if(!this->ReadCellArray(this->NumberOfCells[this->Piece],
                          this->TotalNumberOfCells,
                          this->CellElements[this->Piece],
                          output->GetCells()))
    {
    return 0;
    }
  
  // Construct the cell locations.
  vtkIntArray* locations = output->GetCellLocationsArray();
  int* locs = locations->GetPointer(this->StartCell);
  vtkIdType* begin = output->GetCells()->GetData()->GetPointer(startLoc);
  vtkIdType* cur = begin;
  vtkIdType i;
  for(i=0; i < this->NumberOfCells[this->Piece]; ++i)
    {
    locs[i] = startLoc + cur - begin;
    cur += *cur + 1;
    }
  
  // Read the cooresponding cell types.
  vtkIdType numberOfCells = this->NumberOfCells[this->Piece];
  vtkXMLDataElement* eCells = this->CellElements[this->Piece];
  vtkXMLDataElement* eTypes = this->FindDataArrayWithName(eCells, "types");
  if(!eTypes)
    {
    vtkErrorMacro("Cannot read cell types from " << eCells->GetName()
                  << " in piece " << this->Piece
                  << " because the \"types\" array could not be found.");
    return 0;
    }
  vtkDataArray* c2 = this->CreateDataArray(eTypes);
  if(!c2 || (c2->GetNumberOfComponents() != 1))
    {
    vtkErrorMacro("Cannot read cell types from " << eCells->GetName()
                  << " in piece " << this->Piece
                  << " because the \"types\" array could not be created"
                  << " with one component.");
    return 0;
    }
  c2->SetNumberOfTuples(numberOfCells);
  if(!this->ReadData(eTypes, c2->GetVoidPointer(0),
                     c2->GetDataType(), 0, numberOfCells))
    {
    vtkErrorMacro("Cannot read cell types from " << eCells->GetName()
                  << " in piece " << this->Piece
                  << " because the \"types\" array is not long enough.");
    return 0;
    }
  vtkUnsignedCharArray* cellTypes = this->ConvertToUnsignedCharArray(c2);
  if(!cellTypes)
    {
    vtkErrorMacro("Cannot read cell types from " << eCells->GetName()
                  << " in piece " << this->Piece
                  << " because the \"types\" array could not be converted"
                  << " to a vtkUnsignedCharArray.");
    return 0;
    }
  
  // Copy the cell type data.
  memcpy(output->GetCellTypesArray()->GetPointer(this->StartCell),
         cellTypes->GetPointer(0), numberOfCells);
  
  cellTypes->Delete();
    
  return 1;
}

//----------------------------------------------------------------------------
int vtkXMLUnstructuredGridReader::ReadArrayForCells(vtkXMLDataElement* da,
                                                    vtkDataArray* outArray)
{
  vtkIdType startCell = this->StartCell;
  vtkIdType numCells = this->NumberOfCells[this->Piece];  
  vtkIdType components = outArray->GetNumberOfComponents();
  return this->ReadData(da, outArray->GetVoidPointer(startCell*components),
                        outArray->GetDataType(), 0, numCells*components);
}
