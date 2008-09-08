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

#include "vtkGraphLayoutView.h"

#include "vtkActor.h"
#include "vtkActor2D.h"
#include "vtkAlgorithmOutput.h"
#include "vtkArcParallelEdgeStrategy.h"
#include "vtkCamera.h"
#include "vtkCellData.h"
#include "vtkCircularLayoutStrategy.h"
#include "vtkClustering2DLayoutStrategy.h"
#include "vtkCommand.h"
#include "vtkCommunity2DLayoutStrategy.h"
#include "vtkConstrained2DLayoutStrategy.h"
#include "vtkCoordinate.h"
#include "vtkConvertSelection.h"
#include "vtkDataRepresentation.h"
#include "vtkDynamic2DLabelMapper.h"
#include "vtkEdgeCenters.h"
#include "vtkEdgeLayout.h"
#include "vtkExtractSelectedGraph.h"
#include "vtkFast2DLayoutStrategy.h"
#include "vtkForceDirectedLayoutStrategy.h"
#include "vtkGraphLayout.h"
#include "vtkGraphMapper.h"
#include "vtkGraphToPolyData.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkInteractorStyleRubberBand2D.h"
#include "vtkKdTreeSelector.h"
#include "vtkLookupTable.h"
#include "vtkObjectFactory.h"
#include "vtkPassThroughLayoutStrategy.h"
#include "vtkPassThroughEdgeStrategy.h"
#include "vtkPointData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRandomLayoutStrategy.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkScalarBarActor.h"
#include "vtkScalarBarWidget.h"
#include "vtkSelection.h"
#include "vtkSelectionLink.h"
#include "vtkSimple2DLayoutStrategy.h"
#include "vtkTextProperty.h"
#include "vtkVertexDegree.h"
#include "vtkVertexGlyphFilter.h"
#include "vtkViewTheme.h"
#include "vtkVisibleCellSelector.h"

#include <ctype.h> // for tolower()

