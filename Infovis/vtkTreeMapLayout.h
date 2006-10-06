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
// .NAME vtkTreeMapLayout - layout a vtkTree into a tree map
//
// .SECTION Description
//
// .SECTION Thanks
// Thanks to Brian Wylie and Ken Moreland from Sandia National Laboratories
// for help developing this class.

#ifndef __vtkTreeMapLayout_h
#define __vtkTreeMapLayout_h


#include "vtkTreeAlgorithm.h"

class vtkTreeMapLayoutStrategy;

class VTK_INFOVIS_EXPORT vtkTreeMapLayout : public vtkTreeAlgorithm 
{
public:
  static vtkTreeMapLayout *New();

  vtkTypeRevisionMacro(vtkTreeMapLayout,vtkTreeAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // The field name to use for storing the rectangles for each node.
  // The rectangles are stored in a quadruple float array 
  // (minX, maxX, minY, maxY).
  vtkGetStringMacro(RectanglesFieldName);
  vtkSetStringMacro(RectanglesFieldName);

  // Description:
  // The strategy to use when laying out the tree map.
//  vtkGetObjectMacro(LayoutStrategy, vtkTreeMapLayoutStrategy);
//  vtkSetObjectMacro(LayoutStrategy, vtkTreeMapLayoutStrategy);
  virtual vtkTreeMapLayoutStrategy *GetLayoutStrategy() { return (vtkTreeMapLayoutStrategy *) 0; }
  virtual void SetLayoutStrategy(vtkTreeMapLayoutStrategy * vtkNotUsed(p)) { }

  // Description:
  // Returns the node id that contains pnt (or -1 if no one contains it)
  vtkIdType FindNode(float pnt[2], float *binfo=0);
  
  // Description:
  // Return the min and max 2D points of the 
  // node's bounding box
  void GetBoundingBox(vtkIdType id, float *binfo);

  // Description:
  // Get the modification time of the layout algorithm.
  virtual unsigned long GetMTime();

protected:
  vtkTreeMapLayout();
  ~vtkTreeMapLayout();

  char * RectanglesFieldName;
  vtkTreeMapLayoutStrategy* LayoutStrategy;

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  
private:

  vtkTreeMapLayout(const vtkTreeMapLayout&);  // Not implemented.
  void operator=(const vtkTreeMapLayout&);  // Not implemented.
};

#endif
