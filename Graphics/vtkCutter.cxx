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
#include "vtkCutter.hh"
#include "vtkMergePoints.hh"
#include <math.h>

// Description:
// Construct with user-specified implicit function.
vtkCutter::vtkCutter(vtkImplicitFunction *cf)
{
  this->CutFunction = cf;

  this->Locator = NULL;
  this->SelfCreatedLocator = 0;
}

// Description:
// Overload standard modified time function. If cut functions is modified,
// then this object is modified as well.
unsigned long vtkCutter::GetMTime()
{
  unsigned long mTime=this->vtkDataSetFilter::GetMTime();
  unsigned long cutFuncMTime;

  if ( this->CutFunction != NULL )
    {
    cutFuncMTime = this->CutFunction->GetMTime();
    mTime = ( cutFuncMTime > mTime ? cutFuncMTime : mTime );
    }

  return mTime;
}

//
// Cut through data generating surface.
//
void vtkCutter::Execute()
{
  int cellId, i;
  vtkFloatPoints *cellPts;
  vtkFloatScalars cellScalars(VTK_CELL_SIZE);
  vtkCell *cell;
  vtkFloatScalars *newScalars;
  vtkCellArray *newVerts, *newLines, *newPolys;
  vtkFloatPoints *newPoints;
  float value, *x, s;
  vtkPolyData *output = this->GetOutput();
  int estimatedSize, numCells=this->Input->GetNumberOfCells();
  
  vtkDebugMacro(<< "Executing cutter");
  cellScalars.ReferenceCountingOff();
  
  //
  // Initialize self; create output objects
  //
  if ( !this->CutFunction )
    {
    vtkErrorMacro(<<"No cut function specified");
    return;
    }
//
// Create objects to hold output of contour operation
//
  estimatedSize = (int) pow ((double) numCells, .75);
  estimatedSize = estimatedSize / 1024 * 1024; //multiple of 1024
  if (estimatedSize < 1024) estimatedSize = 1024;

  newPoints = new vtkFloatPoints(estimatedSize,estimatedSize/2);
  newVerts = new vtkCellArray(estimatedSize,estimatedSize/2);
  newLines = new vtkCellArray(estimatedSize,estimatedSize/2);
  newPolys = new vtkCellArray(estimatedSize,estimatedSize/2);
  newScalars = new vtkFloatScalars(estimatedSize,estimatedSize/2);

  // locator used to merge potentially duplicate points
  if ( this->Locator == NULL ) this->CreateDefaultLocator();
  this->Locator->InitPointInsertion (newPoints, this->Input->GetBounds());
//
// Loop over all cells creating scalar function determined by evaluating cell
// points using cut function.
//
  value = 0.0;
  for (cellId=0; cellId < numCells; cellId++)
    {
    cell = Input->GetCell(cellId);
    cellPts = cell->GetPoints();
    for (i=0; i<cellPts->GetNumberOfPoints(); i++)
      {
      x = cellPts->GetPoint(i);
      s = this->CutFunction->EvaluateFunction(x);
      cellScalars.SetScalar(i,s);
      }

    cell->Contour(value, &cellScalars, this->Locator, 
                  newVerts, newLines, newPolys, newScalars);

    } // for all cells
//
// Update ourselves.  Because we don't know upfront how many verts, lines,
// polys we've created, take care to reclaim memory. 
//
  output->SetPoints(newPoints);
  newPoints->Delete();

  if (newVerts->GetNumberOfCells()) output->SetVerts(newVerts);
  newVerts->Delete();

  if (newLines->GetNumberOfCells()) output->SetLines(newLines);
  newLines->Delete();

  if (newPolys->GetNumberOfCells()) output->SetPolys(newPolys);
  newPolys->Delete();

  output->GetPointData()->SetScalars(newScalars);
  newScalars->Delete();

  this->Locator->Initialize();//release any extra memory
  output->Squeeze();
}

// Description:
// Specify a spatial locator for merging points. By default, 
// an instance of vtkMergePoints is used.
void vtkCutter::SetLocator(vtkPointLocator *locator)
{
  if ( this->Locator != locator ) 
    {
    if ( this->SelfCreatedLocator ) this->Locator->Delete();
    this->SelfCreatedLocator = 0;
    this->Locator = locator;
    this->Modified();
    }
}

void vtkCutter::CreateDefaultLocator()
{
  if ( this->SelfCreatedLocator ) this->Locator->Delete();
  this->Locator = new vtkMergePoints;
  this->SelfCreatedLocator = 1;
}

void vtkCutter::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataSetToPolyFilter::PrintSelf(os,indent);

  os << indent << "Cut Function: " << this->CutFunction << "\n";

  if ( this->Locator )
    {
    os << indent << "Locator: " << this->Locator << "\n";
    }
  else
    {
    os << indent << "Locator: (none)\n";
    }
}
