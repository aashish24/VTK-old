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

#include "vtkOpenGLContextDevice2D.h"

#include "vtkPoints2D.h"

#include "vtkFloatArray.h"
#include "vtkSmartPointer.h"

#include "vtkActor2D.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPlane.h"
#include "vtkPlaneCollection.h"
#include "vtkPointData.h"
#include "vtkPolyData.h"
#include "vtkProperty2D.h"
#include "vtkScalarsToColors.h"
#include "vtkUnsignedCharArray.h"
#include "vtkViewport.h"
#include "vtkWindow.h"
#include "vtkgluPickMatrix.h"

#include "vtkTexture.h"
#include "vtkImageData.h"

#include "vtkRenderer.h"
#include "vtkOpenGLRenderer.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkOpenGLExtensionManager.h"
#include "vtkgl.h"

#ifdef VTK_USE_QT
  #include "vtkQtLabelRenderStrategy.h"
#else
  #include "vtkFreeTypeLabelRenderStrategy.h"
#endif

#include "vtkObjectFactory.h"

//-----------------------------------------------------------------------------
class vtkOpenGLContextDevice2D::Private
{
public:
  Private()
  {
    this->texture = 0;
  }

  vtkTexture *texture;
};

//-----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkOpenGLContextDevice2D, "$Revision$");
vtkStandardNewMacro(vtkOpenGLContextDevice2D);

//-----------------------------------------------------------------------------
vtkOpenGLContextDevice2D::vtkOpenGLContextDevice2D()
{
  this->Renderer = 0;
  this->IsTextDrawn = false;
#ifdef VTK_USE_QT
  this->TextRenderer = vtkQtLabelRenderStrategy::New();
#else
  this->TextRenderer = vtkFreeTypeLabelRenderStrategy::New();
#endif
  this->Storage = new vtkOpenGLContextDevice2D::Private;
}

//-----------------------------------------------------------------------------
vtkOpenGLContextDevice2D::~vtkOpenGLContextDevice2D()
{
  this->TextRenderer->Delete();
  this->TextRenderer = 0;
  delete this->Storage;
  this->Storage = 0;
}

//-----------------------------------------------------------------------------
void vtkOpenGLContextDevice2D::Begin(vtkViewport* viewport)
{
  int size[2];
  size[0] = viewport->GetSize()[0];
  size[1] = viewport->GetSize()[1];
  double *vport = viewport->GetViewport();

  // push a 2D matrix on the stack
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho( 0.5, size[0]+0.5,
           0.5, size[1]+0.5,
          -1, 0);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glDisable(GL_LIGHTING);

  glDepthMask(GL_FALSE);

  this->Renderer = vtkRenderer::SafeDownCast(viewport);
  this->TextRenderer->SetRenderer(this->Renderer);
  this->IsTextDrawn = false;

  vtkOpenGLRenderer *gl = vtkOpenGLRenderer::SafeDownCast(viewport);
  if (gl)
    {
    vtkOpenGLRenderWindow *glWin = vtkOpenGLRenderWindow::SafeDownCast(gl->GetRenderWindow());
    if (glWin)
      {
      this->LoadExtensions(glWin->GetExtensionManager());
      }
    }

  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkOpenGLContextDevice2D::End()
{
  if (this->IsTextDrawn)
    {
    this->TextRenderer->EndFrame();
    }
// push a 2D matrix on the stack
  glMatrixMode( GL_PROJECTION);
  glPopMatrix();
  glMatrixMode( GL_MODELVIEW);
  glPopMatrix();
  glEnable( GL_LIGHTING);

  // Turn it back on in case we've turned it off
  glDepthMask( GL_TRUE );

  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkOpenGLContextDevice2D::DrawPoly(float *f, int n)
{
/*  if (f && n == 4)
    {
    float p[] = { f[0], f[1], 0.0,
                  f[2], f[3], 0.0,
                  f[4], f[5], 0.0,
                  f[6], f[7], 0.0 };
    glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 4, &p[0]);
    glEnable(GL_MAP1_VERTEX_3);
    glBegin(GL_LINE_STRIP);
      for (int i = 0; i <= 30; ++i)
        {
        glEvalCoord1f(float(i / 30.0));
        }
    glEnd();
    glDisable(GL_MAP1_VERTEX_3);
    this->DrawPoints(f, 4);
    }
*/
  if(f && n > 0)
    {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, &f[0]);
    glDrawArrays(GL_LINE_STRIP, 0, n);
    glDisableClientState(GL_VERTEX_ARRAY);
    }
  else
    {
    vtkWarningMacro(<< "Points supplied that were not of type float.");
    }
}

//-----------------------------------------------------------------------------
void vtkOpenGLContextDevice2D::DrawPoints(float *f, int n)
{
  if (f && n > 0)
    {
    if (this->Storage->texture)
      {
      this->Storage->texture->Render(this->Renderer);
      glEnable(vtkgl::POINT_SPRITE);
      glTexEnvi(vtkgl::POINT_SPRITE, vtkgl::COORD_REPLACE, GL_TRUE);
      vtkgl::PointParameteri(vtkgl::POINT_SPRITE_COORD_ORIGIN, vtkgl::LOWER_LEFT);
      }

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, &f[0]);
    glDrawArrays(GL_POINTS, 0, n);
    glDisableClientState(GL_VERTEX_ARRAY);

    if (this->Storage->texture)
      {
      glTexEnvi(vtkgl::POINT_SPRITE, vtkgl::COORD_REPLACE, GL_FALSE);
      glDisable(vtkgl::POINT_SPRITE);
      this->Storage->texture->PostRender(this->Renderer);
      glDisable(GL_TEXTURE_2D);
      }
    }
  else
    {
    vtkWarningMacro(<< "Points supplied that were not of type float.");
    }
}

