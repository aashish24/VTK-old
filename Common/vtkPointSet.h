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
// .NAME vtkPointSet - abstract class for specifying dataset behavior
// .SECTION Description
// vtkPointSet is an abstract class that specifies the interface for 
// datasets that explicitly use "point" arrays to represent geometry.
// For example, vtkPolyData and vtkUnstructuredGrid require point arrays
// to specify point position, while vtkStructuredPoints generates point
// positions implicitly.

// .SECTION See Also
// vtkPolyData vtkStructuredGrid vtkUnstructuredGrid

#ifndef __vtkPointSet_h
#define __vtkPointSet_h

#include "vtkDataSet.h"
#include "vtkPointLocator.h"

class VTK_EXPORT vtkPointSet : public vtkDataSet
{
public:
  vtkPointSet();
  ~vtkPointSet();
  vtkPointSet(const vtkPointSet& ps);
  const char *GetClassName() {return "vtkPointSet";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reset to an empty state and free any memory.
  void Initialize();

  // Description:
  // Copy the geometric structure of an input point set object.
  void CopyStructure(vtkDataSet *pd);

  
  // Description:
  // See vtkDataSet for additional information.
  int GetNumberOfPoints();
  float *GetPoint(int ptId) {return this->Points->GetPoint(ptId);};
  void GetPoint(int ptId, float x[3]) {this->Points->GetPoint(ptId,x);};
  int FindPoint(float x[3]);
  int FindCell(float x[3], vtkCell *cell, int cellId, float tol2, int& subId, 
               float pcoords[3], float *weights);

  // Description:
  // Get MTime which also considers its vtkPoints MTime.
  unsigned long GetMTime();

  // Description:
  // Compute the (X, Y, Z)  bounds of the data.
  void ComputeBounds();
  
  // Description:
  // Reclaim any unused memory.
  void Squeeze();

  // Description:
  // Specify point array to define point coordinates.
  vtkSetObjectMacro(Points,vtkPoints);
  vtkGetObjectMacro(Points,vtkPoints);

  // Description:
  // Detect refernce loop PointSet <-> locator.
  void UnRegister(vtkObject *o);
  
protected:
  vtkPoints *Points;
  vtkPointLocator *Locator;

};

inline int vtkPointSet::GetNumberOfPoints()
{
  if (this->Points)
    {
    return this->Points->GetNumberOfPoints();
    }
  else
    {
    return 0;
    }
}


#endif


