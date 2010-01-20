
#include "vtkBarMark.h"
#include "vtkContextScene.h"
#include "vtkContextView.h"
#include "vtkDoubleArray.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkTable.h"

double LeftFunction(vtkMark& m, vtkDataElement& e, vtkIdType i)
{
  return i*20;
}

double BottomFunction(vtkMark& m, vtkDataElement& e, vtkIdType i)
{
  return 0;
}

double WidthFunction(vtkMark& m, vtkDataElement& e, vtkIdType i)
{
  return 15;
}

double HeightFunction(vtkMark& m, vtkDataElement& e, vtkIdType i)
{
  return e.GetValue().ToDouble()*20;
}

vtkColor LineColorFunction(vtkMark& m, vtkDataElement& e, vtkIdType i)
{
  return vtkColor(0.5, 0.0, 0.0);
}

vtkColor FillColorFunction(vtkMark& m, vtkDataElement& e, vtkIdType i)
{
  return vtkColor(0.2*i, 0.2, 1.0);
}

int TestMarks(int, char*[])
{
  // Set up a 2D context view, context test object and add it to the scene
  vtkSmartPointer<vtkContextView> view = vtkSmartPointer<vtkContextView>::New();
  view->GetRenderer()->SetBackground(1.0, 1.0, 1.0);
  view->GetRenderWindow()->SetSize(800, 600);

  vtkSmartPointer<vtkTable> t = vtkSmartPointer<vtkTable>::New();
  vtkSmartPointer<vtkDoubleArray> indexArr = vtkSmartPointer<vtkDoubleArray>::New();
  indexArr->SetName("Value");
  for (vtkIdType i = 0; i < 5; ++i)
    {
    indexArr->InsertNextValue(sin(static_cast<double>(i)) + 1);
    }
  t->AddColumn(indexArr);

  vtkSmartPointer<vtkBarMark> bar = vtkSmartPointer<vtkBarMark>::New();
  bar->SetData(vtkDataElement(t));
  bar->SetLeft(LeftFunction);
  bar->SetBottom(BottomFunction);
  bar->SetWidth(WidthFunction);
  bar->SetHeight(HeightFunction);
  bar->SetLineColor(LineColorFunction);
  bar->SetFillColor(FillColorFunction);
  view->GetScene()->AddItem(bar);

  view->GetInteractor()->Initialize();
  view->GetInteractor()->Start();
  return 0;
}