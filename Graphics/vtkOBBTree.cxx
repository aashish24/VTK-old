/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to Peter C. Everett <pce@world.std.com> for
             improvements and enhancements to vtkOBBTree class.


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
#include "vtkOBBTree.h"
#include "vtkMath.h"
#include "vtkLine.h"
#include "vtkPolyData.h"
#include "vtkMatrix4x4.h"

#define vtkCELLTRIANGLES(CELLPTIDS, TYPE, IDX, PTID0, PTID1, PTID2) \
	{ switch( TYPE ) \
	  { \
	  case VTK_TRIANGLE: \
	  case VTK_POLYGON: \
	  case VTK_QUAD: \
	    PTID0 = CELLPTIDS[0]; \
	    PTID1 = CELLPTIDS[(IDX)+1]; \
	    PTID2 = CELLPTIDS[(IDX)+2]; \
	    break; \
	  case VTK_TRIANGLE_STRIP: \
	    PTID0 = CELLPTIDS[IDX]; \
	    PTID1 = CELLPTIDS[(IDX)+1+((IDX)&1)]; \
	    PTID2 = CELLPTIDS[(IDX)+2-((IDX)&1)]; \
	    break; \
	  default: \
	    PTID0 = PTID1 = PTID2 = -1; \
	  } }

vtkOBBNode::vtkOBBNode()
{
  this->Cells = NULL;
  this->Parent = NULL;
  this->Kids = NULL;
}

vtkOBBNode::~vtkOBBNode()
{
  if (this->Kids) 
    {
    delete [] this->Kids;
    }
  if (this->Cells) 
    { 
    this->Cells->Delete();
    }
}

// Construct with automatic computation of divisions, averaging
// 25 cells per octant.
vtkOBBTree::vtkOBBTree()
{
  this->DataSet = NULL;
  this->Level = 4;
  this->MaxLevel = 12;
  this->Automatic = 1;
  this->Tolerance = 0.01;
  this->Tree = NULL;
}

vtkOBBTree::~vtkOBBTree()
{
  this->FreeSearchStructure();
}

void vtkOBBTree::FreeSearchStructure()
{
  if ( this->Tree )
    {
    this->DeleteTree(this->Tree);
    delete this->Tree;
    this->Tree = NULL;
    }
}

void vtkOBBTree::DeleteTree(vtkOBBNode *OBBptr)
{
  if ( OBBptr->Kids != NULL )
    {
    this->DeleteTree(OBBptr->Kids[0]);
    this->DeleteTree(OBBptr->Kids[1]);
    delete OBBptr->Kids[0];
    delete OBBptr->Kids[1];
    }
}

// Compute an OBB from the list of points given. Return the corner point
// and the three axes defining the orientation of the OBB. Also return
// a sorted list of relative "sizes" of axes for comparison purposes.
void vtkOBBTree::ComputeOBB(vtkPoints *pts, float corner[3], float max[3],
                            float mid[3], float min[3], float size[3])
{
  int numPts, i, pointId;
  float *x, mean[3], xp[3], *v[3], v0[3], v1[3], v2[3];
  float *a[3], a0[3], a1[3], a2[3];
  float tMin[3], tMax[3], closest[3], t;

  //
  // Compute mean
  //
  numPts = pts->GetNumberOfPoints();
  mean[0] = mean[1] = mean[2] = 0.0;
  for (pointId=0; pointId < numPts; pointId++ )
    {
    x = pts->GetPoint(pointId);
    for (i=0; i < 3; i++)
      {
      mean[i] += x[i];
      }
    }
  for (i=0; i < 3; i++)
    {
    mean[i] /= numPts;
    }

  //
  // Compute covariance matrix
  //
  a[0] = a0; a[1] = a1; a[2] = a2; 
  for (i=0; i < 3; i++)
    {
    a0[i] = a1[i] = a2[i] = 0.0;
    }

  for (pointId=0; pointId < numPts; pointId++ )
    {
    x = pts->GetPoint(pointId);
    xp[0] = x[0] - mean[0]; xp[1] = x[1] - mean[1]; xp[2] = x[2] - mean[2];
    for (i=0; i < 3; i++)
      {
      a0[i] += xp[0] * xp[i];
      a1[i] += xp[1] * xp[i];
      a2[i] += xp[2] * xp[i];
      }
    }//for all points

  for (i=0; i < 3; i++)
    {
    a0[i] /= numPts;
    a1[i] /= numPts;
    a2[i] /= numPts;
    }

  //
  // Extract axes (i.e., eigenvectors) from covariance matrix. 
  //
  v[0] = v0; v[1] = v1; v[2] = v2; 
  vtkMath::Jacobi(a,size,v);
  max[0] = v[0][0]; max[1] = v[1][0]; max[2] = v[2][0];
  mid[0] = v[0][1]; mid[1] = v[1][1]; mid[2] = v[2][1];
  min[0] = v[0][2]; min[1] = v[1][2]; min[2] = v[2][2];

  for (i=0; i < 3; i++)
    {
    a[0][i] = mean[i] + max[i];
    a[1][i] = mean[i] + mid[i];
    a[2][i] = mean[i] + min[i];
    }

  //
  // Create oriented bounding box by projecting points onto eigenvectors.
  //
  tMin[0] = tMin[1] = tMin[2] = VTK_LARGE_FLOAT;
  tMax[0] = tMax[1] = tMax[2] = -VTK_LARGE_FLOAT;

  for (pointId=0; pointId < numPts; pointId++ )
    {
    x = pts->GetPoint(pointId);
    for (i=0; i < 3; i++)
      {
      vtkLine::DistanceToLine(x, mean, a[i], t, closest);
      if ( t < tMin[i] )
	{
	tMin[i] = t;
	}
      if ( t > tMax[i] )
	{
	tMax[i] = t;
	}
      }
    }//for all points

  for (i=0; i < 3; i++)
    {
    corner[i] = mean[i] + tMin[0]*max[i] + tMin[1]*mid[i] + tMin[2]*min[i];

    max[i] = (tMax[0] - tMin[0]) * max[i];
    mid[i] = (tMax[1] - tMin[1]) * mid[i];
    min[i] = (tMax[2] - tMin[2]) * min[i];
    }
}

