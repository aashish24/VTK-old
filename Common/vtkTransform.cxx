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
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include <stdlib.h>
#include "vtkTransform.h"
#include "vtkGeneralTransformInverse.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"

// Useful for viewing a double[16] as a double[4][4]
typedef double (*SqMatPtr)[4];


//------------------------------------------------------------------------------
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


// Constructs a transform and sets the following defaults
// preMultiplyFlag = 1 stackSize = 10. It then
// creates an identity matrix as the top matrix on the stack.
vtkTransform::vtkTransform ()
{
  // we call this a matrix4x3 transform because the bottom
  // row of the 4x4 matrix is ignored
  this->TransformType = VTK_MATRIX4X3_TRANSFORM;

  this->PreMultiplyFlag = 1;
  this->StackSize = 10;
  this->Stack = new vtkMatrix4x4 *[this->StackSize];
  *this->Stack = vtkMatrix4x4::New();
  this->StackBottom = this->Stack;
  this->Identity();

  this->Point[0] = this->Point[1] = this->Point[2] = this->Point[3] = 0.0;
  this->DoublePoint[0] = 
    this->DoublePoint[1] = this->DoublePoint[2] = this->DoublePoint[3] = 0.0;
}

// Copy constructor. Creates an instance of vtkTransform and then
// copies its instance variables from the values in t. 
vtkTransform::vtkTransform (const vtkTransform& t)
{
  int i, n;

  this->PreMultiplyFlag = t.PreMultiplyFlag;
  this->StackSize = t.StackSize;
  this->StackBottom = new vtkMatrix4x4 *[this->StackSize];

  // now copy each matrix in the stack
  n = t.Stack-t.StackBottom+1;
  for (i = 0; i < n; i++)
    {
    this->StackBottom[i] = vtkMatrix4x4::New();
    (this->StackBottom[i])->DeepCopy(t.Stack[i]);
    }

  this->Stack = this->StackBottom + (n - 1);
}

// support for the vtkGeneralTransform superclass
void vtkTransform::TransformPoints(vtkPoints *inPts, vtkPoints *outPts)
{
  this->MultiplyPoints(inPts,outPts);
}

// support for the vtkGeneralTransform superclass
void vtkTransform::TransformNormals(vtkPoints *vtkNotUsed(inPts), 
				    vtkPoints *vtkNotUsed(outPts),
				    vtkNormals *inNms, vtkNormals *outNms)
{
  this->MultiplyNormals(inNms,outNms);
}

// support for the vtkGeneralTransform superclass
void vtkTransform::TransformVectors(vtkPoints *vtkNotUsed(inPts), 
				    vtkPoints *vtkNotUsed(outPts),
				    vtkVectors *inVrs, vtkVectors *outVrs)
{
  this->MultiplyVectors(inVrs,outVrs);
}

// support for the vtkGeneralTransform superclass
void vtkTransform::TransformPoint(const double in[3], double out[3])
{
  double x = in[0];
  double y = in[1];
  double z = in[2];

  double (*matrix)[4] = (**this->Stack).Element;

  out[0] = matrix[0][0]*x + matrix[0][1]*y + matrix[0][2]*z + matrix[0][3];
  out[1] = matrix[1][0]*x + matrix[1][1]*y + matrix[1][2]*z + matrix[1][3];
  out[2] = matrix[2][0]*x + matrix[2][1]*y + matrix[2][2]*z + matrix[2][3];
}

// support for the vtkGeneralTransform superclass
void vtkTransform::TransformPoint(const float in[3], float out[3])
{
  float x = in[0];
  float y = in[1];
  float z = in[2];

  double (*matrix)[4] = (**this->Stack).Element;

  out[0] = matrix[0][0]*x + matrix[0][1]*y + matrix[0][2]*z + matrix[0][3];
  out[1] = matrix[1][0]*x + matrix[1][1]*y + matrix[1][2]*z + matrix[1][3];
  out[2] = matrix[2][0]*x + matrix[2][1]*y + matrix[2][2]*z + matrix[2][3];
}

// support for the vtkGeneralTransform superclass
vtkGeneralTransform *vtkTransform::MakeTransform()
{
  return vtkTransform::New();
}

