/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1998 Ken Martin, Will Schroeder, Bill Lorensen.

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
#include "vtkPolyDataMapper.h"
#include "vtkRenderWindow.h"
#include "vtkObjectFactory.h"

#ifdef VTK_USE_OGLR
#include "vtkOpenGLPolyDataMapper.h"
#endif
#ifdef _WIN32
#include "vtkOpenGLPolyDataMapper.h"
#endif
// return the correct type of PolyDataMapper 
vtkPolyDataMapper *vtkPolyDataMapper::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkPolyDataMapper");
  if(ret)
    {
    return (vtkPolyDataMapper*)ret;
    }
  // If the factory was unable to create the object, then create it here.

  char *temp = vtkRenderWindow::GetRenderLibrary();
  
#ifdef VTK_USE_OGLR
  if (!strcmp("OpenGL",temp))
    {
    return vtkOpenGLPolyDataMapper::New();
    }
#endif
#ifdef _WIN32
  if (!strcmp("Win32OpenGL",temp))
    {
    return vtkOpenGLPolyDataMapper::New();
    }
#endif
  
  return new vtkPolyDataMapper;
}

//----------------------------------------------------------------------------
// Specify the input data or filter.
void vtkPolyDataMapper::SetInput(vtkPolyData *input)
{
  this->vtkProcessObject::SetInput(0, input);
}

//----------------------------------------------------------------------------
// Specify the input data or filter.
vtkPolyData *vtkPolyDataMapper::GetInput()
{
  if (this->NumberOfInputs < 1)
    {
    return NULL;
    }
  
  return (vtkPolyData *)(this->Inputs[0]);
}
