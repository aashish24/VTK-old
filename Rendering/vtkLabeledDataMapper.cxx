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
#include "vtkLabeledDataMapper.h"

#include "vtkActor2D.h"
#include "vtkDataArray.h"
#include "vtkDataSet.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkTextMapper.h"
#include "vtkTextProperty.h"

vtkCxxRevisionMacro(vtkLabeledDataMapper, "$Revision$");
vtkStandardNewMacro(vtkLabeledDataMapper);

vtkCxxSetObjectMacro(vtkLabeledDataMapper,Input, vtkDataSet);
vtkCxxSetObjectMacro(vtkLabeledDataMapper,LabelTextProperty,vtkTextProperty);

//----------------------------------------------------------------------------
// Creates a new label mapper

vtkLabeledDataMapper::vtkLabeledDataMapper()
{
  this->Input = NULL;
  this->LabelMode = VTK_LABEL_IDS;

  this->LabelFormat = new char[8]; 
  strcpy(this->LabelFormat,"%g");

  this->LabeledComponent = (-1);
  this->FieldDataArray = 0;
 
  this->NumberOfLabels = 0;
  this->NumberOfLabelsAllocated = 50;

  this->TextMappers = new vtkTextMapper * [this->NumberOfLabelsAllocated];
  for (int i=0; i<this->NumberOfLabelsAllocated; i++)
    {
    this->TextMappers[i] = vtkTextMapper::New();
    }

  this->LabelTextProperty = vtkTextProperty::New();
  this->LabelTextProperty->SetFontSize(12);
  this->LabelTextProperty->SetBold(1);
  this->LabelTextProperty->SetItalic(1);
  this->LabelTextProperty->SetShadow(1);
  this->LabelTextProperty->SetFontFamilyToArial();
}

//----------------------------------------------------------------------------
vtkLabeledDataMapper::~vtkLabeledDataMapper()
{
  if (this->LabelFormat)
    {
    delete [] this->LabelFormat;
    }

  if (this->TextMappers != NULL )
    {
    for (int i=0; i < this->NumberOfLabelsAllocated; i++)
      {
      this->TextMappers[i]->Delete();
      }
    delete [] this->TextMappers;
    }
  
  this->SetInput(NULL);
  this->SetLabelTextProperty(NULL);
}

//----------------------------------------------------------------------------
// Release any graphics resources that are being consumed by this actor.
// The parameter window could be used to determine which graphic
// resources to release.

void vtkLabeledDataMapper::ReleaseGraphicsResources(vtkWindow *win)
{
  if (this->TextMappers != NULL )
    {
    for (int i=0; i < this->NumberOfLabelsAllocated; i++)
      {
      this->TextMappers[i]->ReleaseGraphicsResources(win);
      }
    }
}

//----------------------------------------------------------------------------
void vtkLabeledDataMapper::RenderOverlay(vtkViewport *viewport, 
                                         vtkActor2D *actor)
{
  int i;
  float x[3];
  vtkDataSet *input=this->GetInput();

  if ( ! input )
    {
    vtkErrorMacro(<<"Need input data to render labels");
    return;
    }
  for (i=0; i<this->NumberOfLabels; i++)
    {
    this->Input->GetPoint(i,x);
    actor->GetPositionCoordinate()->SetCoordinateSystemToWorld();
    actor->GetPositionCoordinate()->SetValue(x);
    this->TextMappers[i]->RenderOverlay(viewport, actor);
    }
}

