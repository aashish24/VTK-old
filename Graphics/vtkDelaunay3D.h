/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
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
// .NAME vtkDelaunay3D - create 3D Delaunay triangulation of input points
// .SECTION Description
// vtkDelaunay3D is a filter that constructs a 3D Delaunay
// triangulation from a list of input points. These points may be
// represented by any dataset of type vtkPointSet and subclasses. The
// output of the filter is an unstructured grid dataset. Usually the
// output is a tetrahedral mesh, but if a non-zero alpha distance
// value is specified (called the "alpha" value), then only tetrahedra,
// triangles, edges, and vertices lying within the alpha radius are 
// output. In other words, non-zero alpha values may result in arbitrary
// combinations of tetrahedra, triangles, lines, and vertices. (The notion 
// of alpha value is derived from Edelsbrunner's work on "alpha shapes".)
// 
// The 3D Delaunay triangulation is defined as the triangulation that
// satisfies the Delaunay criterion for n-dimensional simplexes (in
// this case n=3 and the simplexes are tetrahedra). This criterion
// states that a circumsphere of each simplex in a triangulation
// contains only the n+1 defining points of the simplex. (See text for
// more information.) While in two dimensions this translates into an
// "optimal" triangulation, this is not true in 3D, since a measurement 
// for optimality in 3D is not agreed on.
//
// Delaunay triangulations are used to build topological structures
// from unorganized (or unstructured) points. The input to this filter
// is a list of points specified in 3D. (If you wish to create 2D 
// triangulations see vtkDelaunay2D.) The output is an unstructured grid.
// 
// The Delaunay triangulation can be numerically sensitive. To prevent
// problems, try to avoid injecting points that will result in
// triangles with bad aspect ratios (1000:1 or greater). In practice
// this means inserting points that are "widely dispersed", and
// enables smooth transition of triangle sizes throughout the
// mesh. (You may even want to add extra points to create a better
// point distribution.) If numerical problems are present, you will
// see a warning message to this effect at the end of the
// triangulation process.

// .SECTION Caveats
// Points arranged on a regular lattice (termed degenerate cases) can be 
// triangulated in more than one way (at least according to the Delaunay 
// criterion). The choice of triangulation (as implemented by 
// this algorithm) depends on the order of the input points. The first four
// points will form a tetrahedron; other degenerate points (relative to this
// initial tetrahedron) will not break it.
//
// Points that are coincident (or nearly so) may be discarded by the
// algorithm.  This is because the Delaunay triangulation requires
// unique input points.  You can control the definition of coincidence
// with the "Tolerance" instance variable.
//
// The output of the Delaunay triangulation is supposedly a convex hull. In 
// certain cases this implementation may not generate the convex hull. This
// behavior can be controlled by the Offset instance variable. Offset is a
// multiplier used to control the size of the initial triangulation. The 
// larger the offset value, the more likely you will generate a convex hull;
// and the more likely you are to see numerical problems.
//
// The implementation of this algorithm varies from the 2D Delaunay
// algorithm (i.e., vtkDelaunay2D) in an important way. When points are
// injected into the triangulation, the search for the enclosing tetrahedron
// is quite different. In the 3D case, the closest previously inserted point
// point is found, and then the connected tetrahedra are searched to find
// the containing one. (In 2D, a "walk" towards the enclosing triangle is
// performed.) If the triangulation is Delaunay, then an 

// .SECTION See Also
// vtkDelaunay2D vtkGaussianSplatter vtkUnstructuredGrid

#ifndef __vtkDelaunay3D_h
#define __vtkDelaunay3D_h

#include "vtkUnstructuredGridSource.h"

class vtkTetraArray;

class VTK_EXPORT vtkDelaunay3D : public vtkUnstructuredGridSource
{
public:
  vtkTypeMacro(vtkDelaunay3D,vtkUnstructuredGridSource);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct object with Alpha = 0.0; Tolerance = 0.001; Offset = 2.5;
  // BoundingTriangulation turned off.
  static vtkDelaunay3D *New();

  // Description:
  // Specify alpha (or distance) value to control output of this filter.
  // For a non-zero alpha value, only edges or triangles contained within
  // a sphere centered at mesh vertices will be output. Otherwise, only
  // triangles will be output.
  vtkSetClampMacro(Alpha,float,0.0,VTK_LARGE_FLOAT);
  vtkGetMacro(Alpha,float);

  // Description:
  // Specify a tolerance to control discarding of closely spaced points.
  // This tolerance is specified as a fraction of the diagonal length of
  // the bounding box of the points.
  vtkSetClampMacro(Tolerance,float,0.0,1.0);
  vtkGetMacro(Tolerance,float);

  // Description:
  // Specify a multiplier to control the size of the initial, bounding
  // Delaunay triangulation.
  vtkSetClampMacro(Offset,float,2.5,VTK_LARGE_FLOAT);
  vtkGetMacro(Offset,float);

  // Description:
  // Boolean controls whether bounding triangulation points (and associated
  // triangles) are included in the output. (These are introduced as an
  // initial triangulation to begin the triangulation process. This feature
  // is nice for debugging output.)
  vtkSetMacro(BoundingTriangulation,int);
  vtkGetMacro(BoundingTriangulation,int);
  vtkBooleanMacro(BoundingTriangulation,int);