// a method to compute the OBB of a dataset without having to go through the 
// Execute method; It does set 
void vtkOBBTree::ComputeOBB(vtkDataSet *input, float corner[3], float max[3],
                            float mid[3], float min[3], float size[3])
{
  int numPts, numCells, i;
  vtkIdList *cellList;
  vtkDataSet *origDataSet;

  vtkDebugMacro(<<"Computing OBB");

  if ( input == NULL || (numPts = input->GetNumberOfPoints()) < 1 ||
      (input->GetNumberOfCells()) < 1 )
    {
    vtkErrorMacro(<<"Can't compute OBB - no data available!");
    return;
    }
  numCells = input->GetNumberOfCells();

  // save previous value of DataSet and reset after calling ComputeOBB because
  // computeOBB used this->DataSet internally
  origDataSet = this->DataSet;
  this->DataSet = input;

  // these are other member variables that ComputeOBB requires
  this->OBBCount = 0;
  this->InsertedPoints = new int[numPts];
  for (i=0; i < numPts; i++)
    {
    this->InsertedPoints[i] = 0;
    }
  this->PointsList = vtkPoints::New();
  this->PointsList->Allocate(numPts);

  cellList = vtkIdList::New();
  cellList->Allocate(numCells);
  for (i=0; i < numCells; i++)
    {
    cellList->InsertId(i,i);
    }

  this->ComputeOBB(cellList, corner, max, mid, min, size);

  this->DataSet = origDataSet;
  delete [] this->InsertedPoints;
  this->PointsList->Delete();
  cellList->Delete();
  }


// Compute an OBB from the list of cells given. Return the corner point
// and the three axes defining the orientation of the OBB. Also return
// a sorted list of relative "sizes" of axes for comparison purposes.
void vtkOBBTree::ComputeOBB(vtkIdList *cells, float corner[3], float max[3],
                            float mid[3], float min[3], float size[3])
  {
  int numCells, i, j, k, cellId, numPts, type, ptId, pId, qId, rId;
  int *ptIds;
  float *p, *q, *r, mean[3], xp[3], *v[3], v0[3], v1[3], v2[3];
  float *a[3], a0[3], a1[3], a2[3];
  float tMin[3], tMax[3], closest[3], t;
  float dp0[3], dp1[3], tri_mass, tot_mass, c[3];
  
  this->OBBCount++;
  this->PointsList->Reset();
  //
  // Compute mean & moments
  //
  
  numCells = cells->GetNumberOfIds();
  mean[0] = mean[1] = mean[2] = 0.0;
  tot_mass = 0.0;
  a[0] = a0; a[1] = a1; a[2] = a2;
  for ( i=0; i<3; i++ )
    {
    a0[i] = a1[i] = a2[i] = 0.0;
    }
  
  for ( i=0; i < numCells; i++ )
    {
    cellId = cells->GetId( i );
    type = this->DataSet->GetCellType( cellId );
    ((vtkPolyData *)this->DataSet)->GetCellPoints( cellId, numPts, ptIds );
    for ( j=0; j<numPts-2; j++ )
      {
      vtkCELLTRIANGLES( ptIds, type, j, pId, qId, rId );
      if ( pId < 0 )
        {
        continue;
        }
      p = this->DataSet->GetPoint( pId );
      q = this->DataSet->GetPoint( qId );
      r = this->DataSet->GetPoint( rId );
      // p, q, and r are the oriented triangle points.
      // Compute the components of the moment of inertia tensor.
      for ( k=0; k<3; k++ )
        {
        // two edge vectors
        dp0[k] = q[k] - p[k];
        dp1[k] = r[k] - p[k];
        // centroid
        c[k] = (p[k] + q[k] + r[k])/3;
        }
      vtkMath::Cross( dp0, dp1, xp );
      tri_mass = 0.5*vtkMath::Norm( xp );
      tot_mass += tri_mass;
      for ( k=0; k<3; k++ ) 
        {
        mean[k] += tri_mass*c[k];
        }
      
      // on-diagonal terms
      a0[0] += tri_mass*(9*c[0]*c[0] + p[0]*p[0] + q[0]*q[0] + r[0]*r[0])/12;
      a1[1] += tri_mass*(9*c[1]*c[1] + p[1]*p[1] + q[1]*q[1] + r[1]*r[1])/12;
      a2[2] += tri_mass*(9*c[2]*c[2] + p[2]*p[2] + q[2]*q[2] + r[2]*r[2])/12;
      
      // off-diagonal terms
      a0[1] += tri_mass*(9*c[0]*c[1] + p[0]*p[1] + q[0]*q[1] + r[0]*r[1])/12;
      a0[2] += tri_mass*(9*c[0]*c[2] + p[0]*p[2] + q[0]*q[2] + r[0]*r[2])/12;
      a1[2] += tri_mass*(9*c[1]*c[2] + p[1]*p[2] + q[1]*q[2] + r[1]*r[2])/12;
      } // end foreach triangle

    // While computing cell moments, gather all the cell's
    // point coordinates into a single list.
    //
    for ( j=0; j < numPts; j++ )
      {
      if ( this->InsertedPoints[ptIds[j]] != this->OBBCount )
        {
        this->InsertedPoints[ptIds[j]] = this->OBBCount;
        this->PointsList->InsertNextPoint(this->DataSet->GetPoint(ptIds[j]));
        }
      }//for all points of this cell
    } // end foreach cell

  
  // normalize data
  for ( i=0; i<3; i++ )
    {
    mean[i] = mean[i]/tot_mass;
    }
  
  // matrix is symmetric
  a1[0] = a0[1];
  a2[0] = a0[2];
  a2[1] = a1[2];
  
  // get covariance from moments
  for ( i=0; i<3; i++ )
    {
    for ( j=0; j<3; j++ )
      {
      a[i][j] = a[i][j]/tot_mass - mean[i]*mean[j];
      }
    }
    
    //
    // Extract axes (i.e., eigenvectors) from covariance matrix. 
    //
    v[0] = v0; v[1] = v1; v[2] = v2; 
    vtkMath::Jacobi(a,size,v);
    max[0] = v[0][0]; max[1] = v[1][0]; max[2] = v[2][0];
    mid[0] = v[0][1]; mid[1] = v[1][1]; mid[2] = v[2][1];
    min[0] = v[0][2]; min[1] = v[1][2]; min[2] = v[2][2];
    
    for (i=0; i < 3; i++)
      {
      a[0][i] = mean[i] + max[i];
      a[1][i] = mean[i] + mid[i];
      a[2][i] = mean[i] + min[i];
      }
    
    //
    // Create oriented bounding box by projecting points onto eigenvectors.
    //
    tMin[0] = tMin[1] = tMin[2] = VTK_LARGE_FLOAT;
    tMax[0] = tMax[1] = tMax[2] = -VTK_LARGE_FLOAT;
    
    numPts = this->PointsList->GetNumberOfPoints();
    for (ptId=0; ptId < numPts; ptId++ )
      {
      p = this->PointsList->GetPoint(ptId);
      for (i=0; i < 3; i++)
        {
        vtkLine::DistanceToLine(p, mean, a[i], t, closest);
        if ( t < tMin[i] ) 
          {
          tMin[i] = t;
          }
        if ( t > tMax[i] ) 
          {
          tMax[i] = t;
          }
        }
      }//for all points
    
    for (i=0; i < 3; i++)
      {
      corner[i] = mean[i] + tMin[0]*max[i] + tMin[1]*mid[i] + tMin[2]*min[i];
      
      max[i] = (tMax[0] - tMin[0]) * max[i];
      mid[i] = (tMax[1] - tMin[1]) * mid[i];
      min[i] = (tMax[2] - tMin[2]) * min[i];
      }
}

