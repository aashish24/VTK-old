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
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/

#include "vtkVertexListIterator.h"

#include "vtkObjectFactory.h"
#include "vtkGraph.h"

vtkCxxRevisionMacro(vtkVertexListIterator, "$Revision$");
vtkStandardNewMacro(vtkVertexListIterator);
//----------------------------------------------------------------------------
vtkVertexListIterator::vtkVertexListIterator()
{
  this->Current = 0;
  this->End = 0;
  this->Graph = 0;
}

//----------------------------------------------------------------------------
vtkVertexListIterator::~vtkVertexListIterator()
{
  if (this->Graph)
    {
    this->Graph->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkVertexListIterator::SetGraph(vtkGraph *graph)
{
  vtkSetObjectBodyMacro(Graph, vtkGraph, graph);
  if (this->Graph)
    {
    this->Current = 0;
    this->End = this->Graph->GetNumberOfVertices();
    }
}

//----------------------------------------------------------------------------
void vtkVertexListIterator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Graph: " << (this->Graph ? "" : "(null)") << endl;
  if (this->Graph)
    {
    this->Graph->PrintSelf(os, indent.GetNextIndent());
    }
}