//----------------------------------------------------------------------------
void vtkLabeledDataMapper::RenderOpaqueGeometry(vtkViewport *viewport, 
                                                vtkActor2D *actor)
{
  int i, j, numComp = 0, pointIdLabels, activeComp = 0;
  char string[1024], format[1024];
  float val, x[3];
  vtkDataArray *data;
//   float *tuple=NULL;
  vtkDataSet *input=this->GetInput();
  vtkPointData *pd=input->GetPointData();

  if ( ! input )
    {
    vtkErrorMacro(<<"Need input data to render labels");
    return;
    }

  vtkTextProperty *tprop = this->LabelTextProperty;
  if (!tprop)
    {
    vtkErrorMacro(<<"Need text property to render labels");
    return;
    }

  input->Update();

  // Check to see whether we have to rebuild everything
  if ( this->GetMTime() > this->BuildTime || 
       input->GetMTime() > this->BuildTime ||
       tprop->GetMTime() > this->BuildTime)
    {
    vtkDebugMacro(<<"Rebuilding labels");

    // figure out what to label, and if we can label it
    pointIdLabels = 0;
    data = NULL;
    switch (this->LabelMode)
      {
      case VTK_LABEL_IDS:
        pointIdLabels = 1;
        break;
      case VTK_LABEL_SCALARS:
        if ( pd->GetScalars() )
          {
          data = pd->GetScalars();
          }
        break;
      case VTK_LABEL_VECTORS:   
        if ( pd->GetVectors() )
          {
          data = pd->GetVectors();
          }
        break;
      case VTK_LABEL_NORMALS:    
        if ( pd->GetNormals() )
          {
          data = pd->GetNormals();
          }
        break;
      case VTK_LABEL_TCOORDS:    
        if ( pd->GetTCoords() )
          {
          data = pd->GetTCoords();
          }
        break;
      case VTK_LABEL_TENSORS:    
        if ( pd->GetTensors() )
          {
          data = pd->GetTensors();
          }
        break;
      case VTK_LABEL_FIELD_DATA:
        int arrayNum = (this->FieldDataArray < pd->GetNumberOfArrays() ?
                        this->FieldDataArray : pd->GetNumberOfArrays() - 1);
        data = pd->GetArray(arrayNum);
        break;
      }

    // determine number of components and check input
    if ( pointIdLabels )
      {
      ;
      }
    else if ( data )
      {
      numComp = data->GetNumberOfComponents();
      activeComp = 0;
      if ( this->LabeledComponent >= 0 )
        {
        numComp = 1;
        activeComp = (this->LabeledComponent < numComp ? 
                      this->LabeledComponent : numComp - 1);
        }
      }
    else
      {
      vtkErrorMacro(<<"Need input data to render labels");
      return;
      }

    this->NumberOfLabels = this->Input->GetNumberOfPoints();
    if ( this->NumberOfLabels > this->NumberOfLabelsAllocated )
      {
      // delete old stuff
      for (i=0; i < this->NumberOfLabelsAllocated; i++)
        {
        this->TextMappers[i]->Delete();
        }
      delete [] this->TextMappers;

      this->NumberOfLabelsAllocated = this->NumberOfLabels;
      this->TextMappers = new vtkTextMapper * [this->NumberOfLabelsAllocated];
      for (i=0; i<this->NumberOfLabelsAllocated; i++)
        {
        this->TextMappers[i] = vtkTextMapper::New();
        }
      }//if we have to allocate new text mappers
    
    for (i=0; i < this->NumberOfLabels; i++)
      {
      if ( pointIdLabels )
        {
        val = (float)i;
        sprintf(string, this->LabelFormat, val);
        }
      else 
        {
        if ( numComp == 1)
          {
            if (data->GetDataType() == VTK_CHAR) 
              {
                if (strcmp(this->LabelFormat,"%c") != 0) {
                  vtkErrorMacro(<<"Label format must be %c to use with char");
                  return;
                }
                sprintf(string, this->LabelFormat, 
                        (char)data->GetComponent(i, activeComp));
              } else {
                sprintf(string, this->LabelFormat, 
                        data->GetComponent(i, activeComp));
              }
          }
        else
          {
          strcpy(format, "("); strcat(format, this->LabelFormat);
          for (j=0; j<(numComp-1); j++)
            {
            sprintf(string, format, data->GetComponent(i, j));
            strcpy(format,string); strcat(format,", ");
            strcat(format, this->LabelFormat);
            }
          sprintf(string, format, data->GetComponent(i, numComp-1));
          strcat(string, ")");
          }
        }
      this->TextMappers[i]->SetInput(string);
      this->TextMappers[i]->SetTextProperty(tprop);
      }

    this->BuildTime.Modified();
    }

  for (i=0; i<this->NumberOfLabels; i++)
    {
    this->Input->GetPoint(i,x);
    actor->GetPositionCoordinate()->SetCoordinateSystemToWorld();
    actor->GetPositionCoordinate()->SetValue(x);
    this->TextMappers[i]->RenderOpaqueGeometry(viewport, actor);
    }
}

