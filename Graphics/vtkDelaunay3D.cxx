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
#include "vtkDelaunay3D.hh"
#include "vtkMath.hh"
#include "vtkTetra.hh"
#include "vtkTriangle.hh"

// Description:
// Construct object with Alpha = 0.0; Tolerance = 0.001; Offset = 1.25;
// BoundingTriangulation turned off.
vtkDelaunay3D::vtkDelaunay3D()
{
  this->Alpha = 0.0;
  this->Tolerance = 0.001;
  this->BoundingTriangulation = 0;
  this->Offset = 1.0;

  this->Output = new vtkUnstructuredGrid;
  this->Output->SetSource(this);
}

// Determine whether point x is inside of circumsphere of tetrahedron
// defined by points (x1, x2, x3, x4). Returns non-zero if inside sphere.
static int InSphere (float x[3], float x1[3], float x2[3], float x3[3],
                     float x4[3])
{
  static vtkMath math;
  static vtkTetra tetra;
  float radius2, center[3], dist2;

  radius2 = tetra.Circumsphere(x1,x2,x3,x4,center);

  // check if inside/outside circumcircle
  dist2 = (x[0]-center[0]) * (x[0]-center[0]) + 
          (x[1]-center[1]) * (x[1]-center[1]) +
          (x[2]-center[2]) * (x[2]-center[2]);

  if ( dist2 < (0.9999*radius2) ) return 1;
  else return 0;
}

static int NumberOfDuplicatePoints;

// Recursive method to locate tetrahedron containing point. Starts with
// arbitrary tetrahedron (tetra) and "walks" towards it. Influenced by 
// some of Guibas and Stolfi's work. Returns id of enclosing tetra, or -1 
// if no tetrahedron found.
static int FindTetra(float x[3], int ptIds[4], int tetra, 
                     vtkUnstructuredGrid *Mesh, 
                     vtkFloatPoints *points, float tol)
{
  int i, j, npts, inside, i2, i3, i4;
  vtkIdList pts(4), facePts(3);
  static vtkMath math;
  vtkIdList neighbors(2);
  float p[4][3], v12[3], vp[3], vx[3], v32[3], n[3], valx, valp;
  
  // get local tetrahedron info
  Mesh->GetCellPoints(tetra,pts);
  for (i=0; i<4; i++) 
    {
    ptIds[i] = pts.GetId(i);
    points->GetPoint(ptIds[i],p[i]);
    }

  // evaluate in/out of each face
  for (inside=1, i=0; i<4; i++)
    {
    i2 = (i+1) % 4;
    i3 = (i+2) % 4;
    i4 = (i+3) % 4;

    // compute normal and local vectors
    for (j=0; j<3; j++)
      {
      v32[j] = p[i3][j] - p[i2][j];
      v12[j] = p[i][j] - p[i2][j];
      vp[j] = p[i4][j] - p[i2][j];
      vx[j] = x[j] - p[i2][j];
      }

    if ( math.Normalize(vx) <= tol ) //check for duplicate point
      {
      NumberOfDuplicatePoints++;
      return -1;
      }

    if ( math.Normalize(vp) <= 1.0e-04 ) continue; //maybe on face
    math.Cross(v32,v12,n); math.Normalize(n);//face normal

    //see whether point and triangle vertex are on same side of tetra face
    valp = math.Dot(n,vp);
    if ( (valx = math.Dot(n,vx)) <= 1.0e-04 ) continue; //maybe on face

    if ( (valx < 0.0 && valp > 0.0) || (valx > 0.0 && valp < 0.0)  )
      {
      inside = 0;
      facePts.SetId(i,ptIds[i]);
      facePts.SetId(i2,ptIds[i2]);
      facePts.SetId(i3,ptIds[i3]);
      Mesh->GetCellNeighbors(tetra, facePts, neighbors);
      if ( neighbors.GetNumberOfIds() > 0 ) //not boundary
        {
        return FindTetra(x,ptIds,neighbors.GetId(0),Mesh,points,tol);
        }
      }//outside this edge

    }//for each edge

  //must be in this tetraangle if all edges test inside
  if ( !inside ) return -1;
  else return tetra;
}

