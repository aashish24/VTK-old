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
#include "vtkInteractorStyleImage.h"
#include "vtkObjectFactory.h"
#include "vtkMath.h"
#include "vtkCommand.h"

vtkCxxRevisionMacro(vtkInteractorStyleImage, "$Revision$");
vtkStandardNewMacro(vtkInteractorStyleImage);

//----------------------------------------------------------------------------
vtkInteractorStyleImage::vtkInteractorStyleImage() 
{
  this->MotionFactor = 10.0;
  this->RadianToDegree = 180.0 / vtkMath::Pi();

  this->WindowLevelStartPosition[0]   = 0;
  this->WindowLevelStartPosition[1]   = 0;  

  this->WindowLevelCurrentPosition[0] = 0;
  this->WindowLevelCurrentPosition[1] = 0;  
}

//----------------------------------------------------------------------------
vtkInteractorStyleImage::~vtkInteractorStyleImage() 
{
}

//----------------------------------------------------------------------------
void vtkInteractorStyleImage::StartWindowLevel() 
{
  if (this->State != VTKIS_START) 
    {
    return;
    }
  this->StartState(VTKIS_WINDOW_LEVEL);
}
//----------------------------------------------------------------------------
void vtkInteractorStyleImage::EndWindowLevel() 
{
  if (this->State != VTKIS_WINDOW_LEVEL) 
    {
    return;
    }
  this->StopState();
}
//----------------------------------------------------------------------------
void vtkInteractorStyleImage::StartPick() 
{
  if (this->State != VTKIS_START) 
    {
    return;
    }
  this->StartState(VTKIS_PICK);
}
//----------------------------------------------------------------------------
void vtkInteractorStyleImage::EndPick() 
{
  if (this->State != VTKIS_PICK) 
    {
    return;
    }
  this->StopState();
}
//----------------------------------------------------------------------------
void vtkInteractorStyleImage::OnMouseMove(int ctrl, 
                                          int shift, 
                                          int x, 
                                          int y) 
{
  switch (this->State) 
    {
    case VTKIS_WINDOW_LEVEL:
      this->FindPokedCamera(x, y);
      this->WindowLevelXY(x, y);
      break;

    case VTKIS_PAN:
      this->FindPokedCamera(x, y);
      this->PanXY(x, y, this->LastPos[0], this->LastPos[1]);
      break;

    case VTKIS_DOLLY:
      this->FindPokedCamera(x, y);
      this->DollyXY(x - this->LastPos[0], y - this->LastPos[1]);
      break;

    case VTKIS_SPIN:
      this->FindPokedCamera(x, y);
      this->SpinXY(x, y, this->LastPos[0], this->LastPos[1]);
      break;

    case VTKIS_PICK:
      this->FindPokedCamera(x, y);
      this->PickXY(x, y);
      break;
    }

  this->LastPos[0] = x;
  this->LastPos[1] = y;
}
//----------------------------------------------------------------------------
void vtkInteractorStyleImage::OnLeftButtonDown(int ctrl, 
                                               int shift, 
                                               int x, 
                                               int y) 
{
  this->FindPokedRenderer(x, y);
  if (this->CurrentRenderer == NULL)
    {
    return;
    }

  if (shift) 
    {
    if (ctrl) 
      {
      this->StartDolly();
      }
    else 
      {
      this->StartPan();
      }
    } 
  else 
    {
    if (ctrl) 
      {
      this->StartSpin();
      }
    else 
      {
      this->StartWindowLevel();
      this->WindowLevelStartPosition[0] = x;
      this->WindowLevelStartPosition[1] = y;      
      if (this->HasObserver(vtkCommand::StartWindowLevelEvent)) 
        {
        this->InvokeEvent(vtkCommand::StartWindowLevelEvent,this);
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleImage::OnLeftButtonUp(int vtkNotUsed(ctrl), 
                                             int vtkNotUsed(shift), 
                                             int vtkNotUsed(x), 
                                             int vtkNotUsed(y))
{
  switch (this->State) 
    {
    case VTKIS_DOLLY:
      this->EndDolly();
      break;

    case VTKIS_PAN:
      this->EndPan();
      break;

    case VTKIS_SPIN:
      this->EndSpin();
      break;

    case VTKIS_WINDOW_LEVEL:
      if (this->HasObserver(vtkCommand::EndWindowLevelEvent))
        {
        this->InvokeEvent(vtkCommand::EndWindowLevelEvent, this);
        }
      this->EndWindowLevel();
      break;
    }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleImage::OnMiddleButtonDown(int vtkNotUsed(ctrl), 
                                                 int vtkNotUsed(shift), 
                                                 int x, 
                                                 int y) 
{
  this->FindPokedRenderer(x, y);
  if (this->CurrentRenderer == NULL)
    {
    return;
    }
  
  this->StartPan();
}
//----------------------------------------------------------------------------
void vtkInteractorStyleImage::OnMiddleButtonUp(int vtkNotUsed(ctrl), 
                                               int vtkNotUsed(shift), 
                                               int vtkNotUsed(x), 
                                               int vtkNotUsed(y))
{
  switch (this->State) 
    {
    case VTKIS_PAN:
      this->EndPan();
      break;
    }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleImage::OnRightButtonDown(int vtkNotUsed(ctrl), 
                                                int shift, 
                                                int x, 
                                                int y) 
{
  this->FindPokedRenderer(x, y);
  if (this->CurrentRenderer == NULL)
    {
    return;
    }

  if (shift)
    {
    this->StartPick();
    if (this->HasObserver(vtkCommand::StartPickEvent)) 
      {
      this->InvokeEvent(vtkCommand::StartPickEvent, this);
      }
    }
  else 
    {
    this->StartDolly();
    }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleImage::OnRightButtonUp(int vtkNotUsed(ctrl), 
                                              int vtkNotUsed(shift), 
                                              int vtkNotUsed(x), 
                                              int vtkNotUsed(y)) 
{
  switch (this->State) 
    {
    case VTKIS_PICK:
      if (this->HasObserver(vtkCommand::EndPickEvent)) 
        {
        this->InvokeEvent(vtkCommand::EndPickEvent, this);
        }
      this->EndPick();
      break;

    case VTKIS_DOLLY:
      this->EndDolly();
      break;
    }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleImage::OnChar(int ctrl, 
                                     int shift, 
                                     char keycode, 
                                     int repeatcount) 
{
  vtkRenderWindowInteractor *rwi = this->Interactor;

  switch (keycode) 
    {
    case 'f' :      
    case 'F' :
      {
      this->AnimState = VTKIS_ANIM_ON;
      vtkAssemblyPath *path=NULL;
      this->FindPokedRenderer(this->LastPos[0],this->LastPos[1]);
      rwi->GetPicker()->Pick(this->LastPos[0],this->LastPos[1], 0.0, 
                             this->CurrentRenderer);
      vtkAbstractPropPicker *picker;
      if ( (picker=vtkAbstractPropPicker::SafeDownCast(rwi->GetPicker())) )
        {
        path = picker->GetPath();
        }
      if ( path != NULL )
        {
        rwi->FlyToImage(this->CurrentRenderer,picker->GetPickPosition());
        }
      this->AnimState = VTKIS_ANIM_OFF;
      break;
      }

    case 'r' :      
    case 'R' :
      // Allow either shift/ctrl to trigger the usual 'r' binding
      // otherwise trigger reset window level event
      if (shift || ctrl)
        {
        this->Superclass::OnChar(ctrl, shift, keycode, repeatcount);
        }
      else
        {
        if (this->HasObserver(vtkCommand::ResetWindowLevelEvent)) 
          {
          this->InvokeEvent(vtkCommand::ResetWindowLevelEvent, this);
          }
        }
      break;

    default:
      this->Superclass::OnChar(ctrl, shift, keycode, repeatcount);
      break;
    }
}


//----------------------------------------------------------------------------
void vtkInteractorStyleImage::WindowLevelXY(int x, int y)
{
  this->WindowLevelCurrentPosition[0] = x;
  this->WindowLevelCurrentPosition[1] = y;
  
  if (this->HasObserver(vtkCommand::WindowLevelEvent)) 
    {
    this->InvokeEvent(vtkCommand::WindowLevelEvent, this);
    }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleImage::PanXY(int x, int y, int oldX, int oldY)
{
  vtkCamera *cam;
  double viewFocus[4], focalDepth, viewPoint[3];
  float newPickPoint[4], oldPickPoint[4], motionVector[3];
  
  if (this->CurrentRenderer == NULL)
    {
    return;
    }

  // calculate the focal depth since we'll be using it a lot
  cam = this->CurrentRenderer->GetActiveCamera();
  cam->GetFocalPoint(viewFocus);
  this->ComputeWorldToDisplay(viewFocus[0], viewFocus[1],
                              viewFocus[2], viewFocus);
  focalDepth = viewFocus[2];

  this->ComputeDisplayToWorld(double(x), double(y),
                              focalDepth, newPickPoint);
    
  // has to recalc old mouse point since the viewport has moved,
  // so can't move it outside the loop
  this->ComputeDisplayToWorld(double(oldX),double(oldY),
                              focalDepth, oldPickPoint);
  
  // camera motion is reversed
  motionVector[0] = oldPickPoint[0] - newPickPoint[0];
  motionVector[1] = oldPickPoint[1] - newPickPoint[1];
  motionVector[2] = oldPickPoint[2] - newPickPoint[2];
  
  cam->GetFocalPoint(viewFocus);
  cam->GetPosition(viewPoint);
  cam->SetFocalPoint(motionVector[0] + viewFocus[0],
                     motionVector[1] + viewFocus[1],
                     motionVector[2] + viewFocus[2]);
  cam->SetPosition(motionVector[0] + viewPoint[0],
                   motionVector[1] + viewPoint[1],
                   motionVector[2] + viewPoint[2]);
      
  vtkRenderWindowInteractor *rwi = this->Interactor;
  if (this->CurrentLight)
    {
    /* get the first light */
    this->CurrentLight->SetPosition(cam->GetPosition());
    this->CurrentLight->SetFocalPoint(cam->GetFocalPoint());
    }
  
  rwi->Render();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleImage::DollyXY(int vtkNotUsed(dx), int dy)
{
  vtkCamera *cam;
  double dyf = this->MotionFactor * (double)(dy) / (double)(this->Center[1]);
  double zoomFactor = pow((double)1.1, dyf);
  
  if (this->CurrentRenderer == NULL)
    {
    return;
    }
  
  cam = this->CurrentRenderer->GetActiveCamera();
  if (cam->GetParallelProjection())
    {
    cam->SetParallelScale(cam->GetParallelScale()/zoomFactor);
    }
  else
    {
    cam->Dolly(zoomFactor);
    this->ResetCameraClippingRange();
    }
  
  vtkRenderWindowInteractor *rwi = this->Interactor;
  if (this->CurrentLight)
    {
    /* get the first light */
    this->CurrentLight->SetPosition(cam->GetPosition());
    this->CurrentLight->SetFocalPoint(cam->GetFocalPoint());
    }
  
  rwi->Render();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleImage::SpinXY(int x, int y, int oldX, int oldY)
{
  vtkRenderWindowInteractor *rwi = this->Interactor;
  vtkCamera *cam;

  if (this->CurrentRenderer == NULL)
    {
    return;
    }

  double newAngle = atan2((double)(y - this->Center[1]),
                         (double)(x - this->Center[0]));
  double oldAngle = atan2((double)(oldY -this->Center[1]),
                         (double)(oldX - this->Center[0]));
  
  newAngle *= this->RadianToDegree;
  oldAngle *= this->RadianToDegree;

  cam = this->CurrentRenderer->GetActiveCamera();
  cam->Roll(newAngle - oldAngle);
  cam->OrthogonalizeViewUp();
      
  rwi->Render();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleImage::PickXY(int vtkNotUsed(x), int vtkNotUsed(y))
{
  if (this->HasObserver(vtkCommand::PickEvent)) 
    {
    this->InvokeEvent(vtkCommand::PickEvent, this);
    }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleImage::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  
  os << indent << "Window Level Current Position: " <<
    this->WindowLevelCurrentPosition << endl;

  os << indent << "Window Level Start Position: " <<
    this->WindowLevelStartPosition << endl;
}
