/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
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
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkVRMLExporter - export a scene into VRML 2.0 format.
// .SECTION Description
// vtkVRMLExporter is a concrete subclass of vtkExporter that writes VRML 2.0
// files. This is based on the VRML 2.0 draft #3 but it should be pretty
// stable since we aren't using any of the newer features.
//
// .SECTION See Also
// vtkExporter


#ifndef __vtkVRMLExporter_h
#define __vtkVRMLExporter_h

#include <stdio.h>
#include "vtkExporter.h"

class VTK_EXPORT vtkVRMLExporter : public vtkExporter
{
public:
  static vtkVRMLExporter *New();
  vtkTypeMacro(vtkVRMLExporter,vtkExporter);
  void PrintSelf(vtkOstream& os, vtkIndent indent);

  // Description:
  // Specify the name of the VRML file to write.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Specify the Speed of navigation. Default is 4.
  vtkSetMacro(Speed,float);
  vtkGetMacro(Speed,float);

  // Description:
  // Set the file pointer to write to. This will override
  // a FileName if specified.
  void SetFilePointer(FILE *);
  FILE *GetFilePointer();
  
protected:
  vtkVRMLExporter();
  ~vtkVRMLExporter();
  vtkVRMLExporter(const vtkVRMLExporter&) {};
  void operator=(const vtkVRMLExporter&) {};

  void WriteData();
  void WriteALight(vtkLight *aLight, FILE *fp);
  void WriteAnActor(vtkActor *anActor, FILE *fp);
  void WritePointData(vtkPoints *points, vtkNormals *normals, 
		      vtkTCoords *tcoords, vtkScalars *colors, FILE *fp);
  char *FileName;
  FILE *FilePointer;
  float Speed;
};

#endif

