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
/*
 * Copyright (C) 2008 The Trustees of Indiana University.
 * Use, modification and distribution is subject to the Boost Software
 * License, Version 1.0. (See http://www.boost.org/LICENSE_1_0.txt)
 */
#include "vtkPBGLDistributedGraphHelper.h"

#include "assert.h"
#include "vtkGraph.h"
#include "vtkGraphInternals.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkPBGLGraphAdapter.h"
#include "vtkDataSetAttributes.h"
#include "vtkVariantArray.h"
#include "vtkVariantBoostSerialization.h"
#include "vtkDataArray.h"
#include "vtkStringArray.h"
#include <boost/graph/distributed/mpi_process_group.hpp>
#include <boost/bind.hpp>
#include <vtkstd/utility>

//----------------------------------------------------------------------------
// private class vtkPBGLDistributedGraphHelperInternals
//----------------------------------------------------------------------------
class vtkPBGLDistributedGraphHelperInternals : public vtkObject
{
public:
  static vtkPBGLDistributedGraphHelperInternals *New();
  vtkTypeRevisionMacro(vtkPBGLDistributedGraphHelperInternals, vtkObject);

  // Description:
  // Handle a FIND_VERTEX_TAG messagae.
  vtkIdType HandleFindVertex(const vtkVariant& pedigreeId);

  // Description:
  // Handle a FIND_EDGE_SOURCE_TARGET_TAG message.
  vtkstd::pair<vtkIdType, vtkIdType> HandleFindEdgeSourceTarget(vtkIdType id);

  // Description:
  // Add a vertex with the given pedigree, if a vertex with that
  // pedigree ID does not already exist. Returns the ID for that
  // vertex.
  vtkIdType HandleAddVertex(const vtkVariant& pedigreeId);

  // Description:
  // Add a vertex with properties, if a vertex with the
  // pedigree ID (assuming there is one in the properties) does
  // not already exist.  Returns the ID for that vertex.
  vtkIdType HandleAddVertexProps(vtkVariantArray *propArray);

  // Description:
  // Handle a ADD_DIRECTED_BACK_EDGE_TAG or ADD_UNDIRECTED_BACK_END_TAG
  // message.
  void HandleAddBackEdge(vtkEdgeType edge, bool directed);

  // Description:
  // Handle ADD_*DIRECTED_EDGE_*_REPLY_TAG messages.
  vtkEdgeType
  HandleAddEdge(const vtkstd::pair<vtkIdType, vtkIdType>& msg, bool directed);

  // Description:
  // Handle ADD_*DIRECTED_EDGE_NI_*_REPLY_TAG messages.
  vtkEdgeType
  HandleAddEdgeNI(const vtkstd::pair<vtkVariant, vtkIdType>& msg,
                  bool directed);

  // Description:
  // Handle ADD_*DIRECTED_EDGE_IN_*_REPLY_TAG messages
  vtkEdgeType
  HandleAddEdgeIN(const vtkstd::pair<vtkIdType, vtkVariant>& msg,
                  bool directed);

  // Description:
  // Handle ADD_*DIRECTED_EDGE_NN_*_REPLY_TAG messages
  vtkEdgeType
  HandleAddEdgeNN(const vtkstd::pair<vtkVariant, vtkVariant>& msg,
                  bool directed);

  // Description:
  // The helper class of which this structure is a part.
  vtkPBGLDistributedGraphHelper *Helper;

  // Description:
  // Process group used by this helper
  boost::graph::distributed::mpi_process_group process_group;

private:
  vtkPBGLDistributedGraphHelperInternals()
    : process_group(GetRootProcessGroup()) { }

  // Retrieve the root process group.
  static boost::graph::distributed::mpi_process_group&
  GetRootProcessGroup()
  {
    if (!root_process_group)
      {
      root_process_group = new boost::graph::distributed::mpi_process_group();
      }
    return *root_process_group;
  }

  // Description:
  // The "root" process group, to which all of the process groups in
  // VTK's distributed graphs will eventually attach.
  static boost::graph::distributed::mpi_process_group *root_process_group;
};

