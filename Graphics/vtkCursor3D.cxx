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
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.


=========================================================================*/
#include <math.h>
#include "Cursor3D.hh"

// Description:
// Construct with model bounds = (-1,1,-1,1,-1,1), focal point = (0,0,0),
// all parts of cursor visible, and wrapping off.
vtkCursor3D::vtkCursor3D()
{
  this->ModelBounds[0] = -1.0;
  this->ModelBounds[1] = 1.0;
  this->ModelBounds[2] = -1.0;
  this->ModelBounds[3] = 1.0;
  this->ModelBounds[4] = -1.0;
  this->ModelBounds[5] = 1.0;

  this->FocalPoint[0] = 0.0;  
  this->FocalPoint[1] = 0.0;  
  this->FocalPoint[2] = 0.0;

  this->Outline = 1;
  this->Axes = 1;
  this->XShadows = 1;
  this->YShadows = 1;
  this->ZShadows = 1;
  this->Wrap = 0;
}

void vtkCursor3D::Execute()
{
  int i;
  int numPts=0, numLines=0;
  vtkFloatPoints *newPts;
  vtkCellArray *newLines;
  float x[3];
  int ptIds[2];

  vtkDebugMacro(<<"Generating cursor");
  this->Initialize();
//
// Check bounding box and origin
//
  if ( this->Wrap ) 
    {
    for (i=0; i<3; i++)
      {
      this->FocalPoint[i] = ModelBounds[2*i] + 
             fmod((double)(this->FocalPoint[i]-ModelBounds[2*i]), 
                  (double)(ModelBounds[2*i+1]-ModelBounds[2*i]));
      }
    } 
  else 
    {
    for (i=0; i<3; i++)
      {
      if ( this->FocalPoint[i] < this->ModelBounds[2*i] )
        this->FocalPoint[i] = this->ModelBounds[2*i];
      if ( this->FocalPoint[i] > this->ModelBounds[2*i+1] )
        this->FocalPoint[i] = this->ModelBounds[2*i+1];
      }
    }
//
// Allocate storage
//
  if (this->Axes) 
    {
    numPts += 6;
    numLines += 3;
    }

  if (this->Outline) 
    {
    numPts += 8;
    numLines += 12;
    }

  if (this->XShadows) 
    {
    numPts += 8;
    numLines += 4;
    }

  if (this->YShadows) 
    {
    numPts += 8;
    numLines += 4;
    }

  if (this->ZShadows) 
    {
    numPts += 8;
    numLines += 4;
    }

  if ( numPts ) 
    {
    newPts = new vtkFloatPoints(numPts);
    newLines = new vtkCellArray();
    newLines->Allocate(newLines->EstimateSize(numLines,2));
    }
  else return;
//
// Create axes
//
  if ( this->Axes ) 
    {
    x[0] = this->ModelBounds[0]; 
    x[1] = this->FocalPoint[1]; 
    x[2] = this->FocalPoint[2];
    ptIds[0] = newPts->InsertNextPoint(x);

    x[0] = this->ModelBounds[1]; 
    x[1] = this->FocalPoint[1]; 
    x[2] = this->FocalPoint[2];
    ptIds[1] = newPts->InsertNextPoint(x);
    newLines->InsertNextCell(2,ptIds);

    x[0] = this->FocalPoint[0]; 
    x[1] = this->ModelBounds[2]; 
    x[2] = this->FocalPoint[2];
    ptIds[0] = newPts->InsertNextPoint(x);

    x[0] = this->FocalPoint[0]; 
    x[1] = this->ModelBounds[3]; 
    x[2] = this->FocalPoint[2];
    ptIds[1] = newPts->InsertNextPoint(x);
    newLines->InsertNextCell(2,ptIds);

    x[0] = this->FocalPoint[0]; 
    x[1] = this->FocalPoint[1]; 
    x[2] = this->ModelBounds[4]; 
    ptIds[0] = newPts->InsertNextPoint(x);

    x[0] = this->FocalPoint[0]; 
    x[1] = this->FocalPoint[1]; 
    x[2] = this->ModelBounds[5];
    ptIds[1] = newPts->InsertNextPoint(x);
    newLines->InsertNextCell(2,ptIds);
    }
//
// Create outline
//
  if ( this->Outline ) 
    {
    x[0] = this->ModelBounds[0]; 
    x[1] = this->ModelBounds[2]; 
    x[2] = this->ModelBounds[4];
    ptIds[0] = newPts->InsertNextPoint(x);

    x[0] = this->ModelBounds[1]; 
    x[1] = this->ModelBounds[2]; 
    x[2] = this->ModelBounds[4];
    ptIds[1] = newPts->InsertNextPoint(x);
    newLines->InsertNextCell(2,ptIds);

    x[0] = this->ModelBounds[0]; 
    x[1] = this->ModelBounds[3]; 
    x[2] = this->ModelBounds[4];
    ptIds[0] = newPts->InsertNextPoint(x);

    x[0] = this->ModelBounds[1]; 
    x[1] = this->ModelBounds[3]; 
    x[2] = this->ModelBounds[4];
    ptIds[1] = newPts->InsertNextPoint(x);
    newLines->InsertNextCell(2,ptIds);

    x[0] = this->ModelBounds[0]; 
    x[1] = this->ModelBounds[2]; 
    x[2] = this->ModelBounds[5];
    ptIds[0] = newPts->InsertNextPoint(x);

    x[0] = this->ModelBounds[1]; 
    x[1] = this->ModelBounds[2]; 
    x[2] = this->ModelBounds[5];
    ptIds[1] = newPts->InsertNextPoint(x);
    newLines->InsertNextCell(2,ptIds);

    x[0] = this->ModelBounds[0]; 
    x[1] = this->ModelBounds[3]; 
    x[2] = this->ModelBounds[5];
    ptIds[0] = newPts->InsertNextPoint(x);

    x[0] = this->ModelBounds[1]; 
    x[1] = this->ModelBounds[3]; 
    x[2] = this->ModelBounds[5];
    ptIds[1] = newPts->InsertNextPoint(x);
    newLines->InsertNextCell(2,ptIds);
    }
//
// Create x-shadows
//
  if ( this->XShadows ) 
    {
    for (i=0; i<2; i++) 
      {
      x[0] = this->ModelBounds[i]; 
      x[1] = this->ModelBounds[2]; 
      x[2] = this->FocalPoint[2];
      ptIds[0] = newPts->InsertNextPoint(x);

      x[0] = this->ModelBounds[i]; 
      x[1] = this->ModelBounds[3]; 
      x[2] = this->FocalPoint[2];
      ptIds[1] = newPts->InsertNextPoint(x);
      newLines->InsertNextCell(2,ptIds);

      x[0] = this->ModelBounds[i]; 
      x[1] = this->FocalPoint[1]; 
      x[2] = this->ModelBounds[4];
      ptIds[0] = newPts->InsertNextPoint(x);

      x[0] = this->ModelBounds[i]; 
      x[1] = this->FocalPoint[1]; 
      x[2] = this->ModelBounds[5];
      ptIds[1] = newPts->InsertNextPoint(x);
      newLines->InsertNextCell(2,ptIds);
      }
    }
//
//  Create y-shadows
//
  if ( this->YShadows ) 
    {
    for (i=0; i<2; i++) 
      {
      x[0] = this->ModelBounds[0]; 
      x[1] = this->ModelBounds[i+2]; 
      x[2] = this->FocalPoint[2];
      ptIds[0] = newPts->InsertNextPoint(x);

      x[0] = this->ModelBounds[1]; 
      x[1] = this->ModelBounds[i+2]; 
      x[2] = this->FocalPoint[2];
      ptIds[1] = newPts->InsertNextPoint(x);
      newLines->InsertNextCell(2,ptIds);

      x[0] = this->FocalPoint[0]; 
      x[1] = this->ModelBounds[i+2]; 
      x[2] = this->ModelBounds[4];
      ptIds[0] = newPts->InsertNextPoint(x);

      x[0] = this->FocalPoint[0]; 
      x[1] = this->ModelBounds[i+2]; 
      x[2] = this->ModelBounds[5];
      ptIds[1] = newPts->InsertNextPoint(x);
      newLines->InsertNextCell(2,ptIds);
      }
    }
//
//  Create z-shadows
//
  if ( this->ZShadows ) 
    {
    for (i=0; i<2; i++) 
      {
      x[0] = this->ModelBounds[0]; 
      x[1] = this->FocalPoint[1]; 
      x[2] = this->ModelBounds[i+4]; 
      ptIds[0] = newPts->InsertNextPoint(x);

      x[0] = this->ModelBounds[1]; 
      x[1] = this->FocalPoint[1]; 
      x[2] = this->ModelBounds[i+4]; 
      ptIds[1] = newPts->InsertNextPoint(x);
      newLines->InsertNextCell(2,ptIds);

      x[0] = this->FocalPoint[0]; 
      x[1] = this->ModelBounds[2]; 
      x[2] = this->ModelBounds[i+4]; 
      ptIds[0] = newPts->InsertNextPoint(x);

      x[0] = this->FocalPoint[0]; 
      x[1] = this->ModelBounds[3]; 
      x[2] = this->ModelBounds[i+4];
      ptIds[1] = newPts->InsertNextPoint(x);
      newLines->InsertNextCell(2,ptIds);
      }
    }
//
// Update ourselves and release memory
//
  this->SetPoints(newPts);
  newPts->Delete();

  this->SetLines(newLines);
  newLines->Delete();
}