// Return intersection point of line defined by two points (a0,a1) in dataset
// coordinate system; returning cellId (or -1 if no intersection). The 
// argument list returns the intersection parametric coordinate, t, along 
// the line; the coordinate of intersection, x[3]; the cell parametric
// coordinates, pcoords[3]; and subId of the cell. (Not yet implemented.)
int vtkOBBTree::IntersectWithLine(float a0[3], float a1[3], float tol,
                                  float& t, float x[3], float pcoords[3],
                                  int &subId)
{
  int cellId = -1;

  return this->IntersectWithLine( a0, a1, tol, t, x, pcoords,
                                  subId, cellId );
}

// Return intersection point of line defined by two points (a0,a1) in dataset
// coordinate system; returning cellId (or -1 if no intersection). The 
// argument list returns the intersection parametric coordinate, t, along 
// the line; the coordinate of intersection, x[3]; the cell parametric
// coordinates, pcoords[3]; and subId of the cell. (Not yet implemented.)
int vtkOBBTree::IntersectWithLine(float a0[3], float a1[3], float tol,
                                  float& t, float x[3], float pcoords[3],
                                  int &subId, int &cellId)
{
  vtkOBBNode **OBBstack, *node;
  vtkIdList *cells;
  vtkCell *cell;
  int depth, ii, foundIntersection = 0, bestIntersection = 0;
  float tBest = VTK_LARGE_FLOAT, xBest[3], pcoordsBest[3];
  int subIdBest, thisId, cellIdBest = -1;

  OBBstack = new vtkOBBNode *[this->GetLevel()+1];
  OBBstack[0] = this->Tree;
  depth = 1;
  while( depth > 0 )
    { // simulate recursion without the overhead or limitations
    depth--;
    node = OBBstack[depth];
    if ( this->LineIntersectsNode( node, a0, a1 ) )
      {
      if ( node->Kids == NULL )
        { // then this is a leaf node...get Cells
        cells = node->Cells;
        for ( ii=0; ii<cells->GetNumberOfIds(); ii++ )
          {
          thisId = cells->GetId(ii);
          cell = this->DataSet->GetCell( thisId );
          if ( cell->IntersectWithLine( a0, a1, tol, t, x,
                                        pcoords, subId ) )
            { // line intersects cell, but is it the best one?
            foundIntersection++;
            if ( t < tBest )
              { // Yes, it's the best.
              bestIntersection = foundIntersection;
              tBest = t;
              xBest[0] = x[0]; xBest[1] = x[1]; xBest[2] = x[2];
              pcoordsBest[0] = pcoords[0]; pcoordsBest[1] = pcoords[1];
              pcoordsBest[2] = pcoords[2];
              subIdBest = subId;
              cellIdBest = thisId;
              }
            }
          }
        }
      else
        { // push kids onto stack
        OBBstack[depth] = node->Kids[0];
        OBBstack[depth+1] = node->Kids[1];
        depth += 2;
        }
      }
    } // end while

  if ( foundIntersection != bestIntersection )
    {
    t = tBest;
    x[0] = xBest[0]; x[1] = xBest[1]; x[2] = xBest[2];
    pcoords[0] = pcoordsBest[0]; pcoords[1] = pcoordsBest[1];
    pcoords[2] = pcoordsBest[2];
    subId= subIdBest ;
    }

  if ( cellIdBest < 0 )
    {
    return 0;
    }
  else
    {
    cellId = cellIdBest;
    return 1;
    }
}

void vtkOBBNode::DebugPrintTree( int level, double *leaf_vol,
                                 int *minCells, int *maxCells )
  {
  float xp[3], volume, c[3];
  int nCells, i;

  if ( this->Cells != NULL )
    {
    nCells = this->Cells->GetNumberOfIds();
    }
  else
    {
    nCells = 0;
    }
  
  vtkMath::Cross( this->Axes[0], this->Axes[1], xp );
  volume = fabs( vtkMath::Dot( xp, this->Axes[2] ) );
  for ( i=0; i<3; i++ )
    {
    c[i] = this->Corner[i] + 0.5*this->Axes[0][i] + 0.5*this->Axes[1][i]
      + 0.5*this->Axes[2][i];
    }
  
  for ( i=0; i<level; i++ )
    {
    cout<<"  ";
    }
  cout <<level<<" # Cells: "<<nCells<<", Volume: "<<volume<<"\n";
  for ( i=0; i<level; i++ )
    {
    cout<<"  ";
    }
  cout << "    " << vtkMath::Norm( this->Axes[0] ) << " X " <<
                    vtkMath::Norm( this->Axes[1] ) << " X " <<
                    vtkMath::Norm( this->Axes[2] ) << "\n";
  for ( i=0; i<level; i++ )
    {
    cout<<"  ";
    }
  cout << "    Center: " << c[0] << " " << c[1] << " " << c[2] << "\n";
  if ( nCells != 0 )
    {
    *leaf_vol += volume;
    if ( nCells < *minCells )
      *minCells = nCells;
    if ( nCells > *maxCells )
      *maxCells = nCells;
    }
  if ( this->Kids != NULL )
    {
    this->Kids[0]->DebugPrintTree( level+1, leaf_vol, minCells, maxCells );
    this->Kids[1]->DebugPrintTree( level+1, leaf_vol, minCells, maxCells );
    }
  }

