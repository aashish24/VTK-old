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
// .NAME vtkScalarsToColors - map scalar values into colors
// .SECTION Description
// vtkScalarsToColors is a general purpose superclass for objects that
// convert scalars to colors. This include vtkLookupTable classes and
// color transfer functions.
//
// .SECTION See Also
// vtkLookupTable vtkColorTransferFunction

#ifndef __vtkScalarsToColors_h
#define __vtkScalarsToColors_h

#include "vtkObject.h"
#include "vtkUnsignedCharArray.h"

#define VTK_RGBA       4
#define VTK_RGB        3
#define VTK_LUMINANCE  1
#define VTK_LUMINANCE_ALPHA 2

class vtkScalars;

class VTK_EXPORT vtkScalarsToColors : public vtkObject
{
public:
  vtkTypeMacro(vtkScalarsToColors,vtkObject);
  
  // Description:
  // Perform any processing required (if any) before processing 
  // scalars.
  virtual void Build() {};
  
  // Description:
  // Sets/Gets the range of scalars which will eb mapped.
  virtual float *GetRange() = 0;
  virtual void SetRange(float min, float max) = 0;
  void SetRange(float rng[2]) {this->SetRange(rng[0],rng[1]);};
  
  // Description:
  // Map one value through the lookup table and return a color defined
  // as a RGBA unsigned char tuple (4 bytes).
  virtual unsigned char *MapValue(float v) = 0;

  // Description:
  // Map a set of scalars through the lookup table in a single operation. 
  // The output format can be set to VTK_RGBA (4 components), 
  // VTK_RGB (3 components), VTK_LUMINANCE (1 component, greyscale),
  // or VTK_LUMINANCE_ALPHA (2 components)
  // If not supplied, the output format defaults to RGBA.
  void MapScalarsThroughTable(vtkScalars *scalars, 
                              unsigned char *output,
			      int outputFormat);
  void MapScalarsThroughTable(vtkScalars *scalars, 
                              unsigned char *output) 
    { this->MapScalarsThroughTable(scalars,output,VTK_RGBA); };


  // Description:
  // An internal method typically not used in applications.
  virtual void MapScalarsThroughTable2(void *input, unsigned char *output,
                                       int inputDataType, int numberOfValues,
                                       int inputIncrement, 
				       int outputFormat) = 0;

protected:
  vtkScalarsToColors() {};
  ~vtkScalarsToColors() {};
  vtkScalarsToColors(const vtkScalarsToColors &) {};
  void operator=(const vtkScalarsToColors &) {};

};

#endif



