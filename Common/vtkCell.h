/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1995 Ken Martin, Will Schroeder, Bill Lorensen.

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
// .NAME vtkCell - abstract class to specify cell behavior
// .SECTION Description
// vtkCell is an abstract class that specifies the interfaces for data cells.
// Data cells are simple topological elements like points, lines, polygons, 
// and tetrahedra that visualization datasets are composed of. In some 
// cases visualization datasets may explicitly represent cells (e.g., 
// vtkPolyData, vtkUnstructuredGrid), and in some cases, the datasets are 
// implicitly composed of cells (e.g., vtkStructuredPoints).
// .SECTION Caveats
// The #define parameter MAX_CELL_SIZE represents the maximum number of points
// that a cell can have. This parameter is used throughout the code to specify
// sizes of arrays and other structures. As a programmer you must make sure
// that you do not create cells with more than this number of points. (The 
// problem usually comes in with variable length objects like polylines, 
// triangle strips, or polygons).

#ifndef __vtkCell_h
#define __vtkCell_h

#define MAX_CELL_SIZE 512
#define TOL 1.e-05 // Tolerance for geometric calculation

#include "vtkObject.hh"
#include "vtkFloatPoints.hh"
#include "vtkFloatScalars.hh"
#include "vtkIdList.hh"
#include "vtkCellType.hh"

class vtkCellArray;

class vtkCell : public vtkObject
{
public:
  vtkCell();
  void Initialize(int npts, int *pts, vtkPoints *p);
  char *GetClassName() {return "vtkCell";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create concrete copy of this cell.
  virtual vtkCell *MakeObject() = 0;

  // Description:
  // Return the type of cell.
  virtual int GetCellType() = 0;

  // Description:
  // Return the topological dimensional of the cell (0,1,2, or 3).
  virtual int GetCellDimension() = 0;

  // Description:
  // Get the point coordinates for the cell.
  vtkFloatPoints *GetPoints() {return &this->Points;};

  // Description:
  // Return the number of points in the cell.
  int GetNumberOfPoints() {return this->PointIds.GetNumberOfIds();};

  // Description:
  // Return the number of edges in the cell.
  virtual int GetNumberOfEdges() = 0;

  // Description:
  // Return the number of faces in the cell.
  virtual int GetNumberOfFaces() = 0;

  // Description:
  // Return the list of point ids defining cell.
  vtkIdList *GetPointIds() {return &this->PointIds;};

  // Description:
  // For cell point i, return the actual point id.
  int GetPointId(int ptId) {return this->PointIds.GetId(ptId);};

  // Description:
  // Return the edge cell from the edgeId of the cell.
  virtual vtkCell *GetEdge(int edgeId) = 0;

  // Description:
  // Return the face cell from the faceId of the cell.
  virtual vtkCell *GetFace(int faceId) = 0;

  // Description:
  // Given parametric coordinates of a point, return the closest cell boundary,
  // and whether the point is inside or outside of the cell. The cell boundary 
  // is defined by a list of points (pts) that specify a face (3D cell), edge 
  // (2D cell), or vertex (1D cell). If the return value of the method is != 0, 
  // then the point is inside the cell.
  virtual int CellBoundary(int subId, float pcoords[3], vtkIdList& pts) = 0;

  // Description:
  // Given a point x[3] return inside(=1) or outside(=0) cell; evaluate 
  // parametric coordinates, sub-cell id (!=0 only if cell is composite),
  // distance squared of point x[3] to cell (in particular, the sub-cell 
  // indicated), closest point on cell to x[3], and interpolation weights 
  // in cell. Note: on rare occasions a -1 is returned from the method. This 
  // means that numerical error has occured and all data returned from this method
  // should be ignored. Also, inside/outside is determine parametrically. That
  // is, a point is inside if it satisfies parametric limits. This can cause
  // problems for cells of topological dimension 2 or less, since a point in
  // 3D can project onto the cell within parametric limits but be "far" from
  // the cell. Thus the value dist2 may be checked to determine true in/out.
  virtual int EvaluatePosition(float x[3], float closestPoint[3], 
                               int& subId, float pcoords[3], 
                               float& dist2, float weights[MAX_CELL_SIZE]) = 0;

  // Description:
  // Determine global coordinate from subId and parametric coordinates
  virtual void EvaluateLocation(int& subId, float pcoords[3], 
                                float x[3], float weights[MAX_CELL_SIZE]) = 0;

  // Description:
  // Generate contouring primitives.
  virtual void Contour(float value, vtkFloatScalars *cellScalars, 
                       vtkFloatPoints *points, vtkCellArray *verts, 
                       vtkCellArray *lines, vtkCellArray *polys, 
                       vtkFloatScalars *scalars) = 0;

  // Description:
  // Intersect with a ray. Return parametric coordinates (both line and cell)
  // and global intersection coordinates given ray definition and tolerance. 
  // The method returns non-zero value if intersection occurs.
  virtual int IntersectWithLine(float p1[3], float p2[3], float tol, float& t,
                                float x[3], float pcoords[3], int& subId) = 0;

  // Description:
  // Generate simplices of proper dimension. If cell is 3D, tetrahedron are 
  // generated; if 2D triangles; if 1D lines; if 0D points. The form of the
  // output is a sequence of points, each n+1 points (where n is topological 
  // cell dimension) defining a simplex. The index is a parameter that controls
  // which triangulation to use (if more than one is possible). If numerical
  // degeneracy encountered, 0 is returned, otherwise 1 is returned.
  virtual int Triangulate(int index, vtkFloatPoints &pts) = 0;

  // Description:
  // Compute derivatives given cell subId and parametric coordinates. The values
  // array is a series of data value(s) at the cell points. There is a one-to-one
  // correspondance between cell point and data value(s). Dim is the number of 
  // data values per cell point. Derivs are derivaties in the x-y-z coordinate
  // directions for each data value. Thus, if computing derivatives for a 
  // scalar function in a hexahedron, dim=1, 8 values are supplied, and 3 deriv
  // values are returned (i.e., derivatives in x-y-z directions). On the other 
  // hand, if computing derivates of velocity (vx,vy,vz) dim=3, 24 values are
  // supplied ((vx,vy,vz)1, (vx,vy,vz)2, ....()8), and 9 deriva values are
  // returned ((d(vx)/dx),(d(vx)/dy),(d(vx)/dz), (d(vy)/dx),(d(vy)/dy),
  // (d(vy)/dz), (d(vz)/dx),(d(vz)/dy),(d(vz)/dz)).
  virtual void Derivatives(int subId, float pcoords[3], float *values, 
                           int dim, float *derivs) = 0;

  void GetBounds(float bounds[6]);
  float *GetBounds();
  float GetLength2();

  // Quick intersection of cell bounding box.  Returns != 0 for hit.
  char HitBBox(float bounds[6], float origin[3], float dir[3], 
               float coord[3], float& t);

  // left public for quick computational access
  vtkFloatPoints Points;
  vtkIdList PointIds;

};

#endif


