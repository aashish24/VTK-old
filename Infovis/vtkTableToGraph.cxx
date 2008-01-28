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

#include "vtkTableToGraph.h"

#include "vtkBitArray.h"
#include "vtkCellData.h"
#include "vtkCommand.h"
#include "vtkDataArray.h"
#include "vtkEdgeListIterator.h"
#include "vtkExecutive.h"
#include "vtkGraph.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkIntArray.h"
#include "vtkMutableDirectedGraph.h"
#include "vtkMutableUndirectedGraph.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkSmartPointer.h"
#include "vtkStringArray.h"
#include "vtkTable.h"
#include "vtkVariant.h"
#include "vtkVariantArray.h"

#include <vtksys/stl/algorithm>
#include <vtksys/stl/map>
#include <vtksys/stl/set>
#include <vtksys/stl/vector>
using vtksys_stl::map;
using vtksys_stl::make_pair;
using vtksys_stl::pair;
using vtksys_stl::set;
using vtksys_stl::sort;
using vtksys_stl::vector;

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
    }
  else
    {
    this->LinkGraph->AddVertex();
    columnArr->InsertNextValue(column);
    domainArr->InsertNextValue(domainStr);
    hiddenArr->InsertNextValue(hidden);
    }
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkTableToGraph::ClearLinkVertices()
{
  if (!this->LinkGraph)
    {
    this->LinkGraph = vtkMutableDirectedGraph::New();
    }
  this->LinkGraph->Initialize();
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
    if (a.GetType() != b.GetType())
      {
      return a.GetType() < b.GetType();
      }
    if (a.GetType() == VTK_STRING)
      {
      return a.ToString() < b.ToString();
      }
    else
      {
      return a.ToDouble() < b.ToDouble();
      }
  }
};

