/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to C. Charles Law who developed this class.

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
// .NAME vtkImageOpenClose3D - Will perform opening or closing.
// .SECTION Description
// vtkImageOpenClose3D performs opening or closing by having two 
// vtkImageErodeDilates in series.  The size of operation
// is determined by the method SetKernelSize, and the operator is an elispe.
// OpenValue and CloseValue determine how the filter behaves.  For binary
// images Opening and closing behaves as expected.
// Close value is first dilated, and then eroded.
// Open value is first eroded, and then dilated.
// Degenerate two dimesional opening/closing can be achieved by seting the
// one axis the 3D KernelSize to 1.
// Values other than open value and close value are not touched.
// This enables the filter to processes segemented images containing more than
// two tags.


#ifndef __vtkImageOpenClose3D_h
#define __vtkImageOpenClose3D_h


#include "vtkImageFilter.h"
#include "vtkImageDilateErode3D.h"

class VTK_EXPORT vtkImageOpenClose3D : public vtkImageFilter
{
public:
  // Description:
  // Default open value is 0, and default close value is 255.
  vtkImageOpenClose3D();

  ~vtkImageOpenClose3D();
  static vtkImageOpenClose3D *New() {return new vtkImageOpenClose3D;};
  const char *GetClassName() {return "vtkImageOpenClose3D";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // This method considers the sub filters MTimes when computing this objects
  // modified time.
  unsigned long int GetMTime();
  
  // Description:
  // Turn debugging output on. (in sub filters also)
  void DebugOn();
  void DebugOff();

  // Description:
  // Pass modified message to sub filters.
  void Modified();
  
  // Foward Source messages to filter1

  // Description:
  // This method sets the cache object of the filter.
  // It justs feeds the request to the sub filter.
  void SetCache(vtkImageCache *cache);
  
  // Description:
  // This method returns the l;ast cache of the internal pipline.
  vtkImageCache *GetCache();

  
  // Description:
  // This method returns the cache to make a connection
  // It justs feeds the request to the sub filter.
  vtkImageCache *GetOutput();

  // Description:
  // A method used internally, which is part of the vtkImageFilter API.
  // Return the maximum MTime of this filter, and all of the previous filters
  // in the pipline.
  unsigned long GetPipelineMTime();

  // Foward filter messages

  // Description:
  // Set the Input of the filter.
  void SetInput(vtkImageCache *Input);
  void SetInput(vtkStructuredPoints *spts)
    {vtkStructuredPointsToImage *tmp = spts->MakeStructuredPointsToImage();
    this->SetInput(tmp->GetOutput()); tmp->Delete();};

  // Forward dilateErode messages to both filters.

  // Description:
  // Selects the size of gaps or objects removed.
  void SetKernelSize(int size0, int size1, int size2);

  // Description:
  // Determines the value that will opened.  
  // Open value is first eroded, and then dilated.
  void SetOpenValue(float value);
  float GetOpenValue();

  // Description:
  // Determines the value that will closed.
  // Close value is first dilated, and then eroded
  void SetCloseValue(float value);
  float GetCloseValue();
  
  // Description:
  // Needed for Progress functions
  vtkGetObjectMacro(Filter0, vtkImageDilateErode3D);
  vtkGetObjectMacro(Filter1, vtkImageDilateErode3D);

protected:
  
  vtkImageDilateErode3D *Filter0;
  vtkImageDilateErode3D *Filter1;
};

#endif



