/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Toolkit. No part of this file
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
// .NAME vtkWarpScalar - deform geometry with scalar data
// .SECTION Description
// vtkWarpScalar is a filter that modifies point coordinates by moving
// points along point normals by the scalar amount times the scale factor.
// Useful for creating carpet or x-y-z plots.
//    If normals are not present in data, the Normal instance variable will
// be used as the direction along which to warp the geometry. If normals are
// present but you would like to use the Normals instance variable, set the 
// UseNormals boolean to true.

#ifndef __vtkWarpScalar_h
#define __vtkWarpScalar_h

#include "PtS2PtSF.hh"

class vtkWarpScalar : public vtkPointSetToPointSetFilter
{
public:
  vtkWarpScalar();
  ~vtkWarpScalar() {};
  char *GetClassName() {return "vtkWarpScalar";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify value to scale displacement.
  vtkSetMacro(ScaleFactor,float);
  vtkGetMacro(ScaleFactor,float);

  // Description:
  // Turn on/off use of user specified normal. If on, data normals
  // will be ignored and instance variable Normal will be used instead.
  vtkSetMacro(UseNormal,int);
  vtkGetMacro(UseNormal,int);
  vtkBooleanMacro(UseNormal,int);

  // Description:
  // Normal (i.e., direction) along which to warp geometry. Only used
  // if UseNormal boolean set to true or no normals available in data.
  vtkSetVector3Macro(Normal,float);
  vtkGetVectorMacro(Normal,float,3);

protected:
  void Execute();

  float ScaleFactor;
  int UseNormal;
  float Normal[3];

  //BTX
  float *(vtkWarpScalar::*PointNormal)(int id, vtkNormals *normals);
  float *DataNormal(int id, vtkNormals *normals=NULL);
  float *InstanceNormal(int id, vtkNormals *normals=NULL);
  //ETX
};

#endif


