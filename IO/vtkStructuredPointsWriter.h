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
// .NAME vtkStructuredPointsWriter - write vtk structured points data file
// .SECTION Description
// vtkStructuredPointsWriter is a source object that writes ASCII or binary 
// structured points data in vtk file format. See text for format details.
// .SECTION Caveats
// Binary files written on one system may not be writable on other systems.

#ifndef __vtkStructuredPointsWriter_h
#define __vtkStructuredPointsWriter_h

#include "vtkDataWriter.h"
#include "vtkStructuredPoints.h"
#include "vtkImageToStructuredPoints.h"
#include "vtkImageCache.h"

class VTK_EXPORT vtkStructuredPointsWriter : public vtkDataWriter
{
public:
  vtkStructuredPointsWriter() {};
  static vtkStructuredPointsWriter *New() {return new vtkStructuredPointsWriter;};
  const char *GetClassName() {return "vtkStructuredPointsWriter";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify the input data or filter.
  void SetInput(vtkStructuredPoints *input);

  void SetInput(vtkStructuredPoints &input) {this->SetInput(&input);};
  void SetInput(vtkImageCache *cache)
    {this->SetInput(cache->GetImageToStructuredPoints()->GetOutput());}
  vtkStructuredPoints *GetInput() {return (vtkStructuredPoints *)this->Input;};
                               
protected:
  void WriteData();

};

#endif