//
//  Method to form subdivision of space based on the cells provided and
//  subject to the constraints of levels and NumberOfCellsInOctant.
//  The result is directly addressable and of uniform subdivision.
//
void vtkOBBTree::BuildLocator()
{
  int numPts, numCells, i;
  vtkIdList *cellList;

  vtkDebugMacro(<<"Building OBB tree");
  if ( (this->Tree != NULL) && (this->BuildTime > this->MTime)
       && (this->BuildTime > this->DataSet->GetMTime()) )
    {
    return;
    }

  numPts = this->DataSet->GetNumberOfPoints();
  numCells = this->DataSet->GetNumberOfCells();
  if ( this->DataSet == NULL || numPts < 1 || numCells < 1 )
    {
    vtkErrorMacro(<<"Can't build OBB tree - no data available!");
    return;
    }

  this->OBBCount = 0;
  this->InsertedPoints = new int[numPts];
  for (i=0; i < numPts; i++)
    {
    this->InsertedPoints[i] = 0;
    }
  this->PointsList = vtkPoints::New();
  this->PointsList->Allocate(numPts);

  //
  // Begin recursively creating OBB's
  //
  cellList = vtkIdList::New();
  cellList->Allocate(numCells);
  for (i=0; i < numCells; i++)
    {
    cellList->InsertId(i,i);
    }

  this->Tree = new vtkOBBNode;
  this->DeepestLevel = 0;
  this->BuildTree(cellList,this->Tree,0);
  this->Level = this->DeepestLevel;

  vtkDebugMacro(<<"# Cells: " << numCells << ", Deepest tree level: " <<
                this->DeepestLevel <<", Created: " << OBBCount << " OBB nodes");
  if ( this->GetDebug() > 1 )
    { // print tree
    double volume = 0.0;
    int minCells = 65535, maxCells = 0;
    this->Tree->DebugPrintTree( 0, &volume, &minCells, &maxCells );
    cout<<"Total leafnode volume = "<<volume<<"\n";
    cout<<"Min leaf cells: "<<minCells<<", Max leaf cells: "<<maxCells<<"\n";
    cout.flush();
    }

  //
  // Clean up
  //
  delete [] this->InsertedPoints;
  this->PointsList->Delete();

  this->BuildTime.Modified();
}

// NOTE: for better memory usage this recursive method
// frees its first argument
void vtkOBBTree::BuildTree(vtkIdList *cells, vtkOBBNode *OBBptr, int level)
{
  int i, j, numCells=cells->GetNumberOfIds();
  int cellId, ptId;
  vtkIdList *cellPts = vtkIdList::New();
  float size[3];

  if ( level > this->DeepestLevel )
    {
    this->DeepestLevel = level;
    }
  //
  // Now compute the OBB
  //
  this->ComputeOBB(cells, OBBptr->Corner, OBBptr->Axes[0], 
                   OBBptr->Axes[1], OBBptr->Axes[2], size);

  //
  // Check whether to continue recursing; if so, create two children and
  // assign cells to appropriate child.
  //
  if ( level < this->MaxLevel && numCells > this->NumberOfCellsPerBucket )
    {
    vtkIdList *LHlist = vtkIdList::New();
    LHlist->Allocate(cells->GetNumberOfIds()/2);
    vtkIdList *RHlist = vtkIdList::New();
    RHlist->Allocate(cells->GetNumberOfIds()/2);
    float n[3], p[3], c[3], *x, val, ratio, bestRatio;
    int negative, positive, splitAcceptable, splitPlane;
    int foundBestSplit, bestPlane=0, numPts;
    int numInLHnode, numInRHnode;

    //loop over three split planes to find acceptable one
    for (i=0; i < 3; i++) //compute split point
      {
      p[i] = OBBptr->Corner[i] + OBBptr->Axes[0][i]/2.0 + 
             OBBptr->Axes[1][i]/2.0 + OBBptr->Axes[2][i]/2.0;
      }

    bestRatio = 1.0; // worst case ratio
    foundBestSplit = 0;
    for (splitPlane=0,splitAcceptable=0; !splitAcceptable && splitPlane < 3; )
      {
      // compute split normal
      for (i=0 ; i < 3; i++)
        {
        n[i] = OBBptr->Axes[splitPlane][i];
        }
      vtkMath::Normalize(n);

      //traverse cells, assigning to appropriate child list as necessary
      for ( i=0; i < numCells; i++ )
        {
        cellId = cells->GetId(i);
        this->DataSet->GetCellPoints(cellId, *cellPts);
        c[0] = c[1] = c[2] = 0.0;
        numPts = cellPts->GetNumberOfIds();
        for ( negative=positive=j=0; j < numPts; j++ )
          {
          ptId = cellPts->GetId(j);
          x = this->DataSet->GetPoint(ptId);
          val = n[0]*(x[0]-p[0]) + n[1]*(x[1]-p[1]) + n[2]*(x[2]-p[2]);
          c[0] += x[0];
          c[1] += x[1];
          c[2] += x[2];
          if ( val < 0.0 ) 
            {
            negative = 1;
            }
          else 
            {
            positive = 1;
            }
          }

        if ( negative && positive )
          { // Use centroid to decide straddle cases
          c[0] /= numPts;
          c[1] /= numPts;
          c[2] /= numPts;
          if ( n[0]*(c[0]-p[0])+n[1]*(c[1]-p[1])+n[2]*(c[2]-p[2]) < 0.0 )
            {
            LHlist->InsertNextId(cellId);
            }
          else
            {
            RHlist->InsertNextId(cellId);
            }
          }
        else
          {
          if ( negative )
            {
            LHlist->InsertNextId(cellId);
            }
          else
            {
            RHlist->InsertNextId(cellId);
            }
          }
        }//for all cells

      //evaluate this split
      numInLHnode = LHlist->GetNumberOfIds();
      numInRHnode = RHlist->GetNumberOfIds();
      ratio = fabs(((double)numInRHnode-numInLHnode)/numCells);

      //see whether we've found acceptable split plane       
      if ( ratio < 0.6 || foundBestSplit ) //accept right off the bat
        { 
        splitAcceptable = 1;
        }
      else
        { //not a great split try another
        LHlist->Reset();
        RHlist->Reset();
        if ( ratio < bestRatio )
          {
          bestRatio = ratio;
          bestPlane = splitPlane;
          }
        if ( ++splitPlane == 3 && bestRatio < 0.95 )
          { //at closing time, even the ugly ones look good
          splitPlane = bestPlane;
          foundBestSplit = 1;
          }
        } //try another split

      }//for each split

    if ( splitAcceptable ) //otherwise recursion terminates
      {
      vtkOBBNode *LHnode= new vtkOBBNode;
      vtkOBBNode *RHnode= new vtkOBBNode;
      OBBptr->Kids = new vtkOBBNode *[2];
      OBBptr->Kids[0] = LHnode;
      OBBptr->Kids[1] = RHnode;
      LHnode->Parent = OBBptr;
      RHnode->Parent = OBBptr;

      cells->Delete(); cells = NULL; //don't need to keep anymore
      this->BuildTree(LHlist, LHnode, level+1);
      this->BuildTree(RHlist, RHnode, level+1);
      }
    else
      {
      // free up local objects
      LHlist->Delete();
      RHlist->Delete();
      }
    }//if should build tree

  if ( cells && this->RetainCellLists ) 
    {
    cells->Squeeze();
    OBBptr->Cells = cells;
    }
  else if ( cells )
    {
    cells->Delete();
    }
  cellPts->Delete();
}

