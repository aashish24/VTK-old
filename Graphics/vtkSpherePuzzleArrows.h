/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSpherePuzzleArrows - Visualize permutation of the sphere puzzle.
// .SECTION Description
// vtkSpherePuzzleArrows creates 

#ifndef __vtkSpherePuzzleArrows_h
#define __vtkSpherePuzzleArrows_h

#include "vtkPolyDataSource.h"

class vtkCellArray;
class vtkPoints;
class vtkSpherePuzzle;

class VTK_EXPORT vtkSpherePuzzleArrows : public vtkPolyDataSource 
{
public:
  vtkTypeRevisionMacro(vtkSpherePuzzleArrows,vtkPolyDataSource);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  static vtkSpherePuzzleArrows *New();

  // Description:
  vtkSetVectorMacro(Permutation,int,32);
  vtkGetVectorMacro(Permutation,int,32);
  void SetPermutationComponent(int comp, int val);
  void SetPermutation(vtkSpherePuzzle *puz);

protected:
  vtkSpherePuzzleArrows();
  ~vtkSpherePuzzleArrows();

  void Execute();
  void AppendArrow(int id0, int id1, vtkPoints *pts, vtkCellArray *polys);
  
  int Permutation[32];

  float Radius;

private:
  vtkSpherePuzzleArrows(const vtkSpherePuzzleArrows&); // Not implemented
  void operator=(const vtkSpherePuzzleArrows&); // Not implemented
};

#endif


