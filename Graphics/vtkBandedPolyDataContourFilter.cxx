/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen 
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
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkBandedPolyDataContourFilter.h"
#include "vtkEdgeTable.h"
#include "vtkFloatArray.h"
#include "vtkTriangleStrip.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkBandedPolyDataContourFilter, "$Revision$");
vtkStandardNewMacro(vtkBandedPolyDataContourFilter);

// Construct object.
vtkBandedPolyDataContourFilter::vtkBandedPolyDataContourFilter()
{
  this->ContourValues = vtkContourValues::New();
  this->Clipping = 0;
}

vtkBandedPolyDataContourFilter::~vtkBandedPolyDataContourFilter()
{
  this->ContourValues->Delete();
}

int vtkBandedPolyDataContourFilter::ComputeIndex(float val1, float val2)
{
  float val = (val1+val2) / 2.0;
  for (int i=0; i < (this->NumberOfClipValues-1); i++)
    {
    if ( val >= this->ClipValues[i] && val < this->ClipValues[i+1]  )
      {
      return i;
      }
    }
  return 0;
}

int vtkBandedPolyDataContourFilter::ComputeLowerScalarIndex(float val)
{
  for (int i=0; i < (this->NumberOfClipValues-1); i++)
    {
    if ( val >= this->ClipValues[i] && val < this->ClipValues[i+1]  )
      {
      return i;
      }
    }
  return this->NumberOfClipValues - 1;
}

int vtkBandedPolyDataContourFilter::ComputeUpperScalarIndex(float val)
{
  for (int i=0; i < (this->NumberOfClipValues-1); i++)
    {
    if ( val > this->ClipValues[i] && val <= this->ClipValues[i+1]  )
      {
      return i;
      }
    }
  return 0;
}

int vtkBandedPolyDataContourFilter::IsContourValue(float val)
{
  int i;

  // Check to see whether a vertex is an intersection point.
  for ( i=0; i < this->NumberOfClipValues; i++)
    {
    if ( val == this->ClipValues[i] )
      {
      return 1;
      }
    }
  return 0;
}

// v1 assumed < v2
int vtkBandedPolyDataContourFilter::ClipEdge(int v1, int v2,
                                             vtkPoints *newPts, 
                                             vtkDataArray *scalars,
                                             vtkPointData *inPD,
                                             vtkPointData *outPD)
{
  float x[3], t;
  int ptId;

  float *x1 = newPts->GetPoint(v1);
  float *x2 = newPts->GetPoint(v2);

  float s1 = scalars->GetTuple1(v1);
  float s2 = scalars->GetTuple1(v2);
  
  if ( s1 <= s2 )
    {
    int idx1 = this->ComputeLowerScalarIndex(s1);
    int idx2 = this->ComputeUpperScalarIndex(s2);
    for (int i=1; i < (idx2-idx1+1); i++)
      {
      t = (this->ClipValues[idx1+i] - s1) / (s2 - s1);
      x[0] = x1[0] + t*(x2[0]-x1[0]);
      x[1] = x1[1] + t*(x2[1]-x1[1]);
      x[2] = x1[2] + t*(x2[2]-x1[2]);
      ptId = newPts->InsertNextPoint(x);
      outPD->InterpolateEdge(inPD,ptId,v1,v2,t);
      }

    return (idx2-idx1+1);
    }
  else
    {
    int idx2 = this->ComputeLowerScalarIndex(s2);
    int idx1 = this->ComputeUpperScalarIndex(s1);
    for (int i=1; i < (idx1-idx2+1); i++)
      {
      t = (this->ClipValues[idx2+i] - s1) / (s2 - s1);
      x[0] = x1[0] + t*(x2[0]-x1[0]);
      x[1] = x1[1] + t*(x2[1]-x1[1]);
      x[2] = x1[2] + t*(x2[2]-x1[2]);
      ptId = newPts->InsertNextPoint(x);
      outPD->InterpolateEdge(inPD,ptId,v1,v2,t);
      }

    return (idx1-idx2+1);
    }
}


