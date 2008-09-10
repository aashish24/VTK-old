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

#include "vtkExtractSelectedGraph.h"

#include "vtkCellArray.h"
#include "vtkCellData.h"
#include "vtkCommand.h"
#include "vtkConvertSelection.h"
#include "vtkDataArray.h"
#include "vtkEdgeListIterator.h"
#include "vtkEventForwarderCommand.h"
#include "vtkExtractSelection.h"
#include "vtkGraph.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMutableDirectedGraph.h"
#include "vtkMutableUndirectedGraph.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkSelection.h"
#include "vtkSignedCharArray.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkTree.h"
#include "vtkVertexListIterator.h"

#include <vtksys/stl/map>
#include <vtkstd/vector>

vtkCxxRevisionMacro(vtkExtractSelectedGraph, "$Revision$");
vtkStandardNewMacro(vtkExtractSelectedGraph);
//----------------------------------------------------------------------------
vtkExtractSelectedGraph::vtkExtractSelectedGraph()
{
  this->SetNumberOfInputPorts(2);
  this->RemoveIsolatedVertices = true;
}

//----------------------------------------------------------------------------
vtkExtractSelectedGraph::~vtkExtractSelectedGraph()
{
}

//----------------------------------------------------------------------------
int vtkExtractSelectedGraph::FillInputPortInformation(int port, vtkInformation* info)
{
  if (port == 0)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkGraph");
    return 1;
    }
  else if (port == 1)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkSelection");
    return 1;
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkExtractSelectedGraph::SetSelectionConnection(vtkAlgorithmOutput* in)
{
  this->SetInputConnection(1, in);
}

