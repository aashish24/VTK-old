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
#include "vtkVectors.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkVectors* vtkVectors::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkVectors");
  if(ret)
    {
    return (vtkVectors*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkVectors;
}




vtkVectors *vtkVectors::New(int dataType)
{
  vtkVectors *res = vtkVectors::New();
  res->SetDataType(dataType);
  res->GetData()->SetNumberOfComponents(3);
  return res;
}

// Construct object with an initial data array of type float.
vtkVectors::vtkVectors() 
{
  this->MaxNorm = 0.0;
  this->Data->SetNumberOfComponents(3);
}

// Given a list of pt ids, return an array of vectors.
void vtkVectors::GetVectors(vtkIdList *ptIds, vtkVectors *v)
{
  float vector[3];
  int num=ptIds->GetNumberOfIds();
  
  v->SetNumberOfVectors(num);
  for (int i=0; i<num; i++)
    {
    this->GetVector(ptIds->GetId(i),vector);
    v->SetVector(i,vector);
    }
}

// Compute the largest norm for these vectors.
void vtkVectors::ComputeMaxNorm()
{
  int i;
  float *v, norm;

  if ( this->GetMTime() > this->ComputeTime )
    {
    this->MaxNorm = 0.0;
    for (i=0; i<this->GetNumberOfVectors(); i++)
      {
      v = this->GetVector(i);
      norm = vtkMath::Norm(v);
      if ( norm > this->MaxNorm )
	{
	this->MaxNorm = norm;
	}
      }

    this->ComputeTime.Modified();
    }
}

// Return the maximum norm for these vectors.
float vtkVectors::GetMaxNorm()
{
  this->ComputeMaxNorm();
  return this->MaxNorm;
}

void vtkVectors::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkAttributeData::PrintSelf(os,indent);

  os << indent << "Number Of Vectors: " << this->GetNumberOfVectors() << "\n";
  os << indent << "Maximum Euclidean Norm: " << this->GetMaxNorm() << "\n";
}

