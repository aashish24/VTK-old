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
#include <math.h>
#include "vtkClipPolyData.h"
#include "vtkMergePoints.h"
#include "vtkLine.h"
#include "vtkTriangle.h"

//----------------------------------------------------------------------------
// Construct with user-specified implicit function; InsideOut turned off; value
// set to 0.0; and generate clip scalars turned off.
vtkClipPolyData::vtkClipPolyData(vtkImplicitFunction *cf)
{
  this->ClipFunction = cf;
  this->InsideOut = 0;
  this->Locator = NULL;
  this->Value = 0.0;
  this->GenerateClipScalars = 0;

  this->GenerateClippedOutput = 0;
  this->vtkSource::SetOutput(1,vtkPolyData::New());
  this->Outputs[1]->Delete();
}

//----------------------------------------------------------------------------
vtkClipPolyData::~vtkClipPolyData()
{
  if ( this->Locator )
    {
    this->Locator->UnRegister(this);
    this->Locator = NULL;
    }
  this->SetClipFunction(NULL);
}

//----------------------------------------------------------------------------
// Overload standard modified time function. If Clip functions is modified,
// then this object is modified as well.
unsigned long vtkClipPolyData::GetMTime()
{
  unsigned long mTime=this->vtkPolyDataToPolyDataFilter::GetMTime();
  unsigned long time;

  if ( this->ClipFunction != NULL )
    {
    time = this->ClipFunction->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }
  if ( this->Locator != NULL )
    {
    time = this->Locator->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }

  return mTime;
}

vtkPolyData *vtkClipPolyData::GetClippedOutput()
{
  if (this->NumberOfOutputs < 2)
    {
    return NULL;
    }
  
  return (vtkPolyData *)(this->Outputs[1]);
}


