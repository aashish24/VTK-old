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
// .NAME vtkDelaunay2D - create 2D Delaunay triangulation of input points
// .SECTION Description

// vtkDelaunay2D is a filter that constructs a 2D Delaunay triangulation from
// a list of input points. These points may be represented by any dataset of
// type vtkPointSet and subclasses. The output of the filter is a polygonal
// dataset. Usually the output is a triangle mesh, but if a non-zero alpha
// distance value is specified (called the "alpha" value), then only
// triangles, edges, and vertices lying within the alpha radius are
// output. In other words, non-zero alpha values may result in arbitrary
// combinations of triangles, lines, and vertices. (The notion of alpha value
// is derived from Edelsbrunner's work on "alpha shapes".) Also, it is
// possible to generate "constrained triangulations" using this filter.
// A constrained triangulation is one where edges and loops (i.e., polygons)
// can be defined and the triangulation will preserve them (read on for 
// more information).
//
// The 2D Delaunay triangulation is defined as the triangulation that 
// satisfies the Delaunay criterion for n-dimensional simplexes (in this case
// n=2 and the simplexes are triangles). This criterion states that a 
// circumsphere of each simplex in a triangulation contains only the n+1 
// defining points of the simplex. (See "The Visualization Toolkit" text 
// for more information.) In two dimensions, this translates into an optimal 
// triangulation. That is, the maximum interior angle of any triangle is less 
// than or equal to that of any possible triangulation.
// 
// Delaunay triangulations are used to build topological structures
// from unorganized (or unstructured) points. The input to this filter
// is a list of points specified in 3D, even though the triangulation
// is 2D. Thus the triangulation is constructed in the x-y plane, and
// the z coordinate is ignored (although carried through to the
// output). (If you desire to triangulate in a different plane, you'll
// have to use the vtkTransformFilter to transform the points into and
// out of the x-y plane.)
// 
// The Delaunay triangulation can be numerically sensitive in some cases. To
// prevent problems, try to avoid injecting points that will result in
// triangles with bad aspect ratios (1000:1 or greater). In practice this
// means inserting points that are "widely dispersed", and enables smooth
// transition of triangle sizes throughout the mesh. (You may even want to
// add extra points to create a better point distribution.) If numerical
// problems are present, you will see a warning message to this effect at
// the end of the triangulation process.
//
// To create constrained meshes, you must define an additional input. This
// input is an instance of vtkPolyData which contains lines, polylines,
// and/or polygons that define constrained edges and loops. Lines and
// polylines found in the input will be mesh edges in the output. Polygons
// define a loop with inside and outside regions. The inside of the polygon
// is determined by using the right-hand-rule, i.e., looking down the z-axis
// a polygon should be ordered counter-clockwise. Holes in a polygon should
// be ordered clockwise.If you choose to create a constrained triangulation,
// the final mesh may not satisfy the Delaunay criterion. (Noted: the
// lines/polygon edges must not intersect when projected onto the 2D plane.
// It may not be possible to recover all edges due to not enough points in
// the triangulation, or poorly defined edges (coincident or execssively
// long).  The form of the lines or polygons is a list of point ids that
// correspond to the input point ids used to generate the triangulation.)

// .SECTION Caveats
// Points arranged on a regular lattice (termed degenerate cases) can be 
// triangulated in more than one way (at least according to the Delaunay 
// criterion). The choice of triangulation (as implemented by 
// this algorithm) depends on the order of the input points. The first three
// points will form a triangle; other degenerate points will not break
// this triangle.
//
// Points that are coincident (or nearly so) may be discarded by the algorithm.
// This is because the Delaunay triangulation requires unique input points.
// You can control the definition of coincidence with the "Tolerance" instance
// variable.
//
// The output of the Delaunay triangulation is supposedly a convex hull. In 
// certain cases this implementation may not generate the convex hull. This
// behavior can be controlled by the Offset instance variable. Offset is a
// multiplier used to control the size of the initial triangulation. The 
// larger the offset value, the more likely you will generate a convex hull;
// but the more likely you are to see numerical problems.
 
// .SECTION See Also
// vtkDelaunay3D vtkTransformFilter vtkGaussianSplatter

#ifndef __vtkDelaunay2D_h
#define __vtkDelaunay2D_h

#include "vtkPointSet.h"
#include "vtkPolyDataSource.h"

class VTK_EXPORT vtkDelaunay2D : public vtkPolyDataSource
{
public:
  const char *GetClassName() {return "vtkDelaunay2D";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct object with Alpha = 0.0; Tolerance = 0.001; Offset = 1.25;
  // BoundingTriangulation turned off.
  static vtkDelaunay2D *New();

  // Description:
  // Specify the source object used to specify constrained edges and loops.
  // (This is optional.) If set, and lines/polygons are defined, a constrained
  // triangulation is created.
  void SetSource(vtkPolyData *);
  vtkPolyData *GetSource();
  
  // Description:
  // Specify alpha (or distance) value to control output of this filter.
  // For a non-zero alpha value, only edges or triangles contained within
  // a sphere centered at mesh vertices will be output. Otherwise, only
  // triangles will be output.
  vtkSetClampMacro(Alpha,double,0.0,VTK_LARGE_FLOAT);
  vtkGetMacro(Alpha,double);

  // Description:
  // Specify a tolerance to control discarding of closely spaced points.
  // This tolerance is specified as a fraction of the diagonal length of
  // the bounding box of the points.
  vtkSetClampMacro(Tolerance,double,0.0,1.0);
  vtkGetMacro(Tolerance,double);

  // Description:
  // Specify a multiplier to control the size of the initial, bounding
  // Delaunay triangulation.
  vtkSetClampMacro(Offset,double,0.75,VTK_LARGE_FLOAT);
  vtkGetMacro(Offset,double);

  // Description:
  // Boolean controls whether bounding triangulation points (and associated
  // triangles) are included in the output. (These are introduced as an
  // initial triangulation to begin the triangulation process. This feature
  // is nice for debugging output.)
  vtkSetMacro(BoundingTriangulation,int);
  vtkGetMacro(BoundingTriangulation,int);
  vtkBooleanMacro(BoundingTriangulation,int);

  // Description:
  // Set / get the input data or filter.
  virtual void SetInput(vtkPointSet *input);
  vtkPointSet *GetInput();

protected:
  vtkDelaunay2D();
  ~vtkDelaunay2D();
  vtkDelaunay2D(const vtkDelaunay2D&) {};
  void operator=(const vtkDelaunay2D&) {};

  void Execute();

  double Alpha;
  double Tolerance;
  int BoundingTriangulation;
  double Offset;

private:
  vtkPolyData *Mesh; //the created mesh
  double *Points;    //the raw points in double precision
  void SetPoint(int id, double *x)
    {int idx=3*id; 
    this->Points[idx] = x[0];
    this->Points[idx+1] = x[1];
    this->Points[idx+2] = x[2];
    }
      
  void GetPoint(int id, double x[3])
    {double *ptr = this->Points + 3*id;
    x[0] = *ptr++;
    x[1] = *ptr++;
    x[2] = *ptr;
    }

  int NumberOfDuplicatePoints;
  int NumberOfDegeneracies;

  int *RecoverBoundary();
  int RecoverEdge(int p1, int p2);
  void FillPolygons(vtkCellArray *polys, int *triUse);

  int InCircle (double x[3], double x1[3], double x2[3], double x3[3]);
  int FindTriangle(double x[3], int ptIds[3], int tri, double tol, 
                   int nei[3], vtkIdList *neighbors);
  void CheckEdge(int ptId, double x[3], int p1, int p2, int tri);

};

#endif