// Description:
// Set the boundary of the 3D cursor.
void vtkCursor3D::SetModelBounds(float xmin, float xmax, float ymin, float ymax,
                                float zmin, float zmax)
{
  if ( xmin != this->ModelBounds[0] || xmax != this->ModelBounds[1] ||
  ymin != this->ModelBounds[2] || ymax != this->ModelBounds[3] ||
  zmin != this->ModelBounds[4] || zmax != this->ModelBounds[5] )
    {
    this->Modified();

    this->ModelBounds[0] = xmin; this->ModelBounds[1] = xmax; 
    this->ModelBounds[2] = xmin; this->ModelBounds[3] = xmax; 
    this->ModelBounds[4] = xmin; this->ModelBounds[5] = xmax; 

    for (int i=0; i<3; i++)
      if ( this->ModelBounds[2*i] > this->ModelBounds[2*i+1] )
         this->ModelBounds[2*i] = this->ModelBounds[2*i+1];
    }
}

void vtkCursor3D::SetModelBounds(float *bounds)
{
  this->SetModelBounds(bounds[0], bounds[1], bounds[2], bounds[3], bounds[4],
                       bounds[5]);
}

void vtkCursor3D::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolySource::PrintSelf(os,indent);

  os << indent << "ModelBounds: \n";
  os << indent << "  Xmin,Xmax: (" << this->ModelBounds[0] << ", " << this->ModelBounds[1] << ")\n";
  os << indent << "  Ymin,Ymax: (" << this->ModelBounds[2] << ", " << this->ModelBounds[3] << ")\n";
  os << indent << "  Zmin,Zmax: (" << this->ModelBounds[4] << ", " << this->ModelBounds[5] << ")\n";

  os << indent << "Focal Point: (" << this->FocalPoint[0] << ", "
               << this->FocalPoint[1] << ", "
               << this->FocalPoint[2] << ")\n";

  os << indent << "Outline: " << (this->Outline ? "On\n" : "Off\n");
  os << indent << "Axes: " << (this->Axes ? "On\n" : "Off\n");
  os << indent << "XShadows: " << (this->XShadows ? "On\n" : "Off\n");
  os << indent << "YShadows: " << (this->YShadows ? "On\n" : "Off\n");
  os << indent << "ZShadows: " << (this->ZShadows ? "On\n" : "Off\n");
  os << indent << "Wrap: " << (this->Wrap ? "On\n" : "Off\n");
}
