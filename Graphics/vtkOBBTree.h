/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to Peter C. Everett <pce@world.std.com> for
             improvements and enhancements to vtkOBBTree class.


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
// .NAME vtkOBBTree - generate oriented bounding box (OBB) tree
// .SECTION Description
// vtkOBBTree is an object to generate oriented bounding box (OBB) trees.
// An oriented bounding box is a bounding box that does not necessarily line 
// up along coordinate axes. The OBB tree is a hierarchical tree structure 
// of such boxes, where deeper levels of OBB confine smaller regions of space.
//
// To build the OBB, a recursive, top-down process is used. First, the root OBB
// is constructed by finding the mean and covariance matrix of the cells (and
// their points) that define the dataset. The eigenvectors of the covariance
// matrix are extracted, giving a set of three orthogonal vectors that define 
// the tightest-fitting OBB. To create the two children OBB's, a split plane 
// is found that (approximately) divides the number cells in half. These are 
// then assigned to the children OBB's. This process then continues until
// the MaxLevel ivar limits the recursion, or no split plane can be found.
//
// A good reference for OBB-trees is Gottschalk & Manocha in Proceedings of 
// Siggraph `96.

// .SECTION Caveats
// Since this algorithms works from a list of cells, the OBB tree will only 
// bound the "geometry" attached to the cells if the convex hull of the 
// cells bounds the geometry.
//
// Long, skinny cells (i.e., cells with poor aspect ratio) may cause 
// unsatisfactory results. This is due to the fact that this is a top-down
// implementation of the OBB tree, requiring that one or more complete cells
// are contained in each OBB. This requirement makes it hard to find good 
// split planes during the recurion process. A bottom-up implementation would
// go a long way to correcting this problem.

// .SECTION See Also
// vtkLocator vtkCellLocator vtkLocatorFilter

#ifndef __vtkOBBTree_h
#define __vtkOBBTree_h

#include "vtkCellLocator.h"
#include "vtkMatrix4x4.h"

// Special class defines node for the OBB tree
//
//BTX
//
class vtkOBBNode { //;prevent man page generation
public:
  vtkOBBNode();
  ~vtkOBBNode();

  float Corner[3]; //center point of this node
  float Axes[3][3]; //the axes defining the OBB - ordered from long->short
  vtkOBBNode *Parent; //parent node; NULL if root
  vtkOBBNode **Kids; //two children of this node; NULL if leaf
  vtkIdList *Cells; //list of cells in node
  void DebugPrintTree( int level, double *leaf_vol, int *minCells,
                       int *maxCells );
};
//ETX
//

class VTK_EXPORT vtkOBBTree : public vtkCellLocator
{
public:
  const char *GetClassName() {return "vtkOBBTree";};

  // Description:
  // Construct with automatic computation of divisions, averaging
  // 25 cells per octant.
  static vtkOBBTree *New() {return new vtkOBBTree;};

  // Description:
  // Compute an OBB from the list of points given. Return the corner point
  // and the three axes defining the orientation of the OBB. Also return
  // a sorted list of relative "sizes" of axes for comparison purposes.
  void ComputeOBB(vtkPoints *pts, float corner[3], float max[3], 
                  float mid[3], float min[3], float size[3]);

  // Description:
  // Compute an OBB for the input dataset using the cells in the data.
  // Return the corner point and the three axes defining the orientation
  // of the OBB. Also return a sorted list of relative "sizes" of axes for
  // comparison purposes.
  void ComputeOBB(vtkDataSet *input, float corner[3], float max[3],
                            float mid[3], float min[3], float size[3]);

  int IntersectWithLine(float a0[3], float a1[3], float tol,
                        float& t, float x[3], float pcoords[3],
                        int &subId);

  int IntersectWithLine(float a0[3], float a1[3], float tol,
                        float& t, float x[3], float pcoords[3],
                        int &subId, int &cellId);

  //BTX
  // Description:
  // Returns true if nodeB and nodeA are disjoint after optional
  // transformation of nodeB with matrix XformBtoA
  //BTX
  int DisjointOBBNodes( vtkOBBNode *nodeA, vtkOBBNode *nodeB,
                        vtkMatrix4x4 *XformBtoA );

  // Description:
  // Returns true if line intersects node.
  int LineIntersectsNode( vtkOBBNode *pA, float B0[3], float B1[3] );

  // Description:
  // Returns true if triangle (optionally transformed) intersects node.
  int TriangleIntersectsNode( vtkOBBNode *pA,
                                     float p0[3], float p1[3],
                                     float p2[3], vtkMatrix4x4 *XformBtoA );

  // Description:
  // For each intersecting leaf node pair, call function.
  // OBBTreeB is optionally transformed by XformBtoA before testing.
  int IntersectWithOBBTree( vtkOBBTree *OBBTreeB, vtkMatrix4x4 *XformBtoA,
                            int(*function)( vtkOBBNode *nodeA,
                                            vtkOBBNode *nodeB,
                                            vtkMatrix4x4 *Xform,
                                            void *arg ),
                            void *data_arg );
  //ETX

  // Description:
  // Satisfy locator'a abstract interface, see vtkLocator.
  void FreeSearchStructure();
  void BuildLocator();

  // Description:
  // Create polygonal representation for OBB tree at specified level. If
  // level < 0, then the leaf OBB nodes will be gathered. The aspect ratio (ar)
  // and line diameter (d) are used to control the building of the
  // representation. If a OBB node edge ratio's are greater than ar, then the
  // dimension of the OBB is collapsed (OBB->plane->line). A "line" OBB will be
  // represented either as two crossed polygons, or as a line, depending on
  // the relative diameter of the OBB compared to the diameter (d).
  void GenerateRepresentation(int level, vtkPolyData *pd);

  //BTX
protected:
  vtkOBBTree();
  ~vtkOBBTree();
  vtkOBBTree(const vtkOBBTree&) {};
  void operator=(const vtkOBBTree&) {};

  // Compute an OBB from the list of cells given.  This used to be
  // public but should not have been.  A public call has been added
  // so that the funtionality can be accessed.
  void ComputeOBB(vtkIdList *cells, float corner[3], float max[3], 
                       float mid[3], float min[3], float size[3]);

  vtkOBBNode *Tree;
  void BuildTree(vtkIdList *cells, vtkOBBNode *parent, int level);
  vtkPoints *PointsList;
  int *InsertedPoints;
  int OBBCount;
  int DeepestLevel;

  void DeleteTree(vtkOBBNode *OBBptr);
  void GeneratePolygons(vtkOBBNode *OBBptr, int level, int repLevel, 
                        vtkPoints* pts, vtkCellArray *polys);

  //ETX
};

#endif
