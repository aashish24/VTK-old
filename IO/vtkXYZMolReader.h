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
// .NAME vtkXYZMolReader - read Molecular Data files
// .SECTION Description
// vtkXYZMolReader is a source object that reads Molecule files
// The FileName must be specified
//
// .SECTION Thanks
// Dr. Jean M. Favre who developed and contributed this class

#ifndef __vtkXYZMolReader_h
#define __vtkXYZMolReader_h

#include "vtkMoleculeReaderBase.h"


class VTK_IO_EXPORT vtkXYZMolReader : public vtkMoleculeReaderBase
{
public:
  vtkTypeRevisionMacro(vtkXYZMolReader,vtkMoleculeReaderBase);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkXYZMolReader *New();

  // Description:
  // Test whether the file with the given name can be read by this
  // reader.
  virtual int CanReadFile(const char* name);

  // Description:
  // Set the current time step. It should be greater than 0 and smaller than
  // MaxTimeStep.
  vtkSetMacro(TimeStep, int);
  vtkGetMacro(TimeStep, int);

  // Description:
  // Get the maximum time step.
  vtkGetMacro(MaxTimeStep, int);

protected:
  vtkXYZMolReader();
  ~vtkXYZMolReader();

  void ReadSpecificMolecule(FILE* fp);

  // Description:
  // Get next line that is not a comment. It returns the beginning of data on
  // line (skips empty spaces)
  char* GetNextLine(FILE* fp, char* line, int maxlen);

  int GetLine1(const char* line, int *cnt);
  int GetLine2(const char* line, char *name);
  int GetAtom(const char* line, char* atom, float *x);

  void InsertAtom(const char* atom, float *pos);

  vtkSetMacro(MaxTimeStep, int);

  int TimeStep;
  int MaxTimeStep;

private:
  vtkXYZMolReader(const vtkXYZMolReader&);  // Not implemented.
  void operator=(const vtkXYZMolReader&);  // Not implemented.
};

#endif
