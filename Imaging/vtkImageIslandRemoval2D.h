/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to C. Charles Law who developed this class.

Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen.

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
// .NAME vtkImageIslandRemoval2D - Removes small clusters in masks.
// .SECTION Description
// vtkImageIslandRemoval2D computes the area of separate islands in 
// a mask image.  It removes any island that has less than AreaThreshold
// pixels.  Output has the same ScalarType as input.  It generates
// the whole 2D output image for any output request.


#ifndef __vtkImageIslandRemoval2D_h
#define __vtkImageIslandRemoval2D_h


#include "vtkImageToImageFilter.h"

//BTX
typedef struct{
  void *inPtr;
  void *outPtr;
  int idx0;
  int idx1;
  } vtkImage2DIslandPixel;
//ETX

class VTK_EXPORT vtkImageIslandRemoval2D : public vtkImageToImageFilter
{
public:
  // Description:
  // Constructor: Sets default filter to be identity.
  static vtkImageIslandRemoval2D *New();
  vtkTypeMacro(vtkImageIslandRemoval2D,vtkImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Set/Get the cutoff area for removal
  vtkSetMacro(AreaThreshold, int);
  vtkGetMacro(AreaThreshold, int);

  // Description:
  // Set/Get whether to use 4 or 8 neighbors
  vtkSetMacro(SquareNeighborhood, int);
  vtkGetMacro(SquareNeighborhood, int);
  vtkBooleanMacro(SquareNeighborhood, int);

  // Description:
  // Set/Get the value to remove.
  vtkSetMacro(IslandValue, float);
  vtkGetMacro(IslandValue, float);

  // Description:
  // Set/Get the value to put in the place of removed pixels.
  vtkSetMacro(ReplaceValue, float);
  vtkGetMacro(ReplaceValue, float);
  
protected:
  vtkImageIslandRemoval2D();
  ~vtkImageIslandRemoval2D() {};
  vtkImageIslandRemoval2D(const vtkImageIslandRemoval2D&) {};
  void operator=(const vtkImageIslandRemoval2D&) {};

  int AreaThreshold;
  int SquareNeighborhood;
  float IslandValue;
  float ReplaceValue;

  void Execute(vtkImageData *inData, vtkImageData *outData);
  void Execute() { this->vtkImageToImageFilter::Execute(); };
  void Execute(vtkImageData *output)
    { this->vtkImageToImageFilter::Execute(output); };

  // Description:
  // Generate more than requested.  Called by the superclass before
  // an execute, and before output memory is allocated.
  void EnlargeOutputUpdateExtents( vtkDataObject *data );

};

#endif



