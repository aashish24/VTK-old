/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkImageButterworthHighPass - Frequency domain high pass.
// .SECTION Description
// This filter only works on an image after it has been converted to
// frequency domain by a vtkImageFFT filter.  A vtkImageRFFT filter
// can be used to convert the output back into the spatial domain.
// vtkImageButterworthHighPass  the frequency components around 0 are
// attenuated.  Input and output are in floats, with two components
// (complex numbers).
// out(i, j) = 1 / (1 + pow(CutOff/Freq(i,j), 2*Order));

// .SECTION See Also
// vtkImageButterworthLowPass

#ifndef __vtkImageButterworthHighPass_h
#define __vtkImageButterworthHighPass_h


#include "vtkImageToImageFilter.h"

class VTK_IMAGING_EXPORT vtkImageButterworthHighPass : public vtkImageToImageFilter
{
public:
  static vtkImageButterworthHighPass *New();
  vtkTypeRevisionMacro(vtkImageButterworthHighPass,vtkImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the cutoff frequency for each axis.
  // The values are specified in the order X, Y, Z, Time.
  // Units: Cycles per world unit (as defined by the data spacing).
  vtkSetVector3Macro(CutOff,float);
  void SetCutOff(float v) {this->SetCutOff(v, v, v);}
  void SetXCutOff(float v);
  void SetYCutOff(float v);
  void SetZCutOff(float v);
  vtkGetVector3Macro(CutOff,float);
  float GetXCutOff() {return this->CutOff[0];}
  float GetYCutOff() {return this->CutOff[1];}
  float GetZCutOff() {return this->CutOff[2];}

  // Description:
  // The order determines sharpness of the cutoff curve.
  vtkSetMacro(Order, int);
  vtkGetMacro(Order, int);
  
protected:
  vtkImageButterworthHighPass();
  ~vtkImageButterworthHighPass() {};

  int Order;
  float CutOff[3];
  
  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData,
                       int outExt[6], int id);
private:
  vtkImageButterworthHighPass(const vtkImageButterworthHighPass&);  // Not implemented.
  void operator=(const vtkImageButterworthHighPass&);  // Not implemented.
};

#endif



