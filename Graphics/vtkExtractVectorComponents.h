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
// .NAME vtkExtractVectorComponents - extract components of vector as separate scalars
// .SECTION Description
// vtkExtractVectorComponents is a filter that extracts vector components as
// separate scalars. This is accomplished by creating three different outputs.
// Each output is the same as the input, except that the scalar values will be
// one of the three components of the vector. These can be found in the
// VxComponent, VyComponent, and VzComponent.

// .SECTION Caveats
// This filter is unusual in that it creates multiple outputs. 
// If you use the GetOutput() method, you will be retrieving the x vector component.

#ifndef __vtkExtractVectorComponents_h
#define __vtkExtractVectorComponents_h

#include "vtkSource.h"
#include "vtkDataSet.h"

class VTK_EXPORT vtkExtractVectorComponents : public vtkSource
{
public:
  static vtkExtractVectorComponents *New() {
    return new vtkExtractVectorComponents;};
  const char *GetClassName() {return "vtkExtractVectorComponents";};

  // Description:
  // Specify the input data or filter.
  virtual void SetInput(vtkDataSet *input);

  // Description:
  // Get the input data or filter.
  vtkDataSet *GetInput();

  // Description:
  // Get the output dataset representing velocity x-component. If output is
  // NULL then input hasn't been set, which is necessary for abstract
  // objects. (Note: this method returns the same information as the
  // GetOutput() method with an index of 0.)
  vtkDataSet *GetVxComponent();

  // Description:
  // Get the output dataset representing velocity y-component. If output is
  // NULL then input hasn't been set, which is necessary for abstract
  // objects. (Note: this method returns the same information as the
  // GetOutput() method with an index of 1.)
  vtkDataSet *GetVyComponent();
  
  // Description:
  // Get the output dataset representing velocity z-component. If output is
  // NULL then input hasn't been set, which is necessary for abstract
  // objects. (Note: this method returns the same information as the
  // GetOutput() method with an index of 2.)
  vtkDataSet *GetVzComponent();

  // Description:
  // Get the output dataset containing the indicated component. The component
  // is specified by an index between (0,2) corresponding to the x, y, or z
  // vector component. By default, the x component is extracted.
  vtkDataSet *GetOutput(int i=0); //default extracts vector component.

  // Description:
  // For legacy compatibility. Do not use.
  void SetInput(vtkDataSet &input) {this->SetInput(&input);};

protected:
  vtkExtractVectorComponents();
  ~vtkExtractVectorComponents();
  vtkExtractVectorComponents(const vtkExtractVectorComponents&) {};
  void operator=(const vtkExtractVectorComponents&) {};

  void Execute();
  int ComputeInputUpdateExtents(vtkDataObject *output);
};

#endif


