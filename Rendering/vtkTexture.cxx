/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Toolkit. No part of this file or its
contents may be copied, reproduced or altered in any way without the express
written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994

=========================================================================*/
#include <stdlib.h>
#include "Texture.hh"
#include "Renderer.hh"
#include "RenderW.hh"
#include "TextDev.hh"

// Description:
// Construct object and initialize.
vtkTexture::vtkTexture()
{
  this->Repeat = 1;
  this->Interpolate = 0;

  this->Input = NULL;
  this->Device = NULL;
}

void vtkTexture::Load(vtkRenderer *ren)
{
  if (!this->Device)
    {
    this->Device = ren->GetRenderWindow()->MakeTexture();
    }
  this->Device->Load(this,ren);
}

void vtkTexture::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkObject::PrintSelf(os,indent);

  os << indent << "Interpolate: " << (this->Interpolate ? "On\n" : "Off\n");
  os << indent << "Repeat:      " << (this->Repeat ? "On\n" : "Off\n");
  if ( this->Input )
    {
    os << indent << "Input: (" << (void *)this->Input << ")\n";
    }
  else
    {
    os << indent << "Input: (none)\n";
    }
}



void vtkTexture::Render(vtkRenderer *ren)
{
  if (this->Input) //load texture map
    {
    this->Input->Update();
    if (this->Input->GetMTime() > this->GetMTime())
      {
      this->Load(ren);
      }
    }
}
