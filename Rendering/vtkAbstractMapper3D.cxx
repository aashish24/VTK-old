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
#include "vtkAbstractMapper3D.h"
#include "vtkDataSet.h"

vtkCxxRevisionMacro(vtkAbstractMapper3D, "$Revision$");

//-----  This hack needed to compile using gcc3 on OSX until new stdc++.dylib
#ifdef __APPLE_CC__
extern "C"
{
  void oft_initRendering() 
  {
  extern void _ZNSt8ios_base4InitC4Ev();
  _ZNSt8ios_base4InitC4Ev();
  }
}
#endif

// Construct with initial range (0,1).
vtkAbstractMapper3D::vtkAbstractMapper3D()
{
  this->Bounds[0] = this->Bounds[2] = this->Bounds[4] = -1.0;
  this->Bounds[1] = this->Bounds[3] = this->Bounds[5] = 1.0;
  this->Center[0] = this->Center[1] = this->Center[2] = 0.0;
}

// Get the bounds for this Prop as (Xmin,Xmax,Ymin,Ymax,Zmin,Zmax).
void vtkAbstractMapper3D::GetBounds(float bounds[6])
{
  this->GetBounds();
  for (int i=0; i<6; i++)
    {
    bounds[i] = this->Bounds[i];
    }
}

float *vtkAbstractMapper3D::GetCenter()
{
  this->GetBounds();
  for (int i=0; i<3; i++) 
    {
    this->Center[i] = (this->Bounds[2*i+1] + this->Bounds[2*i]) / 2.0;
    }
  return this->Center;
}

float vtkAbstractMapper3D::GetLength()
{
  double diff, l=0.0;
  int i;

  this->GetBounds();
  for (i=0; i<3; i++)
    {
    diff = this->Bounds[2*i+1] - this->Bounds[2*i];
    l += diff * diff;
    }
 
  return (float)sqrt(l);
}

void vtkAbstractMapper3D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

