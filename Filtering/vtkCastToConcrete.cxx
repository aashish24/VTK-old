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
#include "vtkCastToConcrete.h"

// Construct object.
vtkCastToConcrete::vtkCastToConcrete()
{
  this->PolyData = vtkPolyData::New();
  this->PolyData->SetSource(this);
  this->StructuredPoints = vtkStructuredPoints::New();
  this->StructuredPoints->SetSource(this);
  this->StructuredGrid = vtkStructuredGrid::New();
  this->StructuredGrid->SetSource(this);
  this->UnstructuredGrid = vtkUnstructuredGrid::New();
  this->UnstructuredGrid->SetSource(this);
  this->RectilinearGrid = vtkRectilinearGrid::New();
  this->RectilinearGrid->SetSource(this);
}

vtkCastToConcrete::~vtkCastToConcrete()
{
  this->PolyData->Delete();
  this->StructuredPoints->Delete();
  this->StructuredGrid->Delete();
  this->UnstructuredGrid->Delete();
  this->RectilinearGrid->Delete();
  this->Output = NULL;
}

// Special method just passes Update through pipeline.
void vtkCastToConcrete::Update()
{
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
    if ( this->StartMethod )
      {
      (*this->StartMethod)(this->StartMethodArg);
      }
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
}
  
void vtkCastToConcrete::Execute()
{
  vtkDataSet *input=(vtkDataSet *)this->Input;

  vtkDebugMacro(<<"Casting to concrete type...");

  if ( input->GetDataSetType() == VTK_POLY_DATA )
    {
    this->PolyData->CopyStructure(input);
    this->PolyData->GetPointData()->PassData(input->GetPointData());
    this->PolyData->GetCellData()->PassData(input->GetCellData());
    }

  else if ( input->GetDataSetType() == VTK_STRUCTURED_POINTS )
    {
    this->StructuredPoints->CopyStructure(input);
    this->StructuredPoints->GetPointData()->PassData(input->GetPointData());
    this->StructuredPoints->GetCellData()->PassData(input->GetCellData());
    }

  else if ( input->GetDataSetType() == VTK_STRUCTURED_GRID )
    {
    this->StructuredGrid->CopyStructure(input);
    this->StructuredGrid->GetPointData()->PassData(input->GetPointData());
    this->StructuredGrid->GetCellData()->PassData(input->GetCellData());
    }

  else if ( input->GetDataSetType() == VTK_UNSTRUCTURED_GRID )
    {
    this->UnstructuredGrid->CopyStructure(input);
    this->UnstructuredGrid->GetPointData()->PassData(input->GetPointData());
    this->UnstructuredGrid->GetCellData()->PassData(input->GetCellData());
    }

  else if ( input->GetDataSetType() == VTK_RECTILINEAR_GRID )
    {
    this->RectilinearGrid->CopyStructure(input);
    this->RectilinearGrid->GetPointData()->PassData(input->GetPointData());
    this->RectilinearGrid->GetCellData()->PassData(input->GetCellData());
    }

  else
    {
    this->Output = this->Input;
    }
}

// Specify the input data or filter.
void vtkCastToConcrete::SetInput(vtkDataSet *input)
{
  if ( this->Input != input )
    {
    vtkDebugMacro(<<" setting Input to " << (void *)input);
    if (this->Input) {this->Input->UnRegister(this);}
    this->Input = input;
    if (this->Input) {this->Input->Register(this);}
    this->Modified();

    if ( input == NULL )
      {
      return;
      }

    if ( input->GetDataSetType() == VTK_POLY_DATA )
      {
      this->Output = this->PolyData;
      }

    else if ( input->GetDataSetType() == VTK_STRUCTURED_POINTS )
      {
      this->Output = this->StructuredPoints;
      }

    else if ( input->GetDataSetType() == VTK_STRUCTURED_GRID )
      {
      this->Output = this->StructuredGrid;
      }

    else if ( input->GetDataSetType() == VTK_UNSTRUCTURED_GRID )
      {
      this->Output = this->UnstructuredGrid;
      }

    else if ( input->GetDataSetType() == VTK_RECTILINEAR_GRID )
      {
      this->Output = this->RectilinearGrid;
      }

    else
      {
      vtkErrorMacro(<<"Mismatch in data type");
      }
    }
}

// Get the output of this filter. If output is NULL then input hasn't been set
// which is necessary for abstract objects.
vtkDataSet *vtkCastToConcrete::GetOutput()
{
  if ( this->Input == NULL )
    {
    vtkErrorMacro(<<"Filter requires input to be set before output can be retrieved");
    }
  return (vtkDataSet *)this->Output;
}

// Get the output of this filter as type vtkPolyData. Performs run-time
// checking on type. Returns NULL if wrong type.
vtkPolyData *vtkCastToConcrete::GetPolyDataOutput()
{
  if ( this->Input == NULL )
    {
    vtkErrorMacro(<<"Filter requires input to be set before output can be retrieved");
    }
  else
    {
    if ( ((vtkDataSet *)this->Input)->GetDataSetType() != VTK_POLY_DATA )
      {
      vtkErrorMacro(<<"Cannot cast to type requested");
      return NULL;
      }
    }

  return this->PolyData;
}

