/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to Matt Turek who developed this class.

Copyright (c) 1993-1995 Ken Martin, Will Schroeder, Bill Lorensen.

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

#include "vtkImageMapper.h"


#ifdef _WIN32
#include "vtkOpenGLImageMapper.h"
#include "vtkWin32ImageMapper.h"
#else
#ifdef VTK_USE_OGLR
#include "vtkOpenGLImageMapper.h"
#endif
#include "vtkXImageMapper.h"
#endif

#include "vtkActor2D.h"
#include "vtkImager.h"
#include "vtkViewport.h"
#include "vtkWindow.h"
#include "vtkImageData.h"

#define VTK_RINT(x) ((x > 0.0) ? (int)(x + 0.5) : (int)(x - 0.5))

vtkImageMapper::vtkImageMapper()
{
  vtkDebugMacro(<< "vtkImageMapper::vtkImageMapper" );

  this->Input = NULL;

  //this->ColorWindow = 255.0;
  //this->ColorLevel = 127.0;

  this->ColorWindow = 2000;
  this->ColorLevel = 1000;

  this->ZSlice = 0;
}

vtkImageMapper::~vtkImageMapper()
{
  if (this->Input)
    {
    this->GetInput()->UnRegister(this);
    this->Input = NULL;
    }
}

void vtkImageMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkMapper2D::PrintSelf(os, indent);

  os << indent << "Input: " << this->Input << "\n";
  os << indent << "Color Window: " << this->ColorWindow << "\n";
  os << indent << "Color Level: " << this->ColorLevel << "\n";
  os << indent << "ZSlice: " << this->ZSlice << "\n";
}

vtkImageMapper* vtkImageMapper::New()
{
#ifdef _WIN32
#ifndef VTK_USE_NATIVE_IMAGING
  return vtkOpenGLImageMapper::New();
#else
  return vtkWin32ImageMapper::New();
#endif
#else
#ifdef VTK_USE_OGLR
#ifndef VTK_USE_NATIVE_IMAGING
  return vtkOpenGLImageMapper::New();
#else
  return vtkXImageMapper::New();
#endif
#else
  return vtkXImageMapper::New();
#endif
#endif
}

float vtkImageMapper::GetColorShift()
{
  return this->ColorWindow /2.0 - this->ColorLevel;
}

float vtkImageMapper::GetColorScale()
{
  return 255.0 / this->ColorWindow;
}

void vtkImageMapper::RenderStart(vtkViewport* viewport, vtkActor2D* actor)
{
  vtkDebugMacro(<< "vtkImageMapper::RenderOverlay");

  vtkImageData *data;
  int displayExtent[6];
  int wholeExtent[6];

  if (!viewport)
    {
    vtkErrorMacro (<< "vtkImageMapper::Render - Null viewport argument");
    return;
    }

  if (!actor)
    {
    vtkErrorMacro (<<"vtkImageMapper::Render - Null actor argument");
    return;    
    }


  if (!this->Input)
    {
    vtkDebugMacro(<< "vtkImageMapper::Render - Please Set the input.");
    return;
    }

  this->GetInput()->UpdateInformation();
  // start with the wholeExtent
  memcpy(wholeExtent,this->GetInput()->GetWholeExtent(),6*sizeof(int));
  memcpy(displayExtent,this->GetInput()->GetWholeExtent(),6*sizeof(int));

  // Set The z values to the zslice
  displayExtent[4] = this->ZSlice;
  displayExtent[5] = this->ZSlice;

  // scale currently not handled
  //float *scale = actor->GetScale();

  // get the position
  int *pos = actor->GetPositionCoordinate()->GetComputedViewportValue(viewport);
  
  // Get the viewport coordinates
  float* vpt = viewport->GetViewport(); 
  float vCoords[4];
  vCoords[0] = 0.0;
  vCoords[1] = 0.0;
  vCoords[2] = 1.0;
  vCoords[3] = 1.0;
  viewport->NormalizedViewportToViewport(vCoords[0],vCoords[1]);
  viewport->NormalizedViewportToViewport(vCoords[2],vCoords[3]);
  int vSize[2];
  // size excludes last pixel except for last pixelof window
  // this is to prevent overlapping viewports
  vSize[0] = VTK_RINT(vCoords[2]) - VTK_RINT(vCoords[0]);
  vSize[1] = VTK_RINT(vCoords[3]) - VTK_RINT(vCoords[1]);
  viewport->ViewportToNormalizedDisplay(vCoords[2],vCoords[3]);
  if (vCoords[2] == 1.0) 
    {
    vSize[0]++;
    }
  if (vCoords[3] == 1.0)
    {
    vSize[1]++;
    }
  
  // the basic formula is that the draw pos equals
  // the pos + extentPos + clippedAmount
  // The concrete subclass will get the pos in display
  // coordinates so we need to provide the extentPos plus
  // clippedAmount in the PositionAdjustment variable

  
  // Now clip to imager extents
  if (pos[0] + wholeExtent[0] < 0) 
    {
    displayExtent[0] = -pos[0];
    }
  if ((pos[0]+wholeExtent[1]) > vSize[0]) 
    {
    displayExtent[1] = vSize[0] - pos[0];
    }
  if (pos[1] + wholeExtent[2] < 0) 
    {
    displayExtent[2] = -pos[1];
    }
  if ((pos[1]+wholeExtent[3]) > vSize[1])
    {
    displayExtent[3] = vSize[1] - pos[1];
    }

  // check for the condition where no pixels are visible.
  if (    displayExtent[0] > wholeExtent[1] || displayExtent[1] < wholeExtent[0]
       || displayExtent[2] > wholeExtent[3] || displayExtent[3] < wholeExtent[2]
       || displayExtent[4] > wholeExtent[5] || displayExtent[5] < wholeExtent[4])
    {
    return;
    }
  
  this->GetInput()->SetUpdateExtent(displayExtent);

  // set the position adjustment
  this->PositionAdjustment[0] = displayExtent[0];
  this->PositionAdjustment[1] = displayExtent[2];
    
  // Get the region from the input
  this->GetInput()->Update();
  data = this->GetInput();
  if ( !data)
    {
    vtkErrorMacro(<< "Render: Could not get data from input.");
    return;
    }

  this->RenderData(viewport, data, actor);
}

//----------------------------------------------------------------------------
int vtkImageMapper::GetWholeZMin()
{
  int *extent;
  
  if ( ! this->Input)
    {
    return 0;
    }
  this->GetInput()->UpdateInformation();
  extent = this->GetInput()->GetWholeExtent();
  return extent[4];
}

//----------------------------------------------------------------------------
int vtkImageMapper::GetWholeZMax()
{
  int *extent;
  
  if ( ! this->Input)
    {
    return 0;
    }
  this->GetInput()->UpdateInformation();
  extent = this->GetInput()->GetWholeExtent();
  return extent[5];
}


