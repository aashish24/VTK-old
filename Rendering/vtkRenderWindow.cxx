/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Toolkit. No part of this file or its
contents may be copied, reproduced or altered in any way without the express
written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994

=========================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "RenderW.hh"
#include "Interact.hh"

// Description:
// Construct object with screen size 300x300, borders turned on, position
// at (0,0), and double buffering turned on.
vtkRenderWindow::vtkRenderWindow()
{
  this->Size[0] = this->Size[1] = 300;
  this->Position[0] = this->Position[1] = 0;
  this->Borders = 1;
  this->FullScreen = 0;
  this->OldScreen[0] = this->OldScreen[1] = 0;
  this->OldScreen[2] = this->OldScreen[3] = 300;
  this->OldScreen[4] = 1;
  this->Mapped = 0;
  this->DoubleBuffer = 1;
  this->StereoRender = 0;
  this->StereoType = VTK_STEREO_RED_BLUE;
  this->StereoStatus = 0;
  this->Interactor = NULL;
  strcpy(this->Name,"Visualization Toolkit");
  this->AAFrames = 0;
  this->FDFrames = 0;
  this->SubFrames = 0;
  this->AccumulationBuffer = NULL;
  this->CurrentSubFrame = 0;
  this->ResultFrame = NULL;
  this->Filename = NULL;
}

// Description:
// free the memory used by this object
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
// Ask each renderer to render an image. Synchronize this process.
void vtkRenderWindow::Render()
{
  int i;
  int *size;
  int x,y;
  float *p1;
  
  vtkDebugMacro(<< "Starting Render Method.\n");
  
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
      if (this->ResultFrame)
	{
	p2 = this->ResultFrame;
	}
      else
	{
	p2 = this->GetPixelData(0,0,size[0]-1,size[1]-1,0);
	}
      for (y = 0; y < size[1]; y++)
	{
	for (x = 0; x < size[0]; x++)
	  {
	  *p1 += *p2; p1++; p2++;
	  *p1 += *p2; p1++; p2++;
	  *p1 += *p2; p1++; p2++;
	  }
	}
      delete [] p2;
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

      // free any memory
      delete [] this->AccumulationBuffer;
      this->AccumulationBuffer = NULL;
      }
    }
  else
    {
    // get the size
    size = this->GetSize();

    this->DoAARender();
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
    }
  
  if (this->ResultFrame) 
    {
    delete [] this->ResultFrame;
    this->ResultFrame = NULL;
    }
}

// Description:
// Ask each renderer to render an aa image. Synchronize this process.
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
      offsets[0] = drand48() - 0.5;
      offsets[1] = drand48() - 0.5;

      for (this->Renderers.InitTraversal(); 
	   aren = this->Renderers.GetNextItem(); )
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
	   aren = this->Renderers.GetNextItem(); )
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
	if (this->ResultFrame)
	  {
	  p2 = this->ResultFrame;
	  }
	else
	  {
	  p2 = this->GetPixelData(0,0,size[0]-1,size[1]-1,0);
	  }
	for (y = 0; y < size[1]; y++)
	  {
	  for (x = 0; x < size[0]; x++)
	    {
	    *p1 += (float)*p2; p1++; p2++;
	    *p1 += (float)*p2; p1++; p2++;
	    *p1 += (float)*p2; p1++; p2++;
	    }
	  }
	delete [] p2;
	}
      }
    }
  else
    {
    this->DoFDRender();
    }
}


