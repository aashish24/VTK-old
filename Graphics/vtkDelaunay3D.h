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

#include "vtkPointSetFilter.h"
#include "vtkUnstructuredGrid.h"

class vtkSphereArray;

class VTK_EXPORT vtkDelaunay3D : public vtkPointSetFilter
{
public:
  vtkDelaunay3D();
  ~vtkDelaunay3D();
  const char *GetClassName() {return "vtkDelaunay3D";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct object with Alpha = 0.0; Tolerance = 0.001; Offset = 2.5;
  // BoundingTriangulation turned off.
  static vtkDelaunay3D *New() {return new vtkDelaunay3D;};

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
  // Get the output of this filter.
  vtkUnstructuredGrid *GetOutput() {
    return (vtkUnstructuredGrid *)this->Output;};

  // Description:
  // Set / get a spatial locator for merging points. By default, 
  // an instance of vtkMergePoints is used.
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
  // Return the MTime also considering the locator.
  unsigned long GetMTime();

  // Description:
  // For legacy compatability. Do not use.
  void SetLocator(vtkPointLocator& locator) {this->SetLocator(&locator);};  
  vtkUnstructuredGrid *InitPointInsertion(int numPtsToInsert,  int numTetra,
					  vtkPoints &boundingTetraPts, 
					  float bounds[6], vtkPoints* &pts) 
  {return this->InitPointInsertion(numPtsToInsert, numTetra, 
				   &boundingTetraPts, bounds, pts);};
  void InsertPoint(vtkUnstructuredGrid *Mesh, vtkPoints *points,
		   int id, float x[3], vtkIdList &holeTetras) {
    this->InsertPoint(Mesh, points, id, x, &holeTetras);};
  
  
    
protected:
  void Execute();

  float Alpha;
  float Tolerance;
  int BoundingTriangulation;
  float Offset;

  vtkPointLocator *Locator;  //help locate points faster
  
  vtkSphereArray *Spheres;   //used to keep track of circumspheres
  int InSphere(float x[3], int tetraId);
  void InsertSphere(vtkUnstructuredGrid *Mesh, vtkPoints *pts, int tetraId);

  int NumberOfDuplicatePoints; //keep track of bad data
  int NumberOfDegeneracies;

  int FindEnclosingFaces(float x[3], int tetra, vtkUnstructuredGrid *Mesh,
			 vtkPoints *points, float tol,
			 vtkIdList *tetras, vtkIdList *faces,
			 vtkPointLocator *Locator);
  
  int FindTetra(float x[3], int ptIds[4], float p[4][3], 
		int tetra, vtkUnstructuredGrid *Mesh, 
		vtkPoints *points, float tol, int depth);

};

#endif


