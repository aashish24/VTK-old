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
#include <stdlib.h>
#include <math.h>
#include "vtkCellLocator.hh"
#include "vtkLocator.hh"
#include "vtkMath.hh"
#include "vtkIntArray.hh"
#include "vtkVoxel.hh"

#define OUTSIDE 0
#define INSIDE 1

typedef vtkIdList *vtkIdListPtr;

// Description:
// Construct with automatic computation of divisions, averaging
// 25 cells per octant.
vtkCellLocator::vtkCellLocator()
{
  this->DataSet = NULL;
  this->Level = 4;
  this->MaxLevel = 5;
  this->Automatic = 1;
  this->NumberOfCellsInOctant = 25;
  this->Tolerance = 0.01;
  this->Tree = NULL;
  this->H[0] = this->H[1] = this->H[2] = 1.0;
  this->NumberOfDivisions = 1;
}

vtkCellLocator::~vtkCellLocator()
{
  this->Initialize();
}

void vtkCellLocator::Initialize()
{
  // free up octants
  this->FreeSearchStructure();
}

void vtkCellLocator::FreeSearchStructure()
{
  vtkIdList *cellIds;
  int i;

  if ( this->Tree )
    {
    for (i=0; i<this->NumberOfOctants; i++)
      {
      cellIds = this->Tree[i];
      if (cellIds == (void *)INSIDE) cellIds = 0;
      if (cellIds) delete cellIds;
      }
    delete [] this->Tree;
    this->Tree = NULL;
    }
}

void vtkCellLocator::Update()
{
  if ((this->SubDivideTime < this->MTime)||
      (this->DataSet->GetMTime() < this->SubDivideTime))
    {
    this->SubDivide();
    }
}

