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
#include "vtkAppendPolyData.h"

vtkAppendPolyData::vtkAppendPolyData()
{
  this->Output = vtkPolyData::New();
  this->Output->SetSource(this);
  this->InputList = vtkPolyDataCollection::New();
}

vtkAppendPolyData::~vtkAppendPolyData()
{
  this->InputList->Delete();
  this->InputList = NULL;
}

  

// Add a dataset to the list of data to append.
void vtkAppendPolyData::AddInput(vtkPolyData *ds)
{
  if ( ! this->InputList->IsItemPresent(ds) )
    {
    this->Modified();
    this->InputList->AddItem(ds);
    }
}

// Remove a dataset from the list of data to append.
void vtkAppendPolyData::RemoveInput(vtkPolyData *ds)
{
  if ( this->InputList->IsItemPresent(ds) )
    {
    this->Modified();
    this->InputList->RemoveItem(ds);
    }
}

void vtkAppendPolyData::Update()
{
  unsigned long int mtime, pdMtime;
  vtkPolyData *pd;

  // make sure input is available
  if ( this->InputList->GetNumberOfItems() < 1 )
    {
    vtkErrorMacro(<< "No input...can't execute!");
    return;
    }

  // prevent chasing our tail
  if (this->Updating)
    {
    return;
    }

  this->Updating = 1;
  for (mtime=0, this->InputList->InitTraversal(); 
       (pd = this->InputList->GetNextItem());)
    {
    pd->Update();
    pdMtime = pd->GetMTime();
    if ( pdMtime > mtime )
      {
      mtime = pdMtime;
      }
    }
  this->Updating = 0;

  if ( mtime > this->ExecuteTime || this->GetMTime() > this->ExecuteTime )
    {
    for (this->InputList->InitTraversal();(pd=this->InputList->GetNextItem()); )
      {
      if ( pd->GetDataReleased() )
	{
	pd->ForceUpdate();
	}
      }

    if ( this->StartMethod )
      {
      (*this->StartMethod)(this->StartMethodArg);
      }
    this->Output->Initialize(); //clear output
    // reset AbortExecute flag and Progress
    this->AbortExecute = 0;
    this->Progress = 0.0;
    this->Execute();
    this->ExecuteTime.Modified();
    if ( !this->AbortExecute )
      {
      this->UpdateProgress(1.0);
      }
    this->SetDataReleased(0);
    if ( this->EndMethod )
      {
      (*this->EndMethod)(this->EndMethodArg);
      }
    }

  for (this->InputList->InitTraversal(); (pd = this->InputList->GetNextItem()); )
    {
    if ( pd->ShouldIReleaseData() )
      {
      pd->ReleaseData();
      }
    }
}

