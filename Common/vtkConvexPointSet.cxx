/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkConvexPointSet.h"

#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkOrderedTriangulator.h"
#include "vtkPointData.h"
#include "vtkPointLocator.h"
#include "vtkPoints.h"
#include "vtkTetra.h"
#include "vtkTriangle.h"

vtkCxxRevisionMacro(vtkConvexPointSet, "$Revision$");
vtkStandardNewMacro(vtkConvexPointSet);

// Construct the hexahedron with eight points.
vtkConvexPointSet::vtkConvexPointSet()
{
  this->Tetra = vtkTetra::New();
  this->TetraIds = vtkIdList::New();
  this->TetraPoints = vtkPoints::New();
  this->TetraScalars = vtkFloatArray::New();
  this->TetraScalars->SetNumberOfTuples(4);
  this->BoundaryTris = vtkCellArray::New();
  this->BoundaryTris->Allocate(100);
  this->Triangle = vtkTriangle::New();
  this->Triangulator = vtkOrderedTriangulator::New();
  this->Triangulator->PreSortedOff();
}

vtkConvexPointSet::~vtkConvexPointSet()
{
  this->Tetra->Delete();
  this->TetraIds->Delete();
  this->TetraPoints->Delete();
  this->TetraScalars->Delete();
  this->BoundaryTris->Delete();
  this->Triangle->Delete();
}

// Should be called by GetCell() prior to any other method invocation
void vtkConvexPointSet::Initialize()
{
  // Initialize
  vtkIdType numPts=this->GetNumberOfPoints();
  if ( numPts < 1 ) return;

  this->Triangulate(0, this->TetraIds,this->TetraPoints);
}

int vtkConvexPointSet::GetNumberOfFaces()
{
  this->BoundaryTris->Reset();
  this->Triangulator->AddTriangles(this->BoundaryTris);
  return this->BoundaryTris->GetNumberOfCells();
}

vtkCell *vtkConvexPointSet::GetFace(int faceId)
{
  int numCells = this->BoundaryTris->GetNumberOfCells();
  if ( faceId < 0 || faceId >=numCells ) {return NULL;}

  vtkIdType *cells = this->BoundaryTris->GetPointer();

  // Each triangle has three points plus number of points
  vtkIdType *cptr = cells + 4*faceId;
  for (int i=0; i<3; i++)
    {
    this->Triangle->PointIds->SetId(i,this->PointIds->GetId(cptr[i+1]));
    this->Triangle->Points->SetPoint(i,this->Points->GetPoint(cptr[i+1]));
    }

  return this->Triangle;
}

int vtkConvexPointSet::Triangulate(int vtkNotUsed(index), vtkIdList *ptIds,
                                   vtkPoints *pts)
{
  int numPts=this->GetNumberOfPoints();
  int i;
  float *xPtr;
  vtkIdType ptId;

  // Initialize
  ptIds->Reset();
  pts->Reset();
  if ( numPts < 1 ) return 0;
  
  // Initialize Delaunay insertion process.
  // No more than numPts points can be inserted.
  this->Triangulator->InitTriangulation(this->GetBounds(), numPts);

  // Inject cell points into triangulation. Recall that the PreSortedOff() 
  // flag was set which means that the triangulator will order the points 
  // according to point id. We insert points with id == the index into the
  // vtkConvexPointSet::PointIds and Points; but sort on the global point
  // id.
  for (i=0; i<numPts; i++)
    {
    ptId = this->PointIds->GetId(i);
    xPtr = this->Points->GetPoint(i);
    this->Triangulator->InsertPoint(i, ptId, xPtr, xPtr, 0);
    }//for all points
  
  // triangulate the points
  this->Triangulator->Triangulate();

  // Add the triangulation to the mesh
  this->Triangulator->AddTetras(0,ptIds,pts);
  
  return 1;
}

  
void vtkConvexPointSet::Contour(float value,
                                vtkDataArray *cellScalars,
                                vtkPointLocator *locator,
                                vtkCellArray *verts, vtkCellArray *lines, 
                                vtkCellArray *polys, 
                                vtkPointData *inPd, vtkPointData *outPd,
                                vtkCellData *inCd, vtkIdType cellId,
                                vtkCellData *outCd)
{
  // For each tetra, contour it
  int i, j;
  vtkIdType ptId, localId;
  int numTets = this->TetraIds->GetNumberOfIds() / 4;
  for (i=0; i<numTets; i++)
    {
    for (j=0; j<4; j++)
      {
      localId = this->TetraIds->GetId(4*i+j);
      ptId = this->PointIds->GetId(localId);
      this->Tetra->PointIds->SetId(j,ptId);
      this->Tetra->Points->SetPoint(j,this->TetraPoints->GetPoint(4*i+j));
      this->TetraScalars->SetValue(j,cellScalars->GetTuple1(localId));
      }
    this->Tetra->Contour(value,this->TetraScalars,locator,verts,lines,polys,
                         inPd,outPd,inCd,cellId,outCd);
    }
}
    


