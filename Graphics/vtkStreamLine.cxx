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
#include "vtkStreamLine.hh"

vtkStreamLine::vtkStreamLine()
{
  this->StepLength = 0.01;
}

void vtkStreamLine::Execute()
{
  vtkStreamPoint *sPrev, *sPtr;
  vtkFloatPoints *newPts;
  vtkFloatVectors *newVectors;
  vtkFloatScalars *newScalars=NULL;
  vtkCellArray *newLines;
  int i, ptId, j, id;
  vtkIdList pts(2500);
  float tOffset, x[3], v[3], s, r;
  vtkPolyData *output=(vtkPolyData *)this->Output;

  this->vtkStreamer::Integrate();
  if ( this->NumberOfStreamers <= 0 ) return;
//
//  Convert streamer into lines. Lines may be dashed.
//
  newPts  = new vtkFloatPoints(1000);
  newVectors  = new vtkFloatVectors(1000);
  if ( this->Input->GetPointData()->GetScalars() || this->SpeedScalars )
    newScalars = new vtkFloatScalars(1000);
  newLines = new vtkCellArray();
  newLines->Allocate(newLines->EstimateSize(2*this->NumberOfStreamers,VTK_CELL_SIZE));
//
// Loop over all streamers generating points
//
  for (ptId=0; ptId < this->NumberOfStreamers; ptId++)
    {
    if ( this->Streamers[ptId].GetNumberOfPoints() < 2 ) continue;

    sPrev = this->Streamers[ptId].GetStreamPoint(0);

    if ( this->Streamers[ptId].GetNumberOfPoints() == 2 )
      {
      sPtr = this->Streamers[ptId].GetStreamPoint(1);
      if ( sPtr->cellId < 0 ) continue;
      }

    tOffset = sPrev->t;

    for ( i=0, sPtr=sPrev; 
    i < this->Streamers[ptId].GetNumberOfPoints() && sPtr->cellId >= 0;
    i++, sPrev=sPtr, sPtr=this->Streamers[ptId].GetStreamPoint(i) )
      {
//
// Search for end of segment and create line segments
//
      if ( (sPtr->t - tOffset) > this->StepLength )
        {
        while ( tOffset < sPtr->t )
          {
          r = (tOffset - sPrev->t) / (sPtr->t - sPrev->t);

          for (j=0; j<3; j++)
            {
            x[j] = sPrev->x[j] + r * (sPtr->x[j] - sPrev->x[j]);
            v[j] = sPrev->v[j] + r * (sPtr->v[j] - sPrev->v[j]);
            }

          // add point to line
          id = newPts->InsertNextPoint(x);
          pts.InsertNextId(id);
          newVectors->InsertVector(id,v);

          if ( newScalars ) 
            {
            s = sPrev->s + r * (sPtr->s - sPrev->s);
            newScalars->InsertScalar(id,s);
            }
  
          tOffset += this->StepLength;
          } // while

        } //if points should be created
      } //for this streamer

    if ( pts.GetNumberOfIds() > 1 )
      {
      newLines->InsertNextCell(pts);
      pts.Reset();
      }
    } //for all streamers
//
// Update ourselves
//
  vtkDebugMacro(<<"Created " << newPts->GetNumberOfPoints() << " points, "
               << newLines->GetNumberOfCells() << " lines");

  output->SetPoints(newPts);
  newPts->Delete();

  output->GetPointData()->SetVectors(newVectors);
  newVectors->Delete();

  if ( newScalars ) 
    {
    output->GetPointData()->SetScalars(newScalars);
    newScalars->Delete();
    }

  output->SetLines(newLines);
  newLines->Delete();

  output->Squeeze();
}

void vtkStreamLine::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkStreamer::PrintSelf(os,indent);

  os << indent << "Step Length: " << this->StepLength << " <<\n";

}


