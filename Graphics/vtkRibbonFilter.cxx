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
#include "vtkMath.h"
#include "vtkRibbonFilter.h"
#include "vtkPoints.h"
#include "vtkNormals.h"
#include "vtkPolyLine.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkRibbonFilter* vtkRibbonFilter::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkRibbonFilter");
  if(ret)
    {
    return (vtkRibbonFilter*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkRibbonFilter;
}




// Construct ribbon so that width is 0.1, the width does 
// not vary with scalar values, and the width factor is 2.0.
vtkRibbonFilter::vtkRibbonFilter()
{
  this->Width = 0.5;
  this->Angle = 0.0;
  this->VaryWidth = 0;
  this->WidthFactor = 2.0;

  this->DefaultNormal[0] = this->DefaultNormal[1] = 0.0;
  this->DefaultNormal[2] = 1.0;
  
  this->UseDefaultNormal = 0;
}

void vtkRibbonFilter::Execute()
{
  int i, j;
  vtkPoints *inPts;
  vtkNormals *inNormals;
  vtkPointData *pd, *outPD;
  vtkCellData *cd, *outCD;
  vtkCellArray *inLines;
  int numPts = 0;
  int numNewPts = 0;
  vtkPoints *newPts;
  vtkNormals *newNormals;
  vtkCellArray *newStrips;
  int npts, *pts;
  float p[3], pNext[3];
  float *n;
  float s[3], sNext[3], sPrev[3], w[3];
  double BevelAngle;
  int deleteNormals=0, ptId;
  vtkPolyData *input= this->GetInput();
  vtkPolyData *output= this->GetOutput();
  vtkScalars *inScalars=NULL;
  float sFactor=1.0, range[2];
  int ptOffset=0;
  //
  // Initialize
  //
  vtkDebugMacro(<<"Creating ribbon");

  inPts=input->GetPoints();
  inLines = input->GetLines();
  if ( !inPts || 
       (numNewPts=inPts->GetNumberOfPoints()*2) < 1 ||
       !inLines || inLines->GetNumberOfCells() < 1 )
    {
    vtkErrorMacro(<< "No input data!");
    return;
    }

  numPts = inPts->GetNumberOfPoints();
  
  // copy point scalars, vectors, tcoords. Normals may be computed here.
  pd = input->GetPointData();
  outPD = output->GetPointData();
  outPD->CopyNormalsOff();
  outPD->CopyAllocate(pd,numNewPts);

  // copy point scalars, vectors, tcoords.
  cd = input->GetCellData();
  outCD = output->GetCellData();
  outCD->CopyNormalsOff();
  outCD->CopyAllocate(cd, inLines->GetNumberOfCells());
  int inCellId, outCellId;


  if ( !(inNormals=pd->GetNormals()) || this->UseDefaultNormal )
    {
    vtkPolyLine *lineNormalGenerator = vtkPolyLine::New();
    deleteNormals = 1;
    inNormals = vtkNormals::New();

    ((vtkNormals *)inNormals)->Allocate(numPts);
    if ( this->UseDefaultNormal )
      {
      for ( i=0; i < numPts; i++)
        {
        inNormals->SetNormal(i,this->DefaultNormal);
        }
      }
    else
      {
      if ( !lineNormalGenerator->GenerateSlidingNormals(inPts,inLines,(vtkNormals*)inNormals) )
        {
        vtkErrorMacro(<< "No normals for line!\n");
        inNormals->Delete();
        return;
        }
      }
    lineNormalGenerator->Delete();
    }
  //
  // If varying width, get appropriate info.
  //
  if ( this->VaryWidth && (inScalars=pd->GetScalars()) )
    {
    inScalars->GetRange(range);
    }

  newPts = vtkPoints::New();
  newPts->Allocate(numNewPts);
  newNormals = vtkNormals::New();
  newNormals->Allocate(numNewPts);
  newStrips = vtkCellArray::New();
  newStrips->Allocate(newStrips->EstimateSize(1,numNewPts));
  //
  //  Create pairs of points along the line that are later connected into a 
  //  triangle strip.
  //
  inCellId = 0;
  for (inLines->InitTraversal(); inLines->GetNextCell(npts,pts); ++inCellId)
    {
    //
    // Use "averaged" segment to create beveled effect. Watch out for first and 
    // last points.
    //
    for (j=0; j < npts; j++)
      {

      if ( j == 0 ) //first point
        {
        inPts->GetPoint(pts[0],p);
        inPts->GetPoint(pts[1],pNext);
        for (i=0; i<3; i++) 
          {
          sNext[i] = pNext[i] - p[i];
          sPrev[i] = sNext[i];
          }
        }

      else if ( j == (npts-1) ) //last point
        {
        for (i=0; i<3; i++) 
          {
          sPrev[i] = sNext[i];
          p[i] = pNext[i];
          }
        }

      else
        {
        for (i=0; i<3; i++)
          {
          p[i] = pNext[i];
          }
        inPts->GetPoint(pts[j+1],pNext);
        for (i=0; i<3; i++)
          {
          sPrev[i] = sNext[i];
          sNext[i] = pNext[i] - p[i];
          }
        }

      n = inNormals->GetNormal(pts[j]);

      if ( vtkMath::Normalize(sNext) == 0.0 )
        {
        vtkErrorMacro(<<"Coincident points!");
        return;
        }

      for (i=0; i<3; i++)
        {
        s[i] = (sPrev[i] + sNext[i]) / 2.0; //average vector
        }
      vtkMath::Normalize(s);
      
      if ( (BevelAngle = vtkMath::Dot(sNext,sPrev)) > 1.0 )
        {
        BevelAngle = 1.0;
        }
      if ( BevelAngle < -1.0 )
        {
        BevelAngle = -1.0;
        }
      BevelAngle = acos((double)BevelAngle) / 2.0; //(0->90 degrees)
      if ( (BevelAngle = cos(BevelAngle)) == 0.0 )
        {
        BevelAngle = 1.0;
        }

      BevelAngle = this->Width / BevelAngle;

      vtkMath::Cross(s,n,w);
      if ( vtkMath::Normalize(w) == 0.0)
        {
        vtkErrorMacro(<<"Bad normal!");
        return;
        }
      
      if ( inScalars )
        {
        sFactor = 1.0 + ((this->WidthFactor - 1.0) * 
              (inScalars->GetScalar(pts[j]) - range[0]) / (range[1]-range[0]));
        }
      for (i=0; i<3; i++)
        {
        s[i] = p[i] + w[i] * BevelAngle * sFactor;
        }
      ptId = newPts->InsertNextPoint(s);
      newNormals->InsertNormal(ptId,n);
      outPD->CopyData(pd,pts[j],ptId);

      for (i=0; i<3; i++)
        {
        s[i] = p[i] - w[i] * BevelAngle * sFactor;
        }
      ptId = newPts->InsertNextPoint(s);
      newNormals->InsertNormal(ptId,n);
      outPD->CopyData(pd,pts[j],ptId);
      }
    //
    // Generate the strip topology
    //
    outCellId = newStrips->InsertNextCell(npts*2);
    for (i=0; i < npts; i++) 
      {//order important for consistent normals
      newStrips->InsertCellPoint(ptOffset+2*i+1);
      newStrips->InsertCellPoint(ptOffset+2*i);
      }
    outCD->CopyData(cd,inCellId,outCellId);

    
    ptOffset += npts*2;
    } //for this line
  //
  // Update ourselves
  //
  if ( deleteNormals )
    {
    inNormals->Delete();
    }

  output->SetPoints(newPts);
  newPts->Delete();

  output->SetStrips(newStrips);
  newStrips->Delete();

  outPD->SetNormals(newNormals);
  newNormals->Delete();

  output->Squeeze();
}

void vtkRibbonFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolyDataToPolyDataFilter::PrintSelf(os,indent);

  os << indent << "Width: " << this->Width << "\n";
  os << indent << "Angle: " << this->Angle << "\n";
  os << indent << "VaryWidth: " << (this->VaryWidth ? "On\n" : "Off\n");
  os << indent << "Width Factor: " << this->WidthFactor << "\n";
  os << indent << "Use Default Normal: " << this->UseDefaultNormal << "\n";
  os << indent << "Default Normal: " << "( " << 
	this->DefaultNormal[0] << ", " <<
	this->DefaultNormal[1] << ", " <<
	this->DefaultNormal[2] << " )\n";
}

