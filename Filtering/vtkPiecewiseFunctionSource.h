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
// .NAME vtkPiecewiseFunctionSource - abstract class whose subclasses generate piecewise functions
// .SECTION Description
// vtkPiecewiseFunctionSource is an abstract class whose subclasses generate
// piecewise functions

#ifndef __vtkPiecewiseFunctionSource_h
#define __vtkPiecewiseFunctionSource_h

#include "vtkSource.h"

class vtkPiecewiseFunction;

class VTK_FILTERING_EXPORT vtkPiecewiseFunctionSource : public vtkSource
{
public:
  vtkTypeRevisionMacro(vtkPiecewiseFunctionSource,vtkSource);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the output of this source.
  vtkPiecewiseFunction *GetOutput();
  vtkPiecewiseFunction *GetOutput(int idx);
  void SetOutput(vtkPiecewiseFunction *output);

protected:
  vtkPiecewiseFunctionSource();
  ~vtkPiecewiseFunctionSource() {};
  
private:
  vtkPiecewiseFunctionSource(const vtkPiecewiseFunctionSource&);  // Not implemented.
  void operator=(const vtkPiecewiseFunctionSource&);  // Not implemented.
};

#endif