// Create polygonal representation for OBB tree at specified level. If 
// level < 0, then the leaf OBB nodes will be gathered. The aspect ratio (ar)
// and line diameter (d) are used to control the building of the 
// representation. If a OBB node edge ratio's are greater than ar, then the
// dimension of the OBB is collapsed (OBB->plane->line). A "line" OBB will be
// represented either as two crossed polygons, or as a line, depending on
// the relative diameter of the OBB compared to the diameter (d).

void vtkOBBTree::GenerateRepresentation(int level, vtkPolyData *pd)
{
  vtkPoints *pts;
  vtkCellArray *polys;

  if ( this->Tree == NULL )
    {
    vtkErrorMacro(<<"No tree to generate representation from");
    return;
    }

  pts = vtkPoints::New();
  pts->Allocate(5000);
  polys = vtkCellArray::New();
  polys->Allocate(10000);
  this->GeneratePolygons(this->Tree,0,level,pts,polys);

  pd->SetPoints(pts);
  pts->Delete();
  pd->SetPolys(polys);
  polys->Delete();
  pd->Squeeze();
}

void vtkOBBTree::GeneratePolygons(vtkOBBNode *OBBptr, int level, int repLevel,
                                  vtkPoints *pts, vtkCellArray *polys)

{
  if ( level == repLevel || (repLevel < 0 && OBBptr->Kids == NULL) )
    {
    float x[3];
    int ptIds[4], cubeIds[8];

    x[0] = OBBptr->Corner[0];
    x[1] = OBBptr->Corner[1];
    x[2] = OBBptr->Corner[2];
    cubeIds[0] = pts->InsertNextPoint(x);

    x[0] = OBBptr->Corner[0] + OBBptr->Axes[0][0];
    x[1] = OBBptr->Corner[1] + OBBptr->Axes[0][1];
    x[2] = OBBptr->Corner[2] + OBBptr->Axes[0][2];
    cubeIds[1] = pts->InsertNextPoint(x);

    x[0] = OBBptr->Corner[0] + OBBptr->Axes[1][0];
    x[1] = OBBptr->Corner[1] + OBBptr->Axes[1][1];
    x[2] = OBBptr->Corner[2] + OBBptr->Axes[1][2];
    cubeIds[2] = pts->InsertNextPoint(x);

    x[0] = OBBptr->Corner[0] + OBBptr->Axes[0][0] + OBBptr->Axes[1][0];
    x[1] = OBBptr->Corner[1] + OBBptr->Axes[0][1] + OBBptr->Axes[1][1];
    x[2] = OBBptr->Corner[2] + OBBptr->Axes[0][2] + OBBptr->Axes[1][2];
    cubeIds[3] = pts->InsertNextPoint(x);

    x[0] = OBBptr->Corner[0] + OBBptr->Axes[2][0];
    x[1] = OBBptr->Corner[1] + OBBptr->Axes[2][1];
    x[2] = OBBptr->Corner[2] + OBBptr->Axes[2][2];
    cubeIds[4] = pts->InsertNextPoint(x);

    x[0] = OBBptr->Corner[0] + OBBptr->Axes[0][0] + OBBptr->Axes[2][0];
    x[1] = OBBptr->Corner[1] + OBBptr->Axes[0][1] + OBBptr->Axes[2][1];
    x[2] = OBBptr->Corner[2] + OBBptr->Axes[0][2] + OBBptr->Axes[2][2];
    cubeIds[5] = pts->InsertNextPoint(x);

    x[0] = OBBptr->Corner[0] + OBBptr->Axes[1][0] + OBBptr->Axes[2][0];
    x[1] = OBBptr->Corner[1] + OBBptr->Axes[1][1] + OBBptr->Axes[2][1];
    x[2] = OBBptr->Corner[2] + OBBptr->Axes[1][2] + OBBptr->Axes[2][2];
    cubeIds[6] = pts->InsertNextPoint(x);

    x[0] = OBBptr->Corner[0] + OBBptr->Axes[0][0] + OBBptr->Axes[1][0] 
           + OBBptr->Axes[2][0];
    x[1] = OBBptr->Corner[1] + OBBptr->Axes[0][1] + OBBptr->Axes[1][1] 
           + OBBptr->Axes[2][1];
    x[2] = OBBptr->Corner[2] + OBBptr->Axes[0][2] + OBBptr->Axes[1][2] 
           + OBBptr->Axes[2][2];
    cubeIds[7] = pts->InsertNextPoint(x);

    ptIds[0] = cubeIds[0]; ptIds[1] = cubeIds[2]; 
    ptIds[2] = cubeIds[3]; ptIds[3] = cubeIds[1];
    polys->InsertNextCell(4,ptIds);

    ptIds[0] = cubeIds[0]; ptIds[1] = cubeIds[1]; 
    ptIds[2] = cubeIds[5]; ptIds[3] = cubeIds[4];
    polys->InsertNextCell(4,ptIds);

    ptIds[0] = cubeIds[0]; ptIds[1] = cubeIds[4]; 
    ptIds[2] = cubeIds[6]; ptIds[3] = cubeIds[2];
    polys->InsertNextCell(4,ptIds);

    ptIds[0] = cubeIds[1]; ptIds[1] = cubeIds[3]; 
    ptIds[2] = cubeIds[7]; ptIds[3] = cubeIds[5];
    polys->InsertNextCell(4,ptIds);

    ptIds[0] = cubeIds[4]; ptIds[1] = cubeIds[5]; 
    ptIds[2] = cubeIds[7]; ptIds[3] = cubeIds[6];
    polys->InsertNextCell(4,ptIds);

    ptIds[0] = cubeIds[2]; ptIds[1] = cubeIds[6]; 
    ptIds[2] = cubeIds[7]; ptIds[3] = cubeIds[3];
    polys->InsertNextCell(4,ptIds);
    }

  else if ( (level < repLevel || repLevel < 0) && OBBptr->Kids != NULL )
    {
    this->GeneratePolygons(OBBptr->Kids[0],level+1,repLevel,pts,polys);
    this->GeneratePolygons(OBBptr->Kids[1],level+1,repLevel,pts,polys);
    }
}

