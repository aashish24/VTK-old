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
#include "vtkPolyLine.h"
#include "vtkMath.h"
#include "vtkCellArray.h"

vtkCell *vtkPolyLine::MakeObject()
{
  vtkCell *cell = vtkPolyLine::New();
  cell->DeepCopy(*this);
  return cell;
}

// Description:
// Given points and lines, compute normals to lines.
int vtkPolyLine::GenerateNormals(vtkPoints *pts, vtkCellArray *lines, vtkNormals *normals)
{
  int npts, *linePts;
  float s[3], sPrev[3], sNext[3], norm[3], *n, *nPrev;
  float *p, *pPrev, *pNext;
  float n_norm;
  int i, j, fillIn;
  int aNormalComputed, *normalComputed;
//
//  Loop over all lines. 
// 
  for (lines->InitTraversal(); lines->GetNextCell(npts,linePts); )
    {

    // check input
    if ( npts < 1 )
      {
      vtkErrorMacro(<<"Line with no points!");
      return 0;
      }

    else if ( npts == 1 ) //return arbitrary normal
      {
      norm[0] = norm[1] = 0.0;
      norm[2] = 1.0;
      normals->InsertNormal(linePts[0],norm);
      return 1;
      }

    else if ( npts == 2 ) //simple line; directly compute
      {
      pPrev = pts->GetPoint(linePts[0]);
      p = pts->GetPoint(linePts[1]);

      for (i=0; i<3; i++) s[i] = p[i] - pPrev[i];

      if ( (n_norm = vtkMath::Norm(s)) == 0.0 ) //use arbitrary normal
        {
        norm[0] = norm[1] = 0.0;
        norm[2] = 1.0;
        }

      else //else compute normal
        {
        for (i=0; i<3; i++) 
          {
          if ( s[i] != 0.0 ) 
            {
            norm[(i+2)%3] = 0.0;
            norm[(i+1)%3] = 1.0;
            norm[i] = -s[(i+1)%3]/s[i];
            break;
            }
          }
        n_norm = vtkMath::Norm(norm);
        for (i=0; i<3; i++) norm[i] /= n_norm;
        }

      normals->InsertNormal(linePts[0],norm);
      normals->InsertNormal(linePts[1],norm);
      return 1;
      }
//
//  Else have polyline. Initialize normal computation.
//
    normalComputed = new int[npts+1];
    for (i=0; i<=npts; i++) normalComputed[i] = 0;
    p = pts->GetPoint(linePts[0]);
    pNext = pts->GetPoint(linePts[1]);

    // perform cross products along line
    for (aNormalComputed=0, j=1; j < (npts-1); j++) 
      {
      pPrev = p;
      p = pNext;
      pNext = pts->GetPoint(linePts[j+1]);

      for (i=0; i<3; i++) 
        {
        sPrev[i] = p[i] - pPrev[i];
        sNext[i] = pNext[i] - p[i];
        }

      vtkMath::Cross(sPrev,sNext,norm);
      if ( (n_norm = vtkMath::Norm(norm)) != 0.0 ) //okay to use
        {
        for (i=0; i<3; i++) norm[i] /= n_norm;
        normalComputed[j] = aNormalComputed = 1;
        normals->InsertNormal(linePts[j],norm);
        }
      }
//
//  If no normal computed, must be straight line of points. Find one normal.
//
    if ( ! aNormalComputed )
      {
      for (j=1; j < npts; j++) 
        {
        pPrev = pts->GetPoint(linePts[j-1]);
        p = pts->GetPoint(linePts[j]);

        for (i=0; i<3; i++) s[i] = p[i] - pPrev[i];

        if ( (n_norm = vtkMath::Norm(s)) != 0.0 ) //okay to use
          {
          aNormalComputed = 1;

          for (i=0; i<3; i++) 
            {
            if ( s[i] != 0.0 ) 
              {
              norm[(i+2)%3] = 0.0;
              norm[(i+1)%3] = 1.0;
              norm[i] = -s[(i+1)%3]/s[i];
              break;
              }
            }
          n_norm = vtkMath::Norm(norm);
          for (i=0; i<3; i++) norm[i] /= n_norm;

          break;
          }
        }

      if ( ! aNormalComputed ) // must be a bunch of coincident points
        {
        norm[0] = norm[1] = 0.0;
        norm[2] = 1.0;
        }

      for (j=0; j<npts; j++) normals->InsertNormal(linePts[j],norm);
      return 1;
      }
//
//  Fill in normals (from neighbors)
//
    for (fillIn=1; fillIn ; )
      {
      for (fillIn=0, j=0; j<npts; j++)
        {
        if ( ! normalComputed[j] )
          {
          if ( (j+1) < npts && normalComputed[j+1] )
            {
            fillIn = 1;
            normals->InsertNormal(linePts[j],normals->GetNormal(j+1));
            normalComputed[j] = 1;
            }
          else if ( (j-1) >= 0 && normalComputed[j-1] )
            {
            fillIn = 1;
            normals->InsertNormal(linePts[j],normals->GetNormal(j-1));
            normalComputed[j] = 1;
            }
          }
        }
      }
    delete [] normalComputed;
//
//  Check that normals don't flip around wildly.
//
    n = normals->GetNormal(linePts[0]);
    for (j=1; j<npts; j++)
      {
      nPrev = n;
      n = normals->GetNormal(linePts[j]);

      if ( vtkMath::Dot(n,nPrev) < 0.0 ) //reversed sense
        {
        for(i=0; i<3; i++) norm[i] = -n[i];
        normals->InsertNormal(linePts[j],norm);
        }
      }
    }

  return 1;
}

