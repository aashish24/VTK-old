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
// .NAME vtkRectilinearGridGeometryFilter - extract geometry for a rectilinear grid
// .SECTION Description
// vtkRectilinearGridGeometryFilter is a filter that extracts geometry from a
// rectilinear grid. By specifying appropriate i-j-k indices, it is possible
// to extract a point, a curve, a surface, or a "volume". The volume
// is actually a (n x m x o) region of points.
//
// The extent specification is zero-offset. That is, the first k-plane in
// a 50x50x50 rectilinear grid is given by (0,49, 0,49, 0,0).

// .SECTION Caveats
// If you don't know the dimensions of the input dataset, you can use a large
// number to specify extent (the number will be clamped appropriately). For 
// example, if the dataset dimensions are 50x50x50, and you want a the fifth 
// k-plane, you can use the extents (0,100, 0,100, 4,4). The 100 will 
// automatically be clamped to 49.

// .SECTION See Also
// vtkGeometryFilter vtkExtractGrid

#ifndef __vtkRectilinearGridGeometryFilter_h
#define __vtkRectilinearGridGeometryFilter_h

#include "vtkRectilinearGridToPolyDataFilter.h"

class VTK_EXPORT vtkRectilinearGridGeometryFilter : public vtkRectilinearGridToPolyDataFilter
{
public:
  vtkTypeMacro(vtkRectilinearGridGeometryFilter,vtkRectilinearGridToPolyDataFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct with initial extent (0,100, 0,100, 0,0) (i.e., a k-plane).
  static vtkRectilinearGridGeometryFilter *New();

  // Description:
  // Get the extent in topological coordinate range (imin,imax, jmin,jmax,
  // kmin,kmax).
  vtkGetVectorMacro(Extent,int,6);

  // Description:
  // Specify (imin,imax, jmin,jmax, kmin,kmax) indices.
  void SetExtent(int iMin, int iMax, int jMin, int jMax, int kMin, int kMax);

  // Description:
  // Specify (imin,imax, jmin,jmax, kmin,kmax) indices in array form.
  void SetExtent(int *extent);

protected:
  vtkRectilinearGridGeometryFilter();
  ~vtkRectilinearGridGeometryFilter() {};
  vtkRectilinearGridGeometryFilter(const vtkRectilinearGridGeometryFilter&) {};
  void operator=(const vtkRectilinearGridGeometryFilter&) {};

  void Execute();
  int Extent[6];
};

#endif