int vtkOBBTree::DisjointOBBNodes( vtkOBBNode *nodeA,
                                  vtkOBBNode *nodeB,
                                  vtkMatrix4x4 *XformBtoA )
  {
  vtkOBBNode nodeBxformed, *pB, *pA;
  float centerA[3], centerB[3], AtoB[3], in[4], out[4];
  double rangeAmin, rangeAmax, rangeBmin, rangeBmax, dotA, dotB,
         dotAB[3][3];
  double eps;
  int ii, jj, kk;
 
  eps = this->Tolerance;
  pA = nodeA;
  if ( XformBtoA != NULL )
    { // Here we assume that XformBtoA is an orthogonal matrix
    pB = &nodeBxformed;
    in[0] = nodeB->Corner[0]; in[1] = nodeB->Corner[1] ;
    in[2] = nodeB->Corner[2]; in[3] = 1.0;
    XformBtoA->MultiplyPoint( in, out );
    pB->Corner[0] = out[0]/out[3];
    pB->Corner[1] = out[1]/out[3];
    pB->Corner[2] = out[2]/out[3];
    // Clean this up when the bug in MultiplyVectors is fixed!
    for ( ii=0; ii<3; ii++ )
      {
      pB->Axes[0][ii] = nodeB->Corner[ii] + nodeB->Axes[0][ii];
      pB->Axes[1][ii] = nodeB->Corner[ii] + nodeB->Axes[1][ii];
      pB->Axes[2][ii] = nodeB->Corner[ii] + nodeB->Axes[2][ii];
      }
    for ( ii=0; ii<3; ii++ )
      {
      in[0] = pB->Axes[ii][0]; in[1] = pB->Axes[ii][1];
      in[2] = pB->Axes[ii][2]; in[3] = 1.0;
      XformBtoA->MultiplyPoint( in, out );
      pB->Axes[ii][0] = out[0]/out[3];
      pB->Axes[ii][1] = out[1]/out[3];
      pB->Axes[ii][2] = out[2]/out[3];
      }
    for ( ii=0; ii<3; ii++ )
      {
      pB->Axes[0][ii] = pB->Axes[0][ii] - pB->Corner[ii];
      pB->Axes[1][ii] = pB->Axes[1][ii] - pB->Corner[ii];
      pB->Axes[2][ii] = pB->Axes[2][ii] - pB->Corner[ii];
      }
    }
  else
    {
    pB = nodeB;
    }
  for ( ii=0; ii<3; ii++ )
    {
    centerA[ii] = pA->Corner[ii] +
                  0.5*(pA->Axes[0][ii] + pA->Axes[1][ii] + pA->Axes[2][ii]);
    centerB[ii] = pB->Corner[ii] +
                  0.5*(pB->Axes[0][ii] + pB->Axes[1][ii] + pB->Axes[2][ii]);
    AtoB[ii] = centerB[ii] - centerA[ii];
    }

  // Project maximal and minimal corners onto line between centers
  rangeAmin = rangeAmax = vtkMath::Dot( pA->Corner, AtoB );
  rangeBmin = rangeBmax = vtkMath::Dot( pB->Corner, AtoB );
  for ( ii=0; ii<3; ii++ )
    {
    // compute A range
    dotA = vtkMath::Dot( pA->Axes[ii], AtoB );
    if ( dotA > 0 )
      {
      rangeAmax += dotA;
      }
    else
      {
      rangeAmin += dotA;
      }
    
    // compute B range
    dotB = vtkMath::Dot( pB->Axes[ii], AtoB );
    if ( dotB > 0 )
      {
      rangeBmax += dotB;
      }
    else
      {
      rangeBmin += dotB;
      }
    }
  if ( (rangeAmax+eps < rangeBmin) || (rangeBmax+eps < rangeAmin) )
    {
    return( 1 ); // A and B are Disjoint by the 1st test.
    }

  // now check for a separation plane parallel to the faces of B
  for ( ii=0; ii<3; ii++ )
    { // plane is normal to pB->Axes[ii]
    // computing B range is easy...
    rangeBmin = rangeBmax = vtkMath::Dot( pB->Corner, pB->Axes[ii] );
    rangeBmax += vtkMath::Dot( pB->Axes[ii], pB->Axes[ii] );

    // compute A range...
    rangeAmin = rangeAmax = vtkMath::Dot( pA->Corner, pB->Axes[ii] );
    for ( jj=0; jj<3; jj++ )
      {
      // (note: we are saving all 9 dotproducts for future use)
      dotA = dotAB[ii][jj] = vtkMath::Dot( pB->Axes[ii], pA->Axes[jj] );
      if ( dotA > 0 )
        {
        rangeAmax += dotA;
        }
      else
        {
        rangeAmin += dotA;
        }
      }
    if ( (rangeAmax+eps < rangeBmin) || (rangeBmax+eps < rangeAmin) )
      {
      return( 2 ); // A and B are Disjoint by the 3rd test.
      }
    }

  // now check for a separation plane parallel to the faces of A
  for ( ii=0; ii<3; ii++ )
    { // plane is normal to pA->Axes[ii]
    // computing A range is easy...
    rangeAmin = rangeAmax = vtkMath::Dot( pA->Corner, pA->Axes[ii] );
    rangeAmax += vtkMath::Dot( pA->Axes[ii], pA->Axes[ii] );

    // compute B range...
    rangeBmin = rangeBmax = vtkMath::Dot( pB->Corner, pA->Axes[ii] );
    for ( jj=0; jj<3; jj++ )
      {
      // (note: we are using the 9 dotproducts computed earlier)
      dotB = dotAB[jj][ii];
      if ( dotB > 0 )
        {
        rangeBmax += dotB;
        }
      else
        {
        rangeBmin += dotB;
        }
      }
    if ( (rangeAmax+eps < rangeBmin) || (rangeBmax+eps < rangeAmin) )
      {
      return( 3 ); // A and B are Disjoint by the 2nd test.
      }
    }

  // Bad luck: now we must look for a separation plane parallel
  // to one edge from A and one edge from B.
  for ( ii=0; ii<3; ii++ )
    for ( jj=0; jj<3; jj++ )
      {
      // the plane is normal to pA->Axes[ii] X pB->Axes[jj]
      vtkMath::Cross( pA->Axes[ii], pB->Axes[jj], AtoB );
      rangeAmin = rangeAmax = vtkMath::Dot( pA->Corner, AtoB );
      rangeBmin = rangeBmax = vtkMath::Dot( pB->Corner, AtoB );
      for ( kk=0; kk<3; kk++ )
        {
        // compute A range
        dotA = vtkMath::Dot( pA->Axes[kk], AtoB );
        if ( dotA > 0 )
          {
          rangeAmax += dotA;
          }
        else
          {
          rangeAmin += dotA;
          }
    
        // compute B range
        dotB = vtkMath::Dot( pB->Axes[kk], AtoB );
        if ( dotB > 0 )
          {
          rangeBmax += dotB;
          }
        else
          {
          rangeBmin += dotB;
          }
        }
      if ( (rangeAmax+eps < rangeBmin) || (rangeBmax+eps < rangeAmin) )
        {
        return( 4 ); // A and B are Disjoint by the 4th test.
        }
      }
  // if we fall through to here, the OBB's overlap
  return( 0 );
  }

