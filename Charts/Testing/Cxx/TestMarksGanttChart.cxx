#include "vtkBarMark.h"
#include "vtkContextScene.h"
#include "vtkContextView.h"
#include "vtkDoubleArray.h"
#include "vtkLineMark.h"
#include "vtkMath.h"
#include "vtkPanelMark.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkTable.h"

namespace
{
  vtkDataElement DataFunction(vtkMark* vtkNotUsed(m), vtkDataElement& d)
  {
    return d;
  }

  double LeftFunction(vtkMark* vtkNotUsed(m), vtkDataElement& d)
  {
    return d.GetValue(0).ToDouble() * 100.0;
  }

  double HeightFunction(vtkMark* vtkNotUsed(m), vtkDataElement& vtkNotUsed(d))
  {
    return 30.0;
  }

  double WidthFunction(vtkMark* vtkNotUsed(m), vtkDataElement& d)
  {
    return (d.GetValue(1).ToDouble() - d.GetValue(0).ToDouble()) * 50.0;
  }

  double BottomFunction(vtkMark* m, vtkDataElement& vtkNotUsed(d))
  {
    return (m->GetIndex() * 50.0);
  }
}


int TestMarksGanttChart(int, char*[])
{
  // Set up a 2D context view, context test object and add it to the scene
  vtkSmartPointer<vtkContextView> view = vtkSmartPointer<vtkContextView>::New();
  view->GetRenderer()->SetBackground(1.0, 1.0, 1.0);
  view->GetRenderWindow()->SetSize(800, 600);

  vtkSmartPointer<vtkTable> t = vtkSmartPointer<vtkTable>::New();
  vtkSmartPointer<vtkDoubleArray> startTime = vtkSmartPointer<vtkDoubleArray>::New();
  vtkSmartPointer<vtkDoubleArray> compTime = vtkSmartPointer<vtkDoubleArray>::New();
  startTime->SetName("StartTime");
  compTime->SetName("CompTime");

  vtkMath::RandomSeed(time(NULL));

  for (vtkIdType i = 0; i < 4; ++i)
    {
    startTime->InsertNextValue(i);
    }

  for (vtkIdType i = 0; i < 4; ++i)
    {
    compTime->InsertNextValue(i + vtkMath::Random(1.0, 5.0));
    }

  t->AddColumn(startTime);
  t->AddColumn(compTime);

  vtkDataElement data(t);

  // This would set each row as the children.
  data.SetDimension(0);

  vtkSmartPointer<vtkPanelMark> panel = vtkSmartPointer<vtkPanelMark>::New();
  view->GetScene()->AddItem(panel);

  // Set the data on the vtkMark instead of vtkPanelMark.
  vtkMark* bar = panel->Add(vtkMark::BAR);
  bar->SetData(data);
  bar->SetLeft(LeftFunction);
  bar->SetBottom(BottomFunction);
  bar->SetWidth(WidthFunction);
  bar->SetHeight(HeightFunction);

  view->GetInteractor()->Initialize();
  view->GetInteractor()->Start();
  return 0;
}
