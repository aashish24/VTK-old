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
// .NAME vtkSpline - spline abstract class
// .SECTION Description
// vtkSpline is used to create interpolated data points for specified
// data. vtkSpline is an abstract class: its subclasses vtkCardinalSpline
// and vtkKochenekSpline do the interpolation. The current implementation 
// of splines is limited to data dimensions not exceeding four.
//
// Typically a spline is used by adding a sequence of points followed by
// use of an evaluation function (e.g., vtkCardinalSpline::Evaluate()).
// Since these splines are 1D, a point in this context is a independent/
// dependent variable pair. Note that the parameter space of the spline
// ranges from (0,N-1), where N is the number of points in the spline.
//
// Splines can also be set up to be closed or open. Closed splines continue
// from the last point to the first point with continuous function and 
// derivative values. (You don't need to duplicate the first point to close
// the spline, just set ClosedOn.) If the spline is closed, the parameter
// space of the spline becomes (0,N).

// .SECTION See Also
// vtkCardinalSpline vtkKochenekSpline


#ifndef __vtkSpline_h
#define __vtkSpline_h

#include "vtkObject.h"

class vtkPiecewiseFunction;

class VTK_FILTERING_EXPORT vtkSpline : public vtkObject
{
public:
  vtkTypeRevisionMacro(vtkSpline,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Virtual constructor creates a spline of the same type as this one.
  // Note that the created spline does not copy the data from this instance.
  virtual vtkSpline *MakeObject() = 0;

  // Description:
  // Set/Get ClampValue. If On, results of the interpolation will be
  // clamped to the min/max of the input data.
  vtkSetMacro(ClampValue,int);
  vtkGetMacro(ClampValue,int);
  vtkBooleanMacro(ClampValue,int);

  // Description:
  // Compute the coefficients for the spline.
  virtual void Compute () = 0;

  // Description:
  // Interpolate the value of the spline at parametric location of t.
  virtual float Evaluate (float t) = 0;

  // Description:
  // Add a pair of points to be fit with the spline.
  void AddPoint (float t, float x);

  // Description:
  // Remove a point from the data to be fit with the spline.
  void RemovePoint (float t);
 
  // Description:
  // Remove all points from the data.
  void RemoveAllPoints ();

  // Description:
  // Control whether the spline is open or closed. A closed spline forms
  // a continuous loop: the first and last points are the same, and
  // derivatives are continuous.
  vtkSetMacro(Closed,int);
  vtkGetMacro(Closed,int);
  vtkBooleanMacro(Closed,int);

  // Description:
  // Set the type of constraint of the left(right) end points. Four
  // constraints are available:
  // 
  // 0: the first derivative at left(right) most point is determined
  // from the line defined from the first(last) two points.
  //
  // 1: the first derivative at left(right) most point is set to
  // Left(Right)Value.
  // 
  // 2: the second derivative at left(right) most point is set to
  // Left(Right)Value.
  // 
  // 3: the second derivative at left(right)most points is Left(Right)Value
  // times second derivative at first interior point.
  vtkSetClampMacro(LeftConstraint,int,0,3);
  vtkGetMacro(LeftConstraint,int);
  vtkSetClampMacro(RightConstraint,int,0,3);
  vtkGetMacro(RightConstraint,int);

  // Description:
  // The values of the derivative on the left and right sides. The value
  // is used only if the left(right) constraint is type 1-3.
  vtkSetMacro(LeftValue,float);
  vtkGetMacro(LeftValue,float);
  vtkSetMacro(RightValue,float);
  vtkGetMacro(RightValue,float);

  // Description:
  // Return the MTime also considering the Piecewise function.
  unsigned long GetMTime();

  // Description:
  // Deep copy of spline data.
  virtual void DeepCopy(vtkSpline *s);

protected:
  vtkSpline();
  ~vtkSpline ();

  unsigned long ComputeTime;
  int ClampValue;
  float *Intervals;
  float *Coefficients;
  int LeftConstraint;
  float LeftValue;
  int RightConstraint;
  float RightValue;
  vtkPiecewiseFunction *PiecewiseFunction;
  int Closed;

  // Helper methods
  float ComputeLeftDerivative();
  float ComputeRightDerivative();

private:
  vtkSpline(const vtkSpline&);  // Not implemented.
  void operator=(const vtkSpline&);  // Not implemented.
};

#endif

