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

// .NAME vtkMark - base class for items that are part of a vtkContextScene.
//
// .SECTION Description
// Derive from this class to create custom items that can be added to a
// vtkContextScene.

#ifndef __vtkMark_h
#define __vtkMark_h

#include "vtkContextItem.h"
#include "vtkDataElement.h"
#include "vtkSmartPointer.h"
#include "vtkVariant.h"

template <typename T>
class vtkValue
{
public:
  typedef T (*FunctionType)(vtkMark*, vtkDataElement&);
  vtkValue() : Function(NULL) { }
  vtkValue(FunctionType f) : Function(f) { }
  vtkValue(T v) : Constant(v), Function(NULL) { }
  bool IsConstant()
    { return this->Function == NULL; }
  T GetConstant()
    {
    return this->Constant;
    }
  FunctionType GetFunction()
    { return this->Function; }

protected:
  T Constant;
  FunctionType Function;
};

class VTK_CHARTS_EXPORT vtkDataValue : public vtkValue<vtkDataElement>
{
public:
  vtkDataValue() { this->Function = NULL; }
  vtkDataValue(FunctionType f)  { this->Function = f; }
  vtkDataValue(vtkDataElement v) { this->Constant = v; this->Function = NULL; }
  vtkDataElement GetData(vtkMark* m);
};

template <typename T>
class vtkValueHolder
{
public:
  vtkValueHolder() : Dirty(true), Set(false) { }

  void UnsetValue()
    { Set = false; }
  void SetValue(vtkValue<T> v)
    { Dirty = true; Set = true; Value = v; }
  vtkValue<T>& GetValue()
    { return Value; }

  T* GetArray(vtkMark* m)
    {
    this->Update(m);
    if (this->Cache.size() == 0)
      {
      return NULL;
      }
    return &(this->Cache[0]);
    }

  T GetConstant(vtkMark* m)
    {
    this->Update(m);
    if (this->Cache.size() > 0)
      {
      return this->Cache[0];
      }
    return this->Value.GetConstant();
    }

  bool IsSet()
    {
    return this->Set;
    }

  bool IsDirty()
    {
    return this->Dirty;
    }

  void SetDirty(bool b)
    {
    this->Dirty = b;
    }

  void Update(vtkMark* m);

protected:
  vtkValue<T> Value;
  std::vector<T> Cache;
  bool Dirty;
  bool Set;
};

class vtkColor
{
public:
  vtkColor() :
    Red(0.0), Green(0.0), Blue(0.0), Alpha(1.0) { }
  vtkColor(double r, double g, double b) :
    Red(r), Green(g), Blue(b), Alpha(1.0) { }
  vtkColor(double r, double g, double b, double a) :
    Red(r), Green(g), Blue(b), Alpha(a) { }
  double Red;
  double Green;
  double Blue;
  double Alpha;
};

class VTK_CHARTS_EXPORT vtkMark : public vtkContextItem
{
public:
  vtkTypeRevisionMacro(vtkMark, vtkContextItem);
  virtual void PrintSelf(ostream &os, vtkIndent indent);
  static vtkMark* New();

  enum {
    BAR,
    LINE
    };

  static vtkMark* CreateMark(int type);

  virtual void Extend(vtkMark* m);

  virtual bool Paint(vtkContext2D* vtkNotUsed(painter)) { return true; }

  virtual void Update()
    {
    this->Left.Update(this);
    this->Right.Update(this);
    this->Top.Update(this);
    this->Bottom.Update(this);
    this->Title.Update(this);
    this->FillColor.Update(this);
    this->LineColor.Update(this);
    this->LineWidth.Update(this);
    this->Width.Update(this);
    this->Height.Update(this);
    }

  void SetData(vtkDataValue data)
    {
      this->Data = data;
      this->DataChanged();
    }
  vtkDataValue GetData()
    { return this->Data; }

  void SetLeft(vtkValue<double> v)
    { this->Left.SetValue(v); }
  vtkValue<double>& GetLeft()
    { return this->Left.GetValue(); }

  void SetRight(vtkValue<double> v)
    { this->Right.SetValue(v); }
  vtkValue<double>& GetRight()
    { return this->Right.GetValue(); }

  void SetTop(vtkValue<double> v)
    { this->Top.SetValue(v); }
  vtkValue<double>& GetTop()
    { return this->Top.GetValue(); }

  void SetBottom(vtkValue<double> v)
    { this->Bottom.SetValue(v); }
  vtkValue<double>& GetBottom()
    { return this->Bottom.GetValue(); }

