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
#include "vtkImageViewer.h"
#include "vtkObjectFactory.h"
#include "vtkInteractorStyleImage.h"

vtkCxxRevisionMacro(vtkImageViewer, "$Revision$");
vtkStandardNewMacro(vtkImageViewer);

//----------------------------------------------------------------------------
vtkImageViewer::vtkImageViewer()
{
  this->RenderWindow = vtkRenderWindow::New();
  this->Renderer = vtkRenderer::New();
  
  this->ImageMapper = vtkImageMapper::New();
  this->Actor2D     = vtkActor2D::New();

  // setup the pipeline
  this->Actor2D->SetMapper(this->ImageMapper);
  this->Renderer->AddActor2D(this->Actor2D);
  this->RenderWindow->AddRenderer(this->Renderer);

  this->FirstRender = 1;
  
  this->Interactor = 0;
  this->InteractorStyle = 0;
}


//----------------------------------------------------------------------------
vtkImageViewer::~vtkImageViewer()
{
  this->ImageMapper->Delete();
  this->Actor2D->Delete();
  this->RenderWindow->Delete();
  this->Renderer->Delete();

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
void vtkImageViewer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << *this->ImageMapper << endl;
  os << indent << *this->RenderWindow << endl;
  os << indent << *this->Renderer << endl;
  os << indent << *this->Actor2D << endl;
}



//----------------------------------------------------------------------------
void vtkImageViewer::SetSize(int a[2])
{
  this->SetSize(a[0],a[1]);
}
//----------------------------------------------------------------------------
void vtkImageViewer::SetPosition(int a[2])
{
  this->SetPosition(a[0],a[1]);
}


class vtkImageViewerCallback : public vtkCommand
{
public:
  static vtkImageViewerCallback *New() {
    return new vtkImageViewerCallback; }
  
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
        float *range = this->IV->GetInput()->GetScalarRange();
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
      float window = this->InitialWindow;
      float level = this->InitialLevel;
      
      // Compute normalized delta

      float dx = 4.0 * (isi->GetWindowLevelCurrentPosition()[0] - 
                        isi->GetWindowLevelStartPosition()[0]) / size[0];
      float dy = 4.0 * (isi->GetWindowLevelStartPosition()[1] - 
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

      float newWindow = dx + window;
      float newLevel;
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
  
  vtkImageViewer *IV;
  float InitialWindow;
  float InitialLevel;
};


void vtkImageViewer::SetupInteractor(vtkRenderWindowInteractor *rwi)
{
  if (this->Interactor && rwi != this->Interactor)
    {
    this->Interactor->Delete();
    }
  if (!this->InteractorStyle)
    {
    this->InteractorStyle = vtkInteractorStyleImage::New();
    vtkImageViewerCallback *cbk = vtkImageViewerCallback::New();
    cbk->IV = this;
    this->InteractorStyle->AddObserver(vtkCommand::WindowLevelEvent, cbk);
    this->InteractorStyle->AddObserver(vtkCommand::StartWindowLevelEvent, cbk);
    this->InteractorStyle->AddObserver(vtkCommand::ResetWindowLevelEvent, cbk);
    cbk->Delete();
    }
  
  if (!this->Interactor)
    {
    this->Interactor = rwi;
    rwi->Register(this);
    }
  this->Interactor->SetInteractorStyle(this->InteractorStyle);
  this->Interactor->SetRenderWindow(this->RenderWindow);
}

void vtkImageViewer::Render()
{
  if (this->FirstRender)
    {
    // initialize the size if not set yet
    if (this->RenderWindow->GetSize()[0] == 0 && this->ImageMapper->GetInput())
      {
      // get the size from the mappers input
      this->ImageMapper->GetInput()->UpdateInformation();
      int *ext = this->ImageMapper->GetInput()->GetWholeExtent();
      // if it would be smaller than 100 by 100 then limit to 100 by 100
      int xs = ext[1] - ext[0] + 1;
      int ys = ext[3] - ext[2] + 1;
      this->RenderWindow->SetSize(xs < 150 ? 150 : xs,
                                  ys < 100 ? 100 : ys);
      }
    this->FirstRender = 0;  
    }

  this->RenderWindow->Render();
}
