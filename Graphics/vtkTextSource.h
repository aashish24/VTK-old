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
// .NAME vtkTextSource - create polygonal text
// .SECTION Description
// vtkTextSource converts a text string into polygons.  This way you can 
// insert text into your renderings. It uses the 9x15 font from X Windows.
// You can specify if you want the background to be drawn or not. The
// characters are formed by scan converting the raster font into
// quadrilaterals. Colors are assigned to the letters using scalar data.
// To set the color of the characters with the source's actor property, set
// BackingOff on the text source and ScalarVisibilityOff on the associated
// vtkPolyDataMapper. Then, the color can be set using the associated actor's
// property.
//
// vtkVectorText generates higher quality polygonal representations of
// characters.

// .SECTION See Also
// vtkVectorText

#ifndef __vtkTextSource_h
#define __vtkTextSource_h

#include "vtkPolyDataSource.h"

class VTK_EXPORT vtkTextSource : public vtkPolyDataSource 
{
public:
  vtkTextSource();
  ~vtkTextSource();
  const char *GetClassName() {return "vtkTextSource";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct object with no string set and backing enabled.
  static vtkTextSource *New() {return new vtkTextSource;};

  // Description:
  // Set/Get the text to be drawn.
  vtkSetStringMacro(Text);
  vtkGetStringMacro(Text);

  // Description:
  // Controls whether or not a background is drawn with the text.
  vtkSetMacro(Backing,int);
  vtkGetMacro(Backing,int);
  vtkBooleanMacro(Backing,int);

  // Description:
  // Set/Get the foreground color. Default is white (1,1,1). ALpha is always 1.
  vtkSetVector3Macro(ForegroundColor,float);
  vtkGetVectorMacro(ForegroundColor,float,3);

  // Description:
  // Set/Get the background color. Default is black (0,0,0). Alpha is always 1.
  vtkSetVector3Macro(BackgroundColor,float);
  vtkGetVectorMacro(BackgroundColor,float,3);

protected:
  void Execute();
  char *Text;
  int  Backing;
  float ForegroundColor[4];
  float BackgroundColor[4];
};

#endif


