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
#include "vtkXMLStructuredDataReader.h"
#include "vtkXMLDataElement.h"
#include "vtkXMLDataParser.h"
#include "vtkDataSet.h"

vtkCxxRevisionMacro(vtkXMLStructuredDataReader, "$Revision$");

//----------------------------------------------------------------------------
vtkXMLStructuredDataReader::vtkXMLStructuredDataReader()
{
  this->PieceExtents = 0;
  this->PiecePointDimensions = 0;
  this->PiecePointIncrements = 0;
  this->PieceCellDimensions = 0;
  this->PieceCellIncrements = 0;
  this->WholeSlices = 1;

  // Initialize these in case someone calls GetNumberOfPoints or
  // GetNumberOfCells before UpdateInformation is called.
  this->PointDimensions[0] = 0;
  this->PointDimensions[1] = 0;
  this->PointDimensions[2] = 0;
  this->CellDimensions[0] = 0;
  this->CellDimensions[1] = 0;
  this->CellDimensions[2] = 0;
}

//----------------------------------------------------------------------------
vtkXMLStructuredDataReader::~vtkXMLStructuredDataReader()
{
  if(this->NumberOfPieces) { this->DestroyPieces(); }
}

//----------------------------------------------------------------------------
void vtkXMLStructuredDataReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "WholeSlices: " << this->WholeSlices << "\n";
}

//----------------------------------------------------------------------------
int vtkXMLStructuredDataReader::ReadPrimaryElement(vtkXMLDataElement* ePrimary)
{
  // Set the output's whole extent.
  int extent[6];
  if(ePrimary->GetVectorAttribute("WholeExtent", 6, extent) == 6)
    {
    this->GetOutputAsDataSet()->SetWholeExtent(extent);
    }
  else
    {
    vtkErrorMacro(<< this->GetDataSetName() << " element has no WholeExtent.");
    return 0;
    }
  
  return this->Superclass::ReadPrimaryElement(ePrimary);  
}

//----------------------------------------------------------------------------
void vtkXMLStructuredDataReader::SetupEmptyOutput()
{
  // Special extent to indicate no input.
  this->GetOutputAsDataSet()->SetUpdateExtent(1, 0, 1, 0, 1, 0);
}

//----------------------------------------------------------------------------
void vtkXMLStructuredDataReader::SetupPieces(int numPieces)
{
  this->Superclass::SetupPieces(numPieces);
  this->PieceExtents = new int[numPieces*6];
  this->PiecePointDimensions = new int[numPieces*3];
  this->PiecePointIncrements = new int[numPieces*3];
  this->PieceCellDimensions = new int[numPieces*3];
  this->PieceCellIncrements = new int[numPieces*3];
  int i;
  for(i=0; i < numPieces; ++i)
    {
    int* extent = this->PieceExtents + i*6;
    extent[0]=0; extent[1]=-1;
    extent[2]=0; extent[3]=-1;
    extent[4]=0; extent[5]=-1;
    }
}

//----------------------------------------------------------------------------
void vtkXMLStructuredDataReader::DestroyPieces()
{
  delete [] this->PieceExtents;
  delete [] this->PiecePointDimensions;
  delete [] this->PiecePointIncrements;
  delete [] this->PieceCellDimensions;
  delete [] this->PieceCellIncrements;
  this->PieceExtents = 0;
  this->PiecePointDimensions = 0;
  this->PiecePointIncrements = 0;
  this->PieceCellDimensions = 0;
  this->PieceCellIncrements = 0;
}

//----------------------------------------------------------------------------
vtkIdType vtkXMLStructuredDataReader::GetNumberOfPoints()
{
  return (this->PointDimensions[0]*
          this->PointDimensions[1]*
          this->PointDimensions[2]);
}

//----------------------------------------------------------------------------
vtkIdType vtkXMLStructuredDataReader::GetNumberOfCells()
{
  return (this->CellDimensions[0]*
          this->CellDimensions[1]*
          this->CellDimensions[2]);
}