//----------------------------------------------------------------------------
int vtkExtractSelectedGraph::RequestDataObject(
  vtkInformation*, 
  vtkInformationVector** inputVector , 
  vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  if (!inInfo)
    {
    return 0;
    }
  vtkGraph *input = vtkGraph::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  
  if (input)
    {
    vtkInformation* info = outputVector->GetInformationObject(0);
    vtkGraph *output = vtkGraph::SafeDownCast(
      info->Get(vtkDataObject::DATA_OBJECT()));
    
    // Output a vtkDirectedGraph if the input is a tree.
    if (!output 
        || (vtkTree::SafeDownCast(input) && !vtkDirectedGraph::SafeDownCast(output)) 
        || (!vtkTree::SafeDownCast(input) && !output->IsA(input->GetClassName())))
      {
      if (vtkTree::SafeDownCast(input))
        {
        output = vtkDirectedGraph::New();
        }
      else
        {
        output = input->NewInstance();
        }
      output->SetPipelineInformation(info);
      output->Delete();
      this->GetOutputPortInformation(0)->Set(
        vtkDataObject::DATA_EXTENT_TYPE(), output->GetExtentType());
      }
    return 1;
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkExtractSelectedGraph::RequestData(
  vtkInformation* vtkNotUsed(request), 
  vtkInformationVector** inputVector, 
  vtkInformationVector* outputVector)
{
  vtkGraph* input = vtkGraph::GetData(inputVector[0]);
  vtkSelection* selection = vtkSelection::GetData(inputVector[1]);

  // Convert the selection to an INDICES selection
  vtkSmartPointer<vtkSelection> converted;
  converted.TakeReference(vtkConvertSelection::ToIndexSelection(selection, input));
  if (!converted.GetPointer())
    {
    vtkErrorMacro("Selection conversion to INDICES failed.");
    return 0;
    }

  // Make selection into one-level tree if not already.
  if (converted->GetContentType() != vtkSelection::SELECTIONS)
    {
    vtkSmartPointer<vtkSelection> parent = vtkSmartPointer<vtkSelection>::New();
    parent->AddChild(converted);
    converted = parent;
    }

  // Collect vertex and edge selections.
  vtkSmartPointer<vtkIdTypeArray> edgeList = vtkSmartPointer<vtkIdTypeArray>::New();
  bool hasEdges = false;
  vtkSmartPointer<vtkIdTypeArray> vertexList = vtkSmartPointer<vtkIdTypeArray>::New();
  bool hasVertices = false;
  for (unsigned int i = 0; i < converted->GetNumberOfChildren(); ++i)
    {
    vtkSelection* child = converted->GetChild(i);
    vtkIdTypeArray* list = 0;
    if (child->GetFieldType() == vtkSelection::VERTEX)
      {
      list = vertexList;
      hasVertices = true;
      }
    else if (child->GetFieldType() == vtkSelection::EDGE)
      {
      list = edgeList;
      hasEdges = true;
      }

    if (list)
      {
      // Append the selection list to the selection
      vtkIdTypeArray* curList = vtkIdTypeArray::SafeDownCast(child->GetSelectionList());
      if (curList)
        {
        vtkIdType numTuples = curList->GetNumberOfTuples();
        for (vtkIdType j = 0; j < numTuples; ++j)
          {
          list->InsertNextValue(curList->GetValue(j));
          }
        } // end if (curList)
      } // end if (list)
    } // end for each child
  
  // If there is no selection list, return an empty graph
  if (vertexList->GetNumberOfTuples() == 0 && edgeList->GetNumberOfTuples() == 0)
    {
    return 1;
    }

  // Invert the selection if necessary
  int inverse = selection->GetProperties()->Get(vtkSelection::INVERSE());
  if (inverse && hasVertices)
    {
    vtkIdType numVert = input->GetNumberOfVertices();
    vtkSmartPointer<vtkIdTypeArray> inverted = vtkSmartPointer<vtkIdTypeArray>::New();
    for (vtkIdType i = 0; i < numVert; ++i)
      {
      if (vertexList->LookupValue(i) < 0)
        {
        inverted->InsertNextValue(i);
        }
      }
    vertexList = inverted;
    }
  if (inverse && hasEdges)
    {
    vtkIdType numEdges = input->GetNumberOfEdges();
    vtkSmartPointer<vtkIdTypeArray> inverted = vtkSmartPointer<vtkIdTypeArray>::New();
    for (vtkIdType i = 0; i < numEdges; ++i)
      {
      if (edgeList->LookupValue(i) < 0)
        {
        inverted->InsertNextValue(i);
        }
      }
    edgeList = inverted;
    }
  
  vtkSmartPointer<vtkMutableDirectedGraph> dirBuilder = 
    vtkSmartPointer<vtkMutableDirectedGraph>::New();
  vtkSmartPointer<vtkMutableUndirectedGraph> undirBuilder = 
    vtkSmartPointer<vtkMutableUndirectedGraph>::New();
  bool directed;
  vtkGraph* builder = 0;
  if (vtkDirectedGraph::SafeDownCast(input))
    {
    directed = true;
    builder = dirBuilder;
    }
  else
    {
    directed = false;
    builder = undirBuilder;
    }

  // There are three cases to handle:
  // 1. Selecting vertices only: Select the vertices along with any edges
  //    connecting two selected vertices.
  // 2. Selecting edges only: Select the edges along with all vertices
  //    adjacent to a selected edge.
  // 3. Selecting vertices and edges: Select the edges along with all vertices
  //    adjacent to a selected edge, plus any additional vertex specified
  //    in the vertex selection.

  vtkDataSetAttributes* vdIn = input->GetVertexData();
  vtkDataSetAttributes* edIn = input->GetEdgeData();
  vtkDataSetAttributes* vdOut = builder->GetVertexData();
  vtkDataSetAttributes* edOut = builder->GetEdgeData();
  vtkPoints* ptsIn = input->GetPoints();
  vtkPoints* ptsOut = builder->GetPoints();
  vdOut->CopyAllocate(vdIn);
  edOut->CopyAllocate(edIn);
  vtksys_stl::map<vtkIdType, vtkIdType> vertexMap;

  // Step 1: Add the vertices.
  // If the user has specified a vertex selection, add them.
  // Else if only an edge selection and RemoveIsolatedVertices is off,
  //   add all vertices to the output.
  // Otherwise, let the edge selection determine the vertices to add.
  if (hasVertices)
    {
    // Add selected vertices
    vtkIdType numSelectedVerts = vertexList->GetNumberOfTuples();
    for (vtkIdType i = 0; i < numSelectedVerts; ++i)
      {
      vtkIdType inVert = vertexList->GetValue(i);
      vtkIdType outVert = 0;
      if (directed)
        {
        outVert = dirBuilder->AddVertex();
        }
      else
        {
        outVert = undirBuilder->AddVertex();
        }
      vdOut->CopyData(vdIn, inVert, outVert);
      ptsOut->InsertNextPoint(ptsIn->GetPoint(inVert));
      vertexMap[inVert] = outVert;
      }
    }
  else if (!this->RemoveIsolatedVertices)
    {
    // In the special case where there is only an edge selection, the user may
    // specify that they want all vertices in the output.
    vtkIdType numVert = input->GetNumberOfVertices();
    for (vtkIdType inVert = 0; inVert < numVert; ++inVert)
      {
      vtkIdType outVert = 0;
      if (directed)
        {
        outVert = dirBuilder->AddVertex();
        }
      else
        {
        outVert = undirBuilder->AddVertex();
        }
      vdOut->CopyData(vdIn, inVert, outVert);
      ptsOut->InsertNextPoint(ptsIn->GetPoint(inVert));
      vertexMap[inVert] = outVert;
      }
    }

  // Step 2: Add the edges
  // If there is an edge selection, add those edges.
  // Otherwise, add all edges connecting selected vertices.
  if (hasEdges)
    {
    // Add selected edges
    vtkIdType numSelectedEdges = edgeList->GetNumberOfTuples();
    for (vtkIdType i = 0; i < numSelectedEdges; ++i)
      {
      vtkEdgeType e;
      e.Id = edgeList->GetValue(i);
      e.Source = input->GetSourceVertex(e.Id);
      e.Target = input->GetTargetVertex(e.Id);

      // Add source and target vertices if they are not yet in output
      vtkIdType addVert[2];
      int numAddVert = 0;
      if (vertexMap.find(e.Source) == vertexMap.end())
        {
        addVert[numAddVert] = e.Source;
        ++numAddVert;
        }
      if (vertexMap.find(e.Target) == vertexMap.end())
        {
        addVert[numAddVert] = e.Target;
        ++numAddVert;
        }
      for (int j = 0; j < numAddVert; ++j)
        {
        vtkIdType outVert = 0;
        if (directed)
          {
          outVert = dirBuilder->AddVertex();
          }
        else
          {
          outVert = undirBuilder->AddVertex();
          }
        vdOut->CopyData(vdIn, addVert[j], outVert);
        ptsOut->InsertNextPoint(ptsIn->GetPoint(addVert[j]));
        vertexMap[addVert[j]] = outVert;
        }

      // Add the selected edge
      vtkIdType source = vertexMap[e.Source];
      vtkIdType target = vertexMap[e.Target];
      vtkEdgeType f;
      if (directed)
        {
        f = dirBuilder->AddEdge(source, target);
        }
      else
        {
        f = undirBuilder->AddEdge(source, target);
        }
      edOut->CopyData(edIn, e.Id, f.Id);
      // Copy edge layout to the output.
      vtkIdType npts;
      double* pts;
      input->GetEdgePoints(e.Id, npts, pts);
      builder->SetEdgePoints(f.Id, npts, pts);
      }
    }
  else
    {
    // Add edges between selected vertices
    vtkSmartPointer<vtkEdgeListIterator> edges = vtkSmartPointer<vtkEdgeListIterator>::New();
    input->GetEdges(edges);
    while (edges->HasNext())
      {
      vtkEdgeType e = edges->Next();
      if (vertexMap.find(e.Source) != vertexMap.end() &&
          vertexMap.find(e.Target) != vertexMap.end())
        {
        vtkIdType source = vertexMap[e.Source];
        vtkIdType target = vertexMap[e.Target];
        vtkEdgeType f;
        if (directed)
          {
          f = dirBuilder->AddEdge(source, target);
          }
        else
          {
          f = undirBuilder->AddEdge(source, target);
          }
        edOut->CopyData(edIn, e.Id, f.Id);
        // Copy edge layout to the output.
        vtkIdType npts;
        double* pts;
        input->GetEdgePoints(e.Id, npts, pts);
        builder->SetEdgePoints(f.Id, npts, pts);
        }
      }
    }

  // Pass constructed graph to output.
  vtkGraph* output = vtkGraph::GetData(outputVector);
  if (directed)
    {
    if (!output->CheckedShallowCopy(dirBuilder))
      {
      vtkErrorMacro(<<"Invalid graph structure.");
      return 0;
      }
    }
  else
    {
    if (!output->CheckedShallowCopy(undirBuilder))
      {
      vtkErrorMacro(<<"Invalid graph structure.");
      return 0;
      }
    }
  output->GetFieldData()->PassData(input->GetFieldData());

  // Clean up
  output->Squeeze();

  return 1;
}

//----------------------------------------------------------------------------
void vtkExtractSelectedGraph::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "RemoveIsolatedVertices: " 
     << (this->RemoveIsolatedVertices ? "on" : "off") << endl;
}
