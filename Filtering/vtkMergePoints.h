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
// .NAME vtkMergePoints - merge exactly coincident points
// .SECTION Description
// vtkMergePoints is a locator object to quickly locate points in 3D.
// The primary difference between vtkMergePoints and its superclass
// vtkPointLocator is that vtkMergePoints merges precisely coincident points
// and is therefore much faster.
// .SECTION See Also
// vtkCleanPolyData

#ifndef __vtkMergePoints_h
#define __vtkMergePoints_h

#include "vtkPointLocator.h"

class VTK_EXPORT vtkMergePoints : public vtkPointLocator
{
public:
  static vtkMergePoints *New();
  vtkTypeMacro(vtkMergePoints,vtkPointLocator);

  // Description:
  // Determine whether point given by x[3] has been inserted into points list.
  // Return id of previously inserted point if this is true, otherwise return
  // -1.
  int IsInsertedPoint(float x[3]);
  int IsInsertedPoint(float x, float  y, float z)
    {return this->vtkPointLocator::IsInsertedPoint(x, y, z); };

  // Description:
  // Determine whether point given by x[3] has been inserted into points list.
  // Return 0 if point was already in the list, otherwise return 1. If the
  // point was not in the list, it will be ADDED.  In either case, the id of
  // the point (newly inserted or not) is returned in the ptId argument.
  // Note this combines the functionality of IsInsertedPoint() followed
  // by a call to InsertNextPoint().
  int InsertUniquePoint(float x[3], int &ptId);
  
protected:
  vtkMergePoints() {};
  ~vtkMergePoints() {};
  vtkMergePoints(const vtkMergePoints&) {};
  void operator=(const vtkMergePoints&) {};
  
};

#endif