// support for the vtkGeneralTransform superclass
void vtkTransform::DeepCopy(vtkGeneralTransform *transform)
{
  if (transform->GetTransformType() & VTK_INVERSE_TRANSFORM)
    {
    transform = 
      ((vtkGeneralTransformInverse *)transform)->GetInverseTransform(); 
    }	
  if (this->TransformType != transform->GetTransformType())
    {
    vtkErrorMacro(<< "DeepCopy: trying to copy a transform of different type");
    }
  vtkTransform *t = (vtkTransform *)transform;  

  if (t == this)
    {
    return;
    }

  int i, n;

  this->PreMultiplyFlag = t->PreMultiplyFlag;
  this->StackSize = t->StackSize;
  // free old memory
  if (this->StackBottom)
    {
    for (n=this->Stack-this->StackBottom+1, i=0; i < n; i++)
      {
      this->StackBottom[i]->Delete();
      }
    delete [] this->StackBottom;
    } 
  this->StackBottom = new vtkMatrix4x4 *[this->StackSize];
  for ( n=t->Stack-t->StackBottom+1, i=0; i < n; i++ )
    {
    this->StackBottom[i] = vtkMatrix4x4::New();
    (this->StackBottom[i])->DeepCopy(t->Stack[i]);
    }
  this->Stack = this->StackBottom + (n - 1);

  for ( i=0; i < 3; i++)
    {
    this->Point[i] = t->Point[i];
    }

  this->Modified();
}

// Deletes the transformation on the top of the stack and sets the top 
// to the next transformation on the stack.
void vtkTransform::Pop ()
{
  // if we're at the bottom of the stack, don't pop
  if (this->Stack == this->StackBottom)
    {
    return;
    }

  // free the stack matrix storage
  (*this->Stack)->Delete();
  *this->Stack = NULL;

  // update the stack
  this->Stack--;

  this->Modified ();
}

// Sets the internal state of the transform to
// post multiply. All subsequent matrix
// operations will occur after those already represented
// in the current transformation matrix.
void vtkTransform::PostMultiply ()
{
  if (this->PreMultiplyFlag != 0) 
    {
    this->PreMultiplyFlag = 0;
    this->Modified ();
    }
}

// Sets the internal state of the transform to
// pre multiply. All subsequent matrix
// operations will occur before those already represented
// in the current transformation matrix.
void vtkTransform::PreMultiply ()
{
  if (this->PreMultiplyFlag != 1) 
    {
    this->PreMultiplyFlag = 1;
    this->Modified ();
    }
}

// Pushes the current transformation matrix onto the
// transformation stack.
void vtkTransform::Push ()
{
  vtkMatrix4x4 *ctm;

  ctm = *this->Stack;
  this->Stack++;
  if ((this->Stack - this->StackBottom) > this->StackSize) 
    {
    this->Stack--;
    vtkErrorMacro(<<"Exceeded matrix stack size");
    ctm->Delete();
    return;
    }

  // allocate a new matrix on the stack
  (*this->Stack) = vtkMatrix4x4::New();
  
  // set the new matrix to the previous top of stack matrix
  (*this->Stack)->DeepCopy(ctm);

  this->Modified ();
}

// Creates an x rotation matrix and concatenates it with 
// the current transformation matrix. The angle is specified
// in degrees.
void vtkTransform::RotateX ( float angle)
{
  float radians = angle * vtkMath::DegreesToRadians();
  float cosAngle, sinAngle;
  double ScratchPad[16]; // for passing to vtkMatrix4x4 methods
  SqMatPtr ScratchPadMatrix = (SqMatPtr) ScratchPad; // for local manipulation

  // Reset the scratchpad
  vtkMatrix4x4::Identity(ScratchPad);

  if (angle != 0.0) 
    {
    cosAngle = cos (radians);
    sinAngle = sin (radians);

    ScratchPadMatrix[0][0] = 1.0;
    ScratchPadMatrix[1][1] =  cosAngle;
    ScratchPadMatrix[2][1] =  sinAngle;
    ScratchPadMatrix[1][2] = -sinAngle;
    ScratchPadMatrix[2][2] =  cosAngle;
    ScratchPadMatrix[3][3] = 1.0;

    // concatenate with current transformation matrix
    this->Concatenate (ScratchPad);
    }
}

// Creates a y rotation matrix and concatenates it with
// the current transformation matrix. The angle is specified
// in degrees.
void vtkTransform::RotateY ( float angle)
{
  double ScratchPad[16]; // for passing to vtkMatrix4x4 methods
  SqMatPtr ScratchPadMatrix = (SqMatPtr) ScratchPad; // for local manipulation
  float radians = angle * vtkMath::DegreesToRadians();
  float cosAngle, sinAngle;

  vtkMatrix4x4::Identity(ScratchPad);
  if (angle != 0.0) 
    {
    cosAngle = cos (radians);
    sinAngle = sin (radians);

    ScratchPadMatrix[0][0] = cosAngle;
    ScratchPadMatrix[1][1] = 1.0;
    ScratchPadMatrix[2][0] = -sinAngle;
    ScratchPadMatrix[0][2] = sinAngle;
    ScratchPadMatrix[2][2] = cosAngle;
    ScratchPadMatrix[3][3] = 1.0;

    // concatenate with current transformation matrix
    this->Concatenate (ScratchPad);
    }
}

