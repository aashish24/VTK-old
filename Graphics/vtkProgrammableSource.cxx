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
#include "vtkProgrammableSource.h"
#include "vtkPolyData.h"
#include "vtkStructuredGrid.h"
#include "vtkStructuredPoints.h"
#include "vtkUnstructuredGrid.h"
#include "vtkRectilinearGrid.h"

// Construct programmable filter with empty execute method.
vtkProgrammableSource::vtkProgrammableSource()
{
  this->ExecuteMethod = NULL;
  this->ExecuteMethodArg = NULL;
  this->ExecuteMethodArgDelete = NULL;

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

  //This is done because filter superclass assumes output is defined.
  this->Output = this->PolyData;
}

vtkProgrammableSource::~vtkProgrammableSource()
{
  this->StructuredPoints->Delete();
  this->StructuredGrid->Delete();
  this->UnstructuredGrid->Delete();
  this->RectilinearGrid->Delete();
  this->PolyData->Delete();
  // Output should only be one of the above. We set it to NULL
  // so that we don't free it twice
  this->Output = NULL;

  // delete the current arg if there is one and a delete meth
  if ((this->ExecuteMethodArg)&&(this->ExecuteMethodArgDelete))
    {
    (*this->ExecuteMethodArgDelete)(this->ExecuteMethodArg);
    }
}

// Specify the function to use to generate the source data. Note
// that the function takes a single (void *) argument.
void vtkProgrammableSource::SetExecuteMethod(void (*f)(void *), void *arg)
{
  if ( f != this->ExecuteMethod || arg != this->ExecuteMethodArg )
    {
    // delete the current arg if there is one and a delete meth
    if ((this->ExecuteMethodArg)&&(this->ExecuteMethodArgDelete))
      {
      (*this->ExecuteMethodArgDelete)(this->ExecuteMethodArg);
      }
    this->ExecuteMethod = f;
    this->ExecuteMethodArg = arg;
    this->Modified();
    }
}

// Set the arg delete method. This is used to free user memory.
void vtkProgrammableSource::SetExecuteMethodArgDelete(void (*f)(void *))
{
  if ( f != this->ExecuteMethodArgDelete)
    {
    this->ExecuteMethodArgDelete = f;
    this->Modified();
    }
}


// Get the output as a concrete type. This method is typically used by the
// writer of the source function to get the output as a particular type (i.e.,
// it essentially does type casting). It is the users responsibility to know
// the correct type of the output data.
vtkPolyData *vtkProgrammableSource::GetPolyDataOutput()
{
  return this->PolyData;
}

// Get the output as a concrete type.
vtkStructuredPoints *vtkProgrammableSource::GetStructuredPointsOutput()
{
  return this->StructuredPoints;
}

// Get the output as a concrete type.
vtkStructuredGrid *vtkProgrammableSource::GetStructuredGridOutput()
{
  return this->StructuredGrid;
}

// Get the output as a concrete type.
vtkUnstructuredGrid *vtkProgrammableSource::GetUnstructuredGridOutput()
{
  return this->UnstructuredGrid;
}

// Get the output as a concrete type.
vtkRectilinearGrid *vtkProgrammableSource::GetRectilinearGridOutput()
{
  return this->RectilinearGrid;
}


void vtkProgrammableSource::Execute()
{
  vtkDebugMacro(<<"Executing programmable filter");

  // Now invoke the procedure, if specified.
  if ( this->ExecuteMethod != NULL )
    {
    (*this->ExecuteMethod)(this->ExecuteMethodArg);
    }
}

void vtkProgrammableSource::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkSource::PrintSelf(os,indent);

  os << indent << "Execute Time: " <<this->ExecuteTime.GetMTime() << "\n";

}


void vtkProgrammableSource::UnRegister(vtkObject *o)
{
  // detect the circular loop source <-> data
  // If we have two references and one of them is my data
  // and I am not being unregistered by my data, break the loop.
  if (this->ReferenceCount == 6 &&
      this->PolyData->GetReferenceCount() == 1 &&
      this->StructuredGrid->GetReferenceCount() == 1 &&
      this->UnstructuredGrid->GetReferenceCount() == 1 &&
      this->StructuredPoints->GetReferenceCount() == 1 &&
      this->RectilinearGrid->GetReferenceCount() == 1)
    {
    this->PolyData->SetSource(NULL);
    this->StructuredGrid->SetSource(NULL);
    this->UnstructuredGrid->SetSource(NULL);
    this->StructuredPoints->SetSource(NULL);
    this->RectilinearGrid->SetSource(NULL);
    }
  
  this->vtkObject::UnRegister(o);
}

int vtkProgrammableSource::InRegisterLoop(vtkObject *o)
{
  int num = 0;
  int cnum = 0;
  
  if (this->StructuredPoints->GetSource() == this)
    {
    num++;
    cnum += this->StructuredPoints->GetReferenceCount();
    }
  if (this->RectilinearGrid->GetSource() == this)
    {
    num++;
    cnum += this->RectilinearGrid->GetReferenceCount();
    }
  if (this->PolyData->GetSource() == this)
    {
    num++;
    cnum += this->PolyData->GetReferenceCount();
    }
  if (this->StructuredGrid->GetSource() == this)
    {
    num++;
    cnum += this->StructuredGrid->GetReferenceCount();
    }
  if (this->UnstructuredGrid->GetSource() == this)
    {
    num++;
    cnum += this->UnstructuredGrid->GetReferenceCount();
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