  // Description:
  // Set / get a spatial locator for merging points. By default, 
  // an instance of vtkPointLocator is used.
  void SetLocator(vtkPointLocator *locator);
  vtkGetObjectMacro(Locator,vtkPointLocator);

  // Description:
  // Create default locator. Used to create one when none is specified. The 
  // locator is used to eliminate "coincident" points.
  void CreateDefaultLocator();

  // Description:
  // This is a helper method used with InsertPoint() to create 
  // tetrahedronalizations of points. Its purpose is construct an initial
  // Delaunay triangulation into which to inject other points. You must
  // specify the center of a cubical bounding box and its length, as well
  // as the numer of points to insert. The method returns a pointer to
  // an unstructured grid. Use this pointer to manipulate the mesh as
  // necessary. You must delete (with Delete()) the mesh when done.
  // Note: This initialization method places points forming bounding octahedron
  // at the end of the Mesh's point list. That is, InsertPoint() assumes that
  // you will be inserting points between (0,numPtsToInsert-1).
  vtkUnstructuredGrid *InitPointInsertion(float center[3], float length, 
                                          int numPts, vtkPoints* &pts);

  // Description:
  // This is a helper method used with InsertPoint() to create 
  // tetrahedronalizations of points. Its purpose is construct an initial
  // Delaunay triangulation into which to inject other points. You must
  // specify the number of points you wish to insert, and then define an
  // initial Delaunay tetrahedronalization. This is defined by specifying 
  // the number of tetrahedra, and a list of points coordinates defining
  // the tetra (total of 4*numTetra points). The method returns a pointer 
  // to an unstructured grid. Use this pointer to manipulate the mesh as
  // necessary. You must delete (with Delete()) the mesh when done.
  // Note: The points you insert using InsertPoint() will range from
  // (0,numPtsToInsert-1). Make sure that numPtsToInsert is large enough to
  // accomodate this.
  vtkUnstructuredGrid *InitPointInsertion(int numPtsToInsert,  int numTetra,
                          vtkPoints *boundingTetraPts, float bounds[6],
                          vtkPoints* &pts);
  
  // Description:
  // This is a helper method used with InitPointInsertion() to create
  // tetrahedronalizations of points. Its purpose is to inject point at
  // coordinates specified into tetrahedronalization. The point id is an index
  // into the list of points in the mesh structure.  (See
  // vtkDelaunay3D::InitPointInsertion() for more information.)  When you have
  // completed inserting points, traverse the mesh structure to extract desired
  // tetrahedra (or tetra faces and edges).The holeTetras id list lists all the
  // tetrahedra that are deleted (invalid) in the mesh structure.
  void InsertPoint(vtkUnstructuredGrid *Mesh, vtkPoints *points,
                   int id, float x[3], vtkIdList *holeTetras);

  // Description:
  // Invoke this method after all points have been inserted. The purpose of
  // the method is to clean up internal data structures. Note that the 
  // (vtkUnstructuredGrid *)Mesh returned from InitPointInsertion() is NOT
  // deleted, you still are responsible for cleaning that up.
  void EndPointInsertion();

  // Description:
  // Return the MTime also considering the locator.
  unsigned long GetMTime();

  // Description:
  // Set / get the input data or filter.
  virtual void SetInput(vtkPointSet *input);
  vtkPointSet *GetInput();

  // Description:
  // For legacy compatibility. Do not use.
  void SetLocator(vtkPointLocator& locator) {this->SetLocator(&locator);};  
  vtkUnstructuredGrid *InitPointInsertion(int numPtsToInsert,  int numTetra,
                                          vtkPoints &boundingTetraPts, 
                                          float bounds[6], vtkPoints* &pts) 
    {return this->InitPointInsertion(numPtsToInsert, numTetra, 
                                   &boundingTetraPts, bounds, pts);};
  void InsertPoint(vtkUnstructuredGrid *Mesh, vtkPoints *points,
                   int id, float x[3], vtkIdList &holeTetras) 
    {this->InsertPoint(Mesh, points, id, x, &holeTetras);}; 
    
protected:
  vtkDelaunay3D();
  ~vtkDelaunay3D();
  vtkDelaunay3D(const vtkDelaunay3D&) {};
  void operator=(const vtkDelaunay3D&) {};

  void Execute();

  float Alpha;
  float Tolerance;
  int BoundingTriangulation;
  float Offset;

  vtkPointLocator *Locator;  //help locate points faster
  
  vtkTetraArray *TetraArray; //used to keep track of circumspheres/neighbors
  int FindTetra(vtkUnstructuredGrid *Mesh, double x[3], int tetId, int depth);
  int InSphere(double x[3], int tetraId);
  void InsertTetra(vtkUnstructuredGrid *Mesh, vtkPoints *pts, int tetraId);

  int NumberOfDuplicatePoints; //keep track of bad data
  int NumberOfDegeneracies;

  // Keep track of number of references to points to avoid new/delete calls
  int *References;

  int FindEnclosingFaces(float x[3], vtkUnstructuredGrid *Mesh,
                         vtkPoints *points, vtkIdList *tetras, 
                         vtkIdList *faces, vtkPointLocator *Locator);
  
private: //members added for performance
  vtkIdList *Tetras; //used in InsertPoint
  vtkIdList *Faces;  //used in InsertPoint
  vtkIdList *BoundaryPts; //used by InsertPoint
  vtkIdList *CheckedTetras; //used by InsertPoint
  vtkIdList *NeiTetras; //used by InsertPoint

};

#endif


