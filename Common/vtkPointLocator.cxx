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
#include "vtkPointLocator.h"
#include "vtkMath.h"
#include "vtkIntArray.h"
#include "vtkPolyData.h"

class vtkNeighborPoints
{
public:
  vtkNeighborPoints(const int sz, const int ext=1000)
     {this->P = vtkIntArray::New(); this->P->Allocate(3*sz,3*ext);};
  ~vtkNeighborPoints(){this->P->Delete();}; 
  int GetNumberOfNeighbors() {return (this->P->GetMaxId()+1)/3;};
  void Reset() {this->P->Reset();};

  int *GetPoint(int i) {return this->P->GetPointer(3*i);};
  int InsertNextPoint(int *x);

protected:
  vtkIntArray *P;
};

inline int vtkNeighborPoints::InsertNextPoint(int *x) 
{
  int id = this->P->GetMaxId() + 3;
  this->P->InsertValue(id,x[2]);
  this->P->SetValue(id-2, x[0]);
  this->P->SetValue(id-1, x[1]);
  return id/3;
}

// Construct with automatic computation of divisions, averaging
// 25 points per bucket.
vtkPointLocator::vtkPointLocator()
{

  this->Buckets = new vtkNeighborPoints(26,50);
  this->Points = NULL;
  this->Divisions[0] = this->Divisions[1] = this->Divisions[2] = 50;
  this->NumberOfPointsPerBucket = 3;
  this->HashTable = NULL;
  this->NumberOfBuckets = 0;
  this->H[0] = this->H[1] = this->H[2] = 0.0;
  this->InsertionPointId = 0;
  this->InsertionTol2 = 0.0001;
}

vtkPointLocator::~vtkPointLocator()
{
  if ( this->Points )
    {
    this->Points->UnRegister(this);
    this->Points = NULL;
    }
  if ( this->Buckets)
    {
     delete this->Buckets;
    }
  this->FreeSearchStructure();
}

void vtkPointLocator::Initialize()
{
  if ( this->Points )
    {
    this->Points->UnRegister(this);
    this->Points = NULL;
    }
  this->FreeSearchStructure();
}

void vtkPointLocator::FreeSearchStructure()
{
  vtkIdList *ptIds;
  int i;

  if ( this->HashTable )
    {
    for (i=0; i<this->NumberOfBuckets; i++)
      {
      if ( (ptIds = this->HashTable[i]) )
	{
	ptIds->Delete();
	}
      }
    delete [] this->HashTable;
    this->HashTable = NULL;
    }
}

// Given a position x-y-z, return the id of the point closest to it.
int vtkPointLocator::FindClosestPoint(float x, float y, float z)
{
  float xyz[3];

  xyz[0] = x; xyz[1] = y; xyz[2] = z;
  return this->FindClosestPoint(xyz);
}

// Given a position x, return the id of the point closest to it.
int vtkPointLocator::FindClosestPoint(float x[3])
{
  int i, j;
  float minDist2, dist2;
  float pt[3];
  int closest, level;
  int ptId, cno;
  vtkIdList *ptIds;
  int ijk[3], *nei;

  this->BuildLocator(); // will subdivide if modified; otherwise returns
  //
  //  Make sure candidate point is in bounds.  If not, it is outside.
  //
  for (i=0; i<3; i++)
    {
    if ( x[i] < this->Bounds[2*i] || x[i] > this->Bounds[2*i+1] )
      {
      return -1;
      }
    }
  //
  //  Find bucket point is in.  
  //
  for (j=0; j<3; j++) 
    {
    ijk[j] = (int)(((x[j] - this->Bounds[2*j]) / 
        (this->Bounds[2*j+1] - this->Bounds[2*j])) * (this->Divisions[j]-1));
    }
  //
  //  Need to search this bucket for closest point.  If there are no
  //  points in this bucket, search 1st level neighbors, and so on,
  //  until closest point found.
  //
  for (closest=(-1),minDist2=VTK_LARGE_FLOAT,level=0; (closest == -1) && 
	 (level < this->Divisions[0] || level < this->Divisions[1] || 
	  level < this->Divisions[2]); level++) 
    {
    this->GetBucketNeighbors (ijk, this->Divisions, level);

    for (i=0; i<this->Buckets->GetNumberOfNeighbors(); i++) 
      {
      nei = this->Buckets->GetPoint(i);
      cno = nei[0] + nei[1]*this->Divisions[0] + 
            nei[2]*this->Divisions[0]*this->Divisions[1];

      if ( (ptIds = this->HashTable[cno]) != NULL )
        {
        for (j=0; j < ptIds->GetNumberOfIds(); j++) 
          {
          ptId = ptIds->GetId(j);
          this->DataSet->GetPoint(ptId, pt);
          if ( (dist2 = vtkMath::Distance2BetweenPoints(x,pt)) < minDist2 ) 
            {
            closest = ptId;
            minDist2 = dist2;
            }
          }
        }
      }
    }
  //
  // Because of the relative location of the points in the buckets, the
  // point found previously may not be the closest point.  Have to
  // search those bucket neighbors that might also contain point.
  //
  if ( dist2 > 0.0 )
    {
    this->GetOverlappingBuckets (x, ijk, sqrt(dist2),0);

    for (i=0; i<this->Buckets->GetNumberOfNeighbors(); i++) 
      {
      nei = this->Buckets->GetPoint(i);
      cno = nei[0] + nei[1]*this->Divisions[0] + 
            nei[2]*this->Divisions[0]*this->Divisions[1];

      if ( (ptIds = this->HashTable[cno]) != NULL )
        {
        for (j=0; j < ptIds->GetNumberOfIds(); j++) 
          {
          ptId = ptIds->GetId(j);
          this->DataSet->GetPoint(ptId, pt);
          if ( (dist2 = vtkMath::Distance2BetweenPoints(x,pt)) < minDist2 ) 
            {
            closest = ptId;
            minDist2 = dist2;
            }
          }//for each point
        }//if points in bucket
      }//for each overlapping bucket
    }//if not identical point

  return closest;
}

