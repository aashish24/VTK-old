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

#include "vtkObjectFactory.h"
#include "vtkImagingFactory.h"
#include "vtkToolkits.h"

#ifdef VTK_USE_OGLR
#include "vtkOpenGLImageMapper.h"
#include "vtkOpenGLImager.h"
#include "vtkOpenGLImageWindow.h"
#include "vtkXOpenGLTextMapper.h"
#include "vtkOpenGLPolyDataMapper2D.h"
#endif

#ifdef VTK_USE_MESA
#include "vtkMesaImageMapper.h"
#include "vtkMesaImager.h"
#include "vtkMesaImageWindow.h"
#include "vtkXMesaTextMapper.h"
#include "vtkMesaPolyDataMapper2D.h"
#endif

#ifdef _WIN32
#include "vtkOpenGLImageMapper.h"
#include "vtkOpenGLImager.h"
#include "vtkWin32OpenGLImageWindow.h"
#include "vtkWin32OpenGLTextMapper.h"
#include "vtkOpenGLPolyDataMapper2D.h"
#include "vtkWin32TextMapper.h"
#include "vtkWin32ImageWindow.h"
#include "vtkWin32ImageMapper.h"
#include "vtkWin32PolyDataMapper2D.h"
#else
#include "vtkXTextMapper.h"
#include "vtkXImageWindow.h"
#include "vtkXImageMapper.h"
#include "vtkXPolyDataMapper2D.h"
#endif

const char *vtkImagingFactoryGetRenderLibrary()
{
  const char *temp;
  
  // first check the environment variable
  temp = getenv("VTK_RENDERER");
  
  // Backward compatibility
  if ( temp )
    {
    if (!strcmp("oglr",temp))
      {
      temp = "OpenGL";
      }
    else if (!strcmp("woglr",temp))
      {
      temp = "Win32OpenGL";
      }
    else if (strcmp("Mesa",temp) && strcmp("OpenGL",temp) && 
	     strcmp("Win32OpenGL",temp))
      {
      vtkGenericWarningMacro(<<"VTK_RENDERER set to unsupported type:" << temp);
      temp = NULL;
      }
    }
  
  // if the environment variable is set to openGL and the user
  //  does not have opengl but they do have mesa, then use it
#ifndef VTK_USE_OGLR
#ifdef VTK_USE_MESA
	if ( temp != NULL )
	{
		if (!strcmp("OpenGL",temp))
		{
			temp = "Mesa";
		}
	}
#endif
#endif
  
  // if nothing is set then work down the list of possible renderers
  if ( !temp )
    {
#ifdef VTK_USE_MESA
    temp = "Mesa";
#endif
#ifdef VTK_USE_OGLR
    temp = "OpenGL";
#endif
#ifdef _WIN32
    temp = "Win32OpenGL";
#endif
    }
  
  return temp;
}

vtkObject* vtkImagingFactory::CreateInstance(const char* vtkclassname )
{
  // first check the object factory
  vtkObject *ret = vtkObjectFactory::CreateInstance(vtkclassname);
  if (ret)
    {
    return ret;
    }

  const char *rl = vtkImagingFactoryGetRenderLibrary();

#ifdef VTK_USE_OGLR
#ifdef VTK_USE_NATIVE_IMAGING
    if(strcmp(vtkclassname, "vtkTextMapper") == 0)
      {
      return vtkXTextMapper::New();
      }
    if(strcmp(vtkclassname, "vtkImageWindow") == 0)
      {
      return vtkXImageWindow::New();
      }
    if(strcmp(vtkclassname, "vtkImageMapper") == 0)
      {
      return vtkXImageMapper::New();
      }
    if(strcmp(vtkclassname, "vtkPolyDataMapper2D") == 0)
      {
      return vtkXPolyDataMapper2D::New();
      }
#else
  if (!strcmp("OpenGL",rl))
    {
    if(strcmp(vtkclassname, "vtkTextMapper") == 0)
      {
      return vtkXOpenGLTextMapper::New();
      }
    if(strcmp(vtkclassname, "vtkImageWindow") == 0)
      {
      return vtkOpenGLImageWindow::New();
      }
    if(strcmp(vtkclassname, "vtkImager") == 0)
      {
      return vtkOpenGLImager::New();
      }
    if(strcmp(vtkclassname, "vtkImageMapper") == 0)
      {
      return vtkOpenGLImageMapper::New();
      }
    if(strcmp(vtkclassname, "vtkPolyDataMapper2D") == 0)
      {
      return vtkOpenGLPolyDataMapper2D::New();
      }
    }
#endif
#endif

#ifdef _WIN32
#ifdef VTK_USE_NATIVE_IMAGING
  if(strcmp(vtkclassname, "vtkTextMapper") == 0)
    {
    return vtkWin32TextMapper::New();
    }
  if(strcmp(vtkclassname, "vtkImageWindow") == 0)
    {
    return vtkWin32ImageWindow::New();
    }
  if(strcmp(vtkclassname, "vtkImageMapper") == 0)
    {
    return vtkWin32ImageMapper::New();
    }
  if(strcmp(vtkclassname, "vtkPolyDataMapper2D") == 0)
    {
    return vtkWin32PolyDataMapper2D::New();
    }
#else
  if (!strcmp("Win32OpenGL",rl))
    {
    if(strcmp(vtkclassname, "vtkTextMapper") == 0)
      {
      return vtkWin32OpenGLTextMapper::New();
      }
    if(strcmp(vtkclassname, "vtkImageWindow") == 0)
      {
      return vtkWin32OpenGLImageWindow::New();
      }
    if(strcmp(vtkclassname, "vtkImager") == 0)
      {
      return vtkOpenGLImager::New();
      }
    if(strcmp(vtkclassname, "vtkImageMapper") == 0)
      {
      return vtkOpenGLImageMapper::New();
      }
    if(strcmp(vtkclassname, "vtkPolyDataMapper2D") == 0)
      {
      return vtkOpenGLPolyDataMapper2D::New();
      }
    }
#endif
#endif

#ifdef VTK_USE_MESA
#ifdef VTK_USE_NATIVE_IMAGING
    if(strcmp(vtkclassname, "vtkTextMapper") == 0)
      {
      return vtkXTextMapper::New();
      }
    if(strcmp(vtkclassname, "vtkImageWindow") == 0)
      {
      return vtkXImageWindow::New();
      }
    if(strcmp(vtkclassname, "vtkImageMapper") == 0)
      {
      return vtkXImageMapper::New();
      }
    if(strcmp(vtkclassname, "vtkPolyDataMapper2D") == 0)
      {
      return vtkXPolyDataMapper2D::New();
      }
#else
  if (!strcmp("Mesa",rl))
    {
    if(strcmp(vtkclassname, "vtkTextMapper") == 0)
      {
      return vtkXMesaTextMapper::New();
      }
    if(strcmp(vtkclassname, "vtkImageWindow") == 0)
      {
      return vtkMesaImageWindow::New();
      }
    if(strcmp(vtkclassname, "vtkImager") == 0)
      {
      return vtkMesaImager::New();
      }
    if(strcmp(vtkclassname, "vtkImageMapper") == 0)
      {
      return vtkMesaImageMapper::New();
      }
    if(strcmp(vtkclassname, "vtkPolyDataMapper2D") == 0)
      {
      return vtkMesaPolyDataMapper2D::New();
      }
    }
#endif
#endif  
  return 0;
}


