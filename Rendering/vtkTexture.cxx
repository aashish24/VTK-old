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
#include <stdlib.h>
#include "vtkTexture.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkGraphicsFactory.h"

// Construct object and initialize.
vtkTexture::vtkTexture()
{
  this->Repeat = 1;
  this->Interpolate = 0;

  this->Input = NULL;
  this->LookupTable = NULL;
  this->MappedScalars = NULL;
  this->MapColorScalarsThroughLookupTable = 0;

  this->SelfAdjustingTableRange = 0;
}

vtkTexture::~vtkTexture()
{
  if (this->MappedScalars)
    {
    this->MappedScalars->Delete();
    }

  if (this->LookupTable != NULL) 
    {
    this->LookupTable->UnRegister(this);
    }
  if (this->Input) 
    {
    this->Input->UnRegister(this);
    this->Input = NULL;
    }
}

// return the correct type of Texture 
vtkTexture *vtkTexture::New()
{  
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkGraphicsFactory::CreateInstance("vtkTexture");
  return (vtkTexture*)ret;
}

void vtkTexture::SetLookupTable(vtkLookupTable *lut)
{
  if ( this->LookupTable != lut ) 
    {
    if ( this->LookupTable != NULL ) {this->LookupTable->UnRegister(this);}
    this->LookupTable = lut;
    if ( this->LookupTable != NULL ) {this->LookupTable->Register(this);}
    this->Modified();
    }
}

void vtkTexture::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkObject::PrintSelf(os,indent);

  os << indent << "Interpolate: " << (this->Interpolate ? "On\n" : "Off\n");
  os << indent << "Repeat:      " << (this->Repeat ? "On\n" : "Off\n");
  os << indent << "MapColorScalarsThroughLookupTable: " << (this->MapColorScalarsThroughLookupTable  ? "On\n" : "Off\n");

  if ( this->Input )
    {
    os << indent << "Input: (" << (void *)this->Input << ")\n";
    }
  else
    {
    os << indent << "Input: (none)\n";
    }
  if ( this->LookupTable )
    {
    os << indent << "LookupTable:\n";
    this->LookupTable->PrintSelf (os, indent.GetNextIndent ());
    }
  else
    {
    os << indent << "LookupTable: (none)\n";
    }

  if ( this->MappedScalars )
    {
    os << indent << "Mapped Scalars: " << this->MappedScalars << "\n";
    }
  else
    {
    os << indent << "Mapped Scalars: (none)\n";
    }
}

unsigned char *vtkTexture::MapScalarsToColors (vtkScalars *scalars)
{
  int numPts = scalars->GetNumberOfScalars ();
  vtkScalars *mappedScalars;

  // if there is no lookup table, create one
  if (this->LookupTable == NULL)
    {
    this->LookupTable = vtkLookupTable::New();
    this->LookupTable->Register(this);
    this->LookupTable->Delete();
    this->LookupTable->Build ();
    this->SelfAdjustingTableRange = 1;
    }
  // if there is no pixmap, create one
  if (!this->MappedScalars)
    {
    this->MappedScalars = vtkScalars::New(VTK_UNSIGNED_CHAR,4);
    }      
  
  // if the texture created its own lookup table, set the Table Range
  // to the range of the scalar data.
  if (this->SelfAdjustingTableRange) 
    {
    this->LookupTable->SetTableRange (scalars->GetRange());
    }
  
  // map the scalars to colors
  mappedScalars = this->MappedScalars;
  mappedScalars->SetNumberOfScalars(numPts);
  unsigned char *cptr = (unsigned char *)mappedScalars->GetVoidPointer(0);

  this->LookupTable->MapScalarsThroughTable(scalars,cptr);
  
  return cptr;
}

void vtkTexture::Render(vtkRenderer *ren)
{
  if (this->Input) //load texture map
    {
    this->Input->Update();
    this->Load(ren);
    }
}







