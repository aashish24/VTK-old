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
#include "vtkDelaunay2D.h"
#include "vtkMath.h"
#include "vtkTriangle.h"

// Construct object with Alpha = 0.0; Tolerance = 0.001; Offset = 1.25;
// BoundingTriangulation turned off.
vtkDelaunay2D::vtkDelaunay2D()
{
  this->Alpha = 0.0;
  this->Tolerance = 0.001;
  this->BoundingTriangulation = 0;
  this->Offset = 1.0;

  this->Output = vtkPolyData::New();
  this->Output->SetSource(this);
}

// Determine whether point x is inside of circumcircle of triangle
// defined by points (x1, x2, x3). Returns non-zero if inside circle.
// (Note that z-component is ignored.)
static int InCircle (float x[3], float x1[3], float x2[3], float x3[3])
{
  float radius2, center[2], dist2;

  radius2 = vtkTriangle::Circumcircle(x1,x2,x3,center);

  // check if inside/outside circumcircle
  dist2 = (x[0]-center[0]) * (x[0]-center[0]) + 
          (x[1]-center[1]) * (x[1]-center[1]);

  if ( dist2 < (0.99999*radius2) ) return 1;
  else return 0;
}

#define VTK_DEL2D_TOLERANCE 1.0e-06

static int NumberOfDuplicatePoints;
static int NumberOfDegeneracies;

// Recursive method to locate triangle containing point. Starts with arbitrary
// triangle (tri) and "walks" towards it. Influenced by some of Guibas and 
// Stolfi's work. Returns id of enclosing triangle, or -1 if no triangle
// found. Also, the array nei[3] is used to communicate info about points
// that loe on triangle edges: nei[0] is neighboring triangle id, and nei[1]
// and nei[2] are the vertices defining the edge.
static int FindTriangle(float x[3], int ptIds[3], int tri, vtkPolyData *Mesh, 
                        vtkPoints *points, float tol, int nei[3])
{
  int i, j, npts, *pts, inside, i2, i3, newNei;
  vtkIdList *neighbors;
  float p[3][3], n[2], vp[2], vx[2], dp, minProj;
  
  // get local triangle info
  Mesh->GetCellPoints(tri,npts,pts);
  for (i=0; i<3; i++) 
    {
    ptIds[i] = pts[i];
    points->GetPoint(ptIds[i],p[i]);
    }

  // evaluate in/out of each edge
  for (inside=1, minProj=0.0, i=0; i<3; i++)
    {
    i2 = (i+1) % 3;
    i3 = (i+2) % 3;

    // create a 2D edge normal to define a "half-space"; evaluate points (i.e.,
    // candiate point and other triangle vertex not on this edge).
    n[0] = -(p[i2][1] - p[i][1]);
    n[1] = p[i2][0] - p[i][0];
    vtkMath::Normalize2D(n);

    // compute local vectors
    for (j=0; j<2; j++)
      {
      vp[j] = p[i3][j] - p[i][j];
      vx[j] = x[j] - p[i][j];
      }

    //check for duplicate point
    vtkMath::Normalize2D(vp);
    if ( vtkMath::Normalize2D(vx) <= tol ) 
      {
      NumberOfDuplicatePoints++;
      return -1;
      }

    // see if two points are in opposite half spaces
    dp = vtkMath::Dot2D(n,vp) * vtkMath::Dot2D(n,vx);
    if ( dp < VTK_DEL2D_TOLERANCE )
      {
      if ( dp < minProj ) //track edge most orthogonal to point direction
        {
        inside = 0;
        nei[1] = ptIds[i];
        nei[2] = ptIds[i2];
        minProj = dp;
        }
      }//outside this edge
    }//for each edge

  neighbors = vtkIdList::New(); neighbors->Allocate(2);
  if ( inside ) // all edges have tested positive
    {
    nei[0] = (-1);
    neighbors->Delete();
    return tri;
    }

  else if ( !inside && (fabs(minProj) < VTK_DEL2D_TOLERANCE) ) // on edge
    {
    Mesh->GetCellEdgeNeighbors(tri,nei[1],nei[2],neighbors);
    nei[0] = neighbors->GetId(0);
    neighbors->Delete();
    return tri;
    }

  else //walk towards point
    {
    Mesh->GetCellEdgeNeighbors(tri,nei[1],nei[2],neighbors);
    if ( (newNei=neighbors->GetId(0)) == nei[0] )
      {
      NumberOfDegeneracies++;
      neighbors->Delete();
      return -1;
      }
    else
      {
      nei[0] = tri;
      neighbors->Delete();
      return FindTriangle(x,ptIds,newNei,Mesh,points,tol,nei);
      }
    }
}

