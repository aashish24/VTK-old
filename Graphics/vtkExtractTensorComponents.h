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
// .NAME vtkExtractTensorComponents - extract parts of tensor and create a scalar, vector, normal, or texture coordinates.
// .SECTION Description
// vtkExtractTensorComponents is a filter that extracts components of
// a tensor to create a scalar, vector, normal, or texture coords. For
// example, if the tensor contains components of stress, then you
// could extract the normal stress in the x-direction as a scalar
// (i.e., tensor component (0,0).
// 
// To use this filter, you must set some boolean flags to control
// which data is extracted from the tensors, and whether you want to
// pass the tensor data through to the output. Also, you must specify
// the tensor component(s) for each type of data you want to
// extract. The tensor component(s) is(are) specified using matrix notation
// into a 3x3 matrix. That is, use the (row,column) address to specify
// a particular tensor component; and if the data you are extracting
// requires more than one component, use a list of addresses. (Note
// that the addresses are 0-offset -> (0,0) specifies upper left
// corner of the tensor.)
//
// There are two optional methods to extract scalar data. You can
// extract the determinant of the tensor, or you can extract the
// effective stress of the tensor. These require that the ivar
// ExtractScalars is on, and the appropriate scalr extraction mode is
// set.

#ifndef __vtkExtractTensorComponents_h
#define __vtkExtractTensorComponents_h

#include "vtkDataSetToDataSetFilter.h"

#define VTK_EXTRACT_COMPONENT 0
#define VTK_EXTRACT_EFFECTIVE_STRESS 1
#define VTK_EXTRACT_DETERMINANT 2

class VTK_EXPORT vtkExtractTensorComponents : public vtkDataSetToDataSetFilter 
{
public:
  vtkExtractTensorComponents();
  static vtkExtractTensorComponents *New() {return new vtkExtractTensorComponents;};
  const char *GetClassName() {return "vtkExtractTensorComponents";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Boolean controls whether tensor data is passed through to output.
  vtkSetMacro(PassTensorsToOutput,int);
  vtkGetMacro(PassTensorsToOutput,int);
  vtkBooleanMacro(PassTensorsToOutput,int);

  // Description:
  // Boolean controls whether scalar data is extracted from tensor.
  vtkSetMacro(ExtractScalars,int);
  vtkGetMacro(ExtractScalars,int);
  vtkBooleanMacro(ExtractScalars,int);

  // Description:
  // Specify the (row,column) tensor component to extract as a scalar.
  vtkSetVectorMacro(ScalarComponents,int,2);
  vtkGetVectorMacro(ScalarComponents,int,2);

  // Description:
  // Specify how to extract the scalar. You can extract it as one of
  // the components of the tensor, as effective stress, or as the
  // determinant of the tensor. If you extract a copmponent make sure
  // that you set the ScalarComponents ivar.
  vtkSetMacro(ScalarMode,int);
  vtkGetMacro(ScalarMode,int);
  void ScalarIsComponent() {this->SetScalarMode(VTK_EXTRACT_COMPONENT);};
  void ScalarIsEffectiveStress() {this->SetScalarMode(VTK_EXTRACT_EFFECTIVE_STRESS);};
  void ScalarIsDeterminant() {this->SetScalarMode(VTK_EXTRACT_DETERMINANT);};

  // Description:
  // Boolean controls whether vector data is extracted from tensor.
  vtkSetMacro(ExtractVectors,int);
  vtkGetMacro(ExtractVectors,int);
  vtkBooleanMacro(ExtractVectors,int);

  // Description:
  // Specify the ((row,column)0,(row,column)1,(row,column)2) tensor
  // components to extract as a vector.
  vtkSetVectorMacro(VectorComponents,int,6);
  vtkGetVectorMacro(VectorComponents,int,6);


  // Description:
  // Boolean controls whether normal data is extracted from tensor.
  vtkSetMacro(ExtractNormals,int);
  vtkGetMacro(ExtractNormals,int);
  vtkBooleanMacro(ExtractNormals,int);

  // Description:
  // Boolean controls whether normal vector is converted to unit normal
  // after extraction.
  vtkSetMacro(NormalizeNormals,int);
  vtkGetMacro(NormalizeNormals,int);
  vtkBooleanMacro(NormalizeNormals,int);

  // Description:
  // Specify the ((row,column)0,(row,column)1,(row,column)2) tensor
  // components to extract as a vector.
  vtkSetVectorMacro(NormalComponents,int,6);
  vtkGetVectorMacro(NormalComponents,int,6);


  // Description:
  // Boolean controls whether texture coordinates are extracted from tensor.
  vtkSetMacro(ExtractTCoords,int);
  vtkGetMacro(ExtractTCoords,int);
  vtkBooleanMacro(ExtractTCoords,int);

  // Description:
  // Set the dimension of the texture coordinates to extract.
  vtkSetClampMacro(NumberOfTCoords,int,1,3);
  vtkGetMacro(NumberOfTCoords,int);

  // Description:
  // Specify the ((row,column)0,(row,column)1,(row,column)2) tensor
  // components to extract as a vector. Up to NumberOfTCoords
  // components are extracted.
  vtkSetVectorMacro(TCoordComponents,int,6);
  vtkGetVectorMacro(TCoordComponents,int,6);

protected:
  void Execute();

  int PassTensorsToOutput;

  int ExtractScalars;
  int ExtractVectors;
  int ExtractNormals;
  int ExtractTCoords;

  int ScalarMode;
  int ScalarComponents[2];

  int VectorComponents[6];

  int NormalizeNormals;
  int NormalComponents[6];

  int NumberOfTCoords;
  int TCoordComponents[6];

};

#endif

