/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS EVEN, SOFTWARE IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include <stdlib.h>
#include "vtkTransform.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
vtkTransform* vtkTransform::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkTransform");
  if(ret)
    {
    return (vtkTransform*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkTransform;
}

//----------------------------------------------------------------------------
// Constructs a transform and sets the following defaults
// preMultiplyFlag = 1 stackSize = 10. It then
// creates an identity matrix as the top matrix on the stack.
vtkTransform::vtkTransform()
{
  this->PreMultiplyFlag = 1;
  this->StackSize = 10;
  this->Stack = new vtkMatrix4x4 *[this->StackSize];
  this->StackBottom = this->Stack;

  this->Point[0] = this->Point[1] = this->Point[2] = this->Point[3] = 0.0;
  this->DoublePoint[0] = 
    this->DoublePoint[1] = this->DoublePoint[2] = this->DoublePoint[3] = 0.0;
}

//----------------------------------------------------------------------------
vtkTransform::~vtkTransform()
{
  int n = this->Stack-this->StackBottom;
  for (int i = 0; i < n; i++)
    {
    this->StackBottom[i]->Delete();
    }

  delete [] this->StackBottom;
}

//----------------------------------------------------------------------------
void vtkTransform::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkLinearTransform::PrintSelf(os, indent);

  os << indent << (this->PreMultiplyFlag ? "PreMultiply\n" : "PostMultiply\n");

  os << indent << "DoublePoint: " << "( " << 
     this->DoublePoint[0] << ", " << this->DoublePoint[1] << ", " <<
     this->DoublePoint[2] << ", " << this->DoublePoint[3] << ")\n";

  os << indent << "Point: " << "( " << 
     this->Point[0] << ", " << this->Point[1] << ", " <<
     this->Point[2] << ", " << this->Point[3] << ")\n";
}

