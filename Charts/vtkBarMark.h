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

// .NAME vtkBarMark - base class for items that are part of a vtkContextScene.
//
// .SECTION Description
// Derive from this class to create custom items that can be added to a
// vtkContextScene.

#ifndef __vtkBarMark_h
#define __vtkBarMark_h

#include "vtkMark.h"

class vtkBrush;
class vtkDataObject;
class vtkPen;

class VTK_CHARTS_EXPORT vtkBarMark : public vtkMark
{
public:
  vtkTypeRevisionMacro(vtkBarMark, vtkMark);
  virtual void PrintSelf(ostream &os, vtkIndent indent);
  static vtkBarMark* New();

  // Description:
  // Paint event for the item, called whenever the item needs to be drawn,
  virtual bool Paint(vtkContext2D *painter);

//BTX
  // Description:
  // Return true if the supplied x, y coordinate is inside the item.
  virtual bool Hit(const vtkContextMouseEvent &mouse);
//ETX

  void SetWidth(vtkValue<double> v)
    { this->Width.SetValue(v); }
  vtkValue<double>& GetWidth()
    { return this->Width.GetValue(); }

  void SetHeight(vtkValue<double> v)
    { this->Height.SetValue(v); }
  vtkValue<double>& GetHeight()
    { return this->Height.GetValue(); }

  void SetLineColor(vtkValue<vtkColor> v)
    { this->LineColor.SetValue(v); }
  vtkValue<vtkColor>& GetLineColor()
    { return this->LineColor.GetValue(); }

  void SetFillColor(vtkValue<vtkColor> v)
    { this->FillColor.SetValue(v); }
  vtkValue<vtkColor>& GetFillColor()
    { return this->FillColor.GetValue(); }

//BTX
protected:
  vtkBarMark();
  ~vtkBarMark();

  vtkValueHolder<double> Width;
  vtkValueHolder<double> Height;
  vtkValueHolder<vtkColor> FillColor;
  vtkValueHolder<vtkColor> LineColor;
  vtkValueHolder<double> LineWidth;
private:
  vtkBarMark(const vtkBarMark &); // Not implemented.
  void operator=(const vtkBarMark &);   // Not implemented.
//ETX
};

#endif //__vtkBarMark_h
