/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Library. No part of this file or its
contents may be copied, reproduced or altered in any way without the express
written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994

=========================================================================*/
#include <stdlib.h>
#include <math.h>

#include "Trans.hh"

vlTransform::vlTransform ()
{
  // pre multiply is on
  this->PreMultiplyFlag = 1;
  // create a reasonable size stack
  this->StackSize = 10;
  // allocate the stack
  this->Stack = new vlMatrix4x4 *[this->StackSize];
  // put a matrix on the top
  *this->Stack = new vlMatrix4x4;
  // initialize the bottom of the stack
  this->StackBottom = this->Stack;
  // initialize current matrix to identity
  this->Identity ();

  this->Modified ();
}

void vlTransform::Pop ()
  //  Deletes the transformation on the top of the
  //  stack and sets the top to the next transformation
  //  on the stack.
{
  // if we're at the bottom of the stack, don't pop
  if (this->Stack == this->StackBottom) return;

  // free the stack matrix storage
  delete *this->Stack;
  *this->Stack = 0;

  // update the stack
  this->Stack--;

  this->Modified ();
}

void vlTransform::PostMultiply ()
  //  Sets the internal state of the transform to
  //  post multiply. All matrix subsequent matrix
  //  opeartions will occur after those already represented
  //  in the current transformation matrix.
{
  if (this->PreMultiplyFlag != 0) {
    this->PreMultiplyFlag = 0;
    this->Modified ();
  }
}

void vlTransform::PreMultiply ()
  //  Sets the internal state of the transform to
  //  pre multiply. All matrix subsequent matrix
  //  opeartions will occur before those already represented
  //  in the current transformation matrix.
{
  if (this->PreMultiplyFlag != 1) {
    this->PreMultiplyFlag = 1;
    this->Modified ();
  }
}

void vlTransform::Push ()
  //  Pushes the current transformation matrix onto the
  //  transformation stack.
{
  vlMatrix4x4 ctm;

  ctm = **this->Stack;
  this->Stack++;
  if ((this->Stack - this->StackBottom) > this->StackSize) {
    this->Stack--;
    return;
  }
  // allocate a new matrix on the stack

  *this->Stack = new vlMatrix4x4;

  // set the new matrix to the previous top of stack matrix
  **this->Stack = ctm;

  this->Modified ();
}
# define RADIANS_PER_DEGREE     .017453292

void vlTransform::RotateX ( float angle)
  //  Creates an x rotation matrix andn concatenates it with 
  //  the current transformation matrix.
{
  vlMatrix4x4 ctm;

  int i, j;
  float radians = angle * RADIANS_PER_DEGREE;
  float cos_angle, sin_angle;

  if (angle != 0.0) {
    cos_angle = cos (radians);
    sin_angle = sin (radians);

    ctm = 0.0;

    ctm.Element[0][0] = 1.0;
    ctm.Element[1][1] =  cos_angle;
    ctm.Element[2][1] = -sin_angle;
    ctm.Element[1][2] =  sin_angle;
    ctm.Element[2][2] =  cos_angle;
    ctm.Element[3][3] = 1.0;

    // concatenate with current transformation matrix
    this->Concatenate (ctm);
  }
}

void vlTransform::RotateY ( float angle)
  //  Creates a y rotation matrix and concatenates it with 
  //  the current transformation matrix.
{
  vlMatrix4x4 ctm;
  int i, j;
  float radians = angle * RADIANS_PER_DEGREE;
  float cos_angle, sin_angle;

  if (angle != 0.0) {
    cos_angle = cos (radians);
    sin_angle = sin (radians);

    ctm = 0.0;

    ctm.Element[0][0] = cos_angle;
    ctm.Element[1][1] = 1.0;
    ctm.Element[2][0] = sin_angle;
    ctm.Element[0][2] = -sin_angle;
    ctm.Element[2][2] = cos_angle;
    ctm.Element[3][3] = 1.0;

    // concatenate with current transformation matrix
    this->Concatenate (ctm);
  }
}

void vlTransform::RotateZ (float angle)
  //  Creates a z rotation matrix and concatenates it with 
  //  the current transformation matrix.
{
  vlMatrix4x4 ctm;
  int i, j;
  float radians = angle * RADIANS_PER_DEGREE;
  float cos_angle, sin_angle;

  if (angle != 0.0) {
    cos_angle = cos (radians);
    sin_angle = sin (radians);

    ctm = 0.0;

    ctm.Element[0][0] =  cos_angle;
    ctm.Element[1][0] = -sin_angle;
    ctm.Element[0][1] =  sin_angle;
    ctm.Element[1][1] =  cos_angle;
    ctm.Element[2][2] = 1.0;
    ctm.Element[3][3] = 1.0;

    // concatenate with current transformation matrix
    this->Concatenate (ctm);
  }
}

