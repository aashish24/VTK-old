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
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkHull - produce an n-sided convex hull
// .SECTION Description
// vtkHull is a filter which will produce an n-sided convex hull given a
// set of n planes. (The convex hull bounds the input polygonal data.) 
// The planes can be defined in a number of ways including manually 
// specifying each plane; choosing the six face planes of the input's
// bounding box; choosing the eight vertex planes of the input's
// bounding box; choosing the twelve edge planes of the input's
// bounding box; and/or using a recursively subdivided octahedron.
//
// The output of this filter can be used in combination with vtkLODActor 
// to represent a levels-of-detail in the LOD hierarchy.

#ifndef __vtkHull_h
#define __vtkHull_h

#include "vtkPolyDataToPolyDataFilter.h"

class VTK_EXPORT vtkHull : public vtkPolyDataToPolyDataFilter
{
public:
  static vtkHull *New();
  vtkTypeMacro(vtkHull,vtkPolyDataToPolyDataFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Remove all planes from the current set of planes.  
  void RemoveAllPlanes( void );

  // Description:
  // Add a plane to the current set of planes. It will be added at the
  // end of the list, and an index that can later be used to set this
  // plane's normal will be returned. The values A, B, C are from the
  // plane equation Ax + By + Cz + D = 0. This vector does not have to
  // have unit length (but it must have a non-zero length!). If a -1 is
  // returned, then an error occurred while adding the plane.
  int  AddPlane( float A, float B, float C );
  int  AddPlane( float plane[3] );

  // Description:
  // Set the normal values for plane i. This is a plane that was already
  // added to the current set of planes with AddPlane, and is now being
  // modified. The values A, B, C are from the plane equation 
  // Ax + By + Cz + D = 0. This vector does not have to have unit length.
  void SetPlane( int i, float A, float B, float C );
  void SetPlane( int i, float plane[3] );

  // Description:
  // Get the number of planes in the current set of planes.
  vtkGetMacro( NumberOfPlanes, int );
  
  // Description:
  // Add the 8 planes that represent the vertices of a cube - the combination
  // of the three face planes connecting to a vertex - (1,1,1), (1,1,-1),
  // (1,-1,1), (1,-1,1), (-1,1,1), (-1,1,-1), (-1,-1,1), (-1,-1-1).
  void AddCubeVertexPlanes();

  // Description:
  // Add the 12 planes that represent the edges of a cube - halfway between
  // the two connecting face planes - (1,1,0), (-1,-1,0), (-1,1,0), (1,-1,0),
  // (0,1,1), (0,-1,-1), (0,1,-1), (0,-1,1), (1,0,1), (-1,0,-1),
  // (1,0,-1), (-1,0,1)
  void AddCubeEdgePlanes();

  // Description:
  // Add the six planes that make up the faces of a cube - (1,0,0),
  // (-1, 0, 0), (0,1,0), (0,-1,0), (0,0,1), (0,0,-1)
  void AddCubeFacePlanes();

  // Description:
  // Add the planes that represent the normals of the vertices of a polygonal
  // sphere formed by recursively subdividing the triangles in an octahedron.
  // Each triangle is subdivided by connecting the midpoints of the edges thus
  // forming 4 smaller triangles. The level indicates how many subdivisions to do
  // with a level of 0 used to add the 6 planes from the original octahedron, level
  // 1 will add 18 planes, and so on.
  void AddRecursiveSpherePlanes( int level );

protected:
  vtkHull();
  ~vtkHull();
  vtkHull(const vtkHull&) {};
  void operator=(const vtkHull&) {};

  // The planes - 4 floats per plane for A, B, C, D
  float     *Planes;

  // This indicates the current size (in planes - 4*sizeof(float)) of 
  // the this->Planes array. Planes are allocated in chunks so that the
  // array does not need to be reallocated every time a new plane is added
  int       PlanesStorageSize;

  // The number of planes that have been added
  int       NumberOfPlanes;

  // Internal method used to find the position of each plane
  void      ComputePlaneDistances();

  // Internal method used to create the actual polygons from the set 
  // of planes
  void      ClipPolygonsFromPlanes( vtkPoints *points, vtkCellArray *polys );

  // Internal method used to create the initial "big" polygon from the
  // plane equation. This polygon is clipped by all other planes to form
  // the final polygon (or it may be clipped entirely)
  void      CreateInitialPolygon( float *, int );

  // The method that does it all...
  void      Execute();
};

#endif