// Definition
boost::graph::distributed::mpi_process_group *
vtkPBGLDistributedGraphHelperInternals::root_process_group;

vtkStandardNewMacro(vtkPBGLDistributedGraphHelperInternals);
vtkCxxRevisionMacro(vtkPBGLDistributedGraphHelperInternals, "$Revision$");

//----------------------------------------------------------------------------
// class vtkPBGLDistributedGraphHelper
//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkPBGLDistributedGraphHelper, "$Revision$");
vtkStandardNewMacro(vtkPBGLDistributedGraphHelper);

//----------------------------------------------------------------------------
vtkPBGLDistributedGraphHelper::vtkPBGLDistributedGraphHelper()
{
  this->Internals = vtkPBGLDistributedGraphHelperInternals::New();
  this->Internals->Helper = this;
}

//----------------------------------------------------------------------------
vtkPBGLDistributedGraphHelper::~vtkPBGLDistributedGraphHelper()
{
  this->Internals->Delete();
}

//----------------------------------------------------------------------------
void vtkPBGLDistributedGraphHelper::Synchronize()
{
  synchronize(this->Internals->process_group);
}

//----------------------------------------------------------------------------
vtkDistributedGraphHelper *vtkPBGLDistributedGraphHelper::Clone()
{
  return vtkPBGLDistributedGraphHelper::New();
}

//----------------------------------------------------------------------------
boost::graph::distributed::mpi_process_group vtkPBGLDistributedGraphHelper::GetProcessGroup()
{
  return this->Internals->process_group.base();
}

//----------------------------------------------------------------------------
void vtkPBGLDistributedGraphHelper::AddVertexInternal(vtkVariantArray *propertyArr,
                                                      vtkIdType *vertex)
{
  vtkIdType rank
    = this->Graph->GetInformation()->Get(vtkDataObject::DATA_PIECE_NUMBER());
  vtkAbstractArray *peds = this->Graph->GetVertexData()->GetPedigreeIds();
  vtkIdType owner = -1;

  if (peds != NULL)
    {
    vtkIdType pedIdx = this->Graph->GetVertexData()->SetPedigreeIds(peds);
    vtkVariant pedigreeId = propertyArr->GetValue(pedIdx);
    owner = this->GetVertexOwnerByPedigreeId(pedigreeId);
    }

  if ((peds != NULL) && (owner == rank))
    {
    // This little dance keeps us from having to make
    // vtkPBGLDistributedGraphHelper a friend of vtkGraph. It also
    // makes sure that users don't try to be sneaky about adding
    // vertices to non-mutable vtkGraphs.
    if (vtkMutableDirectedGraph *graph
          = vtkMutableDirectedGraph::SafeDownCast(this->Graph))
      {
        if (vertex)
          {
          *vertex = graph->AddVertex(propertyArr);
          }
        else
          {
          graph->LazyAddVertex(propertyArr);
          }
      }
    else if (vtkMutableUndirectedGraph *graph
               = vtkMutableUndirectedGraph::SafeDownCast(this->Graph))
      {
        if (vertex)
          {
          *vertex = graph->AddVertex(propertyArr);
          }
        else
          {
          graph->LazyAddVertex(propertyArr);
          }
      }
    else
      {
      vtkErrorMacro("Cannot add vertices to a non-mutable, distributed graph");
      }
    return;
    }

  if (vertex)
    {
    // Request immediate addition of the vertex, with a reply.
    send_oob_with_reply(this->Internals->process_group, owner,
                        ADD_VERTEX_PROPS_WITH_REPLY_TAG, propertyArr, *vertex);
    }
  else
    {
    // Request addition of the vertex, eventually.
    send(this->Internals->process_group, owner, ADD_VERTEX_PROPS_NO_REPLY_TAG, propertyArr);
    }
}

