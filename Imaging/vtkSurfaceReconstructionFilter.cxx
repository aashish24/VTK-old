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
#include "vtkSurfaceReconstructionFilter.h"
#include "vtkFloatArray.h"
#include "vtkPointLocator.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkSurfaceReconstructionFilter, "$Revision$");
vtkStandardNewMacro(vtkSurfaceReconstructionFilter);

vtkSurfaceReconstructionFilter::vtkSurfaceReconstructionFilter()
{
  this->NeighborhoodSize = 20;
  this->SampleSpacing = -1.0; // negative values cause the algorithm to make a reasonable guess
}

// some simple routines for vector math
void vtkCopyBToA(float* a,float* b) 
{ 
  for(int i=0;i<3;i++) 
    {
    a[i] = b[i]; 
    }
}
void vtkSubtractBFromA(float* a,float* b) 
{ 
  for(int i=0;i<3;i++) 
    {
    a[i] -= b[i]; 
    }
}
void vtkAddBToA(float* a,float* b) 
{ 
  for(int i=0;i<3;i++) 
    {
    a[i] += b[i]; 
    }
}
void vtkMultiplyBy(float* a,float f) 
{ 
  for(int i=0;i<3;i++) 
    {
    a[i] *= f; 
    }
}
void vtkDivideBy(float* a,float f) 
{ 
  for(int i=0;i<3;i++) 
    {
    a[i] /= f; 
    }
}

// Routines for matrix creation
void vtkSRFreeMatrix(float **m, long nrl, long nrh, long ncl, long nch);
float **vtkSRMatrix(long nrl, long nrh, long ncl, long nch);
void vtkSRFreeVector(float *v, long nl, long nh);
float *vtkSRVector(long nl, long nh);

// set a matrix to zero
void vtkSRMakeZero(float **m,long nrl, long nrh, long ncl, long nch) 
{ 
  int i,j; 
  for(i=nrl;i<=nrh;i++) 
    {
    for(j=ncl;j<=nch;j++) 
      {
      m[i][j] = 0.0; 
      }
    }
}

// add v*Transpose(v) to m, where v is 3x1 and m is 3x3
void vtkSRAddOuterProduct(float **m,float *v);

// scalar multiply a matrix
void vtkSRMultiply(float **m,float f,long nrl, long nrh, long ncl, long nch)
{ 
  int i,j; 
  for(i=nrl;i<=nrh;i++) 
    {
    for(j=ncl;j<=nch;j++) 
      {
      m[i][j] *= f; 
      }
    }
}

