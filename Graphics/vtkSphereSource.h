/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1996 Ken Martin, Will Schroeder, Bill Lorensen.

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
// .NAME vtkSphereSource - create a sphere centered at the origin
// .SECTION Description
// vtkSphereSource creates a polygonal sphere of specified radius centered 
// at the origin. The resolution (polygonal discretization) in both the
// latitude (phi) and longitude (theta) directions can be specified. It also is
// possible to create partial spheres by specifying maximum phi and 
// theta angles.

#ifndef __vtkSphereSource_h
#define __vtkSphereSource_h

#include "vtkPolySource.h"

#define VTK_MAX_SPHERE_RESOLUTION 1024

class vtkSphereSource : public vtkPolySource 
{
public:
  vtkSphereSource(int res=8);
  char *GetClassName() {return "vtkSphereSource";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set radius of sphere. Default is .5.
  vtkSetClampMacro(Radius,float,0.0,VTK_LARGE_FLOAT);
  vtkGetMacro(Radius,float);

  // Description:
  // Set the center of the sphere. Default is 0,0,0.
  vtkSetVector3Macro(Center,float);
  vtkGetVectorMacro(Center,float,3);

  // Description:
  // Set the number of points in the longitude direction.
  vtkSetClampMacro(ThetaResolution,int,4,VTK_MAX_SPHERE_RESOLUTION);
  vtkGetMacro(ThetaResolution,int);

  // Description:
  // Set the number of points in the latitude direction.
  vtkSetClampMacro(PhiResolution,int,4,VTK_MAX_SPHERE_RESOLUTION);
  vtkGetMacro(PhiResolution,int);

  // Description:
  // Set the maximum longitude angle.
  vtkSetClampMacro(Theta,float,0.0,360.0);
  vtkGetMacro(Theta,float);

  // Description:
  // Set the maximum latitude angle (0 is at north pole).
  vtkSetClampMacro(Phi,float,0.0,180.0);
  vtkGetMacro(Phi,float);

protected:
  void Execute();
  float Radius;
  float Center[3];
  float Theta;
  float Phi;
  int ThetaResolution;
  int PhiResolution;

};

#endif