//----------------------------------------------------------------------------
void vtkPBGLDistributedGraphHelper::AddVertexInternal(const vtkVariant& pedigreeId,
                                                      vtkIdType *vertex)
{
  vtkIdType rank
    = this->Graph->GetInformation()->Get(vtkDataObject::DATA_PIECE_NUMBER());

  vtkIdType owner = this->GetVertexOwnerByPedigreeId(pedigreeId);

  if (owner == rank)
    {
    // This little dance keeps us from having to make
    // vtkPBGLDistributedGraphHelper a friend of vtkGraph. It also
    // makes sure that users don't try to be sneaky about adding
    // vertices to non-mutable vtkGraphs.
    if (vtkMutableDirectedGraph *graph
          = vtkMutableDirectedGraph::SafeDownCast(this->Graph))
      {
        if (vertex)
          {
          *vertex = graph->AddVertex(pedigreeId);
          }
        else
          {
          graph->LazyAddVertex(pedigreeId);
          }
      }
    else if (vtkMutableUndirectedGraph *graph
               = vtkMutableUndirectedGraph::SafeDownCast(this->Graph))
      {
        if (vertex)
          {
          *vertex = graph->AddVertex(pedigreeId);
          }
        else
          {
          graph->LazyAddVertex(pedigreeId);
          }
      }
    else
      {
      vtkErrorMacro("Cannot add vertices to a non-mutable, distributed graph");
      }
    return;
    }

  if (vertex)
    {
    // Request immediate addition of the vertex, with a reply.
    send_oob_with_reply(this->Internals->process_group, owner,
                        ADD_VERTEX_WITH_REPLY_TAG, pedigreeId, *vertex);
    }
  else
    {
    // Request addition of the vertex, eventually.
    send(this->Internals->process_group, owner, ADD_VERTEX_NO_REPLY_TAG, pedigreeId);
    }
}

//----------------------------------------------------------------------------
void
vtkPBGLDistributedGraphHelper::AddEdgeInternal(vtkIdType u,
                                               vtkIdType v,
                                               bool directed,
                                               vtkVariantArray *propertyArr,
                                               vtkEdgeType *edge)
{
  int rank = this->Graph->GetInformation()->Get(vtkDataObject::DATA_PIECE_NUMBER());
  int numProcs = this->Graph->GetInformation()->Get(vtkDataObject::DATA_NUMBER_OF_PIECES());
  int uOwner = this->GetVertexOwner (u);

  if (uOwner == rank)
    {
    // The source of the edge is local.
    vtkGraphInternals* GraphInternals = this->Graph->GetGraphInternals(true);

    // The edge ID involves our rank and the local number of edges.
    vtkIdType edgeId
      = this->MakeDistributedId(rank, GraphInternals->NumberOfEdges);

    if (propertyArr)      // Add edge properties
      {
      vtkDataSetAttributes *edgeData = this->Graph->GetEdgeData();
      int numProps = propertyArr->GetNumberOfValues();   // # of properties = # of arrays
      assert(numProps == edgeData->GetNumberOfArrays());
      for (int iprop=0; iprop<numProps; iprop++)
        {
        vtkAbstractArray* arr = edgeData->GetAbstractArray(iprop);
        if (vtkDataArray::SafeDownCast(arr))
          {
          vtkDataArray::SafeDownCast(arr)->InsertNextTuple1(propertyArr->GetPointer(iprop)->ToDouble());
          }
        else if (vtkStringArray::SafeDownCast(arr))
          {
          vtkStringArray::SafeDownCast(arr)->InsertNextValue(propertyArr->GetPointer(iprop)->ToString());
          }
        else
          {
          vtkErrorMacro("Unsupported array type");
          }
        }
      }

    // Add the forward edge.
    GraphInternals->Adjacency[this->GetVertexIndex(u)]
      .OutEdges.push_back(vtkOutEdgeType(v, edgeId));

    // We've added an edge.
    GraphInternals->NumberOfEdges++;

    int vOwner = this->GetVertexOwner(v);
    if (vOwner == rank)
      {
        // The target vertex is local. Add the appropriate back edge.
        if (directed)
          {
          GraphInternals->Adjacency[this->GetVertexIndex(v)]
            .InEdges.push_back(vtkInEdgeType(u, edgeId));
          }
        else if (u != v)
          {
          // Avoid storing self-loops twice in undirected graphs
          GraphInternals->Adjacency[this->GetVertexIndex(v)]
            .OutEdges.push_back(vtkOutEdgeType(u, edgeId));
          }
      }
    else
      {
      // The target vertex is remote: send a message asking its
      // owner to add the back edge.
      send(this->Internals->process_group, vOwner,
           directed? ADD_DIRECTED_BACK_EDGE_TAG : ADD_UNDIRECTED_BACK_EDGE_TAG,
           vtkEdgeType(u, v, edgeId));
      }

    if (edge)
      {
      *edge = vtkEdgeType(u, v, edgeId);
      }
    }
  else
    {
    // The source of the edge is non-local.
      if (edge)
        {
        // Send an AddEdge request to the owner of "u", and wait
        // patiently for the reply.
        send_oob_with_reply(this->Internals->process_group, uOwner,
                            directed? ADD_DIRECTED_EDGE_WITH_REPLY_TAG
                                    : ADD_UNDIRECTED_EDGE_WITH_REPLY_TAG,
                            vtkstd::pair<vtkIdType, vtkIdType>(u, v),
                            *edge);
        }
      else
        {
        // We're adding a remote edge, but we don't need to wait
        // until the edge has been added. Just send a message to the
        // owner of the source; we don't need (or want) a reply.
        send(this->Internals->process_group, uOwner,
             directed? ADD_DIRECTED_EDGE_NO_REPLY_TAG
                     : ADD_UNDIRECTED_EDGE_NO_REPLY_TAG,
             vtkstd::pair<vtkIdType, vtkIdType>(u, v));
        }
    }
}

