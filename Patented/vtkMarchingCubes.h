/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


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
// .NAME vtkMarchingCubes - generate iso-surface(s) from volume
// .SECTION Description
// vtkMarchingCubes is a filter that takes as input a volume (e.g., 3D
// structured point set) and generates on output one or more iso-surfaces.
// One or more contour values must be specified to generate the iso-surfaces.
// Alternatively, you can specify a min/max scalar range and the number of
// contours to generate a series of evenly spaced contour values. The current
// implementation requires that the scalar data is defined with "short int"
// data values.
// .SECTION Caveats
// The output primitives are disjoint - that is, points may
// be generated that are coincident but distinct. You may want to use
// vtkCleanPolyData to remove the coincident points. 
// .SECTION See Also
// This filter is specialized to volumes. If you are interested in 
// contouring other types of data, use the general vtkContourFilter.

#ifndef __vtkMarchingCubes_h
#define __vtkMarchingCubes_h

#include "vtkStructuredPointsToPolyDataFilter.hh"

#define VTK_MAX_CONTOURS 256

class vtkMarchingCubes : public vtkStructuredPointsToPolyDataFilter
{
public:
  vtkMarchingCubes();
  char *GetClassName() {return "vtkMarchingCubes";};
  void PrintSelf(ostream& os, vtkIndent indent);

  void SetValue(int i, float value);

  // Description:
  // Return array of contour values (size of numContours).
  vtkGetVectorMacro(Values,float,VTK_MAX_CONTOURS);

  void GenerateValues(int numContours, float range[2]);
  void GenerateValues(int numContours, float range1, float range2);

protected:
  void Execute();

  float Values[VTK_MAX_CONTOURS];
  int NumberOfContours;
  float Range[2];
};

#endif


