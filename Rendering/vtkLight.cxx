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
#include "vtkLight.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkGraphicsFactory.h"

// Create a light with the focal point at the origin and its position
// set to (0,0,1). The lights color is white, intensity=1, and the light 
// is turned on. 
vtkLight::vtkLight()
{
  this->FocalPoint[0] = 0.0;
  this->FocalPoint[1] = 0.0;
  this->FocalPoint[2] = 0.0;

  this->Position[0] = 0.0;
  this->Position[1] = 0.0;
  this->Position[2] = 1.0;

  this->Color[0] = 1.0;
  this->Color[1] = 1.0;
  this->Color[2] = 1.0;

  this->Switch = 1;

  this->Intensity = 1.0;
  this->Positional = 0;
  this->ConeAngle= 30;
  this->AttenuationValues[0] = 1;
  this->AttenuationValues[1] = 0;
  this->AttenuationValues[2] = 0;
  this->Exponent = 1;
}

// return the correct type of light 
vtkLight *vtkLight::New()
{ 
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkGraphicsFactory::CreateInstance("vtkLight");
  return (vtkLight*)ret;
}

void vtkLight::DeepCopy(vtkLight *light)
{
  this->SetFocalPoint(light->GetFocalPoint());
  this->SetPosition(light->GetPosition());
  this->SetIntensity(light->GetIntensity());
  this->SetColor(light->GetColor());
  this->SetSwitch(light->GetSwitch());
  this->SetPositional(light->GetPositional());
  this->SetExponent(light->GetExponent());
  this->SetConeAngle(light->GetConeAngle());
  this->SetAttenuationValues(light->GetAttenuationValues());
}

void vtkLight::PrintSelf(vtkOstream& os, vtkIndent indent)
{
  vtkObject::PrintSelf(os,indent);

  os << indent << "AttenuationValues: (" << this->AttenuationValues[0] << ", " 
    << this->AttenuationValues[1] << ", " << this->AttenuationValues[2] << ")\n";
  os << indent << "Color: (" << this->Color[0] << ", " 
    << this->Color[1] << ", " << this->Color[2] << ")\n";
  os << indent << "Cone Angle: " << this->ConeAngle << "\n";
  os << indent << "Exponent: " << this->Exponent << "\n";
  os << indent << "Focal Point: (" << this->FocalPoint[0] << ", " 
    << this->FocalPoint[1] << ", " << this->FocalPoint[2] << ")\n";
  os << indent << "Intensity: " << this->Intensity << "\n";
  os << indent << "Position: (" << this->Position[0] << ", " 
    << this->Position[1] << ", " << this->Position[2] << ")\n";
  os << indent << "Positional: " << (this->Positional ? "On\n" : "Off\n");
  os << indent << "Switch: " << (this->Switch ? "On\n" : "Off\n");
}


void vtkLight::WriteSelf(vtkOstream& os)
{
  os << this->FocalPoint[0] << " " << this->FocalPoint[1] << " "
     << this->FocalPoint[2] << " ";
  os << this->Position[0] << " " << this->Position[1] << " "
     << this->Position[2] << " ";
  os << this->Intensity << " ";
  os << this->Color[0] << " " << this->Color[1] << " "
     << this->Color[2] << " ";
  os << this->Switch << " ";
  os << this->Positional << " ";
  os << this->Exponent << " ";
  os << this->ConeAngle << " ";
  os << this->AttenuationValues[0] << " " << this->AttenuationValues[1] << " "
     << this->AttenuationValues[2] << " ";
}

void vtkLight::ReadSelf(vtkIstream& is)
{
  is >> this->FocalPoint[0] >> this->FocalPoint[1] >> this->FocalPoint[2] ;
  is >> this->Position[0] >> this->Position[1] >> this->Position[2];
  is >> this->Intensity;
  is >> this->Color[0] >> this->Color[1] >> this->Color[2];
  is >> this->Switch;
  is >> this->Positional;
  is >> this->Exponent;
  is >> this->ConeAngle;
  is >> this->AttenuationValues[0] >> this->AttenuationValues[1] 
     >> this->AttenuationValues[2];
}



