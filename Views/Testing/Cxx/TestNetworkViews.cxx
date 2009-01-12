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

#include "vtkNetworkHierarchy.h"
#include "vtkRegressionTestImage.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSQLDatabaseTableSource.h"
#include "vtkTableToGraph.h"
#include "vtkTreeRingView.h"
#include "vtkVertexDegree.h"
#include "vtkViewTheme.h"

#include "vtkSmartPointer.h"
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

using vtkstd::string;

int TestNetworkViews(int argc, char* argv[])
{
  VTK_CREATE(vtkTesting, testHelper);
  testHelper->AddArguments(argc,const_cast<const char **>(argv));
  string dataRoot = testHelper->GetDataRoot();
  string file = dataRoot+"/Data/Infovis/SQLite/ports_protocols.db";
  
  //Pull the table (that represents relationships/edges) from the database
  VTK_CREATE( vtkSQLDatabaseTableSource, databaseToEdgeTable );
  databaseToEdgeTable->SetURL("sqlite://" + file);
  databaseToEdgeTable->SetQuery("select src, dst, dport, protocol, port_protocol from tcp");
  
  //Pull the table (that represents entities/vertices) from the database
  VTK_CREATE( vtkSQLDatabaseTableSource, databaseToVertexTable );
  databaseToVertexTable->SetURL("sqlite://" + file);
  databaseToVertexTable->SetQuery("select ip, hostname from dnsnames");

  //Make a graph
  VTK_CREATE( vtkTableToGraph, graph );
  graph->AddInputConnection(0,databaseToEdgeTable->GetOutputPort());
  graph->AddInputConnection(1,databaseToVertexTable->GetOutputPort());
  graph->AddLinkVertex("src", "ip", false);
  graph->AddLinkVertex("dst", "ip", false);
  graph->AddLinkEdge("src", "dst");

    //Make a tree out of ip addresses
  VTK_CREATE( vtkNetworkHierarchy, ip_tree );
  ip_tree->AddInputConnection(graph->GetOutputPort());

  VTK_CREATE( vtkTreeRingView, dummy );
  
  //Create a view on city/region/country
  VTK_CREATE( vtkTreeRingView, view1 );
  view1->SetTreeFromInputConnection(ip_tree->GetOutputPort());
  view1->SetGraphFromInputConnection(graph->GetOutputPort());
  view1->SetLabelPriorityArrayName("GraphVertexDegree");
  view1->SetAreaColorArrayName("GraphVertexDegree");
  view1->SetAreaLabelArrayName("ip");
  view1->SetAreaHoverArrayName("ip");
  view1->SetAreaLabelVisibility(true);
  view1->SetEdgeColorArrayName("dport");
  view1->SetColorEdges(true);
  view1->SetInteriorLogSpacingValue(2.);
  view1->SetBundlingStrength(.8);

  // Apply a theme to the views
  vtkViewTheme* const theme = vtkViewTheme::CreateMellowTheme();
  view1->ApplyViewTheme(theme);
  theme->Delete();

  VTK_CREATE( vtkRenderWindow, window1 );
  window1->SetMultiSamples(0);
  window1->SetSize(600, 600);
  VTK_CREATE(vtkRenderWindowInteractor, iren);
  iren->SetRenderWindow(window1);
  dummy->SetupRenderWindow(window1);
  view1->SetupRenderWindow(window1);

  window1->Render();

  int retVal = vtkRegressionTestImage(window1);
  if (retVal == vtkRegressionTester::DO_INTERACTOR)
    {
    iren->Initialize();
    iren->Start();
    
    retVal = vtkRegressionTester::PASSED;
    }
  
  return !retVal;
}
