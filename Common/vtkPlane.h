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
// .NAME vtkPlane - perform various plane computations
// .SECTION Description
// vtkPlane provides methods for various plane computations. These include
// projecting points onto a plane, evaluating the plane equation, and 
// returning plane normal. vtkPlane is a concrete implementation of the 
// abstract class vtkImplicitFunction.

#ifndef __vtkPlane_h
#define __vtkPlane_h

#include <math.h>
#include "vtkImplicitFunction.h"

class VTK_EXPORT vtkPlane : public vtkImplicitFunction
{
public:
  // Description
  // Construct plane passing through origin and normal to z-axis.
  static vtkPlane *New();

  vtkTypeMacro(vtkPlane,vtkImplicitFunction);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description
  // Evaluate plane equation for point x[3].
  float EvaluateFunction(float x[3]);
  float EvaluateFunction(float x, float y, float z)
    {return this->vtkImplicitFunction::EvaluateFunction(x, y, z); } ;

  // Description
  // Evaluate function gradient at point x[3].
  void EvaluateGradient(float x[3], float g[3]);

  // Description:
  // Set/get plane normal. Plane is defined by point and normal.
  vtkSetVector3Macro(Normal,float);
  vtkGetVectorMacro(Normal,float,3);

  // Description:
  // Set/get point through which plane passes. Plane is defined by point 
  // and normal.
  vtkSetVector3Macro(Origin,float);
  vtkGetVectorMacro(Origin,float,3);

  // Description
  // Project a point x onto plane defined by origin and normal. The 
  // projected point is returned in xproj. NOTE : normal assumed to
  // have magnitude 1.
  static void ProjectPoint(float x[3], float origin[3], float normal[3], 
                           float xproj[3]);
  static void ProjectPoint(double x[3], double origin[3], double normal[3], 
                           double xproj[3]);

  // Description
  // Project a point x onto plane defined by origin and normal. The 
  // projected point is returned in xproj. NOTE : normal does NOT have to 
  // have magnitude 1.
  static void GeneralizedProjectPoint(float x[3], float origin[3],
				      float normal[3], float xproj[3]);
  
  // Description:
  // Quick evaluation of plane equation n(x-origin)=0.
  static float Evaluate(float normal[3], float origin[3], float x[3]);
  static float Evaluate(double normal[3], double origin[3], double x[3]);

  // Description:
  // Return the distance of a point x to a plane defined by n(x-p0) = 0. The
  // normal n[3] must be magnitude=1.
  static float DistanceToPlane(float x[3], float n[3], float p0[3]);
  
  // Description:
  // Given a line defined by the two points p1,p2; and a plane defined by the
  // normal n and point p0, compute an intersection. The parametric
  // coordinate along the line is returned in t, and the coordinates of 
  // intersection are returned in x. A zero is returned if the plane and line
  // are parallel.
  static int IntersectWithLine(float p1[3], float p2[3], float n[3], 
                               float p0[3], float& t, float x[3]);


protected:
  vtkPlane();
  ~vtkPlane() {};
  vtkPlane(const vtkPlane&) {};
  void operator=(const vtkPlane&) {};

  float Normal[3];
  float Origin[3];

};

inline float vtkPlane::Evaluate(float normal[3], float origin[3], float x[3])
{
  return normal[0]*(x[0]-origin[0]) + normal[1]*(x[1]-origin[1]) + 
         normal[2]*(x[2]-origin[2]);
}
inline float vtkPlane::Evaluate(double normal[3], double origin[3],double x[3])
{
  return normal[0]*(x[0]-origin[0]) + normal[1]*(x[1]-origin[1]) + 
         normal[2]*(x[2]-origin[2]);
}

inline float vtkPlane::DistanceToPlane(float x[3], float n[3], float p0[3])
{
  return ((float) fabs(n[0]*(x[0]-p0[0]) + n[1]*(x[1]-p0[1]) + 
                       n[2]*(x[2]-p0[2])));
}

#endif


