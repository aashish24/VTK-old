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
authors and that existing copyright notices are retained in all copies. 
Some
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
#include "vtkInteractorStyleTrackball.h"
#include "vtkMath.h"
#include "vtkCellPicker.h"
#include "vtkRenderWindowInteractor.h"

vtkInteractorStyleTrackball::vtkInteractorStyleTrackball()
{
  // for actor interactions
  this->MotionFactor = 10.0;
  this->InteractionPicker = vtkCellPicker::New();
  // set a tight tolerance so picking will be precise.
  this->InteractionPicker->SetTolerance(0.001);
  this->ActorPicked = 0;
  this->InteractionActor = NULL;

  // set to default modes
  this->TrackballMode = VTKIS_JOY;
  this->ActorMode = VTKIS_CAMERA;
  this->ControlMode = VTKIS_CONTROL_OFF;
  this->OldX = 0.0;
  this->OldY = 0.0;
  this->Preprocess = 1;
  this->RadianToDegree = 180.0 / vtkMath::Pi();

  this->NewPickPoint[0] = 0.0;
  this->NewPickPoint[1] = 0.0;
  this->NewPickPoint[2] = 0.0;
  this->NewPickPoint[3] = 1.0;
  this->OldPickPoint[0] = 0.0;
  this->OldPickPoint[1] = 0.0;
  this->OldPickPoint[2] = 0.0;
  this->OldPickPoint[3] = 1.0;
  this->MotionVector[0] = 0.0;
  this->MotionVector[1] = 0.0;
  this->MotionVector[2] = 0.0;
  this->ViewLook[0] = 0.0;
  this->ViewLook[1] = 0.0;
  this->ViewLook[2] = 0.0;
  this->ViewPoint[0] = 0.0;
  this->ViewPoint[1] = 0.0;
  this->ViewPoint[2] = 0.0;
  this->ViewFocus[0] = 0.0;
  this->ViewFocus[1] = 0.0;
  this->ViewFocus[2] = 0.0;
  this->ViewUp[0] = 0.0;
  this->ViewUp[1] = 0.0;
  this->ViewUp[2] = 0.0;
  this->ViewRight[0] = 0.0;
  this->ViewRight[1] = 0.0;
  this->ViewRight[2] = 0.0;  

  this->Origin[0] = 0.0;
  this->Origin[1] = 0.0;
  this->Origin[2] = 0.0;
  this->Position[0] = 0.0;
  this->Position[1] = 0.0;
  this->Position[2] = 0.0;
  this->ObjCenter[0] = 0.0;
  this->ObjCenter[1] = 0.0;
  this->ObjCenter[2] = 0.0;  
  this->DispObjCenter[0] = 0.0;
  this->DispObjCenter[1] = 0.0;
  this->DispObjCenter[2] = 0.0;
  this->Radius = 0.0;
}

vtkInteractorStyleTrackball::~vtkInteractorStyleTrackball() 
{
  this->InteractionPicker->Delete();
}

//----------------------------------------------------------------------------
// Implementations of Joystick Camera/Actor motions follow
//----------------------------------------------------------------------------
// Description:
// rotate the camera in trackball (motion sensitive) style
void vtkInteractorStyleTrackball::TrackballRotateCamera(int x, int y)
{
  if ((this->OldX != x) || (this->OldY != y))
    {
    double rxf = (double)(x - this->OldX) * this->DeltaAzimuth *
      this->MotionFactor;
    double ryf = (double)(y - this->OldY) * this->DeltaElevation *
      this->MotionFactor;
    
    this->CurrentCamera->Azimuth(rxf);
    this->CurrentCamera->Elevation(ryf);
    this->CurrentCamera->OrthogonalizeViewUp();
    vtkRenderWindowInteractor *rwi = this->Interactor;
    if (rwi->GetLightFollowCamera())
      {
      // get the first light
      this->CurrentLight->SetPosition(this->CurrentCamera->GetPosition());
      this->CurrentLight->SetFocalPoint(this->CurrentCamera->GetFocalPoint());
      }	
    this->OldX = x;
    this->OldY = y;
    rwi->Render();
    }
}

// Description:
// spin the camera in trackball (motion sensitive) style
void vtkInteractorStyleTrackball::TrackballSpinCamera(int x, int y)
{
  vtkRenderWindowInteractor *rwi = this->Interactor;
  if ((this->OldX != x) || (this->OldY != y))
    {
    double newAngle = atan2((double)(y - this->Center[1]),
                           (double)(x - this->Center[0]));
    double oldAngle = atan2((double)(this->OldY -this->Center[1]),
                           (double)(this->OldX - this->Center[0]));
  
    newAngle *= this->RadianToDegree;
    oldAngle *= this->RadianToDegree;

    this->CurrentCamera->Roll(newAngle - oldAngle);
    this->CurrentCamera->OrthogonalizeViewUp();
      
    this->OldX = x;
    this->OldY = y;
    rwi->Render();
    }
}


