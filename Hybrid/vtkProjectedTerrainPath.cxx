/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkProjectedTerrainPath.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPriorityQueue.h"
#include "vtkImageData.h"
#include "vtkPolyData.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkDoubleArray.h"
#include "vtkExecutive.h"
#include "vtkMath.h"
#include "vtkPixel.h"
#include <vtkstd/vector>

// Define the edge list class--------------------------------------------------
struct vtkEdge
{
  vtkEdge(vtkIdType v1, vtkIdType v2) : V1(v1), V2(v2) {}
  
  vtkIdType V1;
  vtkIdType V2;
};

class vtkEdgeList : public vtkstd::vector<vtkEdge> {};
typedef vtkEdgeList EdgeListType;
typedef vtkEdgeList::iterator EdgeListIterator;


// Begin vtkProjectedTerrainPath class implementation--------------------------
//
vtkCxxRevisionMacro(vtkProjectedTerrainPath, "$Revision$");
vtkStandardNewMacro(vtkProjectedTerrainPath);

//-----------------------------------------------------------------------------
vtkProjectedTerrainPath::vtkProjectedTerrainPath()
{
  this->SetNumberOfInputPorts(2);

  this->ProjectionMode = SIMPLE_PROJECTION;
  this->HeightOffset = 10.0;
  this->HeightTolerance = 10.0;
  this->SubdivisionFactor = 1000;
  this->PositiveLineError = NULL;
  this->NegativeLineError = NULL;
}

//-----------------------------------------------------------------------------
vtkProjectedTerrainPath::~vtkProjectedTerrainPath()
{
}

//-----------------------------------------------------------------------------
void vtkProjectedTerrainPath::SetSource(vtkImageData *source)
{
  this->SetInput(1, source);
}

//-----------------------------------------------------------------------------
vtkImageData *vtkProjectedTerrainPath::GetSource()
{
  if (this->GetNumberOfInputConnections(1) < 1)
    {
    return NULL;
    }
  return vtkImageData::SafeDownCast(this->GetExecutive()->GetInputData(1, 0));
}

//-----------------------------------------------------------------------------
int vtkProjectedTerrainPath::FillInputPortInformation(int port, 
                                                      vtkInformation *info)
{
  if (port == 0)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
    return 1;
    }
  else if (port == 1)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkImageData");
    return 1;
    }
  return 0;
}

