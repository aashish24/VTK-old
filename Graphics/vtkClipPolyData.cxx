/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1996 Ken Martin, Will Schroeder, Bill Lorensen.

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
#include "vtkClipPolyData.h"
#include "vtkMergePoints.h"
#include "vtkLine.h"
#include "vtkTriangle.h"

// Description:
// Construct with user-specified implicit function; InsideOut turned off; value
// set to 0.0; and generate clip scalars turned off.
vtkClipPolyData::vtkClipPolyData(vtkImplicitFunction *cf)
{
  this->ClipFunction = cf;
  this->InsideOut = 0;
  this->Locator = NULL;
  this->Value = 0.0;
  this->GenerateClipScalars = 0;
  this->SelfCreatedLocator = 0;

  this->GenerateClippedOutput = 0;
  this->ClippedOutput = vtkPolyData::New();
  this->ClippedOutput->SetSource(this);
}

vtkClipPolyData::~vtkClipPolyData()
{
  this->ClippedOutput->Delete();
}

// Description:
// Overload standard modified time function. If Clip functions is modified,
// then this object is modified as well.
unsigned long vtkClipPolyData::GetMTime()
{
  unsigned long mTime=this->vtkPolyDataToPolyDataFilter::GetMTime();
  unsigned long ClipFuncMTime;

  if ( this->ClipFunction != NULL )
    {
    ClipFuncMTime = this->ClipFunction->GetMTime();
    mTime = ( ClipFuncMTime > mTime ? ClipFuncMTime : mTime );
    }

  return mTime;
}