// Get the output of this filter as type vtkStructuredPoints. Performs run-time
// checking on type. Returns NULL if wrong type.
vtkStructuredPoints *vtkCastToConcrete::GetStructuredPointsOutput()
{
  if ( this->Input == NULL )
    {
    vtkErrorMacro(<<"Filter requires input to be set before output can be retrieved");
    }
  else
    {
    if ( ((vtkDataSet *)this->Input)->GetDataSetType() != VTK_STRUCTURED_POINTS )
      {
      vtkErrorMacro(<<"Cannot cast to type requested");
      return NULL;
      }
    }

  return this->StructuredPoints;
}

// Get the output of this filter as type vtkStructuredGrid. Performs run-time
// checking on type. Returns NULL if wrong type.
vtkStructuredGrid *vtkCastToConcrete::GetStructuredGridOutput()
{
  if ( this->Input == NULL )
    {
    vtkErrorMacro(<<"Filter requires input to be set before output can be retrieved");
    }
  else
    {
    if ( ((vtkDataSet *)this->Input)->GetDataSetType() != VTK_STRUCTURED_GRID )
      {
      vtkErrorMacro(<<"Cannot cast to type requested");
      return NULL;
      }
    }

  return this->StructuredGrid;
}

// Get the output of this filter as type vtkUnstructuredGrid. Performs run-time
// checking on type. Returns NULL if wrong type.
vtkUnstructuredGrid *vtkCastToConcrete::GetUnstructuredGridOutput()
{
  if ( this->Input == NULL )
    {
    vtkErrorMacro(<<"Filter requires input to be set before output can be retrieved");
    }
  else
    {
    if ( ((vtkDataSet *)this->Input)->GetDataSetType() != VTK_UNSTRUCTURED_GRID )
      {
      vtkErrorMacro(<<"Cannot cast to type requested");
      return NULL;
      }
    }

  return this->UnstructuredGrid;
}

// Get the output of this filter as type vtkUnstructuredGrid. Performs run-time
// checking on type. Returns NULL if wrong type.
vtkRectilinearGrid *vtkCastToConcrete::GetRectilinearGridOutput()
{
  if ( this->Input == NULL )
    {
    vtkErrorMacro(<<"Filter requires input to be set before output can be retrieved");
    }
  else
    {
    if ( ((vtkDataSet *)this->Input)->GetDataSetType() != VTK_RECTILINEAR_GRID )
      {
      vtkErrorMacro(<<"Cannot cast to type requested");
      return NULL;
      }
    }

  return this->RectilinearGrid;
}

void vtkCastToConcrete::UnRegister(vtkObject *o)
{
  // detect the circular loop source <-> data
  // If we have two references and one of them is my data
  // and I am not being unregistered by my data, break the loop.
  if (this->ReferenceCount == 6 &&
      this->PolyData->GetNetReferenceCount() == 1 &&
      this->StructuredGrid->GetNetReferenceCount() == 1 &&
      this->UnstructuredGrid->GetNetReferenceCount() == 1 &&
      this->StructuredPoints->GetNetReferenceCount() == 1 &&
      this->RectilinearGrid->GetNetReferenceCount() == 1)
    {
    this->PolyData->SetSource(NULL);
    this->StructuredGrid->SetSource(NULL);
    this->UnstructuredGrid->SetSource(NULL);
    this->StructuredPoints->SetSource(NULL);
    this->RectilinearGrid->SetSource(NULL);
    }
  if (this->ReferenceCount == 5 &&
      (this->PolyData == o || this->StructuredGrid == o ||
       this->UnstructuredGrid == o || this->RectilinearGrid == o ||
       this->StructuredPoints == o) &&
      (this->PolyData->GetNetReferenceCount() +
       this->StructuredPoints->GetNetReferenceCount() +
       this->RectilinearGrid->GetNetReferenceCount() +
       this->StructuredGrid->GetNetReferenceCount() +
       this->UnstructuredGrid->GetNetReferenceCount()) == 6)
    {
    this->PolyData->SetSource(NULL);
    this->StructuredGrid->SetSource(NULL);
    this->UnstructuredGrid->SetSource(NULL);
    this->StructuredPoints->SetSource(NULL);
    this->RectilinearGrid->SetSource(NULL);
    }
  
  this->vtkObject::UnRegister(o);
}

int vtkCastToConcrete::InRegisterLoop(vtkObject *o)
{
  int num = 0;
  int cnum = 0;
  
  if (this->StructuredPoints->GetSource() == this)
    {
    num++;
    cnum += this->StructuredPoints->GetNetReferenceCount();
    }
  if (this->RectilinearGrid->GetSource() == this)
    {
    num++;
    cnum += this->RectilinearGrid->GetNetReferenceCount();
    }
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
       this->StructuredPoints == o ||
       this->RectilinearGrid == o ||
       this->StructuredGrid == o ||
       this->UnstructuredGrid == o))
    {
    return 1;
    }
  return 0;
}
