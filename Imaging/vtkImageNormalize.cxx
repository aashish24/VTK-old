/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkImageNormalize.h"
#include "vtkObjectFactory.h"
#include "vtkImageProgressIterator.h"

#include <math.h>

vtkCxxRevisionMacro(vtkImageNormalize, "$Revision$");
vtkStandardNewMacro(vtkImageNormalize);

//----------------------------------------------------------------------------
// This method tells the superclass that the first axis will collapse.
void vtkImageNormalize::ExecuteInformation(vtkImageData *vtkNotUsed(inData), 
                                           vtkImageData *outData)
{
  outData->SetScalarType(VTK_FLOAT);
}

//----------------------------------------------------------------------------
// This execute method handles boundaries.
// it handles boundaries. Pixels are just replicated to get values 
// out of extent.
template <class T>
void vtkImageNormalizeExecute(vtkImageNormalize *self,
                              vtkImageData *inData,
                              vtkImageData *outData,
                              int outExt[6], int id, T *)
{
  vtkImageIterator<T> inIt(inData, outExt);
  vtkImageProgressIterator<float> outIt(outData, outExt, self, id);
  int idxC, maxC;
  float sum;
  T *inVect;
  
  // find the region to loop over
  maxC = inData->GetNumberOfScalarComponents();

  // Loop through ouput pixels
  while (!outIt.IsAtEnd())
    {
    T* inSI = inIt.BeginSpan();
    float *outSI = outIt.BeginSpan();
    float *outSIEnd = outIt.EndSpan();
    while (outSI != outSIEnd)
      {
      // save the start of the vector
      inVect = inSI;
      
      // compute the magnitude.
      sum = 0.0;
      for (idxC = 0; idxC < maxC; idxC++)
        {
        sum += (float)(*inSI) * (float)(*inSI);
        inSI++;
        }
      if (sum > 0.0)
        {
        sum = 1.0 / sqrt(sum);
        }
      
      // now divide to normalize.
      for (idxC = 0; idxC < maxC; idxC++)
        {
        *outSI = (float)(*inVect) * sum;
        inVect++;
        outSI++;
        }
      }
    inIt.NextSpan();
    outIt.NextSpan();
    }
}


//----------------------------------------------------------------------------
// This method contains a switch statement that calls the correct
// templated function for the input data type.  The output data
// must match input type.  This method does handle boundary conditions.
void vtkImageNormalize::ThreadedExecute(vtkImageData *inData, 
                                        vtkImageData *outData,
                                        int outExt[6], int id)
{
  vtkDebugMacro(<< "Execute: inData = " << inData 
  << ", outData = " << outData);
  
  // this filter expects that input is the same type as output.
  if (outData->GetScalarType() != VTK_FLOAT)
    {
    vtkErrorMacro(<< "Execute: output ScalarType, " << outData->GetScalarType()
    << ", must be float");
    return;
    }
  
  switch (inData->GetScalarType())
    {
    vtkTemplateMacro6(vtkImageNormalizeExecute, this, inData,
                     outData, outExt, id, static_cast<VTK_TT *>(0));
    default:
      vtkErrorMacro(<< "Execute: Unknown ScalarType");
      return;
    }
}












