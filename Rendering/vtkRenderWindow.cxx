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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkMath.h"

// Description:
// Construct an instance of  vtkRenderWindow with its screen size 
// set to 300x300, borders turned on, positioned at (0,0), double 
// buffering turned on.
vtkRenderWindow::vtkRenderWindow()
{
  this->Borders = 1;
  this->FullScreen = 0;
  this->OldScreen[0] = this->OldScreen[1] = 0;
  this->OldScreen[2] = this->OldScreen[3] = 300;
  this->OldScreen[4] = 1;
  this->DoubleBuffer = 1;
  this->StereoRender = 0;
  this->StereoType = VTK_STEREO_RED_BLUE;
  this->StereoStatus = 0;
  this->Interactor = NULL;
  this->AAFrames = 0;
  this->FDFrames = 0;
  this->SubFrames = 0;
  this->AccumulationBuffer = NULL;
  this->CurrentSubFrame = 0;
  this->DesiredUpdateRate = 0.0001;
  this->ResultFrame = NULL;
  this->FileName = NULL;
  this->SwapBuffers = 1;
  this->PPMImageFilePtr = NULL;
  this->AbortRender = 0;
  this->InAbortCheck = 0;
  this->AbortCheckMethod = NULL;
  this->AbortCheckMethodArg = NULL;
  this->AbortCheckMethodArgDelete = NULL;
}

vtkRenderWindow::~vtkRenderWindow()
{
  if (this->AccumulationBuffer) 
    {
    delete [] this->AccumulationBuffer;
    this->AccumulationBuffer = NULL;
    }
  if (this->ResultFrame) 
    {
    delete [] this->ResultFrame;
    this->ResultFrame = NULL;
    }
}

// Description:
// Specify a function to be called to check and see if an abort
// of the rendering in progress is desired.
void vtkRenderWindow::SetAbortCheckMethod(void (*f)(void *), void *arg)
{
  if ( f != this->AbortCheckMethod || arg != this->AbortCheckMethodArg )
    {
    // delete the current arg if there is one and a delete meth
    if ((this->AbortCheckMethodArg)&&(this->AbortCheckMethodArgDelete))
      {
      (*this->AbortCheckMethodArgDelete)(this->AbortCheckMethodArg);
      }
    this->AbortCheckMethod = f;
    this->AbortCheckMethodArg = arg;
    this->Modified();
    }
}

// Description:
// Set the arg delete method. This is used to free user memory.
void vtkRenderWindow::SetAbortCheckMethodArgDelete(void (*f)(void *))
{
  if ( f != this->AbortCheckMethodArgDelete)
    {
    this->AbortCheckMethodArgDelete = f;
    this->Modified();
    }
}

char *vtkRenderWindow::GetRenderLibrary()
{
  char *temp;
  
  // first check the environment variable
  temp = getenv("VTK_RENDERER");

  // Backward compatibility
  if ( temp )
    {
    if (!strcmp("sbr",temp)) temp = "Starbase";
    else if (!strcmp("glr",temp)) temp = "GL";
    else if (!strcmp("oglr",temp)) temp = "OpenGL";
    else if (!strcmp("woglr",temp)) temp = "Win32OpenGL";
    else if (!strcmp("xglr",temp)) temp = "XGL";
    else if ( strcmp("Starbase",temp) && strcmp("GL",temp) && 
      strcmp("OpenGL",temp) && strcmp("Win32OpenGL",temp) && strcmp("XGL",temp) )
      {
      vtkGenericWarningMacro(<<"VTK_RENDERER set to unsupported type:" << temp);
      temp = NULL;
      }
    }

  // if nothing is set then work down the list of possible renderers
  if ( !temp )
    {
#ifdef VTK_USE_GLR
    temp = "GL";
#endif
#ifdef VTK_USE_OGLR
    temp = "OpenGL";
#endif
#ifdef VTK_USE_SBR
    temp = "Starbase";
#endif
#ifdef VTK_USE_XGLR
    temp = "XGL";
#endif
#ifdef _WIN32
    temp = "Win32OpenGL";
#endif
    }

  return temp;
}