void vtkConvexPointSet::Clip(float value, 
                             vtkDataArray *cellScalars, 
                             vtkPointLocator *locator, vtkCellArray *tets,
                             vtkPointData *inPD, vtkPointData *outPD,
                             vtkCellData *inCD, vtkIdType cellId,
                             vtkCellData *outCD, int insideOut)
{
  // For each tetra, contour it
  int i, j;
  vtkIdType ptId, localId;
  int numTets = this->TetraIds->GetNumberOfIds() / 4;
  for (i=0; i<numTets; i++)
    {
    for (j=0; j<4; j++)
      {
      localId = this->TetraIds->GetId(4*i+j);
      ptId = this->PointIds->GetId(localId);
      this->Tetra->PointIds->SetId(j,ptId);
      this->Tetra->Points->SetPoint(j,this->TetraPoints->GetPoint(4*i+j));
      this->TetraScalars->SetValue(j,cellScalars->GetTuple1(localId));
      }
    this->Tetra->Clip(value,this->TetraScalars,locator,tets,inPD,outPD,inCD,
                      cellId, outCD, insideOut);
    }
}

int vtkConvexPointSet::CellBoundary(int subId, float pcoords[3], 
                                    vtkIdList *pts)
{
  int i, status, returnStatus=(-1);
  float p[3], *x, dist2, minDist2=VTK_LARGE_FLOAT, pMin[3], closest[3], pc[3];
  float weights[4];

  // Get the current global coordinate
  this->EvaluateLocation(subId, pcoords, p, weights);

  // Find the closest point
  vtkIdType numPts = this->PointIds->GetNumberOfIds();
  for (i=0; i < numPts; i++)
    {
    x = this->Points->GetPoint(i);
    dist2 = vtkMath::Distance2BetweenPoints(x,p);
    if ( dist2 < minDist2 )
      {
      pMin[0] = x[0];
      pMin[1] = x[1];
      pMin[2] = x[2];
      minDist2 = dist2;
      }
    }

  // Get the faces connected to the point, find the closest face
  this->BoundaryTris->Reset();
  this->Triangulator->AddTriangles(this->BoundaryTris);

  vtkIdType npts, *tpts=0;
  for ( minDist2=VTK_LARGE_FLOAT, this->BoundaryTris->InitTraversal(); 
        this->BoundaryTris->GetNextCell(npts,tpts); )
    {
    this->Triangle->PointIds->SetId(0,tpts[0]);
    this->Triangle->PointIds->SetId(1,tpts[1]);
    this->Triangle->PointIds->SetId(2,tpts[2]);
    this->Triangle->Points->SetPoint(0,this->Points->GetPoint(tpts[0]));
    this->Triangle->Points->SetPoint(1,this->Points->GetPoint(tpts[1]));
    this->Triangle->Points->SetPoint(2,this->Points->GetPoint(tpts[2]));
    status = this->Triangle->
      EvaluatePosition(pMin, closest, subId, pc, dist2, weights);
    if ( status != -1 && dist2 < minDist2 && dist2 > 0)
      {
      returnStatus = 1;
      pts->SetNumberOfIds(3);
      pts->SetId(0,this->PointIds->GetId(tpts[0]));
      pts->SetId(1,this->PointIds->GetId(tpts[1]));
      pts->SetId(2,this->PointIds->GetId(tpts[2]));
      minDist2 = dist2;
      }
    }

  return returnStatus;
}

