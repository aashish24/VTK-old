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
// .NAME vtkGESignaReader - read GE Signa ximg files
// .SECTION Description
// vtkGESignaReader is a source object that reads some GE Signa ximg
// files It does support reading in pixel spacing, and slice
// thickness, which is interprested as slice spacing. It always
// produces greyscale unsigned short data and it supports reading in
// rectangular, packed, compressed, and packed&compressed. It does not
// read in slice orientation, or position right now. To use it you
// just need to specify a filename or a file prefix and pattern.

//
// .SECTION See Also
// vtkImageReader2

#ifndef __vtkGESignaReader_h
#define __vtkGESignaReader_h

#include <stdio.h>
#include "vtkImageReader2.h"

class VTK_IO_EXPORT vtkGESignaReader : public vtkImageReader2
{
public:
  static vtkGESignaReader *New();
  vtkTypeMacro(vtkGESignaReader,vtkImageReader2);

protected:
  vtkGESignaReader() {};
  ~vtkGESignaReader() {};

  virtual void ExecuteInformation();
  virtual void ExecuteData(vtkDataObject *out);

private:
  vtkGESignaReader(const vtkGESignaReader&);  // Not implemented.
  void operator=(const vtkGESignaReader&);  // Not implemented.
};
#endif


