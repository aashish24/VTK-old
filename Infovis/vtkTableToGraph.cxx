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

#include "vtkTableToGraph.h"

#include "vtkBitArray.h"
#include "vtkCellData.h"
#include "vtkCommand.h"
#include "vtkDataArray.h"
#include "vtkEdgeListIterator.h"
#include "vtkExecutive.h"
#include "vtkExtractSelectedGraph.h"
#include "vtkGraph.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkMutableDirectedGraph.h"
#include "vtkMutableUndirectedGraph.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkSelection.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkTable.h"
#include "vtkVariant.h"
#include "vtkVariantArray.h"

#include <vtksys/stl/algorithm>
#include <vtksys/stl/map>
#include <vtksys/stl/set>
#include <vtksys/stl/vector>

#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

vtkCxxRevisionMacro(vtkTableToGraph, "$Revision$");
vtkStandardNewMacro(vtkTableToGraph);
vtkCxxSetObjectMacro(vtkTableToGraph, LinkGraph, vtkMutableDirectedGraph);
//---------------------------------------------------------------------------
vtkTableToGraph::vtkTableToGraph()
{
  this->Directed = 0;
  this->LinkGraph = vtkMutableDirectedGraph::New();
  this->SetNumberOfInputPorts(2);
  this->SetNumberOfOutputPorts(1);
}

//---------------------------------------------------------------------------
vtkTableToGraph::~vtkTableToGraph()
{
  this->SetLinkGraph(0);
}

//---------------------------------------------------------------------------
int vtkTableToGraph::ValidateLinkGraph()
{
  if (!this->LinkGraph)
    {
    this->LinkGraph = vtkMutableDirectedGraph::New();
    }
  if (!vtkStringArray::SafeDownCast(
      this->LinkGraph->GetVertexData()->GetAbstractArray("column")))
    {
    if (this->LinkGraph->GetNumberOfVertices() == 0)
      {
      vtkStringArray* column = vtkStringArray::New();
      column->SetName("column");
      this->LinkGraph->GetVertexData()->AddArray(column);
      column->Delete();
      this->Modified();
      }
    else
      {
      vtkErrorMacro("The link graph must contain a string array named \"column\".");
      return 0;
      }
    }
  if (!vtkStringArray::SafeDownCast(
      this->LinkGraph->GetVertexData()->GetAbstractArray("domain")))
    {
    vtkStringArray* domain = vtkStringArray::New();
    domain->SetName("domain");
    domain->SetNumberOfTuples(this->LinkGraph->GetNumberOfVertices());
    for (vtkIdType i = 0; i < this->LinkGraph->GetNumberOfVertices(); i++)
      {
      domain->SetValue(i, "");
      }
    this->LinkGraph->GetVertexData()->AddArray(domain);
    domain->Delete();
    this->Modified();
    }
  if (!vtkBitArray::SafeDownCast(
      this->LinkGraph->GetVertexData()->GetAbstractArray("hidden")))
    {
    vtkBitArray* hidden = vtkBitArray::New();
    hidden->SetName("hidden");
    hidden->SetNumberOfTuples(this->LinkGraph->GetNumberOfVertices());
    this->LinkGraph->GetVertexData()->AddArray(hidden);
    hidden->Delete();
    this->Modified();
    }
  if (!vtkIntArray::SafeDownCast(
      this->LinkGraph->GetVertexData()->GetAbstractArray("active")))
    {
    vtkIntArray* active = vtkIntArray::New();
    active->SetName("active");
    active->SetNumberOfTuples(this->LinkGraph->GetNumberOfVertices());
    for (vtkIdType i = 0; i < this->LinkGraph->GetNumberOfVertices(); ++i)
      {
      active->SetValue(i, 1);
      }
    this->LinkGraph->GetVertexData()->AddArray(active);
    active->Delete();
    this->Modified();
    }
  return 1;
}