// Description:
// Given points and lines, compute normals to lines. These are not true 
// normals, they are "orientation" normals used by classes like vtkTubeFilter
// that control the rotation around the line. The normals try to stay pointing
// in the same direction as much as possible (i.e., minimal rotation).
int vtkPolyLine::GenerateSlidingNormals(vtkPoints *pts, vtkCellArray *lines, vtkNormals *normals)
{
  int npts, *linePts;
  float sPrev[3], sNext[3], q[3], w[3], normal[3], theta;
  float p[3], pPrev[3], pNext[3];
  float c[3], f1, f2;
  int i, j, largeRotation;
//
//  Loop over all lines
// 
  for (lines->InitTraversal(); lines->GetNextCell(npts,linePts); )
    {
//
//  Determine initial starting normal
// 
    if ( npts <= 0 ) continue;

    else if ( npts == 1 ) //return arbitrary
      {
      normal[0] = normal[1] = 0.0;
      normal[2] = 1.0;
      normals->InsertNormal(linePts[0],normal);
      }

    else //more than one point
      {
//
//  Compute first normal. All "new" normals try to point in the same 
//  direction.
//
      for (j=0; j<npts; j++) 
        {

        if ( j == 0 ) //first point
          {
          pts->GetPoint(linePts[0],p);
          pts->GetPoint(linePts[1],pNext);

          for (i=0; i<3; i++) 
            {
            sPrev[i] = pNext[i] - p[i];
            sNext[i] = sPrev[i];
            }

          if ( vtkMath::Normalize(sNext) == 0.0 )
            {
            vtkErrorMacro(<<"Coincident points in polyline...can't compute normals");
            return 0;
            }

	  // the following logic will produce a normal orthogonal
	  // to the first line segment. If we have three points
	  // we use special logic to select a normal orthogonal
	  // too the first two line segments
	  if (npts > 2)
	    {
	    float ftmp[3];
	    
	    pts->GetPoint(linePts[2],ftmp);
            for (i=0; i<3; i++) 
              {
              ftmp[i] = ftmp[i] - pNext[i];
              }
            if ( vtkMath::Normalize(ftmp) == 0.0 )
              {
              vtkErrorMacro(<<"Coincident points in polyline...can't compute normals");
              return 0;
              }
	    // now the starting normal should simply be the cross product
	    // in the following if statement we check for the case where
	    /// the first three points are colinear 
            vtkMath::Cross(sNext,ftmp,normal);
	    }
          if ((npts <= 2)|| (vtkMath::Normalize(normal) == 0.0)) 
	    {
	    for (i=0; i<3; i++) 
	      {
	      // a little trick to find othogonal normal
	      if ( sNext[i] != 0.0 ) 
		{
		normal[(i+2)%3] = 0.0;
		normal[(i+1)%3] = 1.0;
		normal[i] = -sNext[(i+1)%3]/sNext[i];
		break;
		}
	      }
	    }
          vtkMath::Normalize(normal);
          normals->InsertNormal(linePts[0],normal);
          }

        else if ( j == (npts-1) ) //last point; just insert previous
          {
          normals->InsertNormal(linePts[j],normal);
          }

        else //inbetween points
          {
//
//  Generate normals for new point by projecting previous normal
// 
          for (i=0; i<3; i++)
            {
            pPrev[i] = p[i];
            p[i] = pNext[i];
            }
          pts->GetPoint(linePts[j+1],pNext);

          for (i=0; i<3; i++) 
            {
            sPrev[i] = sNext[i];
            sNext[i] = pNext[i] - p[i];
            }

          if ( vtkMath::Normalize(sNext) == 0.0 )
            {
            vtkErrorMacro(<<"Coincident points in polyline...can't compute normals");
            return 0;
            }

          //compute rotation vector
          vtkMath::Cross(sPrev,normal,w);
          if ( vtkMath::Normalize(w) == 0.0 ) 
	    {
	    vtkErrorMacro(<<"normal and sPrev coincident");
	    return 0;
	    }

          //see whether we rotate greater than 90 degrees.
          if ( vtkMath::Dot(sPrev,sNext) < 0.0 ) largeRotation = 1;
          else largeRotation = 0;

          //compute rotation of line segment
          vtkMath::Cross (sNext, sPrev, q);
          if ( (theta=asin((double)vtkMath::Normalize(q))) == 0.0 ) 
            { //no rotation, use previous normal
            normals->InsertNormal(linePts[j],normal);
            continue;
            }
          if ( largeRotation )
            {
            if ( theta > 0.0 ) theta = vtkMath::Pi() - theta;
            else theta = -vtkMath::Pi() - theta;
            }

	  // new method
          for (i=0; i<3; i++) c[i] = sNext[i] + sPrev[i];
	  vtkMath::Normalize(c);
	  f1 = vtkMath::Dot(q,normal);
	  f2 = 1.0 - f1*f1;
	  if (f2 > 0.0)
	    {
	    f2 = sqrt(1.0 - f1*f1);
	    }
	  else
	    {
	    f2 = 0.0;
	    }
	  vtkMath::Cross(c,q,w);
	  vtkMath::Cross(sPrev,q,c);
	  if (vtkMath::Dot(normal,c)*vtkMath::Dot(w,c) < 0)
	    {
	    f2 = -1.0*f2;
	    }
          for (i=0; i<3; i++) normal[i] = f1*q[i] + f2*w[i];
	  
          normals->InsertNormal(linePts[j],normal);
          }//for this point
        }//else
      }//else if
    }//for this line
  return 1;

}