// Description:
// pan the camera in trackball (motion sensitive) style
void vtkInteractorStyleTrackball::TrackballPanCamera(int x, int y)
{
  vtkRenderWindowInteractor *rwi = this->Interactor;
  if ((this->OldX != x) || (this->OldY != y))
    {
    if (this->Preprocess)
      {
      // calculate the focal depth since we'll be using it a lot
      this->CurrentCamera->GetFocalPoint(this->ViewFocus);
      this->ComputeWorldToDisplay(this->ViewFocus[0], this->ViewFocus[1],
                                  this->ViewFocus[2], this->ViewFocus);
      this->FocalDepth = this->ViewFocus[2];

      this->Preprocess = 0;
      }

    this->ComputeDisplayToWorld(double(x), double(y),
                                this->FocalDepth,
                                this->NewPickPoint);
    
    // has to recalc old mouse point since the viewport has moved,
    // so can't move it outside the loop
    this->ComputeDisplayToWorld(double(this->OldX),double(this->OldY),
                                this->FocalDepth, this->OldPickPoint);

    // camera motion is reversed
    this->MotionVector[0] = this->OldPickPoint[0] - this->NewPickPoint[0];
    this->MotionVector[1] = this->OldPickPoint[1] - this->NewPickPoint[1];
    this->MotionVector[2] = this->OldPickPoint[2] - this->NewPickPoint[2];
    
    this->CurrentCamera->GetFocalPoint(this->ViewFocus);
    this->CurrentCamera->GetPosition(this->ViewPoint);
    this->CurrentCamera->SetFocalPoint(this->MotionVector[0] +
                                       this->ViewFocus[0],
                                       this->MotionVector[1] +
                                       this->ViewFocus[1],
                                       this->MotionVector[2] +
                                       this->ViewFocus[2]);
    this->CurrentCamera->SetPosition(this->MotionVector[0] +
                                     this->ViewPoint[0],
                                     this->MotionVector[1] +
                                     this->ViewPoint[1],
                                     this->MotionVector[2] +
                                     this->ViewPoint[2]);
      
    vtkRenderWindowInteractor *rwi = this->Interactor;
    if (rwi->GetLightFollowCamera())
      {
      /* get the first light */
      this->CurrentLight->SetPosition(this->CurrentCamera->GetPosition());
      this->CurrentLight->SetFocalPoint(this->CurrentCamera->GetFocalPoint());
      }
    
    this->OldX = x;
    this->OldY = y;
    rwi->Render();
    }
}

// Description:
// dolly the camera in trackball (motion sensitive) style
// dolly is based on distance from center of screen,
// and the upper half is positive, lower half is negative
void vtkInteractorStyleTrackball::TrackballDollyCamera(int x, int y)
{
  if (this->OldY != y)
    {
    double dyf = this->MotionFactor * (double)(y - this->OldY) /
      (double)(this->Center[1]);
    double zoomFactor = pow((double)1.1, dyf);
          
    if (this->CurrentCamera->GetParallelProjection())
      {
      this->CurrentCamera->
        SetParallelScale(this->CurrentCamera->GetParallelScale()/zoomFactor);
      }
    else
      {
      double *clippingRange = this->CurrentCamera->GetClippingRange();
      this->CurrentCamera->SetClippingRange(clippingRange[0]/zoomFactor,
                                            clippingRange[1]/zoomFactor);
      this->CurrentCamera->Dolly(zoomFactor);
      }
    
    vtkRenderWindowInteractor *rwi = this->Interactor;
    if (rwi->GetLightFollowCamera())
      {
      /* get the first light */
      this->CurrentLight->SetPosition(this->CurrentCamera->GetPosition());
      this->CurrentLight->SetFocalPoint(this->CurrentCamera->GetFocalPoint());
      }

    this->OldX = x;
    this->OldY = y;
    rwi->Render();
    }
}

