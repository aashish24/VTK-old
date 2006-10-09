/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSliceAndDiceLayoutStrategy - a horizontal and vertical slicing tree map layout
//
// .SECTION Description
//
// .SECTION Thanks
// Slice and dice algorithm comes from:
// Shneiderman, B. 1992. Tree visualization with tree-maps: 2-d space-filling approach. 
// ACM Trans. Graph. 11, 1 (Jan. 1992), 92-99. 

#ifndef __vtkSliceAndDiceLayoutStrategy_h
#define __vtkSliceAndDiceLayoutStrategy_h

#include "vtkTreeMapLayoutStrategy.h"

class VTK_INFOVIS_EXPORT vtkSliceAndDiceLayoutStrategy : public vtkTreeMapLayoutStrategy 
{
public:
  static vtkSliceAndDiceLayoutStrategy *New();

  vtkTypeRevisionMacro(vtkSliceAndDiceLayoutStrategy,vtkTreeMapLayoutStrategy);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkGetStringMacro(SizeFieldName);
  vtkSetStringMacro(SizeFieldName);
  void Layout(vtkTree *inputTree, vtkDataArray *coordsArray);

protected:
  vtkSliceAndDiceLayoutStrategy();
  ~vtkSliceAndDiceLayoutStrategy();

private:
  vtkSliceAndDiceLayoutStrategy(const vtkSliceAndDiceLayoutStrategy&);  // Not implemented.
  void operator=(const vtkSliceAndDiceLayoutStrategy&);  // Not implemented.

  char * SizeFieldName;
};

#endif

