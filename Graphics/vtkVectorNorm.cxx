/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include <math.h>
#include "vtkVectorNorm.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkVectorNorm* vtkVectorNorm::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkVectorNorm");
  if(ret)
    {
    return (vtkVectorNorm*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkVectorNorm;
}




// Construct with normalize flag off.
vtkVectorNorm::vtkVectorNorm()
{
  this->Normalize = 0;
  this->AttributeMode = VTK_ATTRIBUTE_MODE_DEFAULT;
}

void vtkVectorNorm::Execute()
{
  int i, numVectors, computePtScalars=1, computeCellScalars=1;
  vtkScalars *newScalars;
  float *v, s, maxScalar;
  vtkVectors *ptVectors, *cellVectors;
  vtkDataSet *input = this->GetInput();
  vtkDataSet *output = this->GetOutput();
  vtkPointData *pd=input->GetPointData(), *outPD=output->GetPointData();
  vtkCellData *cd=input->GetCellData(), *outCD=output->GetCellData();

  // Initialize
  vtkDebugMacro(<<"Computing norm of vectors!");

  // First, copy the input to the output as a starting point
  output->CopyStructure( input );

  ptVectors = pd->GetVectors();
  cellVectors = cd->GetVectors();
  if (!ptVectors || this->AttributeMode == VTK_ATTRIBUTE_MODE_USE_CELL_DATA)
    {
    computePtScalars = 0;
    }

  if (!cellVectors || this->AttributeMode == VTK_ATTRIBUTE_MODE_USE_POINT_DATA)
    {
    computeCellScalars = 0;
    }

  if ( !computeCellScalars && !computePtScalars )
    {
    vtkErrorMacro(<< "No vector norm to compute!");
    return;
    }

  // Allocate / operate on point data
  if ( computePtScalars )
    {
    numVectors = ptVectors->GetNumberOfVectors();
    newScalars = vtkScalars::New();
    newScalars->SetNumberOfScalars(numVectors);

    for (maxScalar=0.0, i=0; i < numVectors; i++)
      {
      v = ptVectors->GetVector(i);
      s = sqrt((double)v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
      if ( s > maxScalar )
	{
	maxScalar = s;
	}
      newScalars->SetScalar(i,s);

      if ( ! (i % 20000) ) 
        {
        vtkDebugMacro(<<"Computing point vector norm #" << i);
        this->UpdateProgress (0.5*i/numVectors);
        }
      }

    // If necessary, normalize
    if ( this->Normalize && maxScalar > 0.0 )
      {
      for (i=0; i < numVectors; i++)
        {
        s = newScalars->GetScalar(i);
        s /= maxScalar;
        newScalars->SetScalar(i,s);
        }
      }

    outPD->SetScalars(newScalars);
    newScalars->Delete();
    }//if computing point scalars

  // Allocate / operate on cell data
  if ( computeCellScalars )
    {
    numVectors = cellVectors->GetNumberOfVectors();
    newScalars = vtkScalars::New();
    newScalars->SetNumberOfScalars(numVectors);

    for (maxScalar=0.0, i=0; i < numVectors; i++)
      {
      v = cellVectors->GetVector(i);
      s = sqrt((double)v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
      if ( s > maxScalar )
	{
	maxScalar = s;
	}
      newScalars->SetScalar(i,s);
      if ( ! (i % 20000) ) 
        {
        vtkDebugMacro(<<"Computing cell vector norm #" << i);
        this->UpdateProgress (0.5*i/numVectors+0.5);
        }
      }

    // If necessary, normalize
    if ( this->Normalize && maxScalar > 0.0 )
      {
      for (i=0; i < numVectors; i++)
        {
        s = newScalars->GetScalar(i);
        s /= maxScalar;
        newScalars->SetScalar(i,s);
        }
      }

    outCD->SetScalars(newScalars);
    newScalars->Delete();
    }//if computing cell scalars

  // Pass appropriate data through to output
  outPD->PassNoReplaceData(pd);
  outCD->PassNoReplaceData(cd);
}

// Return the method for generating scalar data as a string.
char *vtkVectorNorm::GetAttributeModeAsString(void)
{
  if ( this->AttributeMode == VTK_ATTRIBUTE_MODE_DEFAULT )
    {
    return "Default";
    }
  else if ( this->AttributeMode == VTK_ATTRIBUTE_MODE_USE_POINT_DATA )
    {
    return "UsePointData";
    }
  else 
    {
    return "UseCellData";
    }
}

void vtkVectorNorm::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataSetToDataSetFilter::PrintSelf(os,indent);

  os << indent << "Normalize: " << (this->Normalize ? "On\n" : "Off\n");
  os << indent << "Attribute Mode: " << this->GetAttributeModeAsString() 
     << endl;
}