//----------------------------------------------------------------------------
void
vtkPBGLDistributedGraphHelper::AddEdgeInternal(const vtkVariant& uPedigreeId,
                                               vtkIdType v,
                                               bool directed,
                                               vtkVariantArray *propertyArr,
                                               vtkEdgeType *edge)
{
  vtkIdType rank
    = this->Graph->GetInformation()->Get(vtkDataObject::DATA_PIECE_NUMBER());
  vtkIdType uOwner = this->GetVertexOwnerByPedigreeId(uPedigreeId);

  if (uOwner == rank)
    {
    // Resolve the pedigreeId for u immediately and add the edge locally.
    vtkIdType u;
    this->AddVertexInternal(uPedigreeId, &u);
    this->AddEdgeInternal(u, v, directed, propertyArr, edge);
    return;
    }

  // Edge is remote: request its addition.
  if (edge)
    {
    send_oob_with_reply(this->Internals->process_group, uOwner,
                        directed? ADD_DIRECTED_EDGE_NI_WITH_REPLY_TAG
                                : ADD_UNDIRECTED_EDGE_NI_WITH_REPLY_TAG,
                        vtkstd::pair<vtkVariant, vtkIdType>(uPedigreeId, v),
                        *edge);
    }
  else
    {
    send(this->Internals->process_group, uOwner,
         directed? ADD_DIRECTED_EDGE_NI_NO_REPLY_TAG
                 : ADD_UNDIRECTED_EDGE_NI_NO_REPLY_TAG,
         vtkstd::pair<vtkVariant, vtkIdType>(uPedigreeId, v));
    }
}

