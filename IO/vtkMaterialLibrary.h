/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkMaterialLibrary.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkMaterialLibrary - Library for Materials
// .SECTION Description
// This class provides the Material XMLs.

#ifndef __vtkShaderCodeLibrary_h
#define __vtkShaderCodeLibrary_h

#include "vtkObject.h"

class VTK_IO_EXPORT vtkMaterialLibrary : public vtkObject
{
public:
  static vtkMaterialLibrary* New();
  vtkTypeRevisionMacro(vtkMaterialLibrary, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Obtain the code for the shader with given name.
  // Note that Cg shader names are prefixed with CG and
  // GLSL shader names are prefixed with GLSL.
  // This method allocates memory. It's the responsibility
  // of the caller to free this memory.
  static char* GetMaterial(const char* name);

protected:
  vtkMaterialLibrary();
  ~vtkMaterialLibrary();

private:
  vtkMaterialLibrary(const vtkMaterialLibrary&); // Not implemented.
  void operator=(const vtkMaterialLibrary&); // Not implemented.
};

#endif