#undef VTK_DEL2D_TOLERANCE

// Recursive method checks whether edge is Delaunay, and if not, swaps edge.
// Continues until all edges are Delaunay. Points p1 and p2 form the edge in
// question; x is the coordinates of the inserted point; tri is the current
// triangle id; Mesh is a pointer to cell structure.
static void CheckEdge(int ptId, float x[3], int p1, int p2, int tri, 
              vtkPolyData *Mesh, vtkPoints *points)
{
  int i, numNei, nei, npts, *pts, p3;
  float x1[3], x2[3], x3[3];
  vtkIdList *neighbors;
  int swapTri[3];

  points->GetPoint(p1,x1);
  points->GetPoint(p2,x2);

  neighbors = vtkIdList::New();
  neighbors->Allocate(2);

  Mesh->GetCellEdgeNeighbors(tri,p1,p2,neighbors);
  numNei = neighbors->GetNumberOfIds();

  if ( numNei > 0 ) //i.e., not a boundary edge
    {
    // get neighbor info including opposite point
    nei = neighbors->GetId(0);
    Mesh->GetCellPoints(nei, npts, pts);
    for (i=0; i<2; i++)
      if ( pts[i] != p1 && pts[i] != p2 )
        break;

    p3 = pts[i];
    points->GetPoint(p3,x3);

    // see whether point is in circumcircle
    if ( InCircle (x3, x, x1, x2) )
      {// swap diagonal
      Mesh->RemoveReferenceToCell(p1,tri);
      Mesh->RemoveReferenceToCell(p2,nei);
      Mesh->ResizeCellList(ptId,1);
      Mesh->AddReferenceToCell(ptId,nei);
      Mesh->ResizeCellList(p3,1);
      Mesh->AddReferenceToCell(p3,tri);

      swapTri[0] = ptId; swapTri[1] = p3; swapTri[2] = p2;
      Mesh->ReplaceCell(tri,3,swapTri);

      swapTri[0] = ptId; swapTri[1] = p1; swapTri[2] = p3;
      Mesh->ReplaceCell(nei,3,swapTri);

      // two new edges become suspect
      CheckEdge(ptId, x, p2, p3, tri, Mesh, points);
      CheckEdge(ptId, x, p3, p1, nei, Mesh, points);

      }//in circle
    }//interior edge

  neighbors->Delete();
}

