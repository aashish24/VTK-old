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
// .NAME vtkEdgeTable - keep track of edges (edge is pair of integer id's)
// .SECTION Description
// vtkEdgeTable is a general object for keeping track of lists of edges. An
// edge is defined by the pair of point id's (p1,p2). Methods are available
// to insert edges, check if edges exist, and traverse the list of edges.
// Also, it's possible to associate attribute information with each edge.

#ifndef __vtkEdgeTable_h
#define __vtkEdgeTable_h

#include "vtkObject.h"
#include "vtkIdList.h"
class vtkPoints;

class VTK_EXPORT vtkEdgeTable : public vtkObject
{
public:
  // Description:
  // Instantiate object assuming that 1000 edges are to be inserted.
  static vtkEdgeTable *New() {return new vtkEdgeTable;};

  const char *GetClassName() {return "vtkEdgeTable";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Free memory and return to instantiated state.
  void Initialize();

  // Description:
  // Initialize the edge insertion process. Provide an estimate of the
  // number of points in a dataset (the maximum range value of p1 or
  // p2).  The storeAttributes variable controls whether attributes
  // are to be stored with the edge. If on, additional memory will be
  // required by the data structure to store an integer id per each
  // edge.  This method is used in conjunction with one of the two
  // InsertEdge() methods described below (don't mix the InsertEdge()
  // methods).
  int InitEdgeInsertion(int numPoints, int storeAttributes=0);

  // Description:
  // Insert the edge (p1,p2) into the table. It is the user's
  // responsibility to check if the edge has already been inserted
  // (use IsEdge()). If the storAttributes flag in InitEdgeInsertion()
  // has been set, then the method returns a unique integer id (i.e.,
  // the edge id) that can be used to set and get edge
  // attributes. Otherwise, the method will return 1. Do not mix this
  // method with the InsertEdge() method that follows.
  int InsertEdge(int p1, int p2);

  // Description:
  // Insert the edge (p1,p2) into the table with the attribute id
  // specified (make sure the attributeId >= 0). Note that the
  // attributeId is ignored if the storeAttributes variable was set to
  // 0 in the InitEdgeInsertion() method. It is the user's
  // responsibility to check if the edge has already been inserted
  // (use IsEdge()). Do not mix this method with the previous
  // InsertEdge() method.
  void InsertEdge(int p1, int p2, int attributeId);

  // Description: 
  // Return an integer id for the edge, or an attributeId of the edge
  // (p1,p2) if the edge has been previously defined (it depends upon
  // which version of InsertEdge() is being used); otherwise -1. The
  // unique integer id can be used to set and retrieve attributes to
  // the edge.
  int IsEdge(int p1, int p2);

  // Description:
  // Initialize the point insertion process. The newPts is an object
  // representing point coordinates into which incremental insertion methods
  // place their data. The points are associated with the edge.
  int InitPointInsertion(vtkPoints *newPts, int estSize);

  // Description:
  // Insert a unique point on the specified edge. Invoke this method only
  // after InitPointInsertion() has been called. Return 0 if point was 
  // already in the list, otherwise return 1.
  int InsertUniquePoint(int p1, int p2, float x[3], int &ptId);

  // Description:
  // Return the number of edges that have been inserted thus far.
  vtkGetMacro(NumberOfEdges, int);

  // Description:
  // Intialize traversal of edges in table.
  void InitTraversal();

  // Description:
  // Traverse list of edges in table. Return the edge as (p1,p2), where p1
  // and p2 are point id's. Method return value is zero if list is exhausted;
  // non-zero otherwise. The value of p1 is guaranteed to be <= p2.
  int GetNextEdge(int &p1, int &p2);

protected:
  vtkEdgeTable();
  ~vtkEdgeTable();

  vtkIdList **Table;
  vtkIdList **Attributes;
  int StoreAttributes;
  int TableSize;
  int Position[2];
  int Extend;
  int NumberOfEdges;
  vtkPoints *Points; //support point insertion

  vtkIdList **Resize(int size);
};

#endif

