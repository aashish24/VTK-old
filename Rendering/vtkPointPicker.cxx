/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkPointPicker.h"

#include "vtkImageData.h"
#include "vtkMath.h"
#include "vtkMapper.h"
#include "vtkVolumeMapper.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkPointPicker, "$Revision$");
vtkStandardNewMacro(vtkPointPicker);

vtkPointPicker::vtkPointPicker()
{
  this->PointId = -1;
}

float vtkPointPicker::IntersectWithLine(float p1[3], float p2[3], float tol, 
                                        vtkAssemblyPath *path, vtkProp3D *p, 
                                        vtkAbstractMapper3D *m)
{
  vtkIdType numPts;
  vtkIdType ptId, minPtId;
  int i;
  float ray[3], rayFactor, tMin, x[3], t, projXYZ[3], minXYZ[3];
  vtkDataSet *input;
  vtkMapper *mapper;
  vtkVolumeMapper *volumeMapper;

  // Get the underlying dataset
  //
  if ( (mapper=vtkMapper::SafeDownCast(m)) != NULL )
    {
    input = mapper->GetInput();
    }
  else if ( (volumeMapper=vtkVolumeMapper::SafeDownCast(m)) != NULL )
    {
    input = volumeMapper->GetInput();
    }
  else
    {
    return 2.0;
    }

  if ( (numPts = input->GetNumberOfPoints()) < 1 )
    {
    return 2.0;
    }

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

  //  Project each point onto ray.  Keep track of the one within the
  //  tolerance and closest to the eye (and within the clipping range).
  //
  float dist, maxDist, minPtDist=VTK_LARGE_FLOAT;
  for (minPtId=(-1),tMin=VTK_LARGE_FLOAT,ptId=0; ptId<numPts; ptId++) 
    {
    input->GetPoint(ptId,x);

    t = (ray[0]*(x[0]-p1[0]) + ray[1]*(x[1]-p1[1]) + ray[2]*(x[2]-p1[2])) 
        / rayFactor;

    // If we find a point closer than we currently have, see whether it
    // lies within the pick tolerance and clipping planes. We keep track
    // of the point closest to the line (use a fudge factor for points
    // nearly the same distance away.)
    //
    if ( t >= 0.0 && t <= 1.0 && t <= (tMin+this->Tolerance) ) 
      {
      for(maxDist=0.0f, i=0; i<3; i++) 
        {
        projXYZ[i] = p1[i] + t*ray[i];
        dist = fabs(x[i]-projXYZ[i]);
        if ( dist > maxDist )
          {
          maxDist = dist;
          }
        }
      if ( maxDist <= tol && maxDist < minPtDist ) // within tolerance
        {
        minPtId = ptId;
        minXYZ[0]=x[0]; minXYZ[1]=x[1]; minXYZ[2]=x[2];
        minPtDist = maxDist;
        tMin = t; 
       }
      }
    }

  //  Now compare this against other actors.
  //
  if ( minPtId>(-1) && tMin < this->GlobalTMin ) 
    {
    this->MarkPicked(path, p, m, tMin, minXYZ);
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

void vtkPointPicker::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Point Id: " << this->PointId << "\n";
}