#ifdef VTK_USE_GLR
#include "vtkGLRenderWindow.h"
#endif
#ifdef VTK_USE_OGLR
#include "vtkOpenGLRenderWindow.h"
#endif
#ifdef VTK_USE_SBR
#include "vtkStarbaseRenderWindow.h"
#endif
#ifdef VTK_USE_XGLR
#include "vtkXGLRenderWindow.h"
#endif
#ifdef _WIN32
#include "vtkWin32OpenGLRenderWindow.h"
#endif
// return the correct type of RenderWindow 
vtkRenderWindow *vtkRenderWindow::New()
{
  char *temp = vtkRenderWindow::GetRenderLibrary();
  
#ifdef VTK_USE_SBR
  if (!strcmp("Starbase",temp)) return vtkStarbaseRenderWindow::New();
#endif
#ifdef VTK_USE_GLR
  if (!strcmp("GL",temp)) return vtkGLRenderWindow::New();
#endif
#ifdef VTK_USE_OGLR
  if (!strcmp("OpenGL",temp)) return vtkOpenGLRenderWindow::New();
#endif
#ifdef _WIN32
  if (!strcmp("Win32OpenGL",temp)) return vtkWin32OpenGLRenderWindow::New();
#endif
#ifdef VTK_USE_XGLR
  if (!strcmp("XGL",temp)) return vtkXGLRenderWindow::New();
#endif
  
  return new vtkRenderWindow;
}

vtkRenderer *vtkRenderWindow::MakeRenderer()
{
  vtkRenderer *ren = vtkRenderer::New();
  this->AddRenderer(ren);

  // by default we are its parent
  ren->SetRenderWindow(this);
  return ren;
}

// Description:
// Create an interactor that will work with this renderer.
vtkRenderWindowInteractor *vtkRenderWindow::MakeRenderWindowInteractor()
{
  this->Interactor = vtkRenderWindowInteractor::New();
  this->Interactor->SetRenderWindow(this);
  return this->Interactor;
}

// Description:
// Set the interactor that will work with this renderer.
void vtkRenderWindow::SetInteractor(vtkRenderWindowInteractor *rwi)
{
  this->Interactor = rwi;
  if (this->Interactor->GetRenderWindow() != this)
    {
    this->Interactor->SetRenderWindow(this);
    }
}

void vtkRenderWindow::SetDesiredUpdateRate(float rate)
{
  vtkRenderer *aren;

  if (this->DesiredUpdateRate != rate)
    {
    for (this->Renderers.InitTraversal(); 
	 (aren = this->Renderers.GetNextItem()); )
      {
      aren->SetAllocatedRenderTime(1.0/
				   (rate*this->Renderers.GetNumberOfItems()));
      }
    this->DesiredUpdateRate = rate;
    this->Modified();
    }
}


