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

#include "vtkMark.h"

#include "vtkAbstractArray.h"
#include "vtkBrush.h"
#include "vtkContext2D.h"
#include "vtkObjectFactory.h"
#include "vtkPanelMark.h"
#include "vtkPen.h"
#include "vtkTable.h"

#include "vtkBarMark.h"
#include "vtkLineMark.h"

//-----------------------------------------------------------------------------
vtkIdType vtkDataElement::GetNumberOfChildren()
{
  switch (this->Type)
    {
    case TABLE:
      if (this->Dimension == 0)
        {
        return this->Table->GetNumberOfRows();
        }
      else
        {
        return this->Table->GetNumberOfColumns();
        }
    case TABLE_ROW:
      return this->Table->GetNumberOfColumns();
    case ABSTRACT_ARRAY:
      if (this->Dimension == 0)
        {
        return this->AbstractArray->GetNumberOfTuples();
        }
      else
        {
        return this->AbstractArray->GetNumberOfComponents();
        }
    case ABSTRACT_ARRAY_TUPLE:
      return this->AbstractArray->GetNumberOfComponents();
    case ABSTRACT_ARRAY_COMPONENT:
      return this->AbstractArray->GetNumberOfTuples();
    }
  return 0;
}

//-----------------------------------------------------------------------------
vtkDataElement vtkDataElement::GetChild(vtkIdType i)
{
  switch (this->Type)
    {
    case TABLE:
      if (this->Dimension == 0)
        {
        return vtkDataElement(this->Table, i);
        }
      else
        {
        return vtkDataElement(this->Table->GetColumn(i));
        }
    case TABLE_ROW:
      return vtkDataElement(this->Table->GetValue(this->Index, i));
    case ABSTRACT_ARRAY:
      if (this->Dimension == 0)
        {
        return vtkDataElement(this->AbstractArray, i, ABSTRACT_ARRAY_TUPLE);
        }
      else
        {
        return vtkDataElement(this->AbstractArray, i, ABSTRACT_ARRAY_COMPONENT);
        }
    case ABSTRACT_ARRAY_TUPLE:
      return vtkDataElement(this->AbstractArray->GetVariantValue(this->Index*this->AbstractArray->GetNumberOfComponents() + i));
    case ABSTRACT_ARRAY_COMPONENT:
      return vtkDataElement(this->AbstractArray->GetVariantValue(i*this->AbstractArray->GetNumberOfComponents() + this->Index));
    }
  return vtkDataElement();
}

//-----------------------------------------------------------------------------
vtkVariant vtkDataElement::GetValue(vtkIdType i)
{
  switch (this->Type)
    {
    case TABLE:
      if (this->Dimension == 0)
        {
        return this->Table->GetValue(i, 0);
        }
      else
        {
        return this->Table->GetValue(0, i);
        }
    case TABLE_ROW:
      return this->Table->GetValue(this->Index, i);
    case ABSTRACT_ARRAY:
      if (this->Dimension == 0)
        {
        return this->AbstractArray->GetVariantValue(i*this->AbstractArray->GetNumberOfComponents());
        }
      else
        {
        return this->AbstractArray->GetVariantValue(i);
        }
    case ABSTRACT_ARRAY_TUPLE:
      return this->AbstractArray->GetVariantValue(this->Index*this->AbstractArray->GetNumberOfComponents() + i);
    case ABSTRACT_ARRAY_COMPONENT:
      return this->AbstractArray->GetVariantValue(i*this->AbstractArray->GetNumberOfComponents() + this->Index);
    case SCALAR:
      return this->Scalar;
    }
  return vtkVariant();
}

//-----------------------------------------------------------------------------
vtkVariant vtkDataElement::GetValue(std::string str)
{
  switch (this->Type)
    {
    case TABLE_ROW:
      return this->Table->GetValueByName(this->Index, str.c_str());
    }
  return vtkVariant();
}

//-----------------------------------------------------------------------------
bool vtkDataElement::IsValid()
{
  return this->Valid;
}

//-----------------------------------------------------------------------------
vtkDataElement vtkDataValue::GetData(vtkMark* m)
{
  if (this->Function)
    {
    vtkMark* p = m->GetParent();
    vtkDataElement d = p->GetData().GetData(p).GetChild(p->GetIndex());
    return this->Function(m, d);
    }
  return this->Constant;
}

