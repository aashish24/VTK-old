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
#include <math.h>
#include "vtkLineSource.hh"
#include "vtkFloatPoints.hh"
#include "vtkFloatTCoords.hh"

vtkLineSource::vtkLineSource(int res)
{
  this->Pt1[0] = -0.5;
  this->Pt1[1] =  0.0;
  this->Pt1[2] =  0.0;

  this->Pt2[0] =  0.5;
  this->Pt2[1] =  0.0;
  this->Pt2[2] =  0.0;

  this->Resolution = (res < 1 ? 1 : res);
}

void vtkLineSource::Execute()
{
  int numLines=this->Resolution;
  int numPts=this->Resolution+1;
  float x[3], tc[2], v[3];
  int i, j;
  int pts[2];
  vtkFloatPoints *newPoints; 
  vtkFloatTCoords *newTCoords; 
  vtkCellArray *newLines;

  vtkDebugMacro(<<"Creating line");
  this->Initialize();

  newPoints = new vtkFloatPoints(numPts);
  newTCoords = new vtkFloatTCoords(numPts,2);

  newLines = new vtkCellArray;
  newLines->Allocate(newLines->EstimateSize(numLines,2));
//
// Generate points and texture coordinates
//
  for (i=0; i<3; i++) v[i] = this->Pt2[i] - this->Pt1[i];

  tc[1] = 0.0;
  for (i=0; i<numPts; i++) 
    {
    tc[0] = ((float)i/this->Resolution);
    for (j=0; j<3; j++) x[j] = this->Pt1[j] + tc[0]*v[j];
    newPoints->InsertPoint(i,x);
    newTCoords->InsertTCoord(i,tc);
    }
//
//  Generate lines
//
  for (i=0; i < numLines; i++) 
    {
    pts[0] = i;
    pts[1] = i+1;
    newLines->InsertNextCell(2,pts);
    }
//
// Update ourselves and release memory
//
  this->SetPoints(newPoints);
  newPoints->Delete();

  this->PointData.SetTCoords(newTCoords);
  newTCoords->Delete();

  this->SetLines(newLines);
  newLines->Delete();
}

void vtkLineSource::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolySource::PrintSelf(os,indent);

  os << indent << "Resolution: " << this->Resolution << "\n";

  os << indent << "Point 1: (" << this->Pt1[0] << ", "
                                << this->Pt1[1] << ", "
                                << this->Pt1[2] << ")\n";

  os << indent << "Point 2: (" << this->Pt2[0] << ", "
                                << this->Pt2[1] << ", "
                                << this->Pt2[2] << ")\n";
}