// Creates a z rotation matrix and concatenates it with
// the current transformation matrix. The angle is specified
// in degrees.
void vtkTransform::RotateZ (float angle)
{
  double ScratchPad[16]; // for passing to vtkMatrix4x4 methods
  SqMatPtr ScratchPadMatrix = (SqMatPtr) ScratchPad; // for local manipulation
  float radians = angle * vtkMath::DegreesToRadians();
  float cosAngle, sinAngle;

  vtkMatrix4x4::Identity(ScratchPad);
  if (angle != 0.0) 
    {
    cosAngle = cos (radians);
    sinAngle = sin (radians);

    ScratchPadMatrix[0][0] =  cosAngle;
    ScratchPadMatrix[1][0] =  sinAngle;
    ScratchPadMatrix[0][1] = -sinAngle;
    ScratchPadMatrix[1][1] =  cosAngle;
    ScratchPadMatrix[2][2] = 1.0;
    ScratchPadMatrix[3][3] = 1.0;

    // concatenate with current transformation matrix
    this->Concatenate (ScratchPad);
    }
}


// Creates a matrix that rotates angle degrees about an axis
// through the origin and x, y, z. It then concatenates
// this matrix with the current transformation matrix.
void vtkTransform::RotateWXYZ ( float angle, float x, float y, float z)
{
  double ScratchPad[16]; // for passing to vtkMatrix4x4 methods
  SqMatPtr ScratchPadMatrix = (SqMatPtr) ScratchPad; // for local manipulation
  float   radians;
  float   w;
  float   quat[4];
  float   sinAngle;
  float   cosAngle;

  vtkMatrix4x4::Identity(ScratchPad);

  // build a rotation matrix and concatenate it
  quat[0] = angle;
  quat[1] = x;
  quat[2] = y;
  quat[3] = z;

  // convert degrees to radians
  radians = quat[0] * vtkMath::DegreesToRadians() / 2;

  cosAngle = cos (radians);
  sinAngle = sin (radians);

  // normalize x, y, z
  if ( vtkMath::Normalize(quat+1) == 0.0 )
    {
    vtkErrorMacro(<<"Trying to rotate around zero-length axis");
    return;
    }

  w = cosAngle;
  x = quat[1] * sinAngle;
  y = quat[2] * sinAngle;
  z = quat[3] * sinAngle;

  // matrix calculation is taken from Ken Shoemake's
  // "Animation Rotation with Quaternion Curves",
  // Comput. Graphics, vol. 19, No. 3 , p. 253

  ScratchPadMatrix[0][0] = 1 - 2 * y * y - 2 * z * z;
  ScratchPadMatrix[1][1] = 1 - 2 * x * x - 2 * z * z;
  ScratchPadMatrix[2][2] = 1 - 2 * x * x - 2 * y * y;
  ScratchPadMatrix[1][0] =  2 * x * y + 2 * w * z;
  ScratchPadMatrix[2][0] =  2 * x * z - 2 * w * y;
  ScratchPadMatrix[0][1] =  2 * x * y - 2 * w * z;
  ScratchPadMatrix[2][1] =  2 * y * z + 2 * w * x;
  ScratchPadMatrix[0][2] =  2 * x * z + 2 * w * y;
  ScratchPadMatrix[1][2] =  2 * y * z - 2 * w * x;

  // concatenate with current transformation matrix
  this->Concatenate (ScratchPad);
}