extern "C" {
int vtkCompareContourValues(const void *val1, const void *val2)
{
  if ( *((float*)val1) < *((float*)val2) ) 
    {
    return (-1);
    }
  else if ( *((float*)val1) > *((float*)val2) ) 
    {
    return (1);
    }
  else 
    {
    return (0);
    }
}
}

// Create filled contours for polydata
void vtkBandedPolyDataContourFilter::Execute()
{
  vtkPolyData *input = this->GetInput();
  vtkPointData *pd = input->GetPointData();
  vtkCellData *cd = input->GetCellData();
  vtkPolyData *output = this->GetOutput();
  vtkPointData *outPD = input->GetPointData();
  vtkCellData *outCD = output->GetCellData();
  vtkPoints *inPts = input->GetPoints();
  vtkDataArray *inScalars = pd->GetScalars();
  int numPts, numCells;
  int abort=0;
  vtkPoints *newPts;
  int i, j, idx, npts, cellId=0, ptId=0;
  vtkIdType *pts;

  vtkDebugMacro(<<"Executing banded contour filter");

  //  Check input
  //
  numPts = inPts->GetNumberOfPoints();
  numCells = input->GetNumberOfCells();
  if ( numCells < 1 || numPts < 1 )
    {
    vtkErrorMacro(<<"No input data!");
    return;
    }

  // Set up supplemental data structures for processing edge/generating
  // intersections. First we sort the contour values into an ascending
  // list of clip values including the extreme min/max values.
  this->NumberOfClipValues = this->ContourValues->GetNumberOfContours() + 2;
  this->ClipValues = new float[this->NumberOfClipValues];
  float range[2], tol;
  inScalars->GetRange(range); tol = (range[1]-range[0])/100.0;
  this->ClipValues[0] = range[0];
  this->ClipValues[1] = range[1];
  for ( i=2; i<this->NumberOfClipValues; i++)
    {
    this->ClipValues[i] = this->ContourValues->GetValue(i-2);
    }
  qsort((void *)this->ClipValues, this->NumberOfClipValues, sizeof(float), 
        vtkCompareContourValues);
  for ( i=0; i<(this->NumberOfClipValues-1); i++)
    {
    if ( (this->ClipValues[i]+tol) >= this->ClipValues[i+1] )
      {
      for (j=i+1; j<(this->NumberOfClipValues-2); j++)
        {
        this->ClipValues[j] = this->ClipValues[j+1];
        }
      this->NumberOfClipValues--;
      }
    }

  // The original set of points and point data are copied. Later on 
  // intersection points due to clipping will be created.
  newPts = vtkPoints::New();
  newPts->Allocate(3*numPts);

  outPD->InterpolateAllocate(pd,3*numPts,numPts);
  vtkDataArray *outScalars = outPD->GetScalars();
  
  for (i=0; i<numPts; i++)
    {
    newPts->InsertPoint(i,inPts->GetPoint(i));
    outPD->CopyData(pd, i, i);
    }

  // These are the new cell scalars
  vtkFloatArray *newScalars = vtkFloatArray::New();
  newScalars->Allocate(numCells*5,numCells);

  // All vertices are filled and passed through; poly-vertices are broken
  // into single vertices. Cell data per vertex is set.
  //
  if ( input->GetVerts()->GetNumberOfCells() > 0 )
    {
    vtkCellArray *verts = input->GetVerts();
    vtkCellArray *newVerts = vtkCellArray::New();
    newVerts->Allocate(verts->GetSize());
    for ( verts->InitTraversal(); verts->GetNextCell(npts,pts) && !abort; 
          this->GetAbortExecute() )
      {
      for (i=0; i<npts; i++)
        {
        newVerts->InsertNextCell(1,pts+i);
        idx = this->ComputeLowerScalarIndex(inScalars->GetTuple1(pts[i]));
        newScalars->InsertTuple1(cellId++,idx);
        }
      }
    output->SetVerts(newVerts);
    newVerts->Delete();
    }
  
  // Lines are chopped into line segments.
  //
  if ( input->GetLines()->GetNumberOfCells() > 0 )
    {
    int numSegments;
    vtkCellArray *lines = input->GetLines();
    vtkCellArray *newLines = vtkCellArray::New();
    newLines->Allocate(lines->GetSize());
    for ( lines->InitTraversal(); lines->GetNextCell(npts,pts) && !abort; 
          this->GetAbortExecute() )
      {
      for (i=0; i<(npts-1); i++)
        {
        if ( pts[i] < pts[i+1] )
          {
          numSegments = this->ClipEdge(pts[i],pts[i+1],newPts,inScalars,
                                       pd,outPD);
          }
        else
          {
          numSegments = this->ClipEdge(pts[i+1],pts[i],newPts,inScalars,
                                       pd,outPD);
          }
        for (j=0; j<numSegments; j++)
          {
//          newLines->InsertNextCell(2,this->PtIds+j);
//          newScalars->InsertTuple1(cellId++,this->CellScalars[j]);
          }
        }
      }
    output->SetLines(newLines);
    newLines->Delete();
    }
  
  // Polygons are assumed convex and chopped into filled, convex polygons.
  // Triangle strips are treated similarly.
  //
  int numPolys = input->GetPolys()->GetNumberOfCells();
  int numStrips = numStrips=input->GetStrips()->GetNumberOfCells();
  if ( numPolys > 0 || numStrips > 0 )
    {
    // Set up processing. We are going to store an ordered list of
    // intersections along each edge (ordered from smallest point id
    // to largest). These will later be connected into convex polygons
    // which represent a filled region in the cell.
    //
    vtkEdgeTable *edgeTable = vtkEdgeTable::New();
    edgeTable->InitEdgeInsertion(numPts,1); //store attributes on edge

    vtkCellArray *polys = input->GetPolys();
    vtkCellArray *intList = vtkCellArray::New(); //intersection point ids
    vtkCellArray *tmpPolys = NULL;

    // Lump strips and polygons together, if both exist.
    // We'll have to decompose strips into triangles.
    if ( numStrips > 0 ) 
      {
      vtkCellArray *strips = input->GetStrips();
      tmpPolys = vtkCellArray::New();
      if ( numPolys > 0 )
        {
        tmpPolys->DeepCopy(polys);
        }
      else 
        {
        tmpPolys->Allocate(polys->EstimateSize(numStrips,5));
        }
      for ( strips->InitTraversal(); strips->GetNextCell(npts,pts); )
        {
        vtkTriangleStrip::DecomposeStrip(npts, pts, tmpPolys);
        }
      polys = tmpPolys;
      }

    // Process polygons to produce edge intersections.------------------------
    //
    int numEdgePts, numNewPts, numSegments;
    vtkIdType v, vR;
    for ( polys->InitTraversal(); polys->GetNextCell(npts,pts) && !abort; )
      {
      for (i=0; i<npts; i++)
        {
        v = pts[i];
        vR = pts[(i+1) % npts];
        if ( edgeTable->IsEdge(v,vR) == -1 )
          {
          numNewPts = newPts->GetNumberOfPoints();
          if ( v < vR )
            {
            numSegments = this->ClipEdge(v,vR,newPts,inScalars,pd,outPD);
            }
          else
            {
            numSegments = this->ClipEdge(vR,v,newPts,inScalars,pd,outPD);
            }
          numEdgePts = newPts->GetNumberOfPoints() - numNewPts;
          if ( numEdgePts > 0 )
            {
            intList->InsertNextCell(numEdgePts);
            edgeTable->InsertEdge(v,vR,intList->GetInsertLocation(0));
            for (j=0; j<numEdgePts; j++)
              {
              intList->InsertCellPoint(numNewPts+j);
              }
            }
          else //no intersection points along the edge
            {
            edgeTable->InsertEdge(v,vR,-1); //-1 means no points
            }
          }//if edge not processed yet
        }
      }//for all polygons
    
    // Process polygons to produce output triangles.------------------------
    //
    vtkCellArray *newPolys = vtkCellArray::New();
    newPolys->Allocate(polys->GetSize());
    vtkIdType *intPts;
    int numIntPts, intsInc;
    int intersectionPoint;
    int mL, mR, m2L, m2R;
    int numPointsToAdd, numLeftPointsToAdd, numRightPointsToAdd;
    int numPolyPoints, intsIdx;
    int numFullPts, intLoc;
      
    int maxCellSize = polys->GetMaxCellSize();
    int *edgeInts = new int [maxCellSize]; //intersections around polygon

    maxCellSize *= (1 + this->NumberOfClipValues);
    vtkIdType *newPolygon = new vtkIdType [maxCellSize];
    float *s = new float [maxCellSize]; //scalars at vertices
    int *isContourValue = new int [maxCellSize];
    int *isOriginalVertex = new int [maxCellSize];
    vtkIdType *fullPoly = new vtkIdType [maxCellSize];

    for ( polys->InitTraversal(); polys->GetNextCell(npts,pts) && !abort; 
          this->GetAbortExecute() )
      {
      //Create a new polygon that includes all the points including the
      //intersection vertices. This hugely simplifies the logic of the
      //code.
      for ( intersectionPoint=0, numFullPts=0, i=0; i<npts; i++)
        {
        v = pts[i];
        vR = pts[(i+1)%npts];
        
        s[numFullPts] = outScalars->GetTuple1(v);
        if ( (isContourValue[numFullPts]=this->IsContourValue(s[numFullPts])) )
          {
          intersectionPoint = 1;
          }
             
        isOriginalVertex[numFullPts] = 1;
        fullPoly[numFullPts++] = v;

        //see whether intersection points need to be added.
        if ( (intLoc=edgeTable->IsEdge(v,vR)) != -1 )
          {
          intersectionPoint = 1;
          intList->GetCell(intLoc,numIntPts,intPts);
          if ( v < vR ) {intsIdx = 0; intsInc=1;} //order of the edge
          else {intsIdx=numIntPts-1; intsInc=(-1);}
          for ( ; intsIdx >= 0 && intsIdx < numIntPts; intsIdx += intsInc )
            {
            s[numFullPts] = outScalars->GetTuple1(intPts[intsIdx]);
            isContourValue[numFullPts] = 1;
            isOriginalVertex[numFullPts] = 0;
            fullPoly[numFullPts++] = intPts[intsIdx];
            }
          }
        } //for all points and edges
      
      //Very important: have to find the right starting vertex. The vertex
      //needs to be one where the contour values increase in both directions.
      //Really should check whether the vertex is convex.
      for ( i=0; i<numFullPts; i++)
        {
        if ( isOriginalVertex[i] )
          {
          mL = (i-1+numFullPts) % numFullPts;
          mR = (i+1) % numFullPts;

          if ( s[i] <= s[mL] && s[i] <= s[mR] )
            {
            idx = i;
            break;
            }
          }
        }

      //Trivial output - completely in a contour band or a triangle
      if ( ! intersectionPoint || numFullPts == npts || numFullPts <= 3 )
        {
        newPolys->InsertNextCell(npts,pts);
        newScalars->InsertTuple1(cellId++, 
                                 this->ComputeLowerScalarIndex(s[idx]));
        continue;
        }

      //Find the first intersection points in the polygons starting
      //from this vertex and build a polygon.
      numPointsToAdd = 1;
      for ( mR=idx, intersectionPoint=0; !intersectionPoint; )
        {
        numPointsToAdd++;
        mR = (mR + 1) % numFullPts;
        if ( isContourValue[mR] ) intersectionPoint = 1;
        }
      for ( mL=idx, intersectionPoint=0; !intersectionPoint; )
        {
        numPointsToAdd++;
        mL = (mL + numFullPts - 1) % numFullPts;
        if ( isContourValue[mL] ) intersectionPoint = 1;
        }
      for ( numPolyPoints=0, i=0; i<numPointsToAdd; i++)
        {
        newPolygon[numPolyPoints++] = fullPoly[(mL+i)%numFullPts];
        }
      newPolys->InsertNextCell(numPolyPoints,newPolygon);
      newScalars->InsertTuple1(cellId++, 
                               this->ComputeLowerScalarIndex(s[idx]));

      //We've got an edge (mL,mR) that marks the edge of the region not yet
      //clipped. We move this edge forward from intersection point to
      //interection point.
      m2R = mR;
      m2L = mL;
      while ( m2R != m2L )
        {
        numPointsToAdd = (mL > mR ? mL-mR+1 : numFullPts-(mR-mL)+1);
        if ( numPointsToAdd <= 3 )
          {//just a triangle left
          for (numPolyPoints=0, i=0; i<numPointsToAdd; i++)
            {
            newPolygon[i] = fullPoly[(mR+i)%numFullPts];
            }
          newPolys->InsertNextCell(numPointsToAdd,newPolygon);
          newScalars->InsertTuple1(cellId++,
                      this->ComputeLowerScalarIndex(s[mR]));
//                      this->ComputeIndex(s[mR],s[(mR+1)%numFullPts]));
          break;
          }
        else //find the next intersection points
          {
          numLeftPointsToAdd = 0;
          numRightPointsToAdd = 0;
          for ( intersectionPoint=0; 
                !intersectionPoint && ((m2R+1)%numFullPts) != m2L; )
            {
            numRightPointsToAdd++;
            m2R = (m2R + 1) % numFullPts;
            if ( isContourValue[m2R] ) intersectionPoint = 1;
            }
          for ( intersectionPoint=0; 
                !intersectionPoint && ((m2L+numFullPts-1)%numFullPts) != m2R; )
            {
            numLeftPointsToAdd++;
            m2L = (m2L + numFullPts - 1) % numFullPts;
            if ( isContourValue[m2L] ) intersectionPoint = 1;
            }

          //specify the polygon vertices. From m2L to mL, then mR to m2R.
          for ( numPolyPoints=0, i=0; i<numLeftPointsToAdd; i++)
            {
            newPolygon[numPolyPoints++] = fullPoly[(m2L+i)%numFullPts];
            }
          newPolygon[numPolyPoints++] = fullPoly[mL];
          newPolygon[numPolyPoints++] = fullPoly[mR];
          for ( i=1; i<=numRightPointsToAdd; i++)
            {
            newPolygon[numPolyPoints++] = fullPoly[(mR+i)%numFullPts];
            }

          //add the polygon
          newPolys->InsertNextCell(numPolyPoints,newPolygon);
          newScalars->InsertTuple1(cellId++,
                                   this->ComputeLowerScalarIndex(s[mR]));
//          newScalars->InsertTuple1(cellId++,this->ComputeIndex(s[mR],s[m2R]));
          mL = m2L;
          mR = m2R;
          }//add a polygon
        }//while still removing polygons
      }//for all polygons

    delete [] s;
    delete [] edgeInts;
    delete [] newPolygon;
    delete [] isContourValue;
    delete [] isOriginalVertex;

    edgeTable->Delete();
    intList->Delete();
    output->SetPolys(newPolys);
    newPolys->Delete();
    if ( tmpPolys ) {tmpPolys->Delete(); }
    }//for all polygons (and strips) in input
  
  vtkDebugMacro(<<"Created " << cellId << " total cells\n");
  vtkDebugMacro(<<"Created " << output->GetVerts()->GetNumberOfCells() 
                << " verts\n");
  vtkDebugMacro(<<"Created " << output->GetLines()->GetNumberOfCells() 
                << " lines\n");
  vtkDebugMacro(<<"Created " << output->GetPolys()->GetNumberOfCells() 
                << " polys\n");
  vtkDebugMacro(<<"Created " << output->GetStrips()->GetNumberOfCells() 
                << " strips\n");

  //  Update ourselves and release temporary memory
  //
  output->SetPoints(newPts);
  newPts->Delete();

  outCD->SetScalars(newScalars);
  newScalars->Delete();
  
  output->Squeeze();
}


unsigned long int vtkBandedPolyDataContourFilter::GetMTime()
{
  unsigned long mTime=this-> vtkPolyDataToPolyDataFilter::GetMTime();
  unsigned long time;

  time = this->ContourValues->GetMTime();
  mTime = ( time > mTime ? time : mTime );
    
  return mTime;
}

void vtkBandedPolyDataContourFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  this->ContourValues->PrintSelf(os,indent);
  os << indent << "Clipping: " << (this->Clipping ? "On\n" : "Off\n");
  
}