//-----------------------------------------------------------------------------
void vtkSurfaceReconstructionFilter::Execute()
{
  // Initialise the variables we need within this function
  vtkDataSet *input = this->GetInput();
  vtkStructuredPoints *output = this->GetOutput();

  struct SurfacePoint 
  {
    float loc[3];
    float o[3],n[3]; // plane centre and normal
    vtkIdList *neighbors; // id's of points within LocalRadius of this point
    float *costs; // should have same length as neighbors, cost for corresponding points
    char isVisited;

    // simple constructor to initialise the members
    SurfacePoint() : neighbors(vtkIdList::New()), isVisited(0) {}
    ~SurfacePoint() { delete []costs; neighbors->Delete(); }
  };

  const vtkIdType COUNT = input->GetNumberOfPoints();
  SurfacePoint *surfacePoints;

  vtkIdType i, j;
  int k;

  if ( COUNT < 1 )
    {
    vtkErrorMacro(<<"No points to reconstruct");
    return;
    }
  surfacePoints = new SurfacePoint[COUNT];
  
  vtkDebugMacro(<<"Reconstructing " << COUNT << " points");

  //time_t start_time,t1,t2,t3,t4;
  //time(&start_time);

  // --------------------------------------------------------------------------
  // 1. Build local connectivity graph
  // -------------------------------------------------------------------------
  {
  vtkPointLocator *locator = vtkPointLocator::New();
  locator->SetDataSet(input);
  vtkIdList *locals = vtkIdList::New();
  // if a pair is close, add each one as a neighbor of the other
  for(i=0;i<COUNT;i++)
    {
    SurfacePoint *p = &surfacePoints[i];
    vtkCopyBToA(p->loc,input->GetPoint(i));
        locator->FindClosestNPoints(this->NeighborhoodSize,p->loc,locals);
    int iNeighbor;
    for(j=0;j<locals->GetNumberOfIds();j++)
      {
      iNeighbor = locals->GetId(j);
      if(iNeighbor!=i)
        {
        p->neighbors->InsertNextId(iNeighbor);
        surfacePoints[iNeighbor].neighbors->InsertNextId(i);
        }
      }
    }
  locator->Delete();
  locals->Delete();
  }

  //time(&t1);
  // --------------------------------------------------------------------------
  // 2. Estimate a plane at each point using local points
  // --------------------------------------------------------------------------
  {
  float *pointi;
  float **covar,*v3d,*eigenvalues,**eigenvectors;
  covar = vtkSRMatrix(0,2,0,2);
  v3d = vtkSRVector(0,2);
  eigenvalues = vtkSRVector(0,2);
  eigenvectors = vtkSRMatrix(0,2,0,2);
  for(i=0;i<COUNT;i++)
    {
    SurfacePoint *p = &surfacePoints[i];
    
    // first find the centroid of the neighbors
    vtkCopyBToA(p->o,p->loc);
    int number=1;
    vtkIdType neighborIndex;
    for(j=0;j<p->neighbors->GetNumberOfIds();j++)
      {
      neighborIndex = p->neighbors->GetId(j);
      pointi = input->GetPoint(neighborIndex);
      vtkAddBToA(p->o,pointi);
      number++;
      }
    vtkDivideBy(p->o,number);
    // then compute the covariance matrix
    vtkSRMakeZero(covar,0,2,0,2);
    for(k=0;k<3;k++)
      v3d[k] = p->loc[k] - p->o[k];
    vtkSRAddOuterProduct(covar,v3d);
    for(j=0;j<p->neighbors->GetNumberOfIds();j++)
      {
      neighborIndex = p->neighbors->GetId(j);
      pointi = input->GetPoint(neighborIndex);
      for(k=0;k<3;k++)
        {
        v3d[k] = pointi[k] - p->o[k];
        }
      vtkSRAddOuterProduct(covar,v3d);
      }
    vtkSRMultiply(covar,1.0/number,0,2,0,2);
    // then extract the third eigenvector
    vtkMath::Jacobi(covar,eigenvalues,eigenvectors);
    // third eigenvector (column 2, ordered by eigenvalue magnitude) is plane normal
    for(k=0;k<3;k++)
      {
      p->n[k] = eigenvectors[k][2];
      }
    }
  vtkSRFreeMatrix(covar,0,2,0,2);
  vtkSRFreeVector(v3d,0,2);
  vtkSRFreeVector(eigenvalues,0,2);
  vtkSRFreeMatrix(eigenvectors,0,2,0,2);
  }

  //time(&t2);
  //--------------------------------------------------------------------------
  // 3a. Compute a cost between every pair of neighbors for the MST
  // --------------------------------------------------------------------------
  // cost = 1 - |normal1.normal2|
  // ie. cost is 0 if planes are parallel, 1 if orthogonal (least parallel)
  for(i=0;i<COUNT;i++)
    {
    SurfacePoint *p = &surfacePoints[i];
    p->costs = new float[p->neighbors->GetNumberOfIds()];

    // compute cost between all its neighbors
    // (bit inefficient to do this for every point, as cost is symmetric)
    for(j=0;j<p->neighbors->GetNumberOfIds();j++)
      {
      p->costs[j] = 1.0 - 
        fabs(vtkMath::Dot(p->n,surfacePoints[p->neighbors->GetId(j)].n));
      }
    }

  // --------------------------------------------------------------------------
  // 3b. Ensure consistency in plane direction between neighbors
  // --------------------------------------------------------------------------
  // method: guess first one, then walk through tree along most-parallel
  // neighbors MST, flipping the new normal if inconsistent

  // to walk minimal spanning tree, keep record of vertices visited and list
  // of those near to any visited point but not themselves visited. Start
  // with just one vertex as visited.  Pick the vertex in the neighbors list
  // that has the lowest cost connection with a visited vertex. Record this
  // vertex as visited, add any new neighbors to the neighbors list.

  int orientationPropagation=1;
  if(orientationPropagation) 
    {// set to false if you don't want orientation propagation (for testing)
    vtkIdList *nearby = vtkIdList::New(); // list of nearby, unvisited points
    
    // start with some vertex
    int first=0; // index of starting vertex
    surfacePoints[first].isVisited = 1;
    // add all the neighbors of the starting vertex into nearby
    for(j=0;j<surfacePoints[first].neighbors->GetNumberOfIds();j++)
      {
      nearby->InsertNextId(surfacePoints[first].neighbors->GetId(j));
      }
    
    float cost,lowestCost;
    int cheapestNearby = 0, connectedVisited = 0;
    
    // repeat until nearby is empty:
    while(nearby->GetNumberOfIds()>0)
      {
      // for each nearby point:
      vtkIdType iNearby,iNeighbor;
      lowestCost = VTK_LARGE_FLOAT;
      for(i=0;i<nearby->GetNumberOfIds();i++)
        {
        iNearby = nearby->GetId(i);
        // test cost against all neighbors that are members of visited
        for(j=0;j<surfacePoints[iNearby].neighbors->GetNumberOfIds();j++)
          {
          iNeighbor = surfacePoints[iNearby].neighbors->GetId(j);
          if(surfacePoints[iNeighbor].isVisited)
            {
            cost = surfacePoints[iNearby].costs[j];
            // pick lowest cost for this nearby point
            if(cost<lowestCost) 
              {
              lowestCost = cost;
              cheapestNearby = iNearby;
              connectedVisited = iNeighbor;
              // optional: can break out if satisfied with parallelness
              if(lowestCost<0.1)
                {
                i = nearby->GetNumberOfIds();
                break;
                }
              }
            }
          }
        }
      if(connectedVisited == cheapestNearby)
        {
        vtkErrorMacro (<< "Internal error in vtkSurfaceReconstructionFilter");
        return;
        }
      
      // correct the orientation of the point if necessary
      if(vtkMath::Dot(surfacePoints[cheapestNearby].n,
                      surfacePoints[connectedVisited].n)<0.0F)
        {
        // flip this normal
        vtkMultiplyBy(surfacePoints[cheapestNearby].n,-1);
        }
      // add this nearby point to visited
      if(surfacePoints[cheapestNearby].isVisited != 0)
        {
        vtkErrorMacro (<< "Internal error in vtkSurfaceReconstructionFilter");
        return;
        }
      
      surfacePoints[cheapestNearby].isVisited = 1;
      // remove from nearby
      nearby->DeleteId(cheapestNearby);
      // add all new nearby points to nearby
      for(j=0;j<surfacePoints[cheapestNearby].neighbors->GetNumberOfIds();j++)
        {
        iNeighbor = surfacePoints[cheapestNearby].neighbors->GetId(j);
        if(surfacePoints[iNeighbor].isVisited == 0)
          {
          nearby->InsertUniqueId(iNeighbor);
          }
        }
      }
    
    nearby->Delete();
    }

  //time(&t3);

  // --------------------------------------------------------------------------
  // 4. Compute signed distance to surface for every point on a 3D grid
  // --------------------------------------------------------------------------
  {
  // need to know the bounding rectangle
  float bounds[6];
  for(i=0;i<3;i++)
    {
    bounds[i*2]=input->GetBounds()[i*2];
    bounds[i*2+1]=input->GetBounds()[i*2+1];
    }

  // estimate the spacing if required
  if(this->SampleSpacing<=0.0)
    {
    // spacing guessed as cube root of (volume divided by number of points)
    this->SampleSpacing = pow((double)(bounds[1]-bounds[0])*
                              (bounds[3]-bounds[2])*(bounds[5]-bounds[4]) /
                              (float)COUNT, (double)(1.0/3.0));
 
    vtkDebugMacro(<<"Estimated sample spacing as: " << this->SampleSpacing );
    }

  // allow a border around the volume to allow sampling around the extremes
  for(i=0;i<3;i++)
    {
    bounds[i*2]-=this->SampleSpacing*2;
    bounds[i*2+1]+=this->SampleSpacing*2;
    }

  float topleft[3] = {bounds[0],bounds[2],bounds[4]};
  float bottomright[3] = {bounds[1],bounds[3],bounds[5]};
  int dim[3];
  for(i=0;i<3;i++)
    {
    dim[i] = (int)((bottomright[i]-topleft[i])/this->SampleSpacing);
    }
  
  vtkDebugMacro(<<"Created output volume of dimensions: ("
                << dim[0] << ", " << dim[1] << ", " << dim[2] << ")" );

  // initialise the output volume
  output->SetDimensions(dim[0],dim[1],dim[2]);
  output->SetSpacing(this->SampleSpacing, this->SampleSpacing,
                     this->SampleSpacing);
  output->SetOrigin(topleft);
  
  // initialise the point locator (have to use point insertion because we
  // need to set our own bounds, slightly larger than the dataset to allow
  // for sampling around the edge)
  vtkPointLocator *locator = vtkPointLocator::New();
  vtkPoints *newPts = vtkPoints::New();
  locator->InitPointInsertion(newPts,bounds,(int)COUNT);
  for(i=0;i<COUNT;i++)
    {
    locator->InsertPoint(i,surfacePoints[i].loc);
    }

  // go through the array probing the values
  vtkFloatArray *volScalars = vtkFloatArray::New();
  int x,y,z;
  int iClosestPoint;
  int zOffset,yOffset,offset;
  float probeValue;
  float point[3],temp[3];
  for(z=0;z<dim[2];z++)
    {
    zOffset = z*dim[1]*dim[0];
    point[2] = topleft[2] + z*this->SampleSpacing;
    for(y=0;y<dim[1];y++)
      {
      yOffset = y*dim[0] + zOffset;
      point[1] = topleft[1] + y*this->SampleSpacing;
      for(x=0;x<dim[0];x++)
        {
        offset = x + yOffset;
        point[0] = topleft[0] + x*this->SampleSpacing;
        // find the distance from the probe to the plane of the nearest point
        iClosestPoint = locator->FindClosestInsertedPoint(point);
        if(iClosestPoint==-1) 
          {
          vtkErrorMacro (<< "Internal error");
          return;
          }
        vtkCopyBToA(temp,point);
        vtkSubtractBFromA(temp,surfacePoints[iClosestPoint].loc);
        probeValue = vtkMath::Dot(temp,surfacePoints[iClosestPoint].n);
        volScalars->InsertValue(offset,probeValue);
        }
      }
    }
  locator->Delete();
  newPts->Delete();
  
  output->GetPointData()->SetScalars(volScalars);
  volScalars->Delete();
  }

  //time(&t4);
  // Clear up everything
  delete [] surfacePoints;
}

void vtkSurfaceReconstructionFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "Neighborhood Size:" << this->NeighborhoodSize << "\n";
  os << indent << "Sample Spacing:" << this->SampleSpacing << "\n";
}

void vtkSRAddOuterProduct(float **m,float *v)
{
  int i,j;
  for(i=0;i<3;i++)
    {
    for(j=0;j<3;j++)
      {
      m[i][j] += v[i]*v[j];
      }
    }
}

#define VTK_NR_END 1
#define VTK_FREE_ARG char*

// allocate a float vector with subscript range v[nl..nh]
float *vtkSRVector(long nl, long nh)        
{ 
  float *v;

  v = new float [nh-nl+1+VTK_NR_END];
  if (!v) 
    {
    vtkGenericWarningMacro(<<"allocation failure in vector()");
    return NULL;
    }
  
  return (v-nl+VTK_NR_END); 
}

// allocate a float matrix with subscript range m[nrl..nrh][ncl..nch]
float **vtkSRMatrix(long nrl, long nrh, long ncl, long nch)        
{
  long i, nrow=nrh-nrl+1,ncol=nch-ncl+1;
  float **m;

  // allocate pointers to rows
  m = new float * [nrow+VTK_NR_END];
  if (!m) 
    {
    vtkGenericWarningMacro(<<"allocation failure 1 in Matrix()");
    return NULL;
    }
  
  m += VTK_NR_END;
  m -= nrl;

  // allocate rows and set pointers to them
  m[nrl] = new float[nrow*ncol+VTK_NR_END];
  if (!m[nrl]) 
    {
    vtkGenericWarningMacro("allocation failure 2 in Matrix()");
    return NULL;
    }
  
  m[nrl] += VTK_NR_END;
  m[nrl] -= ncl;
  for(i=nrl+1;i<=nrh;i++) 
    {
    m[i] = m[i-1]+ncol;
    }

  // return pointer to array of pointers to rows
  return m;
}

// free a float vector allocated with SRVector()
void vtkSRFreeVector(float *v, long nl, long vtkNotUsed(nh))
{ 
  delete [] (v+nl-VTK_NR_END);
}

// free a float matrix allocated by Matrix()
void vtkSRFreeMatrix(float **m, long nrl, long vtkNotUsed(nrh),
                     long ncl, long vtkNotUsed(nch))
        
{
  delete [] (m[nrl]+ncl-VTK_NR_END);
  delete [] (m+nrl-VTK_NR_END);
}

#undef VTK_NR_END
#undef VTK_FREE_ARG



