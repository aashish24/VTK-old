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
// .NAME vtkPointLocator2D - quickly locate points in 3-space
// .SECTION Description
// vtkPointLocator2D is a spatial search object to quickly locate points in 3D.
// vtkPointLocator2D works by dividing a specified region of space into a regular
// array of "rectangular" buckets, and then keeping a list of points that 
// lie in each bucket. Typical operation involves giving a position in 3D 
// and finding the closest point.
//
// vtkPointLocator2D has two distinct methods of interaction. In the first
// method, you suppy it with a dataset, and it operates on the points in 
// the dataset. In the second method, you supply it with an array of points,
// and the object operates on the array.

// .SECTION Caveats
// Many other types of spatial locators have been developed such as 
// octrees and kd-trees. These are often more efficient for the 
// operations described here.

// .SECTION See Also
// vtkCellPicker vtkPointPicker

#ifndef __vtkPointLocator2D_h
#define __vtkPointLocator2D_h

#include "vtkLocator.h"
#include "vtkPoints.h"
#include "vtkIdList.h"

class vtkNeighborPoints;

class VTK_EXPORT vtkPointLocator2D : public vtkLocator
{
public:
  // Description:
  // Construct with automatic computation of divisions, averaging
  // 25 points per bucket.
  static vtkPointLocator2D *New() {return new vtkPointLocator2D;};

  const char *GetClassName() {return "vtkPointLocator2D";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the number of divisions in x-y directions.
  vtkSetVector2Macro(Divisions,int);
  vtkGetVectorMacro(Divisions,int,2);

  // Description:
  // Specify the average number of points in each bucket.
  vtkSetClampMacro(NumberOfPointsPerBucket,int,1,VTK_LARGE_INTEGER);
  vtkGetMacro(NumberOfPointsPerBucket,int);

  // Description:
  // Given a position x, return the id of the point closest to it.
  virtual int FindClosestPoint(float x[2]);

  // Description:
  // Determine whether point given by x[3] has been inserted into points list.
  // Return id of previously inserted point if this is true, otherwise return
  // -1.
  virtual int IsInsertedPoint(float x[2]);

  // Description:
  // Find the closest N points to a position. This returns the closest
  // N points to a position. A faster method could be created that returned
  // N close points to a position, but neccesarily the exact N closest.
  // The returned points are sorted from closest to farthest.
  virtual void FindClosestNPoints(int N, float x[2], vtkIdList *result);
  virtual void FindClosestNPoints(int N, float x, float y,
				  vtkIdList *result);

  // Description:
  // Find the closest points to a position such that each quadrant of
  // space around the position contains at least N points. Loosely 
  // limit the search to a maximum number of points evalualted, M. 
  virtual void FindDistributedPoints(int N, float x[2], 
				     vtkIdList *result, int M);
  virtual void FindDistributedPoints(int N, float x, float y, 
				     vtkIdList *result, int M);

  // Description:
  // Find all points within a specified radius R of position x.
  // The result is not sorted in any specific manner.
  virtual void FindPointsWithinRadius(float R, float x[2], vtkIdList *result);
  virtual void FindPointsWithinRadius(float R, float x, float y,
				      vtkIdList *result);
  // Description:
  // See vtkLocator interface documentation.
  void Initialize();
  void FreeSearchStructure();
  void BuildLocator();
  void GenerateRepresentation(int level, vtkPolyData *pd);

  // Description:
  // set the points to use when looking up a coordinate
  vtkSetObjectMacro(Points,vtkPoints);
  vtkGetObjectMacro(Points,vtkPoints);
  
protected:
  vtkPointLocator2D();
  ~vtkPointLocator2D();
  vtkPointLocator2D(const vtkPointLocator2D&) {};
  void operator=(const vtkPointLocator2D&) {};

  // place points in appropriate buckets
  void GetBucketNeighbors(int ijk[2], int ndivs[2], int level);
  void GetOverlappingBuckets(float x[2], int ijk[2], float dist, int level);
  void GenerateFace(int face, int i, int j, int k, 
                    vtkPoints *pts, vtkCellArray *polys);

  vtkPoints *Points; // Used for merging points
  int Divisions[2]; // Number of sub-divisions in x-y-z directions
  int NumberOfPointsPerBucket; //Used with previous boolean to control subdivide
  float Bounds[4]; // bounds of points
  vtkIdList **HashTable; // lists of point ids in buckets
  int NumberOfBuckets; // total size of hash table
  float H[2]; // width of each bucket in x-y-z directions
  vtkNeighborPoints *Buckets;
  float InsertionTol2;
};

#endif