vtkCxxRevisionMacro(vtkGraphLayoutView, "$Revision$");
vtkStandardNewMacro(vtkGraphLayoutView);
//----------------------------------------------------------------------------
vtkGraphLayoutView::vtkGraphLayoutView()
{
  this->Coordinate             = vtkSmartPointer<vtkCoordinate>::New();
  this->GraphLayout            = vtkSmartPointer<vtkGraphLayout>::New();
  this->RandomStrategy         = vtkSmartPointer<vtkRandomLayoutStrategy>::New();
  this->Simple2DStrategy       = vtkSmartPointer<vtkSimple2DLayoutStrategy>::New();
  this->Clustering2DStrategy   = vtkSmartPointer<vtkClustering2DLayoutStrategy>::New();
  this->Community2DStrategy    = vtkSmartPointer<vtkCommunity2DLayoutStrategy>::New();
  this->Constrained2DStrategy  = vtkSmartPointer<vtkConstrained2DLayoutStrategy>::New();
  this->Fast2DStrategy         = vtkSmartPointer<vtkFast2DLayoutStrategy>::New();
  this->ForceDirectedStrategy  = vtkSmartPointer<vtkForceDirectedLayoutStrategy>::New();
  this->PassThroughStrategy    = vtkSmartPointer<vtkPassThroughLayoutStrategy>::New();
  this->CircularStrategy       = vtkSmartPointer<vtkCircularLayoutStrategy>::New();
  this->EdgeLayout             = vtkSmartPointer<vtkEdgeLayout>::New();
  this->ArcParallelStrategy    = vtkSmartPointer<vtkArcParallelEdgeStrategy>::New();
  this->PassThroughEdgeStrategy = vtkSmartPointer<vtkPassThroughEdgeStrategy>::New();
  this->VertexDegree           = vtkSmartPointer<vtkVertexDegree>::New();
  this->EdgeCenters            = vtkSmartPointer<vtkEdgeCenters>::New();
  this->GraphMapper            = vtkSmartPointer<vtkGraphMapper>::New();
  this->GraphActor             = vtkSmartPointer<vtkActor>::New();
  this->VertexLabelMapper      = vtkSmartPointer<vtkDynamic2DLabelMapper>::New();
  this->VertexLabelActor       = vtkSmartPointer<vtkActor2D>::New();
  this->EdgeLabelMapper        = vtkSmartPointer<vtkDynamic2DLabelMapper>::New();
  this->EdgeLabelActor         = vtkSmartPointer<vtkActor2D>::New();
  this->VisibleCellSelector    = vtkSmartPointer<vtkVisibleCellSelector>::New();
  this->KdTreeSelector         = vtkSmartPointer<vtkKdTreeSelector>::New();
  this->ExtractSelectedGraph   = vtkSmartPointer<vtkExtractSelectedGraph>::New();
  this->SelectedGraphMapper    = vtkSmartPointer<vtkGraphMapper>::New();
  this->SelectedGraphActor     = vtkSmartPointer<vtkActor>::New();
  this->VertexScalarBar        = vtkSmartPointer<vtkScalarBarWidget>::New();
  this->EdgeScalarBar          = vtkSmartPointer<vtkScalarBarWidget>::New();
  
  this->LayoutStrategyNameInternal = 0;
  this->EdgeLayoutStrategyNameInternal = 0;
  this->IconArrayNameInternal = 0;
  
  // Replace the interactor style.
  vtkInteractorStyleRubberBand2D* style = vtkInteractorStyleRubberBand2D::New();
  this->SetInteractorStyle(style);
  style->Delete();
  
  // Setup view
  this->Renderer->GetActiveCamera()->ParallelProjectionOn();
  //this->InteractorStyle->AddObserver(vtkCommand::SelectionChangedEvent, this->GetObserver());
  this->Coordinate->SetCoordinateSystemToDisplay();
  
  // Setup parameters on the various mappers and actors
  this->VertexLabelMapper->SetLabelModeToLabelFieldData();
  this->VertexLabelMapper->GetLabelTextProperty()->SetColor(1,1,1);
  this->VertexLabelMapper->GetLabelTextProperty()->SetJustificationToCentered();
  this->VertexLabelMapper->GetLabelTextProperty()->SetVerticalJustificationToCentered();
  this->VertexLabelMapper->GetLabelTextProperty()->SetFontSize(12);
  this->VertexLabelMapper->GetLabelTextProperty()->SetItalic(0);
  this->VertexLabelMapper->GetLabelTextProperty()->SetLineOffset(-10);
  this->VertexLabelMapper->SetPriorityArrayName("VertexDegree");
  this->VertexLabelActor->PickableOff();
  this->EdgeLabelMapper->SetLabelModeToLabelFieldData();
  this->EdgeLabelMapper->GetLabelTextProperty()->SetColor(.7,.7,1);
  this->EdgeLabelMapper->GetLabelTextProperty()->SetJustificationToCentered();
  this->EdgeLabelMapper->GetLabelTextProperty()->SetVerticalJustificationToCentered();
  this->EdgeLabelMapper->GetLabelTextProperty()->SetFontSize(10);
  this->EdgeLabelMapper->GetLabelTextProperty()->SetItalic(0);
  this->EdgeLabelMapper->GetLabelTextProperty()->SetLineOffset(-10);
  this->EdgeLabelActor->PickableOff();
  this->SelectedGraphActor->PickableOff();
  this->SelectedGraphActor->SetPosition(0, 0, -0.01);
  this->SelectedGraphMapper->SetScalarVisibility(false);
  this->VertexScalarBar->GetScalarBarActor()->SetLookupTable(
      this->GraphMapper->GetVertexLookupTable());
  this->EdgeScalarBar->GetScalarBarActor()->SetLookupTable(
      this->GraphMapper->GetEdgeLookupTable());
  
  // Set default parameters
  this->SetVertexLabelArrayName("label");
  this->VertexLabelVisibilityOff();
  this->SetEdgeLabelArrayName("label");
  this->EdgeLabelVisibilityOff();
  this->SetVertexColorArrayName("VertexDegree");
  this->ColorVerticesOff();
  this->SetEdgeColorArrayName("weight");
  this->ColorEdgesOff();
  this->EdgeLayoutStrategy   = 0;
  this->EdgeLayoutPreference = this->ArcParallelStrategy;
  this->SetLayoutStrategyToFast2D();
  this->SetEdgeLayoutStrategyToArcParallel();
  
  // Apply default theme
  vtkViewTheme* theme = vtkViewTheme::New();
  this->ApplyViewTheme(theme);
  theme->Delete();
  
  // Connect pipeline
  this->EdgeLayout->SetInputConnection(this->GraphLayout->GetOutputPort());
  this->VertexDegree->SetInputConnection(this->EdgeLayout->GetOutputPort());
  
  this->GraphMapper->SetInputConnection(this->VertexDegree->GetOutputPort());
  this->GraphActor->SetMapper(this->GraphMapper);
  this->VertexLabelMapper->SetInputConnection(this->VertexDegree->GetOutputPort());
  this->VertexLabelActor->SetMapper(this->VertexLabelMapper);
  this->EdgeCenters->SetInputConnection(this->VertexDegree->GetOutputPort());
  this->EdgeLabelMapper->SetInputConnection(this->EdgeCenters->GetOutputPort());
  this->EdgeLabelActor->SetMapper(this->EdgeLabelMapper);

  this->KdTreeSelector->SetInputConnection(this->GraphLayout->GetOutputPort());
  this->ExtractSelectedGraph->SetInputConnection(0, this->EdgeLayout->GetOutputPort());
  vtkSelection* empty = vtkSelection::New();
  empty->SetContentType(vtkSelection::INDICES);
  vtkIdTypeArray* arr = vtkIdTypeArray::New();
  empty->SetSelectionList(arr);
  arr->Delete();
  this->ExtractSelectedGraph->SetInput(1, empty);
  empty->Delete();
  
  this->SelectedGraphMapper->SetInputConnection(this->ExtractSelectedGraph->GetOutputPort());
  this->SelectedGraphActor->SetMapper(this->SelectedGraphMapper);

  // Register for progress.
  this->RegisterProgress(this->GraphLayout);
  this->RegisterProgress(this->EdgeLayout);
  this->RegisterProgress(this->GraphMapper);
  this->RegisterProgress(this->VertexLabelMapper);
  this->RegisterProgress(this->EdgeLabelMapper);
  this->RegisterProgress(this->ExtractSelectedGraph);
  this->RegisterProgress(this->SelectedGraphMapper);
  this->RegisterProgress(this->EdgeCenters);
}

