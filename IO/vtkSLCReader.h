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

// .NAME vtkSLCReader - read an SLC volume file.
// .SECTION Description
// vtkSLCReader reads an SLC file and creates a structured point dataset.
// The size of the volume and the data spacing is set from the SLC file
// header.

#ifndef __vtkSLCReader_h
#define __vtkSLCReader_h

#include <stdio.h>
#include "vtkStructuredPointsSource.h"

class VTK_IO_EXPORT vtkSLCReader : public vtkStructuredPointsSource 
{
public:
  static vtkSLCReader *New();
  vtkTypeMacro(vtkSLCReader,vtkStructuredPointsSource);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the name of the file to read.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Was there an error on the last read performed?
  vtkGetMacro(Error,int);
  
protected:
  vtkSLCReader();
  ~vtkSLCReader();
  vtkSLCReader(const vtkSLCReader&);
  void operator=(const vtkSLCReader&);

  // Stores the FileName of the SLC file to read.
  char *FileName;

  // Reads the file name and builds a vtkStructuredPoints dataset.
  void Execute();

  // Not used now, but will be needed when this is made into an 
  // imaging filter.
  // Sets WholeExtent, Origin, Spacing, ScalarType 
  // and NumberOfComponents of output.
  void ExecuteInformation();
  
  // Decodes an array of eight bit run-length encoded data.
  unsigned char *Decode8BitData( unsigned char *in_ptr, int size );
  int Error;
};

#endif


