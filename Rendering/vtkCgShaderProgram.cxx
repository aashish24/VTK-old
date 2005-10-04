/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*
 * Copyright 2003 Sandia Corporation.
 * Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
 * license for use of this work by or on behalf of the
 * U.S. Government. Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that this Notice and any
 * statement of authorship are reproduced on all copies.
 */

#include "vtkCgShaderProgram.h"
#include "vtkCgShader.h"

#include <vtkXMLMaterialReader.h>
#include <vtkActor.h>
#include <vtkRenderer.h>

#include <vtkObjectFactory.h>


vtkCxxRevisionMacro(vtkCgShaderProgram, "$Revision$");
vtkStandardNewMacro(vtkCgShaderProgram);

//----------------------------------------------------------------------------
vtkCgShaderProgram::vtkCgShaderProgram()
{
  // replace superclass's default shaders with cg-specific ones
  vtkShader* shader = vtkCgShader::New();
  this->SetVertexShader(shader);
  shader->Delete();

  shader = vtkCgShader::New();
  this->SetFragmentShader(shader);
  shader->Delete();
}

//----------------------------------------------------------------------------
vtkCgShaderProgram::~vtkCgShaderProgram()
{
}

//----------------------------------------------------------------------------
void vtkCgShaderProgram::Render(vtkActor *actor, vtkRenderer *renderer )
{
  // Cg requires no communication between vertex and fragment shaders;
  // each can be installed in hardware independently. There's really
  // nothing for vtkCgShaderProgram to do but delegate all shader mechanics
  // tasks to it's vertex and fragment shaders.
  if (this->VertexShader)
    {
    if (this->VertexShader->Compile())
      {
      this->VertexShader->PassShaderVariables(actor, renderer);
      this->VertexShader->Bind();
      }
    }
  if (this->FragmentShader)
    {
    if (this->FragmentShader->Compile())
      {
      this->FragmentShader->PassShaderVariables(actor, renderer);
      this->FragmentShader->Bind();
      }
    }
}

//----------------------------------------------------------------------------
void vtkCgShaderProgram::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
