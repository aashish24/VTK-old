/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Library. No part of this file or its
contents may be copied, reproduced or altered in any way without the express
written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994

=========================================================================*/
#include <stdlib.h>
#include <iostream.h>
#include "Light.hh"

vlLight::vlLight()
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
}


void vlLight::PrintSelf(ostream& os, vlIndent indent)
{
  if (this->ShouldIPrint(vlLight::GetClassName()))
    {
    vlObject::PrintSelf(os,indent);

    os << indent << "Color: (" << this->Color[0] << ", " 
      << this->Color[1] << ", " << this->Color[2] << ")\n";
    os << indent << "Focal Point: (" << this->FocalPoint[0] << ", " 
      << this->FocalPoint[1] << ", " << this->FocalPoint[2] << ")\n";
    os << indent << "Intensity: " << this->Intensity << "\n";
    os << indent << "Position: (" << this->Position[0] << ", " 
      << this->Position[1] << ", " << this->Position[2] << ")\n";
    os << indent << "Switch: " << (this->Switch ? "On\n" : "Off\n");
    }
}