// Description:
// rotate the actor in trackball (motion sensitive) style
void vtkInteractorStyleTrackball::TrackballRotateActor(int x, int y)
{
  vtkRenderWindowInteractor *rwi = this->Interactor;
  if ((this->OldX != x) || (this->OldY != y))
    {

    if (this->Preprocess)
      {
      float *center = this->InteractionActor->GetCenter();
      this->ObjCenter[0] = center[0];
      this->ObjCenter[1] = center[1];
      this->ObjCenter[2] = center[2];

      // GetLength gets the length of the diagonal of the bounding box
      double boundRadius = this->InteractionActor->GetLength() * 0.5;

      // get the view up and view right vectors
      this->CurrentCamera->OrthogonalizeViewUp();
      this->CurrentCamera->ComputeViewPlaneNormal();
      this->CurrentCamera->GetViewUp(this->ViewUp);
      vtkMath::Normalize(this->ViewUp);
      this->CurrentCamera->GetViewPlaneNormal(this->ViewLook);
      vtkMath::Cross(this->ViewUp, this->ViewLook, this->ViewRight);
      vtkMath::Normalize(this->ViewRight);

      // get the furtherest point from object position+origin
      double outsidept[3];
      outsidept[0] = this->ObjCenter[0] + this->ViewRight[0] * boundRadius;
      outsidept[1] = this->ObjCenter[1] + this->ViewRight[1] * boundRadius;
      outsidept[2] = this->ObjCenter[2] + this->ViewRight[2] * boundRadius;

      // convert them to display coord
      this->ComputeWorldToDisplay(this->ObjCenter[0], this->ObjCenter[1],
                                  this->ObjCenter[2], this->DispObjCenter);
      this->ComputeWorldToDisplay(outsidept[0], outsidept[1],
                                  outsidept[2], outsidept);

      // get the radius in display coord
      double ftmp[3];
      ftmp[0] = this->DispObjCenter[0];
      ftmp[1] = this->DispObjCenter[1];
      ftmp[2] = this->DispObjCenter[2];

      this->Radius =
	sqrt(vtkMath::Distance2BetweenPoints(ftmp, outsidept));

      this->HighlightActor(NULL);
      this->Preprocess = 0;
      }

    double nxf = (double)(x - this->DispObjCenter[0]) / this->Radius;
    double nyf = (double)(y - this->DispObjCenter[1]) / this->Radius;
    double oxf = (double)(this->OldX - this->DispObjCenter[0]) / this->Radius;
    double oyf = (double)(this->OldY - this->DispObjCenter[1]) / this->Radius;

    if (((nxf * nxf + nyf * nyf) <= 1.0) &&
        ((oxf * oxf + oyf * oyf) <= 1.0))
      {
	    
      double newXAngle = asin(nxf) * this->RadianToDegree;
      double newYAngle = asin(nyf) * this->RadianToDegree;
      double oldXAngle = asin(oxf) * this->RadianToDegree;
      double oldYAngle = asin(oyf) * this->RadianToDegree;

      double scale[3];
      scale[0] = scale[1] = scale[2] = 1.0;
      double **rotate = new double*[2];
      rotate[0] = new double[4];
      rotate[1] = new double[4];

      rotate[0][0] = newXAngle - oldXAngle;
      rotate[0][1] = this->ViewUp[0];
      rotate[0][2] = this->ViewUp[1];
      rotate[0][3] = this->ViewUp[2];
      
      rotate[1][0] = oldYAngle - newYAngle;
      rotate[1][1] = this->ViewRight[0];
      rotate[1][2] = this->ViewRight[1];
      rotate[1][3] = this->ViewRight[2];
      
      
      this->ActorTransform(this->InteractionActor,
                           this->ObjCenter,
                           2, rotate, scale);

      delete [] rotate[0];
      delete [] rotate[1];
      delete [] rotate;
      
      this->OldX = x;
      this->OldY = y;
      rwi->Render();
      }
    }
}

// Description:
// spin the actor in trackball (motion sensitive) style
void vtkInteractorStyleTrackball::TrackballSpinActor(int x, int y)
{
  vtkRenderWindowInteractor *rwi = this->Interactor;
  if ((this->OldX != x) || (this->OldY != y))
    {
    if (this->Preprocess)
      {
      // get the position plus origin of the object
      float *center = this->InteractionActor->GetCenter();
      this->ObjCenter[0] = center[0];
      this->ObjCenter[1] = center[1];
      this->ObjCenter[2] = center[2];

      // get the axis to rotate around = vector from eye to origin
      if (this->CurrentCamera->GetParallelProjection())
        {
        this->CurrentCamera->ComputeViewPlaneNormal();
        this->CurrentCamera->GetViewPlaneNormal(this->MotionVector);
        }
      else
        {   
        this->CurrentCamera->GetPosition(this->ViewPoint);
        this->MotionVector[0] = this->ViewPoint[0] - this->ObjCenter[0];
        this->MotionVector[1] = this->ViewPoint[1] - this->ObjCenter[1];
        this->MotionVector[2] = this->ViewPoint[2] - this->ObjCenter[2];
        vtkMath::Normalize(this->MotionVector);
        }
      
      this->ComputeWorldToDisplay(this->ObjCenter[0], this->ObjCenter[1],
                                  this->ObjCenter[2], this->DispObjCenter);

      this->HighlightActor(NULL);
      this->Preprocess = 0;
      }
    
    // this has to be in the loop
    double newAngle = atan2((double)(y - this->DispObjCenter[1]),
                           (double)(x - this->DispObjCenter[0]));
    double oldAngle = atan2((double)(this->OldY - this->DispObjCenter[1]),
                           (double)(this->OldX - this->DispObjCenter[0]));
    
    newAngle *= this->RadianToDegree;
    oldAngle *= this->RadianToDegree;

    double scale[3];
    scale[0] = scale[1] = scale[2] = 1.0;
    double **rotate = new double*[1];
    rotate[0] = new double[4];

    rotate[0][0] = newAngle - oldAngle;
    rotate[0][1] = this->MotionVector[0];
    rotate[0][2] = this->MotionVector[1];
    rotate[0][3] = this->MotionVector[2];
  
    this->ActorTransform(this->InteractionActor,
                         this->ObjCenter,
                         1, rotate, scale);

    delete [] rotate[0];
    delete [] rotate;
    
    this->OldX = x;
    this->OldY = y;
    rwi->Render();
    }
}