void vlTransform::RotateWXYZ ( float angle, float x, float y, float z)
  //  Creates a matrix that rotates angle degrees about an axis
  //  through the origin and x, y, z. Then concatenates
  //  this matrix with the current transformation matrix.
{
  vlMatrix4x4 ctm;
  float   radians;
  float   w;
  float   sum;
  float   quat[4];
  float   sin_angle;
  float   cos_angle;
  int     i;
  int     j;

  // build a rotation matrix and concatenate it
  quat[0] = angle;
  quat[1] = x;
  quat[2] = y;
  quat[3] = z;

  // convert degrees to radians
  radians = - quat[0] * RADIANS_PER_DEGREE / 2;

  cos_angle = cos (radians);
  sin_angle = sin (radians);

  // normalize x, y, z
  if (sum = quat[1] * quat[1] + quat[2] * quat[2] + quat[3] * quat[3]) 
    {
    quat[1] *= sin_angle / sqrt(sum);
    quat[2] *= sin_angle / sqrt(sum);
    quat[3] *= sin_angle / sqrt(sum);
    }
  else 
    {
    return;
    }

  w = cos_angle;
  x = quat[1];
  y = quat[2];
  z = quat[3];

  ctm = 0.0;

  // matrix calculation is taken from Ken Shoemake's
  // "Animation Rotation with Quaternion Curves",
  // Comput. Graphics, vol. 19, No. 3 , p. 253

  ctm.Element[0][0] = 1 - 2 * y * y - 2 * z * z;
  ctm.Element[1][1] = 1 - 2 * x * x - 2 * z * z;
  ctm.Element[2][2] = 1 - 2 * x * x - 2 * y * y;
  ctm.Element[1][0] =  2 * x * y + 2 * w * z;
  ctm.Element[2][0] =  2 * x * z - 2 * w * y;
  ctm.Element[0][1] =  2 * x * y - 2 * w * z;
  ctm.Element[2][1] =  2 * y * z + 2 * w * x;
  ctm.Element[0][2] =  2 * x * z + 2 * w * y;
  ctm.Element[1][2] =  2 * y * z - 2 * w * x;
  ctm.Element[3][3] = 1.0;

  // concatenate with current transformation matrix
  this->Concatenate (ctm);
}

void vlTransform::Scale ( float x, float y, float z)
{
  vlMatrix4x4 ctm;
  int     i;
  int     j;

  if (x != 1.0 || y != 1.0 || z != 1.0) {
    ctm = 0.0;

    ctm.Element[0][0] = x;
    if (ctm.Element[0][0] == 0.0) {
      vlErrorMacro(<< "scale: x scale is 0.0, reset to 1.0\n");
      ctm.Element[0][0] = 1.0;
    }

    ctm.Element[1][1] = y;
    if (ctm.Element[1][1] == 0.0) {
      vlErrorMacro(<<  "scale: y scale is 0.0, reset to 1.0\n");
      ctm.Element[1][1] = 1.0;
    }

    ctm.Element[2][2] = z;
    if (ctm.Element[2][2] == 0.0) {
      vlErrorMacro(<< "scale: z scale is 0.0, reset to 1.0\n");
      ctm.Element[2][2] = 1.0;
    }

    ctm.Element[3][3] = 1.0;

    // concatenate with current transformation matrix
    this->Concatenate (ctm);
  }
}

void vlTransform::Translate ( float x, float y, float z)
{
  vlMatrix4x4 ctm;
  int i, j;

  if (x != 0.0 || y != 0.0 || z != 0.0) {
    ctm = 0.0;

    ctm.Element[0][0] = 1.0;
    ctm.Element[1][1] = 1.0;
    ctm.Element[2][2] = 1.0;
    ctm.Element[3][3] = 1.0;

    ctm.Element[0][3] = x;
    ctm.Element[1][3] = y;
    ctm.Element[2][3] = z;

    // concatenate with current transformation matrix
    this->Concatenate (ctm);
  }
}

void vlTransform::GetTranspose (vlMatrix4x4& (transpose))
{
  vlMatrix4x4 temp;
  int i, j;

  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      temp.Element[j][i] = (**this->Stack).Element[i][j];
    }
  }    
  transpose = temp;
}

void vlTransform::Inverse ()
{
  (**this->Stack).Invert (**this->Stack, **this->Stack);

  this->Modified ();
}

void vlTransform::GetInverse ( vlMatrix4x4& inverse)
{
  inverse.Invert (**this->Stack, inverse);
}

