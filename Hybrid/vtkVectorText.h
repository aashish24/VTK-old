/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkVectorText - create polygonal text
// .SECTION Description

// vtkVectorText generates vtkPolyData from an input text string. Besides the
// ASCII alphanumeric characters a-z, A-Z, 0-9, vtkVectorText also supports
// ASCII punctuation marks. (The supported ASCII character set are the codes
// (33-126) inclusive.) The only control character supported is the line feed
// character "\n", which advances to a new line.
//
// To use thie class, you normally couple it with a vtkPolyDataMapper and a
// vtkActor. In this case you would use the vtkActor's transformation methods
// to position, orient, and scale the text. You may also wish to use a
// vtkFollower to orient the text so that it always faces the camera.

// .SECTION See Also
// vtkTextMapper vtkCaptionActor2D

#ifndef __vtkVectorText_h
#define __vtkVectorText_h

#include "vtkPolyDataSource.h"

class VTK_HYBRID_EXPORT vtkVectorText : public vtkPolyDataSource 
{
public:
  static vtkVectorText *New();
  vtkTypeMacro(vtkVectorText,vtkPolyDataSource);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the text to be drawn.
  vtkSetStringMacro(Text);
  vtkGetStringMacro(Text);

protected:
  vtkVectorText();
  ~vtkVectorText();

  void Execute();
  char *Text;
  char *Letters[127];

private:
  vtkVectorText(const vtkVectorText&);  // Not implemented.
  void operator=(const vtkVectorText&);  // Not implemented.
};

#endif


