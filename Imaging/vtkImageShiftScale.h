/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to C. Charles Law who developed this class.

Copyright (c) 1993-1995 Ken Martin, Will Schroeder, Bill Lorensen.

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
// .NAME vtkImageShiftScale - shift and scale an input image
// .SECTION Description
// With vtkImageShiftScale Pixels are shifted and then scaled. As
// a convenience, this class allows you to set the output scalar type
// similar to vtkImageCast. This is because shift scale operations
// frequently convert data types.


#ifndef __vtkImageShiftScale_h
#define __vtkImageShiftScale_h


#include "vtkImageToImageFilter.h"

class VTK_EXPORT vtkImageShiftScale : public vtkImageToImageFilter
{
public:
  vtkImageShiftScale();
  static vtkImageShiftScale *New() {return new vtkImageShiftScale;};
  const char *GetClassName() {return "vtkImageShiftScale";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the shift value.
  vtkSetMacro(Shift,float);
  vtkGetMacro(Shift,float);

  // Description:
  // Set/Get the scale value.
  vtkSetMacro(Scale,float);
  vtkGetMacro(Scale,float);

  // Description:
  // Set the desired output scalar type. The result of the shift 
  // and scale operations is cast to the type specified.
  vtkSetMacro(OutputScalarType, int);
  vtkGetMacro(OutputScalarType, int);
  void SetOutputScalarTypeToFloat() {this->SetOutputScalarType(VTK_FLOAT);};
  void SetOutputScalarTypeToInt() {this->SetOutputScalarType(VTK_INT);};
  void SetOutputScalarTypeToShort() {this->SetOutputScalarType(VTK_SHORT);};
  void SetOutputScalarTypeToUnsignedShort() 
    {this->SetOutputScalarType(VTK_UNSIGNED_SHORT);};
  void SetOutputScalarTypeToUnsignedChar() 
    {this->SetOutputScalarType(VTK_UNSIGNED_CHAR);};

  // Description:
  // When the ClampOverflow flag is on, the data is thresholded so that
  // the output value does not exceed the max or min of the data type.
  // By defualt, ClampOverflow is off.
  vtkSetMacro(ClampOverflow, int);
  vtkGetMacro(ClampOverflow, int);
  vtkBooleanMacro(ClampOverflow, int);
  
protected:
  float Shift;
  float Scale;
  int OutputScalarType;
  int ClampOverflow;
  
  void ExecuteImageInformation();
  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData,
		       int extent[6], int id);
};

#endif