void vtkTransform::RotateWXYZ ( double angle, double x, double y, double z)
{
  double ScratchPad[16]; // for passing to vtkMatrix4x4 methods
  SqMatPtr ScratchPadMatrix = (SqMatPtr) ScratchPad; // for local manipulation
  double   radians;
  double   w;
  double   quat[4];
  double   sinAngle;
  double   cosAngle;

  vtkMatrix4x4::Identity(ScratchPad);

  // build a rotation matrix and concatenate it
  quat[0] = angle;
  quat[1] = x;
  quat[2] = y;
  quat[3] = z;

  // convert degrees to radians
  radians = quat[0] * vtkMath::DegreesToRadians() / 2;

  cosAngle = cos (radians);
  sinAngle = sin (radians);

  // normalize x, y, z
  if ( vtkMath::Normalize(quat+1) == 0.0 )
    {
    vtkErrorMacro(<<"Trying to rotate around zero-length axis");
    return;
    }

  w = cosAngle;
  x = quat[1] * sinAngle;
  y = quat[2] * sinAngle;
  z = quat[3] * sinAngle;

  // matrix calculation is taken from Ken Shoemake's
  // "Animation Rotation with Quaternion Curves",
  // Comput. Graphics, vol. 19, No. 3 , p. 253

  ScratchPadMatrix[0][0] = 1 - 2 * y * y - 2 * z * z;
  ScratchPadMatrix[1][1] = 1 - 2 * x * x - 2 * z * z;
  ScratchPadMatrix[2][2] = 1 - 2 * x * x - 2 * y * y;
  ScratchPadMatrix[1][0] =  2 * x * y + 2 * w * z;
  ScratchPadMatrix[2][0] =  2 * x * z - 2 * w * y;
  ScratchPadMatrix[0][1] =  2 * x * y - 2 * w * z;
  ScratchPadMatrix[2][1] =  2 * y * z + 2 * w * x;
  ScratchPadMatrix[0][2] =  2 * x * z + 2 * w * y;
  ScratchPadMatrix[1][2] =  2 * y * z - 2 * w * x;

  // concatenate with current transformation matrix
  this->Concatenate (ScratchPad);
}

// Scales the current transformation matrix in the x, y and z directions.
// A scale factor of zero will automatically be replaced with one.
void vtkTransform::Scale ( float x, float y, float z)
{
  double ScratchPad[16]; // for passing to vtkMatrix4x4 methods
  SqMatPtr ScratchPadMatrix = (SqMatPtr) ScratchPad; // for local manipulation
  
  vtkMatrix4x4::Identity(ScratchPad);
  if (x != 1.0 || y != 1.0 || z != 1.0) 
    {
    ScratchPadMatrix[0][0] = x;
    ScratchPadMatrix[1][1] = y;
    ScratchPadMatrix[2][2] = z;

    // concatenate with current transformation matrix
    this->Concatenate (ScratchPad);
    }
}

// Scales the current transformation matrix in the x, y and z directions.
// A scale factor of zero will automatically be replaced with one.
void vtkTransform::Scale ( double x, double y, double z)
{
  double ScratchPad[16]; // for passing to vtkMatrix4x4 methods
  SqMatPtr ScratchPadMatrix = (SqMatPtr) ScratchPad; // for local manipulation

  vtkMatrix4x4::Identity(ScratchPad);
  if (x != 1.0 || y != 1.0 || z != 1.0) 
    {
    ScratchPadMatrix[0][0] = x;
    ScratchPadMatrix[1][1] = y;
    ScratchPadMatrix[2][2] = z;

    // concatenate with current transformation matrix
    this->Concatenate (ScratchPad);
    }
}

// Translate the current transformation matrix by the vector {x, y, z}.
void vtkTransform::Translate ( float x, float y, float z)
{
  double ScratchPad[16]; // for passing to vtkMatrix4x4 methods
  SqMatPtr ScratchPadMatrix = (SqMatPtr) ScratchPad; // for local manipulation

  vtkMatrix4x4::Identity(ScratchPad);
  if (x != 0.0 || y != 0.0 || z != 0.0) 
    {
    ScratchPadMatrix[0][3] = x;
    ScratchPadMatrix[1][3] = y;
    ScratchPadMatrix[2][3] = z;

    // concatenate with current transformation matrix
    this->Concatenate (ScratchPad);
    }
}

// Translate the current transformation matrix by the vector {x, y, z}.
void vtkTransform::Translate ( double x, double y, double z)
{
  double ScratchPad[16]; // for passing to vtkMatrix4x4 methods
  SqMatPtr ScratchPadMatrix = (SqMatPtr) ScratchPad; // for local manipulation
  vtkMatrix4x4::Identity(ScratchPad);
  if (x != 0.0 || y != 0.0 || z != 0.0) 
    {
    ScratchPadMatrix[0][3] = x;
    ScratchPadMatrix[1][3] = y;
    ScratchPadMatrix[2][3] = z;

    // concatenate with current transformation matrix
    this->Concatenate (ScratchPad);
    }
}

// Obtain the transpose of the current transformation matrix.
void vtkTransform::GetTranspose (vtkMatrix4x4 *transpose)
{
  double ScratchPad[16]; // for passing to vtkMatrix4x4 methods
  SqMatPtr ScratchPadMatrix = (SqMatPtr) ScratchPad; // for local manipulation
  int i, j;

  for (i = 0; i < 4; i++) 
    {
    for (j = 0; j < 4; j++) 
      {
      ScratchPadMatrix[j][i] = (**this->Stack).Element[i][j];
      }
    } 

  // Copy the result
  for (i = 0; i < 4; i++) 
    {
    for (j = 0; j < 4; j++) 
      {
      (*transpose).Element[i][j] = ScratchPadMatrix[i][j];
      }
    } 

}

