/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to David G. Gobbi who developed this class.

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
// .NAME vtkLinearTransform - superclass for linear geometric transformations
// .SECTION Description
// vtkLinearTransform provides a generic interface for linear 
// (affine or 12 degree-of-freedom) transformations. 
// .SECTION see also
// vtkTransform vtkLinearTransformConcatenation vtkIdentityTransform 


#ifndef __vtkLinearTransform_h
#define __vtkLinearTransform_h

#include "vtkPerspectiveTransform.h"

class VTK_EXPORT vtkLinearTransform : public vtkPerspectiveTransform
{
public:

  vtkTypeMacro(vtkLinearTransform,vtkPerspectiveTransform);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Apply the transformation to a coordinate.  You can use the same 
  // array to store both the input and output point.
  void TransformPoint(const float in[3], float out[3]);

  // Description:
  // Apply the transformation to a double-precision coordinate.  
  // You can use the same array to store both the input and output point.
  void TransformPoint(const double in[3], double out[3]);

  // Description:
  // Apply the transformation to an (x,y,z) coordinate.
  // Use this if you are programming in python, tcl or Java.
  float *TransformPoint(float x, float y, float z) {
    return this->vtkGeneralTransform::TransformPoint(x,y,z); }
  float *TransformPoint(const float point[3]) {
    return this->TransformPoint(point[0],point[1],point[2]); };

  // Description:
  // Apply the transformation to a normal.
  // You can use the same array to store both the input and output.
  void TransformNormal(const float in[3], float out[3]);

  // Description:
  // Apply the transformation to a double-precision normal.
  // You can use the same array to store both the input and output.
  void TransformNormal(const double in[3], double out[3]);

  // Description:
  // Synonymous with TransformFloatNormal(x,y,z).
  // Use this if you are programming in python, tcl or Java.
  float *TransformNormal(float x, float y, float z) {
    return this->TransformFloatNormal(x,y,z); }
  float *TransformNormal(const float normal[3]) {
    return this->TransformFloatNormal(normal[0],normal[1],normal[2]); };

  // Description:
  // Apply the transformation to an (x,y,z) normal.
  // Use this if you are programming in python, tcl or Java.
  float *TransformFloatNormal(float x, float y, float z);
  float *TransformFloatNormal(const float normal[3]) {
    return this->TransformFloatNormal(normal[0],normal[1],normal[2]); };

  // Description:
  // Apply the transformation to a double-precision (x,y,z) normal.
  // Use this if you are programming in python, tcl or Java.
  double *TransformDoubleNormal(double x, double y, double z);
  double *TransformDoubleNormal(const double normal[3]) {
    return this->TransformDoubleNormal(normal[0],normal[1],normal[2]); };

  // Description:
  // Synonymous with TransformFloatVector(x,y,z).
  // Use this if you are programming in python, tcl or Java.
  float *TransformVector(float x, float y, float z) {
    return this->TransformFloatVector(x,y,z); }
  float *TransformVector(const float normal[3]) {
    return this->TransformFloatVector(normal[0],normal[1],normal[2]); };

  // Description:
  // Apply the transformation to a vector.
  // You can use the same array to store both the input and output.
  void TransformVector(const float in[3], float out[3]);

  // Description:
  // Apply the transformation to a double-precision vector.
  // You can use the same array to store both the input and output.
  void TransformVector(const double in[3], double out[3]);

  // Description:
  // Apply the transformation to an (x,y,z) vector.
  // Use this if you are programming in python, tcl or Java.
  float *TransformFloatVector(float x, float y, float z);
  float *TransformFloatVector(const float vec[3]) {
    return this->TransformFloatVector(vec[0],vec[1],vec[2]); };

  // Description:
  // Apply the transformation to a double-precision (x,y,z) vector.
  // Use this if you are programming in python, tcl or Java.
  double *TransformDoubleVector(double x, double y, double z);
  double *TransformDoubleVector(const double vec[3]) {
    return this->TransformDoubleVector(vec[0],vec[1],vec[2]); };

  // Description:
  // Apply the transformation to a series of points, and append the
  // results to outPts.  
  void TransformPoints(vtkPoints *inPts, vtkPoints *outPts);

  // Description:
  // Apply the transformation to a series of normals, and append the
  // results to outNms.  
  virtual void TransformNormals(vtkNormals *inNms, vtkNormals *outNms);

  // Description:
  // Apply the transformation to a series of vectors, and append the
  // results to outVrs.  
  virtual void TransformVectors(vtkVectors *inVrs, vtkVectors *outVrs);

  // Description:
  // Apply the transformation to a combination of points, normals
  // and vectors.  
  void TransformPointsNormalsVectors(vtkPoints *inPts, 
				     vtkPoints *outPts, 
				     vtkNormals *inNms, 
				     vtkNormals *outNms,
				     vtkVectors *inVrs, 
				     vtkVectors *outVrs);

  // Description:
  // Get the inverse of this transform.  If you modify this transform,
  // the returned inverse transform will automatically update.
  vtkGeneralTransform *GetInverse();

  // Description:
  // Get the inverse of this transform, typecast to a vtkLinearTransform.
  vtkLinearTransform *GetLinearInverse() { 
    return (vtkLinearTransform *)this->GetInverse(); }; 

  // Description:
  // This will calculate the transformation without calling Update.
  // Meant for use only within other VTK classes.
  void InternalTransformPoint(const float in[3], float out[3]);

  // Description:
  // This will calculate the transformation as well as its derivative
  // without calling Update.  Meant for use only within other VTK
  // classes.
  void InternalTransformDerivative(const float in[3], float out[3],
				   float derivative[3][3]);

protected:
  vtkLinearTransform() {};
  ~vtkLinearTransform() {};
  vtkLinearTransform(const vtkLinearTransform&) {};
  void operator=(const vtkLinearTransform&) {};
};

#endif





