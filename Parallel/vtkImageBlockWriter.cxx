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
#include "vtkImageBlockWriter.h"
#include "vtkImageClip.h"
#include "vtkStructuredPointsWriter.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkImageBlockWriter, "$Revision$");
vtkStandardNewMacro(vtkImageBlockWriter);

//----------------------------------------------------------------------------
vtkImageBlockWriter::vtkImageBlockWriter()
{
  this->FilePattern = NULL;
  this->Divisions[0] = this->Divisions[1] = this->Divisions[2] = 1;
  this->Overlap = 0;
}

//----------------------------------------------------------------------------
vtkImageBlockWriter::~vtkImageBlockWriter()
{
  this->SetFilePattern(NULL);
}

//----------------------------------------------------------------------------
void vtkImageBlockWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "FilePattern: " << this->FilePattern << endl;
  os << indent << "Overlap: " << this->Overlap << endl;
  os << indent << "Divisions: " << this->Divisions[0] << ", "
     << this->Divisions[1] << ", " << this->Divisions[2] << endl;
}


//----------------------------------------------------------------------------
void vtkImageBlockWriter::SetInput(vtkImageData *input)
{
  this->vtkProcessObject::SetNthInput(0, input);
}


//----------------------------------------------------------------------------
vtkImageData *vtkImageBlockWriter::GetInput()
{
  if (this->NumberOfInputs < 1)
    {
    return NULL;
    }
  
  return (vtkImageData *)(this->Inputs[0]);
}


//----------------------------------------------------------------------------
void vtkImageBlockWriter::Write()
{
  vtkImageData *input = this->GetInput();
  vtkStructuredPointsWriter *writer;
  vtkImageClip *clip;
  char *fileName;
  int i, j, k, temp;
  int *wholeExt, extent[6];

  if (input == NULL)
    {
    vtkErrorMacro("No Input");
    return;
    }
  if (this->FilePattern == NULL)
    {
    vtkErrorMacro("No FilePattern");
    return;
    }

  // Allocate a string for the filename.
  fileName = new char[strlen(this->FilePattern) + 50];

  // Create a writer to do the work.
  clip = vtkImageClip::New();
  clip->ClipDataOn();
  clip->SetInput(input);
  writer = vtkStructuredPointsWriter::New();
  writer->SetInput(clip->GetOutput());
  writer->SetFileTypeToBinary();

  // We need the whole extent.
  input->UpdateInformation();
  wholeExt = input->GetWholeExtent();

  for (k = 0; k < this->Divisions[2]; ++k)
    {
    for (j = 0; j < this->Divisions[1]; ++j)
      {
      for (i = 0; i < this->Divisions[0]; ++i)
        {
        // Compute the filename.
        sprintf(fileName, this->FilePattern, i, j, k);
        writer->SetFileName(fileName);
        // Compute the extent of this block.
        // X
        temp = wholeExt[1] - wholeExt[0] + 1 
                + (this->Divisions[0]-1)*this->Overlap;
        extent[0] = wholeExt[0] + i*temp/this->Divisions[0]
                - i*this->Overlap;
        extent[1] = (wholeExt[0] + (i+1)*temp/this->Divisions[0]) - 1
                - i*this->Overlap;
        // Y
        temp = wholeExt[3] - wholeExt[2] + 1 
                + (this->Divisions[1]-1)*this->Overlap;
        extent[2] = wholeExt[2] + j*temp/this->Divisions[1]
                - j*this->Overlap;
        extent[3] = (wholeExt[2] + (j+1)*temp/this->Divisions[1]) - 1
                - j*this->Overlap;
        // Z
        temp = wholeExt[5] - wholeExt[4] + 1 
                + (this->Divisions[2]-1)*this->Overlap;
        extent[4] = wholeExt[4] + k*temp/this->Divisions[2]
                - k*this->Overlap;
        extent[5] = (wholeExt[4] + (k+1)*temp/this->Divisions[2]) - 1
                - k*this->Overlap;

        clip->SetOutputWholeExtent(extent);

        vtkDebugMacro("writing block " << fileName
            << ": extent " << extent[0] << ", " << extent[1] << ", "
            << extent[2] << ", " << extent[3] << ", "
            << extent[4] << ", " << extent[5]);


        writer->Write();
        }
      }
    }

  // clean up
  writer->Delete();
  clip->Delete();
  delete fileName;
}