// TEMP -- this method needs to be fixed to use the new vtkMatrix4x4 
// style calls. Wait, maybe not. Check on it. --CRV
// Invert the current transformation matrix.
void vtkTransform::Inverse ()
{
  (*this->Stack)->Invert(*this->Stack, *this->Stack);

  this->Modified ();
}

// Return the inverse of the current transformation matrix.
void vtkTransform::GetInverse ( vtkMatrix4x4 *inverse)
{
  inverse->Invert(*this->Stack, inverse);
}

// Get the x, y, z orientation angles from the transformation matrix.
void vtkTransform::GetOrientation(float *prx, float *pry, float *prz)
{
  float *orientation=this->GetOrientation();
  *prx = orientation[0];
  *pry = orientation[1];
  *prz = orientation[2];
}

// Get the x, y, z orientation angles from the transformation matrix as an
// array of three floating point values.
float *vtkTransform::GetOrientation ()
{
  double ScratchPad[16]; // for passing to vtkMatrix4x4 methods
  SqMatPtr ScratchPadMatrix = (SqMatPtr) ScratchPad; // for local manipulation
#define VTK_AXIS_EPSILON 0.001
  float	scaleX, scaleY, scaleZ;
  float   x,y,z;
  float   d;
  float   d1;
  float   d2;
  float   dot;
  float   alpha;
  float   phi;
  float   theta;
  float   cosPhi, sinPhi;
  float   cosTheta, sinTheta;
  float   cosAlpha, sinAlpha;
  float   x2, y2, z2;
  float   x3, y3, z3;
  float   x3p, y3p;

  // copy the matrix into local storage
  vtkMatrix4x4::DeepCopy(ScratchPad,*this->Stack);

  // get scale factors
  this->GetScale (scaleX, scaleY, scaleZ);

  // first rotate about y axis
  x2 = ScratchPadMatrix[2][0] / scaleX;
  y2 = ScratchPadMatrix[2][1] / scaleY;
  z2 = ScratchPadMatrix[2][2] / scaleZ;

  x3 = ScratchPadMatrix[1][0] / scaleX;
  y3 = ScratchPadMatrix[1][1] / scaleY;
  z3 = ScratchPadMatrix[1][2] / scaleZ;

  dot = x2 * x2 + z2 * z2;
  d1 = sqrt (dot);

  if (d1 < VTK_AXIS_EPSILON) 
    {
    cosTheta = 1.0;
    sinTheta = 0.0;
    }
  else 
    {
    cosTheta = z2 / d1;
    sinTheta = x2 / d1;
    }

  theta = atan2 (sinTheta, cosTheta);

  y = -theta / vtkMath::DegreesToRadians();

  // now rotate about x axis
  dot = x2 * x2 + y2 * y2 + z2 * z2;
  d = sqrt (dot);

  if (d < VTK_AXIS_EPSILON) 
    {
    sinPhi = 0.0;
    cosPhi = 1.0;
    }
  else if (d1 < VTK_AXIS_EPSILON) 
    {
    sinPhi = y2 / d;
    cosPhi = z2 / d;
    }
  else 
    {
    sinPhi = y2 / d;
    cosPhi = ( x2 * x2 + z2 * z2) / (d1 * d);
    }

  phi = atan2 (sinPhi, cosPhi);

  x = phi / vtkMath::DegreesToRadians();

  // finally, rotate about z
  x3p = x3 * cosTheta - z3 * sinTheta;
  y3p = - sinPhi * sinTheta * x3 + cosPhi * y3 - sinPhi * cosTheta * z3;
  dot = x3p * x3p + y3p * y3p;

  d2 = sqrt (dot);
  if (d2 < VTK_AXIS_EPSILON) 
    {
    cosAlpha = 1.0;
    sinAlpha = 0.0;
    }
  else 
    {
    cosAlpha = y3p / d2;
    sinAlpha = x3p / d2;
    }

  alpha = atan2 (sinAlpha, cosAlpha);

  z = alpha / vtkMath::DegreesToRadians();

  this->ReturnValue[0] = x;
  this->ReturnValue[1] = y;
  this->ReturnValue[2] = z;
  this->ReturnValue[3] = 0.0;

  return this->ReturnValue;
}


// Return the x, y, z positions from the current transformation matrix.
// This is simply returning the translation component of the 4x4 matrix.
void vtkTransform::GetPosition (float *px, float *py, float *pz)
{
  *px = (**this->Stack).Element[0][3];
  *py = (**this->Stack).Element[1][3];
  *pz = (**this->Stack).Element[2][3];
}