int vtkPolyLine::EvaluatePosition(float x[3], float closestPoint[3],
                                 int& subId, float pcoords[3], 
                                 float& minDist2, float *weights)
{
  float closest[3];
  float pc[3], dist2;
  int ignoreId, i, return_status, status;
  float lineWeights[2];

  pcoords[1] = pcoords[2] = 0.0;

  return_status = 0;
  weights[0] = 0.0;
  for (minDist2=VTK_LARGE_FLOAT,i=0; i<this->Points.GetNumberOfPoints()-1; i++)
    {
    this->Line.Points.SetPoint(0,this->Points.GetPoint(i));
    this->Line.Points.SetPoint(1,this->Points.GetPoint(i+1));
    status = this->Line.EvaluatePosition(x,closest,ignoreId,pc,dist2,lineWeights);
    if ( status != -1 && dist2 < minDist2 )
      {
      return_status = status;
      closestPoint[0] = closest[0]; closestPoint[1] = closest[1]; closestPoint[2] = closest[2]; 
      subId = i;
      pcoords[0] = pc[0];
      minDist2 = dist2;
      weights[i] = lineWeights[0];
      weights[i+1] = lineWeights[1];
      }
    else
      {
      weights[i+1] = 0.0;
      }
    }

  return return_status;
}

void vtkPolyLine::EvaluateLocation(int& subId, float pcoords[3], float x[3],
                                   float *weights)
{
  int i;
  float *a1 = this->Points.GetPoint(subId);
  float *a2 = this->Points.GetPoint(subId+1);

  for (i=0; i<3; i++) 
    {
    x[i] = a1[i] + pcoords[0]*(a2[i] - a1[i]);
    weights[i] = 0.0;
    }

  weights[subId] = pcoords[0];
  weights[subId+1] = 1.0 - pcoords[0];
}

