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
// .NAME vtkImageMaskBits - applies a bit-mask pattern to each component.
//
// .SECTION Description
// vtkImageMaskBits applies a bit-mask pattern to each component.  The bit-mask can be
// applied using a variety of boolean bitwise operators.


#ifndef __vtkImageMaskBits_h
#define __vtkImageMaskBits_h


#include "vtkImageToImageFilter.h"
#include "vtkImageLogic.h"     // included for the boolean operators

class VTK_IMAGING_EXPORT vtkImageMaskBits : public vtkImageToImageFilter
{
public:
  static vtkImageMaskBits *New();
  vtkTypeRevisionMacro(vtkImageMaskBits,vtkImageToImageFilter);
  void PrintSelf(ostream &, vtkIndent);
  
  // Description:
  // Set/Get the bit-masks. Default is 0xffffffff.
  vtkSetVector4Macro(Masks, unsigned int);
  void SetMask(unsigned int  mask)
    {this->SetMasks(mask, mask, mask, mask);}
  void SetMasks(unsigned int  mask1, unsigned int mask2)
    {this->SetMasks(mask1, mask2, 0xffffffff,  0xffffffff);}
  void SetMasks(unsigned int  mask1, unsigned int mask2, unsigned int mask3)
    {this->SetMasks(mask1, mask2, mask3,  0xffffffff);}
  vtkGetVector4Macro(Masks, unsigned int);

  // Description:
  // Set/Get the boolean operator. Default is AND.
  vtkSetMacro(Operation,int);
  vtkGetMacro(Operation,int);
  void SetOperationToAnd() {this->SetOperation(VTK_AND);};
  void SetOperationToOr() {this->SetOperation(VTK_OR);};
  void SetOperationToXor() {this->SetOperation(VTK_XOR);};
  void SetOperationToNand() {this->SetOperation(VTK_NAND);};
  void SetOperationToNor() {this->SetOperation(VTK_NOR);};
   
  
protected:
  vtkImageMaskBits();
  ~vtkImageMaskBits() {};

  void ThreadedExecute(vtkImageData *inData, vtkImageData *outData, 
                       int ext[6], int id);

  unsigned int Masks[4];
  int Operation;      
private:
  vtkImageMaskBits(const vtkImageMaskBits&);  // Not implemented.
  void operator=(const vtkImageMaskBits&);  // Not implemented.
};

#endif










