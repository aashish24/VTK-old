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
/*----------------------------------------------------------------------------
 Copyright (c) Sandia Corporation
 See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.
----------------------------------------------------------------------------*/
#include "vtkBoostBiconnectedComponents.h"

#include <vtkCellData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkIdTypeArray.h>

#include "vtkGraph.h"
#include "vtkGraphToBoostAdapter.h"
#include <boost/graph/biconnected_components.hpp>
#include <boost/vector_property_map.hpp>
#include <boost/version.hpp>
#include <vtksys/stl/vector>
#include <vtksys/stl/utility>

using namespace boost;
using vtksys_stl::vector;
using vtksys_stl::pair;

vtkCxxRevisionMacro(vtkBoostBiconnectedComponents, "$Revision$");
vtkStandardNewMacro(vtkBoostBiconnectedComponents);

vtkBoostBiconnectedComponents::vtkBoostBiconnectedComponents()
{
  this->OutputArrayName = 0;
}

vtkBoostBiconnectedComponents::~vtkBoostBiconnectedComponents()
{
  // release mem
  this->SetOutputArrayName(0);
}

int vtkBoostBiconnectedComponents::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkGraph *input = vtkGraph::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkGraph *output = vtkGraph::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // Send the data to output.
  output->ShallowCopy(input);

  // Biconnected components only works on undirected graphs,
  // so treat the graph as undirected.
  output->SetDirected(false);

  // Run the boost algorithm
  vtkBoostUndirectedGraph g(output);
  typedef vector_property_map<vtkIdType> PMap;
  PMap p;
  vtkGraphEdgePropertyMapHelper<PMap> helper(p);
  vector<vtkIdType> artPoints;
  pair<size_t, vtksys_stl::back_insert_iterator<vector<vtkIdType> > > 
    res(0, vtksys_stl::back_inserter(artPoints));
    
  // Initialize the helper pmap to all -1
  for (vtkIdType e = 0; e < output->GetNumberOfEdges(); e++)
    {
    helper.pmap[e] = -1;
    }
    
  // Call BGL biconnected_components
  
  // FIXME
  #define BOOST_MINOR_VERSION (BOOST_VERSION / 100 % 1000)
  #if BOOST_MINOR_VERSION == 33
  res = biconnected_components(
    g, helper, vtksys_stl::back_inserter(artPoints), vtkGraphIndexMap());
  #endif
  size_t numComp = res.first;
  
  // Create the edge attribute array
  vtkIntArray* edgeComps = vtkIntArray::New();
  if (this->OutputArrayName)
    {
    edgeComps->SetName(this->OutputArrayName);
    }
  else
    {
    edgeComps->SetName("biconnected component");
    }
  edgeComps->Allocate(output->GetNumberOfEdges());
  for (vtkIdType e = 0; e < output->GetNumberOfEdges(); e++)
    {
    edgeComps->InsertNextValue(helper.pmap[e]);
    }
  //edgeComps->SetArray(&(helper.pmap[0]), output->GetNumberOfEdges(), 1);
  output->GetEdgeData()->AddArray(edgeComps);
  edgeComps->Delete();

  // Assign component values to vertices based on the first edge
  // If isolated, assign a new value
  
  // Create the vertex attribute array
  vtkIntArray* vertComps = vtkIntArray::New();
  if (this->OutputArrayName)
    {
    vertComps->SetName(this->OutputArrayName);
    }
  else
    {
    vertComps->SetName("biconnected component");
    }
  vertComps->Allocate(output->GetNumberOfVertices());
  for (vtkIdType u = 0; u < output->GetNumberOfVertices(); u++)
    {
    const vtkIdType* edges;
    vtkIdType nedges;
    output->GetIncidentEdges(u, nedges, edges);
    int comp;
    if (nedges > 0)
      {
      int edgeIndex =0;
      int value = edgeComps->GetValue(edges[edgeIndex]);
      while( (value == -1) && (edgeIndex < nedges-1))
        {
        edgeIndex++;
        value = edgeComps->GetValue(edges[edgeIndex]);
        }
      comp = value;
      }
    else
      {
      comp = numComp;
      numComp++;
      }
    vertComps->InsertNextValue(comp);
    }

  // Articulation points belong to multiple biconnected components.
  // Indicate these by assigning a component value of -1.
  // It belongs to whatever components its incident edges belong to.
  vector<vtkIdType>::size_type i;
  for (i = 0; i < artPoints.size(); i++)
    {
    vertComps->SetValue(artPoints[i], -1);
    }

  output->GetVertexData()->AddArray(vertComps);
  vertComps->Delete();

  // Set the graph back to the correct directedness
  output->SetDirected(input->GetDirected());

  return 1;
}

void vtkBoostBiconnectedComponents::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  
    
  os << indent << "OutputArrayName: " 
     << (this->OutputArrayName ? this->OutputArrayName : "(none)") << endl;
}

