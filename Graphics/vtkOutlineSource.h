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
// .NAME vtkOutlineSource - create wireframe outline around bounding box
// .SECTION Description
// vtkOutlineSource creates a wireframe outline around a user-specified 
// bounding box. 
// The outline may be created aligned with the {x,y,z} axis - in which case
// it is defined by the 6 bounds {xmin,xmax,ymin,ymax,zmin,zmax} via SetBounds()
// Alternatively, the box may be arbitrarily aligned, in which case
// it should be set via the SetCorners() member.

#ifndef __vtkOutlineSource_h
#define __vtkOutlineSource_h

#include "vtkPolyDataSource.h"

#define VTK_BOX_TYPE_AXIS_ALIGNED 0
#define VTK_BOX_TYPE_ORIENTED     1

class VTK_GRAPHICS_EXPORT vtkOutlineSource : public vtkPolyDataSource 
{
public:
  static vtkOutlineSource *New();
  vtkTypeRevisionMacro(vtkOutlineSource,vtkPolyDataSource);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set box type to AxisAligned (default) or Oriented
  // use SetBounds with Axis aligned, and SetCorners with Oriented
  vtkSetMacro(BoxType,int);
  vtkGetMacro(BoxType,int);
  void SetBoxTypeToAxisAligned() 
    {this->SetBoxType(VTK_BOX_TYPE_AXIS_ALIGNED);};
  void SetBoxTypeToOriented() 
    {this->SetBoxType(VTK_BOX_TYPE_ORIENTED);};

  // Description:
  // Specify the bouds of the box to be used in Axis Aligned mode
  vtkSetVector6Macro(Bounds,float);
  vtkGetVectorMacro(Bounds,float,6);

  // Description:
  // Specify the corners of the outline when in Oriented mode,
  // the values are supplied as 8*3 float values
  // The correct corner ordering is using {x,y,z} convention for 
  // the unit cube...
  // {0,0,0},{1,0,0},{0,1,0},{1,1,0},{0,0,1},{1,0,1},{0,1,1},{1,1,1}
  vtkSetVectorMacro(Corners,float,24);
  vtkGetVectorMacro(Corners,float,24);

protected:
  vtkOutlineSource();
  ~vtkOutlineSource() {};

  void Execute();
  int   BoxType;
  float Bounds[6];
  float Corners[24];
private:
  vtkOutlineSource(const vtkOutlineSource&);  // Not implemented.
  void operator=(const vtkOutlineSource&);  // Not implemented.
};

#endif