struct idsort
{
  int id;
  float dist;
};

static int idsortcompare(const void *arg1, const void *arg2)
{
  idsort *v1 = (idsort *)arg1;
  idsort *v2 = (idsort *)arg2;
  if (v1->dist < v2->dist)
    {
    return -1;
    }
  if (v1->dist > v2->dist)
    {
    return 1;
    }
  return 0;  
}

void vtkPointLocator::FindDistributedPoints(int N, float x,
					    float y, float z,
					    vtkIdList *result, int M)
{
  float p[3];
  p[0] = x;
  p[1] = y;
  p[2] = z;
  this->FindDistributedPoints(N,p,result, M);
}

static int GetOctent(float *x,float *pt)
{
  float tmp[3];
  int res = 0;
  
  tmp[0] = pt[0] - x[0];
  tmp[1] = pt[1] - x[1];
  tmp[2] = pt[2] - x[2];
  
  if (tmp[0] > 0.0)
    {
    res += 1;
    }
  if (tmp[1] > 0.0)
    {
    res += 2;
    }
  if (tmp[2] > 0.0)
    {
    res += 4;
    }

  return res;
}

static int GetMin(int *foo)
{
  int result = foo[0];
  int i;
  
  for (i = 1; i < 8; i++)
    {
    if (foo[i] < result)
      {
      result = foo[i];
      }
    }
  return result;
}

static float GetMax(float *foo)
{
  float result = foo[0];
  int i;
  
  for (i = 1; i < 8; i++)
    {
    if (foo[i] > result)
      {
      result = foo[i];
      }
    }
  return result;
}

void vtkPointLocator::FindDistributedPoints(int N, float x[3],
					    vtkIdList *result, int M)
{
  int i, j;
  float minDist2, dist2;
  float *pt;
  int level;
  int ptId, cno;
  vtkIdList *ptIds;
  int ijk[3], *nei;
  int oct;
  int pointsChecked = 0;
  
  // clear out the result
  result->Reset();
    
  this->BuildLocator(); // will subdivide if modified; otherwise returns
  //
  //  Make sure candidate point is in bounds.  If not, it is outside.
  //
  for (i=0; i<3; i++)
    {
    if ( x[i] < this->Bounds[2*i] || x[i] > this->Bounds[2*i+1] )
      {
      return;
      }
    }
  //
  //  Find bucket point is in.  
  //
  for (j=0; j<3; j++) 
    {
    ijk[j] = (int)(((x[j] - this->Bounds[2*j]) / 
        (this->Bounds[2*j+1] - this->Bounds[2*j])) * (this->Divisions[j]-1));
    }

  // there are two steps, first a simple expanding wave of buckets until
  // we have enough points. Then a refinement to make sure we have the
  // N closest points.
  level = 0;
  float maxDistance[8] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  int currentCount[8] = {0,0,0,0,0,0,0,0};
  int minCurrentCount = 0;
  
  idsort *res[8];
  for (i = 0; i < 8; i++) 
    {
    res[i] = new idsort [N];
    }
  
  this->GetBucketNeighbors (ijk, this->Divisions, level);
  while (this->Buckets->GetNumberOfNeighbors() && 
	 minCurrentCount < N &&
	 pointsChecked < M)
    {
    for (i=0; i<this->Buckets->GetNumberOfNeighbors(); i++) 
      {
      nei = this->Buckets->GetPoint(i);
      cno = nei[0] + nei[1]*this->Divisions[0] + 
            nei[2]*this->Divisions[0]*this->Divisions[1];

      if ( (ptIds = this->HashTable[cno]) != NULL )
        {
        for (j=0; j < ptIds->GetNumberOfIds(); j++) 
          {
	  pointsChecked++;
          ptId = ptIds->GetId(j);
          pt = this->DataSet->GetPoint(ptId);
          dist2 = vtkMath::Distance2BetweenPoints(x,pt);
	  oct = GetOctent(x,pt);
	  if (currentCount[oct] < N)
	    {
	    res[oct][currentCount[oct]].dist = dist2;
	    res[oct][currentCount[oct]].id = ptId;
	    if (dist2 > maxDistance[oct])
	      {
	      maxDistance[oct] = dist2;
	      }
	    currentCount[oct] = currentCount[oct] + 1;
	    // compute new minCurrentCount
	    minCurrentCount = GetMin(currentCount);	    
	    if (currentCount[oct] == N)
	      {
	      qsort(res[oct], currentCount[oct], sizeof(idsort),idsortcompare);
	      }
	    }
	  else if (dist2 < maxDistance[oct])
	    {
	    res[oct][N-1].dist = dist2;
	    res[oct][N-1].id = ptId;
	    qsort(res[oct], N, sizeof(idsort), idsortcompare);
	    maxDistance[oct] = res[oct][N-1].dist;
	    }
          }
        }
      }
    level++;
    this->GetBucketNeighbors (ijk, this->Divisions, level);
    }

  // do a sort
  for (i = 0; i < 8; i++)
    {
    qsort(res[i], currentCount[i], sizeof(idsort), idsortcompare);
    }
  
  // Now do the refinement
  this->GetOverlappingBuckets (x, ijk, sqrt(GetMax(maxDistance)),level-1);
  
  for (i=0; pointsChecked < M && i<this->Buckets->GetNumberOfNeighbors(); i++) 
    {
    nei = this->Buckets->GetPoint(i);
    cno = nei[0] + nei[1]*this->Divisions[0] + 
      nei[2]*this->Divisions[0]*this->Divisions[1];
    
    if ( (ptIds = this->HashTable[cno]) != NULL )
      {
      for (j=0; j < ptIds->GetNumberOfIds(); j++) 
	{
	pointsChecked++;
	ptId = ptIds->GetId(j);
	pt = this->DataSet->GetPoint(ptId);
	dist2 = vtkMath::Distance2BetweenPoints(x,pt);
	oct = GetOctent(x,pt);
	if (dist2 < maxDistance[oct])
	  {
	  res[oct][N-1].dist = dist2;
	  res[oct][N-1].id = ptId;
	  qsort(res[oct], N, sizeof(idsort), idsortcompare);
	  maxDistance[oct] = res[oct][N-1].dist;
	  }
	}
      }
    }
    
  // Fill in the IdList
  for (j = 0; j < 8; j++)
    {
    for (i = 0; i < currentCount[j]; i++)
      {
      result->InsertNextId(res[j][i].id);
      }
    delete [] res[j];
    }
}