// Description:
// Ask each renderer owned by this RenderWindow to render its image and 
// synchronize this process.
void vtkRenderWindow::Render()
{
  int *size;
  int x,y;
  float *p1;

  vtkDebugMacro(<< "Starting Render Method.\n");

  // if we are in the middle of an abort check the return now
  if (this->InAbortCheck) return;
  // reset the Abort flag
  this->AbortRender = 0;
  
  if ( this->Interactor && ! this->Interactor->GetInitialized() )
    this->Interactor->Initialize();

  if ((!this->AccumulationBuffer)&&
      (this->SubFrames || this->AAFrames || this->FDFrames))
    {
    // get the size
    size = this->GetSize();

    this->AccumulationBuffer = new float [3*size[0]*size[1]];
    memset(this->AccumulationBuffer,0,3*size[0]*size[1]*sizeof(float));
    }
  
  // handle any sub frames
  if (this->SubFrames)
    {
    // get the size
    size = this->GetSize();

    // draw the images
    this->DoAARender();

    // now accumulate the images 
    if ((!this->AAFrames) && (!this->FDFrames))
      {
      p1 = this->AccumulationBuffer;
      unsigned char *p2;
      unsigned char *p3;
      if (this->ResultFrame)
	{
	p2 = this->ResultFrame;
	}
      else
	{
	p2 = this->GetPixelData(0,0,size[0]-1,size[1]-1,0);
	}
      p3 = p2;
      for (y = 0; y < size[1]; y++)
	{
	for (x = 0; x < size[0]; x++)
	  {
	  *p1 += *p2; p1++; p2++;
	  *p1 += *p2; p1++; p2++;
	  *p1 += *p2; p1++; p2++;
	  }
	}
      delete [] p3;
      }
    
    // if this is the last sub frame then convert back into unsigned char
    this->CurrentSubFrame++;
    if (this->CurrentSubFrame == this->SubFrames)
      {
      float num;
      unsigned char *p2 = new unsigned char [3*size[0]*size[1]];
      
      num = this->SubFrames;
      if (this->AAFrames) num *= this->AAFrames;
      if (this->FDFrames) num *= this->FDFrames;

      this->ResultFrame = p2;
      p1 = this->AccumulationBuffer;
      for (y = 0; y < size[1]; y++)
	{
	for (x = 0; x < size[0]; x++)
	  {
	  *p2 = (unsigned char)(*p1/num); p1++; p2++;
	  *p2 = (unsigned char)(*p1/num); p1++; p2++;
	  *p2 = (unsigned char)(*p1/num); p1++; p2++;
	  }
	}
      
      this->CurrentSubFrame = 0;
      this->CopyResultFrame();

      //###
      this->Renderers.Render2D();

      // free any memory
      delete [] this->AccumulationBuffer;
      this->AccumulationBuffer = NULL;
      }
    }
  else // no subframes
    {
    // get the size
    size = this->GetSize();

    this->DoAARender();
    // if we had some accumulation occur
    if (this->AccumulationBuffer)
      {
      float num;
      unsigned char *p2 = new unsigned char [3*size[0]*size[1]];

      if (this->AAFrames) 
	{
	num = this->AAFrames;
	}
      else
	{
	num = 1;
	}
      if (this->FDFrames) num *= this->FDFrames;

      this->ResultFrame = p2;
      p1 = this->AccumulationBuffer;
      for (y = 0; y < size[1]; y++)
	{
	for (x = 0; x < size[0]; x++)
	  {
	  *p2 = (unsigned char)(*p1/num); p1++; p2++;
	  *p2 = (unsigned char)(*p1/num); p1++; p2++;
	  *p2 = (unsigned char)(*p1/num); p1++; p2++;
	  }
	}
      
      delete [] this->AccumulationBuffer;
      this->AccumulationBuffer = NULL;
      }
    
    this->CopyResultFrame();
    //###
    this->Renderers.Render2D();
    }  

  if (this->ResultFrame) 
    {
    delete [] this->ResultFrame;
    this->ResultFrame = NULL;
    }

}