// Recursive method checks whether face is Delaunay, and if not, swaps face.
// Continues until all faces are Delaunay. Points p1,p2,p3 form the face in
// question; x is the coordinates of the inserted point; tetra is the current
// triangle id; Mesh is a pointer to cell structure.
static void CheckFace(int ptId, float x[3], int p1, int p2, int p3, int tri, 
                      vtkUnstructuredGrid *Mesh, vtkFloatPoints *points)
{
  int i, numNei, nei, npts, *pts, p4;
  float x1[3], x2[3], x3[3];
  vtkIdList neighbors(2), facePts(3);
  int swapTri[3];

  points->GetPoint(p1,x1);
  points->GetPoint(p2,x2);
  points->GetPoint(p3,x3);

  facePts.SetId(0,p1);
  facePts.SetId(1,p2);
  facePts.SetId(2,p3);

  Mesh->GetCellNeighbors(tri,facePts,neighbors);
  numNei = neighbors.GetNumberOfIds();

  if ( numNei > 0 ) //i.e., not a boundary edge
    {
    // get neighbor info including opposite point
    nei = neighbors.GetId(0);
    Mesh->GetCellPoints(nei, npts, pts);
    for (i=0; i<2; i++)
      if ( pts[i] != p1 && pts[i] != p2 )
        break;

    p4 = pts[i];
    points->GetPoint(p4,x3);

    // see whether point is in circumcircle
    if ( InSphere (x3, x, x1, x2, x3) )
      {// swap diagonal
      Mesh->RemoveReferenceToCell(p1,tri);
      Mesh->RemoveReferenceToCell(p2,nei);
      Mesh->ResizeCellList(ptId,1);
      Mesh->AddReferenceToCell(ptId,nei);
      Mesh->ResizeCellList(p4,1);
      Mesh->AddReferenceToCell(p4,tri);

      swapTri[0] = ptId; swapTri[1] = p4; swapTri[2] = p2;
      Mesh->ReplaceCell(tri,3,swapTri);

      swapTri[0] = ptId; swapTri[1] = p1; swapTri[2] = p4;
      Mesh->ReplaceCell(nei,3,swapTri);

      // three new faces become suspect
      CheckFace(ptId, x, p2, p4, p4, tri, Mesh, points);
      CheckFace(ptId, x, p4, p1, p4, nei, Mesh, points);

      }//in circle
    }//interior edge
}

// 3D Delaunay triangulation. Steps are as follows:
//   1. For each point
//   2. Find triangle point is in
//   3. Create 3 triangles from each edge of triangle that point is in
//   4. Recursively evaluate Delaunay criterion for each edge neighbor
//   5. If criterion not satisfied; swap diagonal
// 
void vtkDelaunay3D::Execute()
{
  int numPoints, numTetras, i;
  int ptId, tetra[4];
  vtkPoints *inPoints;
  vtkFloatPoints *points;
  vtkCellArray *alphaTetras;
  vtkUnstructuredGrid *Mesh=new vtkUnstructuredGrid;
  vtkPointSet *input=(vtkPointSet *)this->Input;
  vtkUnstructuredGrid *output=(vtkUnstructuredGrid *)this->Output;
  float x[3];
  int nodes[3][3], pts[3], npts, *triPts;
  vtkIdList neighbors(2), cells(64);
  float center[3], radius, tol;
  vtkMath math;
  char *triUse;

  vtkDebugMacro(<<"Generating 3D Delaunay triangulation");
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
  NumberOfDuplicatePoints = 0;
//
// Create initial bounding triangulation. Have to create bounding points.
// Initialize mesh structure.
//
  points = new vtkFloatPoints(numPoints+8);
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
    x[0] = center[0] + radius*cos((double)(45.0*ptId)*math.DegreesToRadians());
    x[1] = center[1] + radius*sin((double)(45.0*ptId)*math.DegreesToRadians());
    x[2] = 0.0;
    points->SetPoint(numPoints+ptId,x);
    }

  Mesh->Allocate(5*numPoints);

  //create bounding tetras (there are six)
  pts[0] = numPoints; pts[1] = numPoints + 1; pts[2] = numPoints + 2;
  Mesh->InsertNextCell(VTK_TETRA,4,pts);
  pts[0] = numPoints + 2; pts[1] = numPoints + 3; pts[2] = numPoints + 4;
  Mesh->InsertNextCell(VTK_TETRA,4,pts);
  pts[0] = numPoints + 4; pts[1] = numPoints + 5; pts[2] = numPoints + 6;
  Mesh->InsertNextCell(VTK_TETRA,4,pts);
  pts[0] = numPoints + 6; pts[1] = numPoints + 7; pts[2] = numPoints + 0;
  Mesh->InsertNextCell(VTK_TETRA,4,pts);
  pts[0] = numPoints + 0; pts[1] = numPoints + 2; pts[2] = numPoints + 6;
  Mesh->InsertNextCell(VTK_TETRA,4,pts);
  pts[0] = numPoints + 2; pts[1] = numPoints + 4; pts[2] = numPoints + 6;
  Mesh->InsertNextCell(VTK_TETRA,4,pts);
  tetra[0] = 0; //initialize value for FindTetra

  Mesh->SetPoints(points);
  Mesh->BuildLinks();