// 2D Delaunay triangulation. Steps are as follows:
//   1. For each point
//   2. Find triangle point is in
//   3. Create 3 triangles from each edge of triangle that point is in
//   4. Recursively evaluate Delaunay criterion for each edge neighbor
//   5. If criterion not satisfied; swap diagonal
// 
void vtkDelaunay2D::Execute()
{
  int numPoints, numTriangles, i;
  int ptId, tri[4], nei[3], p1, p2;
  vtkPoints *inPoints;
  vtkPoints *points;
  vtkCellArray *triangles;
  vtkPolyData *Mesh=vtkPolyData::New();
  vtkPointSet *input=(vtkPointSet *)this->Input;
  vtkPolyData *output=(vtkPolyData *)this->Output;
  float x[3];
  int nodes[4][3], pts[3], npts, *triPts, numNeiPts, *neiPts;
  vtkIdList *neighbors, *cells;
  float center[3], radius, tol;
  char *triUse = NULL;

  vtkDebugMacro(<<"Generating 2D Delaunay triangulation");
  //
  // Initialize; check input
  //
  if ( (inPoints=input->GetPoints()) == NULL )
    {
    vtkErrorMacro("<<Cannot triangulate; no input points");
    return;
    }

  if ( (numPoints=inPoints->GetNumberOfPoints()) <= 2 )
    {
    vtkErrorMacro("<<Cannot triangulate; need at least 3 input points");
    return;
    }
  
  neighbors = vtkIdList::New(); neighbors->Allocate(2);
  cells = vtkIdList::New(); cells->Allocate(64);
  
  NumberOfDuplicatePoints = 0;
  NumberOfDegeneracies = 0;
  //
  // Create initial bounding triangulation. Have to create bounding points.
  // Initialize mesh structure.
  //
  points = vtkPoints::New(); points->SetNumberOfPoints(numPoints+8);
  for (ptId=0; ptId < numPoints; ptId++)
    {
    points->SetPoint(ptId,inPoints->GetPoint(ptId));
    }

  input->GetCenter(center);
  tol = input->GetLength();
  radius = this->Offset * tol;
  tol *= this->Tolerance;

  for (ptId=0; ptId<8; ptId++)
    {
    x[0] = center[0] + radius*cos((double)(45.0*ptId)*vtkMath::DegreesToRadians());
    x[1] = center[1] + radius*sin((double)(45.0*ptId)*vtkMath::DegreesToRadians());
    x[2] = center[2];
    points->SetPoint(numPoints+ptId,x);
    }

  triangles = vtkCellArray::New();
  triangles->Allocate(triangles->EstimateSize(2*numPoints,3));

  //create bounding triangles (there are six)
  pts[0] = numPoints; pts[1] = numPoints + 1; pts[2] = numPoints + 2;
  triangles->InsertNextCell(3,pts);
  pts[0] = numPoints + 2; pts[1] = numPoints + 3; pts[2] = numPoints + 4;
  triangles->InsertNextCell(3,pts);
  pts[0] = numPoints + 4; pts[1] = numPoints + 5; pts[2] = numPoints + 6;
  triangles->InsertNextCell(3,pts);
  pts[0] = numPoints + 6; pts[1] = numPoints + 7; pts[2] = numPoints + 0;
  triangles->InsertNextCell(3,pts);
  pts[0] = numPoints + 0; pts[1] = numPoints + 2; pts[2] = numPoints + 6;
  triangles->InsertNextCell(3,pts);
  pts[0] = numPoints + 2; pts[1] = numPoints + 4; pts[2] = numPoints + 6;
  triangles->InsertNextCell(3,pts);
  tri[0] = 0; //initialize value for FindTriangle

  Mesh->SetPoints(points);
  Mesh->SetPolys(triangles);
  Mesh->BuildLinks(); //build cell structure
  //
  // For each point; find triangle containing point. Then evaluate three 
  // neighboring triangles for Delaunay criterion. Triangles that do not 
  // satisfy criterion have their edges swapped. This continues recursively 
  // until all triangles have been shown to be Delaunay.
  //
  for (ptId=0; ptId < numPoints; ptId++)
    {
    points->GetPoint(ptId,x); 
    nei[0] = (-1); //where we are coming from...nowhere initially

    if ( (tri[0] = FindTriangle(x,pts,tri[0],Mesh,points,tol,nei)) >= 0 )
      {
      if ( nei[0] < 0 ) //in triangle
        {
        //delete this triangle; create three new triangles
        //first triangle is replaced with one of the new ones
        nodes[0][0] = ptId; nodes[0][1] = pts[0]; nodes[0][2] = pts[1];
        Mesh->RemoveReferenceToCell(pts[2], tri[0]);
        Mesh->ReplaceCell(tri[0], 3, nodes[0]);
        Mesh->ResizeCellList(ptId,1);
        Mesh->AddReferenceToCell(ptId,tri[0]);

        //create two new triangles
        nodes[1][0] = ptId; nodes[1][1] = pts[1]; nodes[1][2] = pts[2];
        tri[1] = Mesh->InsertNextLinkedCell(VTK_TRIANGLE, 3, nodes[1]);

        nodes[2][0] = ptId; nodes[2][1] = pts[2]; nodes[2][2] = pts[0];
        tri[2] = Mesh->InsertNextLinkedCell(VTK_TRIANGLE, 3, nodes[2]);

        // Check edge neighbors for Delaunay criterion. If not satisfied, flip
        // edge diagonal. (This is done recursively.)
        CheckEdge(ptId, x, pts[0], pts[1], tri[0], Mesh, points);
        CheckEdge(ptId, x, pts[1], pts[2], tri[1], Mesh, points);
        CheckEdge(ptId, x, pts[2], pts[0], tri[2], Mesh, points);
        }

      else // on triangle edge
        {
        //update cell list
        Mesh->GetCellPoints(nei[0],numNeiPts,neiPts);
        for (i=0; i<3; i++)
          {
          if ( neiPts[i] != nei[1] && neiPts[i] != nei[2] ) p1 = neiPts[i];
          if ( pts[i] != nei[1] && pts[i] != nei[2] ) p2 = pts[i];
          }
        Mesh->ResizeCellList(p1,1);
        Mesh->ResizeCellList(p2,1);

        //replace two triangles
        Mesh->RemoveReferenceToCell(nei[2],tri[0]);
        Mesh->RemoveReferenceToCell(nei[2],nei[0]);
        nodes[0][0] = ptId; nodes[0][1] = p1; nodes[0][2] = nei[1];
        Mesh->ReplaceCell(tri[0], 3, nodes[0]);
        nodes[1][0] = ptId; nodes[1][1] = p2; nodes[1][2] = nei[1];
        Mesh->ReplaceCell(nei[0], 3, nodes[1]);
	Mesh->ResizeCellList(ptId, 2);
        Mesh->AddReferenceToCell(ptId,tri[0]);
        Mesh->AddReferenceToCell(ptId,nei[0]);

        //create two new triangles
        nodes[2][0] = ptId; nodes[2][1] = p2; nodes[2][2] = nei[2];
        tri[2] = Mesh->InsertNextLinkedCell(VTK_TRIANGLE, 3, nodes[2]);

        nodes[3][0] = ptId; nodes[3][1] = p1; nodes[3][2] = nei[2];
        tri[3] = Mesh->InsertNextLinkedCell(VTK_TRIANGLE, 3, nodes[3]);

        // Check edge neighbors for Delaunay criterion.
        for ( i=0; i<4; i++ )
          {
          CheckEdge (ptId, x, nodes[i][1], nodes[i][2], tri[i], Mesh, points);
          }
        }
      }//if triangle found

    else
      {
      tri[0] = 0; //no triangle found
      }

    if ( ! (ptId % 1000) ) vtkDebugMacro(<<"point #" << ptId);
    }//for all points

  vtkDebugMacro(<<"Triangulated " << numPoints <<" points, " 
                << NumberOfDuplicatePoints << " of which were duplicates");

  if ( NumberOfDegeneracies > 0 )
    {
    vtkWarningMacro(<< NumberOfDegeneracies 
                    << " degenerate triangles encountered, mesh quality suspect");
    }
//
// Finish up by deleting all triangles connected to initial triangulation
//
  numTriangles = Mesh->GetNumberOfCells();
  if ( !this->BoundingTriangulation || this->Alpha > 0.0 )
    {
    triUse = new char[numTriangles];
    for (i=0; i<numTriangles; i++) triUse[i] = 1;
    }

  if ( ! this->BoundingTriangulation )
    {
    for (ptId=numPoints; ptId < (numPoints+8); ptId++)
      {
      Mesh->GetPointCells(ptId, cells);
      for (i=0; i < cells->GetNumberOfIds(); i++)
        {
        triUse[cells->GetId(i)] = 0; //mark as deleted
        }
      }
    }
//
// If non-zero alpha value, then figure out which parts of mesh are
// contained within alpha radius.
//
  if ( this->Alpha > 0.0 )
    {
    float alpha2 = this->Alpha * this->Alpha;
    float x1[3], x2[3], x3[3];
    int j, cellId, numNei, p1, p2, nei;

    vtkCellArray *alphaVerts = vtkCellArray::New();
    alphaVerts->Allocate(numPoints);
    vtkCellArray *alphaLines = vtkCellArray::New();
    alphaLines->Allocate(numPoints);

    char *pointUse = new char[numPoints+8];
    for (ptId=0; ptId < (numPoints+8); ptId++) pointUse[ptId] = 0;

    //traverse all triangles; evaluating Delaunay criterion
    for (i=0; i < numTriangles; i++)
      {
      if ( triUse[i] == 1 )
        {
        Mesh->GetCellPoints(i, npts, triPts);
        points->GetPoint(triPts[0],x1);
        points->GetPoint(triPts[1],x2);
        points->GetPoint(triPts[2],x3);
        if ( vtkTriangle::Circumcircle(x1,x2,x3,center) > alpha2 )
          {
          triUse[i] = 0;
          }
        else
          {
          for (j=0; j<3; j++) pointUse[triPts[j]] = 1; 
          }
        }//if non-deleted triangle
      }//for all triangles

    //traverse all edges see whether we need to create some
    for (cellId=0, triangles->InitTraversal(); 
    triangles->GetNextCell(npts,triPts); cellId++)
      {
      if ( ! triUse[cellId] )
        {
        for (i=0; i < npts; i++) 
          {
          p1 = triPts[i];
          p2 = triPts[(i+1)%npts];

          if (this->BoundingTriangulation || (p1<numPoints && p2<numPoints))
            {
            Mesh->GetCellEdgeNeighbors(cellId,p1,p2,neighbors);
            numNei = neighbors->GetNumberOfIds();

            if ( numNei < 1 || ((nei=neighbors->GetId(0)) > cellId 
				&& !triUse[nei]) )
              {//see whether edge is shorter than Alpha
              points->GetPoint(p1,x1);
              points->GetPoint(p2,x2);
              if ( (vtkMath::Distance2BetweenPoints(x1,x2)*0.25) <= alpha2 )
                {
                pointUse[p1] = 1; pointUse[p2] = 1;
                pts[0] = p1;
                pts[1] = p2;
                alphaLines->InsertNextCell(2,pts);
                }//if passed test
              }//test edge
            }//if valid edge
          }//for all edges of this triangle
        }//if triangle not output
      }//for all triangles

    //traverse all points, create vertices if none used
    for (ptId=0; ptId<(numPoints+8); ptId++)
      {
      if ( !pointUse[ptId] &&  (ptId < numPoints || 
				this->BoundingTriangulation) )
        {
        pts[0] = ptId;
        alphaVerts->InsertNextCell(1,pts);
        }
      }

    // update output
    delete [] pointUse;
    output->SetVerts(alphaVerts);
    alphaVerts->Delete();
    output->SetLines(alphaLines);
    alphaLines->Delete();
    }
  //
  // Update output; free up supporting data structures.
  //
  if ( this->BoundingTriangulation )
    {
    output->SetPoints(points);
    }  
  else
    {
    output->SetPoints(inPoints);
    output->GetPointData()->PassData(input->GetPointData());
    }

  if ( this->Alpha <= 0.0 && this->BoundingTriangulation )
    {
    output->SetPolys(triangles);
    }
  else
    {
    vtkCellArray *alphaTriangles = vtkCellArray::New();
    alphaTriangles->Allocate(numTriangles);
    int *triPts;

    for (i=0; i<numTriangles; i++)
      {
      if ( triUse[i] )
        {
        Mesh->GetCellPoints(i,npts,triPts);
        alphaTriangles->InsertNextCell(3,triPts);
        }
      }
    output->SetPolys(alphaTriangles);
    alphaTriangles->Delete();
    delete [] triUse;
    }

  points->Delete();
  triangles->Delete();
  Mesh->Delete();
  neighbors->Delete();
  cells->Delete();

  output->Squeeze();
}

void vtkDelaunay2D::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPointSetFilter::PrintSelf(os,indent);

  os << indent << "Alpha: " << this->Alpha << "\n";
  os << indent << "Tolerance: " << this->Tolerance << "\n";
  os << indent << "Offset: " << this->Offset << "\n";
  os << indent << "Bounding Triangulation: " << (this->BoundingTriangulation ? "On\n" : "Off\n");
}
