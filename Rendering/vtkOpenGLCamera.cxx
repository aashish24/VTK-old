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
#include <math.h>

#include "vtkRenderWindow.h"
#include "vtkOpenGLRenderer.h"
#include "vtkOpenGLCamera.h"
#include <GL/gl.h>


// Implement base class method.
void vtkOpenGLCamera::Render(vtkRenderer *ren)
{
  float aspect[2];
  float *vport;
  int  *size, lowerLeft[2], upperRight[2];
  vtkMatrix4x4 *matrix = vtkMatrix4x4::New();

  // get the bounds of the window 
  size = (ren->GetRenderWindow())->GetSize();
  
  // find out if we should stereo render
  this->Stereo = (ren->GetRenderWindow())->GetStereoRender();
  vport = ren->GetViewport();

  lowerLeft[0] = (int)(vport[0]*size[0] + 0.5);
  lowerLeft[1] = (int)(vport[1]*size[1] + 0.5);
  upperRight[0] = (int)(vport[2]*size[0] + 0.5);
  upperRight[1] = (int)(vport[3]*size[1] + 0.5);
  upperRight[0]--;
  upperRight[1]--;

  // if were on a stereo renderer draw to special parts of screen
  if (this->Stereo)
    {
    switch ((ren->GetRenderWindow())->GetStereoType())
      {
      case VTK_STEREO_CRYSTAL_EYES:
        if (this->LeftEye)
          {
          glDrawBuffer(GL_BACK_LEFT);
          }
        else
          {
          glDrawBuffer(GL_BACK_RIGHT);
          }
        break;
      default:
        break;
      }
    }
  else
    {
    if (ren->GetRenderWindow()->GetDoubleBuffer())
      {
      glDrawBuffer(GL_BACK);
      }
    else
      {
      glDrawBuffer(GL_FRONT);
      }
    }
  
  glViewport(lowerLeft[0],lowerLeft[1],
	     (upperRight[0]-lowerLeft[0]+1),
	     (upperRight[1]-lowerLeft[1]+1));
  glEnable( GL_SCISSOR_TEST );
  glScissor(lowerLeft[0],lowerLeft[1],
	    (upperRight[0]-lowerLeft[0]+1),
	    (upperRight[1]-lowerLeft[1]+1));
    
  /* for stereo we have to fiddle with aspect */
  if (this->Stereo)
    {
    switch ((ren->GetRenderWindow())->GetStereoType())
      {
      case VTK_STEREO_CRYSTAL_EYES:
	aspect[0] = (float)(upperRight[0]-lowerLeft[0]+1)/
	  (float)(upperRight[1]-lowerLeft[1]+1);
	aspect[1] = 1.0;
	break;
      default:
	aspect[0] = (float)(upperRight[0]-lowerLeft[0]+1)/
	  (float)(upperRight[1]-lowerLeft[1]+1);
	aspect[1] = 1.0;
      }
    }
  else
    {
    aspect[0] = (float)(upperRight[0]-lowerLeft[0]+1)/
      (float)(upperRight[1]-lowerLeft[1]+1);
    aspect[1] = 1.0;
    }
  
  ren->SetAspect(aspect);

  glMatrixMode( GL_PROJECTION);
  matrix->DeepCopy(this->GetPerspectiveTransformMatrix(aspect[0]/aspect[1],
						       -1,1));
  matrix->Transpose();
  // insert camera view transformation 
  glLoadMatrixd(matrix->Element[0]);

  // since lookat modifies the model view matrix do a push 
  // first and set the mmode.  This will be undone in the  
  // render action after the actors! message sis sent      
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  matrix->DeepCopy(this->GetViewTransformMatrix());
  matrix->Transpose();
  
  // insert camera view transformation 
  glMultMatrixd(matrix->Element[0]);

  if ((ren->GetRenderWindow())->GetErase()) 
    {
    ren->Clear();
    }

  // if we have a stereo renderer, draw other eye next time 
  if (this->Stereo)
    {
    if (this->LeftEye)
      {
      this->LeftEye = 0;
      }
    else
      {
      this->LeftEye = 1;
      }
    }

  matrix->Delete();
}