// Description:
// Return list of octants that are intersected by line. The octants
// are ordered along the line and are represented by octant number. To obtain
// the cells in the octant, use the method GetOctantCells().
int vtkCellLocator::IntersectWithLine(float a0[3], float a1[3], float tol,
				      float& t, float x[3], float pcoords[3],
				      int &subId)
{
  static vtkVoxel voxel; // use to take advantage of Hitbbox() method
  float origin[3];
  float direction2[3];
  float direction3[3];
  float hitPosition[3];
  float result;
  float *bounds;
  float bounds2[6];
  int i, leafStart, prod, loop;
  vtkCell *cell;
  int bestCellId;
  int idx, cellId;
  float tMax, dist[3];
  float hits[3];
  int npos[3];
  int pos[3];
  int bestDir;
  float stopDist, currDist;
  
  // get the bounds
  bounds = this->DataSet->GetBounds();

  // convert the line into i,j,k coordinates
  tMax = 0;
  for (i = 0; i < 3; i++) 
    {
    origin[i]      = (a0[i] - bounds[2*i])/(bounds[2*i+1] - bounds[2*i]);
    direction2[i]  = (a1[i] - a0[i])/(bounds[2*i+1] - bounds[2*i]);
    bounds2[2*i]   = 0;
    bounds2[2*i+1] = 1.0;
    tMax += direction2[i]*direction2[i];
    }
  tMax = sqrt(tMax);
  stopDist = tMax*this->NumberOfDivisions;
  for (i = 0; i < 3; i++) 
    {
    direction3[i] = direction2[i]/tMax;
    }
  
  if (voxel.HitBBox(bounds2, origin, direction2, hitPosition, result))
    {
    // start walking through the octants
    prod = this->NumberOfDivisions*this->NumberOfDivisions;
    leafStart = this->NumberOfOctants - this->NumberOfDivisions*prod;
    bestCellId = -1;

    // set up curr and stop dist
    currDist = 0;
    for (i = 0; i < 3; i++)
      {
      currDist += (hitPosition[i] - origin[i])*(hitPosition[i] - origin[i]);
      }
    currDist = sqrt(currDist)*this->NumberOfDivisions;
      
    // add one offset due to the problems around zero
    for (loop = 0; loop <3; loop++)
      {
      hitPosition[loop] = hitPosition[loop]*this->NumberOfDivisions + 1.0;
      }
    pos[0] = (int)hitPosition[0];
    pos[1] = (int)hitPosition[1];
    pos[2] = (int)hitPosition[2];

    idx = leafStart + pos[0] - 1 + (pos[1] - 1)*this->NumberOfDivisions 
      + (pos[2] - 1)*prod;

    while ((bestCellId < 0) && (pos[0] > 0) && (pos[1] > 0) && (pos[2] > 0) &&
	   (pos[0] <= this->NumberOfDivisions) &&
	   (pos[1] <= this->NumberOfDivisions) &&
	   (pos[2] <= this->NumberOfDivisions) &&
	   (currDist < stopDist))
      {
      if (this->Tree[idx])
	{
	for (tMax = VTK_LARGE_FLOAT, cellId=0; 
	     cellId < this->Tree[idx]->GetNumberOfIds(); cellId++) 
	  {
	  cell = this->DataSet->GetCell(this->Tree[idx]->GetId(cellId));
	  
	  if (cell->IntersectWithLine(a0, a1, tol, t, x, pcoords, 
				      subId))
	    {
	    if (t < tMax)
	      {
	      for (loop = 0; loop < 3; loop++)
		{
		hits[loop] = (x[loop] - bounds[loop*2])
		  *this->NumberOfDivisions/
		  (bounds[loop*2+1] - bounds[loop*2]);
		}
	      if (
		  (((int)(hits[0] + 1.01) == pos[0])||
		   ((int)(hits[0] + 1.0) == pos[0])||
		   ((int)(hits[0] + 0.99) == pos[0])) && 
		  (((int)(hits[1] + 1.01) == pos[1])||
		   ((int)(hits[1] + 1.0) == pos[1])||
		   ((int)(hits[1] + 0.99) == pos[1])) && 
		  (((int)(hits[2] + 1.01) == pos[2])||
		   ((int)(hits[2] + 1.0) == pos[2])||
		   ((int)(hits[2] + 0.99) == pos[2])))
		{
		tMax = t;
		bestCellId = this->Tree[idx]->GetId(cellId);
		}
	      }
	    }
	  }
	}

      // move to the next octant
      tMax = 10;
      bestDir = 0;
      for (loop = 0; loop < 3; loop++)
        {
	if (direction3[loop] > 0)
	  {
	  npos[loop] = pos[loop] + 1;
	  dist[loop] = (1.0 - hitPosition[loop] + pos[loop])/direction3[loop];
	  if (dist[loop] == 0) dist[loop] = 1.0/direction3[loop];
	  if (dist[loop] < 0) dist[loop] = 0;
	  if (dist[loop] < tMax)
	    {
	    bestDir = loop;
	    tMax = dist[loop];
	    }
	  }
	if (direction3[loop] < 0)
	  {
	  npos[loop] = pos[loop] - 1;
	  dist[loop] = (pos[loop] - hitPosition[loop])/direction3[loop];
	  if (dist[loop] == 0) dist[loop] = -0.01/direction3[loop];
	  if (dist[loop] < 0) dist[loop] = 0;
	  if (dist[loop] < tMax)
	    {
	    bestDir = loop;
	    tMax = dist[loop];
	    }
	  }
	}
      // update our position
      for (loop = 0; loop < 3; loop++)
	{
	hitPosition[loop] += dist[bestDir]*direction3[loop];
	}
      currDist += dist[bestDir];
      // now make the move, find the smallest distance
      // only cross one boundry at a time
      pos[bestDir] = npos[bestDir];

      idx = leafStart + pos[0] - 1 + (pos[1]-1)*this->NumberOfDivisions + 
	(pos[2]-1)*prod;
      }
    }
  
  if (bestCellId >= 0)
    {
    this->DataSet->GetCell(bestCellId)->
      IntersectWithLine(a0, a1, tol, t, x, pcoords, subId);
    return 1;
    }
  
  // failed to find it recursing
  // loop through all cells to check
  //  for (cellId=0; cellId < this->DataSet->GetNumberOfCells(); cellId++) 
  // {
  // cell = this->DataSet->GetCell(cellId);
  //if (cell->IntersectWithLine(a0, a1, tol, t, x, pcoords, 
  //			subId))
  //  {
  //  vtkErrorMacro(<< " I did see a putty cat!");
  //  return 1;
  //  }
  //}
  
  return 0;
}

// Description:
// Get the cells in an octant.
vtkIdList* vtkCellLocator::GetOctantCells(int octantId)
{
  // is this a leaf ? should check
  //  if (octantId >= 
  //    (this->NumberOfOctants - this->NumberOfDivisions*
  //    this->NumberOfDivisions*this->NumberOfDivisions))
  // {
// return this->Tree[octantId];
//   }
  
// handle parents ?		  
return this->Tree[octantId];
}

// Description:
// Intersect against another vtkCellLocator returning cells that lie in 
// intersecting octants.
int vtkCellLocator::IntersectWithCellLocator(vtkCellLocator& locator, vtkIdList cells)
{
  return 0;
}

