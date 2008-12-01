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

// .NAME Test of vtkLabelPlacer
// .SECTION Description
// this program tests vtkLabelPlacer which uses a sophisticated algorithm to
// prune labels/icons preventing them from overlapping.

#include "vtkActor.h"
#include "vtkActor2D.h"
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkLabeledDataMapper.h"
#include "vtkLabelHierarchy.h"
#include "vtkLabelPlacer.h"
#include "vtkLabelSizeCalculator.h"
#include "vtkMath.h"
#include "vtkPointSetToLabelHierarchy.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPointSet.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkProperty.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkStringArray.h"
#include "vtkTextProperty.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkPolyData.h"
#include "vtkImageData.h"
#include "vtkSmartPointer.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"
#include "vtkRectilinearGrid.h"


#include <vtkTestUtilities.h>
#include <vtkRegressionTestImage.h>

int TestLabelPlacerCoincidentPoints(int argc, char *argv[])
{
  int maxLevels = 5;
  int targetLabels = 32;
  double labelRatio = 0.05;
  int i = 0;
  //char* fname = vtkTestUtilities::ExpandDataFileName(argc, argv, "Data/uniform-001371-5x5x5.vtp");
  int iteratorType = vtkLabelHierarchy::FULL_SORT;
  bool showBounds = false;

  vtkSmartPointer<vtkLabelSizeCalculator> labelSizeCalculator = 
    vtkSmartPointer<vtkLabelSizeCalculator>::New();
  vtkSmartPointer<vtkLabelHierarchy> labelHierarchy = 
    vtkSmartPointer<vtkLabelHierarchy>::New();
  vtkSmartPointer<vtkLabelPlacer> labelPlacer = 
    vtkSmartPointer<vtkLabelPlacer>::New();
  vtkSmartPointer<vtkPointSetToLabelHierarchy> pointSetToLabelHierarchy = 
    vtkSmartPointer<vtkPointSetToLabelHierarchy>::New();
  vtkSmartPointer<vtkXMLPolyDataReader> xmlPolyDataReader =
    vtkSmartPointer<vtkXMLPolyDataReader>::New();

  vtkSmartPointer<vtkPolyDataMapper> polyDataMapper = 
    vtkSmartPointer<vtkPolyDataMapper>::New();
  vtkSmartPointer<vtkActor> actor = 
    vtkSmartPointer<vtkActor>::New();
  vtkSmartPointer<vtkRenderer> renderer = 
    vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindow> renWin = 
    vtkSmartPointer<vtkRenderWindow>::New();
  vtkSmartPointer<vtkRenderWindowInteractor> iren = 
    vtkSmartPointer<vtkRenderWindowInteractor>::New();

  vtkSmartPointer<vtkLabeledDataMapper> labeledMapper = 
    vtkSmartPointer<vtkLabeledDataMapper>::New();
  vtkSmartPointer<vtkActor2D> textActor = vtkSmartPointer<vtkActor2D>::New();

  vtkSmartPointer<vtkPoints> points = vtkPoints::New();

  points->InsertNextPoint(5.0, 5.0, 5.0);
  points->InsertNextPoint(5.0, 5.0, 5.0);
  points->InsertNextPoint(5.0, 5.0, 5.0);
  points->InsertNextPoint(5.0, 5.0, 5.0);
  points->InsertNextPoint(5.0, 5.0, 5.0);
  points->InsertNextPoint(5.0, 5.0, 5.0);
  points->InsertNextPoint(5.0, 5.0, 5.0);
  points->InsertNextPoint(5.0, 5.0, 5.0);
  points->InsertNextPoint(5.0, 5.0, 5.0);
  points->InsertNextPoint(5.0, 5.0, 5.0);

  vtkSmartPointer<vtkCellArray> cells = vtkCellArray::New();

  cells->InsertNextCell(10);
  for(i = 0; i < 10; i++)
    {
    cells->InsertCellPoint(i);
    }

  vtkSmartPointer<vtkPolyData> polyData = vtkPolyData::New();
  polyData->SetPoints(points);
  polyData->SetVerts(cells);

  vtkSmartPointer<vtkStringArray> stringData = vtkStringArray::New();
  stringData->SetNumberOfComponents(1);
  stringData->SetNumberOfTuples(10);
  stringData->SetName("PlaceNames");
  stringData->InsertNextValue("Amsterdam");
  stringData->InsertNextValue("Berlin");
  stringData->InsertNextValue("Cairo");
  stringData->InsertNextValue("New Dehli");
  stringData->InsertNextValue("Hanoi");
  stringData->InsertNextValue("Jakarta");
  stringData->InsertNextValue("London");
  stringData->InsertNextValue("Ottawa");
  stringData->InsertNextValue("Tokyo");
  stringData->InsertNextValue("Washington");

  polyData->GetPointData()->AddArray(stringData);

  vtkMath::RandomSeed(1234);
  vtkSmartPointer<vtkFloatArray> priority = vtkFloatArray::New();
  priority->SetNumberOfComponents(1);
  priority->SetNumberOfTuples(10);
  priority->SetName("Priority");

  for(i = 0; i < 10; i++)
    {
    priority->InsertNextValue(vtkMath::Random(0.0, 10.0));
    }

  polyData->GetPointData()->AddArray(priority);

  vtkSmartPointer<vtkFloatArray> labelSize = vtkFloatArray::New();
  labelSize->SetNumberOfComponents(3);
  labelSize->SetNumberOfTuples(10);
  labelSize->SetName("LabelSize");

  for(i = 0; i < 10; i++)
    {
    labelSize->InsertNextTuple3( vtkMath::Random(8.0, 10.0),
                                 vtkMath::Random(12.0, 16.0),
                                 0.0 );
    }

  polyData->GetPointData()->AddArray(labelSize);
  
  labelSizeCalculator->SetInput(polyData);
  labelSizeCalculator->GetFontProperty()->SetFontSize( 12 );
  labelSizeCalculator->GetFontProperty()->SetFontFamily( vtkTextProperty::GetFontFamilyFromString( "Arial" ) );
  labelSizeCalculator->SetInputArrayToProcess( 0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "PlaceNames" );
  labelSizeCalculator->SetLabelSizeArrayName( "LabelSize" );

  pointSetToLabelHierarchy->AddInputConnection(labelSizeCalculator->GetOutputPort());
  pointSetToLabelHierarchy->SetInputArrayToProcess( 0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "Priority" );
  pointSetToLabelHierarchy->SetInputArrayToProcess( 1, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "LabelSize" );
  pointSetToLabelHierarchy->SetInputArrayToProcess( 2, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "PlaceNames" );
  pointSetToLabelHierarchy->SetMaximumDepth( maxLevels );
  pointSetToLabelHierarchy->SetTargetLabelCount( targetLabels );

  labelPlacer->SetInputConnection( pointSetToLabelHierarchy->GetOutputPort() );
  labelPlacer->SetIteratorType( iteratorType );
  labelPlacer->SetOutputTraversedBounds( showBounds );
  labelPlacer->SetRenderer( renderer );
  labelPlacer->SetMaximumLabelFraction( labelRatio );

  polyDataMapper->SetInputConnection(labelPlacer->GetOutputPort());

  actor->SetMapper(polyDataMapper);

  labelPlacer->Update();

  labeledMapper->SetInputConnection(labelPlacer->GetOutputPort());
  labeledMapper->SetLabelTextProperty(labelSizeCalculator->GetFontProperty());
  labeledMapper->SetFieldDataName("LabelText");
  labeledMapper->SetLabelModeToLabelFieldData();
  labeledMapper->GetLabelTextProperty()->SetColor(0.0, 0.8, 0.2);
  textActor->SetMapper(labeledMapper);
  
  renderer->AddActor(actor);
  renderer->AddActor(textActor);

  renWin->SetSize(300, 300);
  renWin->AddRenderer(renderer);
  renderer->SetBackground(0.0, 0.0, 0.0);
    iren->SetRenderWindow(renWin);

  renWin->Render();
  renderer->ResetCamera();
  renderer->ResetCamera();
  renderer->ResetCamera();

  int retVal = vtkRegressionTestImage( renWin );
  if ( retVal == vtkRegressionTester::DO_INTERACTOR)
    {
    iren->Start();
    }

  return !retVal;
}