//-----------------------------------------------------------------------------
vtkColor vtkMarkUtil::DefaultSeriesColor(vtkMark* m, vtkDataElement&)
{
  unsigned char colors[10][3] = {{166, 206, 227}, {31, 120, 180}, {178, 223, 13}, {51, 160, 44}, {251, 154, 153}, {227, 26, 28}, {253, 191, 111}, {255, 127, 0}, {202, 178, 214}, {106, 61, 154}};
  vtkIdType index = 0;
  if (m->GetParent())
    {
    index = m->GetParent()->GetIndex() % 10;
    }
  return vtkColor(colors[index][0]/255.0, colors[index][1]/255.0, colors[index][2]/255.0);
}

//-----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkMark, "$Revision$");
vtkStandardNewMacro(vtkMark);

//-----------------------------------------------------------------------------
vtkMark::vtkMark()
{
  this->Parent = NULL;
  this->Index = 0;
  this->ParentMarkIndex = 0;
  this->ParentDataIndex = 0;
}

//-----------------------------------------------------------------------------
vtkMark::~vtkMark()
{
}

//-----------------------------------------------------------------------------
vtkMark* vtkMark::CreateMark(int type)
{
  vtkMark* m = NULL;
  switch (type)
    {
    case BAR:
      m = vtkBarMark::New();
      break;
    case LINE:
      m = vtkLineMark::New();
      break;
    }
  return m;
}

//-----------------------------------------------------------------------------
void vtkMark::Extend(vtkMark* m)
{
  this->Data = m->GetData();
  this->Left.SetValue(m->GetLeft());
  this->Right.SetValue(m->GetRight());
  this->Top.SetValue(m->GetTop());
  this->Bottom.SetValue(m->GetBottom());
  this->Title.SetValue(m->GetTitle());
  this->FillColor.SetValue(m->GetFillColor());
  this->LineColor.SetValue(m->GetLineColor());
  this->LineWidth.SetValue(m->GetLineWidth());
  this->Width.SetValue(m->GetWidth());
  this->Height.SetValue(m->GetHeight());
}

double vtkMark::GetCousinLeft()
  {
  if (this->Parent && this->ParentDataIndex > 0)
    {
    vtkMark* cousin = this->Parent->GetMarkInstance(this->ParentMarkIndex, this->ParentDataIndex-1);
    return cousin->Left.GetArray(cousin)[this->Index];
    }
  return 0.0;
  }

double vtkMark::GetCousinRight()
  {
  if (this->Parent && this->ParentDataIndex > 0)
    {
    vtkMark* cousin = this->Parent->GetMarkInstance(this->ParentMarkIndex, this->ParentDataIndex-1);
    return cousin->Right.GetArray(cousin)[this->Index];
    }
  return 0.0;
  }

double vtkMark::GetCousinTop()
  {
  if (this->Parent && this->ParentDataIndex > 0)
    {
    vtkMark* cousin = this->Parent->GetMarkInstance(this->ParentMarkIndex, this->ParentDataIndex-1);
    return cousin->Top.GetArray(cousin)[this->Index];
    }
  return 0.0;
  }

double vtkMark::GetCousinBottom()
  {
  if (this->Parent && this->ParentDataIndex > 0)
    {
    vtkMark* cousin = this->Parent->GetMarkInstance(this->ParentMarkIndex, this->ParentDataIndex-1);
    return cousin->Bottom.GetArray(cousin)[this->Index];
    }
  return 0.0;
  }

double vtkMark::GetCousinWidth()
  {
  if (this->Parent && this->ParentDataIndex > 0)
    {
    vtkMark* cousin = this->Parent->GetMarkInstance(this->ParentMarkIndex, this->ParentDataIndex-1);
    return cousin->Width.GetArray(cousin)[this->Index];
    }
  return 0.0;
  }

double vtkMark::GetCousinHeight()
  {
  if (this->Parent && this->ParentDataIndex > 0)
    {
    vtkMark* cousin = this->Parent->GetMarkInstance(this->ParentMarkIndex, this->ParentDataIndex-1);
    return cousin->Height.GetArray(cousin)[this->Index];
    }
  return 0.0;
  }


//-----------------------------------------------------------------------------
void vtkMark::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
