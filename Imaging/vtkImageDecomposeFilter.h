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
// .NAME vtkImageDecomposeFilter - Filters that execute axes in series.
// .SECTION Description
// This superclass molds the vtkImageIterateFilter superclass so
// it iterates over the axes.  The filter uses dimensionality to 
// determine how many axes to execute (starting from x).  
// The filter also provides convenience methods for permuting information
// retrieved from input, output and vtkImageData.

#ifndef __vtkImageDecomposeFilter_h
#define __vtkImageDecomposeFilter_h


#include "vtkImageIterateFilter.h"

class VTK_EXPORT vtkImageDecomposeFilter : public vtkImageIterateFilter
{
public:
  // Description:
  // Construct an instance of vtkImageDecomposeFilter fitler with default
  // dimensionality 3.
  static vtkImageDecomposeFilter *New() {return new vtkImageDecomposeFilter;};
  const char *GetClassName() {return "vtkImageDecomposeFilter";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Dimensionality is the number of axes which are considered durring
  // execution. To process images dimensionality would be set to 2.
  void SetDimensionality(int dim);
  vtkGetMacro(Dimensionality,int);

  // Description:
  // Obsolete legacy methods.
  void SetFilteredAxes(int axis0);
  void SetFilteredAxes(int axis0, int axis2);
  void SetFilteredAxes(int axis0, int axis2, int axis3);

  // Description:
  // Private methods kept public for template execute functions.
  void PermuteIncrements(int *increments, int &inc0, int &inc1, int &inc2);
  void PermuteExtent(int *extent, int &min0, int &max0, int &min1, int &max1,
		     int &min2, int &max2);
  
protected:
  vtkImageDecomposeFilter();
  ~vtkImageDecomposeFilter() {};

  int Dimensionality;


};

#endif










