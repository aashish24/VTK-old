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
// .NAME vtkContourValues - helper object to manage setting and generating contour values
// .SECTION Description
// vtkContourValues is a general class to manage the creation, generation,
// and retrieval of contour values. This class serves as a helper class for
// contouring classes, or those classes operating on lists of contour values.

// .SECTION See Also
// vtkContourFilter

#ifndef __vtkContourValues_h
#define __vtkContourValues_h

#include "vtkObject.h"
#include "vtkIntArray.h"

class vtkFloatArray;

class VTK_EXPORT vtkContourValues : public vtkObject
{
public:
  // Description:
  // Construct object with a single contour value at 0.0.
  static vtkContourValues *New();

  const char *GetClassName() {return "vtkContourValues";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the ith contour value.
  void SetValue(int i, float value);

  // Description:
  // Get the ith contour value. The return value will be clamped if the
  // index i is out of range.
  float GetValue(int i);

  // Description:
  // Return a pointer to a list of contour values. The contents of the
  // list will be garbage if the number of contours <= 0.
  float *GetValues();

  // Description:
  // Fill a supplied list with contour values. Make sure you've
  // allocated memory of size GetNumberOfContours().
  void GetValues(float *contourValues);

  // Description:
  // Set the number of contours to place into the list. You only really
  // need to use this method to reduce list size. The method SetValue()
  // will automatically increase list size as needed.
  void SetNumberOfContours(const int number);

  // Description:
  // Return the number of contours in the
  int GetNumberOfContours();

  // Description:
  // Generate numContours equally spaced contour values between specified
  // range. Contour values will include min/max range values.
  void GenerateValues(int numContours, float range[2]);

  // Description:
  // Generate numContours equally spaced contour values between specified
  // range. Contour values will include min/max range values.
  void GenerateValues(int numContours, float rangeStart, float rangeEnd);


protected:
  vtkContourValues();
  ~vtkContourValues();
  vtkContourValues(const vtkContourValues&) {};
  void operator=(const vtkContourValues&) {};

  vtkFloatArray *Contours;

};

#endif