//----------------------------------------------------------------------------
void vtkTransform::Identity()
{
  this->Matrix->Identity();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkTransform::Inverse()
{
  this->Matrix->Invert();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkTransform::Transpose()
{
  this->Matrix->Transpose();
  this->Modified();
}

//----------------------------------------------------------------------------
// Set the current matrix directly.
void vtkTransform::SetMatrix(const double Elements[16])
{
  this->Matrix->DeepCopy(Elements);
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkTransform::Translate(double x, double y, double z)
{
  if (x == 0.0 && y == 0.0 && z == 0.0) 
    {
    return;
    }

  double matrix[4][4];
  vtkMatrix4x4::Identity(*matrix);

  matrix[0][3] = x;
  matrix[1][3] = y;
  matrix[2][3] = z;

  this->Concatenate(*matrix);
}

//----------------------------------------------------------------------------
void vtkTransform::RotateWXYZ(double angle, double x, double y, double z)
{
  if (x == 0.0 && y == 0.0 && z == 0.0) 
    {
    vtkErrorMacro(<<"Trying to rotate around zero-length axis");
    return;
    }

  if (angle == 0)
    {
    return;
    }

  // convert to radians
  angle = angle*vtkMath::DoubleDegreesToRadians();

  // make a normalized quaternion
  double w = cos(0.5*angle);
  double f = sin(0.5*angle)/sqrt(x*x+y*y+z*z);
  x *= f;
  y *= f;
  z *= f;

  // convert the quaternion to a matrix
  double matrix[4][4];
  vtkMatrix4x4::Identity(*matrix);

  double ww = w*w;
  double wx = w*x;
  double wy = w*y;
  double wz = w*z;

  double xx = x*x;
  double yy = y*y;
  double zz = z*z;

  double xy = x*y;
  double xz = x*z;
  double yz = y*z;

  double ss = (ww - xx - yy - zz)/2;

  matrix[0][0] = ( ss + xx)*2;
  matrix[1][0] = ( wz + xy)*2;
  matrix[2][0] = (-wy + xz)*2;

  matrix[0][1] = (-wz + xy)*2;
  matrix[1][1] = ( ss + yy)*2;
  matrix[2][1] = ( wx + yz)*2;

  matrix[0][2] = ( wy + xz)*2;
  matrix[1][2] = (-wx + yz)*2;
  matrix[2][2] = ( ss + zz)*2;

  this->Concatenate(*matrix);
}

//----------------------------------------------------------------------------
void vtkTransform::Scale(double x, double y, double z)
{
  if (x == 0.0 && y == 0.0 && z == 0.0) 
    {
    return;
    }

  double matrix[4][4];
  vtkMatrix4x4::Identity(*matrix);

  matrix[0][0] = x;
  matrix[1][1] = y;
  matrix[2][2] = z;

  this->Concatenate(*matrix);
}

//----------------------------------------------------------------------------
// Concatenates the input matrix with the current matrix.
// The setting of the PreMultiply flag determines whether the matrix
// is PreConcatenated or PostConcatenated.
void vtkTransform::Concatenate(const double Elements[16])
{
  if (this->PreMultiplyFlag) 
    {
    vtkMatrix4x4::Multiply4x4(*this->Matrix->Element, Elements, 
			      *this->Matrix->Element);
    }
  else 
    {
    vtkMatrix4x4::Multiply4x4(Elements, *this->Matrix->Element, 
			      *this->Matrix->Element);
    }
  this->Matrix->Modified();
  this->Modified();
}

//----------------------------------------------------------------------------
// Sets the internal state of the Transform to
// post multiply. All subsequent matrix
// operations will occur after those already represented
// in the current Transformation matrix.
void vtkTransform::PostMultiply()
{
  if (this->PreMultiplyFlag != 0) 
    {
    this->PreMultiplyFlag = 0;
    this->Modified ();
    }
}

//----------------------------------------------------------------------------
// Sets the internal state of the Transform to
// pre multiply. All subsequent matrix
// operations will occur before those already represented
// in the current Transformation matrix.
void vtkTransform::PreMultiply()
{
  if (this->PreMultiplyFlag != 1) 
    {
    this->PreMultiplyFlag = 1;
    this->Modified ();
    }
}

//----------------------------------------------------------------------------
// support for the vtkGeneralTransform superclass
void vtkTransform::InternalDeepCopy(vtkGeneralTransform *transform)
{
  vtkTransform *t = (vtkTransform *)transform;

  this->PreMultiplyFlag = t->PreMultiplyFlag;
  this->StackSize = t->StackSize;
  this->Matrix->DeepCopy(t->Matrix);

  // free old stack
  if (this->StackBottom)
    {
    int n = this->Stack-this->StackBottom;
    for (int i = 0; i < n; i++)
      {
      this->StackBottom[i]->Delete();
      }
    delete [] this->StackBottom;
    } 

  // allocate and copy stack
  this->StackBottom = new vtkMatrix4x4 *[this->StackSize];

  // copy the stack
  int n = t->Stack-t->StackBottom;
  for (int i = 0; i < n; i++ )
    {
    this->StackBottom[i] = vtkMatrix4x4::New();
    (this->StackBottom[i])->DeepCopy(t->Stack[i]);
    }
  this->Stack = this->StackBottom + n;

  // copy Point and DoublePoint
  for (int j = 0; j < 3; j++)
    {
    this->Point[j] = t->Point[j];
    this->DoublePoint[j] = t->DoublePoint[j];
    }

  this->Modified();
}

//----------------------------------------------------------------------------
// support for the vtkGeneralTransform superclass
vtkGeneralTransform *vtkTransform::MakeTransform()
{
  return vtkTransform::New();
}

//----------------------------------------------------------------------------
// Deletes the transformation on the top of the stack and sets the top 
// to the next transformation on the stack.
void vtkTransform::Pop()
{
  // if we're at the bottom of the stack, don't pop
  if (this->Stack == this->StackBottom)
    {
    return;
    }

  this->Matrix->DeepCopy(*--this->Stack);

  // free the stack matrix storage
  (*this->Stack)->Delete();
  (*this->Stack) = NULL;

  this->Modified ();
}

//----------------------------------------------------------------------------
// Pushes the current transformation matrix onto the
// transformation stack.
void vtkTransform::Push()
{
  if ((this->Stack - this->StackBottom) > this->StackSize) 
    {
    vtkErrorMacro("Push: Exceeded matrix stack size");
    return;
    }

  // allocate a new matrix on the stack
  (*this->Stack) = vtkMatrix4x4::New();
  
  // set the new matrix to the previous top of stack matrix
  (*this->Stack++)->DeepCopy(this->Matrix);

  this->Modified ();
}

//----------------------------------------------------------------------------
// Return the inverse of the current transformation matrix.
void vtkTransform::GetInverse(vtkMatrix4x4 *inverse)
{
  vtkMatrix4x4::Invert(this->Matrix,inverse);
}

//----------------------------------------------------------------------------
// Obtain the transpose of the current transformation matrix.
void vtkTransform::GetTranspose(vtkMatrix4x4 *transpose)
{
  vtkMatrix4x4::Transpose(this->Matrix,transpose);
}

//----------------------------------------------------------------------------
// Get the x, y, z orientation angles from the transformation matrix as an
// array of three floating point values.
void vtkTransform::GetOrientation(double orientation[3])
{
#define VTK_AXIS_EPSILON 0.001

  // convenient access to matrix
  double (*matrix)[4] = this->Matrix->Element;

  // get scale factors
  double scale[3];
  this->GetScale(scale);

  // first rotate about y axis
  double x2 = matrix[2][0]/scale[0];
  double y2 = matrix[2][1]/scale[1];
  double z2 = matrix[2][2]/scale[2];

  double x3 = matrix[1][0]/scale[0];
  double y3 = matrix[1][1]/scale[1];
  double z3 = matrix[1][2]/scale[2];

  double d1 = sqrt(x2*x2 + z2*z2);

  double cosTheta, sinTheta;
  if (d1 < VTK_AXIS_EPSILON) 
    {
    cosTheta = 1.0;
    sinTheta = 0.0;
    }
  else 
    {
    cosTheta = z2/d1;
    sinTheta = x2/d1;
    }

  double theta = atan2(sinTheta, cosTheta);
  orientation[1] = -theta/vtkMath::DoubleDegreesToRadians();

  // now rotate about x axis
  double d = sqrt(x2*x2 + y2*y2 + z2*z2);

  double sinPhi, cosPhi;
  if (d < VTK_AXIS_EPSILON) 
    {
    sinPhi = 0.0;
    cosPhi = 1.0;
    }
  else if (d1 < VTK_AXIS_EPSILON) 
    {
    sinPhi = y2/d;
    cosPhi = z2/d;
    }
  else 
    {
    sinPhi = y2/d;
    cosPhi = (x2*x2 + z2*z2)/(d1*d);
    }

  double phi = atan2(sinPhi, cosPhi);
  orientation[0] = phi/vtkMath::DoubleDegreesToRadians();

  // finally, rotate about z
  double x3p = x3*cosTheta - z3*sinTheta;
  double y3p = - sinPhi*sinTheta*x3 + cosPhi*y3 - sinPhi*cosTheta*z3;
  double d2 = sqrt(x3p*x3p + y3p*y3p);

  double cosAlpha, sinAlpha;
  if (d2 < VTK_AXIS_EPSILON) 
    {
    cosAlpha = 1.0;
    sinAlpha = 0.0;
    }
  else 
    {
    cosAlpha = y3p/d2;
    sinAlpha = x3p/d2;
    }

  double alpha = atan2(sinAlpha, cosAlpha);
  orientation[2] = alpha/vtkMath::DoubleDegreesToRadians();
}

//----------------------------------------------------------------------------
// vtkTransform::GetOrientationWXYZ 
void vtkTransform::GetOrientationWXYZ(double wxyz[4])
{
  double matrix[4][4]; // for local manipulation
  vtkMatrix4x4::DeepCopy(*matrix,this->Matrix);

  // get scale factors
  double scale[3];
  this->GetScale(scale);
  for (int i = 0; i < 3; i++)
    {
    if (scale[i] != 1.0)
      {
      matrix[0][i] /= scale[i];
      matrix[1][i] /= scale[i];
      matrix[2][i] /= scale[i];
      }
    }

  // create the quaternion
  double quat[4];
  quat[0] = 0.25*(1.0 + matrix[0][0] + matrix[1][1] + matrix[2][2]);

  if (quat[0] > 0.0001)
    {
    quat[0] = sqrt(quat[0]);
    quat[1] = (matrix[2][1] - matrix[1][2])/(4.0*quat[0]);
    quat[2] = (matrix[0][2] - matrix[2][0])/(4.0*quat[0]);
    quat[3] = (matrix[1][0] - matrix[0][1])/(4.0*quat[0]);
    }
  else
    {
    quat[0] = 0;
    quat[1] = -0.5*(matrix[1][1] + matrix[2][2]);

    if (quat[1] > 0.0001)
      {
      quat[1] = sqrt(quat[1]);
      quat[2] = matrix[1][0]/(2.0*quat[1]);
      quat[3] = matrix[2][0]/(2.0*quat[1]);
      }
    else
      {
      quat[1] = 0;
      quat[2] = 0.5*(1.0 - matrix[2][2]);
      if (quat[2] > 0.0001)
        {
        quat[2] = sqrt(quat[2]);
        quat[3] = matrix[2][1]/(2.0*quat[2]);
        }
      else
        {
        quat[2] = 0;
        quat[3] = 1;
        }
      }
    }
  
  // calc the return value wxyz
 double mag = sqrt(quat[1]*quat[1] + quat[2]*quat[2] + quat[3]*quat[3]);

  if (mag)
    {
    wxyz[0] = 2.0*acos(quat[0])/vtkMath::DoubleDegreesToRadians();
    wxyz[1] = quat[1]/mag;
    wxyz[2] = quat[2]/mag;
    wxyz[3] = quat[3]/mag;
    }
  else
    {
    wxyz[0] = 0.0;
    wxyz[1] = 0.0;
    wxyz[2] = 0.0;
    wxyz[3] = 1.0;
    }
}


//----------------------------------------------------------------------------
// Return the position from the current transformation matrix as an array
// of three floating point numbers. This is simply returning the translation 
// component of the 4x4 matrix.
void vtkTransform::GetPosition(double position[3])
{
  position[0] = this->Matrix->Element[0][3];
  position[1] = this->Matrix->Element[1][3];
  position[2] = this->Matrix->Element[2][3];
}

//----------------------------------------------------------------------------
// Return the x, y, z scale factors of the current transformation matrix as 
// an array of three float numbers.
void vtkTransform::GetScale(double scale[3])
{
  // convenient access to matrix
  double (*matrix)[4] = this->Matrix->Element;

  for (int i = 0; i < 3; i++) 
    {
    scale[i] = sqrt(matrix[0][i]*matrix[0][i] +
		    matrix[1][i]*matrix[1][i] +
		    matrix[2][i]*matrix[2][i]);
    }
}

//----------------------------------------------------------------------------
// Returns the result of multiplying the currently set Point by the current 
// transformation matrix. Point is expressed in homogeneous coordinates.
// The setting of the PreMultiplyFlag will determine if the Point is
// Pre or Post multiplied.
//
// These methods are obsolete.
float *vtkTransform::GetPoint()
{
  if (this->PreMultiplyFlag)
    {
    this->Matrix->PointMultiply(this->Point,this->Point);
    }
  else
    {
    this->Matrix->MultiplyPoint(this->Point,this->Point);
    }
  return this->Point;
}

//----------------------------------------------------------------------------
double *vtkTransform::GetDoublePoint()
{
  if (this->PreMultiplyFlag)
    {
    this->Matrix->PointMultiply(this->DoublePoint,this->DoublePoint);
    }
  else
    {
    this->Matrix->MultiplyPoint(this->DoublePoint,this->DoublePoint);
    }
  return this->DoublePoint;
}

//----------------------------------------------------------------------------
void vtkTransform::GetPoint(float p[4])
{
  float *x = this->vtkTransform::GetPoint();
  for (int i = 0; i < 4; i++)
    {
    p[i] = x[i];
    }
}
