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
#include "vtkImageViewer2.h"

#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkImageData.h"
#include "vtkImageMapToWindowLevelColors.h"
#include "vtkInteractorStyleImage.h"
#include "vtkObjectFactory.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"

vtkCxxRevisionMacro(vtkImageViewer2, "$Revision$");
vtkStandardNewMacro(vtkImageViewer2);

//----------------------------------------------------------------------------
// Construct the vtkImageViewer2 with AutoResetCameraClippingRange On by default
vtkImageViewer2::vtkImageViewer2()
{
  this->RenderWindow = vtkRenderWindow::New();
  this->Renderer     = vtkRenderer::New();
  this->ImageActor   = vtkImageActor::New();
  this->WindowLevel  = vtkImageMapToWindowLevelColors::New();
  
  // setup the pipeline
  this->ImageActor->SetInput(this->WindowLevel->GetOutput());
  this->Renderer->AddProp(this->ImageActor);
  this->RenderWindow->AddRenderer(this->Renderer);
  this->FirstRender = 1;
  
  this->Interactor = 0;
  this->InteractorStyle = 0;
  this->AutoResetCameraClippingRange = 1;
}


//----------------------------------------------------------------------------
vtkImageViewer2::~vtkImageViewer2()
{
  this->WindowLevel->Delete();
  this->ImageActor->Delete();
  this->Renderer->Delete();
  this->RenderWindow->Delete();

  if (this->Interactor)
    {
    this->Interactor->Delete();
    }
  if (this->InteractorStyle)
    {
    this->InteractorStyle->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkImageViewer2::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << *this->RenderWindow << endl;
  os << indent << *this->Renderer << endl;
  os << indent << *this->ImageActor << endl;
  os << indent << *this->WindowLevel << endl;
}



//----------------------------------------------------------------------------
void vtkImageViewer2::SetSize(int a[2])
{
  this->SetSize(a[0],a[1]);
}
//----------------------------------------------------------------------------
void vtkImageViewer2::SetPosition(int a[2])
{
  this->SetPosition(a[0],a[1]);
}

//----------------------------------------------------------------------------
class vtkImageViewer2Callback : public vtkCommand
{
public:
  static vtkImageViewer2Callback *New() { return new vtkImageViewer2Callback; }
  
  void Execute(vtkObject *caller, 
               unsigned long event, 
               void *vtkNotUsed(callData))
    {
      if (this->IV->GetInput() == NULL)
        {
        return;
        }

      // Reset

      if (event == vtkCommand::ResetWindowLevelEvent)
        {
        this->IV->GetInput()->UpdateInformation();
        this->IV->GetInput()->SetUpdateExtent
          (this->IV->GetInput()->GetWholeExtent());
        this->IV->GetInput()->Update();
        double *range = this->IV->GetInput()->GetScalarRange();
        this->IV->SetColorWindow(range[1] - range[0]);
        this->IV->SetColorLevel(0.5 * (range[1] + range[0]));
        this->IV->Render();
        return;
        }

      // Start

      if (event == vtkCommand::StartWindowLevelEvent)
        {
        this->InitialWindow = this->IV->GetColorWindow();
        this->InitialLevel = this->IV->GetColorLevel();
        return;
        }
      
      // Adjust the window level here

      vtkInteractorStyleImage *isi = 
        static_cast<vtkInteractorStyleImage *>(caller);

      int *size = this->IV->GetRenderWindow()->GetSize();
      double window = this->InitialWindow;
      double level = this->InitialLevel;
      
      // Compute normalized delta

      double dx = 4.0 * 
        (isi->GetWindowLevelCurrentPosition()[0] - 
         isi->GetWindowLevelStartPosition()[0]) / size[0];
      double dy = 4.0 * 
        (isi->GetWindowLevelStartPosition()[1] - 
         isi->GetWindowLevelCurrentPosition()[1]) / size[1];
      
      // Scale by current values

      if (fabs(window) > 0.01)
        {
        dx = dx * window;
        }
      else
        {
        dx = dx * (window < 0 ? -0.01 : 0.01);
        }
      if (fabs(level) > 0.01)
        {
        dy = dy * level;
        }
      else
        {
        dy = dy * (level < 0 ? -0.01 : 0.01);
        }
      
      // Abs so that direction does not flip

      if (window < 0.0) 
        {
        dx = -1*dx;
        }
      if (level < 0.0) 
        {
        dy = -1*dy;
        }
      
      // Compute new window level

      double newWindow = dx + window;
      double newLevel;
      newLevel = level - dy;
      
      // Stay away from zero and really

      if (fabs(newWindow) < 0.01)
        {
        newWindow = 0.01*(newWindow < 0 ? -1 : 1);
        }
      if (fabs(newLevel) < 0.01)
        {
        newLevel = 0.01*(newLevel < 0 ? -1 : 1);
        }
      
      this->IV->SetColorWindow(newWindow);
      this->IV->SetColorLevel(newLevel);
      this->IV->Render();
    }
  
  vtkImageViewer2 *IV;
  double InitialWindow;
  double InitialLevel;
};

//----------------------------------------------------------------------------
void vtkImageViewer2::SetupInteractor(vtkRenderWindowInteractor *rwi)
{
  if (this->Interactor && rwi != this->Interactor)
    {
    this->Interactor->Delete();
    }
  if (!this->InteractorStyle)
    {
    this->InteractorStyle = vtkInteractorStyleImage::New();
    vtkImageViewer2Callback *cbk = vtkImageViewer2Callback::New();
    cbk->IV = this;
    this->InteractorStyle->AddObserver(
      vtkCommand::WindowLevelEvent, cbk);
    this->InteractorStyle->AddObserver(
      vtkCommand::StartWindowLevelEvent, cbk);
    this->InteractorStyle->AddObserver(
      vtkCommand::ResetWindowLevelEvent, cbk);
    cbk->Delete();
    }
  
  if (!this->Interactor)
    {
    this->Interactor = rwi;
    rwi->Register(this);
    }
  this->Interactor->SetInteractorStyle(this->InteractorStyle);
  this->Interactor->SetRenderWindow(this->RenderWindow);
  this->Renderer->GetActiveCamera()->ParallelProjectionOn();
}

//----------------------------------------------------------------------------
void vtkImageViewer2::Render()
{
  if (this->FirstRender)
    {
    // initialize the size if not set yet
    if (this->RenderWindow->GetSize()[0] == 0 && 
        this->ImageActor->GetInput())
      {
      // get the size from the mappers input
      this->WindowLevel->GetInput()->UpdateInformation();
      int *ext = this->WindowLevel->GetInput()->GetWholeExtent();
      // if it would be smaller than 100 by 100 then limit to 100 by 100
      int xs = ext[1] - ext[0] + 1;
      int ys = ext[3] - ext[2] + 1;
      this->RenderWindow->SetSize(xs < 150 ? 150 : xs,
                                  ys < 100 ? 100 : ys);
      this->Renderer->GetActiveCamera()->SetParallelScale(xs < 150 ? 75 
                                                          : (xs-1)/2.0);
      }
    this->FirstRender = 0;  
    }
  this->RenderWindow->Render();
}


//----------------------------------------------------------------------------
void vtkImageViewer2::SetOffScreenRendering(int i)
{
  this->RenderWindow->SetOffScreenRendering(i);
}

//----------------------------------------------------------------------------
int vtkImageViewer2::GetOffScreenRendering()
{
  return this->RenderWindow->GetOffScreenRendering();
}

//----------------------------------------------------------------------------
void vtkImageViewer2::OffScreenRenderingOn()
{
  this->SetOffScreenRendering(1);
}

//----------------------------------------------------------------------------
void vtkImageViewer2::OffScreenRenderingOff()
{
  this->SetOffScreenRendering(0);
}

//----------------------------------------------------------------------------
void vtkImageViewer2::SetZSlice(int s)
{
  this->ImageActor->SetZSlice(s);
  if(this->AutoResetCameraClippingRange)
    {
    this->Renderer->ResetCameraClippingRange();
    }
}
