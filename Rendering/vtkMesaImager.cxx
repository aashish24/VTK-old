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
// Make sure this is first, so any includes of gl.h can be stoped if needed
#define VTK_IMPLEMENT_MESA_CXX

#include "MangleMesaInclude/gl_mangle.h"
#include "MangleMesaInclude/gl.h"

#include <math.h>
#include "vtkToolkits.h"
#include "vtkMesaImager.h"
#include "vtkRenderWindow.h"
#include "vtkMesaProperty.h"
#include "vtkMesaCamera.h"
#include "vtkMesaLight.h"
#include "vtkCuller.h"

// make sure this file is included before the #define takes place
// so we don't get two vtkMesaImager classes defined.
#include "vtkOpenGLImager.h"
#include "vtkMesaImager.h"

// Make sure vtkMesaImager is a copy of vtkOpenGLImager
// with vtkOpenGLImager replaced with vtkMesaImager
#define vtkOpenGLImager vtkMesaImager
#include "vtkOpenGLImager.cxx"
#undef vtkOpenGLImager

vtkCxxRevisionMacro(vtkMesaImager, "$Revision$");
vtkStandardNewMacro(vtkMesaImager);
