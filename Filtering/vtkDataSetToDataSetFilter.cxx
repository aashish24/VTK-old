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
#include "vtkDataSetToDataSetFilter.h"
#include "vtkPolyData.h"
#include "vtkStructuredGrid.h"
#include "vtkStructuredPoints.h"
#include "vtkUnstructuredGrid.h"
#include "vtkRectilinearGrid.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkDataSetToDataSetFilter* vtkDataSetToDataSetFilter::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkDataSetToDataSetFilter");
  if(ret)
    {
    return (vtkDataSetToDataSetFilter*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkDataSetToDataSetFilter;
}




// Construct object.
vtkDataSetToDataSetFilter::vtkDataSetToDataSetFilter()
{
}

vtkDataSetToDataSetFilter::~vtkDataSetToDataSetFilter()
{
}

//----------------------------------------------------------------------------
// Specify the input data or filter.
void vtkDataSetToDataSetFilter::SetInput(vtkDataSet *input)
{
  vtkDataSet *oldInput = this->GetInput();
  
  if (oldInput != NULL)
    {
    if (input == NULL || 
	oldInput->GetDataObjectType() != input->GetDataObjectType())
      {
      vtkWarningMacro("Changing input type.  Deleting output");
      this->SetOutput(NULL);
      }
    }
  
  if (input != NULL && this->vtkSource::GetOutput(0) == NULL)
    {
    this->vtkSource::SetNthOutput(0, input->MakeObject());
    this->Outputs[0]->ReleaseData();
    this->Outputs[0]->Delete();
    }
  
  this->vtkProcessObject::SetNthInput(0, input);
}

// Get the output of this filter. If output is NULL then input hasn't been set
// which is necessary for abstract objects.
vtkDataSet *vtkDataSetToDataSetFilter::GetOutput()
{
  if (this->GetInput() == NULL )
    {
    vtkErrorMacro(<<"Abstract filters require input to be set before output can be retrieved");
    return NULL;
    }

  // sanity check
  if (this->NumberOfOutputs < 1)
    {
    vtkErrorMacro("Sanity check failed. We should have an output");
    return NULL;
    }

  return (vtkDataSet *)this->Outputs[0];
}

// Get the output as vtkPolyData.
vtkPolyData *vtkDataSetToDataSetFilter::GetPolyDataOutput() 
{
  vtkDataSet *ds = this->GetOutput();
  if (!ds) 
    {
    return NULL;
    }
  if (ds->GetDataObjectType() == VTK_POLY_DATA)
    {
    return (vtkPolyData *)ds;
    }
  return NULL;
}

// Get the output as vtkStructuredPoints.
vtkStructuredPoints *vtkDataSetToDataSetFilter::GetStructuredPointsOutput() 
{
  vtkDataSet *ds = this->GetOutput();
  if (!ds) 
    {
    return NULL;
    }
  if (ds->GetDataObjectType() == VTK_STRUCTURED_POINTS)
    {
    return (vtkStructuredPoints *)ds;
    }
  return NULL;
}

// Get the output as vtkStructuredGrid.
vtkStructuredGrid *vtkDataSetToDataSetFilter::GetStructuredGridOutput()
{
  vtkDataSet *ds = this->GetOutput();
  if (!ds) 
    {
    return NULL;
    }
  if (ds->GetDataObjectType() == VTK_STRUCTURED_GRID)
    {
    return (vtkStructuredGrid *)ds;
    }
  return NULL;
}

// Get the output as vtkUnstructuredGrid.
vtkUnstructuredGrid *vtkDataSetToDataSetFilter::GetUnstructuredGridOutput()
{
  vtkDataSet *ds = this->GetOutput();
  if (!ds) 
    {
    return NULL;
    }
  if (ds->GetDataObjectType() == VTK_UNSTRUCTURED_GRID)
    {
    return (vtkUnstructuredGrid *)ds;
    }
  return NULL;
}

// Get the output as vtkRectilinearGrid. 
vtkRectilinearGrid *vtkDataSetToDataSetFilter::GetRectilinearGridOutput()
{
  vtkDataSet *ds = this->GetOutput();
  if (!ds) 
    {
    return NULL;
    }
  if (ds->GetDataObjectType() == VTK_RECTILINEAR_GRID)
    {
    return (vtkRectilinearGrid *)ds;
    }
  return NULL;
}



//----------------------------------------------------------------------------
// Specify the input data or filter.
vtkDataSet *vtkDataSetToDataSetFilter::GetInput()
{
  if (this->NumberOfInputs < 1)
    {
    return NULL;
    }
  
  return (vtkDataSet *)(this->Inputs[0]);
}

//----------------------------------------------------------------------------
// We know input and output match in type - call the type specific version of
// copy information
void vtkDataSetToDataSetFilter::ExecuteInformation()
{
  this->GetOutput()->CopyTypeSpecificInformation( this->GetInput() );
}

//----------------------------------------------------------------------------

// Copy the update information across
void vtkDataSetToDataSetFilter::ComputeInputUpdateExtents(vtkDataObject *output)
{
  vtkDataObject *input = this->GetInput();

  input->SetUpdatePiece( output->GetUpdatePiece() );
  input->SetUpdateNumberOfPieces( output->GetUpdateNumberOfPieces() );
  input->SetUpdateExtent( output->GetUpdateExtent() );
}