//---------------------------------------------------------------------------
void vtkTableToGraph::AddLinkVertex(const char* column, const char* domain, int hidden)
{
  if (!column)
    {
    vtkErrorMacro("Column name cannot be null");
    }
  
  vtkStdString domainStr = "";
  if (domain)
    {
    domainStr = domain;
    }
  
  if (!this->ValidateLinkGraph())
    {
    return;
    }
  
  vtkStringArray* columnArr = vtkStringArray::SafeDownCast(
      this->LinkGraph->GetVertexData()->GetAbstractArray("column"));
  vtkStringArray* domainArr = vtkStringArray::SafeDownCast(
      this->LinkGraph->GetVertexData()->GetAbstractArray("domain"));
  vtkBitArray* hiddenArr = vtkBitArray::SafeDownCast(
      this->LinkGraph->GetVertexData()->GetAbstractArray("hidden"));
  vtkIntArray* activeArr = vtkIntArray::SafeDownCast(
      this->LinkGraph->GetVertexData()->GetAbstractArray("active"));

  vtkIdType index = -1;
  for (vtkIdType i = 0; i < this->LinkGraph->GetNumberOfVertices(); i++)
    {
    if (!strcmp(column, columnArr->GetValue(i)))
      {
      index = i;
      break;
      }
    }
  if (index >= 0)
    {
    domainArr->SetValue(index, domainStr);
    hiddenArr->SetValue(index, hidden);
    activeArr->SetValue(index, 1);
    }
  else
    {
    this->LinkGraph->AddVertex();
    columnArr->InsertNextValue(column);
    domainArr->InsertNextValue(domainStr);
    hiddenArr->InsertNextValue(hidden);
    activeArr->InsertNextValue(1);
    }
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkTableToGraph::ClearLinkVertices()
{
  this->ValidateLinkGraph();
  vtkIntArray* activeArr = vtkIntArray::SafeDownCast(
    this->LinkGraph->GetVertexData()->GetAbstractArray("active"));
  for (vtkIdType i = 0; i < this->LinkGraph->GetNumberOfVertices(); ++i)
    {
    activeArr->SetValue(i, 0);
    }
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkTableToGraph::AddLinkEdge(const char* column1, const char* column2)
{
  if (!column1 || !column2)
    {
    vtkErrorMacro("Column names may not be null.");
    }
  this->ValidateLinkGraph();
  vtkStringArray* columnArr = vtkStringArray::SafeDownCast(
      this->LinkGraph->GetVertexData()->GetAbstractArray("column"));
  vtkIdType source = -1;
  vtkIdType target = -1;
  for (vtkIdType i = 0; i < this->LinkGraph->GetNumberOfVertices(); i++)
    {
    if (!strcmp(column1, columnArr->GetValue(i)))
      {
      source = i;
      }
    if (!strcmp(column2, columnArr->GetValue(i)))
      {
      target = i;
      }
    }
  if (source < 0)
    {
    this->AddLinkVertex(column1);
    source = this->LinkGraph->GetNumberOfVertices() - 1;
    }
  if (target < 0)
    {
    this->AddLinkVertex(column2);
    target = this->LinkGraph->GetNumberOfVertices() - 1;
    }
  this->LinkGraph->AddEdge(source, target);
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkTableToGraph::ClearLinkEdges()
{
  VTK_CREATE(vtkMutableDirectedGraph, newLinkGraph);
  for (vtkIdType i = 0; i < this->LinkGraph->GetNumberOfVertices(); ++i)
    {
    newLinkGraph->AddVertex();
    }
  newLinkGraph->GetVertexData()->ShallowCopy(this->LinkGraph->GetVertexData());
  this->SetLinkGraph(newLinkGraph);
}

//---------------------------------------------------------------------------
void vtkTableToGraph::LinkColumnPath(
  vtkStringArray* column, 
  vtkStringArray* domain, 
  vtkBitArray* hidden)
{
  vtkMutableDirectedGraph *g = vtkMutableDirectedGraph::New();
  for (vtkIdType i = 0; i < column->GetNumberOfTuples(); i++)
    {
    g->AddVertex();
    }
  for (vtkIdType i = 1; i < column->GetNumberOfTuples(); i++)
    {
    g->AddEdge(i-1, i);
    }
  column->SetName("column");
  g->GetVertexData()->AddArray(column);
  if (domain)
    {
    domain->SetName("domain");
    g->GetVertexData()->AddArray(domain);
    }
  if (hidden)
    {
    hidden->SetName("hidden");
    g->GetVertexData()->AddArray(hidden);
    }
  this->SetLinkGraph(g);
  g->Delete();
}

//---------------------------------------------------------------------------
int vtkTableToGraph::FillInputPortInformation(int port, vtkInformation* info)
{
  if (port == 0)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkTable");
    return 1;
    }
  else if (port == 1)
    {
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkTable");
    return 1;
    }
  return 0;
}

//---------------------------------------------------------------------------
class vtkVariantCompare
{
public:
  bool operator()(
    const vtkVariant& a, 
    const vtkVariant& b) const
  {
    if (a.GetType() == VTK_STRING)
      {
      return a.ToString() < b.ToString();
      }
    else if (a.IsNumeric())
      {
      return a.ToDouble() < b.ToDouble();
      }
    else
      {
      // Punt, just do pointer difference.
      return &a < &b;
      }
  }
};

//---------------------------------------------------------------------------
class vtkTableToGraphCompare
{
public:
  bool operator()(
    const vtksys_stl::pair<vtkStdString, vtkVariant>& a, 
    const vtksys_stl::pair<vtkStdString, vtkVariant>& b) const
  {
    if (a.first != b.first)
      {
      return a.first < b.first;
      }
    return vtkVariantCompare()(a.second, b.second);
  }
};

//---------------------------------------------------------------------------
template <typename T>
vtkVariant vtkTableToGraphGetValue(T* arr, vtkIdType index)
{
  return vtkVariant(arr[index]);
}

//---------------------------------------------------------------------------
template <typename T>
void vtkTableToGraphFindVertices(
  T* arr,                            // The raw edge table column
  vtkIdType size,                    // The size of the edge table column
  vtksys_stl::map<vtksys_stl::pair<vtkStdString, vtkVariant>, vtkIdType, vtkTableToGraphCompare>& vertexMap,
                                     // A map of domain-value pairs to graph id
  vtkStringArray *domainArr,         // The domain of each vertex
  vtkStringArray *stringValueArr,    // The string value of each vertex
  vtkVariantArray *variantValueArr,  // The variant value of each vertex
  vtkIdType & curVertex,             // The current vertex id
  vtkTable *vertexTable,  // An array that holds the actual value of each vertex
  vtkStdString domain)  // The domain of the array
{
  for (vtkIdType i = 0; i < size; i++)
    {
    T v = arr[i];
    vtkVariant val(v);
    vtksys_stl::pair<vtkStdString, vtkVariant> value(domain, val);
    if (vertexMap.count(value) == 0)
      {
      vtkIdType row = vertexTable->InsertNextBlankRow();
      vertexTable->SetValueByName(row, domain, val);
      vertexMap[value] = row;
      domainArr->InsertNextValue(domain);
      stringValueArr->InsertNextValue(val.ToString());
      variantValueArr->InsertNextValue(val);
      curVertex = row;
      }
    }
}

//---------------------------------------------------------------------------
template <typename T>
void vtkTableToGraphFindHiddenVertices(
  T* arr,                            // The raw edge table column
  vtkIdType size,                    // The size of the edge table column
  vtksys_stl::map<vtksys_stl::pair<vtkStdString, vtkVariant>, vtkIdType, vtkTableToGraphCompare>& hiddenMap,
                                     // A map of domain-value pairs to hidden vertex id
  vtkIdType & curHiddenVertex,       // The current hidden vertex id
  vtkStdString domain)  // The domain of the array
{
  for (vtkIdType i = 0; i < size; i++)
    {
    T v = arr[i];
    vtkVariant val(v);
    vtksys_stl::pair<vtkStdString, vtkVariant> value(domain, val);
    if (hiddenMap.count(value) == 0)
      {
      hiddenMap[value] = curHiddenVertex;
      ++curHiddenVertex;
      }
    }
}

//---------------------------------------------------------------------------
int vtkTableToGraph::RequestData(
  vtkInformation*, 
  vtkInformationVector** inputVector, 
  vtkInformationVector* outputVector)
{
  // Check that the link graph is valid
  if (!this->ValidateLinkGraph())
    {
    return 0;
    }

  // Extract edge table
  vtkInformation* edgeTableInfo = inputVector[0]->GetInformationObject(0);
  vtkTable* edgeTable = vtkTable::SafeDownCast(
    edgeTableInfo->Get(vtkDataObject::DATA_OBJECT()));
  
  // Extract vertex table
  vtkInformation* vertexTableInfo = inputVector[1]->GetInformationObject(0);
  vtkTable* vertexTable = 0;
  if (vertexTableInfo)
    {
    vertexTable = vtkTable::SafeDownCast(
      vertexTableInfo->Get(vtkDataObject::DATA_OBJECT()));
    }

  if (vtkIntArray::SafeDownCast(
    this->LinkGraph->GetVertexData()->GetAbstractArray("active")))
    {
    // Extract only the active link graph.
    vtkSelection* activeSel = vtkSelection::New();
    activeSel->SetContentType(vtkSelection::VALUES);
    activeSel->SetFieldType(vtkSelection::VERTEX);
    vtkIntArray* list = vtkIntArray::New();
    list->SetName("active");
    list->InsertNextValue(1);
    activeSel->SetSelectionList(list);
    vtkExtractSelectedGraph* extract = vtkExtractSelectedGraph::New();
    extract->SetInput(0, this->LinkGraph);
    extract->SetInput(1, activeSel);
    extract->Update();
    vtkGraph* g = extract->GetOutput();
    this->LinkGraph->ShallowCopy(g);
    list->Delete();
    activeSel->Delete();
    extract->Delete();
    }

  vtkStringArray* linkColumn = vtkStringArray::SafeDownCast(
    this->LinkGraph->GetVertexData()->GetAbstractArray("column"));
  
  if (!linkColumn)
    {
    vtkErrorMacro("The link graph must have a string array named \"column\".");
    return 0;
    }

  vtkStringArray* linkDomain = vtkStringArray::SafeDownCast(
    this->LinkGraph->GetVertexData()->GetAbstractArray("domain"));
  vtkBitArray* linkHidden = vtkBitArray::SafeDownCast(
    this->LinkGraph->GetVertexData()->GetAbstractArray("hidden"));
  
  // Find all the hidden types
  vtksys_stl::set<vtkStdString> hiddenTypes;
  if (linkHidden)
    {
    for (vtkIdType h = 0; h < linkHidden->GetNumberOfTuples(); h++)
      {
      vtkStdString type;
      if (linkDomain)
        {
        type = linkDomain->GetValue(h);
        }
      if (linkHidden->GetValue(h))
        {
        hiddenTypes.insert(type);
        }
      }
    }

  // Calculate the percent time based on whether there are hidden types
  double createVertexTime = 0.25;
  double createEdgeTime = 0.75;
  double hiddenTime = 0.0;
  if (hiddenTypes.size() > 0)
    {
    createVertexTime = 0.1;
    createEdgeTime = 0.3;
    hiddenTime = 0.6;
    }

  // Create the auxiliary arrays.  These arrays summarize the
  // meaning of each row in the vertex table.
  // Domain contains the domain string of the vertex.
  // Value contains the string value of each vertex (appropriate for labeling).
  // Variant value contains the raw value of the vertex as a variant.
  VTK_CREATE(vtkStringArray, domainArr);
  domainArr->SetName("domain");
  VTK_CREATE(vtkStringArray, stringValueArr);
  stringValueArr->SetName("value");
  VTK_CREATE(vtkVariantArray, variantValueArr);
  variantValueArr->SetName("variantvalue");
  
  // Create the lookup maps for vertices and hidden vertices.
  // When edges are added later, we need to be able to lookup the
  // vertex ID for any domain-value pair.
  vtksys_stl::map<vtksys_stl::pair<vtkStdString, vtkVariant>, 
    vtkIdType, vtkTableToGraphCompare> vertexMap;
  vtksys_stl::map<vtksys_stl::pair<vtkStdString, vtkVariant>, 
    vtkIdType, vtkTableToGraphCompare> hiddenMap;
  
  // Set up the vertex table.  If we have one, just populate the 
  // auxiliary arrays and vetex maps.
  // If we are not provided one, create one using values found in
  // the edge table.
  if (!vertexTable)
    {
    // If we don't have a vertex table, 
    // create one by going through edge table columns.
    vertexTable = vtkTable::New();
    vtkIdType curVertex = 0;
    vtkIdType curHiddenVertex = 0;
    for (vtkIdType c = 0; c < linkColumn->GetNumberOfTuples(); c++)
      {
      vtkStdString domain = "globaldomain";
      if (linkDomain)
        {
        domain = linkDomain->GetValue(c);
        }
      int hidden = 0;
      if (linkHidden)
        {
        hidden = linkHidden->GetValue(c);
        }
      vtkStdString column = linkColumn->GetValue(c);
      vtkAbstractArray *arr = edgeTable->GetColumnByName(column);
      if (!arr)
        {
        vtkErrorMacro( "vtkTableToGraph cannot find edge array: " << column.c_str());
        vertexTable->Delete();
        return 0;
        }
      // For each new domain, add a variant array for that domain 
      // containing the values for only that domain.
      vtkVariantArray *domainValuesArr = 
        vtkVariantArray::SafeDownCast(vertexTable->GetColumnByName(domain));
      if (!domainValuesArr && !hidden)
        {
        domainValuesArr = vtkVariantArray::New();
        domainValuesArr->SetName(domain);
        for (vtkIdType i = 0; i < vertexTable->GetNumberOfRows(); ++i)
          {
          domainValuesArr->InsertNextValue(vtkVariant());
          }
        vertexTable->AddColumn(domainValuesArr);
        domainValuesArr->Delete();
        }
      if (hidden)
        {
        // If these vertices will be hidden, add vertices to the hiddenMap
        // but don't update the vertex table.
        switch(arr->GetDataType())
          {
          vtkExtendedTemplateMacro(vtkTableToGraphFindHiddenVertices(
            static_cast<VTK_TT*>(arr->GetVoidPointer(0)), 
            arr->GetNumberOfTuples(), hiddenMap, curHiddenVertex,
            domain));
          }
        }
      else
        {
        // If the vertices are not hidden, add vertices to the vertexMap,
        // auxiliary arrays, and add rows to the vertex table.
        switch(arr->GetDataType())
          {
          vtkExtendedTemplateMacro(vtkTableToGraphFindVertices(
            static_cast<VTK_TT*>(arr->GetVoidPointer(0)), 
            arr->GetNumberOfTuples(), vertexMap, domainArr, 
            stringValueArr, variantValueArr, curVertex,
            vertexTable, domain));
          }
        }
      double progress = createVertexTime * ((c + 1.0)/linkColumn->GetNumberOfTuples());
      this->InvokeEvent(vtkCommand::ProgressEvent, &progress);
      }
    }
  else
    {
    // The domain is what links the edge and vertex table,
    // so error if we don't have it.
    if (!linkDomain)
      {
      vtkErrorMacro("Domain is required when you have a vertex table");
      return 0;
      }
    
    // We know the number of vertices, so set the auxiliary array sizes.
    vtkIdType numRows = vertexTable->GetNumberOfRows();
    domainArr->SetNumberOfTuples(numRows);
    stringValueArr->SetNumberOfTuples(numRows);
    variantValueArr->SetNumberOfTuples(numRows);
    
    // Keep track of the current hidden vertex id.
    vtkIdType curHiddenVertex = 0;
    
    // For each new domain encountered, iterate through the values
    // of that column, adding vertices for each new value encountered.
    vtksys_stl::set<vtkStdString> domainSet;
    for (vtkIdType c = 0; c < linkDomain->GetNumberOfTuples(); c++)
      {
      vtkStdString domain = linkDomain->GetValue(c);
      if (domainSet.count(domain) > 0)
        {
        continue;
        }
      domainSet.insert(domain);
      int hidden = 0;
      if (linkHidden)
        {
        hidden = linkHidden->GetValue(c);
        }
      
      if (!hidden)
        {
        // If the domain is not hidden, find unique values in the vertex table
        // column.  If there are multiple matches in the column, only the
        // first vertex with that value will be used.
        vtkAbstractArray* arr = vertexTable->GetColumnByName(domain);
        if (!arr)
          {
          vtkErrorMacro("vtkTableToGraph cannot find vertex array: " << domain.c_str());
          return 0;
          }
        for (vtkIdType i = 0; i < arr->GetNumberOfTuples(); ++i)
          {
          vtkVariant val = vertexTable->GetValueByName(i, domain);
          vtksys_stl::pair<vtkStdString, vtkVariant> value(domain, val);
          if (vertexMap.count(value) == 0 && val.IsValid() && val.ToString().length() > 0)
            {
            vertexMap[value] = i;
            domainArr->InsertValue(i, domain);
            stringValueArr->InsertValue(i, val.ToString());
            variantValueArr->InsertValue(i, val);
            }
          }
        }
      else
        {
        // If the domain is hidden, we look through the edge table to 
        // find new hidden vertices which will not be correllated to the
        // vertex table.
        vtkStdString column = linkColumn->GetValue(c);
        vtkAbstractArray* edgeArr = edgeTable->GetColumnByName(column);
        if (!edgeArr)
          {
          vtkErrorMacro( "vtkTableToGraph cannot find edge array: " << column.c_str());
          return 0;
          }
        switch(edgeArr->GetDataType())
          {
          vtkExtendedTemplateMacro(vtkTableToGraphFindHiddenVertices(
            static_cast<VTK_TT*>(edgeArr->GetVoidPointer(0)), 
            edgeArr->GetNumberOfTuples(), hiddenMap, curHiddenVertex, domain));
          } // end switch
        } // end else !hidden
      double progress = createVertexTime * ((c + 1.0)/linkDomain->GetNumberOfTuples());
      this->InvokeEvent(vtkCommand::ProgressEvent, &progress);
      } // end for each domain
    } // end else !vertexTable

  // Create builder for the graph
  VTK_CREATE(vtkMutableDirectedGraph, dirBuilder);
  VTK_CREATE(vtkMutableUndirectedGraph, undirBuilder);
  vtkGraph *builder = 0;
  if (this->Directed)
    {
    builder = dirBuilder;
    }
  else
    {
    builder = undirBuilder;
    }

  // Add the correct number of vertices to the graph based on the number of
  // rows in the vertex table.
  builder->GetVertexData()->PassData(vertexTable->GetFieldData());
  for (vtkIdType i = 0; i < vertexTable->GetNumberOfRows(); ++i)
    {
    if (this->Directed)
      {
      dirBuilder->AddVertex();
      }
    else
      {
      undirBuilder->AddVertex();
      }
    }
  
  // Add the auxiliary arrays to the vertex table.
  builder->GetVertexData()->AddArray(stringValueArr);
  builder->GetVertexData()->AddArray(variantValueArr);
  builder->GetVertexData()->AddArray(domainArr);

  // Now go through the edge table, adding edges.
  // For each row in the edge table, add one edge to the
  // output graph for each edge in the link graph.
  VTK_CREATE(vtkDataSetAttributes, edgeTableData);
  edgeTableData->ShallowCopy(edgeTable->GetFieldData());
  builder->GetEdgeData()->CopyAllocate(edgeTableData);
  vtksys_stl::map<vtkIdType, vtksys_stl::vector< vtksys_stl::pair<vtkIdType, vtkIdType> > > hiddenInEdges;
  vtksys_stl::map<vtkIdType, vtksys_stl::vector<vtkIdType> > hiddenOutEdges;
  int numHiddenToHiddenEdges = 0;
  VTK_CREATE(vtkEdgeListIterator, edges);
  for (vtkIdType r = 0; r < edgeTable->GetNumberOfRows(); r++)
    {
    this->LinkGraph->GetEdges(edges);
    while (edges->HasNext())
      {
      vtkEdgeType e = edges->Next();
      vtkIdType linkSource = e.Source;
      vtkIdType linkTarget = e.Target;
      vtkStdString columnNameSource = linkColumn->GetValue(linkSource);
      vtkStdString columnNameTarget = linkColumn->GetValue(linkTarget);
      vtkStdString typeSource;
      vtkStdString typeTarget;
      if (linkDomain)
        {
        typeSource = linkDomain->GetValue(linkSource);
        typeTarget = linkDomain->GetValue(linkTarget);
        }
      int hiddenSource = 0;
      int hiddenTarget = 0;
      if (linkHidden)
        {
        hiddenSource = linkHidden->GetValue(linkSource);
        hiddenTarget = linkHidden->GetValue(linkTarget);
        }
      vtkAbstractArray* columnSource = edgeTable->GetColumnByName(columnNameSource);
      vtkAbstractArray* columnTarget = edgeTable->GetColumnByName(columnNameTarget);
      vtkVariant valueSource;
      if (!columnSource)
        {
        vtkErrorMacro( "vtkTableToGraph cannot find array: " << columnNameSource.c_str());
        return 0;
        }  
      switch(columnSource->GetDataType())
        {
        vtkExtendedTemplateMacro(valueSource = vtkTableToGraphGetValue(
          static_cast<VTK_TT*>(columnSource->GetVoidPointer(0)), r));
        }
      vtkVariant valueTarget;
      if (!columnTarget)
        {
        vtkErrorMacro( "vtkTableToGraph cannot find array: " << columnNameTarget.c_str());
        return 0;
        }  
      switch(columnTarget->GetDataType())
        {
        vtkExtendedTemplateMacro(valueTarget = vtkTableToGraphGetValue(
          static_cast<VTK_TT*>(columnTarget->GetVoidPointer(0)), r));
        }
      vtksys_stl::pair<vtkStdString, vtkVariant> lookupSource(typeSource, vtkVariant(valueSource));
      vtksys_stl::pair<vtkStdString, vtkVariant> lookupTarget(typeTarget, vtkVariant(valueTarget));
      vtkIdType source = -1;
      vtkIdType target = -1;
      if (!hiddenSource && vertexMap.count(lookupSource) > 0)
        {
        source = vertexMap[lookupSource];
        }
      else if (hiddenSource && hiddenMap.count(lookupSource) > 0)
        {
        source = hiddenMap[lookupSource];
        }
      if (!hiddenTarget && vertexMap.count(lookupTarget) > 0)
        {
        target = vertexMap[lookupTarget];
        }
      else if (hiddenTarget && hiddenMap.count(lookupTarget) > 0)
        {
        target = hiddenMap[lookupTarget];
        }

      if (!hiddenSource && !hiddenTarget)
        {
        if (source >= 0 && target >= 0)
          {
          vtkEdgeType newEdge;
          if (this->Directed)
            {
            newEdge = dirBuilder->AddEdge(source, target);
            }
          else
            {
            newEdge = undirBuilder->AddEdge(source, target);
            }
          builder->GetEdgeData()->CopyData(edgeTableData, r, newEdge.Id);
          }
        }
      else if (hiddenSource && !hiddenTarget)
        {
        hiddenOutEdges[source].push_back(target);
        }
      else if (!hiddenSource && hiddenTarget)
        {
        hiddenInEdges[target].push_back(vtksys_stl::make_pair(source, r));
        }
      else
        {
        // Cannot currently handle edges between hidden vertices.
        ++numHiddenToHiddenEdges;
        }
      }
    if (r % 100 == 0)
      {
      double progress = createVertexTime + createEdgeTime * r / edgeTable->GetNumberOfRows();
      this->InvokeEvent(vtkCommand::ProgressEvent, &progress);
      }
    }
  if (numHiddenToHiddenEdges > 0)
    {
    vtkWarningMacro(<<"TableToGraph does not currently support edges between hidden vertices.");
    }

  // Now add hidden edges.
  vtksys_stl::map<vtkIdType, vtksys_stl::vector<vtkIdType> >::iterator out, outEnd;
  out = hiddenOutEdges.begin();
  outEnd = hiddenOutEdges.end();
  vtkIdType curHidden = 0;
  vtkIdType numHidden = static_cast<vtkIdType>(hiddenOutEdges.size());
  for (; out != outEnd; ++out)
    {
    vtksys_stl::vector<vtkIdType> outVerts = out->second;
    vtksys_stl::vector< vtksys_stl::pair<vtkIdType, vtkIdType> > inVerts = hiddenInEdges[out->first];
    vtksys_stl::vector<vtkIdType>::size_type i, j;
    for (i = 0; i < inVerts.size(); ++i)
      {
      vtkIdType inVertId = inVerts[i].first;
      vtkIdType inEdgeId = inVerts[i].second;
      for (j = 0; j < outVerts.size(); ++j)
        {
        vtkEdgeType newEdge;
        if (this->Directed)
          {
          newEdge = dirBuilder->AddEdge(inVertId, outVerts[j]);
          }
        else
          {
          newEdge = undirBuilder->AddEdge(inVertId, outVerts[j]);
          }
        builder->GetEdgeData()->CopyData(edgeTableData, inEdgeId, newEdge.Id);
        }
      }
    if (curHidden % 100 == 0)
      {
      double progress = createVertexTime + createEdgeTime + hiddenTime * curHidden / numHidden;
      this->InvokeEvent(vtkCommand::ProgressEvent, &progress);
      }
    ++curHidden;
    }

  // Copy structure into output graph.
  vtkInformation* outputInfo = outputVector->GetInformationObject(0);
  vtkGraph* output = vtkGraph::SafeDownCast(
    outputInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!output->CheckedShallowCopy(builder))
    {
    vtkErrorMacro(<<"Invalid graph structure");
    return 0;
    }

  // Clean up
  if (!vertexTableInfo)
    {
    vertexTable->Delete();
    }
  
  return 1;
}

//----------------------------------------------------------------------------
int vtkTableToGraph::RequestDataObject(
  vtkInformation*, 
  vtkInformationVector**, 
  vtkInformationVector* )
{
  vtkGraph *output = 0;
  if (this->Directed)
    {
    output = vtkDirectedGraph::New();
    }
  else
    {
    output = vtkUndirectedGraph::New();
    }
  this->GetExecutive()->SetOutputData(0, output);
  output->Delete();

  return 1;
}

//---------------------------------------------------------------------------
unsigned long vtkTableToGraph::GetMTime()
{
  unsigned long time = this->Superclass::GetMTime();
  unsigned long linkGraphTime = this->LinkGraph->GetMTime();
  time = (linkGraphTime > time ? linkGraphTime : time);
  return time;
}

//---------------------------------------------------------------------------
void vtkTableToGraph::SetVertexTableConnection(vtkAlgorithmOutput* in)
{
  this->SetInputConnection(1, in);
}

//---------------------------------------------------------------------------
void vtkTableToGraph::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Directed: " << this->Directed << endl;
  os << indent << "LinkGraph: " << (this->LinkGraph ? "" : "(null)") << endl;
  if (this->LinkGraph)
    {
    this->LinkGraph->PrintSelf(os, indent.GetNextIndent());
    }
}
