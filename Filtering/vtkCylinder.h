/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1998 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
// .NAME vtkCylinder - implicit function for a cylinder
// .SECTION Description
// vtkCylinder computes the implicit function and function gradient for a
// cylinder. vtkCylinder is a concrete implementation of vtkImplicitFunction.
// Cylinder is centered at Center and axes of rotation is along the
// y-axis. (Use the superclass' vtkImplicitFunction transformation matrix if
// necessary to reposition.)

// .SECTION Caveats
// The cylinder is infinite in extent. To truncate the cylinder use the 
// vtkImplicitBoolean in combination with clipping planes.


#ifndef __vtkCylinder_h
#define __vtkCylinder_h

#include "vtkImplicitFunction.h"

class VTK_EXPORT vtkCylinder : public vtkImplicitFunction
{
public:
  const char *GetClassName() {return "vtkCylinder";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description
  // Construct cylinder radius of 0.5.
  static vtkCylinder *New();

  // Description
  // Evaluate cylinder equation F(x,y,z) = (x-x0)^2 + (z-z0)^2 - R^2.
  float EvaluateFunction(float x[3]);

  // Description
  // Evaluate cylinder function gradient.
  void EvaluateGradient(float x[3], float g[3]);

  // Description:
  // Set/Get cylinder radius.
  vtkSetMacro(Radius,float);
  vtkGetMacro(Radius,float);

  // Description:
  // Set/Get cylinder center
  vtkSetVector3Macro(Center,float);
  vtkGetVectorMacro(Center,float,3);
protected:
  vtkCylinder();
  ~vtkCylinder() {};
  vtkCylinder(const vtkCylinder&) {};
  void operator=(const vtkCylinder&) {};

  float Radius;
  float Center[3];

};

#endif


