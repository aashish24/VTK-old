/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkCleanPolyData - merge duplicate points and remove degenerate primitives
// .SECTION Description
// vtkCleanPolyData is a filter that takes polygonal data as input and
// generates polygonal data as output. vtkCleanPolyData merges duplicate
// points (within specified tolerance), and if enabled transforms degenerate
// topology into appropriate form (for example, triangle is converted
// into line if two points of triangle are merged). Conversion of
// degenerate forms is controlled by the flags
// ConvertLinesToPoints, ConvertPolysToLines, ConvertStripsToPolys
// which act cumulatively such that a degenerate strip may become a poly.
// The full set is
// Line with 1 points -> Vert (if ConvertLinesToPoints)
// Poly with 2 points -> Line (if ConvertPolysToLines)
// Poly with 1 points -> Vert (if ConvertPolysToLines && ConvertLinesToPoints)
// Strp with 3 points -> Poly (if ConvertStripsToPolys)
// Strp with 2 points -> Line (if ConvertStripsToPolys && ConvertPolysToLines)
// Strp with 1 points -> Vert (if ConvertStripsToPolys && ConvertPolysToLines
//   && ConvertLinesToPoints)
//
// If tolerance is specified precisely=0.0, then this object will use
// the vtkMergePoints object to merge points (very fast). Otherwise the
// slower vtkPointLocator is used.
// Before inserting points into the point locator, this class calls
// a function OperateOnPoint which can be used (in subclasses) to further
// refine the cleaning process. See vtkQuantizePolyDataPoints.

// .SECTION Caveats
// Merging points can alter topology, including introducing non-manifold
// forms. Tolerance should be chosen carefully to avoid these problems.
// Subclasses should handle OperateOnBounds as well as OperateOnPoint
// to ensure locator is correctly initialized (i.e. all modified points must
// lie inside modified bounds).
//
// .SECTION See Also
// vtkQuantizePolyDataPoints

#ifndef __vtkCleanPolyData_h
#define __vtkCleanPolyData_h

#include "vtkPolyDataToPolyDataFilter.h"

class VTK_EXPORT vtkCleanPolyData : public vtkPolyDataToPolyDataFilter
{
public:
  static vtkCleanPolyData *New();
  const char *GetClassName() {return "vtkCleanPolyData";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // By default ToleranceIsAbsolute is false and Tolerance is
  // a fraction of Bounding box diagonal, if true, AbsoluteTolerance is
  // used when adding points to locator (merging)
  vtkSetMacro(ToleranceIsAbsolute,int);
  vtkBooleanMacro(ToleranceIsAbsolute,int);
  vtkGetMacro(ToleranceIsAbsolute,int);

  // Description:
  // Specify tolerance in terms of fraction of bounding box length.
  vtkSetClampMacro(Tolerance,float,0.0,1.0);
  vtkGetMacro(Tolerance,float);

  // Description:
  // Specify tolerance in absolute terms
  vtkSetClampMacro(AbsoluteTolerance,float,0.0,VTK_LARGE_FLOAT);
  vtkGetMacro(AbsoluteTolerance,float);

  // Description:
  // Turn on/off conversion of degenerate lines to points
  vtkSetMacro(ConvertLinesToPoints,int);
  vtkBooleanMacro(ConvertLinesToPoints,int);
  vtkGetMacro(ConvertLinesToPoints,int);

  // Description:
  // Turn on/off conversion of degenerate polys to lines
  vtkSetMacro(ConvertPolysToLines,int);
  vtkBooleanMacro(ConvertPolysToLines,int);
  vtkGetMacro(ConvertPolysToLines,int);

  // Description:
  // Turn on/off conversion of degenerate strips to polys
  vtkSetMacro(ConvertStripsToPolys,int);
  vtkBooleanMacro(ConvertStripsToPolys,int);
  vtkGetMacro(ConvertStripsToPolys,int);

  // Description:
  // Set/Get a spatial locator for speeding the search process. By
  // default an instance of vtkMergePoints is used.
  void SetLocator(vtkPointLocator *locator);
  vtkGetObjectMacro(Locator,vtkPointLocator);

  // Description:
  // Create default locator. Used to create one when none is specified.
  void CreateDefaultLocator(void);

  // Description:
  // Release locator
  void ReleaseLocator(void);

  // Description:
  // Get the MTime of this object also considering the locator.
  unsigned long int GetMTime();

  // Description:
  // Perform operation on a point
  virtual void OperateOnPoint(float in[3], float out[3]);

  // Description:
  // Perform operation on bounds
  virtual void OperateOnBounds(float in[6], float out[6]);

protected:
  vtkCleanPolyData();
 ~vtkCleanPolyData();
  vtkCleanPolyData(const vtkCleanPolyData&) {};
  void operator=(const vtkCleanPolyData&) {};

  // Usual data generation method
  void Execute();

  float Tolerance;
  float AbsoluteTolerance;
  int ConvertLinesToPoints;
  int ConvertPolysToLines;
  int ConvertStripsToPolys;
  int ToleranceIsAbsolute;
  vtkPointLocator *Locator;
};

#endif


