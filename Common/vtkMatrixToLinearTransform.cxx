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
OF THIS EVEN, SOFTWARE IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkMatrixToLinearTransform.h"
#include "vtkLinearTransformInverse.h"
#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
vtkMatrixToLinearTransform* vtkMatrixToLinearTransform::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMatrixToLinearTransform");
  if(ret)
    {
    return (vtkMatrixToLinearTransform*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMatrixToLinearTransform;
}

//----------------------------------------------------------------------------
vtkMatrixToLinearTransform::vtkMatrixToLinearTransform()
{
  // delete the original matrix that was created by vtkPerspectiveTransform
  if (this->Matrix) 
    {
    this->Matrix->Delete();
    }
  this->Matrix = NULL;
}

//----------------------------------------------------------------------------
vtkMatrixToLinearTransform::~vtkMatrixToLinearTransform()
{
  // this->Matrix will be deleted by vtkPerspectiveTransform superclass
}

//----------------------------------------------------------------------------
void vtkMatrixToLinearTransform::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkLinearTransform::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkGeneralTransform *vtkMatrixToLinearTransform::MakeTransform()
{
  return vtkMatrixToLinearTransform::New();
}

//----------------------------------------------------------------------------
void vtkMatrixToLinearTransform::DeepCopy(vtkGeneralTransform *transform)
{
  if (strcmp("vtkLinearTransformInverse",transform->GetClassName())==0)
    {
    transform = ((vtkLinearTransformInverse *)transform)->GetTransform();
    }
  if (strcmp("vtkMatrixToLinearTransform",transform->GetClassName()) != 0)
    {
    vtkErrorMacro(<< "DeepCopy: trying to copy a transform of different type");
    return;
    }

  vtkMatrixToLinearTransform *t = (vtkMatrixToLinearTransform *)transform;  

  if (t == this)
    {
    return;
    }

  if (this->Matrix == NULL)
    {
    this->Matrix = vtkMatrix4x4::New();
    }
  this->Matrix->DeepCopy(t->Matrix);
  this->Modified();
}

//----------------------------------------------------------------------------
// Set the current matrix directly.
void vtkMatrixToLinearTransform::SetMatrix(vtkMatrix4x4 *m)
{
  if (this->Matrix != NULL)
    {
    vtkErrorMacro(<< "SetMatrix: once the matrix is set, you cannot change it.");
    return;
    }

  m->Register(this);
  this->Matrix = m;
  this->Modified();
}

//----------------------------------------------------------------------------
// Creates an identity matrix.
void vtkMatrixToLinearTransform::Identity()
{
  this->Matrix->Identity();
  this->Modified();
}

//----------------------------------------------------------------------------
// Inverts the matrix.
void vtkMatrixToLinearTransform::Inverse()
{
  this->Matrix->Invert();
  this->Modified();
}

//----------------------------------------------------------------------------
// Get the MTime
unsigned long vtkMatrixToLinearTransform::GetMTime()
{
  unsigned long mtime = this->vtkLinearTransform::GetMTime();
  unsigned long matrixMTime = 
    ((this->Matrix == NULL) ? 0 : this->Matrix->GetMTime());

  if (matrixMTime > mtime)
    {
    mtime = matrixMTime;
    }
  return mtime;
}
