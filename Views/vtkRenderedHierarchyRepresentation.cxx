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

#include "vtkRenderedHierarchyRepresentation.h"

#include "vtkActor.h"
#include "vtkApplyColors.h"
#include "vtkConvertSelection.h"
#include "vtkExtractSelectedGraph.h"
#include "vtkGraphHierarchicalBundleEdges.h"
#include "vtkGraphLayout.h"
#include "vtkGraphToPolyData.h"
#include "vtkHierarchicalGraphPipeline.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkPolyDataMapper.h"
#include "vtkProp.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderView.h"
#include "vtkSelection.h"
#include "vtkSelectionLink.h"
#include "vtkSelectionNode.h"
#include "vtkSplineGraphEdges.h"
#include "vtkTextProperty.h"
#include "vtkViewTheme.h"

#include <vtkstd/vector>

class vtkRenderedHierarchyRepresentation::Internals
{
public:
  vtkstd::vector<vtkSmartPointer<vtkHierarchicalGraphPipeline> > Graphs;
  vtkstd::vector<vtkSmartPointer<vtkActor> > ActorsToRemove;
};

vtkCxxRevisionMacro(vtkRenderedHierarchyRepresentation, "$Revision$");
vtkStandardNewMacro(vtkRenderedHierarchyRepresentation);

vtkRenderedHierarchyRepresentation::vtkRenderedHierarchyRepresentation()
{
  this->Implementation = new Internals;
  this->SetNumberOfInputPorts(2);
  this->Layout->SetZRange(0);
  this->EdgeVisibilityOff();
}

vtkRenderedHierarchyRepresentation::~vtkRenderedHierarchyRepresentation()
{
  delete this->Implementation;
}

bool vtkRenderedHierarchyRepresentation::ValidIndex(int idx)
{
  return (idx >= 0 &&
          idx < static_cast<int>(this->Implementation->Graphs.size()));
}

void vtkRenderedHierarchyRepresentation::SetGraphEdgeLabelArrayName(const char* name, int idx)
{
  if (this->ValidIndex(idx))
    {
    this->Implementation->Graphs[idx]->SetLabelArrayName(name);
    }
}

const char* vtkRenderedHierarchyRepresentation::GetGraphEdgeLabelArrayName(int idx)
{
  if (this->ValidIndex(idx))
    {
    return this->Implementation->Graphs[idx]->GetLabelArrayName();
    }
  return 0;
}

void vtkRenderedHierarchyRepresentation::SetGraphEdgeLabelVisibility(bool vis, int idx)
{
  if (this->ValidIndex(idx))
    {
    this->Implementation->Graphs[idx]->SetLabelVisibility(vis);
    }
}

bool vtkRenderedHierarchyRepresentation::GetGraphEdgeLabelVisibility(int idx)
{
  if (this->ValidIndex(idx))
    {
    return this->Implementation->Graphs[idx]->GetLabelVisibility();
    }
  return false;
}

void vtkRenderedHierarchyRepresentation::SetGraphEdgeColorArrayName(const char* name, int idx)
{
  if (this->ValidIndex(idx))
    {
    this->Implementation->Graphs[idx]->SetColorArrayName(name);
    }
}

const char* vtkRenderedHierarchyRepresentation::GetGraphEdgeColorArrayName(int idx)
{
  if (this->ValidIndex(idx))
    {
    return this->Implementation->Graphs[idx]->GetColorArrayName();
    }
  return 0;
}

void vtkRenderedHierarchyRepresentation::SetColorGraphEdgesByArray(bool vis, int idx)
{
  if (this->ValidIndex(idx))
    {
    this->Implementation->Graphs[idx]->SetColorEdgesByArray(vis);
    }
}

bool vtkRenderedHierarchyRepresentation::GetColorGraphEdgesByArray(int idx)
{
  if (this->ValidIndex(idx))
    {
    return this->Implementation->Graphs[idx]->GetColorEdgesByArray();
    }
  return false;
}

void vtkRenderedHierarchyRepresentation::SetGraphVisibility(bool vis, int idx)
{
  if (this->ValidIndex(idx))
    {
    this->Implementation->Graphs[idx]->SetVisibility(vis);
    }
}

bool vtkRenderedHierarchyRepresentation::GetGraphVisibility(int idx)
{
  if (this->ValidIndex(idx))
    {
    return this->Implementation->Graphs[idx]->GetVisibility();
    }
  return false;
}

void vtkRenderedHierarchyRepresentation::SetBundlingStrength(double strength, int idx)
{
  if (this->ValidIndex(idx))
    {
    this->Implementation->Graphs[idx]->SetBundlingStrength(strength);
    }
}

double vtkRenderedHierarchyRepresentation::GetBundlingStrength(int idx)
{
  if (this->ValidIndex(idx))
    {
    return this->Implementation->Graphs[idx]->GetBundlingStrength();
    }
  return 0.0;
}

