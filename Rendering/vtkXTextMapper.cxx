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
#include "vtkXTextMapper.h"

void vtkXTextMapper::SetFontSize(int size)
{
  int newSize;

  // Make sure that the font size matches an available X font size.
  // This routine assumes that some standard X fonts are installed.
  switch (size)
    {  
    // available X Font sizes
    case 8:
    case 10:
    case 12:
    case 14:
    case 18:
    case 24:
      newSize = size;
      break;

    // In between sizes use next larger size
    case 9:
      newSize = 10;
      break;
    case 11:
      newSize = 12;
      break;
    case 13:
      newSize = 14;
      break;
    case 15:
    case 16:
    case 17:
      newSize = 18;
      break;
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
      newSize = 24;
      break;

    // catch the values outside the font range 
    default:
      if (size < 8) newSize = 8;
      else if (size > 24) newSize  = 24;
      else newSize = 12;   // just in case we missed something above
      break;
    }
  
  if (this->FontSize != newSize)
    {
      this->FontSize = newSize;
      this->FontMTime.Modified();
    }
  return;
}

int vtkXTextMapper::GetCompositingMode(vtkActor2D* actor)
{

  vtkProperty2D* tempProp = actor->GetProperty();
  int compositeMode = tempProp->GetCompositingOperator();

  switch (compositeMode)
  {
  case VTK_BLACK:
	  return GXclear;
  case VTK_NOT_DEST:
	  return GXinvert;
  case VTK_SRC_AND_DEST:
	  return GXand;
  case VTK_SRC_OR_DEST:
	  return  GXor;
  case VTK_NOT_SRC:
	  return GXcopyInverted;
  case VTK_SRC_XOR_DEST:
	  return GXxor;
  case VTK_SRC_AND_notDEST:
	  return GXandReverse;
  case VTK_SRC:
	  return GXcopy;
  case VTK_WHITE:
	  return GXset;
  default:
	  return GXcopy;
  }

}

void vtkXTextMapper::GetSize(vtkViewport* viewport, int *size)
{
  // Get the window info
  vtkWindow*  window = viewport->GetVTKWindow();
  Display* displayId = (Display*) window->GetGenericDisplayId();

  // Set up the font name string. Note: currently there is no checking to see
  // if we've exceeded the fontname length.
  // Foundry
  char fontname[256]= "*";
 
  // Family
  switch (this->FontFamily)
    {
    case VTK_ARIAL:
      strcat(fontname, "helvetica-");
      break;
    case VTK_COURIER:
      strcat(fontname, "courier-");
      break;
    case VTK_TIMES:
      strcat(fontname, "times-");
      break;
    default:
      strcat(fontname, "helvetica-");
    }

  // Weight
  if (this->Bold == 1)
    {
    strcat(fontname, "bold-");
    }
  else
    {
    strcat (fontname, "medium-");
    }

  // Slant
  if (this->Italic == 1)
    {
    if (this->FontFamily == VTK_TIMES) strcat(fontname, "i-");
    else strcat(fontname, "o-");
    }
  else
    {
    strcat(fontname, "r-");
    }

  char tempString[100];
 
  // Set width, pixels, point size
  sprintf(tempString, "*-%d-*", 10*this->FontSize);

  strcat(fontname, tempString);

  vtkDebugMacro(<<"vtkXTextMapper::Render - Font specifier: " << fontname);

  // Set the font
  int cnt;
  char **fn = XListFonts(displayId, fontname, 1, &cnt);
  if (fn)
    {
    XFreeFontNames(fn);
    }
  if (!cnt)
    {
    sprintf(fontname,"9x15");
    }
  Font font = XLoadFont(displayId,  fontname );
  int dir, as, des;
  XCharStruct overall;
  XQueryTextExtents(displayId, font, this->Input, strlen(this->Input),
		    &dir, &as, &des, &overall);
  size[1] = as + des;
  size[0] = overall.width;
  this->CurrentFont = font;
}