//---------------------------------------------------------------------------
class vtkTableToGraphCompare
{
public:
  bool operator()(
    const pair<vtkStdString, vtkVariant>& a, 
    const pair<vtkStdString, vtkVariant>& b) const
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
template <typename T, typename MutableGraph>
vtkIdType vtkTableToGraphInsertVertices(
  T* arr,                            // The raw edge table column
  vtkIdType size,                    // The size of the edge table column
  vtkStdString domain,               // The domain for the column
  int hidden,                        // Whether this is a hidden column
  MutableGraph g,                    // The builder graph
  map<pair<vtkStdString, vtkVariant>, vtkIdType, vtkTableToGraphCompare>& vertexMap,
                                     // A map of domain-value pairs to graph id
  map<pair<vtkStdString, vtkVariant>, vtkIdType, vtkTableToGraphCompare>& hiddenMap,
                                     // A map of domain-value pairs to hidden vertex id
  vtkIdType & curHiddenVertex,       // The current hidden vertex id
  vtkStringArray* domainArr,         // An array that holds the domain of each vertex
  vtkStringArray* valueArr,          // An array that holds the string value of each vertex
  vtkVariantArray* variantValueArr,  // An array that holds the actual value of each vertex
  vtkDataSetAttributes* vertexTableData)     // A point data storing the input vertex table
{
  vtkIdType notFound = 0;
  vtkAbstractArray* tableArr = 0;
  if (vertexTableData)
    {
    tableArr = vertexTableData->GetAbstractArray(domain);
    }
  for (vtkIdType i = 0; i < size; i++)
    {
    T v = arr[i];
    pair<vtkStdString, vtkVariant> value(domain, vtkVariant(v));
    
    if (hidden)
      {
      if (hiddenMap.count(value) == 0)
        {
        hiddenMap[value] = curHiddenVertex;
        ++curHiddenVertex;
        }
      }
    else
      {
      if (vertexMap.count(value) == 0)
        {
        bool addVertex = true;
        vtkIdType outputVertexId = g->GetNumberOfVertices();
        
        // If we have a vertex table,
        // look for the value in the vertex table data.
        if (vertexTableData)
          {
          addVertex = false;
          if (tableArr)
            {
            vtkVariant var(v);
            vtkIdType inputVertexId = tableArr->LookupValue(var);
            if(inputVertexId >= 0)
              {
              g->GetVertexData()->CopyData(vertexTableData, inputVertexId, outputVertexId);
              addVertex = true;
              }
            else
              {
              addVertex = false;
              }
            }
          if (!addVertex)
            {
            notFound++;
            }
          }

        if (addVertex)
          {
          vtkIdType vertex = g->AddVertex();
          domainArr->InsertNextValue(domain);
          valueArr->InsertNextValue(vtkVariant(v).ToString());
          variantValueArr->InsertNextValue(vtkVariant(v));
          vertexMap[value] = vertex;
          }
        }
      }
    }
  return notFound;
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
  set<vtkStdString> hiddenTypes;
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
  double removeHiddenTime = 0.0;
  if (hiddenTypes.size() > 0)
    {
    createVertexTime = 0.1;
    createEdgeTime = 0.3;
    removeHiddenTime = 0.6;
    }

  VTK_CREATE(vtkStringArray, domainArr);
  domainArr->SetName("domain");
  
  VTK_CREATE(vtkStringArray, valueArr);
  valueArr->SetName("value");
  
  VTK_CREATE(vtkVariantArray, variantValueArr);
  variantValueArr->SetName("variantvalue");

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

  // Copy vertex data table into attributes.
  vtkDataSetAttributes *vertexTableData = 0;
  if (vertexTable && vertexTable->GetNumberOfRows() > 0)
    {
    vertexTableData = vtkDataSetAttributes::New();
    for (vtkIdType c = 0; c < vertexTable->GetNumberOfColumns(); c++)
      {
      vertexTableData->AddArray(vertexTable->GetColumn(c));
      }
    builder->GetVertexData()->CopyAllocate(vertexTableData);
    }

  // Go through all possible values in the columns in the link graph.
  map<pair<vtkStdString, vtkVariant>, vtkIdType, vtkTableToGraphCompare> vertexMap;
  map<pair<vtkStdString, vtkVariant>, vtkIdType, vtkTableToGraphCompare> hiddenMap;
  vtkIdType curHiddenVertex = 0;
  for (vtkIdType c = 0; c < linkColumn->GetNumberOfTuples(); c++)
    {
    vtkStdString domain;
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
    vtkAbstractArray* arr = edgeTable->GetColumnByName(column);
    if (!arr)
      {
      vtkErrorMacro( "vtkTableToGraph cannot find array: " << column.c_str());
      return 0;
      }
    if (this->Directed)
      {
      switch(arr->GetDataType())
        {
        vtkExtendedTemplateMacro(vtkTableToGraphInsertVertices(
          static_cast<VTK_TT*>(arr->GetVoidPointer(0)), 
          arr->GetNumberOfTuples(), domain, hidden, dirBuilder, vertexMap, hiddenMap,
          curHiddenVertex, domainArr, valueArr, variantValueArr, vertexTableData));
        }
      }
    else
      {
      switch(arr->GetDataType())
        {
        vtkExtendedTemplateMacro(vtkTableToGraphInsertVertices(
          static_cast<VTK_TT*>(arr->GetVoidPointer(0)), 
          arr->GetNumberOfTuples(), domain, hidden, undirBuilder, vertexMap, hiddenMap,
          curHiddenVertex, domainArr, valueArr, variantValueArr, vertexTableData));
        }
      }
    double progress = createVertexTime * c / linkColumn->GetNumberOfTuples();
    this->InvokeEvent(vtkCommand::ProgressEvent, &progress);
    }  

  builder->GetVertexData()->AddArray(valueArr);
  builder->GetVertexData()->AddArray(variantValueArr);
  builder->GetVertexData()->AddArray(domainArr);

  // Now go through the table, adding edges
  VTK_CREATE(vtkDataSetAttributes, edgeTableData);
  edgeTableData->ShallowCopy(edgeTable->GetFieldData());
  builder->GetEdgeData()->CopyAllocate(edgeTableData);
  map<vtkIdType, vector< pair<vtkIdType, vtkIdType> > > hiddenInEdges;
  map<vtkIdType, vector<vtkIdType> > hiddenOutEdges;
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
      pair<vtkStdString, vtkVariant> lookupSource(typeSource, vtkVariant(valueSource));
      pair<vtkStdString, vtkVariant> lookupTarget(typeTarget, vtkVariant(valueTarget));
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
        hiddenInEdges[target].push_back(make_pair(source, r));
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
  map<vtkIdType, vector<vtkIdType> >::iterator out, outEnd;
  out = hiddenOutEdges.begin();
  outEnd = hiddenOutEdges.end();
  for (; out != outEnd; ++out)
    {
    vector<vtkIdType> outVerts = out->second;
    vector< pair<vtkIdType, vtkIdType> > inVerts = hiddenInEdges[out->first];
    vector<vtkIdType>::size_type i, j;
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
  if (vertexTableData)
    {
    vertexTableData->Delete();
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
}
