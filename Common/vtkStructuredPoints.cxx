/*=========================================================================


  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 1993-1995 Ken Martin, Will Schroeder, Bill Lorensen.

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
#include <math.h>
#include "vtkStructuredPoints.h"
#include "vtkImageToStructuredPoints.h"
#include "vtkStructuredExtent.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkStructuredPoints* vtkStructuredPoints::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkStructuredPoints");
  if(ret)
    {
    return (vtkStructuredPoints*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkStructuredPoints;
}




vtkStructuredPoints::vtkStructuredPoints()
{
  this->SetScalarType(VTK_FLOAT);
}


int vtkStructuredPoints::ClipUpdateExtentWithWholeExtent()
{
  this->GetStructuredUpdateExtent()->SetExtent(this->GetWholeExtent());
  return 1;
}

// I trying to solve a problem with image information.
// vtkDataSetToDataSetFilters do not have the same methods 
// to fill in information as vtkStructuredPointsSource.
void vtkStructuredPoints::InternalUpdate()
{
  vtkScalars *scalars;

  this->vtkImageData::InternalUpdate();
  scalars = this->GetPointData()->GetScalars();
  if (scalars)
    {
    this->SetScalarType(scalars->GetDataType());
    this->SetNumberOfScalarComponents(scalars->GetNumberOfComponents());
    }
}