void vtkPointLocator::FindClosestNPoints(int N, float x,
					 float y, float z,
					 vtkIdList *result)
{
  float p[3];
  p[0] = x;
  p[1] = y;
  p[2] = z;
  this->FindClosestNPoints(N,p,result);
}

void vtkPointLocator::FindClosestNPoints(int N, float x[3],vtkIdList *result)
{
  int i, j;
  float minDist2, dist2;
  float *pt;
  int level;
  int ptId, cno;
  vtkIdList *ptIds;
  int ijk[3], *nei;
  
  // clear out the result
  result->Reset();
    
  this->BuildLocator(); // will subdivide if modified; otherwise returns
  //
  //  Make sure candidate point is in bounds.  If not, it is outside.
  //
  for (i=0; i<3; i++)
    {
    if ( x[i] < this->Bounds[2*i] || x[i] > this->Bounds[2*i+1] )
      {
      return;
      }
    }
  //
  //  Find bucket point is in.  
  //
  for (j=0; j<3; j++) 
    {
    ijk[j] = (int)(((x[j] - this->Bounds[2*j]) / 
        (this->Bounds[2*j+1] - this->Bounds[2*j])) * (this->Divisions[j]-1));
    }

  // there are two steps, first a simple expanding wave of buckets until
  // we have enough points. Then a refinement to make sure we have the
  // N closest points.
  level = 0;
  float maxDistance = 0.0;
  int currentCount = 0;
  idsort *res = new idsort [N];
  
  this->GetBucketNeighbors (ijk, this->Divisions, level);
  while (this->Buckets->GetNumberOfNeighbors() && currentCount < N)
    {
    for (i=0; i<this->Buckets->GetNumberOfNeighbors(); i++) 
      {
      nei = this->Buckets->GetPoint(i);
      cno = nei[0] + nei[1]*this->Divisions[0] + 
            nei[2]*this->Divisions[0]*this->Divisions[1];

      if ( (ptIds = this->HashTable[cno]) != NULL )
        {
        for (j=0; j < ptIds->GetNumberOfIds(); j++) 
          {
          ptId = ptIds->GetId(j);
          pt = this->DataSet->GetPoint(ptId);
          dist2 = vtkMath::Distance2BetweenPoints(x,pt);
	  if (currentCount < N)
	    {
	    res[currentCount].dist = dist2;
	    res[currentCount].id = ptId;
	    if (dist2 > maxDistance)
	      {
	      maxDistance = dist2;
	      }
	    currentCount++;
	    if (currentCount == N)
	      {
	      qsort(res, currentCount, sizeof(idsort), idsortcompare);
	      }
	    }
	  else if (dist2 < maxDistance)
	    {
	    res[N-1].dist = dist2;
	    res[N-1].id = ptId;
	    qsort(res, N, sizeof(idsort), idsortcompare);
	    maxDistance = res[N-1].dist;
	    }
          }
        }
      }
    level++;
    this->GetBucketNeighbors (ijk, this->Divisions, level);
    }

  // do a sort
  qsort(res, currentCount, sizeof(idsort), idsortcompare);

  // Now do the refinement
  this->GetOverlappingBuckets (x, ijk, sqrt(maxDistance),level-1);
  
  for (i=0; i<this->Buckets->GetNumberOfNeighbors(); i++) 
    {
    nei = this->Buckets->GetPoint(i);
    cno = nei[0] + nei[1]*this->Divisions[0] + 
      nei[2]*this->Divisions[0]*this->Divisions[1];
    
    if ( (ptIds = this->HashTable[cno]) != NULL )
      {
      for (j=0; j < ptIds->GetNumberOfIds(); j++) 
	{
	ptId = ptIds->GetId(j);
	pt = this->DataSet->GetPoint(ptId);
	dist2 = vtkMath::Distance2BetweenPoints(x,pt);
	if (dist2 < maxDistance)
	  {
	  res[N-1].dist = dist2;
	  res[N-1].id = ptId;
	  qsort(res, N, sizeof(idsort), idsortcompare);
	  maxDistance = res[N-1].dist;
	  }
	}
      }
    }
    
  // Fill in the IdList
  result->SetNumberOfIds(currentCount);
  for (i = 0; i < currentCount; i++)
    {
    result->SetId(i,res[i].id);
    }

  delete [] res;
}