int vtkOBBTree::TriangleIntersectsNode( vtkOBBNode *nodeA,
                                        float p0[3], float p1[3], float p2[3],
                                        vtkMatrix4x4 *XformBtoA )
  {
  vtkOBBNode *pA;
  float p0Xformed[3], p1Xformed[3], p2Xformed[3];
  float *pB[3], in[4], out[4], v0[3], v1[3], AtoB[3], xprod[3];
  double rangeAmin, rangeAmax, rangeBmin, rangeBmax, dotA, dotB;
  double eps;
  int ii, jj, kk;
 
  eps = this->Tolerance;
  pA = nodeA;
  if ( XformBtoA != NULL )
    { // Here we assume that XformBtoA is an orthogonal matrix
    pB[0] = p0Xformed; pB[1] = p1Xformed; pB[2] = p2Xformed;
    for ( ii=0; ii<3; ii++ )
      {
      p0Xformed[ii] = p0[ii];
      p1Xformed[ii] = p1[ii];
      p2Xformed[ii] = p2[ii];
      }
    for ( ii=0; ii<3; ii++ )
      {
      in[0] = pB[ii][0]; in[1] = pB[ii][1] ; in[2] = pB[ii][2]; in[3] = 1.0;
      XformBtoA->MultiplyPoint( in, out );
      pB[ii][0] = out[0]/out[3];
      pB[ii][1] = out[1]/out[3];
      pB[ii][2] = out[2]/out[3];
      }
    }
  else
    {
    pB[0] = p0; pB[1] = p1; pB[2] = p2;
    }

  // now check for a separation plane parallel to the triangle
  for ( ii=0; ii<3; ii++ )
    { // plane is normal to the triangle
    v0[ii] = pB[1][ii] - pB[0][ii];
    v1[ii] = pB[2][ii] - pB[0][ii];
    }
  vtkMath::Cross( v0, v1, xprod );
  // computing B range is easy...
  rangeBmin = rangeBmax = vtkMath::Dot( pB[0], xprod );
  // compute A range...
  rangeAmin = rangeAmax = vtkMath::Dot( pA->Corner, xprod );
  for ( jj=0; jj<3; jj++ )
    {
    dotA = vtkMath::Dot( xprod, pA->Axes[jj] );
    if ( dotA > 0 )
      {
      rangeAmax += dotA;
      }
    else
      {
      rangeAmin += dotA;
      }
    }
  if ( (rangeAmax+eps < rangeBmin) || (rangeBmax+eps < rangeAmin) )
    {
    return( 0 ); // A and B are Disjoint by the 1st test.
    }
  
  // now check for a separation plane parallel to the faces of A
  for ( ii=0; ii<3; ii++ )
    { // plane is normal to pA->Axes[ii]
    // computing A range is easy...
    rangeAmin = rangeAmax = vtkMath::Dot( pA->Corner, pA->Axes[ii] );
    rangeAmax += vtkMath::Dot( pA->Axes[ii], pA->Axes[ii] );

    // compute B range...
    rangeBmin = rangeBmax = vtkMath::Dot( pB[0], pA->Axes[ii] );
    dotB = vtkMath::Dot( pB[1], pA->Axes[ii] );
    if ( dotB > rangeBmax )
      {
      rangeBmax = dotB;
      }
    else
      {
      rangeBmin = dotB;
      }
    
    dotB = vtkMath::Dot( pB[2], pA->Axes[ii] );
    if ( dotB > rangeBmax )
      {
      rangeBmax = dotB;
      }
    else if ( dotB < rangeBmin )
      {
      rangeBmin = dotB;
      }

    if ( (rangeAmax+eps < rangeBmin) || (rangeBmax+eps < rangeAmin) )
      {
      return( 0 ); // A and B are Disjoint by the 2nd test.
      }
    }

  // Bad luck: now we must look for a separation plane parallel
  // to one edge from A and one edge from B.
  for ( ii=0; ii<3; ii++ )
    {
    for ( jj=0; jj<3; jj++ )
      {
      // the plane is normal to pA->Axes[ii] X (pB[jj+1]-pB[jj])
      v0[0] = pB[(jj+1)%3][0] - pB[jj][0];
      v0[1] = pB[(jj+1)%3][1] - pB[jj][1];
      v0[2] = pB[(jj+1)%3][2] - pB[jj][2];
      vtkMath::Cross( pA->Axes[ii], v0, AtoB );
      rangeAmin = rangeAmax = vtkMath::Dot( pA->Corner, AtoB );
      rangeBmin = rangeBmax = vtkMath::Dot( pB[jj], AtoB );
      for ( kk=0; kk<3; kk++ )
        {
        // compute A range
        dotA = vtkMath::Dot( pA->Axes[kk], AtoB );
        if ( dotA > 0 )
          {
          rangeAmax += dotA;
          }
        else
          {
          rangeAmin += dotA;
          }
        }
      // compute B range
      dotB = vtkMath::Dot( pB[(jj+2)%3], AtoB );
      if ( dotB > rangeBmax )
        {
        rangeBmax = dotB;
        }
      else
        {
        rangeBmin = dotB;
        }

      if ( (rangeAmax+eps < rangeBmin) || (rangeBmax+eps < rangeAmin) )
        {
        return( 0 ); // A and B are Disjoint by the 3rd test.
        }
      }
    }
  
  // if we fall through to here, the OBB overlaps the triangle.
  return( 1 );
  }

