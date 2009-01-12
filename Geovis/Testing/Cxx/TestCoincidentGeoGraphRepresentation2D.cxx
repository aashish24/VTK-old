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
// .SECTION Description

#include "vtkCamera.h"
#include "vtkDataObject.h"
#include "vtkDataSetAttributes.h"
#include "vtkDoubleArray.h"
#include "vtkGeoAlignedImageSource.h"
#include "vtkGeoAlignedImageRepresentation.h"
#include "vtkGeoFileImageSource.h"
#include "vtkGeoFileTerrainSource.h"
#include "vtkGeoGraphRepresentation2D.h"
#include "vtkGeoProjection.h"
#include "vtkGeoProjectionSource.h"
#include "vtkGeoTransform.h"
#include "vtkGeoTerrain2D.h"
#include "vtkGeoTerrainNode.h"
#include "vtkGeoView2D.h"
#include "vtkGraphLayoutView.h"
#include "vtkJPEGReader.h"
#include "vtkMutableUndirectedGraph.h"
#include "vtkPolyData.h"
#include "vtkRegressionTestImage.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSmartPointer.h"
#include "vtkStdString.h"
#include "vtkTestUtilities.h"
#include "vtkTIFFReader.h"

#include <vtksys/ios/sstream>

int TestCoincidentGeoGraphRepresentation2D(int argc, char* argv[])
{
  int projNum = 44;
  char* fname = vtkTestUtilities::ExpandDataFileName(
    argc, argv, "Data/NE2_ps_bath_small.jpg");
  vtkStdString imageFile = fname;
  vtkStdString imageReadPath = ".";
  vtkStdString imageSavePath = ".";
  vtkStdString terrainReadPath = ".";
  vtkStdString terrainSavePath = ".";
  double locationTol = 5.0;
  double textureTol = 1.0;
  for (int a = 1; a < argc; a++)
    {
    if (!strcmp(argv[a], "-P"))
      {
      ++a;
      projNum = atoi(argv[a]);
      continue;
      }
    if (!strcmp(argv[a], "-IF"))
      {
      ++a;
      imageFile = argv[a];
      continue;
      }
    if (!strcmp(argv[a], "-IR"))
      {
      ++a;
      imageReadPath = argv[a];
      continue;
      }
    if (!strcmp(argv[a], "-IS"))
      {
      ++a;
      imageSavePath = argv[a];
      continue;
      }
    if (!strcmp(argv[a], "-TR"))
      {
      ++a;
      terrainReadPath = argv[a];
      continue;
      }
    if (!strcmp(argv[a], "-TS"))
      {
      ++a;
      terrainSavePath = argv[a];
      continue;
      }
    if (!strcmp(argv[a], "-LT"))
      {
      ++a;
      locationTol = atof(argv[a]);
      continue;
      }
    if (!strcmp(argv[a], "-TT"))
      {
      ++a;
      textureTol = atof(argv[a]);
      continue;
      }
    if (!strcmp(argv[a], "-I"))
      {
      continue;
      }
    if (!strcmp(argv[a], "-D") ||
        !strcmp(argv[a], "-T") ||
        !strcmp(argv[a], "-V"))
      {
      ++a;
      continue;
      }
    cerr
      << "\nUsage:\n"
      << "  -P  proj - Projection ID (default 40)\n"
      << "  -IF file - Image file\n"
      << "  -IR path - Image database read path\n"
      << "  -IS path - Image database save path\n"
      << "  -TR file - Terrain databse read path\n"
      << "  -TS file - Terrain databse save path\n"
      << "  -LT tol  - Set geometry tolerance in pixels (default 5.0)\n"
      << "  -TT tol  - Set texture tolerance in pixels (default 1.0)\n";
    return 0;
    }

  // Create the view
  vtkSmartPointer<vtkRenderWindow> win = vtkSmartPointer<vtkRenderWindow>::New();
  vtkSmartPointer<vtkGeoView2D> view = vtkSmartPointer<vtkGeoView2D>::New();
  view->SetupRenderWindow(win);

  // Create the terrain
  vtkSmartPointer<vtkGeoTerrain2D> terrain =
    vtkSmartPointer<vtkGeoTerrain2D>::New();
  vtkSmartPointer<vtkGeoSource> terrainSource;
  vtkGeoProjectionSource* projSource = vtkGeoProjectionSource::New();
  projSource->SetProjection(projNum);
  vtkSmartPointer<vtkGeoTransform> transform = 
    vtkSmartPointer<vtkGeoTransform>::New();
  vtkSmartPointer<vtkGeoProjection> proj =
    vtkSmartPointer<vtkGeoProjection>::New();
  proj->SetName(vtkGeoProjection::GetProjectionName(projNum));
  transform->SetDestinationProjection(proj);
  terrainSource.TakeReference(projSource);
  terrain->SetSource(terrainSource);
  view->SetSurface(terrain);

  // Create background image
  vtkSmartPointer<vtkGeoAlignedImageRepresentation> imageRep =
    vtkSmartPointer<vtkGeoAlignedImageRepresentation>::New();
  vtkSmartPointer<vtkGeoSource> imageSource;
  vtkGeoAlignedImageSource* alignedSource = vtkGeoAlignedImageSource::New();
  vtkSmartPointer<vtkJPEGReader> reader =
    vtkSmartPointer<vtkJPEGReader>::New();
  reader->SetFileName(imageFile.c_str());
  reader->Update();
  alignedSource->SetImage(reader->GetOutput());
  imageSource.TakeReference(alignedSource);
  imageRep->SetSource(imageSource);
  view->AddRepresentation(imageRep);

  // Create second image
  /*char* fname2 = vtkTestUtilities::ExpandDataFileName(
      argc, argv, "Data/masonry-wide.jpg");
  vtkSmartPointer<vtkJPEGReader> reader2 =
    vtkSmartPointer<vtkJPEGReader>::New();
  reader2->SetFileName(fname2);
  reader2->Update();
  vtkSmartPointer<vtkGeoAlignedImageSource> imageSource2 =
    vtkSmartPointer<vtkGeoAlignedImageSource>::New();
  imageSource2->SetImage(reader2->GetOutput());
  vtkSmartPointer<vtkGeoAlignedImageRepresentation> imageRep2 =
    vtkSmartPointer<vtkGeoAlignedImageRepresentation>::New();
  imageRep2->SetSource(imageSource2);
  view->AddRepresentation(imageRep2);*/

  // Add a graph representation
  vtkSmartPointer<vtkMutableUndirectedGraph> graph = 
    vtkSmartPointer<vtkMutableUndirectedGraph>::New();
  vtkSmartPointer<vtkDoubleArray> latArr = 
    vtkSmartPointer<vtkDoubleArray>::New();
  vtkSmartPointer<vtkDoubleArray> lonArr = 
    vtkSmartPointer<vtkDoubleArray>::New();
  latArr->SetNumberOfTuples(128);
  lonArr->SetNumberOfTuples(128);
  latArr->SetName("latitude");
  lonArr->SetName("longitude");
  vtkIdType v;

  for (v = 0; v < 20; ++v)
    {
    latArr->SetValue(v, 0.0);
    lonArr->SetValue(v, 0.0);
    graph->AddVertex();
    }

  for (v = 20; v < 40; ++v)
    {
    latArr->SetValue(v, 42);
    lonArr->SetValue(v, -73);
    graph->AddVertex();
    }

  for (v = 40; v < 49; ++v)
    {
    latArr->SetValue(v, 35);
    lonArr->SetValue(v, -106);
    graph->AddVertex();
    }

  for (v = 49; v < 66; ++v)
    {
    latArr->SetValue(v, 39);
    lonArr->SetValue(v, 116);
    graph->AddVertex();
    }

   for (v = 66; v < 80; ++v)
    {
    latArr->SetValue(v, -31);
    lonArr->SetValue(v, 115);
    graph->AddVertex();
    }

  for (v = 80; v < 105; ++v)
    {
    latArr->SetValue(v, 48.87);
    lonArr->SetValue(v, 2.29);
    graph->AddVertex();
    }

  for (v = 105; v < 122; ++v)
    {
    latArr->SetValue(v, -34.44);
    lonArr->SetValue(v, -59.20);
    graph->AddVertex();
    }

  // SANTAREM
  latArr->SetValue(122, -2.26);
  lonArr->SetValue(122, -54.41);
  graph->AddVertex();

  // CAIRO
  latArr->SetValue(123, 30.03);
  lonArr->SetValue(123, 31.15);
  graph->AddVertex();

  // TEHRAN
  latArr->SetValue(124, 35.40);
  lonArr->SetValue(124, 51.26);
  graph->AddVertex();

  // MOSCOW
  latArr->SetValue(125, 55.45);
  lonArr->SetValue(125, 37.42);
  graph->AddVertex();

  // CALCUTTA
  latArr->SetValue(126, 22.30);
  lonArr->SetValue(126, 88.20);
  graph->AddVertex();

  // JAKARTA
  latArr->SetValue(127, -6.08);
  lonArr->SetValue(127, 106.45);
  graph->AddVertex();

  graph->GetVertexData()->AddArray(latArr);
  graph->GetVertexData()->AddArray(lonArr);

  for (v = 1; v < 20; ++v)
    {
    graph->AddEdge(0, v);
    }

  for (v = 21; v < 40; ++v)
    {
    graph->AddEdge(v, v - 1);
    }

  for (v = 41; v < 49; ++v)
    {
    graph->AddEdge(v, v - 1);
    }

  for (v = 50; v < 66; ++v)
    {
    graph->AddEdge(v, v - 1);
    }

  for (v = 67; v < 80; ++v)
    {
    graph->AddEdge(v, v - 1);
    }

  for (v = 81; v < 105; ++v)
    {
    graph->AddEdge(80, v);
    }

  for (v = 106; v < 122; ++v)
    {
    graph->AddEdge(105, v);
    }

  graph->AddEdge(122, 123);
  graph->AddEdge(122, 20);
  graph->AddEdge(20, 40);
  graph->AddEdge(122, 105);
  graph->AddEdge(123, 124);
  graph->AddEdge(123, 0);
  graph->AddEdge(124, 125);
  graph->AddEdge(125, 80);
  graph->AddEdge(124, 126);
  graph->AddEdge(126, 49);
  graph->AddEdge(126, 127);
  graph->AddEdge(127, 66);

  vtkSmartPointer<vtkGeoGraphRepresentation2D> graphRep =
    vtkSmartPointer<vtkGeoGraphRepresentation2D>::New();
  graphRep->SetTransform(transform);
  graphRep->SetInput(graph);
  view->AddRepresentation(graphRep);

  // Serialize databases
  if (imageSavePath.length() > 0)
    {
    imageRep->SaveDatabase(imageSavePath);
    }
  if (terrainSavePath.length() > 0)
    {
    terrain->SaveDatabase(terrainSavePath, 4);
    }

  // Reload databases
  if (terrainReadPath.length() > 0)
    {
    terrainSource->ShutDown();
    vtkGeoFileTerrainSource* source = vtkGeoFileTerrainSource::New();
    source->SetPath(terrainReadPath.c_str());
    terrainSource.TakeReference(source);
    }
  terrain->SetSource(terrainSource);
  if (imageReadPath.length() > 0)
    {
    imageSource->ShutDown();
    vtkGeoFileImageSource* source = vtkGeoFileImageSource::New();
    source->SetPath(imageReadPath.c_str());
    imageSource.TakeReference(source);
    }
  imageRep->SetSource(imageSource);

  // Set up the viewport
  win->SetSize(900, 600);
  vtkSmartPointer<vtkGeoTerrainNode> root =
    vtkSmartPointer<vtkGeoTerrainNode>::New();
  terrainSource->FetchRoot(root);
  double bounds[6];
  root->GetModel()->GetBounds(bounds);
  bounds[0] = bounds[0] - (bounds[1] - bounds[0])*0.01;
  bounds[1] = bounds[1] + (bounds[1] - bounds[0])*0.01;
  bounds[2] = bounds[2] - (bounds[3] - bounds[2])*0.01;
  bounds[3] = bounds[3] + (bounds[3] - bounds[2])*0.01;
  double scalex = (bounds[1] - bounds[0])/2.0;
  double scaley = (bounds[3] - bounds[2])/2.0;
  double scale = (scalex > scaley) ? scalex : scaley;
  view->GetRenderer()->GetActiveCamera()->SetParallelScale(scale);

  view->Update();
  view->GetRenderer()->ResetCamera();
  view->GetRenderer()->GetActiveCamera()->Zoom(2.1);
  int retVal = vtkRegressionTestImage(win);
  if (retVal == vtkRegressionTester::DO_INTERACTOR)
    {
    win->GetInteractor()->Initialize();
    win->GetInteractor()->Start();
    }

  terrainSource->ShutDown();
  imageSource->ShutDown();
  //imageSource2->ShutDown();

  delete [] fname;
  //delete [] fname2;
  return !retVal;
}