int vtkPolyLine::CellBoundary(int subId, float pcoords[3], vtkIdList& pts)
{
  pts.SetNumberOfIds(1);

  if ( pcoords[0] >= 0.5 )
    {
    pts.SetId(0,this->PointIds.GetId(subId+1));
    if ( pcoords[0] > 1.0 ) return 0;
    else return 1;
    }
  else
    {
    pts.SetId(0,this->PointIds.GetId(subId));
    if ( pcoords[0] < 0.0 ) return 0;
    else return 1;
    }
}

void vtkPolyLine::Contour(float value, vtkScalars *cellScalars,
                         vtkPointLocator *locator, vtkCellArray *verts, 
                         vtkCellArray *lines, vtkCellArray *polys, 
                         vtkPointData *inPd, vtkPointData *outPd)
{
  int i;
  vtkScalars *lineScalars=vtkScalars::New();
  lineScalars->SetNumberOfScalars(2);

  for ( i=0; i<this->Points.GetNumberOfPoints()-1; i++)
    {
    this->Line.Points.SetPoint(0,this->Points.GetPoint(i));
    this->Line.Points.SetPoint(1,this->Points.GetPoint(i+1));

    if ( outPd )
      {
      this->Line.PointIds.SetId(0,this->PointIds.GetId(i));
      this->Line.PointIds.SetId(1,this->PointIds.GetId(i+1));
      }

    lineScalars->SetScalar(0,cellScalars->GetScalar(i));
    lineScalars->SetScalar(1,cellScalars->GetScalar(i+1));

    this->Line.Contour(value, lineScalars, locator, verts,
		       lines, polys, inPd, outPd);
    }
  lineScalars->Delete();
}

//
// Intersect with sub-lines
//
int vtkPolyLine::IntersectWithLine(float p1[3], float p2[3],float tol,float& t,
                                  float x[3], float pcoords[3], int& subId)
{
  int subTest;

  for (subId=0; subId<this->Points.GetNumberOfPoints()-1; subId++)
    {
    this->Line.Points.SetPoint(0,this->Points.GetPoint(subId));
    this->Line.Points.SetPoint(1,this->Points.GetPoint(subId+1));

    if ( this->Line.IntersectWithLine(p1, p2, tol, t, x, pcoords, subTest) )
      return 1;
    }

  return 0;
}

int vtkPolyLine::Triangulate(int vtkNotUsed(index), vtkIdList &ptIds,
                             vtkPoints &pts)
{
  pts.Reset();
  ptIds.Reset();

  for (int subId=0; subId<this->Points.GetNumberOfPoints()-1; subId++)
    {
    pts.InsertNextPoint(this->Points.GetPoint(subId));
    ptIds.InsertNextId(this->PointIds.GetId(subId));

    pts.InsertNextPoint(this->Points.GetPoint(subId+1));
    ptIds.InsertNextId(this->PointIds.GetId(subId+1));
    }

  return 1;
}

void vtkPolyLine::Derivatives(int subId, float pcoords[3], float *values, 
                              int dim, float *derivs)
{
  this->Line.PointIds.SetNumberOfIds(2);

  this->Line.Points.SetPoint(0,this->Points.GetPoint(subId));
  this->Line.Points.SetPoint(1,this->Points.GetPoint(subId+1));

  this->Line.Derivatives(0, pcoords, values, dim, derivs);
}

void vtkPolyLine::Clip(float value, vtkScalars *cellScalars, 
                       vtkPointLocator *locator, vtkCellArray *lines,
                       vtkPointData *inPd, vtkPointData *outPd,
                       int insideOut)
{
  int i;
  vtkScalars *lineScalars=vtkScalars::New();
  lineScalars->SetNumberOfScalars(2);

  for ( i=0; i < this->Points.GetNumberOfPoints()-1; i++)
    {
    this->Line.Points.SetPoint(0,this->Points.GetPoint(i));
    this->Line.Points.SetPoint(1,this->Points.GetPoint(i+1));

    this->Line.PointIds.SetId(0,this->PointIds.GetId(i));
    this->Line.PointIds.SetId(1,this->PointIds.GetId(i+1));

    lineScalars->SetScalar(0,cellScalars->GetScalar(i));
    lineScalars->SetScalar(1,cellScalars->GetScalar(i+1));

    this->Line.Clip(value, lineScalars, locator, lines, inPd, outPd, insideOut);
    }
  
  lineScalars->Delete();
}