//----------------------------------------------------------------------------
vtkGraphLayoutView::~vtkGraphLayoutView()
{
  // Delete internally created objects.
  // Note: All of the smartpointer objects 
  //       will be deleted for us
    
  vtkGraphLayoutStrategy *nullGraphLayout = 0;
  this->SetLayoutStrategy(nullGraphLayout);
  vtkEdgeLayoutStrategy *nullEdgeLayout = 0;
  this->SetEdgeLayoutStrategy(nullEdgeLayout);
  this->SetLayoutStrategyNameInternal(0);
  this->SetEdgeLayoutStrategyNameInternal(0);
  this->SetIconArrayNameInternal(0);

  // UnRegister for progress.
  this->UnRegisterProgress(this->GraphLayout);
  this->UnRegisterProgress(this->EdgeLayout);
  this->UnRegisterProgress(this->GraphMapper);
  this->UnRegisterProgress(this->VertexLabelMapper);
  this->UnRegisterProgress(this->EdgeLabelMapper);
  this->UnRegisterProgress(this->ExtractSelectedGraph);
  this->UnRegisterProgress(this->SelectedGraphMapper);
  this->UnRegisterProgress(this->EdgeCenters);
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::SetVertexLabelArrayName(const char* name)
{
  this->VertexLabelMapper->SetFieldDataName(name);
}

//----------------------------------------------------------------------------
const char* vtkGraphLayoutView::GetVertexLabelArrayName()
{
  return this->VertexLabelMapper->GetFieldDataName();
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::SetEdgeLabelArrayName(const char* name)
{
  this->EdgeLabelMapper->SetFieldDataName(name);
}

//----------------------------------------------------------------------------
const char* vtkGraphLayoutView::GetEdgeLabelArrayName()
{
  return this->EdgeLabelMapper->GetFieldDataName();
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::SetVertexLabelVisibility(bool vis)
{
  this->VertexLabelActor->SetVisibility(vis);
}

//----------------------------------------------------------------------------
bool vtkGraphLayoutView::GetVertexLabelVisibility()
{
  return this->VertexLabelActor->GetVisibility() ? true : false;
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::VertexLabelVisibilityOn()
{
  this->VertexLabelActor->SetVisibility(true);
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::VertexLabelVisibilityOff()
{
  this->VertexLabelActor->SetVisibility(false);
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::SetEdgeLabelVisibility(bool vis)
{
  this->EdgeLabelActor->SetVisibility(vis);
}

//----------------------------------------------------------------------------
bool vtkGraphLayoutView::GetEdgeLabelVisibility()
{
  return this->EdgeLabelActor->GetVisibility() ? true : false;
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::EdgeLabelVisibilityOn()
{
  this->EdgeLabelActor->SetVisibility(true);
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::EdgeLabelVisibilityOff()
{
  this->EdgeLabelActor->SetVisibility(false);
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::SetVertexColorArrayName(const char* name)
{
  this->GraphMapper->SetVertexColorArrayName(name);
  this->VertexScalarBar->GetScalarBarActor()->SetTitle(name);
}

//----------------------------------------------------------------------------
const char* vtkGraphLayoutView::GetVertexColorArrayName()
{
  return this->GraphMapper->GetVertexColorArrayName();
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::SetColorVertices(bool vis)
{
  this->GraphMapper->SetColorVertices(vis);
}

//----------------------------------------------------------------------------
bool vtkGraphLayoutView::GetColorVertices()
{
  return this->GraphMapper->GetColorVertices();
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::ColorVerticesOn()
{
  this->GraphMapper->ColorVerticesOn();
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::ColorVerticesOff()
{
  this->GraphMapper->ColorVerticesOff();
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::SetVertexScalarBarVisibility(bool vis)
{
  this->VertexScalarBar->SetEnabled(vis);
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::SetEdgeColorArrayName(const char* name)
{
  this->GraphMapper->SetEdgeColorArrayName(name);
  this->EdgeScalarBar->GetScalarBarActor()->SetTitle(name);
}

//----------------------------------------------------------------------------
const char* vtkGraphLayoutView::GetEdgeColorArrayName()
{
  return this->GraphMapper->GetEdgeColorArrayName();
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::SetColorEdges(bool vis)
{
  this->GraphMapper->SetColorEdges(vis);
}

//----------------------------------------------------------------------------
bool vtkGraphLayoutView::GetColorEdges()
{
  return this->GraphMapper->GetColorEdges();
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::ColorEdgesOn()
{
  this->GraphMapper->ColorEdgesOn();
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::ColorEdgesOff()
{
  this->GraphMapper->ColorEdgesOff();
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::SetEdgeScalarBarVisibility(bool vis)
{
  this->EdgeScalarBar->SetEnabled(vis);
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::SetScaledGlyphs(bool arg)
{
  this->GraphMapper->SetScaledGlyphs(arg);
  vtkWarningMacro("Setting ScaledGlyphs to " << arg);
}

//----------------------------------------------------------------------------
bool vtkGraphLayoutView::GetScaledGlyphs()
{
  return this->GraphMapper->GetScaledGlyphs();
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::SetScalingArrayName(const char* name)
{
  this->GraphMapper->SetScalingArrayName(name);
}

//----------------------------------------------------------------------------
const char* vtkGraphLayoutView::GetScalingArrayName()
{
  return this->GraphMapper->GetScalingArrayName();
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::SetIconArrayName(const char* name)
{
  this->SetIconArrayNameInternal(name);
  this->GraphMapper->SetIconArrayName(name);
}

//----------------------------------------------------------------------------
const char* vtkGraphLayoutView::GetIconArrayName()
{
  return this->GetIconArrayNameInternal();
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::AddIconType(char *type, int index)
{
  this->GraphMapper->AddIconType(type, index);
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::ClearIconTypes()
{
  this->GraphMapper->ClearIconTypes();
}

//----------------------------------------------------------------------------
int vtkGraphLayoutView::IsLayoutComplete()
{
  if (this->GraphLayout)
    {
    return this->GraphLayout->IsLayoutComplete();
    }
    
  // If I don't have a strategy I guess it's
  // better to say I'm done than not done :)
  return 1;
}
  
//----------------------------------------------------------------------------
void vtkGraphLayoutView::UpdateLayout()
{
  // See if the graph layout is complete
  // if it's not then set it as modified
  // and call a render on the render window
  if (!this->IsLayoutComplete())
    {
    this->GraphLayout->Modified();
    if (this->GetRenderWindow())
      {
      this->Renderer->ResetCamera();
      this->GetRenderWindow()->Render();
      }
    }
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::SetLayoutStrategy(vtkGraphLayoutStrategy *s)
{
  // Set the edge layout to pass through if the graph layout is.
  if(vtkPassThroughLayoutStrategy::SafeDownCast(s))
    {
    this->EdgeLayoutPreference = this->EdgeLayoutStrategy;
    this->SetEdgeLayoutStrategy("passthrough");
    }
  else if(this->EdgeLayoutStrategy != this->EdgeLayoutPreference)
    {
    // Otherwise, set it to whatever our preferred strategy is
    this->SetEdgeLayoutStrategy(this->EdgeLayoutPreference);
    }

  this->LayoutStrategy = s;
  this->GraphLayout->SetLayoutStrategy(this->LayoutStrategy);
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::SetLayoutStrategy(const char* name)
{
  this->LayoutStrategy = this->Simple2DStrategy;
  if (!name)
    {
    return;
    }
  
  // Take out spaces and make lowercase.
  char str[20];
  strncpy(str, name, 20);
  int pos = 0;
  for (int i = 0; str[i] != '\0' && i < 20; i++, pos++)
    {
    if (str[i] == ' ')
      {
      pos--;
      }
    else
      {
      str[pos] = tolower(str[i]);
      }
    }
  str[pos] = '\0';
  
  if (!strcmp(str, "random"))
    {
    this->LayoutStrategy = this->RandomStrategy;
    }
  else if (!strcmp(str, "forcedirected"))
    {
    this->LayoutStrategy = this->ForceDirectedStrategy;
    }
  else if (!strcmp(str, "simple2d"))
    {
    this->LayoutStrategy = this->Simple2DStrategy;
    }
  else if (!strcmp(str, "clustering2d"))
    {
    this->LayoutStrategy = this->Clustering2DStrategy;
    }
  else if (!strcmp(str, "community2d"))
    {
    this->LayoutStrategy = this->Community2DStrategy;
    }
  else if (!strcmp(str, "constrained2d"))
    {
    this->LayoutStrategy = this->Constrained2DStrategy;
    }
  else if (!strcmp(str, "fast2d"))
    {
    this->LayoutStrategy = this->Fast2DStrategy;
    }
  else if (!strcmp(str, "passthrough"))
    {
    this->LayoutStrategy = this->PassThroughStrategy;
    }
  else if (!strcmp(str, "circular"))
    {
    this->LayoutStrategy = this->CircularStrategy;
    }
  else
    {
    vtkErrorMacro("Unknown strategy " << name << " (" << str << ").");
    return;
    }

  // Set the edge layout to pass through if the graph layout is.
  if(vtkPassThroughLayoutStrategy::SafeDownCast(this->LayoutStrategy))
    {
    this->EdgeLayoutPreference = this->EdgeLayoutStrategy;
    this->SetEdgeLayoutStrategy("passthrough");
    }
  else if(this->EdgeLayoutStrategy != this->EdgeLayoutPreference)
    {
    // Otherwise, set it to whatever our preferred strategy is
    this->SetEdgeLayoutStrategy(this->EdgeLayoutPreference);
    }

  this->GraphLayout->SetLayoutStrategy(this->LayoutStrategy);
  this->SetLayoutStrategyNameInternal(name);
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::SetEdgeLayoutStrategy(vtkEdgeLayoutStrategy *s)
{
  // If our graph layout strategy is PassThrough, just store this edge
  // layout strategy for later
  if(vtkPassThroughLayoutStrategy::SafeDownCast(this->LayoutStrategy))
    {
    this->EdgeLayoutPreference = s;
    return;
    }

  this->EdgeLayoutStrategy = s;
  this->EdgeLayout->SetLayoutStrategy(this->EdgeLayoutStrategy);
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::SetEdgeLayoutStrategy(const char* name)
{
  this->EdgeLayoutStrategy = this->ArcParallelStrategy;
  if (!name)
    {
    return;
    }
  
  // Take out spaces and make lowercase.
  char str[20];
  strncpy(str, name, 20);
  int pos = 0;
  for (int i = 0; str[i] != '\0' && i < 20; i++, pos++)
    {
    if (str[i] == ' ')
      {
      pos--;
      }
    else
      {
      str[pos] = tolower(str[i]);
      }
    }
  str[pos] = '\0';
  
  if (!strcmp(str, "arcparallel"))
    {
    this->EdgeLayoutStrategy = this->ArcParallelStrategy;
    }
  else if (!strcmp(str, "passthrough"))
    {
    this->EdgeLayoutStrategy = this->PassThroughEdgeStrategy;
    }
  else
    {
    vtkErrorMacro("Unknown strategy " << name << " (" << str << ").");
    return;
    }

  // If our graph layout strategy is PassThrough, just store this edge
  // layout strategy for later
  if(vtkPassThroughLayoutStrategy::SafeDownCast(this->LayoutStrategy))
    {
    this->EdgeLayoutPreference = this->EdgeLayoutStrategy;
    this->EdgeLayoutStrategy = this->PassThroughEdgeStrategy;
    return;
    }

  this->EdgeLayout->SetLayoutStrategy(this->EdgeLayoutStrategy);
  this->SetEdgeLayoutStrategyNameInternal(name);
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::SetIterationsPerLayout(int iterations)
{
  // Hmmm... this seems a bit silly, probably a better way
  vtkGraphLayoutStrategy* strategy = this->GraphLayout->GetLayoutStrategy();
  vtkSimple2DLayoutStrategy *simple = 
    vtkSimple2DLayoutStrategy::SafeDownCast(strategy);
  vtkFast2DLayoutStrategy *fast = 
    vtkFast2DLayoutStrategy::SafeDownCast(strategy);
  if (simple)
    {
    simple->SetIterationsPerLayout(iterations);
    }
  else if (fast)
    {
    fast->SetIterationsPerLayout(iterations);
    }
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::SetIconTexture(vtkTexture *texture)
{
  this->GraphMapper->SetIconTexture(texture);
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::SetIconSize(int *size)
{
  this->GraphMapper->SetIconSize(size);
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::SetIconAlignment(int alignment)
{
  this->GraphMapper->SetIconAlignment(alignment);
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::SetIconVisibility(bool b)
{
  this->GraphMapper->SetIconVisibility(b);
}

//----------------------------------------------------------------------------
bool vtkGraphLayoutView::GetIconVisibility()
{
  return this->GraphMapper->GetIconVisibility();
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::SetupRenderWindow(vtkRenderWindow* win)
{
  this->Superclass::SetupRenderWindow(win);
  win->GetInteractor()->SetInteractorStyle(this->InteractorStyle);
  this->VertexScalarBar->SetInteractor(win->GetInteractor());
  this->EdgeScalarBar->SetInteractor(win->GetInteractor());
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::AddInputConnection( int port, int item,
  vtkAlgorithmOutput* conn, vtkAlgorithmOutput* selectionConn)
{
  if( port != 0 || item != 0 )
  {
    vtkErrorMacro("This view only supports one representation.");
    return;
  }
  else if (this->GraphLayout->GetNumberOfInputConnections(0) == 0)
    {
    this->GraphLayout->SetInputConnection(conn);
    this->ExtractSelectedGraph->SetInputConnection(1, selectionConn);
  
    this->Renderer->AddActor(this->GraphActor);
    this->Renderer->AddActor(this->SelectedGraphActor);
    this->Renderer->AddActor(this->VertexLabelActor);
    this->Renderer->AddActor(this->EdgeLabelActor);
    }
  else
    {
    vtkErrorMacro("This view only supports one representation.");
    return;
    }
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::RemoveInputConnection( int port, int item,
  vtkAlgorithmOutput* conn, vtkAlgorithmOutput* selectionConn)
{
  if( port != 0 || item != 0 )
    {
    vtkErrorMacro("This view only supports one representation.");
    }

  if (this->GraphLayout->GetNumberOfInputConnections(0) > 0 &&
      this->GraphLayout->GetInputConnection(0, 0) == conn)
    {
    this->GraphLayout->RemoveInputConnection(0, conn);
    this->ExtractSelectedGraph->RemoveInputConnection(1, selectionConn);
  
    this->Renderer->RemoveActor(this->GraphActor);
    this->Renderer->RemoveActor(this->SelectedGraphActor);
    this->Renderer->RemoveActor(this->VertexLabelActor);
    this->Renderer->RemoveActor(this->EdgeLabelActor);
    }
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::MapToXYPlane(
  double displayX, double displayY, 
  double &x, double &y)
{
  this->Coordinate->SetViewport(this->Renderer);
  this->Coordinate->SetValue(displayX, displayY);
  double* pt = this->Coordinate->GetComputedWorldValue(0);

  vtkCamera* camera = this->Renderer->GetActiveCamera();
  double cameraPos[3];
  camera->GetPosition(cameraPos);

  double t = -cameraPos[2] / (pt[2] - cameraPos[2]);
  double r[3];
  for (vtkIdType i = 0; i < 3; i++)
    {
    r[i] = cameraPos[i] + t*(pt[i] - cameraPos[i]);
    }
  x = r[0];
  y = r[1];
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::ProcessEvents(
  vtkObject* caller, 
  unsigned long eventId, 
  void* callData)
{
  if (caller == this->InteractorStyle && eventId == vtkCommand::SelectionChangedEvent
      && this->GraphLayout->GetNumberOfInputConnections(0) > 0)
    {
    // Create the selection
    unsigned int* rect = reinterpret_cast<unsigned int*>(callData);
    bool singleSelectMode = false;
    unsigned int pos1X = rect[0];
    unsigned int pos1Y = rect[1];
    unsigned int pos2X = rect[2];
    unsigned int pos2Y = rect[3];
    int stretch = 2;
    if (pos1X == pos2X && pos1Y == pos2Y)
      {
      singleSelectMode = true;
      pos1X = pos1X - stretch > 0 ? pos1X - stretch : 0;
      pos1Y = pos1Y - stretch > 0 ? pos1Y - stretch : 0;
      pos2X = pos2X + stretch;
      pos2Y = pos2Y + stretch;
      }
    double pt1[2];
    double pt2[2];
    this->MapToXYPlane(pos1X, pos1Y, pt1[0], pt1[1]);
    this->MapToXYPlane(pos2X, pos2Y, pt2[0], pt2[1]);
    double minX = pt1[0] < pt2[0] ? pt1[0] : pt2[0];
    double maxX = pt1[0] < pt2[0] ? pt2[0] : pt1[0];
    double minY = pt1[1] < pt2[1] ? pt1[1] : pt2[1];
    double maxY = pt1[1] < pt2[1] ? pt2[1] : pt1[1];
    this->KdTreeSelector->SetSelectionBounds(
      minX, maxX, minY, maxY, -1, 1);
    this->KdTreeSelector->SetSingleSelection(singleSelectMode);
    double radiusX = 2*(maxX-minX);
    double radiusY = 2*(maxY-minY);
    double dist2 = radiusX*radiusX + radiusY*radiusY;
    this->KdTreeSelector->SetSingleSelectionThreshold(dist2);
    this->KdTreeSelector->Update();
    vtkSelection* kdSelection = this->KdTreeSelector->GetOutput();

    // Convert to the proper selection type.
    this->GraphLayout->Update();
    vtkDataObject* data = this->GraphLayout->GetOutput();
    vtkSmartPointer<vtkSelection> selection;
    selection.TakeReference(vtkConvertSelection::ToSelectionType(
      kdSelection, data, this->SelectionType, this->SelectionArrayNames));
    
    // FIXME: Selection on edges needs to be supported.
#if 0
    // If the selection is empty, do a visible cell selection
    // to attempt to pick up an edge.
    vtkAbstractArray* list = selection->GetSelectionList();
    if (list && (list->GetNumberOfTuples() == 0))
      {
      // The edge actor must be opaque for visible cell selector.
      double opacity = this->EdgeActor->GetProperty()->GetOpacity();
      this->EdgeActor->GetProperty()->SetOpacity(1);
      
      unsigned int screenMinX = pos1X < pos2X ? pos1X : pos2X;
      unsigned int screenMaxX = pos1X < pos2X ? pos2X : pos1X;
      unsigned int screenMinY = pos1Y < pos2Y ? pos1Y : pos2Y;
      unsigned int screenMaxY = pos1Y < pos2Y ? pos2Y : pos1Y;
      this->VisibleCellSelector->SetRenderer(this->Renderer);
      this->VisibleCellSelector->SetArea(screenMinX, screenMinY, screenMaxX, screenMaxY);
      this->VisibleCellSelector->SetProcessorId(0);
      this->VisibleCellSelector->SetRenderPasses(0, 0, 0, 0, 1);
      this->VisibleCellSelector->Select();  
      vtkIdTypeArray* ids = vtkIdTypeArray::New();
      this->VisibleCellSelector->GetSelectedIds(ids);
      
      // Set the opacity back to the original value.
      this->EdgeActor->GetProperty()->SetOpacity(opacity);
      
      vtkIdTypeArray* selectedIds = vtkIdTypeArray::New();
      vtkGraph* graph = this->GraphLayout->GetOutput();
      for (vtkIdType i = 0; i < ids->GetNumberOfTuples(); i++)
        {
        vtkIdType edge = ids->GetValue(4*i+3);
        vtkIdType source = graph->GetSourceVertex(edge);
        vtkIdType target = graph->GetTargetVertex(edge);
        selectedIds->InsertNextValue(source);
        selectedIds->InsertNextValue(target);
        if (singleSelectMode)
          {
          break;
          }
        }
      
      selection->Delete();
      selection = vtkSelection::New();
      selection->GetProperties()->Set(vtkSelection::CONTENT_TYPE(), vtkSelection::INDICES);
      selection->GetProperties()->Set(vtkSelection::FIELD_TYPE(), vtkSelection::POINT);
      selection->SetSelectionList(selectedIds);
      ids->Delete();
      selectedIds->Delete();      
      }
#endif
    
    // If this is a union selection, append the selection
    if (rect[4] == vtkInteractorStyleRubberBand2D::SELECT_UNION)
      {
      vtkSelection* oldSelection =
        this->GetRepresentation()->GetSelectionLink()->GetSelection();
      selection->Union(oldSelection);
      }
    
    // Call select on the representation(s)
    this->GetRepresentation()->Select(this, selection);
    }
  else
    {
    Superclass::ProcessEvents(caller, eventId, callData);
    }
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::PrepareForRendering()
{
  // Make sure we have a representation.
  vtkDataRepresentation* rep = this->GetRepresentation();
  if (!rep)
    {
    return;
    }
  
  // Make sure the input connection is up to date.
  vtkAlgorithmOutput* conn = rep->GetInputConnection();
  if (this->GraphLayout->GetInputConnection(0, 0) != conn)
    {
      this->RemoveInputConnection( 0, 0,
      this->GraphLayout->GetInputConnection(0, 0),
      this->ExtractSelectedGraph->GetInputConnection(1, 0));
    this->AddInputConnection(0, 0, conn, rep->GetSelectionConnection());
    }
  
  this->Superclass::PrepareForRendering();
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::ApplyViewTheme(vtkViewTheme* theme)
{

  // Take some parameters from the theme and apply
  // to objects within this class
  this->Renderer->SetBackground(theme->GetBackgroundColor());
  this->Renderer->SetBackground2(theme->GetBackgroundColor2());
  this->Renderer->SetGradientBackground(true);
  
  this->VertexLabelMapper->GetLabelTextProperty()->
    SetColor(theme->GetVertexLabelColor());
  this->EdgeLabelMapper->GetLabelTextProperty()->
    SetColor(theme->GetEdgeLabelColor());
    
  // Pass theme to the graph mapper
  this->GraphMapper->ApplyViewTheme(theme);
  
  // Set vertex size and edge size on mapper
  this->GraphMapper->SetVertexPointSize(theme->GetPointSize());
  this->GraphMapper->SetEdgeLineWidth(theme->GetLineWidth());
  
  
  // Pull selection info from theme, create a new theme, 
  // and pass to the selection graph mapper
  vtkViewTheme *selectTheme = vtkViewTheme::New();
  selectTheme->SetPointColor(theme->GetSelectedPointColor());
  selectTheme->SetCellColor(theme->GetSelectedCellColor());
  selectTheme->SetOutlineColor(theme->GetSelectedPointColor());
  this->SelectedGraphMapper->ApplyViewTheme(selectTheme);
  
  // Set vertex size and edge size on mapper
  this->SelectedGraphMapper->SetVertexPointSize(theme->GetPointSize()+2);
  this->SelectedGraphMapper->SetEdgeLineWidth(theme->GetLineWidth()+1);
  
  selectTheme->Delete();
}

void vtkGraphLayoutView::SetVertexLabelFontSize(const int size)
{
  this->VertexLabelMapper->GetLabelTextProperty()->SetFontSize(size);
}

int vtkGraphLayoutView::GetVertexLabelFontSize()
{
  return this->VertexLabelMapper->GetLabelTextProperty()->GetFontSize();
}

void vtkGraphLayoutView::SetEdgeLabelFontSize(const int size)
{
  this->EdgeLabelMapper->GetLabelTextProperty()->SetFontSize(size);
}

int vtkGraphLayoutView::GetEdgeLabelFontSize()
{
  return this->EdgeLabelMapper->GetLabelTextProperty()->GetFontSize();
}

//----------------------------------------------------------------------------
void vtkGraphLayoutView::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Coordinate: " << endl;
  this->Coordinate->PrintSelf(os, indent.GetNextIndent());
  os << indent << "GraphLayout: " << endl;
  this->GraphLayout->PrintSelf(os, indent.GetNextIndent());
  os << indent << "RandomStrategy: " << endl;
  this->RandomStrategy->PrintSelf(os, indent.GetNextIndent());
  os << indent << "Simple2DStrategy: " << endl;
  this->Simple2DStrategy->PrintSelf(os, indent.GetNextIndent());
  os << indent << "Clustering2DStrategy: " << endl;
  this->Clustering2DStrategy->PrintSelf(os, indent.GetNextIndent());
  os << indent << "Community2DStrategy: " << endl;
  this->Community2DStrategy->PrintSelf(os, indent.GetNextIndent());
  os << indent << "Constrained2DStrategy: " << endl;
  this->Community2DStrategy->PrintSelf(os, indent.GetNextIndent());
  os << indent << "Fast2DStrategy: " << endl;
  this->Fast2DStrategy->PrintSelf(os, indent.GetNextIndent());
  os << indent << "ForceDirectedStrategy: " << endl;
  this->ForceDirectedStrategy->PrintSelf(os, indent.GetNextIndent());
  os << indent << "PassThroughStrategy: " << endl;
  this->PassThroughStrategy->PrintSelf(os, indent.GetNextIndent());
  os << indent << "CircularStrategy: " << endl;
  this->CircularStrategy->PrintSelf(os, indent.GetNextIndent());
  os << indent << "GraphLayout: " << endl;
  this->GraphLayout->PrintSelf(os, indent.GetNextIndent());
  os << indent << "VertexDegree: " << endl;
  this->VertexDegree->PrintSelf(os, indent.GetNextIndent());
  os << indent << "GraphMapper: " << endl;
  this->GraphMapper->PrintSelf(os, indent.GetNextIndent());
  os << indent << "SelectedGraphMapper: " << endl;
  this->SelectedGraphMapper->PrintSelf(os, indent.GetNextIndent());
  os << indent << "VertexLabelMapper: " << endl;
  this->VertexLabelMapper->PrintSelf(os, indent.GetNextIndent());
  os << indent << "EdgeLabelMapper: " << endl;
  this->EdgeLabelMapper->PrintSelf(os, indent.GetNextIndent());
  os << indent << "KdTreeSelector: " << endl;
  this->KdTreeSelector->PrintSelf(os, indent.GetNextIndent());
  os << indent << "VisibleCellSelector: " << endl;
  this->VisibleCellSelector->PrintSelf(os, indent.GetNextIndent());
  os << indent << "ExtractSelectedGraph: " << endl;
  this->ExtractSelectedGraph->PrintSelf(os, indent.GetNextIndent());
  os << indent << "LayoutStrategyName: "
     << (this->LayoutStrategyNameInternal ? this->LayoutStrategyNameInternal : "(null)") << endl;
  os << indent << "LayoutStrategy: " 
     << (this->LayoutStrategy ? "" : "(none)") << endl;
  if (this->LayoutStrategy)
    {
    this->LayoutStrategy->PrintSelf(os, indent.GetNextIndent());   
    }
  os << indent << "EdgeLayoutStrategy: " 
     << (this->EdgeLayoutStrategy ? "" : "(none)") << endl;
  if (this->EdgeLayoutStrategy)
    {
    this->EdgeLayoutStrategy->PrintSelf(os, indent.GetNextIndent());   
    }
  if (this->GetRepresentation())
    {
    os << indent << "VertexLabelActor: " << endl;
    this->VertexLabelActor->PrintSelf(os, indent.GetNextIndent());
    os << indent << "EdgeLabelActor: " << endl;
    this->EdgeLabelActor->PrintSelf(os, indent.GetNextIndent());
    }
}

