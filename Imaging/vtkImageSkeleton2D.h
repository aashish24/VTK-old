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
// .NAME vtkImageSkeleton2D - Skeleton of 2D images.
// .SECTION Description
// vtkImageSkeleton2D should leave only single pixel width lines
// of non-zero-valued pixels (values of 1 are not allowed).  
// It works by errosion on a 3x3 neighborhood with special rules.
// Number of iterations determines how far the filter can errode.
// There are three pruning levels:  
//  prune == 0 will leave traces on all angles...
//  prune == 1 will not leave traces on 135 degree angles, but will on 90.
//  prune == 2 does not leave traces on any angles leaving only closed loops.
// Output scalar type is the same as input.



#ifndef __vtkImageSkeleton2D_h
#define __vtkImageSkeleton2D_h

#include "vtkImageIterateFilter.h"

class VTK_EXPORT vtkImageSkeleton2D : public vtkImageIterateFilter
{
public:
  vtkImageSkeleton2D();
  static vtkImageSkeleton2D *New() {return new vtkImageSkeleton2D;};
  const char *GetClassName() {return "vtkImageSkeleton2D";};

  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // When prune is on, only closed loops are left un changed.
  vtkSetMacro(Prune,int);
  vtkGetMacro(Prune,int);
  vtkBooleanMacro(Prune,int);

  // Description:
  // Sets the number of cycles in the errosion.
  void SetNumberOfIterations(int num);
  
protected:
  int Prune;

  void ComputeRequiredInputUpdateExtent(int *extent, int *wholeExtent);
  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData,
		       int outExt[6], int id);
};

#endif



