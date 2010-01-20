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

#include "vtkObjectFactory.h"

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
vtkCxxRevisionMacro(vtkMark, "$Revision$");

//-----------------------------------------------------------------------------
vtkMark::vtkMark()
{
}

//-----------------------------------------------------------------------------
vtkMark::~vtkMark()
{
}

//-----------------------------------------------------------------------------
void vtkMark::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
