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
#include "vtkPointPicker.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkPointPicker* vtkPointPicker::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkPointPicker");
  if(ret)
    {
    return (vtkPointPicker*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkPointPicker;
}




vtkPointPicker::vtkPointPicker()
{
  this->PointId = -1;
}

float vtkPointPicker::IntersectWithLine(float p1[3], float p2[3], float tol, 
					vtkActor *assem, vtkActor *a, 
					vtkMapper *m)
{
  vtkDataSet *input=m->GetInput();
  int numPts;
  int ptId, i, minPtId;
  float ray[3], rayFactor, tMin, *p, t, projXYZ[3], minXYZ[3];

  if ( (numPts = input->GetNumberOfPoints()) < 1 )
    {
    return 2.0;
    }
//
//   Determine appropriate info
//
  for (i=0; i<3; i++)
    {
    ray[i] = p2[i] - p1[i];
    }
  if (( rayFactor = vtkMath::Dot(ray,ray)) == 0.0 ) 
    {
    vtkErrorMacro("Cannot process points");
    return 2.0;
    }
//
//  Project each point onto ray.  Keep track of the one within the
//  tolerance and closest to the eye (and within the clipping range).
//
  for (minPtId=(-1),tMin=VTK_LARGE_FLOAT,ptId=0; ptId<numPts; ptId++) 
    {
    p = input->GetPoint(ptId);

    t = (ray[0]*(p[0]-p1[0]) + ray[1]*(p[1]-p1[1]) + ray[2]*(p[2]-p1[2])) 
        / rayFactor;
//
//  If we find a point closer than we currently have, see whether it
//  lies within the pick tolerance and clipping planes.
//
    if ( t >= 0.0 && t <= 1.0 && t < tMin ) 
      {
      for(i=0; i<3; i++) 
        {
        projXYZ[i] = p1[i] + t*ray[i];
        if ( fabs(p[i]-projXYZ[i]) > tol )
	  {
	  break;
	  }
        }
      if ( i > 2 ) // within tolerance
        {
        minPtId = ptId;
        minXYZ[0]=p[0]; minXYZ[1]=p[1]; minXYZ[2]=p[2];
        tMin = t;
        }
      }
    }
//
//  Now compare this against other actors.
//
  if ( minPtId>(-1) && tMin < this->GlobalTMin ) 
    {
    this->MarkPicked(assem, a, m, tMin, minXYZ);
    this->PointId = minPtId;
    vtkDebugMacro("Picked point id= " << minPtId);
    }

  return tMin;
}

void vtkPointPicker::Initialize()
{
  this->PointId = (-1);
  this->vtkPicker::Initialize();
}

void vtkPointPicker::PrintSelf(vtkOstream& os, vtkIndent indent)
{
  this->vtkPicker::PrintSelf(os,indent);

  os << indent << "Point Id: " << this->PointId << "\n";
}