// Description:
// pan the actor in trackball (motion sensitive) style
void vtkInteractorStyleTrackball::TrackballPanActor(int x, int y)
{
  vtkRenderWindowInteractor *rwi = this->Interactor;
  if ((this->OldX != x) || (this->OldY != y))
    {
    if (this->Preprocess)
      {
      // use initial center as the origin from which to pan
      float *center = this->InteractionActor->GetCenter();
      this->ObjCenter[0] = center[0];
      this->ObjCenter[1] = center[1];
      this->ObjCenter[2] = center[2];
      this->ComputeWorldToDisplay(this->ObjCenter[0], this->ObjCenter[1],
                                  this->ObjCenter[2], this->DispObjCenter);
      this->FocalDepth = this->DispObjCenter[2];
      
      this->HighlightActor(NULL);
      this->Preprocess = 0;
      }
  
    this->ComputeDisplayToWorld(double(x), double(y),
                                this->FocalDepth,
                                this->NewPickPoint);

    this->ComputeDisplayToWorld(double(this->OldX), double(this->OldY),
                                this->FocalDepth, this->OldPickPoint);

    this->MotionVector[0] = this->NewPickPoint[0] - this->OldPickPoint[0];
    this->MotionVector[1] = this->NewPickPoint[1] - this->OldPickPoint[1];
    this->MotionVector[2] = this->NewPickPoint[2] - this->OldPickPoint[2];

    if (this->InteractionActor->GetUserMatrix() != NULL)
      {
      vtkTransform *t = vtkTransform::New();
      t->PostMultiply();
      t->SetMatrix(*(this->InteractionActor->GetUserMatrix()));
      t->Translate(this->MotionVector[0], this->MotionVector[1], 
		   this->MotionVector[2]);
      this->InteractionActor->GetUserMatrix()->DeepCopy(t->GetMatrixPointer());
      t->Delete();
      }
    else
      {
      this->InteractionActor->AddPosition(this->MotionVector);
      }
      
    this->OldX = x;
    this->OldY = y;
    rwi->Render();
    }
}


// Description:
// Dolly the actor in trackball (motion sensitive) style
void vtkInteractorStyleTrackball::TrackballDollyActor(int x, int y)
{
  vtkRenderWindowInteractor *rwi = this->Interactor;
  if (this->OldY != y)
    {
    if (this->Preprocess)
      {
      this->CurrentCamera->GetPosition(this->ViewPoint);
      this->CurrentCamera->GetFocalPoint(this->ViewFocus);

      this->HighlightActor(NULL);
      this->Preprocess = 0;
      }
    
    double yf = (double)(this->OldY - y) / (double)(this->Center[1]) *
      this->MotionFactor;
    double dollyFactor = pow((double)1.1, yf);

    dollyFactor -= 1.0;
    this->MotionVector[0] = (this->ViewPoint[0] -
                             this->ViewFocus[0]) * dollyFactor;
    this->MotionVector[1] = (this->ViewPoint[1] -
                             this->ViewFocus[1]) * dollyFactor;
    this->MotionVector[2] = (this->ViewPoint[2] -
                             this->ViewFocus[2]) * dollyFactor;
    
    if (this->InteractionActor->GetUserMatrix() != NULL)
      {
      vtkTransform *t = vtkTransform::New();
      t->PostMultiply();
      t->SetMatrix(*(this->InteractionActor->GetUserMatrix()));
      t->Translate(this->MotionVector[0], this->MotionVector[1], 
		   this->MotionVector[2]);
      this->InteractionActor->GetUserMatrix()->DeepCopy(t->GetMatrixPointer());
      t->Delete();
      }
    else
      {
      this->InteractionActor->AddPosition(this->MotionVector);
      }
  
    this->OldX = x;
    this->OldY = y;
    rwi->Render();
    }
}

// Description:
// Scale the actor in trackball (motion sensitive) style
void vtkInteractorStyleTrackball::TrackballScaleActor(int x, int y)
{
  vtkRenderWindowInteractor *rwi = this->Interactor;
  if ((this->OldX != x) || (this->OldY != y))
    {
    if (this->Preprocess)
      {
      float *center = this->InteractionActor->GetCenter();
      this->ObjCenter[0] = center[0];
      this->ObjCenter[1] = center[1];
      this->ObjCenter[2] = center[2];

      this->HighlightActor(NULL);
      this->Preprocess = 0;
      }
    
    double yf = (double)(y - this->OldY) / (double)(this->Center[1]) *
      this->MotionFactor;
    double scaleFactor = pow((double)1.1, yf);

    double **rotate = NULL;

    double scale[3];
    scale[0] = scale[1] = scale[2] = scaleFactor;

    this->ActorTransform(this->InteractionActor,
                         this->ObjCenter,
                         0, rotate, scale);

    this->OldX = x;
    this->OldY = y;
    rwi->Render();
    }
}

void vtkInteractorStyleTrackball::PrintSelf(ostream& os, vtkIndent indent) 
{
  this->vtkInteractorStyle::PrintSelf(os,indent);

  os << indent << "Interaction Picker: " << this->InteractionPicker;
  os << indent << "Actor Picked: " <<
    (this->ActorPicked ? "Yes\n" : "No\n");
  if ( this->InteractionActor )
    {
    os << indent << "Interacting Actor: " << this->InteractionActor << "\n";
    }
  else
    {
    os << indent << "Interacting Actor: (none)\n";
    }
  os << indent << "Mode: " <<
    (this->ActorMode ? "Actor\n" : "Camera\n");
  os << indent << "Mode: " <<
    (this->TrackballMode ? "Trackball\n" : "Joystick\n");
  os << indent << "Control Key: " <<
    (this->ControlMode ? "On\n" : "Off\n");
  os << indent << "Preprocessing: " <<
    (this->Preprocess ? "Yes\n" : "No\n");

}