void vtkRenderedHierarchyRepresentation::SetGraphEdgeLabelFontSize(int size, int idx)
{
  if (this->ValidIndex(idx))
    {
    this->Implementation->Graphs[idx]->GetLabelTextProperty()->SetFontSize(size);
    }
}

int vtkRenderedHierarchyRepresentation::GetGraphEdgeLabelFontSize(int idx)
{
  if (this->ValidIndex(idx))
    {
    return this->Implementation->Graphs[idx]->GetLabelTextProperty()->GetFontSize();
    }
  return 0;
}

bool vtkRenderedHierarchyRepresentation::AddToView(vtkView* view)
{
  this->Superclass::AddToView(view);
  vtkRenderView* rv = vtkRenderView::SafeDownCast(view);
  if (rv)
    {
    return true;
    }
  return false;
}

bool vtkRenderedHierarchyRepresentation::RemoveFromView(vtkView* view)
{
  this->Superclass::RemoveFromView(view);
  vtkRenderView* rv = vtkRenderView::SafeDownCast(view);
  if (rv)
    {
    return true;
    }
  return false;
}

vtkSelection* vtkRenderedHierarchyRepresentation::ConvertSelection(
  vtkView* view, vtkSelection* sel)
{
  vtkSelection* converted = this->Superclass::ConvertSelection(view, sel);

  int numGraphs = static_cast<int>(this->Implementation->Graphs.size());
  for (int i = 0; i < numGraphs; ++i)
    {
    vtkHierarchicalGraphPipeline* p = this->Implementation->Graphs[i];
    vtkSelection* conv = p->ConvertSelection(view, sel);
    if (conv)
      {
      for (unsigned int j = 0; j < conv->GetNumberOfNodes(); ++j)
        {
        converted->AddNode(conv->GetNode(j));
        }
      conv->Delete();
      }
    }
  //cerr << "Tree converted: " << endl;
  //converted->Dump();

  return converted;
}

void vtkRenderedHierarchyRepresentation::SetupInputConnections()
{
  this->Superclass::SetupInputConnections();

  // Add new graph objects if needed.
  size_t numGraphs = static_cast<size_t>(this->GetNumberOfInputConnections(1));
  while (numGraphs > this->Implementation->Graphs.size())
    {
    this->Implementation->Graphs.push_back(
      vtkSmartPointer<vtkHierarchicalGraphPipeline>::New());
    }

  // Keep track of actors to remove if the number of input connections
  // decreased.
  for (size_t i = numGraphs; i < this->Implementation->Graphs.size(); ++i)
    {
    this->Implementation->ActorsToRemove.push_back(
      this->Implementation->Graphs[i]->GetActor());
    }
  this->Implementation->Graphs.resize(numGraphs);

  // Setup input connections for bundled graphs.
  for (size_t i = 0; i < numGraphs; ++i)
    {
    vtkHierarchicalGraphPipeline* p = this->Implementation->Graphs[i];
    p->SetupInputConnections(
      this->GetInput(1, i)->GetProducerPort(),
      this->Layout->GetOutputPort(),
      this->GetAnnotationConnection(),
      this->GetSelectionConnection());
    }
}

void vtkRenderedHierarchyRepresentation::PrepareForRendering(vtkRenderView* view)
{
  for (size_t i = 0; i < this->Implementation->ActorsToRemove.size(); ++i)
    {
    view->GetRenderer()->RemoveActor(this->Implementation->ActorsToRemove[i]);
    }
  this->Implementation->ActorsToRemove.clear();
  for (size_t i = 0; i < this->Implementation->Graphs.size(); ++i)
    {
    if (!view->GetRenderer()->HasViewProp(this->Implementation->Graphs[i]->GetActor()))
      {
      view->GetRenderer()->AddActor(this->Implementation->Graphs[i]->GetActor());
      }
    }
}

void vtkRenderedHierarchyRepresentation::ApplyViewTheme(vtkViewTheme* theme)
{
  this->Superclass::ApplyViewTheme(theme);

  // Update all the graphs on the second input port before traversing them.
  this->Update();

  for (size_t i = 0; i < this->Implementation->Graphs.size(); ++i)
    {
    vtkHierarchicalGraphPipeline* p = this->Implementation->Graphs[i];
    p->ApplyViewTheme(theme);
    }
}

int vtkRenderedHierarchyRepresentation::FillInputPortInformation(int port, vtkInformation* info)
{
  if (port == 0)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkTree");
    return 1;
    }
  else if (port == 1)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkGraph");
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    info->Set(vtkAlgorithm::INPUT_IS_REPEATABLE(), 1);
    return 1;
    }
  return 0;
}

void vtkRenderedHierarchyRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
