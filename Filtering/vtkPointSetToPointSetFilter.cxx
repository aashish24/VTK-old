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
#include "vtkPointSetToPointSetFilter.h"
#include "vtkPolyData.h"
#include "vtkStructuredGrid.h"
#include "vtkUnstructuredGrid.h"

// Construct object.
vtkPointSetToPointSetFilter::vtkPointSetToPointSetFilter()
{
  this->PolyData = vtkPolyData::New();
  this->PolyData->SetSource(this);
  
  this->StructuredGrid = vtkStructuredGrid::New();
  this->StructuredGrid->SetSource(this);
  
  this->UnstructuredGrid = vtkUnstructuredGrid::New();
  this->UnstructuredGrid->SetSource(this);
  
  this->Output = NULL;
}

vtkPointSetToPointSetFilter::~vtkPointSetToPointSetFilter()
{
  this->PolyData->Delete();
  this->StructuredGrid->Delete();
  this->UnstructuredGrid->Delete();
  this->Output = NULL;
}

// Specify the input data or filter.
void vtkPointSetToPointSetFilter::SetInput(vtkPointSet *input)
{
  if ( this->Input != input )
    {
    vtkDebugMacro(<<" setting Input to " << (void *)input);
    if (this->Input) {this->Input->UnRegister(this);}
    this->Input = input;
    if (this->Input) {this->Input->Register(this);}
    this->Modified();

    if ( this->Input == NULL )
      {
      return;
      }

    if ( input->GetDataSetType() == VTK_POLY_DATA )
      {
      this->Output = this->PolyData;
      }
    else if ( input->GetDataSetType() == VTK_STRUCTURED_GRID )
      {
      this->Output = this->StructuredGrid;
      }
    else if ( input->GetDataSetType() == VTK_UNSTRUCTURED_GRID )
      {
      this->Output = this->UnstructuredGrid;
      }
    else
      {
      vtkErrorMacro(<<"Mismatch in data type");
      }
    }
}


// Update input to this filter and the filter itself. Note that we are 
// overloading this method because the output is an abstract dataset type.
// This requires special treatment.
void vtkPointSetToPointSetFilter::Update()
{
  // make sure output has been created
  if ( !this->Output )
    {
    vtkErrorMacro(<< "No output has been created...need to set input");
    return;
    }

  // make sure input is available
  if ( !this->Input )
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
  this->Input->Update();
  this->Updating = 0;

  if (this->Input->GetMTime() > this->ExecuteTime ||
  this->GetMTime() > this->ExecuteTime )
    {
    if ( this->Input->GetDataReleased() )
      {
      this->Input->ForceUpdate();
      }

    if ( this->StartMethod )
      {
      (*this->StartMethod)(this->StartMethodArg);
      }
    // clear points and point data output 
    ((vtkDataSet *)this->Output)->CopyStructure((vtkDataSet *)this->Input);
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

  if ( this->Input->ShouldIReleaseData() )
    {
    this->Input->ReleaseData();
    }
}

  
// Get the output of this filter. If output is NULL, then input hasn't been
// set, which is necessary for abstract filter objects.
vtkPointSet *vtkPointSetToPointSetFilter::GetOutput()
{
  if ( this->Output == NULL )
    {
    vtkErrorMacro(<<"Abstract filters require input to be set before output can be retrieved");
    }
  return (vtkPointSet *)this->Output;
}

// Get the output as vtkPolyData. Performs run-time checking.
vtkPolyData *vtkPointSetToPointSetFilter::GetPolyDataOutput() 
{
  return this->PolyData;
}

// Get the output as vtkStructuredGrid. Performs run-time checking.
vtkStructuredGrid *vtkPointSetToPointSetFilter::GetStructuredGridOutput()
{
  return this->StructuredGrid;
}

// Get the output as vtkUnstructuredGrid. Performs run-time checking.
vtkUnstructuredGrid *vtkPointSetToPointSetFilter::GetUnstructuredGridOutput()
{
  return this->UnstructuredGrid;
}

void vtkPointSetToPointSetFilter::UnRegister(vtkObject *o)
{
  // detect the circular loop source <-> data
  // If we have two references and one of them is my data
  // and I am not being unregistered by my data, break the loop.
  if (this->ReferenceCount == 4 &&
      this->PolyData != o && this->StructuredGrid != o &&
      this->UnstructuredGrid != o &&
      this->PolyData->GetNetReferenceCount() == 1 &&
      this->StructuredGrid->GetNetReferenceCount() == 1 &&
      this->UnstructuredGrid->GetNetReferenceCount() == 1)
    {
    this->PolyData->SetSource(NULL);
    this->StructuredGrid->SetSource(NULL);
    this->UnstructuredGrid->SetSource(NULL);
    }
  if (this->ReferenceCount == 3 &&
      (this->PolyData == o || this->StructuredGrid == o ||
      this->UnstructuredGrid == o) &&
      (this->PolyData->GetNetReferenceCount() +
      this->StructuredGrid->GetNetReferenceCount() +
      this->UnstructuredGrid->GetNetReferenceCount()) == 4)
    {
    this->PolyData->SetSource(NULL);
    this->StructuredGrid->SetSource(NULL);
    this->UnstructuredGrid->SetSource(NULL);
    }
  
  this->vtkObject::UnRegister(o);
}

int vtkPointSetToPointSetFilter::InRegisterLoop(vtkObject *o)
{
  int num = 0;
  int cnum = 0;
  
  if (this->PolyData->GetSource() == this)
    {
    num++;
    cnum += this->PolyData->GetNetReferenceCount();
    }
  if (this->StructuredGrid->GetSource() == this)
    {
    num++;
    cnum += this->StructuredGrid->GetNetReferenceCount();
    }
  if (this->UnstructuredGrid->GetSource() == this)
    {
    num++;
    cnum += this->UnstructuredGrid->GetNetReferenceCount();
    }
  
  // if no one outside is using us
  // and our data objects are down to one net reference
  // and we are being asked by one of our data objects
  if (this->ReferenceCount == num &&
      cnum == (num + 1) &&
      (this->PolyData == o ||
       this->StructuredGrid == o ||
       this->UnstructuredGrid == o))
    {
    return 1;
    }
  return 0;
}