void vtkPointLocator::FindPointsWithinRadius(float R, float x,
					     float y, float z,
					     vtkIdList *result)
{
  float p[3];
  p[0] = x;
  p[1] = y;
  p[2] = z;
  this->FindPointsWithinRadius(R,p,result);
}

void vtkPointLocator::FindPointsWithinRadius(float R, float x[3],
					     vtkIdList *result)
{
  int i, j;
  float minDist2, dist2;
  float *pt;
  int level;
  int ptId, cno;
  vtkIdList *ptIds;
  int ijk[3], *nei;
  float R2 = R*R;
  
  this->BuildLocator(); // will subdivide if modified; otherwise returns
  //
  //  Make sure candidate point is in bounds.  If not, it is outside.
  //
  for (i=0; i<3; i++)
    {
    if ( x[i] < this->Bounds[2*i] || x[i] > this->Bounds[2*i+1] )
      {
      return;
      }
    }
  //
  //  Find bucket point is in.  
  //
  for (j=0; j<3; j++) 
    {
    ijk[j] = (int)(((x[j] - this->Bounds[2*j]) / 
        (this->Bounds[2*j+1] - this->Bounds[2*j])) * (this->Divisions[j]-1));
    }

  // get all buckets within a distance
  this->GetOverlappingBuckets (x, ijk, R, 0);
  // add the original bucket
  this->Buckets->InsertNextPoint(ijk);

  // clear out the result
  result->Reset();
  
  for (i=0; i<this->Buckets->GetNumberOfNeighbors(); i++) 
    {
    nei = this->Buckets->GetPoint(i);
    cno = nei[0] + nei[1]*this->Divisions[0] + 
      nei[2]*this->Divisions[0]*this->Divisions[1];
    
    if ( (ptIds = this->HashTable[cno]) != NULL )
      {
      for (j=0; j < ptIds->GetNumberOfIds(); j++) 
	{
	ptId = ptIds->GetId(j);
	pt = this->DataSet->GetPoint(ptId);
	dist2 = vtkMath::Distance2BetweenPoints(x,pt);
	if (dist2 <= R2)
	  {
	  result->InsertNextId(ptId);
	  }
	}
      }
    }

}

//
//  Method to form subdivision of space based on the points provided and
//  subject to the constraints of levels and NumberOfPointsPerBucket.
//  The result is directly addressable and of uniform subdivision.
//
void vtkPointLocator::BuildLocator()
{
  float *bounds;
  int numBuckets;
  float level;
  int ndivs[3], product;
  int i, j, ijk[3];
  int idx;
  vtkIdList *bucket;
  int numPts;
  float *x;
  typedef vtkIdList *vtkIdListPtr;

  if ( (this->HashTable != NULL) && (this->BuildTime > this->MTime)
       && (this->BuildTime > this->DataSet->GetMTime()) )
    {
    return;
    }

  vtkDebugMacro( << "Hashing points..." );
  this->Level = 1; //only single lowest level

  if ( !this->DataSet || (numPts = this->DataSet->GetNumberOfPoints()) < 1 )
    {
    vtkErrorMacro( << "No points to subdivide");
    return;
    }
  //
  //  Make sure the appropriate data is available
  //
  if ( this->HashTable )
    {
    this->FreeSearchStructure();
    }
  //
  //  Size the root bucket.  Initialize bucket data structure, compute 
  //  level and divisions.
  //
  bounds = this->DataSet->GetBounds();
  for (i=0; i<3; i++)
    {
    this->Bounds[2*i] = bounds[2*i];
    this->Bounds[2*i+1] = bounds[2*i+1];
    if ( this->Bounds[2*i+1] <= this->Bounds[2*i] ) //prevent zero width
      {
      this->Bounds[2*i+1] = this->Bounds[2*i] + 1.0;
      }
    }

  if ( this->Automatic ) 
    {
    level = (float) numPts / this->NumberOfPointsPerBucket;
    level = ceil( pow((double)level,(double)0.33333333) );
    for (i=0; i<3; i++)
      {
      ndivs[i] = (int) level;
      }
    } 
  else 
    {
    for (i=0; i<3; i++)
      {
      ndivs[i] = (int) this->Divisions[i];
      }
    }

  for (i=0; i<3; i++) 
    {
    ndivs[i] = (ndivs[i] > 0 ? ndivs[i] : 1);
    this->Divisions[i] = ndivs[i];
    }

  this->NumberOfBuckets = numBuckets = ndivs[0]*ndivs[1]*ndivs[2];
  this->HashTable = new vtkIdListPtr[numBuckets];
  memset (this->HashTable, (int)NULL, numBuckets*sizeof(vtkIdListPtr));
  //
  //  Compute width of bucket in three directions
  //
  for (i=0; i<3; i++) 
    {
    this->H[i] = (this->Bounds[2*i+1] - this->Bounds[2*i]) / ndivs[i] ;
    }
  //
  //  Insert each point into the appropriate bucket.  Make sure point
  //  falls within bucket.
  //
  product = ndivs[0]*ndivs[1];
  for (i=0; i<numPts; i++) 
    {
    x = this->DataSet->GetPoint(i);
    for (j=0; j<3; j++) 
      {
      ijk[j] = (int) ((float) ((x[j] - this->Bounds[2*j]) / 
                        (this->Bounds[2*j+1] - this->Bounds[2*j])) * (ndivs[j]- 1));
      }

    idx = ijk[0] + ijk[1]*ndivs[0] + ijk[2]*product;
    bucket = this->HashTable[idx];
    if ( ! bucket )
      {
      bucket = vtkIdList::New();
      bucket->Allocate(this->NumberOfPointsPerBucket/2,
		       this->NumberOfPointsPerBucket/3);
      this->HashTable[idx] = bucket;
      }
    bucket->InsertNextId(i);
    }

  this->BuildTime.Modified();
}