//----------------------------------------------------------------------------
void
vtkPBGLDistributedGraphHelper::AddEdgeInternal(vtkIdType u,
                                               const vtkVariant& vPedigreeId,
                                               bool directed,
                                               vtkVariantArray *propertyArr,
                                               vtkEdgeType *edge)
{
  vtkIdType rank
    = this->Graph->GetInformation()->Get(vtkDataObject::DATA_PIECE_NUMBER());
  vtkIdType vOwner = this->GetVertexOwnerByPedigreeId(vPedigreeId);

  if (vOwner == rank || edge)
    {
    // Resolve the pedigree ID for v immediately and add the edge.
    vtkIdType v;
    this->AddVertexInternal(vPedigreeId, &v);
    this->AddEdgeInternal(u, v, directed, propertyArr, edge);
    return;
    }

  // v is remote and we don't care when the edge is added. Ask the
  // owner of v to resolve the pedigree ID of v and add the edge.
  send(this->Internals->process_group, vOwner,
       directed? ADD_DIRECTED_EDGE_IN_NO_REPLY_TAG
               : ADD_UNDIRECTED_EDGE_IN_NO_REPLY_TAG,
       vtkstd::pair<vtkIdType, vtkVariant>(u, vPedigreeId));
}

//----------------------------------------------------------------------------
void
vtkPBGLDistributedGraphHelper::AddEdgeInternal(const vtkVariant& uPedigreeId,
                                               const vtkVariant& vPedigreeId,
                                               bool directed,
                                               vtkVariantArray *propertyArr,
                                               vtkEdgeType *edge)
{
  vtkIdType rank
    = this->Graph->GetInformation()->Get(vtkDataObject::DATA_PIECE_NUMBER());
  vtkIdType uOwner = this->GetVertexOwnerByPedigreeId(vPedigreeId);

  if (uOwner == rank)
    {
    // Resolve the pedigree ID for u immediately and add the edge.
    vtkIdType u;
    this->AddVertexInternal(uPedigreeId, &u);
    this->AddEdgeInternal(u, vPedigreeId, directed, propertyArr, edge);
    return;
    }

  vtkIdType vOwner = this->GetVertexOwnerByPedigreeId(vPedigreeId);
  if (vOwner == rank || edge)
    {
    // Resolve the pedigree ID for v immediately and add the edge.
    vtkIdType v;
    this->AddVertexInternal(vPedigreeId, &v);
    this->AddEdgeInternal(uPedigreeId, v, directed, propertyArr, edge);
    return;
    }

  // Neither u nor v is local, and we don't care when the edge is
  // added, so ask the owner of v to resolve the pedigree ID of v and
  // add the edge.
  send(this->Internals->process_group, vOwner,
       directed? ADD_DIRECTED_EDGE_NN_NO_REPLY_TAG
               : ADD_UNDIRECTED_EDGE_NN_NO_REPLY_TAG,
       vtkstd::pair<vtkVariant, vtkVariant>(uPedigreeId, vPedigreeId));
}

//----------------------------------------------------------------------------
vtkIdType
vtkPBGLDistributedGraphHelper::FindVertex(const vtkVariant& pedigreeId)
{
  vtkIdType rank
    = this->Graph->GetInformation()->Get(vtkDataObject::DATA_PIECE_NUMBER());
  vtkIdType owner = this->GetVertexOwnerByPedigreeId(pedigreeId);
  if (owner == rank)
    {
    // The vertex is local; just ask the local part of the graph.
    return this->Graph->FindVertex(pedigreeId);
    }

  // The vertex is remote; Send a message looking for this
  // vertex.
  vtkIdType result;
  send_oob_with_reply(this->Internals->process_group, owner,
                      FIND_VERTEX_TAG, pedigreeId, result);
  return result;
}

//----------------------------------------------------------------------------
void
vtkPBGLDistributedGraphHelper::FindEdgeSourceAndTarget(vtkIdType id,
                                                       vtkIdType *source,
                                                       vtkIdType *target)
{
  vtkIdType rank
    = this->Graph->GetInformation()->Get(vtkDataObject::DATA_PIECE_NUMBER());
  vtkIdType owner = this->GetEdgeOwner(id);

  if (owner == rank)
    {
    if (source)
      {
      *source = this->Graph->GetSourceVertex(id);
      }
    if (target)
      {
      *target = this->Graph->GetTargetVertex(id);
      }
    return;
    }

  vtkstd::pair<vtkIdType, vtkIdType> result;
  send_oob_with_reply(this->Internals->process_group, owner,
                      FIND_EDGE_SOURCE_TARGET_TAG, id, result);

  if (source)
    {
    *source = result.first;
    }
  if (target)
    {
    *target = result.second;
    }
}

