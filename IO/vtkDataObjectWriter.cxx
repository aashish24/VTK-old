/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen.

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
#include "vtkDataObjectWriter.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkDataObjectWriter* vtkDataObjectWriter::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkDataObjectWriter");
  if(ret)
    {
    return (vtkDataObjectWriter*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkDataObjectWriter;
}




vtkDataObjectWriter::vtkDataObjectWriter()
{
  this->Writer = vtkDataWriter::New();
}

vtkDataObjectWriter::~vtkDataObjectWriter()
{
  this->Writer->Delete();
}

//----------------------------------------------------------------------------
void vtkDataObjectWriter::SetInput(vtkDataObject *input)
{
  this->vtkProcessObject::SetNthInput(0, input);
}

//----------------------------------------------------------------------------
vtkDataObject *vtkDataObjectWriter::GetInput()
{
  if (this->NumberOfInputs < 1)
    {
    return NULL;
    }
  
  return (vtkDataObject *)(this->Inputs[0]);
}

// Write FieldData data to file
void vtkDataObjectWriter::WriteData()
{
  ostream *fp;
  vtkFieldData *f=this->GetInput()->GetFieldData();

  vtkDebugMacro(<<"Writing vtk FieldData data...");

  if ( !(fp=this->Writer->OpenVTKFile()) || !this->Writer->WriteHeader(fp) )
    {
    return;
    }
  //
  // Write FieldData data specific stuff
  //
  this->Writer->WriteFieldData(fp, f);
  
  this->Writer->CloseVTKFile(fp);  
}

void vtkDataObjectWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkWriter::PrintSelf(os,indent);
  
  os << indent << "File Name: " 
     << (this->Writer->GetFileName() ? this->Writer->GetFileName() : "(none)") << "\n";

  if ( this->Writer->GetFileType() == VTK_BINARY )
    {
    os << indent << "File Type: BINARY\n";
    }
  else
    {
    os << indent << "File Type: ASCII\n";
    }

  if ( this->Writer->GetHeader() )
    {
    os << indent << "Header: " << this->Writer->GetHeader() << "\n";
    }
  else
    {
    os << indent << "Header: (None)\n";
    }

  if ( this->Writer->GetFieldDataName() )
    {
    os << indent << "Field Data Name: " << this->Writer->GetFieldDataName() << "\n";
    }
  else
    {
    os << indent << "Field Data Name: (None)\n";
    }

}

