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
// .NAME vtkConeSource - generate polygonal cone 
// .SECTION Description
// vtkConeSource creates a cone centered at origin and pointing down the 
// x-axis. Depending upon the resolution of this object, different 
// representations are created. If resolution=0 a line is created; if 
// resolution=1, a single triangle is created; if resolution=2, two 
// crossed triangles are created. For resolution > 2, a 3D cone (with 
// resolution number of sides) is created. It also is possible to control 
// whether the bottom of the cone is capped with a (resolution-sided) 
// polygon, and to specify the height and radius of the cone.

#ifndef __vtkConeSource_h
#define __vtkConeSource_h

#include "vtkPolyDataSource.h"

class VTK_GRAPHICS_EXPORT vtkConeSource : public vtkPolyDataSource 
{
public:
  vtkTypeRevisionMacro(vtkConeSource,vtkPolyDataSource);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct with default resolution 6, height 1.0, radius 0.5, and
  // capping on.
  static vtkConeSource *New();

  // Description:
  // Set the height of the cone.
  vtkSetClampMacro(Height,float,0.0,VTK_LARGE_FLOAT)
  vtkGetMacro(Height,float);

  // Description:
  // Set the radius of the cone.
  vtkSetClampMacro(Radius,float,0.0,VTK_LARGE_FLOAT)
  vtkGetMacro(Radius,float);

  // Description:
  // Set the number of facets used to represent the cone.
  vtkSetClampMacro(Resolution,int,0,VTK_CELL_SIZE)
  vtkGetMacro(Resolution,int);

  // Description:
  // Set the angle of the cone. As a side effect, this method sets radius.
  void SetAngle (float angle);
  float GetAngle ();

  // Description:
  // Turn on/off whether to cap the cone with a polygon.
  vtkSetMacro(Capping,int);
  vtkGetMacro(Capping,int);
  vtkBooleanMacro(Capping,int);

protected:
  vtkConeSource(int res=6);
  ~vtkConeSource() {};

  void Execute();
  void ExecuteInformation();
  float Height;
  float Radius;
  int Resolution;
  int Capping;
private:
  vtkConeSource(const vtkConeSource&);  // Not implemented.
  void operator=(const vtkConeSource&);  // Not implemented.
};

#endif