//----------------------------------------------------------------------------
void vtkPBGLDistributedGraphHelper::AttachToGraph(vtkGraph *graph)
{
  this->Graph = graph;

  if (graph &&
      (graph->GetNumberOfVertices() != 0 || graph->GetNumberOfEdges() != 0))
    {
    vtkErrorMacro("Cannot attach a distributed graph helper to a non-empty vtkGraph");
    }

  if (this->Graph)
    {
    // Set the piece number and number of pieces so that the
    // vtkGraph knows the layout of the graph.
    this->Graph->GetInformation()->Set(
      vtkDataObject::DATA_PIECE_NUMBER(),
      process_id(this->Internals->process_group));
    this->Graph->GetInformation()->Set(
      vtkDataObject::DATA_NUMBER_OF_PIECES(),
      num_processes(this->Internals->process_group));

    // Add our triggers to the process group
    typedef vtkstd::pair<vtkIdType, vtkIdType> IdPair;
    this->Internals->process_group.make_distributed_object();
    this->Internals->process_group.trigger_with_reply<vtkVariant>
      (FIND_VERTEX_TAG,
       boost::bind(&vtkPBGLDistributedGraphHelperInternals::HandleFindVertex,
                   this->Internals, _3));
    this->Internals->process_group.trigger_with_reply<vtkIdType>
      (FIND_EDGE_SOURCE_TARGET_TAG,
       boost::bind(&vtkPBGLDistributedGraphHelperInternals
                     ::HandleFindEdgeSourceTarget,
                   this->Internals, _3));
    this->Internals->process_group.trigger<vtkVariant>
      (ADD_VERTEX_NO_REPLY_TAG,
       boost::bind(&vtkPBGLDistributedGraphHelperInternals::HandleAddVertex,
                   this->Internals, _3));
    this->Internals->process_group.trigger_with_reply<vtkVariant>
      (ADD_VERTEX_WITH_REPLY_TAG,
       boost::bind(&vtkPBGLDistributedGraphHelperInternals::HandleAddVertex,
                   this->Internals, _3));
    this->Internals->process_group.trigger<vtkVariantArray *>
      (ADD_VERTEX_PROPS_NO_REPLY_TAG,
       boost::bind(&vtkPBGLDistributedGraphHelperInternals::HandleAddVertexProps,
                   this->Internals, _3));
    this->Internals->process_group.trigger_with_reply<vtkVariantArray *>
      (ADD_VERTEX_PROPS_WITH_REPLY_TAG,
       boost::bind(&vtkPBGLDistributedGraphHelperInternals::HandleAddVertexProps,
                   this->Internals, _3));
    this->Internals->process_group.trigger<vtkEdgeType>
      (ADD_DIRECTED_BACK_EDGE_TAG,
       boost::bind(&vtkPBGLDistributedGraphHelperInternals::HandleAddBackEdge,
                   this->Internals, _3, true));
    this->Internals->process_group.trigger<vtkEdgeType>
      (ADD_UNDIRECTED_BACK_EDGE_TAG,
       boost::bind(&vtkPBGLDistributedGraphHelperInternals::HandleAddBackEdge,
                   this->Internals, _3, false));

    // Add edge for (id, id) pairs
    this->Internals->process_group.trigger<IdPair>
      (ADD_DIRECTED_EDGE_NO_REPLY_TAG,
       boost::bind(&vtkPBGLDistributedGraphHelperInternals::HandleAddEdge,
                   this->Internals, _3, true));
    this->Internals->process_group.trigger<IdPair>
      (ADD_UNDIRECTED_EDGE_NO_REPLY_TAG,
       boost::bind(&vtkPBGLDistributedGraphHelperInternals::HandleAddEdge,
                   this->Internals, _3, false));
    this->Internals->process_group.trigger_with_reply<IdPair>
      (ADD_DIRECTED_EDGE_WITH_REPLY_TAG,
       boost::bind(&vtkPBGLDistributedGraphHelperInternals::HandleAddEdge,
                   this->Internals, _3, true));
    this->Internals->process_group.trigger_with_reply<IdPair>
      (ADD_UNDIRECTED_EDGE_WITH_REPLY_TAG,
       boost::bind(&vtkPBGLDistributedGraphHelperInternals::HandleAddEdge,
                   this->Internals, _3, false));

    // Add edge for (pedigree, id) pairs
    typedef vtkstd::pair<vtkVariant, vtkIdType> PedigreeIdPair;
    this->Internals->process_group.trigger<PedigreeIdPair>
      (ADD_DIRECTED_EDGE_NI_NO_REPLY_TAG,
       boost::bind(&vtkPBGLDistributedGraphHelperInternals::HandleAddEdgeNI,
                   this->Internals, _3, true));
    this->Internals->process_group.trigger<PedigreeIdPair>
      (ADD_UNDIRECTED_EDGE_NI_NO_REPLY_TAG,
       boost::bind(&vtkPBGLDistributedGraphHelperInternals::HandleAddEdgeNI,
                   this->Internals, _3, false));
    this->Internals->process_group.trigger_with_reply<PedigreeIdPair>
      (ADD_DIRECTED_EDGE_NI_WITH_REPLY_TAG,
       boost::bind(&vtkPBGLDistributedGraphHelperInternals::HandleAddEdgeNI,
                   this->Internals, _3, true));
    this->Internals->process_group.trigger_with_reply<PedigreeIdPair>
      (ADD_UNDIRECTED_EDGE_NI_WITH_REPLY_TAG,
       boost::bind(&vtkPBGLDistributedGraphHelperInternals::HandleAddEdgeNI,
                   this->Internals, _3, false));

    // Add edge for (id, pedigree) pairs
    typedef vtkstd::pair<vtkIdType, vtkVariant> IdPedigreePair;
    this->Internals->process_group.trigger<IdPedigreePair>
      (ADD_DIRECTED_EDGE_IN_NO_REPLY_TAG,
       boost::bind(&vtkPBGLDistributedGraphHelperInternals::HandleAddEdgeIN,
                   this->Internals, _3, true));
    this->Internals->process_group.trigger<IdPedigreePair>
      (ADD_UNDIRECTED_EDGE_IN_NO_REPLY_TAG,
       boost::bind(&vtkPBGLDistributedGraphHelperInternals::HandleAddEdgeIN,
                   this->Internals, _3, false));

    // Add edge for (pedigree, pedigree) pairs
    typedef vtkstd::pair<vtkVariant, vtkVariant> PedigreePedigreePair;
    this->Internals->process_group.trigger<PedigreePedigreePair>
      (ADD_DIRECTED_EDGE_NN_NO_REPLY_TAG,
       boost::bind(&vtkPBGLDistributedGraphHelperInternals::HandleAddEdgeNN,
                   this->Internals, _3, true));
    this->Internals->process_group.trigger<PedigreePedigreePair>
      (ADD_UNDIRECTED_EDGE_NN_NO_REPLY_TAG,
       boost::bind(&vtkPBGLDistributedGraphHelperInternals::HandleAddEdgeNN,
                   this->Internals, _3, false));
    }

  // vtkDistributedGraphHelper will set up the appropriate masks.
  this->Superclass::AttachToGraph(graph);
}

