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
#include "vtkDataSetMapper.h"
#include "vtkPolyDataMapper.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkDataSetMapper* vtkDataSetMapper::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkDataSetMapper");
  if(ret)
    {
    return (vtkDataSetMapper*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkDataSetMapper;
}




vtkDataSetMapper::vtkDataSetMapper()
{
  this->GeometryExtractor = NULL;
  this->PolyDataMapper = NULL;
}

vtkDataSetMapper::~vtkDataSetMapper()
{
  // delete internally created objects.
  if ( this->GeometryExtractor )
    {
    this->GeometryExtractor->Delete();
    }
  if ( this->PolyDataMapper )
    {
    this->PolyDataMapper->Delete();
    }
}

void vtkDataSetMapper::SetInput(vtkDataSet *input)
{
  this->vtkProcessObject::SetNthInput(0, input);
}

vtkDataSet *vtkDataSetMapper::GetInput()
{
  if (this->NumberOfInputs < 1)
    {
    return NULL;
    }
  
  return (vtkDataSet *)(this->Inputs[0]);
}

void vtkDataSetMapper::ReleaseGraphicsResources( vtkWindow *renWin )
{
  if (this->PolyDataMapper)
    {
    this->PolyDataMapper->ReleaseGraphicsResources( renWin );
    }
}

//
// Receives from Actor -> maps data to primitives
//
void vtkDataSetMapper::Render(vtkRenderer *ren, vtkActor *act)
{
  //
  // make sure that we've been properly initialized
  //
  if ( !this->GetInput() )
    {
    vtkErrorMacro(<< "No input!\n");
    return;
    } 
  //
  // Need a lookup table
  //
  if ( this->LookupTable == NULL )
    {
    this->CreateDefaultLookupTable();
    }
  this->LookupTable->Build();
  //
  // Now can create appropriate mapper
  //
  if ( this->PolyDataMapper == NULL ) 
    {
    vtkGeometryFilter *gf = vtkGeometryFilter::New();
    vtkPolyDataMapper *pm = vtkPolyDataMapper::New();
    pm->SetInput(gf->GetOutput());

    this->GeometryExtractor = gf;
    this->PolyDataMapper = pm;
    }
  //
  // share clipping planes with the PolyDataMapper
  //
  if (this->ClippingPlanes != this->PolyDataMapper->GetClippingPlanes()) 
    {
    this->PolyDataMapper->SetClippingPlanes(this->ClippingPlanes);
    }
  //
  // For efficiency: if input type is vtkPolyData, there's no need to pass it thru
  // the geometry filter.
  //
  if ( this->GetInput()->GetDataObjectType() == VTK_POLY_DATA )
    {
    this->PolyDataMapper->SetInput((vtkPolyData *)(this->GetInput()));
    }
  else
    {
    this->GeometryExtractor->SetInput(this->GetInput());
    this->PolyDataMapper->SetInput(this->GeometryExtractor->GetOutput());
    }
  
  // update ourselves in case something has changed
  this->PolyDataMapper->SetLookupTable(this->GetLookupTable());
  this->PolyDataMapper->SetScalarVisibility(this->GetScalarVisibility());
  this->PolyDataMapper->SetScalarRange(this->GetScalarRange());
  this->PolyDataMapper->SetImmediateModeRendering
    (this->GetImmediateModeRendering());
  this->PolyDataMapper->SetColorMode(this->GetColorMode());
  this->PolyDataMapper->SetScalarMode(this->GetScalarMode());

  this->PolyDataMapper->Render(ren,act);

  this->TimeToDraw = this->PolyDataMapper->GetTimeToDraw();
}

void vtkDataSetMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMapper::PrintSelf(os,indent);

  if ( this->PolyDataMapper )
    {
    os << indent << "Poly Mapper: (" << this->PolyDataMapper << ")\n";
    }
  else
    {
    os << indent << "Poly Mapper: (none)\n";
    }

  if ( this->GeometryExtractor )
    {
    os << indent << "Geometry Extractor: (" << this->GeometryExtractor << ")\n";
    }
  else
    {
    os << indent << "Geometry Extractor: (none)\n";
    }
}

unsigned long vtkDataSetMapper::GetMTime()
{
  unsigned long mTime=this->vtkMapper::GetMTime();
  unsigned long time;

  if ( this->LookupTable != NULL )
    {
    time = this->LookupTable->GetMTime();
    mTime = ( time > mTime ? time : mTime );
    }

  return mTime;
}