// Description:
// Ask each renderer to render an image. Synchronize this process.
void vtkRenderWindow::DoFDRender()
{
  int i;

  // handle any focal depth
  if (this->FDFrames)
    {
    int *size;
    int x,y;
    unsigned char *p2;
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

      offsets[0] = drand48(); // radius
      offsets[1] = drand48()*360.0; // angle

      // store offsets for each renderer 
      for (this->Renderers.InitTraversal(); 
	   aren = this->Renderers.GetNextItem(); )
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
	   aren = this->Renderers.GetNextItem(); )
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
      for (y = 0; y < size[1]; y++)
	{
	for (x = 0; x < size[0]; x++)
	  {
	  *p1 += (float)*p2; p1++; p2++;
	  *p1 += (float)*p2; p1++; p2++;
	  *p1 += (float)*p2; p1++; p2++;
	  }
	}
      delete [] p2;;
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
// Ask each renderer to render an image.
void vtkRenderWindow::DoStereoRender()
{
  this->Start();
  this->StereoUpdate();
  this->Renderers.Render();
  if (this->StereoRender)
    {
    this->StereoMidpoint();
    this->Renderers.Render();
    this->StereoRenderComplete();
    }
}

// Description:
// Add a renderer to the list of renderers.
void vtkRenderWindow::AddRenderers(vtkRenderer *ren)
{
  // we are its parent 
  ren->SetRenderWindow(this);
  this->Renderers.AddItem(ren);
}

// Description:
// Remove a renderer from the list of renderers.
void vtkRenderWindow::RemoveRenderers(vtkRenderer *ren)
{
  // we are its parent 
  this->Renderers.RemoveItem(ren);
}

// Description:
// Set the size of the window in screen coordinates.
void vtkRenderWindow::SetSize(int a[2])
{
  this->SetSize(a[0],a[1]);
}

// Description:
// Set the size of the window in screen coordinates.
void vtkRenderWindow::SetPosition(int a[2])
{
  this->SetPosition(a[0],a[1]);
}
// Description:
// Set the size of the window in screen coordinates.
void vtkRenderWindow::SetPosition(int x, int y)
{
  // if we arent mappen then just set the ivars 
  if (!this->Mapped)
    {
    if ((this->Position[0] != x)||(this->Position[1] != y))
      {
      this->Modified();
      }
    this->Position[0] = x;
    this->Position[1] = y;
    }
}

void vtkRenderWindow::PrintSelf(ostream& os, vtkIndent indent)
{
  int *temp;

  vtkObject::PrintSelf(os,indent);

  os << indent << "Borders: " << (this->Borders ? "On\n":"Off\n");
  os << indent << "Double Buffer: " << (this->DoubleBuffer ? "On\n":"Off\n");
  os << indent << "Full Screen: " << (this->FullScreen ? "On\n":"Off\n");
  os << indent << "Name: " << this->Name << "\n";
  temp = this->GetPosition();
  os << indent << "Position: (" << temp[0] << ", " << temp[1] << ")\n";
  temp = this->GetSize();
  os << indent << "Renderers:\n";
  this->Renderers.PrintSelf(os,indent.GetNextIndent());
  os << indent << "Size: (" << temp[0] << ", " << temp[1] << ")\n";
  os << indent << "Stereo Render: " 
     << (this->StereoRender ? "On\n":"Off\n");

  if ( this->Filename )
    os << indent << "Filename: " << this->Filename << "\n";
  else
    os << indent << "Filename: (None)\n";
}


void vtkRenderWindow::SaveImageAsPPM()
{
  int    *size;
  FILE   *fp;
  unsigned char *buffer;
  int i;

  // get the size
  size = this->GetSize();
  // get the data
  buffer = this->GetPixelData(0,0,size[0]-1,size[1]-1,1);

  //  open the ppm file and write header 
  if ( this->Filename != NULL && *this->Filename != '\0')
    {
    fp = fopen(this->Filename,"w");
    if (!fp)
      {
      vtkErrorMacro(<< "RenderWindow unable to open image file for writing\n");
      delete [] buffer;
      return;
      }
 
    // write out the header info 
    fprintf(fp,"P6\n%i %i\n255\n",size[0],size[1]);
 
    // now write the binary info 
    for (i = size[1]-1; i >= 0; i--)
      {
      fwrite(buffer + i*size[0]*3,3,size[0],fp);
      }
    fclose(fp);
    }

  delete [] buffer;
}


// Description:
// Update system if needed due to stereo rendering.
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
// Handles work required between the left and right eye renders.
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
// Handles work required between the left and right eye renders.
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


// Description:
// Handles work required at end of render cycle
void vtkRenderWindow::CopyResultFrame(void)
{
  if (this->ResultFrame)
    {
    int *size;

    // get the size
    size = this->GetSize();
    this->SetPixelData(0,0,size[0]-1,size[1]-1,this->ResultFrame,1);
    }
  else
    {
    this->Frame();
    }
}

// Description:
// Indicates if a StereoOn will require the window to be remapped.
int vtkRenderWindow::GetRemapWindow(void)
{
  switch (this->StereoType) 
    {
    case VTK_STEREO_RED_BLUE: return 0;
    case VTK_STEREO_CRYSTAL_EYES: return 1;
    }
}