//
//  Internal function to get bucket neighbors at specified level
//
void vtkPointLocator::GetBucketNeighbors(int ijk[3], int ndivs[3], int level)
{
  int i, j, k, min, max, minLevel[3], maxLevel[3];
  int nei[3];
  //
  //  Initialize
  //
  this->Buckets->Reset();
  //
  //  If at this bucket, just place into list
  //
  if ( level == 0 ) 
    {
    this->Buckets->InsertNextPoint(ijk);
    return;
    }
  //
  //  Create permutations of the ijk indices that are at the level
  //  required. If these are legal buckets, add to list for searching.
  //
  for ( i=0; i<3; i++ ) 
    {
    min = ijk[i] - level;
    max = ijk[i] + level;
    minLevel[i] = ( min > 0 ? min : 0);
    maxLevel[i] = ( max < (ndivs[i]-1) ? max : (ndivs[i]-1));
    }

  for ( i= minLevel[0]; i <= maxLevel[0]; i++ ) 
    {
    for ( j= minLevel[1]; j <= maxLevel[1]; j++ ) 
      {
      for ( k= minLevel[2]; k <= maxLevel[2]; k++ ) 
	{
        if (i == (ijk[0] + level) || i == (ijk[0] - level) ||
	    j == (ijk[1] + level) || j == (ijk[1] - level) ||
	    k == (ijk[2] + level) || k == (ijk[2] - level) ) 
          {
          nei[0]=i; nei[1]=j; nei[2]=k;
          this->Buckets->InsertNextPoint(nei);
          }
        }
      }
    }

  return;
}

//
// Internal method to find those buckets that are within distance specified
// only those buckets outside of level radiuses of ijk are returned
void vtkPointLocator::GetOverlappingBuckets(float x[3], int ijk[3], 
					    float dist, int level)
{
  int i, j, k, nei[3], minLevel[3], maxLevel[3];

  // Initialize
  this->Buckets->Reset();

  // Determine the range of indices in each direction
  for (i=0; i < 3; i++)
    {
    minLevel[i] = (int) ((float) (((x[i]-dist) - this->Bounds[2*i]) / 
        (this->Bounds[2*i+1] - this->Bounds[2*i])) * (this->Divisions[i] - 1));
    maxLevel[i] = (int) ((float) (((x[i]+dist) - this->Bounds[2*i]) / 
        (this->Bounds[2*i+1] - this->Bounds[2*i])) * (this->Divisions[i] - 1));

    if ( minLevel[i] < 0 )
      {
      minLevel[i] = 0;
      }
    if ( maxLevel[i] >= this->Divisions[i] )
      {
      maxLevel[i] = this->Divisions[i] - 1;
      }
    }

  for ( i= minLevel[0]; i <= maxLevel[0]; i++ ) 
    {
    for ( j= minLevel[1]; j <= maxLevel[1]; j++ ) 
      {
      for ( k= minLevel[2]; k <= maxLevel[2]; k++ ) 
        {
        if ( i < (ijk[0]-level) || i > (ijk[0]+level) ||
	     j < (ijk[1]-level) || j > (ijk[1]+level) ||
	     k < (ijk[2]-level) || k > (ijk[2]+level))
          {
          nei[0]=i; nei[1]=j; nei[2]=k;
          this->Buckets->InsertNextPoint(nei);
          }
        }
      }
    }
}


// specifically for point insertion
float InsertionLevel;

// Initialize the point insertion process. The newPts is an object representing
// point coordinates into which incremental insertion methods place their 
// data. Bounds are the box that the points lie in.
int vtkPointLocator::InitPointInsertion(vtkPoints *newPts, float bounds[6])
{
  return this->InitPointInsertion(newPts,bounds,0);
}