// Append data sets into single unstructured grid
void vtkAppendPolyData::Execute()
{
  int scalarsPresentInPD, vectorsPresentInPD;
  int normalsPresentInPD, tcoordsPresentInPD;
  int tensorsPresentInPD, fieldPresentInPD;
  int scalarsPresentInCD, vectorsPresentInCD;
  int normalsPresentInCD, tcoordsPresentInCD;
  int tensorsPresentInCD, fieldPresentInCD;
  vtkPolyData *ds;
  vtkPoints  *inPts;
  vtkPoints *newPts;
  vtkCellArray *inVerts, *newVerts;
  vtkCellArray *inLines, *newLines;
  vtkCellArray *inPolys, *newPolys;
  vtkCellArray *inStrips, *newStrips;
  int i, ptId, ptOffset, cellId, cellOffset;
  int numPts, numCells;
  vtkPointData *pd = NULL;
  vtkCellData *cd = NULL;
  int npts, *pts;
  vtkPolyData *output = (vtkPolyData *)this->Output;
  vtkPointData *outputPD = output->GetPointData();
  vtkCellData *outputCD = output->GetCellData();
  
  vtkDebugMacro(<<"Appending data together");

  // loop over all data sets, checking to see what point data is available.
  numPts = 0;
  numCells = 0;
  scalarsPresentInPD = 1;
  vectorsPresentInPD = 1;
  normalsPresentInPD = 1;
  tcoordsPresentInPD = 1;
  tensorsPresentInPD = 1;
  fieldPresentInPD = 1;
  scalarsPresentInCD = 1;
  vectorsPresentInCD = 1;
  normalsPresentInCD = 1;
  tcoordsPresentInCD = 1;
  tensorsPresentInCD = 1;
  fieldPresentInCD = 1;

  for (this->InputList->InitTraversal(); (ds = this->InputList->GetNextItem()); )
    {
    numPts += ds->GetNumberOfPoints();
    numCells += ds->GetNumberOfCells();
    pd = ds->GetPointData();

    if ( pd && pd->GetScalars() == NULL )
      {
      scalarsPresentInPD &= 0;
      }
    if ( pd && pd->GetVectors() == NULL )
      {
      vectorsPresentInPD &= 0;
      }
    if ( pd && pd->GetNormals() == NULL )
      {
      normalsPresentInPD &= 0;
      }
    if ( pd && pd->GetTCoords() == NULL )
      {
      tcoordsPresentInPD &= 0;
      }
    if ( pd && pd->GetTensors() == NULL )
      {
      tensorsPresentInPD &= 0;
      }
    if ( pd && pd->GetFieldData() == NULL )
      {
      fieldPresentInPD &= 0;
      }

    cd = ds->GetCellData();
    if ( cd && cd->GetScalars() == NULL )
      {
      scalarsPresentInCD &= 0;
      }
    if ( cd && cd->GetVectors() == NULL )
      {
      vectorsPresentInCD &= 0;
      }
    if ( cd && cd->GetNormals() == NULL )
      {
      normalsPresentInCD &= 0;
      }
    if ( cd && cd->GetTCoords() == NULL )
      {
      tcoordsPresentInCD &= 0;
      }
    if ( cd && cd->GetTensors() == NULL )
      {
      tensorsPresentInCD &= 0;
      }
    if ( cd && cd->GetFieldData() == NULL )
      {
      fieldPresentInCD &= 0;
      }

    }

  if ( numPts < 1 || numCells < 1 )
    {
    vtkErrorMacro(<<"No data to append!");
    return;
    }

  // Now can allocate memory

  if ( !scalarsPresentInPD )
    {
    outputPD->CopyScalarsOff();
    }
  if ( !vectorsPresentInPD )
    {
    outputPD->CopyVectorsOff();
    }
  if ( !normalsPresentInPD )
    {
    outputPD->CopyNormalsOff();
    }
  if ( !tcoordsPresentInPD )
    {
    outputPD->CopyTCoordsOff();
    }
  if ( !tensorsPresentInPD )
    {
    outputPD->CopyTensorsOff();
    }
  if ( !fieldPresentInPD )
    {
    outputPD->CopyFieldDataOff();
    }
  outputPD->CopyAllocate(pd,numPts);

  // now do cell data
  if ( !scalarsPresentInCD )
    {
    outputCD->CopyScalarsOff();
    }
  if ( !vectorsPresentInCD )
    {
    outputCD->CopyVectorsOff();
    }
  if ( !normalsPresentInCD )
    {
    outputCD->CopyNormalsOff();
    }
  if ( !tcoordsPresentInCD )
    {
    outputCD->CopyTCoordsOff();
    }
  if ( !tensorsPresentInCD )
    {
    outputCD->CopyTensorsOff();
    }
  if ( !fieldPresentInCD )
    {
    outputCD->CopyFieldDataOff();
    }
  outputCD->CopyAllocate(cd,numCells);

  newPts = vtkPoints::New();
  newPts->SetNumberOfPoints(numPts);

  newVerts = vtkCellArray::New();
  newVerts->Allocate(numCells*4);

  newLines = vtkCellArray::New();
  newLines->Allocate(numCells*4);

  newPolys = vtkCellArray::New();
  newPolys->Allocate(numCells*4);

  newStrips = vtkCellArray::New();
  newStrips->Allocate(numCells*4);

  // loop over all input sets
  for (ptOffset=0, cellOffset = 0, this->InputList->InitTraversal(); 
       (ds = this->InputList->GetNextItem());
       ptOffset+=numPts, cellOffset+=numCells)
    {
    numPts = ds->GetNumberOfPoints();
    numCells = ds->GetNumberOfCells();
    pd = ds->GetPointData();
    cd = ds->GetCellData();
    
    inPts = ds->GetPoints();
    inVerts = ds->GetVerts();
    inLines = ds->GetLines();
    inPolys = ds->GetPolys();
    inStrips = ds->GetStrips();

    // copy points and point data
    for (ptId=0; ptId < numPts; ptId++)
      {
      newPts->SetPoint(ptId+ptOffset,inPts->GetPoint(ptId));
      outputPD->CopyData(pd,ptId,ptId+ptOffset);
      }

    // copy cell data
    for (cellId=0; cellId < numCells; cellId++)
      {
      outputCD->CopyData(cd,cellId,cellId+cellOffset);
      }

    // copy the cells
    for (inVerts->InitTraversal(); inVerts->GetNextCell(npts,pts); )
      {
      newVerts->InsertNextCell(npts);
      for (i=0; i < npts; i++)
	{
	newVerts->InsertCellPoint(pts[i]+ptOffset);
	}
      }
  
    for (inLines->InitTraversal(); inLines->GetNextCell(npts,pts); )
      {
      newLines->InsertNextCell(npts);
      for (i=0; i < npts; i++)
	{
	newLines->InsertCellPoint(pts[i]+ptOffset);
	}
      }
  
    for (inPolys->InitTraversal(); inPolys->GetNextCell(npts,pts); )
      {
      newPolys->InsertNextCell(npts);
      for (i=0; i < npts; i++)
	{
	newPolys->InsertCellPoint(pts[i]+ptOffset);
	}
      }
  
    for (inStrips->InitTraversal(); inStrips->GetNextCell(npts,pts); )
      {
      newStrips->InsertNextCell(npts);
      for (i=0; i < npts; i++)
	{
	newStrips->InsertCellPoint(pts[i]+ptOffset);
	}
      }
    }
//
// Update ourselves and release memory
//
  output->SetPoints(newPts);
  newPts->Delete();

  if ( newVerts->GetNumberOfCells() > 0 )
    {
    output->SetVerts(newVerts);
    }
  newVerts->Delete();

  if ( newLines->GetNumberOfCells() > 0 )
    {
    output->SetLines(newLines);
    }
  newLines->Delete();

  if ( newPolys->GetNumberOfCells() > 0 )
    {
    output->SetPolys(newPolys);
    }
  newPolys->Delete();

  if ( newStrips->GetNumberOfCells() > 0 )
    {
    output->SetStrips(newStrips);
    }
  newStrips->Delete();

  output->Squeeze();
}

void vtkAppendPolyData::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkFilter::PrintSelf(os,indent);

  os << indent << "Input DataSets:\n";
  this->InputList->PrintSelf(os,indent.GetNextIndent());
}

