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
// .NAME vtkImageCityBlockDistance - 1,2 or 3D distance map.
// .SECTION Description
// vtkImageCityBlockDistance creates a distance map using the city block
// (Manhatten) distance measure.  The input is a mask.  Zero values are
// considered boundaries.  The output pixel is the minimum of the input pixel
// and the distance to a boundary (or neighbor value + 1 unit).
// distance values are calculated in pixels.
// The filter works by taking 6 passes (for 3d distasnce map): 2 along each 
// axis (forward and backward). Each pass keeps a running minimum distance.
// For some reason, I preserve the sign if the distance.  If the input 
// mask is initially negative, the output distances will be negative.
// Distances maps can have inside (negative regions) 
// and outsides (positive regions).

#ifndef __vtkImageCityBlockDistance_h
#define __vtkImageCityBlockDistance_h


#include "vtkImageDecomposeFilter.h"
#include "vtkImageCityBlockDistance.h"

class VTK_EXPORT vtkImageCityBlockDistance : public vtkImageDecomposeFilter
{
public:
  vtkImageCityBlockDistance();
  static vtkImageCityBlockDistance *New() 
    {return new vtkImageCityBlockDistance;};
  const char *GetClassName() {return "vtkImageCityBlockDistance";};
  
  // Description:
  // Intercepts the caches Update to make the region larger than requested.
  // Create the whole output array.
  void InterceptCacheUpdate();

  
protected:

  void ComputeRequiredInputUpdateExtent(int inExt[6], int outExt[6]);
  void Execute(vtkImageData *inData, vtkImageData *outData);
};

#endif



