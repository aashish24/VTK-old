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
#include "vtkInteractorStyleJoystickActor.h"
#include "vtkObjectFactory.h"
#include "vtkMath.h"

//----------------------------------------------------------------------------
vtkInteractorStyleJoystickActor *vtkInteractorStyleJoystickActor::New() 
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkInteractorStyleJoystickActor");
  if(ret)
    {
    return (vtkInteractorStyleJoystickActor*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkInteractorStyleJoystickActor;
}


//----------------------------------------------------------------------------
vtkInteractorStyleJoystickActor::vtkInteractorStyleJoystickActor() 
{
  int i;
  
  this->MotionFactor = 10.0;
  this->State = VTK_INTERACTOR_STYLE_ACTOR_NONE;
  this->RadianToDegree = 180.0 / vtkMath::Pi();
  this->InteractionProp = NULL;

  for (i = 0; i < 3; i++)
    {
    this->ViewUp[i] = 0.0;
    this->ViewLook[i] = 0.0;
    this->ViewRight[i] = 0.0;
    this->ObjCenter[i] = 0.0;
    this->DispObjCenter[i] = 0.0;
    this->NewPickPoint[i] = 0.0;
    this->OldPickPoint[i] = 0.0;
    this->MotionVector[i] = 0.0;
    this->ViewPoint[i] = 0.0;
    this->ViewFocus[i] = 0.0;
    }
  this->NewPickPoint[3] = 1.0;
  this->OldPickPoint[3] = 1.0;
  
  this->Radius = 0.0;

  this->InteractionPicker = vtkCellPicker::New();
}

//----------------------------------------------------------------------------
vtkInteractorStyleJoystickActor::~vtkInteractorStyleJoystickActor() 
{
  this->InteractionPicker->Delete();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleJoystickActor::OnMouseMove(int vtkNotUsed(ctrl), 
						  int vtkNotUsed(shift),
						  int x, int y) 
{
  if (this->State == VTK_INTERACTOR_STYLE_ACTOR_ROTATE)
    {
    this->FindPokedCamera(x, y);
    this->RotateXY(x, y);
    }
  else if (this->State == VTK_INTERACTOR_STYLE_ACTOR_PAN)
    {
    this->FindPokedCamera(x, y);
    this->PanXY(x, y);
    }
  else if (this->State == VTK_INTERACTOR_STYLE_ACTOR_ZOOM)
    {
    this->FindPokedCamera(x, y);
    this->DollyXY(x, y);
    }
  else if (this->State == VTK_INTERACTOR_STYLE_ACTOR_SPIN)
    {
    this->FindPokedCamera(x, y);
    this->SpinXY(x, y);
    }
  else if (this->State == VTK_INTERACTOR_STYLE_ACTOR_SCALE)
    {
    this->FindPokedCamera(x, y);
    this->ScaleXY(x, y);
    }

  this->LastPos[0] = x;
  this->LastPos[1] = y;
}


//----------------------------------------------------------------------------
void vtkInteractorStyleJoystickActor::RotateXY(int x, int y)
{
  vtkRenderWindowInteractor *rwi = this->Interactor;
  vtkCamera *cam;

  if (this->CurrentRenderer == NULL)
    {
    return;
    }
  
  cam = this->CurrentRenderer->GetActiveCamera();
  
  // first get the origin of the assembly
  float *center = this->InteractionProp->GetCenter();
  this->ObjCenter[0] = center[0];
  this->ObjCenter[1] = center[1];
  this->ObjCenter[2] = center[2];
  
  // GetLength gets the length of the diagonal of the bounding box
  double boundRadius = this->InteractionProp->GetLength() * 0.5;
  
  // get the view up and view right vectors
  cam->OrthogonalizeViewUp();
  cam->ComputeViewPlaneNormal();
  cam->GetViewUp(this->ViewUp);
  vtkMath::Normalize(this->ViewUp);
  cam->GetViewPlaneNormal(this->ViewLook);
  vtkMath::Cross(this->ViewUp, this->ViewLook, this->ViewRight);
  vtkMath::Normalize(this->ViewRight);
  
  // get the furtherest point from object bounding box center
  float outsidept[3];
  outsidept[0] = this->ObjCenter[0] + this->ViewRight[0] * boundRadius;
  outsidept[1] = this->ObjCenter[1] + this->ViewRight[1] * boundRadius;
  outsidept[2] = this->ObjCenter[2] + this->ViewRight[2] * boundRadius;
  
  // convert to display coord
  this->ComputeWorldToDisplay(this->ObjCenter[0], this->ObjCenter[1],
			      this->ObjCenter[2], this->DispObjCenter);
  this->ComputeWorldToDisplay(outsidept[0], outsidept[1],
			      outsidept[2], outsidept);
  
  this->Radius = sqrt(vtkMath::Distance2BetweenPoints(this->DispObjCenter,
						      outsidept));
  
  this->HighlightProp3D(NULL);
  
  double nxf = (double)(x - this->DispObjCenter[0]) / this->Radius;
  double nyf = (double)(y - this->DispObjCenter[1]) / this->Radius;
  
  if (nxf > 1.0)
    {
    nxf = 1.0;
    }
  else if (nxf < -1.0)
    {
    nxf = -1.0;
    }
  
  if (nyf > 1.0)
    {
    nyf = 1.0;
    }
  else if (nyf < -1.0)
    {
    nyf = -1.0;
    }
  
  double newXAngle = asin(nxf) * this->RadianToDegree / this->MotionFactor;
  double newYAngle = asin(nyf) * this->RadianToDegree / this->MotionFactor;
  
  double scale[3];
  scale[0] = scale[1] = scale[2] = 1.0;
  double **rotate = new double*[2];
  rotate[0] = new double[4];
  rotate[1] = new double[4];
  
  rotate[0][0] = newXAngle;
  rotate[0][1] = this->ViewUp[0];
  rotate[0][2] = this->ViewUp[1];
  rotate[0][3] = this->ViewUp[2];
  
  rotate[1][0] = -newYAngle;
  rotate[1][1] = this->ViewRight[0];
  rotate[1][2] = this->ViewRight[1];
  rotate[1][3] = this->ViewRight[2];
  
  
  this->Prop3DTransform(this->InteractionProp,
			this->ObjCenter,
			2, rotate, scale);
  
  delete [] rotate[0];
  delete [] rotate[1];
  delete [] rotate;
  
  rwi->Render();
}
  
//----------------------------------------------------------------------------
void vtkInteractorStyleJoystickActor::PanXY(int x, int y)
{
  if (this->CurrentRenderer == NULL)
    {
    return;
    }

  vtkRenderWindowInteractor *rwi = this->Interactor;
  
  // use initial center as the origin from which to pan
  float *center = this->InteractionProp->GetCenter();
  this->ObjCenter[0] = center[0];
  this->ObjCenter[1] = center[1];
  this->ObjCenter[2] = center[2];
  
  this->ComputeWorldToDisplay(this->ObjCenter[0], this->ObjCenter[1],
			      this->ObjCenter[2], this->DispObjCenter);
  this->FocalDepth = this->DispObjCenter[2];
  
  this->HighlightProp3D(NULL);
  
  this->ComputeDisplayToWorld(double(x), double(y),
                              this->FocalDepth,
                              this->NewPickPoint);
  
  /*
   * Compute a translation vector, moving everything 1/10
   * the distance to the cursor. (Arbitrary scale factor)
   */
  this->MotionVector[0] = (this->NewPickPoint[0] -
                           this->ObjCenter[0]) / this->MotionFactor;
  this->MotionVector[1] = (this->NewPickPoint[1] -
                           this->ObjCenter[1]) / this->MotionFactor;
  this->MotionVector[2] = (this->NewPickPoint[2] -
                           this->ObjCenter[2]) / this->MotionFactor;
  
  if (this->InteractionProp->GetUserMatrix() != NULL)
    {
    vtkTransform *t = vtkTransform::New();
    t->PostMultiply();
    t->SetMatrix(*(this->InteractionProp->GetUserMatrix()));
    t->Translate(this->MotionVector[0], this->MotionVector[1],
                 this->MotionVector[2]);
    this->InteractionProp->GetUserMatrix()->DeepCopy(t->GetMatrix());
    t->Delete();
    }
  else
    {
    this->InteractionProp->AddPosition(this->MotionVector);
    }
  
  rwi->Render();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleJoystickActor::DollyXY(int vtkNotUsed(x), int y)
{
  vtkCamera *cam;
  
  if (this->CurrentRenderer == NULL)
    {
    return;
    }
  
  cam = this->CurrentRenderer->GetActiveCamera();

  vtkRenderWindowInteractor *rwi = this->Interactor;
  // dolly is based on distance from center of screen,
  // and the upper half is positive, lower half is negative
  
  cam->GetPosition(this->ViewPoint);
  cam->GetFocalPoint(this->ViewFocus);
  
  // use initial center as the origin from which to pan
  float *center = this->InteractionProp->GetCenter();
  this->ObjCenter[0] = center[0];
  this->ObjCenter[1] = center[1];
  this->ObjCenter[2] = center[2];
  this->ComputeWorldToDisplay(this->ObjCenter[0], this->ObjCenter[1],
			      this->ObjCenter[2], this->DispObjCenter);
  
  this->HighlightProp3D(NULL);
  
  double yf = (double)(y - this->DispObjCenter[1]) /
    (double)(this->Center[1]);
  double dollyFactor = pow((double)1.1, yf);

  dollyFactor -= 1.0;
  this->MotionVector[0] = (this->ViewPoint[0] -
                           this->ViewFocus[0]) * dollyFactor;
  this->MotionVector[1] = (this->ViewPoint[1] -
                           this->ViewFocus[1]) * dollyFactor;
  this->MotionVector[2] = (this->ViewPoint[2] -
                           this->ViewFocus[2]) * dollyFactor;

  if (this->InteractionProp->GetUserMatrix() != NULL)
    {
    vtkTransform *t = vtkTransform::New();
    t->PostMultiply();
    t->SetMatrix(*(this->InteractionProp->GetUserMatrix()));
    t->Translate(this->MotionVector[0], this->MotionVector[1],
                 this->MotionVector[2]);
    this->InteractionProp->GetUserMatrix()->DeepCopy(t->GetMatrix());
    t->Delete();
    }
  else
    {
    this->InteractionProp->AddPosition(this->MotionVector);
    }

  rwi->Render();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleJoystickActor::SpinXY(int vtkNotUsed(x), int y)
{
  vtkRenderWindowInteractor *rwi = this->Interactor;
  vtkCamera *cam;

  if (this->CurrentRenderer == NULL)
    {
    return;
    }
  
  cam = this->CurrentRenderer->GetActiveCamera();
  
  // get the axis to rotate around = vector from eye to origin
  float *center = this->InteractionProp->GetCenter();
  this->ObjCenter[0] = center[0];
  this->ObjCenter[1] = center[1];
  this->ObjCenter[2] = center[2];
  
  if (cam->GetParallelProjection())
    {
    // if parallel projection, want to get the view plane normal...
    cam->ComputeViewPlaneNormal();
    cam->GetViewPlaneNormal(this->MotionVector);
    }
  else
    {
    // perspective projection, get vector from eye to center of actor
    cam->GetPosition(this->ViewPoint);
    this->MotionVector[0] = this->ViewPoint[0] - this->ObjCenter[0];
    this->MotionVector[1] = this->ViewPoint[1] - this->ObjCenter[1];
    this->MotionVector[2] = this->ViewPoint[2] - this->ObjCenter[2];
    vtkMath::Normalize(this->MotionVector);
    }
  
  this->ComputeWorldToDisplay(this->ObjCenter[0], this->ObjCenter[1],
			      this->ObjCenter[2], this->DispObjCenter);
  
  this->HighlightProp3D(NULL);
  
  double yf = (double)(y - this->DispObjCenter[1]) / (double)(this->Center[1]);
  if (yf > 1.0)
    {
    yf = 1.0;
    }
  else if (yf < -1.0)
    {
    yf = -1.0;
    }

  double newAngle = asin(yf) * this->RadianToDegree / this->MotionFactor;

  double scale[3];
  scale[0] = scale[1] = scale[2] = 1.0;
  double **rotate = new double*[1];
  rotate[0] = new double[4];

  rotate[0][0] = newAngle;
  rotate[0][1] = this->MotionVector[0];
  rotate[0][2] = this->MotionVector[1];
  rotate[0][3] = this->MotionVector[2];

  this->Prop3DTransform(this->InteractionProp,
			this->ObjCenter,
			1, rotate, scale);

  delete [] rotate[0];
  delete [] rotate;

  rwi->Render();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleJoystickActor::ScaleXY(int vtkNotUsed(x), int y)
{
  vtkRenderWindowInteractor *rwi = this->Interactor;

  if (this->CurrentRenderer == NULL)
    {
    return;
    }
  
  // Uniform scale is based on distance from center of screen,
  // and the upper half is positive, lower half is negative

  // use bounding box center as the origin from which to pan
  float *center = this->InteractionProp->GetCenter();
  this->ObjCenter[0] = center[0];
  this->ObjCenter[1] = center[1];
  this->ObjCenter[2] = center[2];
  
  this->ComputeWorldToDisplay(this->ObjCenter[0], this->ObjCenter[1],
			      this->ObjCenter[2], this->DispObjCenter);
  
  this->HighlightProp3D(NULL);
  
  double yf = (double)(y - this->DispObjCenter[1]) /
    (double)(this->Center[1]);
  double scaleFactor = pow((double)1.1, yf);
  
  double **rotate = NULL;
  
  double scale[3];
  scale[0] = scale[1] = scale[2] = scaleFactor;
  
  this->Prop3DTransform(this->InteractionProp,
			this->ObjCenter,
			0, rotate, scale);
  
  rwi->Render();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleJoystickActor::OnLeftButtonDown(int ctrl, int shift, 
						       int x, int y) 
{
  this->FindPokedRenderer(x, y);
  this->FindPickedActor(x, y);
  
  if (this->CurrentRenderer == NULL)
    {
    return;
    }

  this->UpdateInternalState(ctrl, shift, x, y);
  if (shift)
    {
    this->StartPan();
    this->State = VTK_INTERACTOR_STYLE_ACTOR_PAN;
    }
  else if (this->CtrlKey)
    {
    this->StartSpin();
    this->State = VTK_INTERACTOR_STYLE_ACTOR_SPIN;
    }
  else
    {
    this->StartRotate();
    this->State = VTK_INTERACTOR_STYLE_ACTOR_ROTATE;
    }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleJoystickActor::OnLeftButtonUp(int vtkNotUsed(ctrl),
						     int vtkNotUsed(shift), 
						     int vtkNotUsed(x), 
						     int vtkNotUsed(y)) 
{
  if (this->State == VTK_INTERACTOR_STYLE_ACTOR_SPIN)
    {
    this->EndSpin();
    }
  else
    {
    this->EndRotate();
    }
  
  this->State = VTK_INTERACTOR_STYLE_ACTOR_NONE;
}

//----------------------------------------------------------------------------
void vtkInteractorStyleJoystickActor::OnMiddleButtonDown(int ctrl, int shift, 
							 int x, int y) 
{
  this->FindPokedRenderer(x, y);
  this->FindPickedActor(x, y);
  
  if (this->CurrentRenderer == NULL)
    {
    return;
    }

  this->UpdateInternalState(ctrl, shift, x, y);
  if (this->CtrlKey)
    {
    this->StartDolly();
    this->State = VTK_INTERACTOR_STYLE_ACTOR_ZOOM;
    }
  else
    {
    this->StartPan();
    this->State = VTK_INTERACTOR_STYLE_ACTOR_PAN;
    }
}
//----------------------------------------------------------------------------
void vtkInteractorStyleJoystickActor::OnMiddleButtonUp(int vtkNotUsed(ctrl),
						       int vtkNotUsed(shift), 
						       int vtkNotUsed(x),
						       int vtkNotUsed(y)) 
{
  if (this->State == VTK_INTERACTOR_STYLE_ACTOR_ZOOM)
    {
    this->EndDolly();
    }
  else
    {
    this->EndPan();
    }
  this->State = VTK_INTERACTOR_STYLE_ACTOR_NONE;
}

//----------------------------------------------------------------------------
void vtkInteractorStyleJoystickActor::OnRightButtonDown(int vtkNotUsed(ctrl),
							int vtkNotUsed(shift), 
							int x, int y) 
{
  this->FindPokedRenderer(x, y);
  this->FindPickedActor(x, y);
  
  if (this->CurrentRenderer == NULL)
    {
    return;
    }
  
  this->StartUniformScale();
  this->State = VTK_INTERACTOR_STYLE_ACTOR_SCALE;
}

//----------------------------------------------------------------------------
void vtkInteractorStyleJoystickActor::OnRightButtonUp(int vtkNotUsed(ctrl),
						      int vtkNotUsed(shift), 
						      int vtkNotUsed(x),
						      int vtkNotUsed(y)) 
{
  this->EndZoom();
  this->State = VTK_INTERACTOR_STYLE_ACTOR_NONE;
}

//----------------------------------------------------------------------------
void vtkInteractorStyleJoystickActor::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkInteractorStyle::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkInteractorStyleJoystickActor::FindPickedActor(int x, int y)
{
  this->InteractionPicker->Pick(x, y, 0.0, this->CurrentRenderer);
  vtkProp *prop = this->InteractionPicker->GetProp();
  if ( prop != NULL )
    {
    vtkProp3D *prop3D = vtkProp3D::SafeDownCast(prop);
    if ( prop3D != NULL )
      {
      this->InteractionProp = prop3D;
      }
    }

  // refine the answer to whether an actor was picked.  CellPicker()
  // returns true from Pick() if the bounding box was picked,
  // but we only want something to be picked if a cell was actually
  // selected
  this->PropPicked = (this->InteractionProp != NULL);
}

//----------------------------------------------------------------------------
void vtkInteractorStyleJoystickActor::Prop3DTransform(vtkProp3D *prop3D,
						      double *boxCenter,
						      int numRotation,
						      double **rotate,
						      double *scale)
{
  vtkMatrix4x4 *oldMatrix = vtkMatrix4x4::New();
  prop3D->GetMatrix(oldMatrix);
  
  float orig[3];
  prop3D->GetOrigin(orig);
  
  vtkTransform *newTransform = vtkTransform::New();
  newTransform->PostMultiply();
  if (prop3D->GetUserMatrix() != NULL) 
    {
    newTransform->SetMatrix(*(prop3D->GetUserMatrix()));
    }
  else 
    {
    newTransform->SetMatrix(*oldMatrix);
    }
  
  newTransform->Translate(-(boxCenter[0]), -(boxCenter[1]), -(boxCenter[2]));
  
  for (int i = 0; i < numRotation; i++) 
    {
    newTransform->RotateWXYZ(rotate[i][0], rotate[i][1],
                             rotate[i][2], rotate[i][3]);
    }
  
  if ((scale[0] * scale[1] * scale[2]) != 0.0) 
    {
    newTransform->Scale(scale[0], scale[1], scale[2]);
    }
  
  newTransform->Translate(boxCenter[0], boxCenter[1], boxCenter[2]);
  
  // now try to get the composit of translate, rotate, and scale
  newTransform->Translate(-(orig[0]), -(orig[1]), -(orig[2]));
  newTransform->PreMultiply();
  newTransform->Translate(orig[0], orig[1], orig[2]);
  
  if (prop3D->GetUserMatrix() != NULL) 
    {
    newTransform->GetMatrix(prop3D->GetUserMatrix());
    }
  else 
    {
    prop3D->SetPosition(newTransform->GetPosition());
    prop3D->SetScale(newTransform->GetScale());
    prop3D->SetOrientation(newTransform->GetOrientation());
    }
  oldMatrix->Delete();
  newTransform->Delete();
}

void vtkInteractorStyleJoystickActor::Prop3DTransform(vtkProp3D *prop3D,
						      float *boxCenter,
						      int numRotation,
						      double **rotate,
						      double *scale)
{
  double boxCenter2[3];
  boxCenter2[0] = boxCenter[0];
  boxCenter2[1] = boxCenter[1];
  boxCenter2[2] = boxCenter[2];
  this->Prop3DTransform(prop3D,boxCenter2,numRotation,rotate,scale);
}

void vtkInteractorStyleJoystickActor::OnTimer(void) 
{
  vtkRenderWindowInteractor *rwi = this->Interactor;
  switch (this->State) 
    {
    case VTKIS_START:
      if (this->AnimState == VTKIS_ANIM_ON)
        {
                rwi->DestroyTimer();
                rwi->Render();
                rwi->CreateTimer(VTKI_TIMER_FIRST);
         }
      break;
    case VTK_INTERACTOR_STYLE_ACTOR_ROTATE:
      this->RotateXY(this->LastPos[0], this->LastPos[1]);
      rwi->CreateTimer(VTKI_TIMER_UPDATE);
      break;
    case VTK_INTERACTOR_STYLE_ACTOR_PAN:
      this->PanXY(this->LastPos[0], this->LastPos[1]);
      rwi->CreateTimer(VTKI_TIMER_UPDATE);
      break;
    case VTK_INTERACTOR_STYLE_ACTOR_ZOOM:
      this->DollyXY(this->LastPos[0], this->LastPos[1]);
      rwi->CreateTimer(VTKI_TIMER_UPDATE);
      break;
    case VTK_INTERACTOR_STYLE_ACTOR_SPIN:
      this->SpinXY(this->LastPos[0], this->LastPos[1]);
      rwi->CreateTimer(VTKI_TIMER_UPDATE);
      break;
    case VTKIS_TIMER:
      rwi->Render();
      rwi->CreateTimer(VTKI_TIMER_UPDATE);
      break;
    default :
      break;
    }
}
