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
#include "vtkSphereSource.h"
#include "vtkPoints.h"
#include "vtkNormals.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkSphereSource* vtkSphereSource::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkSphereSource");
  if(ret)
    {
    return (vtkSphereSource*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkSphereSource;
}




//----------------------------------------------------------------------------
// Construct sphere with radius=0.5 and default resolution 8 in both Phi
// and Theta directions. Theta ranges from (0,360) and phi (0,180) degrees.
vtkSphereSource::vtkSphereSource(int res)
{
  res = res < 4 ? 4 : res;
  this->Radius = 0.5;
  this->Center[0] = 0.0;
  this->Center[1] = 0.0;
  this->Center[2] = 0.0;

  this->ThetaResolution = res;
  this->PhiResolution = res;
  this->StartTheta = 0.0;
  this->EndTheta = 360.0;
  this->StartPhi = 0.0;
  this->EndPhi = 180.0;
}

//----------------------------------------------------------------------------
void vtkSphereSource::Execute()
{
  int i, j;
  int jStart, jEnd, numOffset;
  int numPts, numPolys;
  vtkPoints *newPoints; 
  vtkNormals *newNormals;
  vtkCellArray *newPolys;
  float x[3], n[3], deltaPhi, deltaTheta, phi, theta, radius, norm;
  float startTheta, endTheta, startPhi, endPhi;
  int pts[3], base, numPoles=0, thetaResolution, phiResolution;
  vtkPolyData *output = this->GetOutput();
  //
  // Set things up; allocate memory
  //

  vtkDebugMacro("SphereSource Executing");

  numPts = this->PhiResolution * this->ThetaResolution + 2;
  // creating triangles
  numPolys = this->PhiResolution * 2 * this->ThetaResolution;

  newPoints = vtkPoints::New();
  newPoints->Allocate(numPts);
  newNormals = vtkNormals::New();
  newNormals->Allocate(numPts);
  newPolys = vtkCellArray::New();
  newPolys->Allocate(newPolys->EstimateSize(numPolys,3));
  //
  // Create sphere
  //
  // Create north pole if needed
  if ( this->StartPhi <= 0.0 )
    {
    x[0] = this->Center[0];
    x[1] = this->Center[1];
    x[2] = this->Center[2] + this->Radius;
    newPoints->InsertPoint(numPoles,x);

    x[0] = x[1] = 0.0; x[2] = 1.0;
    newNormals->InsertNormal(numPoles,x);
    numPoles++;
    }

  // Create south pole if needed
  if ( this->EndPhi >= 180.0 )
    {
    x[0] = this->Center[0];
    x[1] = this->Center[1];
    x[2] = this->Center[2] - this->Radius;
    newPoints->InsertPoint(numPoles,x);

    x[0] = x[1] = 0.0; x[2] = -1.0;
    newNormals->InsertNormal(numPoles,x);
    numPoles++;
    }

  // Check data, determine increments, and convert to radians
  startTheta = (this->StartTheta < this->EndTheta ? this->StartTheta : this->EndTheta);
  startTheta *= vtkMath::Pi() / 180.0;
  endTheta = (this->EndTheta > this->StartTheta ? this->EndTheta : this->StartTheta);
  endTheta *= vtkMath::Pi() / 180.0;

  startPhi = (this->StartPhi < this->EndPhi ? this->StartPhi : this->EndPhi);
  startPhi *= vtkMath::Pi() / 180.0;
  endPhi = (this->EndPhi > this->StartPhi ? this->EndPhi : this->StartPhi);
  endPhi *= vtkMath::Pi() / 180.0;


  phiResolution = this->PhiResolution - numPoles;
  deltaPhi = (endPhi - startPhi) / (this->PhiResolution - 1);
  thetaResolution = (fabs(this->StartTheta - this->EndTheta) >= 360.0 ?
		     this->ThetaResolution : this->ThetaResolution - 1);
  deltaTheta = (endTheta - startTheta) / thetaResolution;


  jStart = (this->StartPhi <= 0.0 ? 1 : 0);
  jEnd = (this->EndPhi >= 180.0 ? this->PhiResolution - 1 
        : this->PhiResolution);


  // Create intermediate points
  for (i=0; i < this->ThetaResolution; i++)
    {
    theta = startTheta + i*deltaTheta;
    for (j=jStart; j<jEnd; j++)
      {
      phi = startPhi + j*deltaPhi;
      radius = this->Radius * sin((double)phi);
      n[0] = radius * cos((double)theta);
      n[1] = radius * sin((double)theta);
      n[2] = this->Radius * cos((double)phi);
      x[0] = n[0] + this->Center[0];
      x[1] = n[1] + this->Center[1];
      x[2] = n[2] + this->Center[2];
      newPoints->InsertNextPoint(x);

      if ( (norm = vtkMath::Norm(n)) == 0.0 )
	{
	norm = 1.0;
	}
      n[0] /= norm; n[1] /= norm; n[2] /= norm; 
      newNormals->InsertNextNormal(n);
      }
    }

  // Generate mesh connectivity
  base = phiResolution * this->ThetaResolution;
 
  if ( this->StartPhi <= 0.0 )  // around north pole
    {
    for (i=0; i < thetaResolution; i++)
      {
      pts[0] = phiResolution*i + numPoles;
      pts[1] = (phiResolution*(i+1) % base) + numPoles;
      pts[2] = 0;
      newPolys->InsertNextCell(3,pts);
      }
    }
  
  if ( this->EndPhi >= 180.0 ) // around south pole
    {
    numOffset = phiResolution - 1 + numPoles;


    for (i=0; i < thetaResolution; i++)
      {
      pts[0] = phiResolution*i + numOffset;
      pts[2] = ((phiResolution*(i+1)) % base) + numOffset;
      pts[1] = numPoles - 1;
      newPolys->InsertNextCell(3,pts);
      }
    }

  // bands inbetween poles
  for (i=0; i < thetaResolution; i++)
    {
    for (j=0; j < (phiResolution-1); j++)
      {
      pts[0] = phiResolution*i + j + numPoles;
      pts[1] = pts[0] + 1;
      pts[2] = ((phiResolution*(i+1)+j) % base) + numPoles + 1;
      newPolys->InsertNextCell(3,pts);

      pts[1] = pts[2];
      pts[2] = pts[1] - 1;
      newPolys->InsertNextCell(3,pts);
      }
    }
  //
  // Update ourselves and release memeory
  //
  output->SetPoints(newPoints);
  newPoints->Delete();

  output->GetPointData()->SetNormals(newNormals);
  newNormals->Delete();

  output->SetPolys(newPolys);
  newPolys->Delete();
}

void vtkSphereSource::PrintSelf(vtkOstream& os, vtkIndent indent)
{
  vtkPolyDataSource::PrintSelf(os,indent);

  os << indent << "Theta Resolution: " << this->ThetaResolution << "\n";
  os << indent << "Phi Resolution: " << this->PhiResolution << "\n";
  os << indent << "Theta Start: " << this->StartTheta << "\n";
  os << indent << "Phi Start: " << this->StartPhi << "\n";
  os << indent << "Theta End: " << this->EndTheta << "\n";
  os << indent << "Phi End: " << this->EndPhi << "\n";
  os << indent << "Radius: " << this->Radius << "\n";
  os << indent << "Center: (" << this->Center[0] << ", " 
     << this->Center[1] << ", " << this->Center[2] << ")\n";
}


//----------------------------------------------------------------------------
void vtkSphereSource::ExecuteInformation()
{
  int numTris, numPts;
  unsigned long size;
  
  // ignore poles
  numPts = this->ThetaResolution * (this->PhiResolution + 1);
  numTris = this->ThetaResolution * this->PhiResolution * 2;
  size = numPts * 3 * sizeof(float);
  size += numTris * 4 * sizeof(int);
  
  // convert to kilobytes
  size = (size / 1000) + 1;
  
}
