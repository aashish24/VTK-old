
#include "vtkBarMark.h"
#include "vtkContextScene.h"
#include "vtkContextView.h"
#include "vtkDoubleArray.h"
#include "vtkPanelMark.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkTable.h"

double PanelLeftFunction(vtkMark* m, vtkDataElement& e)
{
  return m->GetIndex()*15;
}

double PanelBottomFunction(vtkMark* m, vtkDataElement& e)
{
  return 0;
}

double LeftFunction(vtkMark* m, vtkDataElement& e)
{
  return m->GetIndex()*50;
}

double BottomFunction(vtkMark* m, vtkDataElement& e)
{
  return 0;
}

double WidthFunction(vtkMark* m, vtkDataElement& e)
{
  return 15;
}

double HeightFunction(vtkMark* m, vtkDataElement& e)
{
  return e.GetValue().ToDouble()*200;
}

vtkColor LineColorFunction(vtkMark* m, vtkDataElement& e)
{
  return vtkColor(0.0, 0.0, 0.0);
}

vtkColor FillColorFunction(vtkMark* m, vtkDataElement& e)
{
  unsigned char colors[10][3] = {{166, 206, 227}, {31, 120, 180}, {178, 223, 13}, {51, 160, 44}, {251, 154, 153}, {227, 26, 28}, {253, 191, 111}, {255, 127, 0}, {202, 178, 214}, {106, 61, 154}};
  vtkIdType index = m->GetParent()->GetIndex() % 10;
  return vtkColor(colors[index][0]/255.0, colors[index][1]/255.0, colors[index][2]/255.0);
}

int TestMarks(int, char*[])
{
  // Set up a 2D context view, context test object and add it to the scene
  vtkSmartPointer<vtkContextView> view = vtkSmartPointer<vtkContextView>::New();
  view->GetRenderer()->SetBackground(1.0, 1.0, 1.0);
  view->GetRenderWindow()->SetSize(800, 600);

  vtkSmartPointer<vtkTable> t = vtkSmartPointer<vtkTable>::New();
  vtkSmartPointer<vtkDoubleArray> arr1 = vtkSmartPointer<vtkDoubleArray>::New();
  arr1->SetName("Array1");
  vtkSmartPointer<vtkDoubleArray> arr2 = vtkSmartPointer<vtkDoubleArray>::New();
  arr2->SetName("Array2");
  vtkSmartPointer<vtkDoubleArray> arr3 = vtkSmartPointer<vtkDoubleArray>::New();
  arr3->SetName("Array3");
  for (vtkIdType i = 0; i < 20; ++i)
    {
    arr1->InsertNextValue(sin(i/5.0) + 1);
    arr2->InsertNextValue(cos(i/5.0) + 1);
    arr3->InsertNextValue(i/20.0);
    }
  t->AddColumn(arr1);
  t->AddColumn(arr2);
  t->AddColumn(arr3);

  vtkDataElement data(t);
  data.SetDimension(1);

  vtkSmartPointer<vtkPanelMark> panel = vtkSmartPointer<vtkPanelMark>::New();
  panel->SetData(data);
  panel->SetLeft(PanelLeftFunction);
  panel->SetBottom(PanelBottomFunction);
  view->GetScene()->AddItem(panel);

  vtkSmartPointer<vtkBarMark> bar = vtkSmartPointer<vtkBarMark>::New();
  //bar->SetData(vtkDataElement(t));
  bar->SetLeft(LeftFunction);
  bar->SetBottom(BottomFunction);
  bar->SetWidth(WidthFunction);
  bar->SetHeight(HeightFunction);
  bar->SetLineColor(LineColorFunction);
  bar->SetFillColor(FillColorFunction);
  panel->Add(bar);

  view->GetInteractor()->Initialize();
  view->GetInteractor()->Start();
  return 0;
}