  void SetTitle(vtkValue<std::string> v)
    { this->Title.SetValue(v); }
  vtkValue<std::string>& GetTitle()
    { return this->Title.GetValue(); }

  void SetLineColor(vtkValue<vtkColor> v)
    { this->LineColor.SetValue(v); }
  vtkValue<vtkColor>& GetLineColor()
    { return this->LineColor.GetValue(); }

  void SetFillColor(vtkValue<vtkColor> v)
    { this->FillColor.SetValue(v); }
  vtkValue<vtkColor>& GetFillColor()
    { return this->FillColor.GetValue(); }

  void SetLineWidth(vtkValue<double> v)
    { this->LineWidth.SetValue(v); }
  vtkValue<double>& GetLineWidth()
    { return this->LineWidth.GetValue(); }

  void SetWidth(vtkValue<double> v)
    { this->Width.SetValue(v); }
  vtkValue<double>& GetWidth()
    { return this->Width.GetValue(); }

  void SetHeight(vtkValue<double> v)
    { this->Height.SetValue(v); }
  vtkValue<double>& GetHeight()
    { return this->Height.GetValue(); }

  void SetParent(vtkPanelMark* p)
    { this->Parent = p; }
  vtkPanelMark* GetParent()
    { return this->Parent; }

  vtkSetMacro(ParentMarkIndex, int);
  vtkGetMacro(ParentMarkIndex, int);

  vtkSetMacro(ParentDataIndex, int);
  vtkGetMacro(ParentDataIndex, int);

  void SetIndex(vtkIdType i)
    { this->Index = i; }
  vtkIdType GetIndex()
    { return this->Index; }

  double GetCousinLeft();
  double GetCousinRight();
  double GetCousinTop();
  double GetCousinBottom();
  double GetCousinWidth();
  double GetCousinHeight();

  virtual void DataChanged()
    {
    this->Left.SetDirty(true);
    this->Right.SetDirty(true);
    this->Top.SetDirty(true);
    this->Bottom.SetDirty(true);
    this->Title.SetDirty(true);
    this->FillColor.SetDirty(true);
    this->LineColor.SetDirty(true);
    this->LineWidth.SetDirty(true);
    this->Width.SetDirty(true);
    this->Height.SetDirty(true);
    }

  virtual int GetType() { return BAR; }

//BTX
protected:
  vtkMark();
  ~vtkMark();

  bool PaintBar(vtkContext2D *painter);
  bool PaintLine(vtkContext2D *painter);

  vtkDataValue Data;
  vtkValueHolder<double> Left;
  vtkValueHolder<double> Right;
  vtkValueHolder<double> Top;
  vtkValueHolder<double> Bottom;
  vtkValueHolder<std::string> Title;
  vtkValueHolder<vtkColor> FillColor;
  vtkValueHolder<vtkColor> LineColor;
  vtkValueHolder<double> LineWidth;
  vtkValueHolder<double> Width;
  vtkValueHolder<double> Height;

  vtkPanelMark* Parent;
  vtkIdType ParentMarkIndex;
  vtkIdType ParentDataIndex;
  vtkIdType Index;

private:
  vtkMark(const vtkMark &); // Not implemented.
  void operator=(const vtkMark &);   // Not implemented.
//ETX
};

template <typename T>
void vtkValueHolder<T>::Update(vtkMark* m)
{
  if (!this->Dirty)
    {
    return;
    }
  vtkDataElement d = m->GetData().GetData(m);
  vtkIdType numChildren = 1;
  if(d.IsValid())
    {
    numChildren = d.GetNumberOfChildren();
    }

  this->Cache.resize(numChildren);
  if (this->Value.IsConstant())
    {
    for (vtkIdType i = 0; i < numChildren; ++i)
      {
      this->Cache[i] = this->Value.GetConstant();
      }
    }
  else
    {
    for (vtkIdType i = 0; i < numChildren; ++i)
      {
      m->SetIndex(i);
      vtkDataElement e = d.GetChild(i);
      this->Cache[i] = this->Value.GetFunction()(m, e);
      }
    }
  this->Dirty = false;
}

//-----------------------------------------------------------------------------
class VTK_CHARTS_EXPORT vtkMarkUtil
{
public:
  static vtkColor DefaultSeriesColor(vtkMark* m, vtkDataElement&);
  static double StackLeft(vtkMark* m, vtkDataElement&)
  {
    return m->GetCousinLeft() + m->GetCousinWidth();
  }
  static double StackBottom(vtkMark* m, vtkDataElement&)
  {
    return m->GetCousinBottom() + m->GetCousinHeight();
  }
};

#endif //__vtkMark_h