//
//  Method to form subdivision of space based on the cells provided and
//  subject to the constraints of levels and NumberOfCellsInOctant.
//  The result is directly addressable and of uniform subdivision.
//
void vtkCellLocator::SubDivide()
{
  vtkCell *cell;
  float *bounds, *cellBounds;
  int numCells;
  int ndivs, product;
  int i, j, k, cellId, ijkMin[3], ijkMax[3];
  int idx, parentOffset;
  vtkIdList *octant;
  int numCellsInOctant = this->NumberOfCellsInOctant;
  typedef vtkIdList *vtkIdListPtr;
  int prod, numOctants;

  if ( this->Tree != NULL && this->SubDivideTime > this->MTime ) return;

  vtkDebugMacro( << "Subdividing octree..." );

  if ( !this->DataSet || (numCells = this->DataSet->GetNumberOfCells()) < 1 )
    {
    vtkErrorMacro( << "No cells to subdivide");
    return;
    }
  //
  //  Make sure the appropriate data is available
  //
  if ( this->Tree ) this->FreeSearchStructure();
  //
  //  Size the root cell.  Initialize cell data structure, compute 
  //  level and divisions.
  //
  bounds = this->DataSet->GetBounds();
  for (i=0; i<6; i++) this->Bounds[i] = bounds[i];

  if ( this->Automatic ) 
    {
    this->Level = (int) (ceil(log((double)numCells/numCellsInOctant) / 
                              (log((double) 8.0))));
    this->Level =(this->Level > this->MaxLevel ? this->MaxLevel : this->Level);
    } 

  // compute number of octants and number of divisions
  for (ndivs=1,prod=1,numOctants=1,i=0; i<this->Level; i++) 
    {
    ndivs *= 2;
    prod *= 8;
    numOctants += prod;
    }
  this->NumberOfDivisions = ndivs;
  this->NumberOfOctants = numOctants;

  this->Tree = new vtkIdListPtr[numOctants];
  memset (this->Tree, (int)NULL, numOctants*sizeof(vtkIdListPtr));
  //
  //  Compute width of leaf octant in three directions
  //
  for (i=0; i<3; i++) this->H[i] = (bounds[2*i+1] - bounds[2*i]) / ndivs;
  //
  //  Insert each cell into the appropriate octant.  Make sure cell
  //  falls within octant.
  //
  parentOffset = numOctants - (ndivs * ndivs * ndivs);
  product = ndivs * ndivs;
  for (cellId=0; cellId<numCells; cellId++) 
    {
    cell = this->DataSet->GetCell(cellId);
    cellBounds = cell->GetBounds();

    // find min/max locations of bounding box
    for (i=0; i<3; i++)
      {
      ijkMin[i] = (int)(((cellBounds[2*i] - bounds[2*i])/ 
                         (bounds[2*i+1] - bounds[2*i])) * ndivs * 0.999);
      ijkMax[i] = (int)(((cellBounds[2*i+1] - bounds[2*i])*1.001/ 
                         (bounds[2*i+1] - bounds[2*i])) * ndivs);
      if (ijkMax[i] == ndivs)
	{
	ijkMax[i]--;
	}
      }
    
    // each octant inbetween min/max point may have cell in it
    for ( k = ijkMin[2]; k <= ijkMax[2]; k++ )
      {
      for ( j = ijkMin[1]; j <= ijkMax[1]; j++ )
        {
        for ( i = ijkMin[0]; i <= ijkMax[0]; i++ )
          {
          idx = parentOffset + i + j*ndivs + k*product;
          this->MarkParents((void*)INSIDE,idx);
          octant = this->Tree[idx];
          if ( ! octant )
            {
            octant = new vtkIdList(numCellsInOctant);
            this->Tree[idx] = octant;
            }
          octant->InsertNextId(cellId);
          }
        }
      }

    } //for all cells

  this->SubDivideTime.Modified();
}

void vtkCellLocator::MarkParents(void* a, int idx)
{
  int parentIdx = 0;
  int prod = 1;
  int offset;
  
  // will's code, well ken's code really
  if (idx <= 0) return;

  while ((parentIdx + prod*8) < idx)
    {
    prod *= 8;
    parentIdx += prod;
    }
  
  offset = idx - parentIdx - 1;
  parentIdx -= prod + 1;
  
  parentIdx = parentIdx + (offset/8);
  
  // if it already matches just return
  if (a == this->Tree[parentIdx]) return;
  
  this->Tree[parentIdx] = (vtkIdList *)a;
  this->MarkParents(a,parentIdx);
}

