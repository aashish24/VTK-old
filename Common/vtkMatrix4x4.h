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
// .NAME vtkMatrix4x4 - represent and manipulate 4x4 transformation matrices
// .SECTION Description
// vtkMatrix4x4 is a class to represent and manipulate 4x4 matrices. 
// Specifically, it is designed to work on 4x4 transformation matrices
// found in 3D rendering using homogeneous coordinates [x y z w].

// .SECTION See Also
// vtkTransform

#ifndef __vtkMatrix4x4_h
#define __vtkMatrix4x4_h

#include "vtkObject.h"

class VTK_EXPORT vtkMatrix4x4 : public vtkObject
{
 public:
  double Element[4][4];
  static vtkMatrix4x4 *New() {return new vtkMatrix4x4;};
  const char *GetClassName() {return "vtkMatrix4x4";};
  void PrintSelf (ostream& os, vtkIndent indent);

  // Description:
  // Construct a 4x4 identity matrix.
  vtkMatrix4x4();

  // Description:
  // Construct a 4x4 matrix with the values which are contained in the
  // argument m.
  vtkMatrix4x4(const vtkMatrix4x4& m);
  
  // Description:
  // Set the elements of the matrix to the same values as the elements
  // of the source Matrix.
  void DeepCopy(vtkMatrix4x4 *source);
  
  // Description:
  // Set all of the elements to zero.
  void Zero();
  
  // Description:
  // Matrix Inversion (adapted from Richard Carling in "Graphics Gems," 
  // Academic Press, 1990).
  void Invert(vtkMatrix4x4 *in,vtkMatrix4x4 *out);
  void Invert(void) { this->Invert(this,this);};

  // Description:
  // Transpose the matrix and put it into out. 
  void Transpose(vtkMatrix4x4 *in,vtkMatrix4x4 *out);
  void Transpose(void) { this->Transpose(this,this);};

  // Description:
  // Multiply this matrix by a point (in homogeneous coordinates). 
  // and return the result in result. The in[4] and result[4] 
  // arrays must both be allocated but they can be the same array.
  void MultiplyPoint(float in[4], float out[4]);
  void MultiplyPoint(double in[4], double out[4]);

  // Description:
  // Multiply a point (in homogeneous coordinates) by this matrix,
  // and return the result in result. The in[4] and result[4] 
  // arrays must both be allocated, but they can be the same array.
  void PointMultiply(float in[4], float out[4]);
  void PointMultiply(double in[4], double out[4]);

  // Description:
  // Compute adjoint of the matrix and put it into out.
  void Adjoint(vtkMatrix4x4 *in,vtkMatrix4x4 *out);

  // Description:
  // Compute the determinant of the matrix and return it.
  float Determinant(vtkMatrix4x4 *in);

  // Description:
  // Sets the element i,j in the matrix.
  void SetElement(int i, int j, double value);
//BTX
  void SetElement(int i, int j, float value);
//ETX
  // Description:
  // Returns the element i,j from the matrix.
  double GetElement(int i, int j) const {return this->Element[i][j];};

  // Description: 
  // For legacy compatibility. Do not use.
  double *operator[](const unsigned int i) {return &(this->Element[i][0]);};
  const double *operator[](unsigned int i) const
    { return &(this->Element[i][0]); }  
  void operator= (double element);
  vtkMatrix4x4& operator= (const vtkMatrix4x4& source);
  void Adjoint(vtkMatrix4x4 &in,vtkMatrix4x4 &out){this->Adjoint(&in,&out);}
  float Determinant(vtkMatrix4x4 &in) {return this->Determinant(&in);}
  void Invert(vtkMatrix4x4 &in,vtkMatrix4x4 &out)
    {this->Invert(&in,&out);}
  void Transpose(vtkMatrix4x4 &in,vtkMatrix4x4 &out)
    {this->Transpose(&in,&out);}
};

inline void vtkMatrix4x4::SetElement (int i, int j, float value)
{
  if (this->Element[i][j] != value)
    {
    this->Element[i][j] = value;
    this->Modified();
    }
}
inline void vtkMatrix4x4::SetElement (int i, int j, double value)
{
  if (this->Element[i][j] != value)
    {
    this->Element[i][j] = value;
    this->Modified();
    }
}

#endif