//
// For each point; find triangle containing point. Then evaluate three 
// neighboring tetras for Delaunay criterion. Tetras that do not 
// satisfy criterion have their edges swapped. This continues recursively 
// until all tetras have been shown to be Delaunay.
//
  for (ptId=0; ptId < numPoints; ptId++)
    {
    points->GetPoint(ptId,x);
    if ( (tetra[0] = FindTetra(x,pts,tetra[0],Mesh,points,tol)) >= 0 )
      {
      //delete this triangle; create three new tetras
      //first triangle is replaced with one of the new ones
      nodes[0][0] = ptId; nodes[0][1] = pts[0]; nodes[0][2] = pts[1];
      Mesh->RemoveReferenceToCell(pts[2], tetra[0]);
      Mesh->ReplaceCell(tetra[0], 3, nodes[0]);
      Mesh->ResizeCellList(ptId,1);
      Mesh->AddReferenceToCell(ptId,tetra[0]);

      //create two new tetras
      nodes[1][0] = ptId; nodes[1][1] = pts[1]; nodes[1][2] = pts[2];
      tetra[1] = Mesh->InsertNextLinkedCell(VTK_TETRA, 3, nodes[1]);

      nodes[2][0] = ptId; nodes[2][1] = pts[2]; nodes[2][2] = pts[0];
      tetra[2] = Mesh->InsertNextLinkedCell(VTK_TETRA, 3, nodes[2]);

      // Check face neighbors for Delaunay criterion. If not satisfied, 
      // "swap" face. (This is done recursively.)
      CheckFace(ptId, x, pts[0], pts[1], pts[2], tetra[0], Mesh, points);
      CheckFace(ptId, x, pts[1], pts[2], pts[3], tetra[1], Mesh, points);
      CheckFace(ptId, x, pts[2], pts[3], pts[0], tetra[2], Mesh, points);
      CheckFace(ptId, x, pts[3], pts[0], pts[1], tetra[3], Mesh, points);

      }//if triangle found
    }//for all points

  vtkDebugMacro(<<"Triangulated " << numPoints <<" points, " 
                << NumberOfDuplicatePoints << " of which were duplicates");
//
// Finish up by deleting all tetras connected to initial triangulation
//
  numTetras = Mesh->GetNumberOfCells();
  if ( !this->BoundingTriangulation || this->Alpha > 0.0 )
    {
    triUse = new char[numTetras];
    for (i=0; i<numTetras; i++) triUse[i] = 1;
    }

  if ( ! this->BoundingTriangulation )
    {
    for (ptId=numPoints; ptId < (numPoints+8); ptId++)
      {
      Mesh->GetPointCells(ptId, cells);
      for (i=0; i < cells.GetNumberOfIds(); i++)
        {
        triUse[cells.GetId(i)] = 0; //mark as deleted
        }
      }
    }
//
// If non-zero alpha value, then figure out which parts of mesh are
// contained within alpha radius.
//
  if ( this->Alpha > 0.0 )
    {
    char *pointUse = new char[numPoints+8];
    for (ptId=0; ptId < (numPoints+8); ptId++) pointUse[ptId] = 0;

    //traverse all points, create vertices if none used
    for (ptId=0; ptId<(numPoints+8); ptId++)
      {
      if ( !pointUse[ptId] &&  (ptId < numPoints || this->BoundingTriangulation) )
        {
        pts[0] = ptId;
        output->InsertNextCell(VTK_VERTEX,1,pts);
        }
      }

    // update output
    delete [] pointUse;
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
//    output->SetPolys(tetras);
    }
  else
    {
    vtkCellArray *alphaTetras = new vtkCellArray(numTetras);
    int *triPts;

    for (i=0; i<numTetras; i++)
      {
      if ( triUse[i] )
        {
        Mesh->GetCellPoints(i,npts,triPts);
        output->InsertNextCell(VTK_TETRA,4,triPts);
        }
      }
    delete [] triUse;
    }

  points->Delete();
  delete Mesh;

  output->Squeeze();
}

void vtkDelaunay3D::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPointSetFilter::PrintSelf(os,indent);

  os << indent << "Alpha: " << this->Alpha << "\n";
  os << indent << "Tolerance: " << this->Tolerance << "\n";
  os << indent << "Offset: " << this->Offset << "\n";
  os << indent << "Bounding Triangulation: " << (this->BoundingTriangulation ? "On\n" : "Off\n");
}