// vtkTransform::GetOrientationWXYZ 
float *vtkTransform::GetOrientationWXYZ ()
{
  double ScratchPad[16]; // for passing to vtkMatrix4x4 methods
  SqMatPtr ScratchPadMatrix = (SqMatPtr) ScratchPad; // for local manipulation
  float	scaleX, scaleY, scaleZ;
  double temp1[16];
  SqMatPtr temp1Matrix = (SqMatPtr) temp1;
  float quat[4];
  float mag;
  
  // get scale factors
  this->GetScale (scaleX, scaleY, scaleZ);
  if (scaleX != 1.0f || scaleY != 1.0f || scaleZ != 1.0f) 
    {
    vtkMatrix4x4::Identity(temp1);
    temp1Matrix[0][0] = 1.0 / scaleX;
    temp1Matrix[1][1] = 1.0 / scaleY;
    temp1Matrix[2][2] = 1.0 / scaleZ;
    vtkTransform::Multiply4x4(&(*this->Stack)->Element[0][0], temp1, ScratchPad);
    }
  else
    {
    ScratchPadMatrix = (SqMatPtr)&(*this->Stack)->Element[0][0];
    }
  
  quat[0] = 0.25*(1.0 + ScratchPadMatrix[0][0] + 
		  ScratchPadMatrix[1][1] 
		  + ScratchPadMatrix[2][2]);

  if (quat[0] > 0.0001)
    {
    quat[0] = sqrt(quat[0]);
    quat[1] = (ScratchPadMatrix[2][1] - 
	       ScratchPadMatrix[1][2])/(4.0*quat[0]);
    quat[2] = (ScratchPadMatrix[0][2] - 
	       ScratchPadMatrix[2][0])/(4.0*quat[0]);
    quat[3] = (ScratchPadMatrix[1][0] - 
	       ScratchPadMatrix[0][1])/(4.0*quat[0]);
    }
  else
    {
    quat[0] = 0;
    quat[1] = -0.5*(ScratchPadMatrix[1][1] + 
		    ScratchPadMatrix[2][2]);
    if (quat[1] > 0.0001)
      {
      quat[1] = sqrt(quat[1]);
      quat[2] = ScratchPadMatrix[1][0]/(2.0*quat[1]);
      quat[3] = ScratchPadMatrix[2][0]/(2.0*quat[1]);
      }
    else
      {
      quat[1] = 0;
      quat[2] = 0.5*(1.0 - ScratchPadMatrix[2][2]);
      if (quat[2] > 0.0001)
        {
        quat[2] = sqrt(quat[2]);
        quat[3] = ScratchPadMatrix[2][1]/(2.0*quat[2]);
        }
      else
        {
        quat[2] = 0;
        quat[3] = 1;
        }
      }
    }
  
  // calc the return value wxyz
  mag = sqrt(quat[1]*quat[1] + quat[2]*quat[2] + quat[3]*quat[3]);

  if (mag)
    {
    this->ReturnValue[0] = 180.0*2.0*acos(quat[0])/3.1415926;
    this->ReturnValue[1] = quat[1]/mag;
    this->ReturnValue[2] = quat[2]/mag;
    this->ReturnValue[3] = quat[3]/mag;
    }
  else
    {
    this->ReturnValue[0] = 0;
    this->ReturnValue[1] = 0;
    this->ReturnValue[2] = 0;
    this->ReturnValue[3] = 1;
    }
  
  return this->ReturnValue;

} // vtkTransform::GetOrientationWXYZ 


// Return the position from the current transformation matrix as an array
// of three floating point numbers. This is simply returning the translation 
// component of the 4x4 matrix.
float *vtkTransform::GetPosition()
{
  this->ReturnValue[0] = (**this->Stack).Element[0][3];
  this->ReturnValue[1] = (**this->Stack).Element[1][3];
  this->ReturnValue[2] = (**this->Stack).Element[2][3];
  this->ReturnValue[3] = 0.0;

  return this->ReturnValue;
}

// Return the x, y, z scale factors of the current transformation matrix.
void vtkTransform::GetScale (float *px, float *py, float *pz)
{
  float *scale=this->GetScale();

  *px = scale[0];
  *py = scale[1];
  *pz = scale[2];
}

// Return the x, y, z scale factors of the current transformation matrix as 
// an array of three float numbers.
float *vtkTransform::GetScale()
{
  int	i;
  vtkMatrix4x4 *temp;

  temp = *this->Stack;

  // find scale factors

  for (i = 0; i < 3; i++) 
    {
    this->ReturnValue[i] = sqrt (temp->Element[0][i] * temp->Element[0][i] +
				 temp->Element[1][i] * temp->Element[1][i] +
				 temp->Element[2][i] * temp->Element[2][i]);
    }
  this->ReturnValue[3] = 0.0;

  return this->ReturnValue;
}

