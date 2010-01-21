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
void vtkPanelMark::Add(vtkMark* m)
{
  this->Marks.push_back(vtkSmartPointer<vtkMark>(m));
  m->SetParent(this);
}

//-----------------------------------------------------------------------------
bool vtkPanelMark::Paint(vtkContext2D* painter)
{
  double* left = this->Left.GetArray(this);
  double* right = this->Right.GetArray(this);
  double* top = this->Top.GetArray(this);
  double* bottom = this->Bottom.GetArray(this);
  vtkDataElement data = this->Data.GetConstant(this);
  vtkIdType numChildren = data.GetNumberOfChildren();
  if (!painter->GetTransform())
    {
    vtkSmartPointer<vtkTransform2D> trans = vtkSmartPointer<vtkTransform2D>::New();
    trans->Identity();
    painter->SetTransform(trans);
    }
  for (vtkIdType i = 0; i < numChildren; ++i)
    {
    this->Index = i;
    painter->GetTransform()->Translate(left[i], bottom[i]);
    vtkDataElement childData = data.GetChild(i);
    for (size_t j = 0; j < this->Marks.size(); ++j)
      {
      this->Marks[j]->SetData(childData);
      this->Marks[j]->Paint(painter);
      }
    painter->GetTransform()->Translate(-left[i], -bottom[i]);
    }
  return true;
}

//-----------------------------------------------------------------------------
void vtkPanelMark::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
