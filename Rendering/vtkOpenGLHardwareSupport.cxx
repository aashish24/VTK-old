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

#include "vtkOpenGLHardwareSupport.h"

#include "vtkObjectFactory.h"
#include "vtkOpenGL.h"
#include "vtkOpenGLExtensionManager.h"
#include "vtkRenderWindow.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkgl.h"

vtkCxxRevisionMacro(vtkOpenGLHardwareSupport, "$Revision$");
vtkStandardNewMacro(vtkOpenGLHardwareSupport);

vtkCxxSetObjectMacro(vtkOpenGLHardwareSupport, ExtensionManager, vtkOpenGLExtensionManager);


// ----------------------------------------------------------------------------
vtkOpenGLHardwareSupport::vtkOpenGLHardwareSupport()
{
  this->ExtensionManager = NULL;
}

// ----------------------------------------------------------------------------
vtkOpenGLHardwareSupport::~vtkOpenGLHardwareSupport()
{
  this->SetExtensionManager(NULL);
}

// ----------------------------------------------------------------------------
int vtkOpenGLHardwareSupport::GetNumberOfTextureUnits()
{
  if ( ! vtkgl::MultiTexCoord2d || ! vtkgl::ActiveTexture )
    {
    if(!ExtensionManagerSet())
      {
      return 1;
      }

    // multitexture is a core feature of OpenGL 1.3.
    // multitexture is an ARB extension of OpenGL 1.2.1
    int supports_GL_1_3 = this->ExtensionManager->ExtensionSupported( "GL_VERSION_1_3" );
    int supports_GL_1_2_1 = this->ExtensionManager->ExtensionSupported("GL_VERSION_1_2");
    int supports_ARB_mutlitexture = 
      this->ExtensionManager->ExtensionSupported("GL_ARB_multitexture");
    
    if(supports_GL_1_3)
      {
      this->ExtensionManager->LoadExtension("GL_VERSION_1_3");
      }
    else if(supports_GL_1_2_1 && supports_ARB_mutlitexture)
      {
      this->ExtensionManager->LoadExtension("GL_VERSION_1_2");
      this->ExtensionManager->LoadCorePromotedExtension("GL_ARB_multitexture");
      }
    else
      {
      return 1;
      }
    }

  GLint numSupportedTextures = 1;
  glGetIntegerv(vtkgl::MAX_TEXTURE_UNITS, &numSupportedTextures);

  return numSupportedTextures;
}

// ----------------------------------------------------------------------------
bool vtkOpenGLHardwareSupport::GetSupportsMultiTexturing()
{ 
  if ( ! vtkgl::MultiTexCoord2d || ! vtkgl::ActiveTexture )
    {
    if(!ExtensionManagerSet())
      {
      return false;
      }

    // multitexture is a core feature of OpenGL 1.3.
    // multitexture is an ARB extension of OpenGL 1.2.1
    int supports_GL_1_3 = this->ExtensionManager->ExtensionSupported( "GL_VERSION_1_3" );
    int supports_GL_1_2_1 = this->ExtensionManager->ExtensionSupported("GL_VERSION_1_2");
    int supports_ARB_mutlitexture = 
      this->ExtensionManager->ExtensionSupported("GL_ARB_multitexture");

    if(supports_GL_1_3 || supports_GL_1_2_1 || supports_ARB_mutlitexture)
      {
      return true;
      }

    return false;
    }
  else
    {
    return true;
    }
}

//-----------------------------------------------------------------------------
void vtkOpenGLHardwareSupport::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << this->ExtensionManager << endl;
  this->Superclass::PrintSelf(os,indent);
}

bool vtkOpenGLHardwareSupport::ExtensionManagerSet()
{
  if(!this->ExtensionManager)
    {
    vtkErrorMacro("" << this->GetClassName() << ": requires an ExtensionManager set.");
    return false;
    }
  if(!this->ExtensionManager->GetRenderWindow())
    {
    vtkErrorMacro("" << this->GetClassName() << ": requires an ExtensionManager with Render Window set.");
    return false;
    }
  return true;
}