int vtkConvexPointSet::EvaluatePosition(float x[3],
                                        float* vtkNotUsed(closestPoint),
                                        int& subId, float pcoords[3],
                                        float& minDist2, float *weights)
{
  float pc[3], dist2;
  int ignoreId, i, j, returnStatus=0, status;
  float tempWeights[4];
  float closest[3];
  vtkIdType ptId;
  int numTets = this->TetraIds->GetNumberOfIds() / 4;


  for (minDist2=VTK_LARGE_FLOAT, i=0; i<numTets; i++)
    {
    for (j=0; j<4; j++)
      {
      ptId = this->PointIds->GetId(this->TetraIds->GetId(4*i+j));
      this->Tetra->PointIds->SetId(j,ptId);
      this->Tetra->Points->SetPoint(j,this->TetraPoints->GetPoint(4*i+j));
      }

    status = this->Tetra->EvaluatePosition(x,closest,ignoreId,pc,dist2,
                                           tempWeights);
    if ( status != -1 && dist2 < minDist2 )
      {
      returnStatus = status;
      minDist2 = dist2;
      subId = i;
      pcoords[0] = pc[0];
      pcoords[1] = pc[1];
      pcoords[2] = pc[2];
      weights[0] = tempWeights[0];
      weights[1] = tempWeights[1];
      weights[2] = tempWeights[2];
      weights[3] = tempWeights[3];
      }
    }

  return returnStatus;
}

void vtkConvexPointSet::EvaluateLocation(int& subId, float pcoords[3], 
                                         float x[3], float *weights)
{
  vtkIdType ptId;

  for (int j=0; j<4; j++)
    {
    ptId = this->PointIds->GetId(this->TetraIds->GetId(4*subId+j));
    this->Tetra->PointIds->SetId(j,ptId);
    this->Tetra->Points->SetPoint(j,this->TetraPoints->GetPoint(4*subId+j));
    }

  this->Tetra->EvaluateLocation(subId, pcoords, x, weights);
}

int vtkConvexPointSet::IntersectWithLine(float p1[3], float p2[3], float tol, 
                                         float& minT, float x[3], 
                                         float pcoords[3], int& subId)
{
  int subTest, i, j;
  vtkIdType ptId;
  float t, pc[3], xTemp[3];
  

  int numTets = this->TetraIds->GetNumberOfIds() / 4;
  int status = 0;
  
  for (minT=VTK_LARGE_FLOAT, i=0; i<numTets; i++)
    {
    for (j=0; j<4; j++)
      {
      ptId = this->PointIds->GetId(this->TetraIds->GetId(4*i+j));
      this->Tetra->PointIds->SetId(j,ptId);
      this->Tetra->Points->SetPoint(j,this->TetraPoints->GetPoint(4*i+j));
      }

    if (this->Tetra->IntersectWithLine(p1,p2,tol,t,xTemp,pc,subTest) &&
      t < minT )
      {
      status = 1;
      subId = i;
      minT = t;
      x[0] = xTemp[0];
      x[1] = xTemp[1];
      x[2] = xTemp[2];
      pcoords[0] = pc[0];
      pcoords[1] = pc[1];
      pcoords[2] = pc[2];
      }
    }
  
  return status;
}

void vtkConvexPointSet::Derivatives(int subId, float pcoords[3], 
                                    float *values, int dim, float *derivs)
{
  vtkIdType ptId;

  for (int j=0; j<4; j++)
    {
    ptId = this->PointIds->GetId(this->TetraIds->GetId(4*subId+j));
    this->Tetra->PointIds->SetId(j,ptId);
    this->Tetra->Points->SetPoint(j,this->TetraPoints->GetPoint(4*subId+j));
    }

  this->Tetra->Derivatives(subId, pcoords, values, dim, derivs);
}

float *vtkConvexPointSet::GetParametricCoords()
{
  int numPts = this->PointIds->GetNumberOfIds();
  if ( ! this->ParametricCoords )
    {
    this->ParametricCoords = vtkFloatArray::New();
    }

  this->ParametricCoords->SetNumberOfComponents(3);
  this->ParametricCoords->SetNumberOfTuples(numPts);
  float p[3], *x, *bounds = this->GetBounds();
  int i, j;
  for (i=0; i < numPts; i++)
    {
    x = this->Points->GetPoint(i);
    for (j=0; j<3; j++)
      {
      p[j] = (x[j] - bounds[2*j]) / (bounds[2*j+1] - bounds[2*j]);
      }
    this->ParametricCoords->SetTuple(i,p);
    }

  return this->ParametricCoords->GetPointer(0);
}