//
// Clip through data generating surface.
//
void vtkClipPolyData::Execute()
{
  vtkPolyData *input = (vtkPolyData *)this->Input;
  vtkPolyData *output = this->GetOutput();
  int cellId, i;
  vtkFloatPoints *cellPts;
  vtkFloatScalars *clipScalars = NULL;;
  vtkScalars *inputScalars;
  vtkFloatScalars cellScalars(VTK_CELL_SIZE); cellScalars.ReferenceCountingOff();
  vtkCell *cell;
  vtkCellArray *newVerts, *newLines, *newPolys, *connList=NULL;
  vtkCellArray *clippedVerts=NULL, *clippedLines=NULL;
  vtkCellArray *clippedPolys=NULL, *clippedList=NULL;
  vtkFloatPoints *newPoints;
  vtkIdList *cellIds;
  float value, s;
  int estimatedSize, numCells=input->GetNumberOfCells();
  int numPts=input->GetNumberOfPoints();
  vtkPoints *inPts=input->GetPoints();  
  int numberOfPoints;
  vtkPointData *inPD, *outPD;
  
  vtkDebugMacro(<< "Clipping polygonal data");
  
  //
  // Initialize self; create output objects
  //
  if ( !this->ClipFunction && this->GenerateClipScalars)
    {
    vtkErrorMacro(<<"No clip function specified and GenerateClipScalarsOn");
    return;
    }

  if ( numPts < 1 || inPts == NULL )
    {
    vtkErrorMacro(<<"No data to clip");
    return;
    }
  //
  // Create objects to hold output of clip operation
  //
  estimatedSize = numCells;
  estimatedSize = estimatedSize / 1024 * 1024; //multiple of 1024
  if (estimatedSize < 1024) estimatedSize = 1024;

  newPoints = vtkFloatPoints::New();
  newPoints->Allocate(numPts,numPts/2);
  newVerts = vtkCellArray::New();
  newVerts->Allocate(estimatedSize,estimatedSize/2);
  newLines = vtkCellArray::New();
  newLines->Allocate(estimatedSize,estimatedSize/2);
  newPolys = vtkCellArray::New();
  newPolys->Allocate(estimatedSize,estimatedSize/2);

  // locator used to merge potentially duplicate points
  if ( this->Locator == NULL ) this->CreateDefaultLocator();
  this->Locator->InitPointInsertion (newPoints, input->GetBounds());

  // Interpolate data along edge. If generating clip scalars, do the necessary setup.
  if ( this->GenerateClipScalars )
    {
    clipScalars = vtkFloatScalars::New();
    clipScalars->Allocate(numPts);
    inPD = new vtkPointData(*(input->GetPointData()));
    inPD->SetScalars(clipScalars);
    for ( i=0; i < numPts; i++ )
      {
      s = this->ClipFunction->FunctionValue(inPts->GetPoint(i));
      clipScalars->InsertScalar(i,s);
      }
    }
  else 
    {
    inPD = input->GetPointData();
    if (inPD->GetScalars () == NULL)
      {
      vtkErrorMacro(<<"No input scalars and GenerateClipScalarsOff");
      return;
      }      
    }
    
  inputScalars = inPD->GetScalars ();

  outPD = output->GetPointData();
  outPD->InterpolateAllocate(inPD,estimatedSize,estimatedSize/2);

  // If generating second output, setup clipped output
  if ( this->GenerateClippedOutput )
    {
    this->ClippedOutput->Initialize();
    clippedVerts = vtkCellArray::New();
    clippedVerts->Allocate(estimatedSize,estimatedSize/2);
    clippedLines = vtkCellArray::New();
    clippedLines->Allocate(estimatedSize,estimatedSize/2);
    clippedPolys = vtkCellArray::New();
    clippedPolys->Allocate(estimatedSize,estimatedSize/2);
    }

  // perform clipping on cells
  value = this->Value;
  for (cellId=0; cellId < numCells; cellId++)
    {
    cell = input->GetCell(cellId);
    cellPts = cell->GetPoints();
    cellIds = cell->GetPointIds();
    numberOfPoints = cellPts->GetNumberOfPoints();

    // evaluate implicit cutting function
    for ( i=0; i < numberOfPoints; i++ )
      {
      s = inputScalars->GetScalar(cellIds->GetId(i));
      cellScalars.InsertScalar(i, s);
      }

    switch ( cell->GetCellDimension() )
      {

      case 0: //points are generated-------------------------------
        connList = newVerts;
        clippedList = clippedVerts;
        break;

      case 1: //lines are generated----------------------------------
        connList = newLines;
        clippedList = clippedLines;
        break;

      case 2: //triangles are generated------------------------------
        connList = newPolys;
        clippedList = clippedPolys;
        break;

      } //switch

    cell->Clip(this->Value, &cellScalars, this->Locator, connList,
               inPD, outPD, this->InsideOut);

    if ( this->GenerateClippedOutput )
      {
      cell->Clip(this->Value, &cellScalars, this->Locator, clippedList,
                 inPD, outPD, !this->InsideOut);
      }

    } //for each cell

  vtkDebugMacro(<<"Created: " 
               << newPoints->GetNumberOfPoints() << " points, " 
               << newVerts->GetNumberOfCells() << " verts, " 
               << newLines->GetNumberOfCells() << " lines, " 
               << newPolys->GetNumberOfCells() << " polys");

  if ( this->GenerateClippedOutput )
    {
    vtkDebugMacro(<<"Created (clipped output): " 
                 << clippedVerts->GetNumberOfCells() << " verts, " 
                 << clippedLines->GetNumberOfCells() << " lines, " 
                 << clippedPolys->GetNumberOfCells() << " triangles");
    }

  //
  // Update ourselves.  Because we don't know upfront how many verts, lines,
  // polys we've created, take care to reclaim memory. 
  //
  if (clipScalars) clipScalars->Delete();
  if ( this->GenerateClipScalars ) inPD->Delete();

  if (newVerts->GetNumberOfCells()) output->SetVerts(newVerts);
  newVerts->Delete();

  if (newLines->GetNumberOfCells()) output->SetLines(newLines);
  newLines->Delete();

  if (newPolys->GetNumberOfCells()) output->SetPolys(newPolys);
  newPolys->Delete();

  if ( this->GenerateClippedOutput )
    {
    this->ClippedOutput->SetPoints(newPoints);

    if (clippedVerts->GetNumberOfCells()) this->ClippedOutput->SetVerts(clippedVerts);
    clippedVerts->Delete();

    if (clippedLines->GetNumberOfCells()) this->ClippedOutput->SetLines(clippedLines);
    clippedLines->Delete();

    if (clippedPolys->GetNumberOfCells()) this->ClippedOutput->SetPolys(clippedPolys);
    clippedPolys->Delete();
    
    this->ClippedOutput->GetPointData()->PassData(outPD);
    this->ClippedOutput->Squeeze();
    }

  output->SetPoints(newPoints);
  newPoints->Delete();

  this->Locator->Initialize();//release any extra memory
  output->Squeeze();
}


// Description:
// Specify a spatial locator for merging points. By default, 
// an instance of vtkMergePoints is used.
void vtkClipPolyData::SetLocator(vtkPointLocator *locator)
{
  if ( this->Locator != locator ) 
    {
    if ( this->SelfCreatedLocator ) this->Locator->Delete();
    this->SelfCreatedLocator = 0;
    this->Locator = locator;
    this->Modified();
    }
}

void vtkClipPolyData::CreateDefaultLocator()
{
  if ( this->SelfCreatedLocator ) this->Locator->Delete();
  this->Locator = vtkMergePoints::New();
  this->SelfCreatedLocator = 1;
}

void vtkClipPolyData::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolyDataToPolyDataFilter::PrintSelf(os,indent);

  os << indent << "Clip Function: " << this->ClipFunction << "\n";
  os << indent << "InsideOut: " << (this->InsideOut ? "On\n" : "Off\n");
  os << indent << "Value: " << this->Value << "\n";
  if ( this->Locator )
    {
    os << indent << "Locator: " << this->Locator << "\n";
    }
  else
    {
    os << indent << "Locator: (none)\n";
    }

  os << indent << "Generate Clip Scalars: " << (this->GenerateClipScalars ? "On\n" : "Off\n");

  os << indent << "Generate Clipped Output: " << (this->GenerateClippedOutput ? "On\n" : "Off\n");
}
