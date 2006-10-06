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
// .NAME vtkGraphDFSIterator - depth first search iterator through a vtkGraph
//
// .SECTION Description
//


#ifndef __vtkGraphDFSIterator_h
#define __vtkGraphDFSIterator_h

#include "vtkObject.h"

class vtkGraph;
class vtkGraphDFSIteratorInternals;
class vtkIntArray;
class vtkIdList;

class VTK_FILTERING_EXPORT vtkGraphDFSIterator : public vtkObject
{
public:
  static vtkGraphDFSIterator* New();
  vtkTypeRevisionMacro(vtkGraphDFSIterator, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  //BTX
  enum ModeType
    {
    DISCOVER,
    FINISH
    };
  //ETX

  // Description:
  // Set the graph to iterate over.
  void SetGraph(vtkGraph* graph);

  // Description:
  // Set the visit mode of the iterator.  Mode can be
  //   DISCOVER (0): Order by discovery time
  //   FINISH   (1): Order by finish time
  // Default is DISCOVER.
  // Use DISCOVER for top-down algorithms where parents need to be processed before children.
  // Use FINISH for bottom-up algorithms where children need to be processed before parents.
  void SetMode(int mode);

  // Description:
  // The start node of the search.
  // If not set (or set to a negative value), starts at the node with index 0 in a vtkGraph,
  // or the root of a vtkTree.
  void SetStartNode(vtkIdType node);

  vtkIdType Next();
  bool HasNext();
protected:
  vtkGraphDFSIterator();
  ~vtkGraphDFSIterator();

  void Initialize();
  vtkIdType NextInternal();

  vtkGraph* Graph;
  int Mode;
  vtkIdType StartNode;
  vtkIdType CurRoot;
  vtkGraphDFSIteratorInternals* Internals;
  vtkIntArray* Color;
  vtkIdType NumBlack;
  vtkIdType NextId;

  //BTX
  enum ColorType
    {
    WHITE,
    GRAY,
    BLACK
    };
  //ETX

private:
  vtkGraphDFSIterator(const vtkGraphDFSIterator &);  // Not implemented.
  void operator=(const vtkGraphDFSIterator &);        // Not implemented.
};


#endif

