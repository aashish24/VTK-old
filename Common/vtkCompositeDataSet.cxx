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
#include "vtkCompositeDataSet.h"

#include "vtkCompositeDataIterator.h"
#include "vtkCompositeDataVisitor.h"
#include "vtkDataSet.h"

vtkCxxRevisionMacro(vtkCompositeDataSet, "$Revision$");

//----------------------------------------------------------------------------
vtkCompositeDataSet::vtkCompositeDataSet()
{
}

//----------------------------------------------------------------------------
vtkCompositeDataSet::~vtkCompositeDataSet()
{
}

//----------------------------------------------------------------------------
void vtkCompositeDataSet::Initialize()
{
  this->Superclass::Initialize();
}

//----------------------------------------------------------------------------
void vtkCompositeDataSet::SetUpdateExtent(int piece, int numPieces, int ghostLevel)
{
  this->UpdatePiece = piece;
  this->UpdateNumberOfPieces = numPieces;
  this->UpdateGhostLevel = ghostLevel;
  this->UpdateExtentInitialized = 1;
}

//----------------------------------------------------------------------------
void vtkCompositeDataSet::GetUpdateExtent(int &piece, int &numPieces, int &ghostLevel)
{
  piece = this->UpdatePiece;
  numPieces = this->UpdateNumberOfPieces;
  ghostLevel = this->UpdateGhostLevel;
}


//----------------------------------------------------------------------------
void vtkCompositeDataSet::PrintSelf(ostream& os, vtkIndent indent)
{
  // this->UpdateExtent
  this->Superclass::PrintSelf(os,indent);
}