//----------------------------------------------------------------------------
int vtkXMLStructuredDataReader::ReadPiece(vtkXMLDataElement* ePiece)
{
  if(!this->Superclass::ReadPiece(ePiece)) { return 0; }
  int* pieceExtent = this->PieceExtents + this->Piece*6;
  
  // Read the extent of the piece.
  if(strcmp(ePiece->GetName(), "Piece") == 0)
    {
    if(!ePiece->GetAttribute("Extent"))
      {
      vtkErrorMacro("Piece has no extent.");
      }
    if(ePiece->GetVectorAttribute("Extent", 6, pieceExtent) < 6)
      {
      vtkErrorMacro("Extent attribute is not 6 integers.");
      return 0;
      }
    }
  else if(ePiece->GetVectorAttribute("WholeExtent", 6, pieceExtent) < 6)
    {
    vtkErrorMacro("WholeExtent attribute is not 6 integers.");
    return 0;
    }
  
  // Compute the dimensions and increments for this piece's extent.
  int* piecePointDimensions = this->PiecePointDimensions + this->Piece*3;
  int* piecePointIncrements = this->PiecePointIncrements + this->Piece*3;
  int* pieceCellDimensions = this->PieceCellDimensions + this->Piece*3;
  int* pieceCellIncrements = this->PieceCellIncrements + this->Piece*3;  
  this->ComputeDimensions(pieceExtent, piecePointDimensions, 1);
  this->ComputeIncrements(pieceExtent, piecePointIncrements, 1); 
  this->ComputeDimensions(pieceExtent, pieceCellDimensions, 0);
  this->ComputeIncrements(pieceExtent, pieceCellIncrements, 0);
  
  return 1;
}

//----------------------------------------------------------------------------
void vtkXMLStructuredDataReader::ReadXMLData()
{
  // Get the requested Update Extent.
  this->GetOutputAsDataSet()->GetUpdateExtent(this->UpdateExtent);
  
  vtkDebugMacro("Updating extent "
                << this->UpdateExtent[0] << " " << this->UpdateExtent[1] << " "
                << this->UpdateExtent[2] << " " << this->UpdateExtent[3] << " "
                << this->UpdateExtent[4] << " " << this->UpdateExtent[5]
                << "\n");
  
  // Prepare increments for the update extent.
  this->ComputeDimensions(this->UpdateExtent, this->PointDimensions, 1);
  this->ComputeIncrements(this->UpdateExtent, this->PointIncrements, 1);
  this->ComputeDimensions(this->UpdateExtent, this->CellDimensions, 0);
  this->ComputeIncrements(this->UpdateExtent, this->CellIncrements, 0);  
  
  // Let superclasses read data.  This also allocates output data.
  this->Superclass::ReadXMLData();
  
  // Read the data needed from each piece.
  int i;
  for(i=0;i < this->NumberOfPieces;++i)
    {
    int* pieceExtent = this->PieceExtents + i*6;
    // Intersect the extents to get the part we need to read.
    if(this->IntersectExtents(pieceExtent, this->UpdateExtent,
                              this->SubExtent))
      {
      vtkDebugMacro("Reading extent "
                    << this->SubExtent[0] << " " << this->SubExtent[1] << " "
                    << this->SubExtent[2] << " " << this->SubExtent[3] << " "
                    << this->SubExtent[4] << " " << this->SubExtent[5]
                    << " from piece " << i);
      
      this->ComputeDimensions(this->SubExtent, this->SubPointDimensions, 1);
      this->ComputeDimensions(this->SubExtent, this->SubCellDimensions, 0);
      
      // Read the data from this piece.
      this->ReadPieceData(i);
      }
    }
  
  // We filled the exact update extent in the output.
  this->SetOutputExtent(this->UpdateExtent);  
}

