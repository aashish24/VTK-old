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
#include "vtkXMLPRectilinearGridReader.h"

#include "vtkDataArray.h"
#include "vtkObjectFactory.h"
#include "vtkRectilinearGrid.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLRectilinearGridReader.h"

vtkCxxRevisionMacro(vtkXMLPRectilinearGridReader, "$Revision$");
vtkStandardNewMacro(vtkXMLPRectilinearGridReader);

//----------------------------------------------------------------------------
vtkXMLPRectilinearGridReader::vtkXMLPRectilinearGridReader()
{
  // Copied from vtkRectilinearGridReader constructor:
  this->SetOutput(vtkRectilinearGrid::New());
  // Releasing data for pipeline parallism.
  // Filters will know it is empty. 
  this->Outputs[0]->ReleaseData();
  this->Outputs[0]->Delete();
}

//----------------------------------------------------------------------------
vtkXMLPRectilinearGridReader::~vtkXMLPRectilinearGridReader()
{
}

//----------------------------------------------------------------------------
void vtkXMLPRectilinearGridReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkXMLPRectilinearGridReader::SetOutput(vtkRectilinearGrid *output)
{
  this->Superclass::SetNthOutput(0, output);
}

//----------------------------------------------------------------------------
vtkRectilinearGrid* vtkXMLPRectilinearGridReader::GetOutput()
{
  if(this->NumberOfOutputs < 1)
    {
    return 0;
    }
  return static_cast<vtkRectilinearGrid*>(this->Outputs[0]);
}

//----------------------------------------------------------------------------
vtkRectilinearGrid* vtkXMLPRectilinearGridReader::GetOutput(int idx)
{
  return static_cast<vtkRectilinearGrid*>(this->Superclass::GetOutput(idx));
}

//----------------------------------------------------------------------------
vtkRectilinearGrid* vtkXMLPRectilinearGridReader::GetPieceInput(int index)
{
  vtkXMLRectilinearGridReader* reader =
    static_cast<vtkXMLRectilinearGridReader*>(this->PieceReaders[index]);
  return reader->GetOutput();
}

//----------------------------------------------------------------------------
const char* vtkXMLPRectilinearGridReader::GetDataSetName()
{
  return "PRectilinearGrid";
}

//----------------------------------------------------------------------------
void vtkXMLPRectilinearGridReader::SetOutputExtent(int* extent)
{
  this->GetOutput()->SetExtent(extent);
}

//----------------------------------------------------------------------------
void vtkXMLPRectilinearGridReader::GetPieceInputExtent(int index, int* extent)
{
  this->GetPieceInput(index)->GetExtent(extent);
}

//----------------------------------------------------------------------------
int
vtkXMLPRectilinearGridReader::ReadPrimaryElement(vtkXMLDataElement* ePrimary)
{
  if(!this->Superclass::ReadPrimaryElement(ePrimary)) { return 0; }
  
  // Find the PCoordinates element.
  this->PCoordinatesElement = 0;
  int i;
  int numNested = ePrimary->GetNumberOfNestedElements();
  for(i=0;i < numNested; ++i)
    {
    vtkXMLDataElement* eNested = ePrimary->GetNestedElement(i);
    if((strcmp(eNested->GetName(), "PCoordinates") == 0) &&
       (eNested->GetNumberOfNestedElements() == 3))
      {
      this->PCoordinatesElement = eNested;
      }
    }
  
  // If there is any volume, we require a PCoordinates element.
  if(!this->PCoordinatesElement)
    {
    int extent[6];
    this->GetOutput()->GetWholeExtent(extent);
    if((extent[0] <= extent[1]) && (extent[2] <= extent[3]) &&
       (extent[4] <= extent[5]))
      {
      vtkErrorMacro("Could not find PCoordinates element with 3 arrays.");
      return 0;
      }
    }
  
  return 1;
}

//----------------------------------------------------------------------------
void vtkXMLPRectilinearGridReader::SetupOutputInformation()
{
  this->Superclass::SetupOutputInformation();  
  vtkRectilinearGrid* output = this->GetOutput();  
  
  if(!this->PCoordinatesElement)
    {
    // Empty volume.
    return;
    }
  
  // Create the coordinate arrays.
  vtkXMLDataElement* xc = this->PCoordinatesElement->GetNestedElement(0);
  vtkXMLDataElement* yc = this->PCoordinatesElement->GetNestedElement(1);
  vtkXMLDataElement* zc = this->PCoordinatesElement->GetNestedElement(2);
  vtkDataArray* x = this->CreateDataArray(xc);
  vtkDataArray* y = this->CreateDataArray(yc);
  vtkDataArray* z = this->CreateDataArray(zc);
  if(x && y && z)
    {
    output->SetXCoordinates(x);
    output->SetYCoordinates(y);
    output->SetZCoordinates(z);
    x->Delete();
    y->Delete();
    z->Delete();
    }
  else
    {
    if(x) { x->Delete(); }
    if(y) { y->Delete(); }
    if(z) { z->Delete(); }
    this->InformationError = 1;
    }
}

//----------------------------------------------------------------------------
void vtkXMLPRectilinearGridReader::SetupOutputData()
{
  this->Superclass::SetupOutputData();
  
  // Allocate the coordinate arrays.
  vtkRectilinearGrid* output = this->GetOutput();  
  output->GetXCoordinates()->SetNumberOfTuples(this->PointDimensions[0]);
  output->GetYCoordinates()->SetNumberOfTuples(this->PointDimensions[1]);
  output->GetZCoordinates()->SetNumberOfTuples(this->PointDimensions[2]);  
}

//----------------------------------------------------------------------------
int vtkXMLPRectilinearGridReader::ReadPieceData()
{
  if(!this->Superclass::ReadPieceData()) { return 0; }
  
  // Copy the coordinates arrays from the input piece.
  vtkRectilinearGrid* input = this->GetPieceInput(this->Piece);
  vtkRectilinearGrid* output = this->GetOutput();
  this->CopySubCoordinates(this->SubPieceExtent, this->UpdateExtent,
                           this->SubExtent, input->GetXCoordinates(),
                           output->GetXCoordinates());
  this->CopySubCoordinates(this->SubPieceExtent+2, this->UpdateExtent+2,
                           this->SubExtent+2, input->GetYCoordinates(),
                           output->GetYCoordinates());
  this->CopySubCoordinates(this->SubPieceExtent+4, this->UpdateExtent+4,
                           this->SubExtent+4, input->GetZCoordinates(),
                           output->GetZCoordinates());
  
  return 1;
}

//----------------------------------------------------------------------------
vtkXMLDataReader* vtkXMLPRectilinearGridReader::CreatePieceReader()
{
  return vtkXMLRectilinearGridReader::New();
}

//----------------------------------------------------------------------------
void vtkXMLPRectilinearGridReader::CopySubCoordinates(int* inBounds,
                                                      int* outBounds,
                                                      int* subBounds,
                                                      vtkDataArray* inArray,
                                                      vtkDataArray* outArray)
{
  unsigned int components = inArray->GetNumberOfComponents();
  unsigned int tupleSize = inArray->GetDataTypeSize()*components;
  
  int destStartIndex = subBounds[0] - outBounds[0];
  int sourceStartIndex = subBounds[0] - inBounds[0];
  int length = subBounds[1] - subBounds[0] + 1;
  
  memcpy(outArray->GetVoidPointer(destStartIndex*components),
         inArray->GetVoidPointer(sourceStartIndex*components),
         length*tupleSize);
}
