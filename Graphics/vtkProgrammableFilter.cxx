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
#include "vtkProgrammableFilter.h"
#include "vtkPolyData.h"
#include "vtkStructuredGrid.h"
#include "vtkStructuredPoints.h"
#include "vtkUnstructuredGrid.h"
#include "vtkRectilinearGrid.h"

// Construct programmable filter with empty execute method.
vtkProgrammableFilter::vtkProgrammableFilter()
{
  this->ExecuteMethod = NULL;
  this->ExecuteMethodArg = NULL;

  this->OutputPolyData = vtkPolyData::New();
  this->OutputPolyData->SetSource(this);
  
  this->OutputStructuredPoints = vtkStructuredPoints::New();
  this->OutputStructuredPoints->SetSource(this);
  
  this->OutputStructuredGrid = vtkStructuredGrid::New();
  this->OutputStructuredGrid->SetSource(this);
  
  this->OutputUnstructuredGrid = vtkUnstructuredGrid::New();
  this->OutputUnstructuredGrid->SetSource(this);
  
  this->OutputRectilinearGrid = vtkRectilinearGrid::New();
  this->OutputRectilinearGrid->SetSource(this);

  //This is done because filter superclass assumes output is defined.
  this->Output = this->OutputPolyData;
}

vtkProgrammableFilter::~vtkProgrammableFilter()
{
  this->OutputPolyData->Delete();
  this->OutputStructuredPoints->Delete();
  this->OutputStructuredGrid->Delete();
  this->OutputUnstructuredGrid->Delete();
  this->OutputRectilinearGrid->Delete();
  // Output should only be one of the above. We set it to NULL
  // so that we don't free it twice
  this->Output = NULL;
  // delete the current arg if there is one and a delete meth
  if ((this->ExecuteMethodArg)&&(this->ExecuteMethodArgDelete))
    {
    (*this->ExecuteMethodArgDelete)(this->ExecuteMethodArg);
    }
}

// Specify the input data or filter.
void vtkProgrammableFilter::SetInput(vtkDataSet *input)
{
  if ( this->Input != input )
    {
    vtkDebugMacro(<<" setting Input to " << (void *)input);
    if (this->Input) {this->Input->UnRegister(this);}
    this->Input = input;
    if (this->Input) {this->Input->Register(this);}
    this->Modified();
    }
}

// Get the input as a concrete type. This method is typically used by the
// writer of the filter function to get the input as a particular type (i.e.,
// it essentially does type casting). It is the users responsibility to know
// the correct type of the input data.
vtkPolyData *vtkProgrammableFilter::GetPolyDataInput()
{
  return (vtkPolyData *)this->Input;
}

// Get the input as a concrete type.
vtkStructuredPoints *vtkProgrammableFilter::GetStructuredPointsInput()
{
  return (vtkStructuredPoints *)this->Input;
}

// Get the input as a concrete type.
vtkStructuredGrid *vtkProgrammableFilter::GetStructuredGridInput()
{
  return (vtkStructuredGrid *)this->Input;
}

// Get the input as a concrete type.
vtkUnstructuredGrid *vtkProgrammableFilter::GetUnstructuredGridInput()
{
  return (vtkUnstructuredGrid *)this->Input;
}

// Get the input as a concrete type.
vtkRectilinearGrid *vtkProgrammableFilter::GetRectilinearGridInput()
{
  return (vtkRectilinearGrid *)this->Input;
}

// Specify the function to use to operate on the point attribute data. Note
// that the function takes a single (void *) argument.
void vtkProgrammableFilter::SetExecuteMethod(void (*f)(void *), void *arg)
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
void vtkProgrammableFilter::SetExecuteMethodArgDelete(void (*f)(void *))
{
  if ( f != this->ExecuteMethodArgDelete)
    {
    this->ExecuteMethodArgDelete = f;
    this->Modified();
    }
}