//----------------------------------------------------------------------------
//
// Clip through data generating surface.
//
void vtkClipPolyData::Execute()
{
  vtkPolyData *input = this->GetInput();
  vtkPolyData *output = this->GetOutput();
  int cellId, i, updateTime;
  vtkPoints *cellPts;
  vtkScalars *clipScalars;
  vtkScalars *cellScalars; 
  vtkCell *cell;
  vtkCellArray *newVerts, *newLines, *newPolys, *connList=NULL;
  vtkCellArray *clippedVerts=NULL, *clippedLines=NULL;
  vtkCellArray *clippedPolys=NULL, *clippedList=NULL;
  vtkPoints *newPoints;
  vtkIdList *cellIds;
  float s;
  int estimatedSize, numCells=input->GetNumberOfCells();
  int numPts=input->GetNumberOfPoints();
  vtkPoints *inPts=input->GetPoints();  
  int numberOfPoints;
  vtkPointData *inPD=input->GetPointData(), *outPD = output->GetPointData();
  vtkCellData *inCD=input->GetCellData(), *outCD = output->GetCellData();
  
  vtkDebugMacro(<< "Clipping polygonal data");
  
  //
  // Initialize self; create output objects
  //
  if ( numPts < 1 || inPts == NULL )
    {
    vtkErrorMacro(<<"No data to clip");
    return;
    }

  if ( !this->ClipFunction && this->GenerateClipScalars )
    {
    vtkErrorMacro(<<"Cannot generate clip scalars if no clip function defined");
    return;
    }

  this->UpdateProgress(0.0f);

  //
  // Create objects to hold output of clip operation
  //
  estimatedSize = numCells;
  estimatedSize = estimatedSize / 1024 * 1024; //multiple of 1024
  if (estimatedSize < 1024)
    {
    estimatedSize = 1024;
    }

  newPoints = vtkPoints::New();
  newPoints->Allocate(numPts,numPts/2);
  newVerts = vtkCellArray::New();
  newVerts->Allocate(estimatedSize,estimatedSize/2);
  newLines = vtkCellArray::New();
  newLines->Allocate(estimatedSize,estimatedSize/2);
  newPolys = vtkCellArray::New();
  newPolys->Allocate(estimatedSize,estimatedSize/2);

  // locator used to merge potentially duplicate points
  if ( this->Locator == NULL )
    {
    this->CreateDefaultLocator();
    }
  this->Locator->InitPointInsertion (newPoints, input->GetBounds());

  // Determine whether we're clipping with input scalars or a clip function
  // and to necessary setup.
  if ( this->ClipFunction )
    {
    vtkScalars *tmpScalars = vtkScalars::New();
    tmpScalars->SetNumberOfScalars(numPts);
    inPD = vtkPointData::New();
    inPD->ShallowCopy(input->GetPointData());//copies original
    if ( this->GenerateClipScalars )
      {
      inPD->SetScalars(tmpScalars);
      }
    for ( i=0; i < numPts; i++ )
      {
      s = this->ClipFunction->FunctionValue(inPts->GetPoint(i));
      tmpScalars->SetScalar(i,s);
      }
    clipScalars = (vtkScalars *)tmpScalars;
    }
  else //using input scalars
    {
    clipScalars = inPD->GetScalars();
    if ( !clipScalars )
      {
      vtkErrorMacro(<<"Cannot clip without clip function or input scalars");
      return;
      }
    }
    
  if ( !this->GenerateClipScalars && !input->GetPointData()->GetScalars())
    {
    outPD->CopyScalarsOff();
    }
  else
    {
    outPD->CopyScalarsOn();
    }
  outPD->InterpolateAllocate(inPD,estimatedSize,estimatedSize/2);
  outCD->CopyAllocate(inCD,estimatedSize,estimatedSize/2);

  // If generating second output, setup clipped output
  if ( this->GenerateClippedOutput )
    {
    this->GetClippedOutput()->Initialize();
    clippedVerts = vtkCellArray::New();
    clippedVerts->Allocate(estimatedSize,estimatedSize/2);
    clippedLines = vtkCellArray::New();
    clippedLines->Allocate(estimatedSize,estimatedSize/2);
    clippedPolys = vtkCellArray::New();
    clippedPolys->Allocate(estimatedSize,estimatedSize/2);
    }

  cellScalars = vtkScalars::New();
  cellScalars->Allocate(VTK_CELL_SIZE);
  
  // perform clipping on cells
  updateTime = numCells / 50;  // update every 2%
  if (updateTime < 1)
    {
    updateTime = 1;
    }

  for (cellId=0; cellId < numCells; cellId++)
    {
    cell = input->GetCell(cellId);
    cellPts = cell->GetPoints();
    cellIds = cell->GetPointIds();
    numberOfPoints = cellPts->GetNumberOfPoints();

    // evaluate implicit cutting function
    for ( i=0; i < numberOfPoints; i++ )
      {
      s = clipScalars->GetScalar(cellIds->GetId(i));
      cellScalars->InsertScalar(i, s);
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

    cell->Clip(this->Value, cellScalars, this->Locator, connList,
               inPD, outPD, inCD, cellId, outCD, this->InsideOut);

    if ( this->GenerateClippedOutput )
      {
      cell->Clip(this->Value, cellScalars, this->Locator, clippedList,
                 inPD, outPD, inCD, cellId, outCD, !this->InsideOut);
      }

    if (cellId % updateTime == 0)
      {
      this->UpdateProgress((float)cellId / numCells);
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
  if ( this->ClipFunction ) 
    {
    clipScalars->Delete();
    inPD->Delete();
    }

  if (newVerts->GetNumberOfCells())
    {
    output->SetVerts(newVerts);
    }
  newVerts->Delete();

  if (newLines->GetNumberOfCells())
    {
    output->SetLines(newLines);
    }
  newLines->Delete();

  if (newPolys->GetNumberOfCells())
    {
    output->SetPolys(newPolys);
    }
  newPolys->Delete();

  if ( this->GenerateClippedOutput )
    {
    this->GetClippedOutput()->SetPoints(newPoints);

    if (clippedVerts->GetNumberOfCells())
      {
      this->GetClippedOutput()->SetVerts(clippedVerts);
      }
    clippedVerts->Delete();

    if (clippedLines->GetNumberOfCells())
      {
      this->GetClippedOutput()->SetLines(clippedLines);
      }
    clippedLines->Delete();

    if (clippedPolys->GetNumberOfCells())
      {
      this->GetClippedOutput()->SetPolys(clippedPolys);
      }
    clippedPolys->Delete();
    
    this->GetClippedOutput()->GetPointData()->PassData(outPD);
    this->GetClippedOutput()->Squeeze();
    }

  output->SetPoints(newPoints);
  newPoints->Delete();
  cellScalars->Delete();
  
  this->Locator->Initialize();//release any extra memory
  output->Squeeze();
}


//----------------------------------------------------------------------------
// Specify a spatial locator for merging points. By default, 
// an instance of vtkMergePoints is used.
void vtkClipPolyData::SetLocator(vtkPointLocator *locator)
{
  if ( this->Locator == locator)
    {
    return;
    }
  
  if ( this->Locator )
    {
    this->Locator->UnRegister(this);
    this->Locator = NULL;
    }

  if ( locator )
    {
    locator->Register(this);
    }

  this->Locator = locator;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkClipPolyData::CreateDefaultLocator()
{
  if ( this->Locator == NULL )
    {
    this->Locator = vtkMergePoints::New();
    }
}


//----------------------------------------------------------------------------
void vtkClipPolyData::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkPolyDataToPolyDataFilter::PrintSelf(os,indent);

  if ( this->ClipFunction )
    {
    os << indent << "Clip Function: " << this->ClipFunction << "\n";
    }
  else
    {
    os << indent << "Clip Function: (none)\n";
    }
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