//----------------------------------------------------------------------------
vtkIdType
vtkPBGLDistributedGraphHelperInternals::
HandleFindVertex(const vtkVariant& pedigreeId)
{
  return this->Helper->FindVertex(pedigreeId);
}

//----------------------------------------------------------------------------
vtkstd::pair<vtkIdType, vtkIdType>
vtkPBGLDistributedGraphHelperInternals::
HandleFindEdgeSourceTarget(vtkIdType id)
{
  vtkstd::pair<vtkIdType, vtkIdType> result;
  this->Helper->FindEdgeSourceAndTarget(id, &result.first, &result.second);
  return result;
}

//----------------------------------------------------------------------------
vtkIdType
vtkPBGLDistributedGraphHelperInternals::
HandleAddVertexProps(vtkVariantArray *propArr)
{
  vtkIdType result;
  this->Helper->AddVertexInternal(propArr, &result);
  return result;
}

//----------------------------------------------------------------------------
vtkIdType
vtkPBGLDistributedGraphHelperInternals::
HandleAddVertex(const vtkVariant& pedigreeId)
{
  vtkIdType result;
  this->Helper->AddVertexInternal(pedigreeId, &result);
  return result;
}

//----------------------------------------------------------------------------
void
vtkPBGLDistributedGraphHelperInternals::HandleAddBackEdge
  (vtkEdgeType edge, bool directed)
{
  assert(edge.Source != edge.Target);
  assert(this->Helper->GetVertexOwner(edge.Target)
         == this->Helper->Graph->GetInformation()->Get(vtkDataObject::DATA_PIECE_NUMBER()));
  vtkGraphInternals *GraphInternals = this->Helper->Graph->GetGraphInternals(true);
  if (directed)
    {
    GraphInternals->Adjacency[this->Helper->GetVertexIndex(edge.Target)]
      .InEdges.push_back(vtkInEdgeType(edge.Source, edge.Id));
    }
  else
    {
    GraphInternals->Adjacency[this->Helper->GetVertexIndex(edge.Target)]
      .OutEdges.push_back(vtkOutEdgeType(edge.Source, edge.Id));
    }
}