//-----------------------------------------------------------------------------
int vtkProjectedTerrainPath::RequestData(vtkInformation *,
                                         vtkInformationVector **inputVector,
                                         vtkInformationVector *outputVector)
{
  // get the input and output
  vtkInformation *linesInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *imageInfo = inputVector[1]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  vtkPolyData *lines = vtkPolyData::SafeDownCast(
    linesInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkImageData *image = vtkImageData::SafeDownCast(
    imageInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  
  vtkPoints *inPoints = lines->GetPoints();
  vtkIdType numPts = inPoints->GetNumberOfPoints();
  vtkCellArray *inLines = lines->GetLines();
  vtkIdType numLines = inLines->GetNumberOfCells();
  if ( numLines <= 0 )
    {
    vtkErrorMacro("This filter requires lines as input");
    return 1;
    }
  
  image->GetDimensions(this->Dimensions);
  image->GetOrigin(this->Origin);
  image->GetSpacing(this->Spacing);
  image->GetExtent(this->Extent);
  this->Heights = image->GetPointData()->GetScalars();
  
  this->Points = vtkPoints::New();
  this->Points->SetDataTypeToDouble();
  this->Points->Allocate(numPts);
  output->SetPoints(this->Points);
  this->Points->Delete(); //ok reference counting
  
  vtkPointData *inPD = lines->GetPointData();
  vtkPointData *outPD = output->GetPointData();
  outPD->CopyAllocate(inPD);

  // The algorithm runs in three parts. First, the existing points are
  // projected onto the image (with the height offset). Next, if requested
  // the edges are checked for occlusion. Finally, if requested, the edges
  // are adjusted to hug the terrain.
  int ij[2];
  vtkIdType i;
  double x[3], z, loc[2];
  for (i=0; i<numPts; i++)
    {
    inPoints->GetPoint(i,x);
    this->GetImageIndex(x,loc,ij);
    z = this->GetHeight(loc,ij);
    this->Points->InsertPoint(i, x[0], x[1], z);
    outPD->CopyData(inPD,i,i);
    }

  // If mode is simple, then just spit out the original polylines
  if ( this->ProjectionMode == SIMPLE_PROJECTION )
    {
    output->SetLines(inLines);
    return 1;
    }

  // If here, we've got to get fancy and start the subdivision process.
  // This means creating some fancy data structures: a dynamic list
  // for the edges. The structure is implicit: the first two entries
  // in the list (i,i+1) form an edge; the next two (i+1,i+2) form the
  // next edge, and so on. The list contains point ids referring to
  // the this->Points array.
  vtkIdType j, npts, *pts;
  this->EdgeList = new EdgeListType;
  for ( inLines->InitTraversal(); inLines->GetNextCell(npts,pts); )
    {
    for (j=0; j<(npts-1); j++)
      {
      this->EdgeList->push_back(vtkEdge(pts[j],pts[j+1]));
      this->ComputeError(this->EdgeList->size()-1);
      }
    }
  this->PositiveLineError = vtkPriorityQueue::New();
  this->NegativeLineError = vtkPriorityQueue::New();
  
  if ( this->ProjectionMode == NONOCCLUDED_PROJECTION )
    {
    }
  else //if ( this->ProjectionMode == HUG_PROJECTION )
    {
    }

  // Clean up
  delete this->EdgeList;
  this->PositiveLineError->Delete();
  this->NegativeLineError->Delete();
  
  return 1;
}

//-----------------------------------------------------------------------------
// Warning: this method may return negative indices. This is expected behavior
//
inline void vtkProjectedTerrainPath::GetImageIndex(double x[3], 
                                                     double loc[2], int ij[2])
{
  loc[0] = (x[0] - this->Origin[0]) / this->Spacing[0];
  ij[0] = (int) (floor(loc[0]));
  loc[1] = (x[1] - this->Origin[1]) / this->Spacing[1];
  ij[1] = (int) (floor(loc[1]));
}

// if the line lies outside of the image.
double vtkProjectedTerrainPath::GetHeight(double loc[2], int ij[2])
{  
  //  Compute the ij location (assuming 2D image plane)
  //
  int i;
  double pcoords[2];
  for (i=0; i<2; i++) 
    {
    if ( ij[i] >= this->Extent[i*2] && ij[i] < this->Extent[i*2 + 1] )
      {
      pcoords[i] = loc[i] - (double)ij[i];
      }

    else if ( ij[i] < this->Extent[i*2] || ij[i] > this->Extent[i*2+1] ) 
      {
      return this->HeightOffset;
      } 

    else //if ( ij[i] == this->Extent[i*2+1] )
      {
      if (this->Dimensions[i] == 1)
        {
        pcoords[i] = 0.0;
        }
      else
        {
        ij[i] -= 1;
        pcoords[i] = 1.0;
        }
      }
    }
  
  // Interpolate the height
  double weights[4], s0, s1, s2, s3;
  vtkPixel::InterpolationFunctions(pcoords,weights);
  s0 = this->Heights->GetTuple1(ij[0]+   ij[1]*this->Dimensions[0]);
  s1 = this->Heights->GetTuple1(ij[0]+1+ ij[1]*this->Dimensions[0]);
  s2 = this->Heights->GetTuple1(ij[0]+  (ij[1]+1)*this->Dimensions[0]);
  s3 = this->Heights->GetTuple1(ij[0]+1+(ij[1]+1)*this->Dimensions[0]);
    
  return (this->Origin[2] + this->HeightOffset + s0*weights[0] + s1*weights[1] + 
    s2*weights[2] + s3*weights[3]);
}

//-----------------------------------------------------------------------------
//This method has the side effect of inserting the edge into the queues
void vtkProjectedTerrainPath::ComputeError(vtkIdType edgeId)
{
  vtkEdge &e =(*this->EdgeList)[edgeId];
  double p1[3], p2[3];
  this->Points->GetPoint(e.V1,p1);
  this->Points->GetPoint(e.V2,p2);

  // Now evaluate the edge as it passes over the pixel cell edges. The
  // interpolation functions are such that the maximum values have to
  // take place on the boundary of the cell. We process the cell edges in
  // two passes: first the x-edges, then the y-edges.
  double negError = VTK_LARGE_FLOAT;
  double posError = -VTK_LARGE_FLOAT;
  double x[3], loc[2], t, zMap, loc1[2], loc2[2], *x1, *x2, error;
  int ij[2], ij1[2], ij2[2], numInt, i;
  
  // Process the x intersections
  if ( p2[0] >= p1[0] ) //sort along x-axis
    {
    x1 = p1;
    x2 = p2;
    }
  else
    {
    x1 = p2;
    x2 = p1;
    }
  this->GetImageIndex(x1,loc1,ij1);
  this->GetImageIndex(x2,loc2,ij2);

  if ( (numInt=ij2[0]-ij1[0]) > 0 ) //if there are any x-intersections
    {
    for (i=1; i<=numInt; i++)
      {
      if ( (ij1[0]+i) >= this->Extent[0] )
        {
        x[0] = this->Origin[0] + (ij1[0]+i)*this->Spacing[0];
        t = (x[0] - x1[0]) / (x2[0] - x1[0]);
        x[1] = x1[1] + t*(x2[1]-x1[1]);
        x[2] = x1[2] + t*(x2[2]-x1[2]);
        this->GetImageIndex(x,loc,ij);
        zMap = this->GetHeight(loc,ij);
        error = x[2] - zMap;
        if ( error >= 0.0 )
          {
          posError = (error > posError ? error : posError);
          }
        else
          {
          negError = (error < negError ? error : negError);
          }
        } //if lying on image
      } //for all x-intersection points
    } //if x-intersections

  // Process the y intersections
  if ( p2[1] >= p1[1] ) //sort along y-axis
    {
    x1 = p1;
    x2 = p2;
    }
  else
    {
    x1 = p2;
    x2 = p1;
    }
  this->GetImageIndex(x1,loc1,ij1);
  this->GetImageIndex(x2,loc2,ij2);

  if ( (numInt=ij2[1]-ij1[1]) > 0 ) //if there are any x-intersections
    {
    for (i=1; i<=numInt; i++)
      {
      if ( (ij1[1]+i) >= this->Extent[2] )
        {
        x[1] = this->Origin[1] + (ij1[1]+i)*this->Spacing[1];
        t = (x[1] - x1[1]) / (x2[1] - x1[1]);
        x[0] = x1[0] + t*(x2[0]-x1[0]);
        x[2] = x1[2] + t*(x2[2]-x1[2]);
        this->GetImageIndex(x,loc,ij);
        zMap = this->GetHeight(loc,ij);
        error = x[2] - zMap;
        if ( error >= 0.0 )
          {
          posError = (error > posError ? error : posError);
          }
        else
          {
          negError = (error < negError ? error : negError);
          }
        } //if lying on image
      } //for all x-intersection points
    } //if x-intersections

  // Okay, insert the maximum errors for this edge in the queues
  this->PositiveLineError->Insert(-posError,edgeId);
  this->NegativeLineError->Insert(negError,edgeId);
}


//-----------------------------------------------------------------------------
void vtkProjectedTerrainPath::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
 
  os << indent << "Projection Mode: ";
  if ( this->ProjectionMode == SIMPLE_PROJECTION )
    {
    os << "Simple Projection\n";
    }
  else if ( this->ProjectionMode == NONOCCLUDED_PROJECTION )
    {
    os << "Non-occluded Projection\n";
    }
  else //if ( this->ProjectionMode == HUG_PROJECTION )
    {
    os << "Hug Projection\n";
    }
  
  os << indent << "Height Offset: " << this->HeightOffset << "\n";
  os << indent << "Height Tolerance: " << this->HeightTolerance << "\n";
  os << indent << "Subdivision Factor: " << this->SubdivisionFactor << "\n";
  
}
