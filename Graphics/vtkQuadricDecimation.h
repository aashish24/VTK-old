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
// .NAME vtkQuadricDecimation - reduce the number of triangles in a mesh
// .SECTION Description
// vtkQuadricDecimation is a filter to reduce the number of triangles in
// a triangle mesh, forming a good approximation to the original geometry. 
// The input to vtkQuadricDecimation is a vtkPolyData object, and only
// triangles are treated. If you desire to decimate polygonal meshes, first
// triangulate the polygons with vtkTriangleFilter object.
//
// The quadric error metric used is the one outlined in Hughues Hoppe's
// Vis '99 paper, "New Quadric Metric for Simplifying Meshes with Appearance
// Attributes."


#ifndef __vtkQuadricDecimation_h
#define __vtkQuadricDecimation_h

#include "vtkPolyDataToPolyDataFilter.h"
#include "vtkEdgeTable.h"
#include "vtkPriorityQueue.h"
#include "vtkIdList.h"

typedef struct {
  float GeometricQuadric[10];
  float *AttributeQuadric;
} VTK_ERROR_QUADRIC;

class VTK_EXPORT vtkQuadricDecimation : public vtkPolyDataToPolyDataFilter
{
public:
  vtkTypeMacro(vtkQuadricDecimation, vtkPolyDataToPolyDataFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkQuadricDecimation *New();

  // Description:
  // Set/Get the maximum allowable cost of collapsing an edge.
  vtkSetMacro(MaximumCost, float);
  vtkGetMacro(MaximumCost, float);
  
protected:
  vtkQuadricDecimation();
  ~vtkQuadricDecimation();
  vtkQuadricDecimation(const vtkQuadricDecimation&) {};
  void operator=(const vtkQuadricDecimation&) {};

  void Execute();

  // Description:
  // Compute quadric for this vertex.
  void ComputeQuadric(int pointId);

  // Description:
  // Add the quadrics for these 2 points since the edge between them has
  // been collapsed.
  void AddQuadric(int oldPtId, int newPtId);
  
  // Description:
  // Compute cost for contracting this edge and the point that gives us this
  // cost.
  float ComputeCost(int edgeId, float x[3]);

  // Description:
  // Find all edges that will have an endpoint change ids because of an edge
  // collapse.  p1Id and p2Id are the endpoints of the edge.  p2Id is the
  // pointId being removed.
  void FindAffectedEdges(int p1Id, int p2Id, vtkIdList *edges,
                         vtkPolyData *mesh);
  
  // Description:
  // Find a cell that uses this edge.
  int GetEdgeCellId(int p1Id, int p2Id, vtkPolyData *mesh);
  
  float MaximumCost;
  vtkEdgeTable *Edges;
  vtkIdList *EndPoint1List;
  vtkIdList *EndPoint2List;
  vtkPriorityQueue *EdgeCosts;
  VTK_ERROR_QUADRIC *ErrorQuadrics;
};

#endif