// Initialize the point insertion process. The newPts is an object representing
// point coordinates into which incremental insertion methods place their 
// data. Bounds are the box that the points lie in.
int vtkPointLocator::InitPointInsertion(vtkPoints *newPts, float bounds[6],
					int estNumPts)
{
  int i;
  int maxDivs;
  typedef vtkIdList *vtkIdListPtr;
  float hmin;
  int ndivs[3];
  float level;

  this->InsertionPointId = 0;
  if ( this->HashTable )
    {
    this->FreeSearchStructure();
    }
  if ( newPts == NULL )
    {
    vtkErrorMacro(<<"Must define points for point insertion");
    return 0;
    }
  if (this->Points != NULL)
    {
    this->Points->UnRegister(this);
    }
  this->Points = newPts;
  this->Points->Register(this);

  for (i=0; i<3; i++)
    {
    this->Bounds[2*i] = bounds[2*i];
    this->Bounds[2*i+1] = bounds[2*i+1];
    if ( this->Bounds[2*i+1] <= this->Bounds[2*i] )
      {
      this->Bounds[2*i+1] = this->Bounds[2*i] + 1.0;
      }
    }

  if ( this->Automatic && (estNumPts > 0) )
    {
    level = (float) estNumPts / this->NumberOfPointsPerBucket;
    level = ceil( pow((double)level,(double)0.33333333) );
    for (i=0; i<3; i++)
      {
      ndivs[i] = (int) level;
      }
    } 
  else 
    {
    for (i=0; i<3; i++)
      {
      ndivs[i] = (int) this->Divisions[i];
      }
    }

  for (i=0; i<3; i++) 
    {
    ndivs[i] = (ndivs[i] > 0 ? ndivs[i] : 1);
    this->Divisions[i] = ndivs[i];
    }

  this->NumberOfBuckets = ndivs[0]*ndivs[1]*ndivs[2];
  this->HashTable = new vtkIdListPtr[this->NumberOfBuckets];
  memset (this->HashTable, (int)NULL, this->NumberOfBuckets*
	  sizeof(vtkIdListPtr));
  //
  //  Compute width of bucket in three directions
  //
  for (i=0; i<3; i++) 
    {
    this->H[i] = (this->Bounds[2*i+1] - this->Bounds[2*i]) / ndivs[i] ;
    }

  this->InsertionTol2 = this->Tolerance * this->Tolerance;

  for (maxDivs=0, hmin=VTK_LARGE_FLOAT, i=0; i<3; i++) 
    {
    hmin = (this->H[i] < hmin ? this->H[i] : hmin);
    maxDivs = (maxDivs > this->Divisions[i] ? maxDivs : this->Divisions[i]);
    }
  InsertionLevel = ceil ((double) this->Tolerance / hmin);
  InsertionLevel = (InsertionLevel > maxDivs ? maxDivs : InsertionLevel);
  
  return 1;
}


// Incrementally insert a point into search structure. The method returns
// the insertion location (i.e., point id). You should use the method 
// IsInsertedPoint() to see whether this point has already been
// inserted (that is, if you desire to prevent dulicate points).
// Before using this method you must make sure that newPts have been
// supplied, the bounds has been set properly, and that divs are 
// properly set. (See InitPointInsertion().)
int vtkPointLocator::InsertNextPoint(float x[3])
{
  int i, ijk[3];
  int idx;
  vtkIdList *bucket;
  //
  //  Locate bucket that point is in.
  //
  for (i=0; i<3; i++)
    {
    ijk[i] = (int) ((float) ((x[i] - this->Bounds[2*i]) / 
        (this->Bounds[2*i+1] - this->Bounds[2*i])) * (this->Divisions[i] - 1));
    }

  idx = ijk[0] + ijk[1]*this->Divisions[0] + 
        ijk[2]*this->Divisions[0]*this->Divisions[1];

  if ( ! (bucket = this->HashTable[idx]) )
    {
    bucket = vtkIdList::New();
    bucket->Allocate(this->NumberOfPointsPerBucket/2,
	             this->NumberOfPointsPerBucket/3);
    this->HashTable[idx] = bucket;
    }

  bucket->InsertNextId(this->InsertionPointId);
  this->Points->InsertPoint(this->InsertionPointId,x);
  return this->InsertionPointId++;
}

// Incrementally insert a point into search structure with a particular
// index value. You should use the method IsInsertedPoint() to see whether 
// this point has already been inserted (that is, if you desire to prevent
// dulicate points). Before using this method you must make sure that 
// newPts have been supplied, the bounds has been set properly, and that 
// divs are properly set. (See InitPointInsertion().)
void vtkPointLocator::InsertPoint(int ptId, float x[3])
{
  int i, ijk[3];
  int idx;
  vtkIdList *bucket;
  //
  //  Locate bucket that point is in.
  //
  for (i=0; i<3; i++)
    {
    ijk[i] = (int) ((float) ((x[i] - this->Bounds[2*i]) / 
        (this->Bounds[2*i+1] - this->Bounds[2*i])) * (this->Divisions[i] - 1));
    }

  idx = ijk[0] + ijk[1]*this->Divisions[0] + 
        ijk[2]*this->Divisions[0]*this->Divisions[1];

  if ( ! (bucket = this->HashTable[idx]) )
    {
    bucket = vtkIdList::New();
    bucket->Allocate(this->NumberOfPointsPerBucket/2,
                     this->NumberOfPointsPerBucket/3);
    this->HashTable[idx] = bucket;
    }

  bucket->InsertNextId(ptId);
  this->Points->InsertPoint(ptId,x);
}