int vtkOBBTree::LineIntersectsNode( vtkOBBNode *pA,
                                    float B0[3], float B1[3] )
  {
  float AtoB[3], Bvec[3];
  double rangeAmin, rangeAmax, rangeBmin, rangeBmax, dotA, dotB;
  int ii, jj;
 
  // check for a separation plane parallel to the faces of A
  for ( ii=0; ii<3; ii++ )
    { // plane is normal to pA->Axes[ii]
    // computing A range is easy...
    rangeAmin = rangeAmax = vtkMath::Dot( pA->Corner, pA->Axes[ii] );
    rangeAmax += vtkMath::Dot( pA->Axes[ii], pA->Axes[ii] );

    // compute B range...
    rangeBmin = rangeBmax = vtkMath::Dot( B0, pA->Axes[ii] );
    dotB = vtkMath::Dot( B1, pA->Axes[ii] );
    if ( dotB < rangeBmin )
      {
      rangeBmin = dotB;
      }
    else
      {
      rangeBmax = dotB;
      }
    
    if ( (rangeAmax < rangeBmin) || (rangeBmax < rangeAmin) )
      {
      return( 0 ); // A and B are Disjoint by the 1st test.
      }
    }

  // now check for a separation plane normal to the line segment B
  // plane is normal to B1-B0
  Bvec[0] = B1[0] - B0[0];
  Bvec[1] = B1[1] - B0[1];
  Bvec[2] = B1[2] - B0[2];
  // computing B range is easy...
  rangeBmin = vtkMath::Dot( B0, Bvec );
  rangeBmax = vtkMath::Dot( B1, Bvec );

  // compute A range...
  rangeAmin = rangeAmax = vtkMath::Dot( pA->Corner, Bvec );
  for ( jj=0; jj<3; jj++ )
    {
    dotA = vtkMath::Dot( pA->Axes[ii], Bvec );
    if ( dotA > 0 )
      {
      rangeAmax += dotA;
      }
    else
      {
      rangeAmin += dotA;
      }
    }
  if ( (rangeAmax < rangeBmin) || (rangeBmax < rangeAmin) )
    {
    return( 0 ); // A and B are Disjoint by the 2nd test.
    }

  // Bad luck: now we must look for a separation plane parallel
  // to one edge from A and line B.
  for ( ii=0; ii<3; ii++ )
    {
    // the plane is normal to pA->Axes[ii] X Bvec
    vtkMath::Cross( pA->Axes[ii], Bvec, AtoB );
    rangeAmin = rangeAmax = vtkMath::Dot( pA->Corner, AtoB );
    rangeBmin = rangeBmax = vtkMath::Dot( B0, AtoB );
    for ( jj=0; jj<3; jj++ )
      {
      // compute A range
      dotA = vtkMath::Dot( pA->Axes[jj], AtoB );
      if ( dotA > 0 )
        {
        rangeAmax += dotA;
        }
      else
        {
        rangeAmin += dotA;
        }
      }

    // compute B range
    dotB = vtkMath::Dot( Bvec, AtoB );
    if ( dotB > 0 )
      {
      rangeBmax += dotB;
      }
    else
      {
      rangeBmin += dotB;
      }
    }
    if ( (rangeAmax < rangeBmin) || (rangeBmax < rangeAmin) )
      {
      return( 0 ); // A and B are Disjoint by the 3rd test.
      }
    
  return( 1 ); // if we fall through to here, the line must intersect the OBB
  }

// Intersect this OBBTree with OBBTreeB (as transformed) and
// call processing function for each intersecting leaf node pair.
// If the processing function returns a negative integer, terminate.
int vtkOBBTree::IntersectWithOBBTree( vtkOBBTree *OBBTreeB,
                                      vtkMatrix4x4 *XformBtoA,
                                      int(*function)( vtkOBBNode *nodeA,
                                                      vtkOBBNode *nodeB,
                                                      vtkMatrix4x4 *Xform,
                                                      void *arg ),
                                      void *data_arg )
  {
  int maxdepth, mindepth, depth, returnValue = 0, count = 0,
      maxStackDepth;
  vtkOBBNode **OBBstackA, **OBBstackB, *nodeA, *nodeB;

  // Intersect OBBs and process intersecting leaf nodes.
  maxdepth = this->GetLevel();
  if ( (mindepth = OBBTreeB->GetLevel()) > maxdepth )
    {
    mindepth = maxdepth;
    maxdepth = OBBTreeB->GetLevel();
    }
  // Compute maximum theoretical recursion depth.
  maxStackDepth = 3*mindepth + 2*(maxdepth-mindepth) + 1;

  OBBstackA = new vtkOBBNode *[maxStackDepth];
  OBBstackB = new vtkOBBNode *[maxStackDepth];
  OBBstackA[0] = this->Tree;
  OBBstackB[0] = OBBTreeB->Tree;
  depth = 1;
  while( depth > 0 && returnValue > -1 )
    { // simulate recursion without overhead of real recursion.
    depth--;
    nodeA = OBBstackA[depth];
    nodeB = OBBstackB[depth];
    if ( !this->DisjointOBBNodes( nodeA, nodeB, XformBtoA ) )
      {
      if ( nodeA->Kids == NULL )
        {
        if ( nodeB->Kids == NULL )
          { // then this is a pair of intersecting leaf nodes to process
          returnValue = (*function)( nodeA, nodeB, XformBtoA, data_arg );
          if ( returnValue >= 0 )
            {
            count += returnValue;
            }
          else
            {
            count = returnValue;
            }
          }
        else
          { // A is a leaf, but B goes deeper.
          OBBstackA[depth] = nodeA;
          OBBstackB[depth] = nodeB->Kids[0];
          OBBstackA[depth+1] = nodeA;
          OBBstackB[depth+1] = nodeB->Kids[1];
          depth += 2;
          }
        }
      else
        {
        if ( nodeB->Kids == NULL )
          { // B is a leaf, but A goes deeper.
          OBBstackB[depth] = nodeB;
          OBBstackA[depth] = nodeA->Kids[0];
          OBBstackB[depth+1] = nodeB;
          OBBstackA[depth+1] = nodeA->Kids[1];
          depth += 2;
          }
        else
          { // neither A nor B are leaves. Go to the next level.
          OBBstackA[depth] = nodeA->Kids[0];
          OBBstackB[depth] = nodeB->Kids[0];
          OBBstackA[depth+1] = nodeA->Kids[1];
          OBBstackB[depth+1] = nodeB->Kids[0];
          OBBstackA[depth+2] = nodeA->Kids[0];
          OBBstackB[depth+2] = nodeB->Kids[1];
          OBBstackA[depth+3] = nodeA->Kids[1];
          OBBstackB[depth+3] = nodeB->Kids[1];
          depth += 4;
          }
        }
      }
    }
  // cleanup
  delete OBBstackA;
  delete OBBstackB;

  return( count );
  }