void vtkInteractorStyleTrackball::JoystickRotateActor(int x, int y)
{
  vtkRenderWindowInteractor *rwi = this->Interactor;
  if (this->Preprocess)
    {
    // first get the origin of the assembly
    float *center = this->InteractionActor->GetCenter();
    this->ObjCenter[0] = center[0];
    this->ObjCenter[1] = center[1];
    this->ObjCenter[2] = center[2];

    // GetLength gets the length of the diagonal of the bounding box
    double boundRadius = this->InteractionActor->GetLength() * 0.5;

    // get the view up and view right vectors
    this->CurrentCamera->OrthogonalizeViewUp();
    this->CurrentCamera->ComputeViewPlaneNormal();
    this->CurrentCamera->GetViewUp(this->ViewUp);
    vtkMath::Normalize(this->ViewUp);
    this->CurrentCamera->GetViewPlaneNormal(this->ViewLook);
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

    this->HighlightActor(NULL);
    this->Preprocess = 0;
    }


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


  this->ActorTransform(this->InteractionActor,
                       this->ObjCenter,
                       2, rotate, scale);

  delete [] rotate[0];
  delete [] rotate[1];
  delete [] rotate;

  rwi->Render();
}

void vtkInteractorStyleTrackball::JoystickSpinActor(int x, int y)
{
  vtkRenderWindowInteractor *rwi = this->Interactor;
  // get the axis to rotate around = vector from eye to origin
  if (this->Preprocess)
    {

    float *center = this->InteractionActor->GetCenter();
    this->ObjCenter[0] = center[0];
    this->ObjCenter[1] = center[1];
    this->ObjCenter[2] = center[2];

    if (this->CurrentCamera->GetParallelProjection())
      {
      // if parallel projection, want to get the view plane normal...
      this->CurrentCamera->ComputeViewPlaneNormal();
      this->CurrentCamera->GetViewPlaneNormal(this->MotionVector);
      }
    else
      {
      // perspective projection, get vector from eye to center of actor
      this->CurrentCamera->GetPosition(this->ViewPoint);
      this->MotionVector[0] = this->ViewPoint[0] - this->ObjCenter[0];
      this->MotionVector[1] = this->ViewPoint[1] - this->ObjCenter[1];
      this->MotionVector[2] = this->ViewPoint[2] - this->ObjCenter[2];
      vtkMath::Normalize(this->MotionVector);
      }

    this->ComputeWorldToDisplay(this->ObjCenter[0], this->ObjCenter[1],
                                this->ObjCenter[2], this->DispObjCenter);

    this->HighlightActor(NULL);
    this->Preprocess = 0;
    }

  double yf = (double)(y -this->DispObjCenter[1]) / (double)(this->Center[1]);
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

  this->ActorTransform(this->InteractionActor,
                       this->ObjCenter,
                       1, rotate, scale);

  delete [] rotate[0];
  delete [] rotate;

  rwi->Render();
}

void vtkInteractorStyleTrackball::JoystickPanActor(int x, int y)
{
  vtkRenderWindowInteractor *rwi = this->Interactor;
  if (this->Preprocess)
    {
    // use initial center as the origin from which to pan
    float *center = this->InteractionActor->GetCenter();
    this->ObjCenter[0] = center[0];
    this->ObjCenter[1] = center[1];
    this->ObjCenter[2] = center[2];

    this->ComputeWorldToDisplay(this->ObjCenter[0], this->ObjCenter[1],
                                this->ObjCenter[2], this->DispObjCenter);
    this->FocalDepth = this->DispObjCenter[2];

    this->HighlightActor(NULL);
    this->Preprocess = 0;
    }

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

  if (this->InteractionActor->GetUserMatrix() != NULL)
    {
    vtkTransform *t = vtkTransform::New();
    t->PostMultiply();
    t->SetMatrix(*(this->InteractionActor->GetUserMatrix()));
    t->Translate(this->MotionVector[0], this->MotionVector[1],
		 this->MotionVector[2]);
    this->InteractionActor->GetUserMatrix()->DeepCopy(t->GetMatrixPointer());
    t->Delete();
    }
  else
    {
    this->InteractionActor->AddPosition(this->MotionVector);
    }

  rwi->Render();
}

void vtkInteractorStyleTrackball::JoystickDollyActor(int x, int y)
{
  vtkRenderWindowInteractor *rwi = this->Interactor;
  // dolly is based on distance from center of screen,
  // and the upper half is positive, lower half is negative

  if (this->Preprocess)
    {
    this->CurrentCamera->GetPosition(this->ViewPoint);
    this->CurrentCamera->GetFocalPoint(this->ViewFocus);

    // use initial center as the origin from which to pan
    float *center = this->InteractionActor->GetCenter();
    this->ObjCenter[0] = center[0];
    this->ObjCenter[1] = center[1];
    this->ObjCenter[2] = center[2];
    this->ComputeWorldToDisplay(this->ObjCenter[0], this->ObjCenter[1],
                                this->ObjCenter[2], this->DispObjCenter);

    this->HighlightActor(NULL);
    this->Preprocess = 0;
    }

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

  if (this->InteractionActor->GetUserMatrix() != NULL)
    {
    vtkTransform *t = vtkTransform::New();
    t->PostMultiply();
    t->SetMatrix(*(this->InteractionActor->GetUserMatrix()));
    t->Translate(this->MotionVector[0], this->MotionVector[1],
		 this->MotionVector[2]);
    this->InteractionActor->GetUserMatrix()->DeepCopy(t->GetMatrixPointer());
    t->Delete();
    }
  else
    {
    this->InteractionActor->AddPosition(this->MotionVector);
    }

  rwi->Render();
}