// Description:
// Handle rendering any antialiased frames.
void vtkRenderWindow::DoAARender()
{
  int i;
  
  // handle any anti aliasing
  if (this->AAFrames)
    {
    int *size;
    int x,y;
    float *p1;
    vtkRenderer *aren;
    vtkCamera *acam;
    float *dpoint;
    float offsets[2];
    float origfocus[4];
    float worldOffset[3];

    // get the size
    size = this->GetSize();

    origfocus[3] = 1.0;

    for (i = 0; i < AAFrames; i++)
      {
      // jitter the cameras
      offsets[0] = vtkMath::Random() - 0.5;
      offsets[1] = vtkMath::Random() - 0.5;

      for (this->Renderers.InitTraversal(); 
	   (aren = this->Renderers.GetNextItem()); )
	{
	acam = aren->GetActiveCamera();

	// calculate the amount to jitter
	memcpy(origfocus,acam->GetFocalPoint(),12);
	aren->SetWorldPoint(origfocus);
	aren->WorldToDisplay();
	dpoint = aren->GetDisplayPoint();
	aren->SetDisplayPoint(dpoint[0] + offsets[0],
			      dpoint[1] + offsets[1],
			      dpoint[2]);
	aren->DisplayToWorld();
	dpoint = aren->GetWorldPoint();
	dpoint[0] /= dpoint[3];
	dpoint[1] /= dpoint[3];
	dpoint[2] /= dpoint[3];
	acam->SetFocalPoint(dpoint);

	worldOffset[0] = dpoint[0] - origfocus[0];
	worldOffset[1] = dpoint[1] - origfocus[1];
	worldOffset[2] = dpoint[2] - origfocus[2];

	dpoint = acam->GetPosition();
	acam->SetPosition(dpoint[0]+worldOffset[0],
			  dpoint[1]+worldOffset[1],
			  dpoint[2]+worldOffset[2]);
	}

      // draw the images
      this->DoFDRender();

      // restore the jitter to normal
      for (this->Renderers.InitTraversal(); 
	   (aren = this->Renderers.GetNextItem()); )
	{
	acam = aren->GetActiveCamera();

	// calculate the amount to jitter
	memcpy(origfocus,acam->GetFocalPoint(),12);
	aren->SetWorldPoint(origfocus);
	aren->WorldToDisplay();
	dpoint = aren->GetDisplayPoint();
	aren->SetDisplayPoint(dpoint[0] - offsets[0],
			      dpoint[1] - offsets[1],
			      dpoint[2]);
	aren->DisplayToWorld();
	dpoint = aren->GetWorldPoint();
	dpoint[0] /= dpoint[3];
	dpoint[1] /= dpoint[3];
	dpoint[2] /= dpoint[3];
	acam->SetFocalPoint(dpoint);

	worldOffset[0] = dpoint[0] - origfocus[0];
	worldOffset[1] = dpoint[1] - origfocus[1];
	worldOffset[2] = dpoint[2] - origfocus[2];

	dpoint = acam->GetPosition();
	acam->SetPosition(dpoint[0]+worldOffset[0],
			  dpoint[1]+worldOffset[1],
			  dpoint[2]+worldOffset[2]);
	}


      // now accumulate the images 
      p1 = this->AccumulationBuffer;
      if (!this->FDFrames)
	{
	unsigned char *p2;
	unsigned char *p3;
	if (this->ResultFrame)
	  {
	  p2 = this->ResultFrame;
	  }
	else
	  {
	  p2 = this->GetPixelData(0,0,size[0]-1,size[1]-1,0);
	  }
	p3 = p2;
	for (y = 0; y < size[1]; y++)
	  {
	  for (x = 0; x < size[0]; x++)
	    {
	    *p1 += (float)*p2; p1++; p2++;
	    *p1 += (float)*p2; p1++; p2++;
	    *p1 += (float)*p2; p1++; p2++;
	    }
	  }
	delete [] p3;
	}
      }
    }
  else
    {
    this->DoFDRender();
    }
}


// Description:
// Handle rendering any focal depth frames.
void vtkRenderWindow::DoFDRender()
{
  int i;
  
  // handle any focal depth
  if (this->FDFrames)
    {
    int *size;
    int x,y;
    unsigned char *p2;
    unsigned char *p3;
    float *p1;
    vtkRenderer *aren;
    vtkCamera *acam;
    float focalDisk;
    float viewUp[4];
    float *vpn;
    float *dpoint;
    vtkTransform aTrans;
    float offsets[2];
    float *orig;

    // get the size
    size = this->GetSize();

    viewUp[3] = 1.0;

    orig = new float [3*this->Renderers.GetNumberOfItems()];

    for (i = 0; i < FDFrames; i++)
      {
      int j = 0;

      offsets[0] = vtkMath::Random(); // radius
      offsets[1] = vtkMath::Random()*360.0; // angle

      // store offsets for each renderer 
      for (this->Renderers.InitTraversal(); 
	   (aren = this->Renderers.GetNextItem()); )
	{
	acam = aren->GetActiveCamera();
	focalDisk = acam->GetFocalDisk()*offsets[0];

	memcpy(viewUp,acam->GetViewUp(),12);
	vpn = acam->GetViewPlaneNormal();
	aTrans.Identity();
	aTrans.Scale(focalDisk,focalDisk,focalDisk);
	aTrans.RotateWXYZ(offsets[1],vpn[0],vpn[1],vpn[2]);
	aTrans.SetPoint(viewUp);
	vpn = aTrans.GetPoint();
	dpoint = acam->GetPosition();

	// store the position for later
	memcpy(orig + j*3,dpoint,12);
	j++;

	acam->SetPosition(dpoint[0]+vpn[0],
			  dpoint[1]+vpn[1],
			  dpoint[2]+vpn[2]);
	}

      // draw the images
      this->DoStereoRender();

      // restore the jitter to normal
      j = 0;
      for (this->Renderers.InitTraversal(); 
	   (aren = this->Renderers.GetNextItem()); )
	{
	acam = aren->GetActiveCamera();
	acam->SetPosition(orig + j*3);
	j++;
	}

      // get the pixels for accumulation
      // now accumulate the images 
      p1 = this->AccumulationBuffer;
      if (this->ResultFrame)
	{
	p2 = this->ResultFrame;
	}
      else
	{
	p2 = this->GetPixelData(0,0,size[0]-1,size[1]-1,0);
	}
      p3 = p2;
      for (y = 0; y < size[1]; y++)
	{
	for (x = 0; x < size[0]; x++)
	  {
	  *p1 += (float)*p2; p1++; p2++;
	  *p1 += (float)*p2; p1++; p2++;
	  *p1 += (float)*p2; p1++; p2++;
	  }
	}
      delete [] p3;
      }
  
    // free memory
    delete [] orig;
    }
  else
    {
    this->DoStereoRender();
    }
}


