/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen.

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
#include "vtkOutlineSource.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkOutlineSource* vtkOutlineSource::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkOutlineSource");
  if(ret)
    {
    return (vtkOutlineSource*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkOutlineSource;
}




//----------------------------------------------------------------------------
vtkOutlineSource::vtkOutlineSource()
{
  for (int i=0; i<3; i++) 
    {
    this->Bounds[2*i] = -1.0;
    this->Bounds[2*i+1] = 1.0;
    }
}

//----------------------------------------------------------------------------
void vtkOutlineSource::Execute()
{
  float *bounds;
  float x[3];
  int pts[2];
  vtkPoints *newPts;
  vtkCellArray *newLines;
  vtkPolyData *output = this->GetOutput();
  
  vtkDebugMacro(<< "Generating outline");
  //
  // Initialize
  //
  bounds = this->Bounds;
  //
  // Allocate storage and create outline
  //
  newPts = vtkPoints::New();
  newPts->Allocate(8);
  newLines = vtkCellArray::New();
  newLines->Allocate(newLines->EstimateSize(12,2));

  x[0] = bounds[0]; x[1] = bounds[2]; x[2] = bounds[4];
  newPts->InsertPoint(0,x);
  x[0] = bounds[1]; x[1] = bounds[2]; x[2] = bounds[4];
  newPts->InsertPoint(1,x);
  x[0] = bounds[0]; x[1] = bounds[3]; x[2] = bounds[4];
  newPts->InsertPoint(2,x);
  x[0] = bounds[1]; x[1] = bounds[3]; x[2] = bounds[4];
  newPts->InsertPoint(3,x);
  x[0] = bounds[0]; x[1] = bounds[2]; x[2] = bounds[5];
  newPts->InsertPoint(4,x);
  x[0] = bounds[1]; x[1] = bounds[2]; x[2] = bounds[5];
  newPts->InsertPoint(5,x);
  x[0] = bounds[0]; x[1] = bounds[3]; x[2] = bounds[5];
  newPts->InsertPoint(6,x);
  x[0] = bounds[1]; x[1] = bounds[3]; x[2] = bounds[5];
  newPts->InsertPoint(7,x);

  pts[0] = 0; pts[1] = 1;
  newLines->InsertNextCell(2,pts);
  pts[0] = 2; pts[1] = 3;
  newLines->InsertNextCell(2,pts);
  pts[0] = 4; pts[1] = 5;
  newLines->InsertNextCell(2,pts);
  pts[0] = 6; pts[1] = 7;
  newLines->InsertNextCell(2,pts);
  pts[0] = 0; pts[1] = 2;
  newLines->InsertNextCell(2,pts);
  pts[0] = 1; pts[1] = 3;
  newLines->InsertNextCell(2,pts);
  pts[0] = 4; pts[1] = 6;
  newLines->InsertNextCell(2,pts);
  pts[0] = 5; pts[1] = 7;
  newLines->InsertNextCell(2,pts);
  pts[0] = 0; pts[1] = 4;
  newLines->InsertNextCell(2,pts);
  pts[0] = 1; pts[1] = 5;
  newLines->InsertNextCell(2,pts);
  pts[0] = 2; pts[1] = 6;
  newLines->InsertNextCell(2,pts);
  pts[0] = 3; pts[1] = 7;
  newLines->InsertNextCell(2,pts);
  //
  // Update selves and release memory
  //
  output->SetPoints(newPts);
  newPts->Delete();

  output->SetLines(newLines);
  newLines->Delete();
}


//----------------------------------------------------------------------------
void vtkOutlineSource::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolyDataSource::PrintSelf(os,indent);

  os << indent << "Bounds: (" << this->Bounds[0] << ", " 
     << this->Bounds[1] << ") (" << this->Bounds[2] << ") ("
     << this->Bounds[3] << ") (" << this->Bounds[4] << ") ("
     << this->Bounds[5] << ")\n";
}