// Determine whether point given by x[3] has been inserted into points list.
// Return id of previously inserted point if this is true, otherwise return
// -1.
int vtkPointLocator::IsInsertedPoint(float x[3])
{
  int i, j, ijk[3];
  int idx;
  vtkIdList *bucket;
  //
  //  Locate bucket that point is in.
  //
  for (i=0; i<3; i++)
    {
    ijk[i] = (int) ((float) ((x[i] - this->Bounds[2*i]) / 
        (this->Bounds[2*i+1] - this->Bounds[2*i])) * (this->Divisions[i] - 1));
    }

  idx = ijk[0] + ijk[1]*this->Divisions[0] + 
        ijk[2]*this->Divisions[0]*this->Divisions[1];

  bucket = this->HashTable[idx];
  if ( ! bucket )
    {
    return -1;
    }
  else // see whether we've got duplicate point
    {
    //
    // Check the list of points in that bucket for merging.  Also need to 
    // search all neighboring buckets within the tolerance.  The number 
    // and level of neighbors to search depends upon the tolerance and 
    // the bucket width.
    //
    int *nei, lvtk, cno, ptId;
    vtkIdList *ptIds;
    float *pt;

    for (lvtk=0; lvtk <= InsertionLevel; lvtk++)
      {
      this->GetBucketNeighbors (ijk, this->Divisions, lvtk);

      for ( i=0; i < this->Buckets->GetNumberOfNeighbors(); i++ ) 
        {
        nei = this->Buckets->GetPoint(i);
        cno = nei[0] + nei[1]*this->Divisions[0] + 
              nei[2]*this->Divisions[0]*this->Divisions[1];

        if ( (ptIds = this->HashTable[cno]) != NULL )
          {
          for (j=0; j < ptIds->GetNumberOfIds(); j++) 
            {
            ptId = ptIds->GetId(j);
            pt = this->Points->GetPoint(ptId);

            if ( vtkMath::Distance2BetweenPoints(x,pt) <= this->InsertionTol2 )
              {
              return ptId;
              }
            }
          } //if points in bucket
        } //for each neighbor
      } //for neighbors at this level
    } // else check duplicate point

  return -1;
}


int vtkPointLocator::InsertUniquePoint(float x[3], int &id)
{
  int ptId;

  ptId = this->IsInsertedPoint(x);
  
  if (ptId > -1)
    {
    id = ptId;
    return 0;
    }
  else
    {
    id = this->InsertNextPoint(x);
    return 1;
    }
}


// Given a position x, return the id of the point closest to it. This method
// is used when performing incremental point insertion.
int vtkPointLocator::FindClosestInsertedPoint(float x[3])
{
  int i, j;
  float minDist2, dist2;
  float *pt;
  int closest, level;
  int ptId, cno;
  vtkIdList *ptIds;
  int ijk[3], *nei;
  int MULTIPLES;
  float diff;
  //
  //  Make sure candidate point is in bounds.  If not, it is outside.
  //
  for (i=0; i<3; i++)
    {
    if ( x[i] < this->Bounds[2*i] || x[i] > this->Bounds[2*i+1] )
      {
      return -1;
      }
    }
  //
  //  Find bucket point is in.  
  //
  for (j=0; j<3; j++) 
    {
    ijk[j] = (int)(((x[j] - this->Bounds[2*j]) / 
        (this->Bounds[2*j+1] - this->Bounds[2*j])) * (this->Divisions[j]-1));
    }
  //
  //  Need to search this bucket for closest point.  If there are no
  //  points in this bucket, search 1st level neighbors, and so on,
  //  until closest point found.
  //
  for (closest=0,minDist2=VTK_LARGE_FLOAT,level=0; (closest == 0) && 
  (level < this->Divisions[0] || level < this->Divisions[1] || 
  level < this->Divisions[2]); level++) 
    {
    this->GetBucketNeighbors (ijk, this->Divisions, level);

    for (i=0; i<this->Buckets->GetNumberOfNeighbors(); i++) 
      {
      nei = this->Buckets->GetPoint(i);
      cno = nei[0] + nei[1]*this->Divisions[0] + 
            nei[2]*this->Divisions[0]*this->Divisions[1];

      if ( (ptIds = this->HashTable[cno]) != NULL )
        {
        for (j=0; j < ptIds->GetNumberOfIds(); j++) 
          {
          ptId = ptIds->GetId(j);
          pt = this->Points->GetPoint(ptId);
          if ( (dist2 = vtkMath::Distance2BetweenPoints(x,pt)) < minDist2 ) 
            {
            closest = ptId;
            minDist2 = dist2;
            }
          }
        }
      }
    }
  //
  //  Because of the relative location of the points in the spatial_hash, this
  //  may not be the closest point.  Have to search those bucket
  //  neighbors (one level further out) that might also contain point.
  //
  this->GetBucketNeighbors (ijk, this->Divisions, level);
  //
  //  Don't want to search all the neighbors, only those that could
  //  possibly have points closer than the current closest.
  //
  for (i=0; i<this->Buckets->GetNumberOfNeighbors(); i++) 
    {
    nei = this->Buckets->GetPoint(i);

    for (dist2=0,j=0; j<3; j++) 
      {
      if ( ijk[j] != nei[j] ) 
        {
        MULTIPLES = (ijk[j]>nei[j] ? (nei[j]+1) : nei[j]);
        diff = (this->Bounds[2*j] + MULTIPLES * this->H[j]) - x[j];
        dist2 += diff*diff;
        }
      }

      if ( dist2 < minDist2 ) 
        {
        cno = nei[0] + nei[1]*this->Divisions[0] + nei[2]*this->Divisions[0]*this->Divisions[1];

        if ( (ptIds = this->HashTable[cno]) )
          {
          for (j=0; j < ptIds->GetNumberOfIds(); j++) 
            {
            ptId = ptIds->GetId(j);
            pt = this->Points->GetPoint(ptId);
            if ( (dist2 = vtkMath::Distance2BetweenPoints(x,pt)) < minDist2 ) 
              {
              closest = ptId;
              minDist2 = dist2;
              }
            }
          }
        }
      }

    return closest;
}