//----------------------------------------------------------------------------
vtkEdgeType
vtkPBGLDistributedGraphHelperInternals::HandleAddEdge
  (const vtkstd::pair<vtkIdType, vtkIdType>& msg, bool directed)
{
  vtkEdgeType result;
  this->Helper->AddEdgeInternal(msg.first, msg.second, directed, 0, &result);
  return result;
}

//----------------------------------------------------------------------------
vtkEdgeType
vtkPBGLDistributedGraphHelperInternals::HandleAddEdgeNI
  (const vtkstd::pair<vtkVariant, vtkIdType>& msg, bool directed)
{
  vtkEdgeType result;
  this->Helper->AddEdgeInternal(msg.first, msg.second, directed, 0, &result);
  return result;
}

//----------------------------------------------------------------------------
vtkEdgeType
vtkPBGLDistributedGraphHelperInternals::HandleAddEdgeIN
  (const vtkstd::pair<vtkIdType, vtkVariant>& msg, bool directed)
{
  vtkEdgeType result;
  this->Helper->AddEdgeInternal(msg.first, msg.second, directed, 0, &result);
  return result;
}

//----------------------------------------------------------------------------
vtkEdgeType
vtkPBGLDistributedGraphHelperInternals::HandleAddEdgeNN
  (const vtkstd::pair<vtkVariant, vtkVariant>& msg, bool directed)
{
  vtkEdgeType result;
  this->Helper->AddEdgeInternal(msg.first, msg.second, directed, 0, &result);
  return result;
}

//----------------------------------------------------------------------------
// Parallel BGL interface functions
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
boost::graph::distributed::mpi_process_group
process_group(vtkGraph *graph)
{
  vtkDistributedGraphHelper *helper = graph->GetDistributedGraphHelper();
  if (!helper)
    {
    vtkErrorWithObjectMacro(graph, "A vtkGraph without a distributed graph helper is not a distributed graph");
    return boost::graph::distributed::mpi_process_group();
    }

  vtkPBGLDistributedGraphHelper *pbglHelper
    = vtkPBGLDistributedGraphHelper::SafeDownCast(helper);
  if (!pbglHelper)
    {
    vtkErrorWithObjectMacro(graph, "A vtkGraph with a non-Parallel BGL distributed graph helper cannot be used with the Parallel BGL");
    return boost::graph::distributed::mpi_process_group();
    }

  return pbglHelper->Internals->process_group.base();
}
