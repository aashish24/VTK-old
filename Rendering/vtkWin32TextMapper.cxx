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
#include "vtkWin32TextMapper.h"
#include "vtkWin32ImageWindow.h"

vtkWin32TextMapper::vtkWin32TextMapper()
{
  this->LastSize[0] = 0;
  this->LastSize[1] = 0;
  this->Font = 0;
}

void vtkWin32TextMapper::GetSize(vtkViewport* viewport, int *size)
{
  if ( this->NumberOfLines > 1 )
    {
    this->GetMultiLineSize(viewport, size);
    return;
    }

  // Check to see whether we have to rebuild anything
  if ( this->GetMTime() < this->BuildTime)
    {
    size[0] = this->LastSize[0];
    size[1] = this->LastSize[1];
    return;
    }
  
  // Check for input
  if (this->Input == NULL) 
    {
    vtkErrorMacro (<<"vtkWin32TextMapper::Render - No input");
    return;
    }

  // Get the window information for display
  vtkWindow*  window = viewport->GetVTKWindow();
  // Get the device context from the window
  HDC hdc = (HDC) window->GetGenericContext();
 
  // Create the font
  LOGFONT fontStruct;
  char fontname[32];
  DWORD family;
  switch (this->FontFamily)
    {
    case VTK_ARIAL:
      strcpy(fontname, "Arial");
	  family = FF_SWISS;
	  break;
	case VTK_TIMES:
      strcpy(fontname, "Times Roman");
	  family = FF_ROMAN;
	  break;
	case VTK_COURIER:
      strcpy(fontname, "Courier");
	  family = FF_MODERN;
	  break;
	default:
      strcpy(fontname, "Arial");
	  family = FF_SWISS;
	  break;
    }
  fontStruct.lfHeight = MulDiv(this->FontSize, 
			       window->GetDPI(), 72);  
  // height in logical units
  fontStruct.lfWidth = 0;  // default width
  fontStruct.lfEscapement = 0;
  fontStruct.lfOrientation = 0;
  if (this->Bold == 1)
    {
    fontStruct.lfWeight = FW_BOLD;
    }
  else 
    {
    fontStruct.lfWeight = FW_NORMAL;
    }
  fontStruct.lfItalic = this->Italic;
  fontStruct.lfUnderline = 0;
  fontStruct.lfStrikeOut = 0;
  fontStruct.lfCharSet = ANSI_CHARSET;
  fontStruct.lfOutPrecision = OUT_DEFAULT_PRECIS;
  fontStruct.lfClipPrecision = CLIP_DEFAULT_PRECIS;
  fontStruct.lfQuality = DEFAULT_QUALITY;
  fontStruct.lfPitchAndFamily = DEFAULT_PITCH | family;
  strcpy(fontStruct.lfFaceName, fontname);
   
  if (this->Font)
    {
    DeleteObject(this->Font);
    }
  this->Font = CreateFontIndirect(&fontStruct);
  HFONT hOldFont = (HFONT) SelectObject(hdc, this->Font);

  // Define bounding rectangle
  RECT rect;
  rect.left = 0;
  rect.top = 0;
  rect.bottom = 0;
  rect.right = 0;

  // Calculate the size of the bounding rectangle
  size[1] = DrawText(hdc, this->Input, strlen(this->Input), &rect, 
		     DT_CALCRECT|DT_LEFT|DT_NOPREFIX);
  size[0] = rect.right - rect.left + 1;
  this->LastSize[0] = size[0];
  this->LastSize[1] = size[1];
  this->BuildTime.Modified();
}