// Build polygonal representation of locator. Create faces that separate
// inside/outside buckets, or separate inside/boundary of locator.
void vtkPointLocator::GenerateRepresentation(int vtkNotUsed(level), vtkPolyData *pd)
{
  vtkPoints *pts;
  vtkCellArray *polys;
  int ii, i, j, k, idx, offset[3], minusOffset[3], inside, sliceSize;

  if ( this->HashTable == NULL ) 
    {
    vtkErrorMacro(<<"Can't build representation...no data!");
    return;
    }

  pts = vtkPoints::New();
  pts->Allocate(5000);
  polys = vtkCellArray::New();
  polys->Allocate(10000);

  // loop over all buckets, creating appropriate faces 
  sliceSize = this->Divisions[0] * this->Divisions[1];
  for ( k=0; k < this->Divisions[2]; k++)
    {
    offset[2] = k * sliceSize;
    minusOffset[2] = (k-1) * sliceSize;
    for ( j=0; j < this->Divisions[1]; j++)
      {
      offset[1] = j * this->Divisions[0];
      minusOffset[1] = (j-1) * this->Divisions[0];
      for ( i=0; i < this->Divisions[0]; i++)
        {
        offset[0] = i;
        minusOffset[0] = i - 1;
        idx = offset[0] + offset[1] + offset[2];
        if ( this->HashTable[idx] == NULL )
	  {
	  inside = 0;
	  }
        else
	  {
	  inside = 1;
	  }

        //check "negative" neighbors
        for (ii=0; ii < 3; ii++)
          {
          if ( minusOffset[ii] < 0 )
            {
            if ( inside )
	      {
	      this->GenerateFace(ii,i,j,k,pts,polys);
	      }
            }
          else
            {
            if ( ii == 0 )
	      {
	      idx = minusOffset[0] + offset[1] + offset[2];
	      }
            else if ( ii == 1 )
	      {
	      idx = offset[0] + minusOffset[1] + offset[2];
	      }
            else
	      {
	      idx = offset[0] + offset[1] + minusOffset[2];
	      }

            if ( (this->HashTable[idx] == NULL && inside) ||
            (this->HashTable[idx] != NULL && !inside) )
              {
              this->GenerateFace(ii,i,j,k,pts,polys);
              }
            }
          //those buckets on "positive" boundaries can generate faces specially
          if ( (i+1) >= this->Divisions[0] && inside )
            {
            this->GenerateFace(0,i+1,j,k,pts,polys);
            }
          if ( (j+1) >= this->Divisions[1] && inside )
            {
            this->GenerateFace(1,i,j+1,k,pts,polys);
            }
          if ( (k+1) >= this->Divisions[2] && inside )
            {
            this->GenerateFace(2,i,j,k+1,pts,polys);
            }

          }//over negative faces
        }//over i divisions
      }//over j divisions
    }//over k divisions


  pd->SetPoints(pts);
  pts->Delete();
  pd->SetPolys(polys);
  polys->Delete();
  pd->Squeeze();
}

void vtkPointLocator::GenerateFace(int face, int i, int j, int k, 
                                   vtkPoints *pts, vtkCellArray *polys)
{
  int ids[4];
  float origin[3], x[3];

  // define first corner
  origin[0] = this->Bounds[0] + i * this->H[0];
  origin[1] = this->Bounds[2] + j * this->H[1];
  origin[2] = this->Bounds[4] + k * this->H[2];
  ids[0] = pts->InsertNextPoint(origin);

  if ( face == 0 ) //x face
    {
    x[0] = origin[0];
    x[1] = origin[1] + this->H[1];
    x[2] = origin[2];
    ids[1] = pts->InsertNextPoint(x);

    x[0] = origin[0];
    x[1] = origin[1] + this->H[1];
    x[2] = origin[2] + this->H[2];
    ids[2] = pts->InsertNextPoint(x);

    x[0] = origin[0];
    x[1] = origin[1];
    x[2] = origin[2] + this->H[2];
    ids[3] = pts->InsertNextPoint(x);
    }

  else if ( face == 1 ) //y face
    {
    x[0] = origin[0] + this->H[0];
    x[1] = origin[1];
    x[2] = origin[2];
    ids[1] = pts->InsertNextPoint(x);

    x[0] = origin[0] + this->H[0];
    x[1] = origin[1];
    x[2] = origin[2] + this->H[2];
    ids[2] = pts->InsertNextPoint(x);

    x[0] = origin[0];
    x[1] = origin[1];
    x[2] = origin[2] + this->H[2];
    ids[3] = pts->InsertNextPoint(x);
    }

  else //z face
    {
    x[0] = origin[0] + this->H[0];
    x[1] = origin[1];
    x[2] = origin[2];
    ids[1] = pts->InsertNextPoint(x);

    x[0] = origin[0] + this->H[0];
    x[1] = origin[1] + this->H[1];
    x[2] = origin[2];
    ids[2] = pts->InsertNextPoint(x);

    x[0] = origin[0];
    x[1] = origin[1] + this->H[1];
    x[2] = origin[2];
    ids[3] = pts->InsertNextPoint(x);
    }

  polys->InsertNextCell(4,ids);
}


void vtkPointLocator::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkLocator::PrintSelf(os,indent);

  os << indent << "Number of Points Per Bucket: " << this->NumberOfPointsPerBucket << "\n";
  os << indent << "Divisions: (" << this->Divisions[0] << ", " 
     << this->Divisions[1] << ", " << this->Divisions[2] << ")\n";

}

