/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Library. No part of this file
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
#include "vlMath.hh"
#include "RibbonF.hh"
#include "FPoints.hh"
#include "FNormals.hh"
#include "PolyLine.hh"

// Description:
// Construct ribbon so that width is 0.1, no normal rotation, the width does 
// not vary with scalar values, and the width factor is 2.0.
vlRibbonFilter::vlRibbonFilter()
{
  this->Width = 0.5;
  this->Angle = 0.0;
  this->VaryWidth = 0;
  this->WidthFactor = 2.0;
}

void vlRibbonFilter::Execute()
{
  int i, j, k;
  vlPoints *inPts;
  vlNormals *inNormals;
  vlPointData *pd;
  vlCellArray *inLines;
  int numNewPts;
  vlFloatPoints *newPts;
  vlFloatNormals *newNormals;
  vlCellArray *newStrips;
  int npts, *pts;
  float p[3], pNext[3];
  float *n;
  float s[3], sNext[3], sPrev[3], w[3];
  double BevelAngle;
  vlMath math;
  float theta;
  int deleteNormals=0, ptId;
  vlPolyData *input=(vlPolyData *)this->Input;
  vlScalars *inScalars=NULL;
  float sFactor=1.0, range[2];
  int ptOffset=0;
//
// Initialize
//
  vlDebugMacro(<<"Creating ribbon");
  this->Initialize();

  if ( !(inPts=input->GetPoints()) || 
  (numNewPts=inPts->GetNumberOfPoints()*2) < 1 ||
  !(inLines = input->GetLines()) || inLines->GetNumberOfCells() < 1 )
    {
    vlErrorMacro(<< ": No input data!\n");
    return;
    }

  // copy scalars, vectors, tcoords. Normals may be computed here.
  pd = input->GetPointData();
  this->PointData.CopyNormalsOff();
  this->PointData.CopyAllocate(pd,numNewPts);

  if ( !(inNormals=pd->GetNormals()) )
    {
    vlPolyLine lineNormalGenerator;
    deleteNormals = 1;
    inNormals = new vlFloatNormals(numNewPts);
    if ( !lineNormalGenerator.GenerateSlidingNormals(inPts,inLines,(vlFloatNormals*)inNormals) )
      {
      vlErrorMacro(<< "No normals for line!\n");
      delete inNormals;
      return;
      }
    }
//
// If varying width, get appropriate info.
//
  if ( this->VaryWidth && (inScalars=pd->GetScalars()) )
    {
    inScalars->GetRange(range);
    }

  newPts = new vlFloatPoints(numNewPts);
  newNormals = new vlFloatNormals(numNewPts);
  newStrips = new vlCellArray;
  newStrips->Allocate(newStrips->EstimateSize(1,numNewPts));
//
//  Create pairs of points along the line that are later connected into a 
//  triangle strip.
//
  theta = this->Angle * math.DegreesToRadians();
  for (inLines->InitTraversal(); inLines->GetNextCell(npts,pts); )
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
        for (i=0; i<3; i++) p[i] = pNext[i];
        inPts->GetPoint(pts[j+1],pNext);
        for (i=0; i<3; i++)
          {
          sPrev[i] = sNext[i];
          sNext[i] = pNext[i] - p[i];
          }
        }

      n = inNormals->GetNormal(pts[j]);

      if ( math.Normalize(sNext) == 0.0 )
        {
        vlErrorMacro(<<"Coincident points!");
        return;
        }

      for (i=0; i<3; i++) s[i] = (sPrev[i] + sNext[i]) / 2.0; //average vector
      math.Normalize(s);
      
      if ( (BevelAngle = math.Dot(sNext,sPrev)) > 1.0 ) BevelAngle = 1.0;
      if ( BevelAngle < -1.0 ) BevelAngle = -1.0;
      BevelAngle = acos((double)BevelAngle) / 2.0; //(0->90 degrees)
      if ( (BevelAngle = cos(BevelAngle)) == 0.0 ) BevelAngle = 1.0;

      BevelAngle = this->Width / BevelAngle;

      math.Cross(s,n,w);
      if ( math.Normalize(w) == 0.0)
        {
        vlErrorMacro(<<"Bad normal!");
        return;
        }
      
      if ( inScalars )
        sFactor = 1.0 + ((this->WidthFactor - 1.0) * 
                  (inScalars->GetScalar(j) - range[0]) / (range[1]-range[0]));

      for (i=0; i<3; i++) w[i] = p[i] + w[i] * BevelAngle * sFactor;
      ptId = newPts->InsertNextPoint(w);
      newNormals->InsertNormal(ptId,n);
      this->PointData.CopyData(pd,pts[j],ptId);

      for (i=0; i<3; i++) w[i] = p[i] - w[i] * BevelAngle * sFactor;
      ptId = newPts->InsertNextPoint(w);
      newNormals->InsertNormal(ptId,n);
      this->PointData.CopyData(pd,pts[j],ptId);
      }
//
// Generate the strip topology
//
    newStrips->InsertNextCell(npts*2);
    for (i=0; i < npts; i++) 
      {
      newStrips->InsertCellPoint(ptOffset+2*i);
      newStrips->InsertCellPoint(ptOffset+2*i+1);
      }
    
    ptOffset += npts*2;
    } //for this line
//
// Update ourselves
//
  if ( deleteNormals ) delete inNormals;

  this->SetPoints(newPts);

  this->SetStrips(newStrips);
  this->PointData.SetNormals(newNormals);
  this->Squeeze();
}

void vlRibbonFilter::PrintSelf(ostream& os, vlIndent indent)
{
  vlPolyToPolyFilter::PrintSelf(os,indent);

  os << indent << "Width: " << this->Width << "\n";
  os << indent << "Angle: " << this->Angle << "\n";
  os << indent << "VaryWidth: " << (this->VaryWidth ? "On\n" : "Off\n");
  os << indent << "Width Factor: " << this->WidthFactor << "\n";
}

