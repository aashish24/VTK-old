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
// .NAME vtkQuadricClustering - reduce the number of triangles in a mesh
// .SECTION Description

// vtkQuadricClustering is a filter to reduce the number of triangles in a
// triangle mesh, forming a good approximation to the original geometry.  The
// input to vtkQuadricClustering is a vtkPolyData object, and only triangles
// are treated. If you desire to decimate polygonal meshes, first triangulate
// the polygons with vtkTriangleFilter object. 
//
// The algorithm used is the one described by Peter Lindstrom in his Siggraph
// 2000 paper, "Out-of-Core Simplification of Large Polygonal Models."  The
// general approach of the algorithm is to cluster vertices in a uniform
// binning of space, accumulating the quadric of each triangle (pushed out to
// the triangles vertices) within each bin, and then determining an optimal
// position for a single vertex in a bin by using the accumulated quadric. In
// more detail, the algorithm first gets the bounds of the input poly data.
// It then breaks this bounding volume into a user-specified number of
// spatial bins.  It then reads each triangle from the input and hashes its
// vertices into these bins.  (If this is the first time a bin has been
// visited, initialize its quadric to the 0 matrix.)  If 2 or more vertices
// of the triangle fall in the same bin, the triangle is dicarded.  If the
// triangle is not discarded, the algorithm then computes the error quadric
// for this triangle and adds it to the existing quadric for each bin
// corresponding to a vertex of this triangle.  It adds the triangle to the
// list of output triangles as a list of vertex identifiers.  (There is one
// vertex id per bin.)  After all the triangles have been read, the
// representative vertex for each bin is computed using the quadric for that
// bin.  This determines the spatial location of the vertices of each of the
// triangles in the output.
//
// To use this filter, specify the divisions defining the spatial subdivision
// in the x, y, and z directions. You must also specify an input vtkPolyData.
//
// This filter can take multiple inputs.  The user can either specifiy them
// in the same way as for other filters that take multiple inputs (i.e., calls
// to SetInput and/or AddInput), or the user can explicity call StartAppend,
// Append (once for each input), and EndAppend.  StartAppend sets up the data
// structure to hold the quadric matrices.  Append processes each triangle in
// the input poly data it was called on, hashes its vertices to the appropriate
// bins, determines whether to keep this triangle, and updates the appropriate
// quadric matrices.  EndAppend determines the spatial location of each of the
// representative vertices for the visited bins.  The reason a user might want
// to explicitly call StartAppend, Append, and EndAppend is to avoid having
// all the input data sets in memory at once.

// .SECTION Caveats
// This filter can drastically affect topology, i.e., topology is not 
// preserved.

// .SECTION See Also
// vtkDecimatePro vtkDecimate

#ifndef __vtkQuadricClustering_h
#define __vtkQuadricClustering_h

#include "vtkPolyDataToPolyDataFilter.h"
#include "vtkTimerLog.h"
#include "vtkDataSetCollection.h"

typedef struct {
  int VertexId;
  float Quadric[9];
} VTK_POINT_QUADRIC;

class VTK_EXPORT vtkQuadricClustering : public vtkPolyDataToPolyDataFilter
{
public:
  vtkTypeMacro(vtkQuadricClustering, vtkPolyDataToPolyDataFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkQuadricClustering *New();

  // Description:
  // Set/Get the number of divisions along each axis for the spatial bins.
  // The number of spatial bins is NumberOfXDivisions*NumberOfYDivisions*
  // NumberOfZDivisions.
  vtkSetClampMacro(NumberOfXDivisions, int, 2, VTK_LARGE_INTEGER);
  vtkSetClampMacro(NumberOfYDivisions, int, 2, VTK_LARGE_INTEGER);
  vtkSetClampMacro(NumberOfZDivisions, int, 2, VTK_LARGE_INTEGER);
  vtkGetMacro(NumberOfXDivisions, int);
  vtkGetMacro(NumberOfYDivisions, int);
  vtkGetMacro(NumberOfZDivisions, int);
  void SetNumberOfDivisions(int div[3]);
  int *GetNumberOfDivisions();
  void GetNumberOfDivisions(int div[3]);

  // Description:
  // Set the bounds for the input poly data.  This method is public to allow
  // the user to specify bounds larger than those of the 1st input specified.
  vtkSetVector6Macro(Bounds, float);
  
  // Description:
  // Add another input to the list of those to be decimated.
  void AddInput(vtkPolyData *input);

  // Description:
  // Get any input of this filter.
  vtkPolyData *GetInput(int idx);
  vtkPolyData *GetInput() { return (vtkPolyData*)this->GetInput( 0 ); };
  
  // Description:
  // Remove a dataset from the list of data to append.
  void RemoveInput(vtkPolyData *in);

  // Description:
  // Returns a copy of the input array.  Modifications to this list
  // will not be reflected in the actual inputs.
  vtkDataSetCollection *GetInputList();
  
  // Description:
  // Keep superclass's UpdateData from being called if Input is NULL.
  virtual void UpdateData(vtkDataObject *output);
  
  // Description:
  // This method must be called before Append.  It initializes the data
  // structure that holds the quadic for each bin.
  void StartAppend();
  
  // Description:
  // Process the triangles from this input poly data.
  void Append(vtkPolyData *pd);
  
  // Description:
  // Determine the representative point for each bin that has been visited.
  // This method must be called after the last call to Append.
  void EndAppend();
  
  // Description:
  // Set/Get the number of inputs.  (This is used for updating progress.)
  vtkSetMacro(NumberOfExpectedInputs, int);
  vtkGetMacro(NumberOfExpectedInputs, int);
  
protected:
  vtkQuadricClustering();
  ~vtkQuadricClustering();
  vtkQuadricClustering(const vtkQuadricClustering&) {};
  void operator=(const vtkQuadricClustering&) {};

  void Execute();

  // Description:
  // Given a point, determine what bin it falls into.
  int HashPoint(float point[3]);
  
  // Description:
  // Determine the representative point for this bin.
  void ComputeRepresentativePoint(float quadric[9], int binId,
				  float point[3]);

  // Description:
  // Set the bin sizes for each of the 3 dimensions.
  vtkSetMacro(XBinSize, float);
  vtkSetMacro(YBinSize, float);
  vtkSetMacro(ZBinSize, float);

  // Description:
  // Initialize the quadric matrix to 0's.
  void InitializeQuadric(float quadric[9]);
  
  // Description:
  // Add this quadric to the quadric already associated with this bin.
  void AddQuadric(int binId, float quadric[9]);

  int NumberOfXDivisions;
  int NumberOfYDivisions;
  int NumberOfZDivisions;
 
  float Bounds[6];
  float XBinSize;
  float YBinSize;
  float ZBinSize;
  VTK_POINT_QUADRIC* QuadricArray;
  int NumberOfBinsUsed;
  int BinSizeSet;
  vtkCellArray *OutputTriangles;
  vtkDataSetCollection *InputList;
  vtkTimerLog* Log;
  int NumberOfExpectedInputs;
  int AbortExecute; // We need this ivar because if we abort execute during
                    // Append, we don't want EndAppend to execute either.
};

#endif
