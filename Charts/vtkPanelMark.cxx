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

#include "vtkPanelMark.h"

#include "vtkContext2D.h"
#include "vtkObjectFactory.h"
#include "vtkTransform2D.h"

//-----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkPanelMark, "$Revision$");
vtkStandardNewMacro(vtkPanelMark);

//-----------------------------------------------------------------------------
vtkPanelMark::vtkPanelMark()
{
}

//-----------------------------------------------------------------------------
vtkPanelMark::~vtkPanelMark()
{
}

//-----------------------------------------------------------------------------
vtkMark* vtkPanelMark::Add(int type)
{
  vtkSmartPointer<vtkMark> m = vtkSmartPointer<vtkMark>::New();
  m->SetType(type);

  // Set defaults
  switch(type)
    {
    case BAR:
      m->SetFillColor(vtkMarkUtil::DefaultSeriesColor);
      m->SetLineWidth(1);
      m->SetLineColor(vtkColor(0.0, 0.0, 0.0, 1.0));
      break;
    case LINE:
      m->SetLineColor(vtkMarkUtil::DefaultSeriesColor);
      m->SetLineWidth(2);
      break;
    }

  if (this->Marks.size() > 0)
    {
    m->Extend(this->Marks.back());
    }
  this->Marks.push_back(m);
  m->SetParent(this);
  return m;
}

//-----------------------------------------------------------------------------
void vtkPanelMark::Update()
{
  this->MarkInstances.clear();
  this->Left.Update(this);
  this->Right.Update(this);
  this->Top.Update(this);
  this->Bottom.Update(this);
  vtkDataElement data = this->Data.GetData(this);
  vtkIdType numMarks = this->Marks.size();
  vtkIdType numChildren = data.GetNumberOfChildren();
  for (size_t j = 0; j < numMarks; ++j)
    {
    for (vtkIdType i = 0; i < numChildren; ++i)
      {
      this->Index = i;
      this->Marks[j]->DataChanged();
      this->Marks[j]->Update();
      vtkSmartPointer<vtkMark> m = vtkSmartPointer<vtkMark>::New();
      m->Extend(this->Marks[j]);
      m->SetType(this->Marks[j]->GetType());
      m->SetParent(this->Marks[j]->GetParent());
      m->SetParentMarkIndex(j);
      m->SetParentDataIndex(i);
      this->MarkInstances.push_back(m);
      }
    }
}

//-----------------------------------------------------------------------------
bool vtkPanelMark::Paint(vtkContext2D* painter)
{
  //TODO: Be smarter about the update
  this->Update();

  if (!painter->GetTransform())
    {
    vtkSmartPointer<vtkTransform2D> trans = vtkSmartPointer<vtkTransform2D>::New();
    trans->Identity();
    painter->SetTransform(trans);
    }

  double* left = this->Left.GetArray(this);
  double* bottom = this->Bottom.GetArray(this);
  vtkDataElement data = this->Data.GetData(this);
  vtkIdType numMarks = this->Marks.size();
  vtkIdType numChildren = data.GetNumberOfChildren();
  for (size_t j = 0; j < numMarks; ++j)
    {
    for (vtkIdType i = 0; i < numChildren; ++i)
      {
      this->Index = i;
      painter->GetTransform()->Translate(left[i], bottom[i]);
      this->MarkInstances[j*numChildren + i]->Paint(painter);
      painter->GetTransform()->Translate(-left[i], -bottom[i]);
      }
    }
  return true;
}

//-----------------------------------------------------------------------------
void vtkPanelMark::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
