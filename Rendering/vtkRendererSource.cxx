/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Library. No part of this file or its
contents may be copied, reproduced or altered in any way without the express
written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994

=========================================================================*/
#include "RenSrc.hh"
#include "RenderW.hh"
#include "Pixmap.hh"

vlRendererSource::vlRendererSource()
{
  this->Input = NULL;
}

void vlRendererSource::Execute()
{
  int i, numPts, numOutPts;
  vlPixmap *outScalars;
  float x1,y1,x2,y2;
  unsigned char *pixels;
  vlRenderer *input = (vlRenderer *)this->Input;

  vlDebugMacro(<<"Converting points");
  this->Initialize();

  if (this->Input == NULL )
    {
    vlErrorMacro(<<"Please specify a renderer as input!");
    return;
    }

  // calc the pixel range for the renderer
  this->Input->SetViewPoint(-1,-1,0);
  this->Input->ViewToDisplay();
  x1 = this->Input->GetDisplayPoint()[0];
  y1 = this->Input->GetDisplayPoint()[1];
  this->Input->SetViewPoint(1,1,0);
  this->Input->ViewToDisplay();
  x2 = this->Input->GetDisplayPoint()[0];
  y2 = this->Input->GetDisplayPoint()[1];

  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;
  if (x2 >= (this->Input->GetRenderWindow())->GetSize()[0]) 
    {
    x2 = (this->Input->GetRenderWindow())->GetSize()[0] - 1;
    }
  if (y2 >= (this->Input->GetRenderWindow())->GetSize()[1]) 
    {
    y2 = (this->Input->GetRenderWindow())->GetSize()[1] - 1;
    }

  // Get origin, aspect ratio and dimensions from this->Input
  this->SetDimensions((int)(x2 - x1 + 1),(int)(y2 -y1 + 1),1);
  this->SetAspectRatio(1,1,1);
  this->SetOrigin(0,0,0);

  // Allocate data.  Scalar type is FloatScalars.
  numOutPts = this->Dimensions[0] * this->Dimensions[1];
  outScalars = new vlPixmap;

  pixels = (this->Input->GetRenderWindow())->GetPixelData((int)x1,(int)y1,
							  (int)x2,(int)y2);

  // copy scalars over
  memcpy(outScalars->WritePtr(0,numOutPts),pixels,3*numOutPts);

  // Update ourselves
  this->PointData.SetScalars(outScalars);

  // free the memory
  delete [] pixels;
}

void vlRendererSource::PrintSelf(ostream& os, vlIndent indent)
{
  vlStructuredPointsSource::PrintSelf(os,indent);

  if ( this->Input )
    {
    os << indent << "Input:\n";
    this->Input->PrintSelf(os,indent.GetNextIndent());
    }
  else
    {
    os << indent << "Input: (none)\n";
    }
}