// Description:
// Handle rendering the two different views for stereo rendering.
void vtkRenderWindow::DoStereoRender()
{
  this->Start();
  this->StereoUpdate();
  if (this->StereoType != VTK_STEREO_RIGHT)
    { // render the left eye
    this->Renderers.Render();
    }

  if (this->StereoRender)
    {
    this->StereoMidpoint();
    if (this->StereoType != VTK_STEREO_LEFT)
      { // render the right eye
      this->Renderers.Render();
      }
    this->StereoRenderComplete();
    }
}

// Description:
// Add a renderer to the list of renderers.
void vtkRenderWindow::AddRenderer(vtkRenderer *ren)
{
  // we are its parent 
  ren->SetRenderWindow(this);
  this->Renderers.AddItem(ren);
}

// Description:
// Remove a renderer from the list of renderers.
void vtkRenderWindow::RemoveRenderer(vtkRenderer *ren)
{
  // we are its parent 
  this->Renderers.RemoveItem(ren);
}

int vtkRenderWindow::CheckAbortStatus()
{
  if (!this->InAbortCheck)
    {
    this->InAbortCheck = 1;
    if (this->AbortCheckMethod) 
      {
      (*this->AbortCheckMethod)(this->AbortCheckMethodArg);
      }
    this->InAbortCheck = 0;
    }
  return this->AbortRender;
}

void vtkRenderWindow::PrintSelf(ostream& os, vtkIndent indent)
{
  int *temp;

  vtkWindow::PrintSelf(os,indent);

  os << indent << "Borders: " << (this->Borders ? "On\n":"Off\n");
  os << indent << "Double Buffer: " << (this->DoubleBuffer ? "On\n":"Off\n");
  os << indent << "Full Screen: " << (this->FullScreen ? "On\n":"Off\n");
  os << indent << "Renderers:\n";
  this->Renderers.PrintSelf(os,indent.GetNextIndent());
  os << indent << "Stereo Render: " 
     << (this->StereoRender ? "On\n":"Off\n");

  os << indent << "File Name: " 
     << (this->FileName ? this->FileName : "(none)") << "\n";

  if ( this->AbortCheckMethod )
    {
    os << indent << "AbortCheck method defined.\n";
    }
  else
    {
    os << indent << "No AbortCheck method.\n";
    }
}


void vtkRenderWindow::SaveImageAsPPM()
{
  if( this->OpenPPMImageFile() )
    {
    this->WritePPMImageFile();
    this->ClosePPMImageFile();
    }
}


int vtkRenderWindow::OpenPPMImageFile()
{
  //  open the ppm file and write header 
  if ( this->FileName != NULL && *this->FileName != '\0')
    {
    this->PPMImageFilePtr = fopen(this->FileName,"wb");
    if (!this->PPMImageFilePtr)
      {
      vtkErrorMacro(<< "RenderWindow unable to open image file for writing\n");
      return 0;
      }
  }
  return 1;
}