void vtkXTextMapper::RenderOverlay(vtkViewport* viewport, vtkActor2D* actor)
{
  if (this->Input == NULL) 
    {
    vtkDebugMacro (<<"vtkXTextMapper::Render - No input");
    return;
    }
  
  // Get the window info
  vtkWindow*  window = viewport->GetVTKWindow();
  Display* displayId = (Display*) window->GetGenericDisplayId();
  GC gc = (GC) window->GetGenericContext();
  Window windowId = (Window) window->GetGenericWindowId();

  int* actorPos = 
    actor->GetPositionCoordinate()->GetComputedLocalDisplayValue(viewport);

  // Set up the font color
  float* actorColor = actor->GetProperty()->GetColor();
  unsigned char red = (unsigned char) (actorColor[0] * 255.0);
  unsigned char green = (unsigned char) (actorColor[1] * 255.0);
  unsigned char  blue = (unsigned char)  (actorColor[2] * 255.0);

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
  
  // Use the color masks from the visual
  unsigned long rmask = 0;
  unsigned long gmask = 0;
  unsigned long bmask = 0;

  XWindowAttributes winAttribs;
  XGetWindowAttributes(displayId, windowId, &winAttribs);
 
  XVisualInfo temp1;
  temp1.visualid = winAttribs.visual->visualid;

  int nvisuals = 0;
  XVisualInfo* visuals = XGetVisualInfo(displayId, VisualIDMask, &temp1,
                                        &nvisuals);   

  if (nvisuals == 0)  vtkErrorMacro(<<"Could not get color masks");

  rmask = visuals->red_mask;
  gmask = visuals->green_mask;
  bmask = visuals->blue_mask;
  
  XFree(visuals);

  // Compute the shifts to match up the pixel bits with the mask bits
  int rshift = 0;
  while ( ((rmask & 0x80000000) == 0) && (rshift < 32))
    {
    rmask = rmask << 1;
    rshift++;
    }

  int gshift = 0;
  while ( ((gmask & 0x80000000) == 0) && (gshift < 32))
    {
    gmask = gmask << 1;
    gshift++;
    }

  int bshift = 0;
  while ( ((bmask & 0x80000000) == 0) && (bshift < 32))
    {
    bmask = bmask << 1;
    bshift++;
    }

  // Mask the colors into the foreground variable
  unsigned long foreground = 0;
  foreground = foreground | ((rmask & (red << 24)) >> rshift);
  foreground = foreground | ((gmask & (green << 24)) >> gshift);
  foreground = foreground | ((bmask & (blue << 24)) >> bshift);

  unsigned long shadowForeground = 0;
  shadowForeground = shadowForeground | ((rmask & (shadowRed << 24)) >>rshift);
  shadowForeground = shadowForeground | ((gmask & (shadowGreen<<24)) >>gshift);
  shadowForeground = shadowForeground | ((bmask & (shadowBlue<< 24)) >>bshift);

  // compute the size of the string so that we can center it etc.
  // a side effect is that this->CurrentFont will be set so that
  // we can use it here. That saves the expensice process of 
  // computing it again
  int size[2];
  this->GetSize(viewport,size);
  XSetFont(displayId, gc, this->CurrentFont);
  
  // adjust actorPos to account for justification
  int pos[2];
  pos[0] = actorPos[0];
  pos[1] = actorPos[1];
  switch (this->Justification)
    {
    // do nothing for case 0 left
    case 1: pos[0] = pos[0] - size[0] / 2; break;
    case 2: pos[0] = pos[0] - size[0]; break;
    }
  
    
  // Set the compositing mode for the actor
  int compositeMode = this->GetCompositingMode(actor);
  XSetFunction(displayId, gc, compositeMode);
 
  // Get the drawable to draw into
  Drawable drawable = (Drawable) window->GetGenericDrawable();
  if (!drawable) vtkErrorMacro(<<"Window returned NULL drawable!");

  // Draw the shadow
  if (this->Shadow)
    {
    XSetForeground(displayId, gc, shadowForeground);
    XDrawString(displayId, drawable, gc, pos[0]+1, pos[1]+1,
                this->Input, strlen(this->Input));
    }
  
  // Draw the string
  XSetForeground(displayId, gc, foreground);
  XDrawString(displayId, drawable, gc, pos[0], pos[1],
              this->Input, strlen(this->Input));
 
  // Flush the X queue
  XFlush(displayId);
  XSync(displayId, False);
}







