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
// .NAME vtkShrinkFilter - shrink cells composing an arbitrary data set
// .SECTION Description
// vtkShrinkFilter shrinks cells composing an arbitrary data set 
// towards their centroid. The centroid of a cell is computed as 
// the average position of the cell points. Shrinking results in 
// disconnecting the cells from one another. The output of this filter is
// of general dataset type vtkUnstructuredGrid.

// .SECTION Caveats
// It is possible to turn cells inside out or cause self intersection
// in special cases.

// .SECTION See Also
// vtkShrinkPolyData

#ifndef __vtkShrinkFilter_h
#define __vtkShrinkFilter_h

#include "vtkDataSetToUnstructuredGridFilter.h"

class VTK_EXPORT vtkShrinkFilter : public vtkDataSetToUnstructuredGridFilter
{
public:
  static vtkShrinkFilter *New() {return new vtkShrinkFilter;};
  const char *GetClassName() {return "vtkShrinkFilter";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the fraction of shrink for each cell.
  vtkSetClampMacro(ShrinkFactor,float,0.0,1.0);

  // Description:
  // Get the fraction of shrink for each cell.
  vtkGetMacro(ShrinkFactor,float);

protected:
  vtkShrinkFilter(float sf=0.5);
  ~vtkShrinkFilter() {};
  vtkShrinkFilter(const vtkShrinkFilter&) {};
  void operator=(const vtkShrinkFilter&) {};

  void Execute();
  float ShrinkFactor;
};

#endif


