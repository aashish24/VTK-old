/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to C. Charles Law who developed this class.

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
// .NAME vtkImageExtractComponents - Outputs a single component
// .SECTION Description
// vtkImageExtractComponents takes an input with any number of components
// and outputs some of them.  It does involve a copy of the data.

// .SECTION See Also
// vtkImageAppendComponents

#ifndef __vtkImageExtractComponents_h
#define __vtkImageExtractComponents_h


#include "vtkImageToImageFilter.h"

class VTK_IMAGING_EXPORT vtkImageExtractComponents : public vtkImageToImageFilter
{
public:
  static vtkImageExtractComponents *New();
  vtkTypeMacro(vtkImageExtractComponents,vtkImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the components to extract.
  void SetComponents(int c1);
  void SetComponents(int c1, int c2);
  void SetComponents(int c1, int c2, int c3);
  vtkGetVector3Macro(Components,int);
  
  // Description:
  // Get the number of components to extract. This is set implicitly by the 
  // SetComponents() method.
  vtkGetMacro(NumberOfComponents,int);

protected:
  vtkImageExtractComponents();
  ~vtkImageExtractComponents() {};

  int NumberOfComponents;
  int Components[3];

  void ExecuteInformation(vtkImageData *inData, vtkImageData *outData);
  void ExecuteInformation(){this->vtkImageToImageFilter::ExecuteInformation();};
  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData, 
		       int ext[6], int id);
private:
  vtkImageExtractComponents(const vtkImageExtractComponents&);  // Not implemented.
  void operator=(const vtkImageExtractComponents&);  // Not implemented.
};

#endif










