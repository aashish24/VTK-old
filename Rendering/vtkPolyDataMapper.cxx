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
#include "vtkPolyDataMapper.h"

#include "vtkRenderWindow.h"
#include "vtkGraphicsFactory.h"
#include "vtkPolyData.h"

vtkCxxRevisionMacro(vtkPolyDataMapper, "$Revision$");

//----------------------------------------------------------------------------
// Needed when we don't use the vtkStandardNewMacro.
vtkInstantiatorNewMacro(vtkPolyDataMapper);

//----------------------------------------------------------------------------
// return the correct type of PolyDataMapper 
vtkPolyDataMapper *vtkPolyDataMapper::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkGraphicsFactory::CreateInstance("vtkPolyDataMapper");
  return (vtkPolyDataMapper*)ret;
}


//----------------------------------------------------------------------------
vtkPolyDataMapper::vtkPolyDataMapper()
{
  this->Piece = 0;
  this->NumberOfPieces = 1;
  this->NumberOfSubPieces = 1;
  this->GhostLevel = 0;
}

void vtkPolyDataMapper::Render(vtkRenderer *ren, vtkActor *act) 
{
  int currentPiece, nPieces;
  vtkPolyData *input = this->GetInput();
  
  if (input == NULL)
    {
    vtkErrorMacro("Mapper has no input.");
    return;
    }
  
  nPieces = this->NumberOfPieces * this->NumberOfSubPieces;

  for(int i=0; i<this->NumberOfSubPieces; i++)
    {
    // If more than one pieces, render in loop.
    currentPiece = this->NumberOfSubPieces * this->Piece + i;
    input->SetUpdateExtent(currentPiece, nPieces, this->GhostLevel);
    this->RenderPiece(ren, act);
    }
}

//----------------------------------------------------------------------------
void vtkPolyDataMapper::SetInput(vtkPolyData *input)
{
  this->vtkProcessObject::SetNthInput(0, input);
}

//----------------------------------------------------------------------------
// Specify the input data or filter.
vtkPolyData *vtkPolyDataMapper::GetInput()
{
  if (this->NumberOfInputs < 1)
    {
    return NULL;
    }
  
  return (vtkPolyData *)(this->Inputs[0]);
}

// Update the network connected to this mapper.
void vtkPolyDataMapper::Update()
{
  int currentPiece, nPieces = this->NumberOfPieces;
  vtkPolyData* input = this->GetInput();

  // If the estimated pipeline memory usage is larger than
  // the memory limit, break the current piece into sub-pieces.
  if (this->GetInput()) 
    {
    currentPiece = this->NumberOfSubPieces * this->Piece;
    input->SetUpdateExtent(currentPiece, this->NumberOfSubPieces*nPieces, 
                           this->GhostLevel);
    }

  this->vtkMapper::Update();
}

// Get the bounds for the input of this mapper as 
// (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax).
float *vtkPolyDataMapper::GetBounds()
{
  static float bounds[] = {-1.0,1.0, -1.0,1.0, -1.0,1.0};

  if ( ! this->GetInput() ) 
    {
    return bounds;
    }
  else
    {
    this->Update();
    this->GetInput()->GetBounds(this->Bounds);
    // if the bounds indicate NAN and subpieces are being used then 
    // return NULL
    if (((this->Bounds[0] == -VTK_LARGE_FLOAT) || 
         (this->Bounds[0] == VTK_LARGE_FLOAT)) &&
        this->NumberOfSubPieces > 1)
      {
      return NULL;
      }
    return this->Bounds;
    }
}

void vtkPolyDataMapper::ShallowCopy(vtkAbstractMapper *mapper)
{
  vtkPolyDataMapper *m = vtkPolyDataMapper::SafeDownCast(mapper);
  if ( m != NULL )
    {
    this->SetInput(m->GetInput());
    this->SetGhostLevel(m->GetGhostLevel());
    this->SetNumberOfPieces(m->GetNumberOfPieces());
    this->SetNumberOfSubPieces(m->GetNumberOfSubPieces());
    }

  // Now do superclass
  this->vtkMapper::ShallowCopy(mapper);
}

void vtkPolyDataMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Piece : " << this->Piece << endl;
  os << indent << "NumberOfPieces : " << this->NumberOfPieces << endl;
  os << indent << "GhostLevel: " << this->GhostLevel << endl;
  os << indent << "Number of sub pieces: " << this->NumberOfSubPieces
     << endl;
}
