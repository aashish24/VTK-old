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
vtkTransform::vtkTransform()
{
  this->Input = NULL;

  // most of the functionality is provided by the concatenation
  this->Concatenation = vtkTransformConcatenation::New();

  // the stack will be allocated the first time Push is called
  this->Stack = NULL;

  // initialize the legacy 'Point' info
  this->Point[0] = this->Point[1] = this->Point[2] = this->Point[3] = 0.0;
  this->DoublePoint[0] = 
    this->DoublePoint[1] = this->DoublePoint[2] = this->DoublePoint[3] = 0.0;

  // save the original matrix MTime as part of a hack to support legacy code
  this->MatrixUpdateMTime = this->Matrix->GetMTime();
}

//----------------------------------------------------------------------------
vtkTransform::~vtkTransform()
{
  this->SetInput(NULL);

  if (this->Concatenation)
    {
    this->Concatenation->Delete();
    }
  if (this->Stack)
    {
    this->Stack->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkTransform::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Update();

  this->vtkLinearTransform::PrintSelf(os, indent);
  os << indent << "Input: (" << this->Input << ")\n";
  this->Concatenation->PrintSelf(os, indent);

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
  this->Concatenation->Identity();
  this->Modified();

  // support for the legacy hack in InternalUpdate
  if (this->Matrix->GetMTime() > this->MatrixUpdateMTime)
    {
    vtkDebugMacro(<<"Identity: Legacy Hack");
    this->Matrix->Identity();
    }
}

//----------------------------------------------------------------------------
void vtkTransform::Inverse() 
{
  this->Concatenation->Inverse();
  this->Modified();

  // for the legacy hack in InternalUpdate
  if (this->Matrix->GetMTime() > this->MatrixUpdateMTime)
    {
    vtkDebugMacro(<<"Inverse: Legacy Hack");
    this->Matrix->Invert();
    }
}

//----------------------------------------------------------------------------
void vtkTransform::InternalDeepCopy(vtkAbstractTransform *gtrans)
{
  vtkTransform *transform = (vtkTransform *)gtrans;

  // copy the input
  this->SetInput(transform->Input);

  // copy the concatenation
  this->Concatenation->DeepCopy(transform->Concatenation);

  // copy the stack
  if (transform->Stack)
    {
    if (this->Stack == NULL)
      {
      this->Stack = vtkTransformConcatenationStack::New();
      }
    this->Stack->DeepCopy(transform->Stack);
    }
  else
    {
    if (this->Stack)
      {
      this->Stack->Delete();
      this->Stack = NULL;
      }
    }

  // legacy stuff: copy Point and DoublePoint
  for (int j = 0; j < 3; j++)
    {
    this->Point[j] = transform->Point[j];
    this->DoublePoint[j] = transform->DoublePoint[j];
    }

  // to support the legacy hack in InternalUpdate
  this->Matrix->DeepCopy(transform->Matrix);
  this->MatrixUpdateMTime = this->Matrix->GetMTime();
  if (transform->Matrix->GetMTime() > transform->MatrixUpdateMTime)
    { // this copies the legacy hack flag to the transform
    vtkDebugMacro(<<"InternalDeepCopy: Legacy Hack");
    this->MatrixUpdateMTime--;
    }
}

//----------------------------------------------------------------------------
void vtkTransform::InternalUpdate()
{
  int i;
  int nTransforms = this->Concatenation->GetNumberOfTransforms();
  int nPreTransforms = this->Concatenation->GetNumberOfPreTransforms();

  // check to see whether someone has been fooling around with our matrix
  int doTheLegacyHack = 0;
  if (this->Matrix->GetMTime() > this->MatrixUpdateMTime)
    {
    vtkDebugMacro(<<"InternalUpdate: this->Matrix was modified by something other than 'this'");

    // check to see if we have any inputs or concatenated transforms
    int isPipelined = (this->Input != 0);
    for (i = 0; i < nTransforms && !isPipelined; i++)
      { // the vtkSimpleTransform is just a matrix placeholder, 
	// it is not a real transform
      isPipelined = 
	!this->Concatenation->GetTransform(i)->IsA("vtkSimpleTransform");
      }
    // do the legacy hack only if we have no input transforms
    doTheLegacyHack = !isPipelined;
    }

  // copy matrix from input
  if (this->Input)
    {
    this->Matrix->DeepCopy(this->Input->GetMatrix());
    // if inverse flag is set, invert the matrix
    if (this->Concatenation->GetInverseFlag())
      {
      this->Matrix->Invert();
      }
    }
  else if (doTheLegacyHack)
    {
    vtkDebugMacro(<<"InternalUpdate: doing hack to support legacy code");
    // this heuristic works perfectly if GetMatrix() or GetMatrixPointer()
    // was called immediately prior to the matrix modifications 
    // (fortunately, this is almost always the case)
    if (this->Matrix->GetMTime() > this->Concatenation->GetMaxMTime())
      { // don't apply operations that occurred after matrix modification
      nPreTransforms = nTransforms = 0;
      }
    }
  else
    {  // otherwise, we start with the identity transform as our base
    this->Matrix->Identity();
    }

  // concatenate PreTransforms 
  for (i = nPreTransforms-1; i >= 0; i--)
    {
    vtkHomogenousTransform *transform = 
      (vtkHomogenousTransform *)this->Concatenation->GetTransform(i);
    vtkMatrix4x4::Multiply4x4(this->Matrix,transform->GetMatrix(),
			      this->Matrix);
    }

  // concatenate PostTransforms
  for (i = nPreTransforms; i < nTransforms; i++)
    {
    vtkHomogenousTransform *transform = 
      (vtkHomogenousTransform *)this->Concatenation->GetTransform(i);
    vtkMatrix4x4::Multiply4x4(transform->GetMatrix(),this->Matrix,
			      this->Matrix);
    }

  if (doTheLegacyHack)
    { // the transform operations have been incorporated into the matrix,
      // so delete them
    this->Concatenation->Identity();
    }
  else
    { // having this in the 'else' forces the legacy flag to be sticky
    this->MatrixUpdateMTime = this->Matrix->GetMTime();
    }
}  

//----------------------------------------------------------------------------
void vtkTransform::Concatenate(vtkLinearTransform *transform)
{
  if (transform->CircuitCheck(this))
    {
    vtkErrorMacro("Concatenate: this would create a circular reference.");
    return; 
    }
  this->Concatenation->Concatenate(transform); 
  this->Modified(); 
};

//----------------------------------------------------------------------------
void vtkTransform::SetInput(vtkLinearTransform *input)
{
  if (this->Input == input) 
    { 
    return; 
    }
  if (input && input->CircuitCheck(this)) 
    {
    vtkErrorMacro("SetInput: this would create a circular reference.");
    return; 
    }
  if (this->Input) 
    { 
    this->Input->Delete(); 
    }
  this->Input = input;
  if (this->Input) 
    { 
    this->Input->Register(this); 
    } 
  this->Modified();
}

//----------------------------------------------------------------------------
int vtkTransform::CircuitCheck(vtkAbstractTransform *transform)
{
  if (this->vtkLinearTransform::CircuitCheck(transform) ||
      this->Input && this->Input->CircuitCheck(transform))
    {
    return 1;
    }

  int n = this->Concatenation->GetNumberOfTransforms();
  for (int i = 0; i < n; i++)
    {
    if (this->Concatenation->GetTransform(i)->CircuitCheck(transform))
      {
      return 1;
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
vtkAbstractTransform *vtkTransform::MakeTransform()
{
  return vtkTransform::New();
}

//----------------------------------------------------------------------------
unsigned long vtkTransform::GetMTime()
{
  unsigned long mtime = this->vtkLinearTransform::GetMTime();
  unsigned long mtime2;

  // checking the matrix MTime is part of the legacy hack in InternalUpdate
  if ((mtime2 = this->Matrix->GetMTime()) > this->MatrixUpdateMTime)
    {
    if (mtime2 > mtime)
      {
      mtime = mtime2;
      }
    }  

  if (this->Input)
    {
    mtime2 = this->Input->GetMTime();
    if (mtime2 > mtime)
      {
      mtime = mtime2;
      }
    }
  mtime2 = this->Concatenation->GetMaxMTime();
  if (mtime2 > mtime)
    {
    return mtime2;
    }
  return mtime;
}

//----------------------------------------------------------------------------
// Get the x, y, z orientation angles from the transformation matrix as an
// array of three floating point values.
void vtkTransform::GetOrientation(double orientation[3])
{
#define VTK_AXIS_EPSILON 0.001

  this->Update();
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
  this->Update();

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
  this->Update();

  position[0] = this->Matrix->Element[0][3];
  position[1] = this->Matrix->Element[1][3];
  position[2] = this->Matrix->Element[2][3];
}

//----------------------------------------------------------------------------
// Return the x, y, z scale factors of the current transformation matrix as 
// an array of three float numbers.
void vtkTransform::GetScale(double scale[3])
{
  this->Update();

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
// Return the inverse of the current transformation matrix.
void vtkTransform::GetInverse(vtkMatrix4x4 *inverse)
{
  vtkMatrix4x4::Invert(this->GetMatrix(),inverse);
}

//----------------------------------------------------------------------------
// Obtain the transpose of the current transformation matrix.
void vtkTransform::GetTranspose(vtkMatrix4x4 *transpose)
{
  vtkMatrix4x4::Transpose(this->GetMatrix(),transpose);
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
  if (this->Concatenation->GetPreMultiplyFlag())
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
  if (this->Concatenation->GetPreMultiplyFlag())
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