//-----------------------------------------------------------------------------
void vtkOpenGLContextDevice2D::DrawQuad(float *f, int n)
{
  if (f && n > 0)
    {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, f);
    glDrawArrays(GL_QUADS, 0, n);
    glDisableClientState(GL_VERTEX_ARRAY);
    }
  else
    {
    vtkWarningMacro(<< "Points supplied that were not of type float.");
    }
}

//-----------------------------------------------------------------------------
void vtkOpenGLContextDevice2D::DrawText(float *point, vtkTextProperty *prop,
                                  const vtkStdString &string)
{
  if (!this->IsTextDrawn)
    {
    this->IsTextDrawn = true;
    this->TextRenderer->StartFrame();
    }

  int p[] = { point[0], point[1] };
  this->TextRenderer->RenderLabel(&p[0], prop, string);
}

//-----------------------------------------------------------------------------
void vtkOpenGLContextDevice2D::DrawImage(float *p, int n, vtkImageData *image)
{
  vtkTexture *tex =vtkTexture::New();
  tex->SetInput(image);
  tex->Render(this->Renderer);
  int *extent = image->GetExtent();
  float points[] = { p[0]          , p[1],
                     p[0]+extent[1], p[1],
                     p[0]+extent[1], p[1]+extent[3],
                     p[0]          , p[1]+extent[3] };

  float texCoord[] = { 0.0, 0.0,
                       1.0, 0.0,
                       1.0, 1.0,
                       0.0, 1.0 };

  glColor4ub(255, 255, 255, 255);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glVertexPointer(2, GL_FLOAT, 0, &points[0]);
  glTexCoordPointer(2, GL_FLOAT, 0, &texCoord[0]);
  glDrawArrays(GL_QUADS, 0, 4);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  tex->PostRender(this->Renderer);
  glDisable(GL_TEXTURE_2D);
  tex->Delete();
}

//-----------------------------------------------------------------------------
unsigned int vtkOpenGLContextDevice2D::AddPointSprite(vtkImageData *image)
{
  this->Storage->texture = vtkTexture::New();
  this->Storage->texture->SetInput(image);
}

//-----------------------------------------------------------------------------
void vtkOpenGLContextDevice2D::SetColor4(unsigned char *color)
{
  glColor4ubv(color);
}

//-----------------------------------------------------------------------------
void vtkOpenGLContextDevice2D::SetColor(unsigned char *color)
{
  glColor3ubv(color);
}

//-----------------------------------------------------------------------------
void vtkOpenGLContextDevice2D::SetPointSize(float size)
{
  glPointSize(size);
}

//-----------------------------------------------------------------------------
void vtkOpenGLContextDevice2D::SetLineWidth(float width)
{
  glLineWidth(width);
}

//-----------------------------------------------------------------------------
void vtkOpenGLContextDevice2D::SetViewExtents(float *x)
{
  glLoadIdentity();
  glOrtho( x[0], x[2],
           x[1], x[3],
          -1, 0);
}

//-----------------------------------------------------------------------------
void vtkOpenGLContextDevice2D::PushMatrix()
{
  glMatrixMode( GL_MODELVIEW );
  glPushMatrix();
}

//-----------------------------------------------------------------------------
void vtkOpenGLContextDevice2D::PopMatrix()
{
  glMatrixMode( GL_MODELVIEW );
  glPopMatrix();
}

//-----------------------------------------------------------------------------
void vtkOpenGLContextDevice2D::SetClipping(int *x)
{
  // Test the glScissor function
  vtkDebugMacro(<< "Clipping area: " << x[0] << "\t" << x[1]
                << "\t" << x[2] << "\t" << x[3]);
  glScissor(x[0], x[1], x[2], x[3]);
  glEnable(GL_SCISSOR_TEST);
}

//-----------------------------------------------------------------------------
void vtkOpenGLContextDevice2D::DisableClipping()
{
  glDisable(GL_SCISSOR_TEST);
}

//-----------------------------------------------------------------------------
bool vtkOpenGLContextDevice2D::LoadExtensions(vtkOpenGLExtensionManager *m)
{
  bool supportsGL15=m->ExtensionSupported("GL_VERSION_1_5");
  if(supportsGL15)
    {
    m->LoadExtension("GL_VERSION_1_5");
    }
}

//-----------------------------------------------------------------------------
void vtkOpenGLContextDevice2D::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Renderer: ";
  if (this->Renderer)
  {
    os << endl;
    this->Renderer->PrintSelf(os, indent.GetNextIndent());
  }
  else
    {
    os << "(none)" << endl;
    }
  os << indent << "Text Renderer: ";
  if (this->Renderer)
  {
    os << endl;
    this->TextRenderer->PrintSelf(os, indent.GetNextIndent());
  }
  else
    {
    os << "(none)" << endl;
    }
}
