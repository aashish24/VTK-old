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
#include "vtkCylinder.h"

// Description
// Construct cylinder radius of 0.5.
vtkCylinder::vtkCylinder()
{
  this->Center[0] = this->Center[1] = this->Center[2] = 0.0;
  this->Radius = 0.5;
}

// Description
// Evaluate cylinder equation F(x,y,z) = (x-x0)^2 + (z-z0)^2 - R^2.
float vtkCylinder::EvaluateFunction(float xyz[3])
{
  float x = xyz[0] - this->Center[0];
  float z = xyz[2] - this->Center[2];

  return ( x * x + z * z - this->Radius*this->Radius );
}

// Description
// Evaluate cylinder function gradient.
void vtkCylinder::EvaluateGradient(float x[3], float g[3])
{
  g[0] = 2.0 * x[0];
  g[1] = 0.0;
  g[2] = 2.0 * x[2];
}

void vtkCylinder::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkImplicitFunction::PrintSelf(os,indent);

  os << indent << "Center: " << "( " << this->Center[0] << ", " <<
     this->Center[1] << ", " << this->Center[2] << " )";

  os << indent << "Radius: " << this->Radius << "\n";
}