void vlTransform::GetOrientation ( float & x, float & y, float & z)
{
#define AXIS_EPSILON .01
	float	scale_x, scale_y, scale_z;
	vlMatrix4x4  temp;
	float   d;
	float   d1;
	float   d2;
	float   dot;
	float   alpha;
	float   phi;
	float   theta;
	float   cos_phi, sin_phi;
	float   cos_theta, sin_theta;
	float   cos_alpha, sin_alpha;
	float   x2, y2, z2;
	float   x3, y3, z3;
	float   x3p, y3p;

  // copy the matrix into local storage

  temp = **this->Stack;

  // get scale factors

  this->GetScale (scale_x, scale_y, scale_z);

  // first rotate about y axis

  x2 = temp.Element[2][0] / scale_x;
  y2 = temp.Element[2][1] / scale_y;
  z2 = temp.Element[2][2] / scale_z;

  x3 = temp.Element[1][0] / scale_x;
  y3 = temp.Element[1][1] / scale_y;
  z3 = temp.Element[1][2] / scale_z;

  dot = x2 * x2 + z2 * z2;
  d1 = sqrt (dot);

  if (d1 < AXIS_EPSILON) {
    cos_theta = 1.0;
    sin_theta = 0.0;
  }
  else {
    cos_theta = z2 / d1;
    sin_theta = x2 / d1;
  }

  theta = atan2 (sin_theta, cos_theta);

  y = theta / RADIANS_PER_DEGREE;

  // now rotate about x axis

  dot = x2 * x2 + y2 * y2 + z2 * z2;
  d = sqrt (dot);

  if (d < AXIS_EPSILON) {
    sin_phi = 0.0;
    cos_phi = 1.0;
  }
  else if (d1 < AXIS_EPSILON) {
    sin_phi = y2 / d;
    cos_phi = z2 / d;
  }
  else {
    sin_phi = y2 / d;
    cos_phi = ( x2 * x2 + z2 * z2) / (d1 * d);
  }

  phi = atan2 (sin_phi, cos_phi);

  x = - phi / RADIANS_PER_DEGREE;

  // finally, rotate about z

  x3p = x3 * cos_theta - z3 * sin_theta;
  y3p = - sin_phi * sin_theta * x3 + cos_phi * y3 - sin_phi * cos_theta * z3;
  dot = x3p * x3p + y3p * y3p;

  d2 = sqrt (dot);
  if (d2 < AXIS_EPSILON) {
    cos_alpha = 1.0;
    sin_alpha = 0.0;
  }
  else {
    cos_alpha = y3p / d2;
    sin_alpha = x3p / d2;
  }

  alpha = atan2 (sin_alpha, cos_alpha);

  z = - alpha / RADIANS_PER_DEGREE;
}

void vlTransform::GetPosition (float & x,float & y,float & z)
{
	x = (**this->Stack).Element[0][3];
	y = (**this->Stack).Element[1][3];
	z = (**this->Stack).Element[2][3];
}

void vlTransform::GetScale ( float & x, float & y, float & z)
{
  int	i;
  float	scale[3];
  vlMatrix4x4 temp;

  // copy the matrix into local storage

  temp = **this->Stack;

  // find scale factors

  for (i = 0; i < 3; i++) {
    scale[i] = sqrt (temp.Element[i][0] * temp.Element[i][0] +
	temp.Element[i][1] * temp.Element[i][1] +
	temp.Element[i][2] * temp.Element[i][2]);
  }
  x = scale[0];
  y = scale[1];
  z = scale[2];
}

vlMatrix4x4 & vlTransform::GetMatrix ()
{
  return **this->Stack;;
}

void vlTransform::Identity ()
// Places an identity matrix on the top of the stack.
{
  vlMatrix4x4 ctm;

  int i, j;

  ctm = 0.0;

  for (i = 0; i < 4; i++) {
    ctm.Element[i][i] = 1.0;
  }
  **this->Stack = ctm;
}

void vlTransform::Concatenate (vlMatrix4x4 & matrix)
{
  if (this->PreMultiplyFlag) {
    this->Multiply4x4 (**this->Stack, matrix, **this->Stack);
  }
  else {
    this->Multiply4x4 (matrix, **this->Stack, **this->Stack);
  }
  this->Modified ();
}

void vlTransform::Multiply4x4 ( vlMatrix4x4 & a, vlMatrix4x4 & b, vlMatrix4x4 & c)
{
  int i, j, k;
  vlMatrix4x4 result;

  result = 0.0;
  for (i = 0; i < 4; i++) {
    for (k = 0; k < 4; k++) {
      for (j = 0; j < 4; j++) {
        result.Element[i][k] += a.Element[i][j] * b.Element[j][k];
      }
    }
  }
  c = result;
}

void vlTransform::Transpose ()
{
  this->GetTranspose (**this->Stack);
}

void vlTransform::GetMatrix (vlMatrix4x4 & ctm)
{
  ctm = **this->Stack;
}

vlTransform::~vlTransform ()
{
  // delete all matrices on the stack

  while (this->Stack != this->StackBottom) this->Pop ();

  // delete the bottom matrix
  delete *this->Stack;

  // delet the stack itself
  delete this->Stack;
}

void vlTransform::PrintSelf (ostream& os, vlIndent indent)
{
  vlMatrix4x4 ctm;
  int i, j;

  if (this->ShouldIPrint (vlTransform::GetClassName()))
    {
	  vlObject::PrintSelf(os, indent);

	  os << indent << "Current Transformation:" << "\n";

	  (**this->Stack).PrintSelf (os, indent.GetNextIndent());
    }
}

float *vlTransform::GetVector()
{
  this->Stack[0]->VectorMultiply(this->Vector,this->Vector);
  return this->Vector;
}
