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
// .NAME vtkScalarTree - organize data according to scalar values (used to accelerate contouring operations)
// .SECTION Description
// vtkScalarTree creates a pointerless binary tree that helps search for
// cells that lie within a particular scalar range. This object is used to
// accelerate some contouring (and other scalar-baed techniques).
// 
// The tree consists of an array of (min,max) scalar range pairs per node in
// the tree. The (min,max) range is determined from looking at the range of
// the children of the tree node. If the node is a leaf, then the range is
// determined by scanning the range of scalar data in n cells in the
// dataset. The n cells are determined by arbitrary selecting cell ids from
// id(i) to id(i+n), and where n is specified using the BranchingFactor
// ivar. Note that leaf node i=0 contains the scalar range computed from
// cell ids (0,n-1); leaf node i=1 contains the range from cell ids (n,2n-1);
// and so on. The implication is that there are no direct lists of cell ids
// per leaf node, instead the cell ids are implicitly known.

#ifndef __vtkScalarTree_h
#define __vtkScalarTree_h

#include "vtkObject.h"
#include "vtkDataSet.h"

typedef struct _vtkScalarRange
  {
  float min;
  float max;
  } vtkScalarRange;


class VTK_EXPORT vtkScalarTree : public vtkObject
{
public:

// Description:
// Instantiate scalar tree with maximum level of 20 and branching
// factor of 5.
  vtkScalarTree();

  ~vtkScalarTree();
  static vtkScalarTree *New() {return new vtkScalarTree;};
  const char *GetClassName() {return "vtkScalarTree";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Build the tree from the points/cells defining this dataset.
  vtkSetObjectMacro(DataSet,vtkDataSet);
  vtkGetObjectMacro(DataSet,vtkDataSet);

  // Description:
  // Set the branching factor for the tree. This is the number of
  // children per tree node. Smaller values (minimum is 2) mean deeper
  // trees and more memory overhead. Larger values mean shallower
  // trees, less memory usage, but worse performance.
  vtkSetClampMacro(BranchingFactor,int,2,VTK_LARGE_INTEGER);
  vtkGetMacro(BranchingFactor,int);

  // Description:
  // Get the level of the locator (determined automatically if Automatic is 
  // true). The value of this ivar may change each time the locator is built.
  vtkGetMacro(Level,int);

  // Description:
  // Set the maximum allowable level for the tree. 
  vtkSetClampMacro(MaxLevel,int,1,VTK_LARGE_INTEGER);
  vtkGetMacro(MaxLevel,int);

  // Methods control building of tree

// Description:
// Construct the scalar tree from the dataset provided. Checks build times
// and modified time from input and reconstructs the tree if necessaery.
  void BuildTree();


// Description:
// Initialize locator. Frees memory and resets object as appropriate.
  void Initialize();


  // Methods provided for traversing cells based on scalar value

// Description:
// Begin to traverse the cells based on a scalar value. Returned cells
// will have scalar values that span the scalar value specified.
  void InitTraversal(float scalarValue);


// Description:
// Return the next cell that may contain scalar value specified to
// initialize traversal. The value NULL is returned if the list is
// exhausted. Make sure that InitTraversal() has been invoked first or
// you'll get erratic behavior.
  vtkCell *GetNextCell(int& cellId, vtkIdList* &ptIds,
                       vtkScalars& cellScalars);


protected:
  vtkDataSet *DataSet;
  vtkScalars *Scalars;
  int MaxLevel;
  int Level;
  int BranchingFactor; //number of children per node

  vtkScalarRange *Tree; //pointerless scalar range tree
  int TreeSize; //allocated size of tree
  vtkTimeStamp BuildTime; //time at which tree was built

private:
  float ScalarValue; //current scalar value for traversal
  int TreeIndex; //traversal location within tree
  int LeafOffset; //offset to leaf nodes of tree
  int ChildNumber; //current child in traversal
  int CellId; //current cell id being examined
  int FindStartLeaf(int index, int level);
  int FindNextLeaf(int index,int level);
};

#endif