//----------------------------------------------------------------------------
void vtkLabeledDataMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  if ( this->Input )
    {
    os << indent << "Input: (" << this->Input << ")\n";
    }
  else
    {
    os << indent << "Input: (none)\n";
    }

  if (this->LabelTextProperty)
    {
    os << indent << "Label Text Property:\n";
    this->LabelTextProperty->PrintSelf(os,indent.GetNextIndent());
    }
  else
    {
    os << indent << "Label Text Property: (none)\n";
    }

  os << indent << "Label Mode: ";
  if ( this->LabelMode == VTK_LABEL_IDS )
    {
    os << "Label Ids\n";
    }
  else if ( this->LabelMode == VTK_LABEL_SCALARS )
    {
    os << "Label Scalars\n";
    }
  else if ( this->LabelMode == VTK_LABEL_VECTORS )
    {
    os << "Label Vectors\n";
    }
  else if ( this->LabelMode == VTK_LABEL_NORMALS )
    {
    os << "Label Normals\n";
    }
  else if ( this->LabelMode == VTK_LABEL_TCOORDS )
    {
    os << "Label TCoords\n";
    }
  else if ( this->LabelMode == VTK_LABEL_TENSORS )
    {
    os << "Label Tensors\n";
    }
  else
    {
    os << "Label Field Data\n";
    }

  os << indent << "Label Format: " << this->LabelFormat << "\n";

  os << indent << "Labeled Component: ";
  if ( this->LabeledComponent < 0 )
    {
    os << "(All Components)\n";
    }
  else
    {
    os << this->LabeledComponent << "\n";
    }

  os << indent << "Field Data Array: " << this->FieldDataArray << "\n";
}

//----------------------------------------------------------------------------
// Backward compatibility calls

#ifndef VTK_REMOVE_LEGACY_CODE
void vtkLabeledDataMapper::SetFontFamily(int val) 
{ 
  VTK_LEGACY_METHOD(SetFontFamily, "4.2");
  if (this->LabelTextProperty)
    {
    this->LabelTextProperty->SetFontFamily(val); 
    }
}
#endif

#ifndef VTK_REMOVE_LEGACY_CODE
int vtkLabeledDataMapper::GetFontFamily()
{ 
  VTK_LEGACY_METHOD(GetFontFamily, "4.2");
  if (this->LabelTextProperty)
    {
    return this->LabelTextProperty->GetFontFamily(); 
    }
  else
    {
    return 0;
    }
}
#endif

#ifndef VTK_REMOVE_LEGACY_CODE
void vtkLabeledDataMapper::SetFontSize(int size) 
{ 
  VTK_LEGACY_METHOD(SetFontSize, "4.2");
  if (this->LabelTextProperty)
    {
    this->LabelTextProperty->SetFontSize(size); 
    }
}
#endif

#ifndef VTK_REMOVE_LEGACY_CODE
int vtkLabeledDataMapper::GetFontSize()
{ 
  VTK_LEGACY_METHOD(GetFontSize, "4.2");
  if (this->LabelTextProperty)
    {
    return this->LabelTextProperty->GetFontSize(); 
    }
  else
    {
    return 0;
    }
}
#endif
  
#ifndef VTK_REMOVE_LEGACY_CODE
void vtkLabeledDataMapper::SetBold(int val)
{ 
  VTK_LEGACY_METHOD(SetBold, "4.2");
  if (this->LabelTextProperty)
    {
    this->LabelTextProperty->SetBold(val); 
    }
}
#endif

#ifndef VTK_REMOVE_LEGACY_CODE
int vtkLabeledDataMapper::GetBold()
{ 
  VTK_LEGACY_METHOD(GetBold, "4.2");
  if (this->LabelTextProperty)
    {
    return this->LabelTextProperty->GetBold(); 
    }
  else
    {
    return 0;
    }
}
#endif
  
#ifndef VTK_REMOVE_LEGACY_CODE
void vtkLabeledDataMapper::SetItalic(int val)
{ 
  VTK_LEGACY_METHOD(SetItalic, "4.2");
  if (this->LabelTextProperty)
    {
    this->LabelTextProperty->SetItalic(val); 
    }
}
#endif

#ifndef VTK_REMOVE_LEGACY_CODE
int vtkLabeledDataMapper::GetItalic()
{ 
  VTK_LEGACY_METHOD(GetItalic, "4.2");
  if (this->LabelTextProperty)
    {
    return this->LabelTextProperty->GetItalic(); 
    }
  else
    {
    return 0;
    }
}
#endif

#ifndef VTK_REMOVE_LEGACY_CODE
void vtkLabeledDataMapper::SetShadow(int val)
{ 
  VTK_LEGACY_METHOD(SetShadow, "4.2");
  if (this->LabelTextProperty)
    {
    this->LabelTextProperty->SetShadow(val); 
    }
}
#endif

#ifndef VTK_REMOVE_LEGACY_CODE
int vtkLabeledDataMapper::GetShadow()
{ 
  VTK_LEGACY_METHOD(GetShadow, "4.2");
  if (this->LabelTextProperty)
    {
    return this->LabelTextProperty->GetShadow(); 
    }
  else
    {
    return 0;
    }
}
#endif
