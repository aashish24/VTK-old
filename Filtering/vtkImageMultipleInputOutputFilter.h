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
// .NAME vtkImageMultipleInputOutputFilter - Generic filter that has N inputs.
// .SECTION Description
// vtkImageMultipleInputOutputFilter is a super class for filters that 
// have any number of inputs. Steaming is not available in this class yet.

// .SECTION See Also
// vtkImageToImageFilter vtkImageInPlaceFilter vtkImageTwoInputFilter
// vtkImageTwoOutputFilter



#ifndef __vtkImageMultipleInputOutputFilter_h
#define __vtkImageMultipleInputOutputFilter_h


#include "vtkImageMultipleInputFilter.h"


class VTK_FILTERING_EXPORT vtkImageMultipleInputOutputFilter : public vtkImageMultipleInputFilter
{
public:
  static vtkImageMultipleInputOutputFilter *New();
  vtkTypeMacro(vtkImageMultipleInputOutputFilter,vtkImageSource);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get one input to this filter.
  vtkImageData *GetOutput(int num);
  vtkImageData *GetOutput();

  // Description:
  // The execute method created by the subclass.
  // This is kept public instead of protected since it is called
  // from a non-member thread function.
  virtual void ThreadedExecute(vtkImageData **inDatas, 
			       vtkImageData **outDatas,
			       int extent[6], int threadId);

protected:
  vtkImageMultipleInputOutputFilter();
  ~vtkImageMultipleInputOutputFilter();

  void ComputeInputUpdateExtents( vtkDataObject *output );
  
  virtual void ComputeInputUpdateExtent( int inExt[6], 
					 int outExt[6], 
					 int whichInput );


  void ExecuteData(vtkDataObject *out);

  // this should never be called
  virtual void ThreadedExecute(vtkImageData **inDatas, 
			       vtkImageData *outData,
			       int extent[6], int threadId);
  virtual void ExecuteInformation(vtkImageData **, vtkImageData *) {};

  // This one gets called by the superclass.
  void ExecuteInformation();
  // This is the one you should override.
  virtual void ExecuteInformation(vtkImageData **, vtkImageData **) {};
private:
  vtkImageMultipleInputOutputFilter(const vtkImageMultipleInputOutputFilter&);  // Not implemented.
  void operator=(const vtkImageMultipleInputOutputFilter&);  // Not implemented.
};

#endif







