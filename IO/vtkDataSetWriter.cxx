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
#include "vtkDataSetWriter.h"
#include "vtkPolyDataWriter.h"
#include "vtkStructuredPointsWriter.h"
#include "vtkStructuredGridWriter.h"
#include "vtkUnstructuredGridWriter.h"
#include "vtkRectilinearGridWriter.h"

// Specify the input data or filter.
void vtkDataSetWriter::SetInput(vtkDataSet *input)
{
  if ( this->Input != input )
    {
    vtkDebugMacro(<<" setting Input to " << (void *)input);
    if (this->Input) {this->Input->UnRegister(this);}
    this->Input = input;
    if (this->Input) {this->Input->Register(this);}
    this->Modified();
    }
}

void vtkDataSetWriter::WriteData()
{
  int type;
  vtkDataWriter *writer = NULL;

  vtkDebugMacro(<<"Writing vtk dataset...");

  type = ((vtkDataSet *)this->Input)->GetDataSetType();
  if ( type == VTK_POLY_DATA )
    {
    vtkPolyDataWriter *pwriter = vtkPolyDataWriter::New();
    pwriter->SetInput((vtkPolyData *)this->Input);
    writer = (vtkDataWriter *)pwriter;
    }

  else if ( type == VTK_STRUCTURED_POINTS )
    {
    vtkStructuredPointsWriter *spwriter = vtkStructuredPointsWriter::New();
    spwriter->SetInput((vtkStructuredPoints *)this->Input);
    writer = (vtkDataWriter *)spwriter;
    }

  else if ( type == VTK_STRUCTURED_GRID )
    {
    vtkStructuredGridWriter *sgwriter = vtkStructuredGridWriter::New();
    sgwriter->SetInput((vtkStructuredGrid *)this->Input);
    writer = (vtkDataWriter *)sgwriter;
    }

  else if ( type == VTK_UNSTRUCTURED_GRID )
    {
    vtkUnstructuredGridWriter *ugwriter = vtkUnstructuredGridWriter::New();
    ugwriter->SetInput((vtkUnstructuredGrid *)this->Input);
    writer = (vtkDataWriter *)ugwriter;
    }

  else if ( type == VTK_RECTILINEAR_GRID )
    {
    vtkRectilinearGridWriter *rgwriter = vtkRectilinearGridWriter::New();
    rgwriter->SetInput((vtkRectilinearGrid *)this->Input);
    writer = (vtkDataWriter *)rgwriter;
    }

  else
    {
    vtkErrorMacro(<< "Cannot write dataset type: " << type);
    return;
    }

  writer->SetFileName(this->FileName);
  writer->SetScalarsName(this->ScalarsName);
  writer->SetVectorsName(this->VectorsName);
  writer->SetNormalsName(this->NormalsName);
  writer->SetTensorsName(this->TensorsName);
  writer->SetTCoordsName(this->TCoordsName);
  writer->SetLookupTableName(this->LookupTableName);
  writer->SetFieldDataName(this->FieldDataName);
  writer->SetFileType(this->FileType);
  writer->SetDebug(this->Debug);
  writer->Write();
  writer->Delete();
}

void vtkDataSetWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataWriter::PrintSelf(os,indent);
}
