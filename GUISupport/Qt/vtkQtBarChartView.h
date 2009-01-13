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
/*----------------------------------------------------------------------------
 Copyright (c) Sandia Corporation
 See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.
----------------------------------------------------------------------------*/
// .NAME vtkQtBarChartView - Wraps a vtkQtChartArea into a VTK view.
//
// .SECTION Description
// vtkQtBarChartView is a type vtkQtChartView designed for line charts.
//
// .SECTION See Also
// vtkQtChartView

#ifndef __vtkQtBarChartView_h
#define __vtkQtBarChartView_h

#include "vtkQtChartViewBase.h"

class vtkQtChartArea;

class QVTK_EXPORT vtkQtBarChartView : public vtkQtChartViewBase
{
public:
  static vtkQtBarChartView *New();
  vtkTypeRevisionMacro(vtkQtBarChartView, vtkQtChartViewBase);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Updates the view.
  virtual void Update();
  
protected:
  vtkQtBarChartView();
  ~vtkQtBarChartView();
  
private:
  vtkQtBarChartView(const vtkQtBarChartView&);  // Not implemented.
  void operator=(const vtkQtBarChartView&);  // Not implemented.
};

#endif