void vtkWin32TextMapper::RenderOverlay(vtkViewport* viewport, 
				       vtkActor2D* actor)
{
  vtkDebugMacro (<< "RenderOverlay");

  // Check for input
  if ( this->NumberOfLines > 1 )
    {
    this->RenderMultipleLines(viewport, actor);
    return;
    }

  // Check for input
  if (this->Input == NULL) 
    {
    vtkErrorMacro (<<"Render - No input");
    return;
    }

  int size[2];
  this->GetSize(viewport, size);

  // Get the window information for display
  vtkWindow*  window = viewport->GetVTKWindow();
  // Get the device context from the window
  HDC hdc = (HDC) window->GetGenericContext();
 
  // Get the position of the text actor
  POINT ptDestOff;
  int* actorPos = 
    actor->GetPositionCoordinate()->GetComputedLocalDisplayValue(viewport);
  ptDestOff.x = actorPos[0];
  ptDestOff.y = actorPos[1] - (this->LineOffset * this->LineSpacing * size[1]);

  // Set up the font color from the text actor
  unsigned char red = 0;
  unsigned char green = 0;
  unsigned char blue = 0;
  float*  actorColor = actor->GetProperty()->GetColor();
  red = (unsigned char) (actorColor[0] * 255.0);
  green = (unsigned char) (actorColor[1] * 255.0);
  blue = (unsigned char) (actorColor[2] * 255.0);

  // Set up the shadow color
  float intensity;
  intensity = (red + green + blue)/3.0;

  unsigned char shadowRed, shadowGreen, shadowBlue;
  if (intensity > 128)
    {
    shadowRed = shadowBlue = shadowGreen = 0;
    }
  else
    {
    shadowRed = shadowBlue = shadowGreen = 255;
    }


  // Define bounding rectangle
  RECT rect;
  rect.left = ptDestOff.x;
  rect.top = ptDestOff.y;
  rect.bottom = ptDestOff.y;
  rect.right = ptDestOff.x;

  SelectObject(hdc, this->Font);
  rect.right = rect.left + size[0];
  rect.top = rect.bottom - size[1];
  
  // Set the compositing operator
  SetROP2(hdc, R2_COPYPEN);

  int winJust;
  switch (this->Justification)
    {
    int tmp;
    case VTK_TEXT_LEFT: winJust = DT_LEFT; break;
    case VTK_TEXT_CENTERED:
      winJust = DT_CENTER;
      tmp = rect.right - rect.left + 1;
      rect.left = rect.left - tmp/2;
      rect.right = rect.left + tmp;
      break;
    case VTK_TEXT_RIGHT: 
      winJust = DT_RIGHT;
      tmp = rect.right - rect.left + 1;
      rect.left = rect.right;
      rect.right = rect.right - tmp;
    }
  switch (this->VerticalJustification)
    {
    case VTK_TEXT_TOP: 
      rect.top = rect.bottom;
      rect.bottom = rect.bottom - size[1];
      break;
    case VTK_TEXT_CENTERED:
      rect.bottom = rect.bottom - size[1]/2;
      rect.top = rect.bottom + size[1];
      break;
    case VTK_TEXT_BOTTOM: 
      break;
    }

  // Set the colors for the shadow
  long status;
  if (this->Shadow)
    {
    status = SetTextColor(hdc, RGB(shadowRed, shadowGreen, shadowBlue));
    if (status == CLR_INVALID)
      vtkErrorMacro(<<"vtkWin32TextMapper::Render - Set shadow color failed!");

    // Set the background mode to transparent
    SetBkMode(hdc, TRANSPARENT);

    // Draw the shadow text
    rect.left++;  rect.top++; rect.bottom++; rect.right++;
    DrawText(hdc, this->Input, strlen(this->Input), &rect,winJust|DT_NOPREFIX);
    rect.left--;  rect.top--; rect.bottom--; rect.right--;
    }
  
  // set the colors for the foreground
  status = SetTextColor(hdc, RGB(red, green, blue));
  if (status == CLR_INVALID)
    vtkErrorMacro(<<"vtkWin32TextMapper::Render - SetTextColor failed!");

  // Set the background mode to transparent
  SetBkMode(hdc, TRANSPARENT);

  // Draw the text
  DrawText(hdc, this->Input, strlen(this->Input), &rect, winJust|DT_NOPREFIX);

}
