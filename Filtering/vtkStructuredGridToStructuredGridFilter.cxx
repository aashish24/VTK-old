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
#include "vtkStructuredGridToStructuredGridFilter.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkStructuredGridToStructuredGridFilter* vtkStructuredGridToStructuredGridFilter::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkStructuredGridToStructuredGridFilter");
  if(ret)
    {
    return (vtkStructuredGridToStructuredGridFilter*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkStructuredGridToStructuredGridFilter;
}




//----------------------------------------------------------------------------
// Specify the input data or filter.
void vtkStructuredGridToStructuredGridFilter::SetInput(vtkStructuredGrid *input)
{
  this->vtkProcessObject::SetNthInput(0, input);
}

//----------------------------------------------------------------------------
// Specify the input data or filter.
vtkStructuredGrid *vtkStructuredGridToStructuredGridFilter::GetInput()
{
  if (this->NumberOfInputs < 1)
    {
    return NULL;
    }
  
  return (vtkStructuredGrid *)(this->Inputs[0]);
}


//----------------------------------------------------------------------------
// just copy WholeExtent fropm the first input.
void vtkStructuredGridToStructuredGridFilter::ExecuteInformation()
{
  vtkStructuredGrid *input = this->GetInput();
  vtkStructuredGrid *output = this->GetOutput();
  
  if (output == NULL || input == NULL)
    {
    return;
    }
  
  output->SetWholeExtent(input->GetWholeExtent());
}

//----------------------------------------------------------------------------
int vtkStructuredGridToStructuredGridFilter::ComputeInputUpdateExtents(
                                                           vtkDataObject *data)
{
  vtkStructuredGrid *output = (vtkStructuredGrid *)data;
  
  if (this->NumberOfInputs > 1)
    {
    vtkErrorMacro("Subclass did not implement ComputeInputUpdateExtent");
    return 0;
    }
  
  this->GetInput()->CopyUpdateExtent(output);
  return 1;
}