// Returns the current transformation matrix.
vtkMatrix4x4 *vtkTransform::GetMatrixPointer()
{
  return *this->Stack;
}

// Set the current matrix directly.
void vtkTransform::SetMatrix(vtkMatrix4x4 &m)
{
  (*this->Stack)->DeepCopy(&m);
}

// Set the current matrix directly.
void vtkTransform::SetMatrix(double Elements[16])
{
  (*this->Stack)->DeepCopy(Elements);
}

// Creates an identity matrix and makes it the current transformation matrix.
void vtkTransform::Identity()
{
  (*this->Stack)->Identity();
  this->Modified();
}

// Concatenates the input matrix with the current transformation matrix.
// The resulting matrix becomes the new current transformation matrix.
// The setting of the PreMultiply flag determines whether the matrix
// is PreConcatenated or PostConcatenated.
void vtkTransform::Concatenate (vtkMatrix4x4 *matrix)
{
  if (this->PreMultiplyFlag) 
    {
    this->Multiply4x4 (*this->Stack, matrix, *this->Stack);
    }
  else 
    {
    this->Multiply4x4 (matrix, *this->Stack, *this->Stack);
    }
  this->Modified ();
}

void vtkTransform::Concatenate(double Elements[16])
{
  if (this->PreMultiplyFlag) 
    {
    this->Multiply4x4 (&(*this->Stack)->Element[0][0], Elements, 
		       &(*this->Stack)->Element[0][0]);
    }
  else 
    {
    this->Multiply4x4 (Elements, &(*this->Stack)->Element[0][0], 
		       &(*this->Stack)->Element[0][0]);
    }
  (*this->Stack)->Modified();
  this->Modified ();
}

// Multiplies matrices a and b and stores the result in c.
void vtkTransform::Multiply4x4(vtkMatrix4x4 *a, vtkMatrix4x4 *b, 
			       vtkMatrix4x4 *c)
{
  int i, k;
  double Accum[4][4];

  for (i = 0; i < 4; i++) 
    {
    for (k = 0; k < 4; k++) 
      {
      Accum[i][k] = a->Element[i][0] * b->Element[0][k] +
        a->Element[i][1] * b->Element[1][k] +
        a->Element[i][2] * b->Element[2][k] +
        a->Element[i][3] * b->Element[3][k];
      }
    }

  // Copy to final dest
  for (i = 0; i < 4; i++)
    {
    c->Element[i][0] = Accum[i][0];
    c->Element[i][1] = Accum[i][1];
    c->Element[i][2] = Accum[i][2];
    c->Element[i][3] = Accum[i][3];
    }
  c->Modified();
}

// Multiplies matrices a and b and stores the result in c.
void vtkTransform::Multiply4x4(double a[16], double b[16], double c[16])
{
  SqMatPtr aMat = (SqMatPtr) a;
  SqMatPtr bMat = (SqMatPtr) b;
  SqMatPtr cMat = (SqMatPtr) c;
  int i, k;
  double Accum[4][4];

  for (i = 0; i < 4; i++) 
    {
    for (k = 0; k < 4; k++) 
      {
      Accum[i][k] = aMat[i][0] * bMat[0][k] +
                    aMat[i][1] * bMat[1][k] +
                    aMat[i][2] * bMat[2][k] +
                    aMat[i][3] * bMat[3][k];
      }
    }

  // Copy to final dest
  for (i = 0; i < 4; i++)
    {
    cMat[i][0] = Accum[i][0];
    cMat[i][1] = Accum[i][1];
    cMat[i][2] = Accum[i][2];
    cMat[i][3] = Accum[i][3];
    }

}

// Transposes the current transformation matrix.
void vtkTransform::Transpose ()
{
  this->GetTranspose(*this->Stack);
  this->Modified();
}

// Returns the current transformation matrix.
void vtkTransform::GetMatrix (vtkMatrix4x4 *ctm)
{
  ctm->DeepCopy(*this->Stack);
}

vtkTransform::~vtkTransform ()
{
  int i, n;

  for (n=this->Stack-this->StackBottom+1, i=0; i < n; i++)
    {
    this->StackBottom[i]->Delete();
    }

  delete [] this->StackBottom;
}