//----------------------------------------------------------------------------
int vtkXMLStructuredDataReader::ReadArrayForPoints(vtkXMLDataElement* da,
                                                   vtkDataArray* outArray)
{
  int* pieceExtent = this->PieceExtents + this->Piece*6;
  int* piecePointDimensions = this->PiecePointDimensions + this->Piece*3;
  int* piecePointIncrements = this->PiecePointIncrements + this->Piece*3;
  if(!this->ReadSubExtent(pieceExtent, piecePointDimensions,
                          piecePointIncrements, this->UpdateExtent,
                          this->PointDimensions, this->PointIncrements,
                          this->SubExtent, this->SubPointDimensions,
                          da, outArray))
    {
    vtkErrorMacro("Error reading extent "
                  << this->SubExtent[0] << " " << this->SubExtent[1] << " "
                  << this->SubExtent[2] << " " << this->SubExtent[3] << " "
                  << this->SubExtent[4] << " " << this->SubExtent[5]
                  << " from piece " << this->Piece);
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------------
int vtkXMLStructuredDataReader::ReadArrayForCells(vtkXMLDataElement* da,
                                                  vtkDataArray* outArray)
{
  int* pieceExtent = this->PieceExtents + this->Piece*6;
  int* pieceCellDimensions = this->PieceCellDimensions + this->Piece*3;
  int* pieceCellIncrements = this->PieceCellIncrements + this->Piece*3;
  if(!this->ReadSubExtent(pieceExtent, pieceCellDimensions,
                          pieceCellIncrements, this->UpdateExtent,
                          this->CellDimensions, this->CellIncrements,
                          this->SubExtent, this->SubCellDimensions,
                          da, outArray))
    {
    vtkErrorMacro("Error reading extent "
                  << this->SubExtent[0] << " " << this->SubExtent[1] << " "
                  << this->SubExtent[2] << " " << this->SubExtent[3] << " "
                  << this->SubExtent[4] << " " << this->SubExtent[5]
                  << " from piece " << this->Piece);
    return 0;
    }
  return 1;
}

//----------------------------------------------------------------------------
int
vtkXMLStructuredDataReader
::ReadSubExtent(int* inExtent, int* inDimensions, int* inIncrements,
                int* outExtent, int* outDimensions, int* outIncrements,
                int* subExtent, int* subDimensions, vtkXMLDataElement* da,
                vtkDataArray* array)
{
  unsigned int components = array->GetNumberOfComponents();
  
  if((inDimensions[0] == outDimensions[0]) &&
     (inDimensions[1] == outDimensions[1]))
    {
    if(inDimensions[2] == outDimensions[2])
      {
      // Read the whole volume at once.
      unsigned int volumeTuples =
        (inDimensions[0]*inDimensions[1]*inDimensions[2]);
      if(!this->ReadData(da, array->GetVoidPointer(0), array->GetDataType(),
                         0, volumeTuples*components))
        {
        return 0;
        }
      }
    else
      {
      // Read an entire slice at a time.
      unsigned int sliceTuples = inDimensions[0]*inDimensions[1];
      int k;
      for(k=0;k < subDimensions[2];++k)
        {
        unsigned int sourceTuple =
          this->GetStartTuple(inExtent, inIncrements,
                              subExtent[0], subExtent[2], subExtent[4]+k);
        unsigned int destTuple =
          this->GetStartTuple(outExtent, outIncrements,
                              subExtent[0], subExtent[2], subExtent[4]+k);
        if(!this->ReadData(da, array->GetVoidPointer(destTuple*components),
                           array->GetDataType(), sourceTuple*components,
                           sliceTuples*components))
          {
          return 0;
          }
        }
      }
    }
  else
    {
    if(!this->WholeSlices)
      {
      // Read a row at a time.
      unsigned int rowTuples = subDimensions[0];
      int j,k;
      for(k=0;k < subDimensions[2];++k)
        {
        for(j=0;j < subDimensions[1];++j)
          {
          unsigned int sourceTuple =
            this->GetStartTuple(inExtent, inIncrements,
                                subExtent[0], subExtent[2]+j, subExtent[4]+k);
          unsigned int destTuple =
            this->GetStartTuple(outExtent, outIncrements,
                                subExtent[0], subExtent[2]+j, subExtent[4]+k);
          if(!this->ReadData(da, array->GetVoidPointer(destTuple*components),
                             array->GetDataType(), sourceTuple*components,
                             rowTuples*components))
            {
            return 0;
            }
          }
        }
      }
    else
      {
      // Read in each slice and copy the needed rows from it.
      unsigned int rowTuples = subDimensions[0];
      unsigned int partialSliceTuples = inDimensions[0]*subDimensions[1];
      unsigned long tupleSize = components*array->GetDataTypeSize();
      vtkDataArray* temp = array->MakeObject();
      temp->SetNumberOfTuples(partialSliceTuples);
      int k;
      for(k=0;k < subDimensions[2];++k)
        {
        unsigned int inTuple =
          this->GetStartTuple(inExtent, inIncrements,
                              inExtent[0], subExtent[2], subExtent[4]+k);
        int memExtent[6];
        memExtent[0] = inExtent[0];
        memExtent[1] = inExtent[1];
        memExtent[2] = subExtent[2];
        memExtent[3] = subExtent[3];
        memExtent[4] = subExtent[4]+k;
        memExtent[5] = subExtent[4]+k;
        if(!this->ReadData(da, temp->GetVoidPointer(0), temp->GetDataType(),
                           inTuple*components,
                           partialSliceTuples*components))
          {
          temp->Delete();
          return 0;
          }
        int j;
        for(j=0;j < subDimensions[1];++j)
          {
          unsigned int sourceTuple =
            this->GetStartTuple(memExtent, inIncrements,
                                subExtent[0], subExtent[2]+j, subExtent[4]+k);
          unsigned int destTuple =
            this->GetStartTuple(outExtent, outIncrements,
                                subExtent[0], subExtent[2]+j, subExtent[4]+k);
          memcpy(array->GetVoidPointer(destTuple*components),
                 temp->GetVoidPointer(sourceTuple*components),
                 tupleSize*rowTuples);
          }
        }
      temp->Delete();
      }
    }
  return 1;
}
