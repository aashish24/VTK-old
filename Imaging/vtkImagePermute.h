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
// .NAME vtkImagePermute -  Permutes axes of input.
// .SECTION Description
// vtkImagePermute reorders the axes of the input. Filtered axes specify
// the input axes which become X, Y, Z.  The input has to have the
// same scalar type of the output. The filter does copy the 
// data when it executes. 

#ifndef __vtkImagePermute_h
#define __vtkImagePermute_h


#include "vtkImageToImageFilter.h"

class VTK_EXPORT vtkImagePermute : public vtkImageToImageFilter
{
public:
  static vtkImagePermute *New() {return new vtkImagePermute;};
  const char *GetClassName() {return "vtkImagePermute";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // The filtered axes are the input axes that get relabeled to X,Y,Z.
  vtkSetVector3Macro(FilteredAxes, int);
  vtkGetVector3Macro(FilteredAxes, int);
  
protected:
  vtkImagePermute();
  ~vtkImagePermute() {};

  int  FilteredAxes[3];
  void ExecuteInformation(vtkImageData *inData, vtkImageData *outData);
  void ComputeRequiredInputUpdateExtent(int inExt[6], int outExt[6]);
  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData, 
		       int ext[6], int id);
};

#endif