void vtkTransform::PrintSelf (ostream& os, vtkIndent indent)
{
  vtkGeneralTransform::PrintSelf(os, indent);

  os << indent << (this->PreMultiplyFlag ? "PreMultiply\n" : "PostMultiply\n");

  os << indent << "DoublePoint: " << "( " << 
     this->DoublePoint[0] << ", " << this->DoublePoint[1] << ", " <<
     this->DoublePoint[2] << ", " << this->DoublePoint[3] << ")\n";

  os << indent << "Point: " << "( " << 
     this->Point[0] << ", " << this->Point[1] << ", " <<
     this->Point[2] << ", " << this->Point[3] << ")\n";

  os << indent << "Current Transformation:" << "\n";

  (**this->Stack).PrintSelf (os, indent.GetNextIndent());
}

// Returns the result of multiplying the currently set Point by the current 
// transformation matrix. Point is expressed in homogeneous coordinates.
// The setting of the PreMultiplyFlag will determine if the Point is
// Pre or Post multiplied.
float *vtkTransform::GetPoint()
{
  if (this->PreMultiplyFlag)
    {
    this->Stack[0]->PointMultiply(this->Point,this->Point);
    }
  else
    {
    this->Stack[0]->MultiplyPoint(this->Point,this->Point);
    }
  return this->Point;
}

double *vtkTransform::GetDoublePoint()
{
  if (this->PreMultiplyFlag)
    {
    this->Stack[0]->PointMultiply(this->DoublePoint,this->DoublePoint);
    }
  else
    {
    this->Stack[0]->MultiplyPoint(this->DoublePoint,this->DoublePoint);
    }
  return this->DoublePoint;
}

void vtkTransform::GetPoint(float p[4])
{
  float *x=this->vtkTransform::GetPoint();
  for (int i=0; i<4; i++)
    {
    p[i] = x[i];
    }
}

// Multiplies a list of points (inPts) by the current transformation matrix.
// Transformed points are appended to the output list (outPts).
void vtkTransform::MultiplyPoints(vtkPoints *inPts, vtkPoints *outPts)
{
  float newX[3];
  float *x;
  int ptId, i;
  int numPts = inPts->GetNumberOfPoints();

  for (ptId=0; ptId < numPts; ptId++)
    {
    x = inPts->GetPoint(ptId);
    for (i=0; i<3; i++)
      {
      newX[i] = (**this->Stack).Element[i][0] * x[0] +
                (**this->Stack).Element[i][1] * x[1] +
                (**this->Stack).Element[i][2] * x[2] +
                (**this->Stack).Element[i][3];
      }

    outPts->InsertNextPoint(newX);
    }
}

// Multiplies a list of vectors (inVectors) by the current transformation 
// matrix. The transformed vectors are appended to the output list 
// (outVectors). The translational component of the matrix is ignored. 
void vtkTransform::MultiplyVectors(vtkVectors *inVectors, 
				   vtkVectors *outVectors)
{
   float newV[3];
   float *v;
   int ptId, i;
   int numVectors = inVectors->GetNumberOfVectors();

   for (ptId=0; ptId < numVectors; ptId++)
     {
     v = inVectors->GetVector(ptId);
     for (i=0; i<3; i++)
       {
       newV[i] = (**this->Stack).Element[i][0] * v[0] +
                 (**this->Stack).Element[i][1] * v[1] +
                 (**this->Stack).Element[i][2] * v[2];
       }

     outVectors->InsertNextVector(newV);
     }
}

// Multiplies a list of normals (inNormals) by the current
// transformation matrix.  The transformed normals are then appended
// to the output list (outNormals).  This is a special multiplication,
// since these are normals. It multiplies the normals by the
// transposed inverse of the matrix, ignoring the translational
// components.
void vtkTransform::MultiplyNormals(vtkNormals *inNormals, vtkNormals *outNormals)
{
  double ScratchPad[16]; // for passing to vtkMatrix4x4 methods
  SqMatPtr ScratchPadMatrix = (SqMatPtr) ScratchPad; // for local manipulation
  float newN[3];
  float *n;
  int ptId, i;
  int numNormals = inNormals->GetNumberOfNormals();

  vtkMatrix4x4::DeepCopy(ScratchPad, *this->Stack);
  vtkMatrix4x4::Invert (ScratchPad,ScratchPad);
  vtkMatrix4x4::Transpose (ScratchPad,ScratchPad);
  
  for (ptId=0; ptId < numNormals; ptId++)
    {
    n = inNormals->GetNormal(ptId);
    for (i=0; i<3; i++)
      {
      newN[i] = ScratchPadMatrix[i][0] * n[0] +
                ScratchPadMatrix[i][1] * n[1] +
                ScratchPadMatrix[i][2] * n[2];
      }

    vtkMath::Normalize(newN);
    outNormals->InsertNextNormal(newN);
    }
}