void vtkRenderWindow::ClosePPMImageFile()
{
  fclose(this->PPMImageFilePtr);
  this->PPMImageFilePtr = NULL;
}


void vtkRenderWindow::WritePPMImageFile()
{
  int    *size;
  unsigned char *buffer;
  int i;

  // get the size
  size = this->GetSize();
  // get the data
  buffer = this->GetPixelData(0,0,size[0]-1,size[1]-1,1);

  if(!this->PPMImageFilePtr)
    {
    vtkErrorMacro(<< "RenderWindow: no image file for writing\n");
    return;
    }
 
  // write out the header info 
  fprintf(this->PPMImageFilePtr,"P6\n%i %i\n255\n",size[0],size[1]);
 
  // now write the binary info 
  for (i = size[1]-1; i >= 0; i--)
    {
    fwrite(buffer + i*size[0]*3,1,size[0]*3,this->PPMImageFilePtr);
    }

  delete [] buffer;
}



// Description:
// Update the system, if needed, due to stereo rendering. For some stereo 
// methods, subclasses might need to switch some hardware settings here.
void vtkRenderWindow::StereoUpdate(void)
{
  // if stereo is on and it wasn't before
  if (this->StereoRender && (!this->StereoStatus))
    {
    switch (this->StereoType) 
      {
      case VTK_STEREO_RED_BLUE:
	{
        this->StereoStatus = 1;
	}
      }
    }
  else if ((!this->StereoRender) && this->StereoStatus)
    {
    switch (this->StereoType) 
      {
      case VTK_STEREO_RED_BLUE:
	{
        this->StereoStatus = 0;
	}
      }
    }
}

// Description:
// Intermediate method performs operations required between the rendering
// of the left and right eye.
void vtkRenderWindow::StereoMidpoint(void)
{
  switch (this->StereoType) 
    {
    case VTK_STEREO_RED_BLUE:
      {
      int *size;
      // get the size
      size = this->GetSize();
      // get the data
      this->StereoBuffer = this->GetPixelData(0,0,size[0]-1,size[1]-1,0);
      }
    }
}

// Description:
// Handles work required once both views have been rendered when using
// stereo rendering.
void vtkRenderWindow::StereoRenderComplete(void)
{
  switch (this->StereoType) 
    {
    case VTK_STEREO_RED_BLUE:
      {
      unsigned char *buff;
      unsigned char *p1, *p2, *p3;
      unsigned char* result;
      int *size;
      int x,y;
      int res;

      // get the size
      size = this->GetSize();
      // get the data
      buff = this->GetPixelData(0,0,size[0]-1,size[1]-1,0);
      p1 = this->StereoBuffer;
      p2 = buff;

      // allocate the result
      result = new unsigned char [size[0]*size[1]*3];
      if (!result)
	{
	vtkErrorMacro(<<"Couldn't allocate memory for RED BLUE stereo.");
	return;
	}
      p3 = result;

      // now merge the two images 
      for (x = 0; x < size[0]; x++)
	{
	for (y = 0; y < size[1]; y++)
	  {
	  res = p1[0] + p1[1] + p1[2];
	  p3[0] = res/3;
	  res = p2[0] + p2[1] + p2[2];
	  p3[1] = 0;
	  p3[2] = res/3;
	  
	  p1 += 3;
	  p2 += 3;
	  p3 += 3;
	  }
	}
      this->ResultFrame = result;
      delete [] this->StereoBuffer;
      this->StereoBuffer = NULL;
      delete [] buff;
      }
      break;
    }
}


void vtkRenderWindow::CopyResultFrame(void)
{
  if (this->ResultFrame)
    {
    int *size;

    // get the size
    size = this->GetSize();
    this->SetPixelData(0,0,size[0]-1,size[1]-1,this->ResultFrame,0);
    }

  this->Frame();
}

// Description:
// This method indicates if a StereoOn/Off will require the window to 
// be remapped. Some types of stereo rendering require a new window
// to be created.
int vtkRenderWindow::GetRemapWindow(void)
{
  switch (this->StereoType) 
    {
    case VTK_STEREO_RED_BLUE: return 0;
    case VTK_STEREO_CRYSTAL_EYES: return 1;
    }
  return 0;
}
