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
// .NAME vtkImageNoiseSource - Create an image filled with noise.
// .SECTION Description
// vtkImageNoiseSource just produces images filled with noise.  The only
// option now is uniform noise specified by a min and a max.  There is one
// major problem with this source. Every time it executes, it will output
// different pixel values.  This has important implications when a stream
// requests overlapping regions.  The same pixels will have different values
// on different updates.


#ifndef __vtkImageNoiseSource_h
#define __vtkImageNoiseSource_h


#include "vtkImageSource.h"


class VTK_EXPORT vtkImageNoiseSource : public vtkImageSource 
{
public:
  static vtkImageNoiseSource *New();
  const char *GetClassName() {return "vtkImageNoiseSource";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the minimum and maximum values for the generated noise.
  vtkSetMacro(Minimum, float);
  vtkGetMacro(Minimum, float);
  vtkSetMacro(Maximum, float);
  vtkGetMacro(Maximum, float);

  // Description:
  // Set how large of an image to generate.
  void SetWholeExtent(int xMinx, int xMax, int yMin, int yMax,
		      int zMin, int zMax);

  // Description:
  // updates the inage information, (Extent, Scalar type, etc).
  void UpdateInformation();

protected:
  vtkImageNoiseSource();
  ~vtkImageNoiseSource() {};
  vtkImageNoiseSource(const vtkImageNoiseSource&) {};
  void operator=(const vtkImageNoiseSource&) {};

  float Minimum;
  float Maximum;
  int WholeExtent[6];

  void Execute(vtkImageData *data);
};


#endif

  
