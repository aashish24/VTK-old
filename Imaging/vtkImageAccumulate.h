/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to C. Charles Law who developed this class

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
// .NAME vtkImageAccumulate - Generalized histograms upto 4 dimensions.
// .SECTION Description
// vtkImageAccumulate - This filter divides component space into
// discrete bins.  It then counts the number of pixels associated
// with each bin.  The output is this "scatter plot".
// The input can be any type, but the output is always int.


#ifndef __vtkImageAccumulate_h
#define __vtkImageAccumulate_h


#include "vtkImageToImageFilter.h"

class VTK_EXPORT vtkImageAccumulate : public vtkImageToImageFilter
{
public:
  static vtkImageAccumulate *New();
  vtkTypeMacro(vtkImageAccumulate,vtkImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get - The component spacing is the dimension of each cell.
  vtkSetVector3Macro(ComponentSpacing, float);
  vtkGetVector3Macro(ComponentSpacing, float);

  // Description:
  // Set/Get - The component origin is the location of bin (0, 0, 0).
  vtkSetVector3Macro(ComponentOrigin, float);
  vtkGetVector3Macro(ComponentOrigin, float);

  // Description:
  // Set/Get - The component extent is the number/extent of the bins.  
  void SetComponentExtent(int extent[6]);
  void SetComponentExtent(int minX, int maxX, int minY, int maxY, 
			  int minZ, int maxZ);
  void GetComponentExtent(int extent[6]);
  int *GetComponentExtent() {return this->ComponentExtent;}
  
protected:
  vtkImageAccumulate();
  ~vtkImageAccumulate() {};
  vtkImageAccumulate(const vtkImageAccumulate&) {};
  void operator=(const vtkImageAccumulate&) {};

  float ComponentSpacing[3];
  float ComponentOrigin[3];
  int ComponentExtent[6];

  void ExecuteInformation(vtkImageData *input, vtkImageData *output);
  void ComputeInputUpdateExtent(int inExt[6], int outExt[6]);
  void ExecuteInformation(){this->vtkImageToImageFilter::ExecuteInformation();};
  void Execute(vtkImageData *inData, vtkImageData *outData);
  void Execute() { this->vtkImageToImageFilter::Execute(); };
  void Execute(vtkImageData *outData) { this->vtkImageToImageFilter::Execute(outData); };

  // Description:
  // Generate more than requested.  Called by the superclass before
  // an execute, and before output memory is allocated.
  void EnlargeOutputUpdateExtents( vtkDataObject *data );

};

#endif



