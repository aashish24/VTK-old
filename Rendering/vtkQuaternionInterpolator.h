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
// .NAME vtkQuaternionInterpolator - interpolate a quaternion
// .SECTION Description
// This class is used to interpolate a series of quaternions (that represent
// the rotations of a 3D object).  The interpolation may be linear in form
// (using spherical linear interpolation SLERP), or via spline interpolation
// (using SQUAD). In either case the interpolation is specialized to
// quaternions since the interpolation occurs on the surface of the unit
// quaternion sphere.
//
// To use this class, specify at least two pairs of (t,q[4]) with the
// AddQuaternion() method.  Next interpolate the tuples with the
// InterpolateQuaternion(t,q[4]) method, where "t" must be in the range of
// (t_min,t_max) parameter values specified by the AddQuaternion() method ( t
// is clamped otherwise), and q[4] is filled in by the method.
//


#ifndef __vtkQuaternionInterpolator_h
#define __vtkQuaternionInterpolator_h

#include "vtkObject.h"

class vtkQuaternionList;


class VTK_RENDERING_EXPORT vtkQuaternionInterpolator : public vtkObject
{
public:
  vtkTypeRevisionMacro(vtkQuaternionInterpolator, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Instantiate the class.
  static vtkQuaternionInterpolator* New();

  // Description:
  // Return the number of quaternions in the list of quaternions to be
  // interpolated.
  int GetNumberOfQuaternions();

  // Description:
  // Obtain some information about the interpolation range. The numbers
  // returned (corresponding to parameter t, usually thought of as time)
  // are undefined if the list of transforms is empty. This is a convenience
  // method for interpolation.
  double GetMinimumT();
  double GetMaximumT();

  // Description:
  // Reset the class so that it contains no data; i.e., the array of (t,q[4])
  // information is discarded.
  void Initialize();
  
  // Description:
  // Add another quaternion to the list of quaternions to be interpolated.
  // Note that using the same time t value more than once replaces the
  // previous quaternion at t.  At least one quaternions must be added to
  // define an interpolation functios.
  void AddQuaternion(double t, double q[4]);

  // Description:
  // Delete the quaternion at a particular parameter t. If there is no
  // quaternion tuple defined at t, then the method does nothing.
  void RemoveQuaternion(double t);

  // Description:
  // Interpolate the list of quaternions and determine a new quaternion
  // (i.e., fill in the quaternion provided). If t is outside the range of
  // (min,max) values, then t is clamped to lie within the range. 
  void InterpolateQuaternion(double t, double q[4]);

//BTX
  // Description:
  // Enums to control the type of interpolation to use.
  enum {INTERPOLATION_TYPE_LINEAR=0,
        INTERPOLATION_TYPE_SPLINE
  };
//ETX

  // Description:
  // Specify which type of function to use for interpolation. By default
  // (SetInterpolationFunctionToSpline()), cubic spline interpolation using a
  // modifed Kochanek basis is employed. Otherwise, if
  // SetInterpolationFunctionToLinear() is invoked, linear spherical interpolation
  // is used between each pair of quaternions.
  vtkSetClampMacro(InterpolationType,int,INTERPOLATION_TYPE_LINEAR,
                   INTERPOLATION_TYPE_SPLINE);
  vtkGetMacro(InterpolationType,int);
  void SetInterpolationTypeToLinear()
    {this->SetInterpolationType(INTERPOLATION_TYPE_LINEAR);}
  void SetInterpolationTypeToSpline()
    {this->SetInterpolationType(INTERPOLATION_TYPE_SPLINE);}

protected:
  vtkQuaternionInterpolator();
  virtual ~vtkQuaternionInterpolator();

  // Specify the type of interpolation to use
  int InterpolationType;

  // Internal variables for interpolation functions
  vtkQuaternionList *QuaternionList; //used for linear quaternion interpolation
  
  // Internal method for spherical, linear interpolation
  void Slerp(double t, double q0[4], double q1[4], double q[4]);

  // Internal methods for spherical, cubic interpolation
  void InnerPoint(double q0[4], double q1[4], double q2[4], double q[4]);
  void Squad(double t, double q0[4], double a[4], double b[4], double q1[4], 
             double q[4]);

private:
  vtkQuaternionInterpolator(const vtkQuaternionInterpolator&);  // Not implemented.
  void operator=(const vtkQuaternionInterpolator&);  // Not implemented.

};

#endif
