/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkCardinalSpline - computes an interpolating spline using a
// a Cardinal basis.

// .SECTION Description
// vtkCardinalSpline is a concrete implementation of vtkSpline using a
// Cardinal basis.

// .SECTION See Also
// vtkSpline vtkKochanekSpline


#ifndef __vtkCardinalSpline_h
#define __vtkCardinalSpline_h

#include "vtkSpline.h"

class VTK_FILTERING_EXPORT vtkCardinalSpline : public vtkSpline
{
public:
  static vtkCardinalSpline *New();

  vtkTypeRevisionMacro(vtkCardinalSpline,vtkSpline);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Virtual constructor creates a spline of the same type as this one.
  // Note that the created spline does not copy the data from this instance.
  virtual vtkSpline *MakeObject()
    { return vtkCardinalSpline::New(); }

  // Description
  // Compute Cardinal Splines for each dependent variable
  void Compute ();

  // Description:
  // Evaluate a 1D cardinal spline.
  virtual float Evaluate (float t);

  // Description:
  // Deep copy of cardinal spline data.
  virtual void DeepCopy(vtkSpline *s);

protected:
  vtkCardinalSpline();
  ~vtkCardinalSpline() {}

  void Fit1D (int n, float *x, float *y, float *w, float coefficients[][4],
              int leftConstraint, float leftValue, int rightConstraint, 
              float rightValue);

  void FitClosed1D (int n, float *x, float *y, float *w, 
                    float coefficients[][4]);

private:
  vtkCardinalSpline(const vtkCardinalSpline&);  // Not implemented.
  void operator=(const vtkCardinalSpline&);  // Not implemented.
};

#endif

