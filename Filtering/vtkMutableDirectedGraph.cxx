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

#include "vtkMutableDirectedGraph.h"

#include "vtkGraphEdge.h"
#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
// class vtkMutableDirectedGraph
//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkMutableDirectedGraph, "$Revision$");
vtkStandardNewMacro(vtkMutableDirectedGraph);
//----------------------------------------------------------------------------
vtkMutableDirectedGraph::vtkMutableDirectedGraph()
{
  this->GraphEdge = vtkGraphEdge::New();
}

//----------------------------------------------------------------------------
vtkMutableDirectedGraph::~vtkMutableDirectedGraph()
{
  this->GraphEdge->Delete();
}

//----------------------------------------------------------------------------
vtkIdType vtkMutableDirectedGraph::AddVertex()
{
  return this->AddVertex(0);
}
//----------------------------------------------------------------------------
vtkIdType vtkMutableDirectedGraph::AddVertex(vtkVariantArray *propertyArr)
{
  vtkIdType vertex;
  this->AddVertexInternal(propertyArr, &vertex);
  return vertex;
}

//----------------------------------------------------------------------------
vtkIdType vtkMutableDirectedGraph::AddVertex(const vtkVariant& pedigreeId)
{
  vtkIdType vertex;
  this->AddVertexInternal(pedigreeId, &vertex);
  return vertex;
}

//----------------------------------------------------------------------------
vtkEdgeType vtkMutableDirectedGraph::AddEdge(vtkIdType u, vtkIdType v)
{
  return this->AddEdge(u, v, 0);
}

//----------------------------------------------------------------------------
vtkEdgeType vtkMutableDirectedGraph::AddEdge(vtkIdType u, vtkIdType v,
                                             vtkVariantArray *propertyArr)
{
  vtkEdgeType e;
  this->AddEdgeInternal(u, v, true, propertyArr, &e);
  return e;
}

//----------------------------------------------------------------------------
vtkEdgeType vtkMutableDirectedGraph::AddEdge(const vtkVariant& u, vtkIdType v,
                                             vtkVariantArray *propertyArr)
{
  vtkEdgeType e;
  this->AddEdgeInternal(u, v, true, propertyArr, &e);
  return e;
}

//----------------------------------------------------------------------------
vtkEdgeType vtkMutableDirectedGraph::AddEdge(vtkIdType u, const vtkVariant& v,
                                             vtkVariantArray *propertyArr)
{
  vtkEdgeType e;
  this->AddEdgeInternal(u, v, true, propertyArr, &e);
  return e;
}

//----------------------------------------------------------------------------
vtkEdgeType vtkMutableDirectedGraph::AddEdge(const vtkVariant& u, 
                                             const vtkVariant& v,
                                             vtkVariantArray *propertyArr)
{
  vtkEdgeType e;
  this->AddEdgeInternal(u, v, true, propertyArr, &e);
  return e;
}

//----------------------------------------------------------------------------
void vtkMutableDirectedGraph::LazyAddVertex()
{
  this->LazyAddVertex(0);
}
//----------------------------------------------------------------------------
void vtkMutableDirectedGraph::LazyAddVertex(vtkVariantArray *propertyArr)
{
  this->AddVertexInternal(propertyArr, 0);
}

//----------------------------------------------------------------------------
void vtkMutableDirectedGraph::LazyAddVertex(const vtkVariant& pedigreeId)
{
  this->AddVertexInternal(pedigreeId, 0);
}

//----------------------------------------------------------------------------
void vtkMutableDirectedGraph::LazyAddEdge(vtkIdType u, vtkIdType v,
                                          vtkVariantArray *propertyArr)
{
  this->AddEdgeInternal(u, v, true, propertyArr, 0);
}

//----------------------------------------------------------------------------
void vtkMutableDirectedGraph::LazyAddEdge(const vtkVariant& u, vtkIdType v,
                                          vtkVariantArray *propertyArr)
{
  this->AddEdgeInternal(u, v, true, propertyArr, 0);
}

//----------------------------------------------------------------------------
void vtkMutableDirectedGraph::LazyAddEdge(vtkIdType u, const vtkVariant& v,
                                          vtkVariantArray *propertyArr)
{
  this->AddEdgeInternal(u, v, true, propertyArr, 0);
}

//----------------------------------------------------------------------------
void vtkMutableDirectedGraph::LazyAddEdge(const vtkVariant& u, 
                                          const vtkVariant& v,
                                          vtkVariantArray *propertyArr)
{
  this->AddEdgeInternal(u, v, true, propertyArr, 0);
}

//----------------------------------------------------------------------------
vtkGraphEdge *vtkMutableDirectedGraph::AddGraphEdge(vtkIdType u, vtkIdType v)
{
  vtkEdgeType e = this->AddEdge(u, v);
  this->GraphEdge->SetSource(e.Source);
  this->GraphEdge->SetTarget(e.Target);
  this->GraphEdge->SetId(e.Id);
  return this->GraphEdge;
}

//----------------------------------------------------------------------------
vtkIdType vtkMutableDirectedGraph::AddChild(vtkIdType parent,
  vtkVariantArray *propertyArr/* = 0*/)
{
  vtkIdType v = this->AddVertex();
  this->AddEdge(parent, v, propertyArr);
  return v;
}

//----------------------------------------------------------------------------
void vtkMutableDirectedGraph::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