// Get the output as a concrete type. This method is typically used by the
// writer of the filter function to get the output as a particular type (i.e.,
// it essentially does type casting). It is the users responsibility to know
// the correct type of the output data.
vtkPolyData *vtkProgrammableFilter::GetPolyDataOutput()
{
  return this->OutputPolyData;
}

// Get the output as a concrete type.
vtkStructuredPoints *vtkProgrammableFilter::GetStructuredPointsOutput()
{
  return this->OutputStructuredPoints;
}

// Get the output as a concrete type.
vtkStructuredGrid *vtkProgrammableFilter::GetStructuredGridOutput()
{
  return this->OutputStructuredGrid;
}

// Get the output as a concrete type.
vtkUnstructuredGrid *vtkProgrammableFilter::GetUnstructuredGridOutput()
{
  return this->OutputUnstructuredGrid;
}

// Get the output as a concrete type.
vtkRectilinearGrid *vtkProgrammableFilter::GetRectilinearGridOutput()
{
  return this->OutputRectilinearGrid;
}


void vtkProgrammableFilter::Execute()
{
  vtkDebugMacro(<<"Executing programmable filter");

  // Now invoke the procedure, if specified.
  if ( this->ExecuteMethod != NULL )
    {
    (*this->ExecuteMethod)(this->ExecuteMethodArg);
    }
}

void vtkProgrammableFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkSource::PrintSelf(os,indent);

  os << indent << "Execute Time: " <<this->ExecuteTime.GetMTime() << "\n";

  if ( this->Input )
    {
    os << indent << "Input: (" << (void *)this->Input << ")\n";
    }
  else
    {
    os << indent << "Input: (none)\n";
    }
}


void vtkProgrammableFilter::UnRegister(vtkObject *o)
{
  // detect the circular loop source <-> data
  // If we have two references and one of them is my data
  // and I am not being unregistered by my data, break the loop.
  if (this->ReferenceCount == 6 &&
      this->OutputPolyData->GetReferenceCount() == 1 &&
      this->OutputStructuredGrid->GetReferenceCount() == 1 &&
      this->OutputUnstructuredGrid->GetReferenceCount() == 1 &&
      this->OutputStructuredPoints->GetReferenceCount() == 1 &&
      this->OutputRectilinearGrid->GetReferenceCount() == 1)
    {
    this->OutputPolyData->SetSource(NULL);
    this->OutputStructuredGrid->SetSource(NULL);
    this->OutputUnstructuredGrid->SetSource(NULL);
    this->OutputStructuredPoints->SetSource(NULL);
    this->OutputRectilinearGrid->SetSource(NULL);
    }
  
  this->vtkObject::UnRegister(o);
}

int vtkProgrammableFilter::InRegisterLoop(vtkObject *o)
{
  int num = 0;
  int cnum = 0;
  
  if (this->OutputStructuredPoints->GetSource() == this)
    {
    num++;
    cnum += this->OutputStructuredPoints->GetNetReferenceCount();
    }
  if (this->OutputRectilinearGrid->GetSource() == this)
    {
    num++;
    cnum += this->OutputRectilinearGrid->GetNetReferenceCount();
    }
  if (this->OutputPolyData->GetSource() == this)
    {
    num++;
    cnum += this->OutputPolyData->GetNetReferenceCount();
    }
  if (this->OutputStructuredGrid->GetSource() == this)
    {
    num++;
    cnum += this->OutputStructuredGrid->GetNetReferenceCount();
    }
  if (this->OutputUnstructuredGrid->GetSource() == this)
    {
    num++;
    cnum += this->OutputUnstructuredGrid->GetNetReferenceCount();
    }
  
  // if no one outside is using us
  // and our data objects are down to one net reference
  // and we are being asked by one of our data objects
  if (this->ReferenceCount == num &&
      cnum == (num + 1) &&
      (this->OutputPolyData == o ||
       this->OutputStructuredPoints == o ||
       this->OutputRectilinearGrid == o ||
       this->OutputStructuredGrid == o ||
       this->OutputUnstructuredGrid == o))
    {
    return 1;
    }
  return 0;
}
