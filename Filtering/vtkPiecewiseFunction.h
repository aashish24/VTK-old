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

// .NAME vtkPiecewiseFunction - Defines a 1D piecewise function.
// 
// .SECTION Description
// Defines a piecewise linear function mapping. Used for transfer functions
// in volume rendering.

#ifndef __vtkPiecewiseFunction_h
#define __vtkPiecewiseFunction_h

#include "vtkDataObject.h"

class VTK_EXPORT vtkPiecewiseFunction : public vtkDataObject
{
public:
  static vtkPiecewiseFunction *New() {return new vtkPiecewiseFunction;};
  const char *GetClassName() {return "vtkPiecewiseFunction";};
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkDataObject *MakeObject();
  void Initialize();
  void DeepCopy( vtkPiecewiseFunction *f );

  // Description:
  // Return what type of dataset this is.
  int GetDataObjectType() {return VTK_PIECEWISE_FUNCTION;};
  
  // Description:
  // Get the number of points used to specify the function
  int  GetSize();

  // Description:
  // Add/Remove points to/from the function. If a duplicate point is added
  // then the function value is changed at that location.
  void AddPoint( float x, float val );
  void RemovePoint( float x );

  // Description:
  // Removes all points from the function. 
  void RemoveAllPoints();

  // Description:
  // Add a line segment to the function. All points defined between the
  // two points specified are removed from the function.
  void AddSegment( float x1, float val1, float x2, float val2 );

  // Description:
  // Returns the value of the function at the specified location using
  // the specified interpolation. Returns zero if the specified location
  // is outside the min and max points of the function.
  float GetValue( float x );

  // Description:
  // Returns a pointer to the data stored in the table.
  float *GetDataPointer() {return this->Function;};

  // Description:
  // Returns the min and max point locations of the function.
  float *GetRange();

  // Description:
  // Fills in an array of function values evaluated at regular intervals
  void GetTable( float x1, float x2, int size, float *table );

  // Description:
  // When zero range clamping is Off, GetValue() returns 0.0 when a
  // value is requested outside of the points specified.
  // When zero range clamping is On, GetValue() returns the value at
  // the value at the lowest point for a request below all points 
  // specified and returns the value at the highest point for a request 
  // above all points specified. On is the default.
  vtkSetMacro( Clamping, int );
  vtkGetMacro( Clamping, int );
  vtkBooleanMacro( Clamping, int );

  // Description:
  // Return the type of function:
  // Function Types:
  //    0 : Constant        (No change in slope between end points)
  //    1 : NonDecreasing   (Always increasing or zero slope)
  //    2 : NonIncreasing   (Always decreasing or zero slope)
  //    3 : Varied          (Contains both decreasing and increasing slopes)
  char  *GetType();

  // Description:
  // Get the mtime of this object - override to consider the
  // mtime of the source as well.
  unsigned long GetMTime();

  // Description:
  // Returns the first point location which precedes a non-zero segment of the
  // function. Note that the value at this point may be zero.
  float GetFirstNonZeroValue();

protected:
  vtkPiecewiseFunction();
  ~vtkPiecewiseFunction();
  vtkPiecewiseFunction(const vtkPiecewiseFunction&) {};
  void operator=(const vtkPiecewiseFunction&) {};

  // Size of the array used to store function points
  int	ArraySize;

  // Determines the function value outside of defined points
  // Zero = always return 0.0 outside of defined points
  // One  = clamp to the lowest value below defined points and
  //        highest value above defined points
  int	Clamping;

  // Array of points ((X,Y) pairs)
  float	*Function;

  // Number of points used to specify function
  int   FunctionSize;

  // Min and max range of function point locations
  float FunctionRange[2];

  // Increases size of the function array. The array grows by a factor of 2
  // when the array limit has been reached.
  void IncreaseArraySize();

  // Private function to add a point to the function. Returns the array
  // index of the inserted point.
  int InsertPoint( float x, float val );

  // Move points one index down or up in the array. This is useful for 
  // inserting and deleting points into the array.
  void MovePoints( int index, int down );
};

#endif