void vtkInteractorStyleTrackball::JoystickScaleActor(int x, int y)
{
  vtkRenderWindowInteractor *rwi = this->Interactor;
  // Uniform scale is based on distance from center of screen,
  // and the upper half is positive, lower half is negative

  if (this->Preprocess)
    {
    // use bounding box center as the origin from which to pan
    float *center = this->InteractionActor->GetCenter();
    this->ObjCenter[0] = center[0];
    this->ObjCenter[1] = center[1];
    this->ObjCenter[2] = center[2];

    this->ComputeWorldToDisplay(this->ObjCenter[0], this->ObjCenter[1],
                                this->ObjCenter[2], this->DispObjCenter);

    this->HighlightActor(NULL);
    this->Preprocess = 0;
    }

  double yf = (double)(y - this->DispObjCenter[1]) /
    (double)(this->Center[1]);
  double scaleFactor = pow((double)1.1, yf);

  double **rotate = NULL;

  double scale[3];
  scale[0] = scale[1] = scale[2] = scaleFactor;

  this->ActorTransform(this->InteractionActor,
                       this->ObjCenter,
                       0, rotate, scale);

  rwi->Render();
}


//----------------------------------------------------------------------------
void vtkInteractorStyleTrackball::ActorTransform(vtkActor *actor,
                                                 double *boxCenter,
                                                 int numRotation,
                                                 double **rotate,
                                                 double *scale)
{
  vtkMatrix4x4 *oldMatrix = vtkMatrix4x4::New();
  actor->GetMatrix(oldMatrix);
  
  float orig[3];
  actor->GetOrigin(orig);
  
  vtkTransform *newTransform = vtkTransform::New();
  newTransform->PostMultiply();
  if (actor->GetUserMatrix() != NULL) 
    {
    newTransform->SetMatrix(*(actor->GetUserMatrix()));
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
  
  if (actor->GetUserMatrix() != NULL) 
    {
    newTransform->GetMatrix(actor->GetUserMatrix());
    }
  else 
    {
    actor->SetPosition(newTransform->GetPosition());
    actor->SetScale(newTransform->GetScale());
    actor->SetOrientation(newTransform->GetOrientation());
    }
  oldMatrix->Delete();
  newTransform->Delete();
}



//----------------------------------------------------------------------------
// Intercept any keypresses which are style independent here and do the rest in
// subclasses - none really required yet!
//----------------------------------------------------------------------------
void vtkInteractorStyleTrackball::OnChar(int ctrl, int shift, 
                                         char keycode, int repeatcount) 
{
  // first invoke superclass method
  this->vtkInteractorStyle::OnChar(ctrl,shift,keycode,repeatcount);

  vtkRenderWindowInteractor *rwi = this->Interactor;

  // catch additional keycodes
  switch (keycode) 
    {
    case 'j':
    case 'J':
      if (this->State == VTKIS_START) 
        {
        this->TrackballMode = VTKIS_JOY;
        }
      break;
      
    case 't':
    case 'T':
      if (this->State == VTKIS_START) 
        {
        this->TrackballMode = VTKIS_TRACK;
        }
      break;
      
    case 'o':
    case 'O':
      if (this->State == VTKIS_START) 
        {
        if (this->ActorMode != VTKIS_ACTOR)
          {
          // reset the actor picking variables
          this->InteractionActor = NULL;
          this->ActorPicked = 0;
          this->HighlightActor(NULL);          
          this->ActorMode = VTKIS_ACTOR;
          }
        }
      break;
      
    case 'c':
    case 'C':
      if (this->State == VTKIS_START) 
        {
        if (this->ActorMode != VTKIS_CAMERA)
          {
          // reset the actor picking variables
          this->InteractionActor = NULL;
          this->ActorPicked = 0;
          this->HighlightActor(NULL);
          this->ActorMode = VTKIS_CAMERA;
          }
        }
      break;
    }
}

//----------------------------------------------------------------------------
// By overriding the RotateCamera, RotateActor members we can
// use this timer routine for Joystick or Trackball - quite tidy
//----------------------------------------------------------------------------
void vtkInteractorStyleTrackball::OnTimer(void) 
{
  vtkRenderWindowInteractor *rwi = this->Interactor;
  switch (this->State) 
    {
    case VTKIS_START:
      break;
      
    case VTKIS_ROTATE:  // rotate with respect to an axis perp to look
      if (this->ActorMode && this->ActorPicked)
        {
        if (this->TrackballMode)
          {
          this->TrackballRotateActor(this->LastPos[0],
                                   this->LastPos[1]);
          }
        else
          {
          this->JoystickRotateActor(this->LastPos[0],
                                  this->LastPos[1]);
          }
        }
      else if (!(this->ActorMode))
        {
        if (this->TrackballMode)
          {
          this->TrackballRotateCamera(this->LastPos[0],
                                    this->LastPos[1]);
          }
        else
          {
          this->RotateCamera(this->LastPos[0],this->LastPos[1]);
          }
        }
      rwi->CreateTimer(VTKI_TIMER_UPDATE);
      break;
      
    case VTKIS_PAN: // move perpendicular to camera's look vector
      if (this->ActorMode && this->ActorPicked)
        {
        if (this->TrackballMode)
          { 
          this->TrackballPanActor(this->LastPos[0],
                                  this->LastPos[1]);
          }
        else
          {
          this->JoystickPanActor(this->LastPos[0],
                                 this->LastPos[1]);
          }
        }
      else if (!(this->ActorMode))
        {
        if (this->TrackballMode)
          {
          this->TrackballPanCamera(this->LastPos[0],
                                   this->LastPos[1]);
          }
        else
          {
          this->PanCamera(this->LastPos[0],this->LastPos[1]);
          }
        }
      rwi->CreateTimer(VTKI_TIMER_UPDATE);
      break;
      
    case VTKIS_ZOOM:
      if (!(this->ActorMode))
        {
        if (this->TrackballMode)
          { 
          this->TrackballDollyCamera(this->LastPos[0],
                                   this->LastPos[1]);
          }
        else
          {
          this->DollyCamera(this->LastPos[0],this->LastPos[1]);
          }
        }
      rwi->CreateTimer(VTKI_TIMER_UPDATE);
      break;
      
    case VTKIS_SPIN:
      if (this->ActorMode && this->ActorPicked)
        {
        if (this->TrackballMode)
          { 
          this->TrackballSpinActor(this->LastPos[0],
                                 this->LastPos[1]);
          }
        else
          {
          this->JoystickSpinActor(this->LastPos[0],
                                this->LastPos[1]);
          }
        }
      else if (!(this->ActorMode))
        {
        if (this->TrackballMode)
          {
          this->TrackballSpinCamera(this->LastPos[0],
                                  this->LastPos[1]);
          }
        else
          {
          this->SpinCamera(this->LastPos[0],this->LastPos[1]);
          }
        }
      rwi->CreateTimer(VTKI_TIMER_UPDATE);
      break;
      
    case VTKIS_DOLLY:  // move along camera's view vector
      if (this->ActorMode && this->ActorPicked)
        {
        if (this->TrackballMode)
          { 
          this->TrackballDollyActor(this->LastPos[0],
                                  this->LastPos[1]);
          }
        else
          {
          this->JoystickDollyActor(this->LastPos[0],
                                 this->LastPos[1]);
          }
        }
      rwi->CreateTimer(VTKI_TIMER_UPDATE);
      break;
      
    case VTKIS_USCALE:
      if (this->ActorMode && this->ActorPicked)
        {
        if (this->TrackballMode)
          {
          this->TrackballScaleActor(this->LastPos[0],
                                    this->LastPos[1]);
          }
        else
          {
          this->JoystickScaleActor(this->LastPos[0],
                                   this->LastPos[1]);
          }
        }
      rwi->CreateTimer(VTKI_TIMER_UPDATE);
      break;
      
    case VTKIS_TIMER:
      rwi->CreateTimer(VTKI_TIMER_UPDATE);
      break;

    default :
      break;
    }
}

void vtkInteractorStyleTrackball::ActorTransform(vtkActor *actor,
                                                float *boxCenter,
                                                int numRotation,
                                                double **rotate,
                                                double *scale)
{
  double boxCenter2[3];
  boxCenter2[0] = boxCenter[0];
  boxCenter2[1] = boxCenter[1];
  boxCenter2[2] = boxCenter[2];
  this->ActorTransform(actor,boxCenter2,numRotation,rotate,scale);
}

void vtkInteractorStyleTrackball::FindPickedActor(int X, int Y)
{
  this->InteractionPicker->Pick(X,Y, 0.0, this->CurrentRenderer);
  
  // now go through the actor collection and decide which is closest
  vtkActor *closestActor = NULL, *actor;
  vtkActorCollection *actors = this->InteractionPicker->GetActors();
  vtkPoints *pickPositions = this->InteractionPicker->GetPickedPositions();
  int i = 0;
  float *pickPoint, d;
  float distToCamera = VTK_LARGE_FLOAT;
  if (actors && actors->GetNumberOfItems() > 0)
    {
    actors->InitTraversal();
    this->CurrentCamera->GetPosition(this->ViewPoint);
    while (i < pickPositions->GetNumberOfPoints())
      {
      actor = actors->GetNextItem();
      if (actor != NULL)
        {
        pickPoint = pickPositions->GetPoint(i);
        double dtmp[3];
        dtmp[0] = pickPoint[0];
        dtmp[1] = pickPoint[1];
        dtmp[2] = pickPoint[2];
        d = vtkMath::Distance2BetweenPoints(dtmp, this->ViewPoint);
        if (distToCamera > d)
          {
          distToCamera = d;
          closestActor = actor;
          }
        }
      i++;
      }
    }
  
  this->InteractionActor = closestActor;
  // refine the answer to whether an actor was picked.  CellPicker()
  // returns true from Pick() if the bounding box was picked,
  // but we only want something to be picked if a cell was actually
  // selected
  this->ActorPicked = (this->InteractionActor != NULL);
}

//----------------------------------------------------------------------------
void vtkInteractorStyleTrackball::OnLeftButtonDown(int ctrl, int shift, 
                                          int X, int Y) 
{
  //
  this->OldX = X;
  this->OldY = Y;
  this->UpdateInternalState(ctrl, shift, X, Y);

  vtkRenderWindowInteractor *rwi = this->Interactor;
  this->FindPokedCamera(X, Y);
  this->Preprocess = 1;
  if (this->LeftButtonPressMethod) 
    {
    (*this->LeftButtonPressMethod)(this->LeftButtonPressMethodArg);
    }
  else 
    {
    if (this->ActorMode)
      {
      this->FindPickedActor(X,Y);    
      }
    if (this->ShiftKey) 
      { // I haven't got a Middle button !
      if (this->CtrlKey) 
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
      if (this->CtrlKey) this->StartSpin();
      else         this->StartRotate();
      }
    }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleTrackball::OnLeftButtonUp(int ctrl, int shift, int X, int Y) 
{
  //
 this->UpdateInternalState(ctrl, shift, X, Y);
  //
  if (this->LeftButtonReleaseMethod) 
    {
    (*this->LeftButtonReleaseMethod)(this->LeftButtonReleaseMethodArg);
    }
  else 
    {
    if (this->ShiftKey) 
      {
      if (this->CtrlKey) this->EndDolly();
      else         this->EndPan();
      } 
    else 
      {
      if (this->CtrlKey) this->EndSpin();
      else               this->EndRotate();
      }
    }
  this->OldX = 0.0;
  this->OldY = 0.0;
  if (this->ActorMode && this->ActorPicked)
    {
    this->HighlightActor(this->InteractionActor);
    }
  else if (this->ActorMode)
    {
    this->HighlightActor(NULL);
    }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleTrackball::OnMiddleButtonDown(int ctrl, int shift, 
                                            int X, int Y) 
{
  this->OldX = X;
  this->OldY = Y;
  //
  this->UpdateInternalState(ctrl, shift, X, Y);
  //
  this->Preprocess = 1;
  this->FindPokedCamera(X, Y);
  //
  if (this->MiddleButtonPressMethod) 
    {
    (*this->MiddleButtonPressMethod)(this->MiddleButtonPressMethodArg);
    }
  else 
    {
    if (this->ActorMode)
      {
      this->FindPickedActor(X,Y);    
      }
    if (this->CtrlKey) 
      {
      this->StartDolly();
      }
    else
      {
      this->StartPan();
      }
    }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleTrackball::OnMiddleButtonUp(int ctrl, int shift, 
                                                   int X, int Y) 
{
  //
 this->UpdateInternalState(ctrl, shift, X, Y);
  //
  if (this->MiddleButtonReleaseMethod) 
    {
    (*this->MiddleButtonReleaseMethod)(this->MiddleButtonReleaseMethodArg);
    }
  else 
    {
    if (this->CtrlKey) this->EndDolly();
    else         this->EndPan();
    }
  this->OldX = 0.0;
  this->OldY = 0.0;
  if (this->ActorMode && this->ActorPicked)
    {
    this->HighlightActor(this->InteractionActor);
    }
  else if (this->ActorMode)
    {
    this->HighlightActor(NULL);
    }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleTrackball::OnRightButtonDown(int ctrl, int shift, 
                                                    int X, int Y) 
{
  this->OldX = X;
  this->OldY = Y;
  //
  this->UpdateInternalState(ctrl, shift, X, Y);
  this->FindPokedCamera(X, Y);
  this->Preprocess = 1;
  if (this->RightButtonPressMethod) 
    {
    (*this->RightButtonPressMethod)(this->RightButtonPressMethodArg);
    }
  else 
    {
    if (this->ActorMode)
      {
      this->FindPickedActor(X,Y);    
      this->StartUniformScale();
      }
    else
      {
      this->StartZoom();
      }
    }
}
//----------------------------------------------------------------------------
void vtkInteractorStyleTrackball::OnRightButtonUp(int ctrl, int shift, 
                                                  int X, int Y) 
{
  //
 this->UpdateInternalState(ctrl, shift, X, Y);
  //
  if (this->RightButtonReleaseMethod) 
    {
    (*this->RightButtonReleaseMethod)(this->RightButtonReleaseMethodArg);
    }
  else 
    {
    if (this->ActorMode)
      {
      this->EndUniformScale();
      }
    else
      {
      this->EndZoom();
      }
    }
  this->OldX = 0.0;
  this->OldY = 0.0;
  if (this->ActorMode && this->ActorPicked)
    {
    this->HighlightActor(this->InteractionActor);
    }
  else if (this->ActorMode)
    {
    this->HighlightActor(NULL);
    }
}

//----------------------------------------------------------------------------
void vtkInteractorStyleTrackball::SetTrackballModeToTrackball()
{
  if (this->TrackballMode == VTKIS_TRACK)
    {
    return;
    }
  this->TrackballMode = VTKIS_TRACK;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleTrackball::SetTrackballModeToJoystick()
{
  if (this->TrackballMode == VTKIS_JOY)
    {
    return;
    }
  this->TrackballMode = VTKIS_JOY;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleTrackball::SetActorModeToCamera()
{
  if (this->ActorMode == VTKIS_CAMERA)
    {
    return;
    }
  this->ActorMode = VTKIS_CAMERA;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkInteractorStyleTrackball::SetActorModeToActor()
{
  if (this->ActorMode == VTKIS_ACTOR)
    {
    return;
    }
  this->ActorMode = VTKIS_ACTOR;
  this->Modified();

}
