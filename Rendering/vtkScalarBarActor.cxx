/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 1993-2000 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkScalarBarActor.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkScalarBarActor* vtkScalarBarActor::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkScalarBarActor");
  if(ret)
    {
    return (vtkScalarBarActor*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkScalarBarActor;
}




// Instantiate object with 64 maximum colors; 5 labels; font size 12
// of font Arial (bolding, italic, shadows on); %%-#6.3g label
// format, no title, and vertical orientation. The initial scalar bar
// size is (0.05 x 0.8) of the viewport size.
vtkScalarBarActor::vtkScalarBarActor()
{
  this->LookupTable = NULL;
  this->Position2Coordinate = vtkCoordinate::New();
  this->Position2Coordinate->SetCoordinateSystemToNormalizedViewport();
  this->Position2Coordinate->SetValue(0.17, 0.8);
  this->Position2Coordinate->SetReferenceCoordinate(this->PositionCoordinate);
  
  this->PositionCoordinate->SetCoordinateSystemToNormalizedViewport();
  this->PositionCoordinate->SetValue(0.82,0.1);
  
  this->MaximumNumberOfColors = 64;
  this->NumberOfLabels = 5;
  this->NumberOfLabelsBuilt = 0;
  this->Orientation = VTK_ORIENT_VERTICAL;
  this->Title = NULL;

  this->Bold = 1;
  this->Italic = 1;
  this->Shadow = 1;
  this->FontFamily = VTK_ARIAL;
  this->LabelFormat = new char[8]; 
  sprintf(this->LabelFormat,"%s","%-#6.3g");

  this->TitleMapper = vtkTextMapper::New();
  this->TitleMapper->SetJustificationToCentered();
  this->TitleActor = vtkActor2D::New();
  this->TitleActor->SetMapper(this->TitleMapper);
  this->TitleActor->GetPositionCoordinate()->
    SetReferenceCoordinate(this->PositionCoordinate);
  
  this->TextMappers = NULL;
  this->TextActors = NULL;

  this->ScalarBar = vtkPolyData::New();
  this->ScalarBarMapper = vtkPolyDataMapper2D::New();
  this->ScalarBarMapper->SetInput(this->ScalarBar);
  this->ScalarBarActor = vtkActor2D::New();
  this->ScalarBarActor->SetMapper(this->ScalarBarMapper);
  this->ScalarBarActor->GetPositionCoordinate()->
    SetReferenceCoordinate(this->PositionCoordinate);
  this->LastOrigin[0] = 0;
  this->LastOrigin[1] = 0;
  this->LastSize[0] = 0;
  this->LastSize[1] = 0;
}

// Release any graphics resources that are being consumed by this actor.
// The parameter window could be used to determine which graphic
// resources to release.
void vtkScalarBarActor::ReleaseGraphicsResources(vtkWindow *win)
{
  this->TitleActor->ReleaseGraphicsResources(win);
  if (this->TextMappers != NULL )
    {
    for (int i=0; i < this->NumberOfLabelsBuilt; i++)
      {
      this->TextActors[i]->ReleaseGraphicsResources(win);
      }
    }
  this->ScalarBarActor->ReleaseGraphicsResources(win);
}


vtkScalarBarActor::~vtkScalarBarActor()
{
  this->Position2Coordinate->Delete();
  this->Position2Coordinate = NULL;
  
  if (this->LabelFormat) 
    {
    delete [] this->LabelFormat;
    this->LabelFormat = NULL;
    }

  this->TitleMapper->Delete();
  this->TitleActor->Delete();

  if (this->TextMappers != NULL )
    {
    for (int i=0; i < this->NumberOfLabelsBuilt; i++)
      {
      this->TextMappers[i]->Delete();
      this->TextActors[i]->Delete();
      }
    delete [] this->TextMappers;
    delete [] this->TextActors;
    }

  this->ScalarBar->Delete();
  this->ScalarBarMapper->Delete();
  this->ScalarBarActor->Delete();

  if (this->Title)
    {
    delete [] this->Title;
    this->Title = NULL;
    }
  
  this->SetLookupTable(NULL);
}

void vtkScalarBarActor::SetWidth(float w)
{
  float *pos;

  pos = this->Position2Coordinate->GetValue();
  this->Position2Coordinate->SetCoordinateSystemToNormalizedViewport();
  this->Position2Coordinate->SetValue(w,pos[1]);
}

void vtkScalarBarActor::SetHeight(float w)
{
  float *pos;

  pos = this->Position2Coordinate->GetValue();
  this->Position2Coordinate->SetCoordinateSystemToNormalizedViewport();
  this->Position2Coordinate->SetValue(pos[0],w);
}
    
float vtkScalarBarActor::GetWidth()
{
  return this->Position2Coordinate->GetValue()[0];
}
float vtkScalarBarActor::GetHeight()
{
  return this->Position2Coordinate->GetValue()[1];
}

int vtkScalarBarActor::RenderOverlay(vtkViewport *viewport)
{
  int renderedSomething = 0;
  int i;
  
  // Everything is built, just have to render
  if (this->Title != NULL)
    {
    renderedSomething += this->TitleActor->RenderOverlay(viewport);
    }
  this->ScalarBarActor->RenderOverlay(viewport);
  if( this->TextActors == NULL)
    {
     vtkWarningMacro(<<"Need a mapper to render a scalar bar");
     return renderedSomething;
    }
  
  for (i=0; i<this->NumberOfLabels; i++)
    {
    renderedSomething += this->TextActors[i]->RenderOverlay(viewport);
    }

  renderedSomething = (renderedSomething > 0)?(1):(0);

  return renderedSomething;
}

int vtkScalarBarActor::RenderOpaqueGeometry(vtkViewport *viewport)
{
  int renderedSomething = 0;
  int i;
  int size[2];
  int stringHeight, stringWidth;
  int fontSize;
  int titleHeight;
  
  if ( ! this->LookupTable )
    {
    vtkWarningMacro(<<"Need a mapper to render a scalar bar");
    return 0;
    }

  // Check to see whether we have to rebuild everything
  if ( viewport->GetMTime() > this->BuildTime || 
       ( viewport->GetVTKWindow() && 
	 viewport->GetVTKWindow()->GetMTime() > this->BuildTime ) )
    {
    // if the viewport has changed we may - or may not need
    // to rebuild, it depends on if the projected coords chage
    int *barOrigin;
    barOrigin = this->PositionCoordinate->GetComputedViewportValue(viewport);
    size[0] = 
      this->Position2Coordinate->GetComputedViewportValue(viewport)[0] -
      barOrigin[0];
    size[1] = 
      this->Position2Coordinate->GetComputedViewportValue(viewport)[1] -
      barOrigin[1];
    if (this->LastSize[0] != size[0] || this->LastSize[1] != size[1] ||
	this->LastOrigin[0] != barOrigin[0] || 
	this->LastOrigin[1] != barOrigin[1])
      {
      this->Modified();
      }
    }
  
  // Check to see whether we have to rebuild everything
  if ( this->GetMTime() > this->BuildTime || 
       this->LookupTable->GetMTime() > this->BuildTime )
    {
    vtkDebugMacro(<<"Rebuilding subobjects");

    // Delete previously constructed objects
    //
    if (this->TextMappers != NULL )
      {
      for (i=0; i < this->NumberOfLabelsBuilt; i++)
	{
	this->TextMappers[i]->Delete();
	this->TextActors[i]->Delete();
	}
      delete [] this->TextMappers;
      delete [] this->TextActors;
      }

    // Build scalar bar object
    //
    vtkScalarsToColors *lut = this->LookupTable;
    // we hard code how many steps to display
    int numColors = this->MaximumNumberOfColors;
    float *range = lut->GetRange();

    int numPts = 2*(numColors + 1);
    vtkPoints *pts = vtkPoints::New();
    pts->SetNumberOfPoints(numPts);
    vtkCellArray *polys = vtkCellArray::New();
    polys->Allocate(polys->EstimateSize(numColors,4));
    vtkScalars *colors = vtkScalars::New(VTK_UNSIGNED_CHAR,3);
    colors->SetNumberOfScalars(numColors);
    vtkUnsignedCharArray *colorData = (vtkUnsignedCharArray *)colors->GetData();

    this->ScalarBarActor->SetProperty(this->GetProperty());
    this->ScalarBar->Initialize();
    this->ScalarBar->SetPoints(pts);
    this->ScalarBar->SetPolys(polys);
    this->ScalarBar->GetCellData()->SetScalars(colors);
    pts->Delete(); polys->Delete(); colors->Delete();

    // get the viewport size in display coordinates
    int *barOrigin, barWidth, barHeight;
    barOrigin = this->PositionCoordinate->GetComputedViewportValue(viewport);
    size[0] = 
      this->Position2Coordinate->GetComputedViewportValue(viewport)[0] -
      barOrigin[0];
    size[1] = 
      this->Position2Coordinate->GetComputedViewportValue(viewport)[1] -
      barOrigin[1];
    this->LastOrigin[0] = barOrigin[0];
    this->LastOrigin[1] = barOrigin[1];
    this->LastSize[0] = size[0];
    this->LastSize[1] = size[1];
    
    // Update all the composing objects
    //
    if (this->Title == NULL )
      {
      this->TitleActor->VisibilityOff();
      }
    this->TitleActor->VisibilityOn();
    this->TitleActor->SetProperty(this->GetProperty());
    this->TitleMapper->SetInput(this->Title);
    this->TitleMapper->SetBold(this->Bold);
    this->TitleMapper->SetItalic(this->Italic);
    this->TitleMapper->SetShadow(this->Shadow);
    this->TitleMapper->SetFontFamily(this->FontFamily);

    
    // find the best size for the font
    int tempi[2];
    int target;
    if ( this->Orientation == VTK_ORIENT_VERTICAL )
      {
      target = (int)(size[1]*0.05 + 0.05*size[0]);
      }
    else
      {
      target = (int)(size[1]*0.07 + size[0]*0.03);
      }
    if (this->Title == NULL || (strlen(this->Title) == 0))
      {
      // dummy up a title to force a fontsize calculation (the same font
      // size is used for title and ticks, so if we dummy up a title, the
      // font size selected for the ticks will be same regardless of whether
      // a title is supplied or not)
      this->TitleMapper->SetInput("foo");
      }
    fontSize = target;
    this->TitleMapper->SetFontSize(fontSize);
    this->TitleMapper->GetSize(viewport,tempi);

    while (tempi[1] < target*this->TitleMapper->
           GetNumberOfLines(this->TitleMapper->GetInput()) && 
           fontSize < 100 ) 
      {
      fontSize++;
      this->TitleMapper->SetFontSize(fontSize);
      this->TitleMapper->GetSize(viewport,tempi);
      }
    
    while ((tempi[1] > target*this->TitleMapper->
            GetNumberOfLines(this->TitleMapper->GetInput()) ||
            tempi[0] > size[0] )
           && fontSize > 0 )
      {
      fontSize--;
      this->TitleMapper->SetFontSize(fontSize);
      this->TitleMapper->GetSize(viewport,tempi);
      }
    
    
    stringHeight = tempi[1];
    if (this->Title == NULL || (strlen(this->Title) == 0))
      {
      // if no title was originally specified, then we used a dummy
      // title above to force a font size selection.  We need to reset
      // the title and indicate that no space is needed for the title
      this->TitleMapper->SetInput( this->Title );
      stringHeight = 0;
      }
      
    this->TextMappers = new vtkTextMapper * [this->NumberOfLabels];
    this->TextActors = new vtkActor2D * [this->NumberOfLabels];
    char string[512];
    float val;
    for (i=0; i < this->NumberOfLabels; i++)
      {
      this->TextMappers[i] = vtkTextMapper::New();
      val = range[0] + (float)i/(this->NumberOfLabels-1) * (range[1]-range[0]);
      sprintf(string, this->LabelFormat, val);
      this->TextMappers[i]->SetInput(string);
      this->TextMappers[i]->SetFontSize(fontSize);
      this->TextMappers[i]->SetBold(this->Bold);
      this->TextMappers[i]->SetItalic(this->Italic);
      this->TextMappers[i]->SetShadow(this->Shadow);
      this->TextMappers[i]->SetFontFamily(this->FontFamily);
      this->TextActors[i] = vtkActor2D::New();
      this->TextActors[i]->SetMapper(this->TextMappers[i]);
      this->TextActors[i]->SetProperty(this->GetProperty());
      this->TextActors[i]->GetPositionCoordinate()->
	SetReferenceCoordinate(this->PositionCoordinate);
      }
    
    this->NumberOfLabelsBuilt = this->NumberOfLabels;

    // generate points
    float x[3]; x[2] = 0.0;
    float delta;
    if ( this->Orientation == VTK_ORIENT_VERTICAL )
      {
      // need to find maximum width
      stringWidth = 0;
      for (i=0; i < this->NumberOfLabels; i++)
	{
	this->TextMappers[i]->GetSize(viewport,tempi);
	if (stringWidth < tempi[0])
	  {
	  stringWidth = tempi[0];
	  }
	}
      barWidth = size[0] - 4 - stringWidth;
      barHeight = (int)(size[1] - stringHeight*2.2);
      titleHeight = stringHeight + 
        (stringHeight / this->TitleMapper->
         GetNumberOfLines(this->TitleMapper->GetInput()) ) * 1.2; 
      barHeight = (int)(size[1] - titleHeight);
      delta=(float)barHeight/numColors;
      for (i=0; i<numPts/2; i++)
	{
	x[0] = 0;
	x[1] = i*delta;
        pts->SetPoint(2*i,x);
	x[0] = barWidth;
        pts->SetPoint(2*i+1,x);
	}
      }
    else
      {
      barWidth = size[0];
      titleHeight = stringHeight + 
        (stringHeight / this->TitleMapper->
         GetNumberOfLines(this->TitleMapper->GetInput()) ) * 1.6; 
      barHeight = (int)(size[1] - titleHeight);
      delta=(float)barWidth/numColors;
      for (i=0; i<numPts/2; i++)
	{
	x[0] = i*delta;
	x[1] = barHeight;
        pts->SetPoint(2*i,x);
	x[1] = 0;
        pts->SetPoint(2*i+1,x);
	}
      }

    //polygons & cell colors
    unsigned char *rgba, *rgb;
    int ptIds[4];
    for (i=0; i<numColors; i++)
      {
      ptIds[0] = 2*i;
      ptIds[1] = ptIds[0] + 1;
      ptIds[2] = ptIds[1] + 2;
      ptIds[3] = ptIds[0] + 2;
      polys->InsertNextCell(4,ptIds);

      rgba = lut->MapValue(range[0] + (range[1] - range[0])*
                           ((float)i /(numColors-1.0)));
      rgb = colorData->GetPointer(3*i); //write into array directly
      rgb[0] = rgba[0];
      rgb[1] = rgba[1];
      rgb[2] = rgba[2];
      }

    // Now position everything properly
    //
    if (this->Orientation == VTK_ORIENT_VERTICAL)
      {
      int sizeTextData[2];
      
      // center the title
      this->TitleActor->SetPosition(size[0]/2, size[1] - stringHeight);
      
      for (i=0; i < this->NumberOfLabels; i++)
	{
	val = (float)i/(this->NumberOfLabels-1) *barHeight;
	this->TextMappers[i]->SetJustificationToLeft();
        this->TextMappers[i]->GetSize(viewport,sizeTextData);
        this->TextActors[i]->SetPosition(barWidth+3,
                                         val - sizeTextData[1]/2);
	}
      }
    else
      {
      this->TitleActor->SetPosition(size[0]/2, size[1] - stringHeight);
      for (i=0; i < this->NumberOfLabels; i++)
	{
	val = (float)i/(this->NumberOfLabels-1) * barWidth;
	this->TextMappers[i]->SetJustificationToCentered();
	this->TextActors[i]->SetPosition(val, barHeight + 0.2*stringHeight);
	}
      }

    this->BuildTime.Modified();
    }

  // Everything is built, just have to render
  if (this->Title != NULL)
    {
    renderedSomething += this->TitleActor->RenderOpaqueGeometry(viewport);
    }
  this->ScalarBarActor->RenderOpaqueGeometry(viewport);
  for (i=0; i<this->NumberOfLabels; i++)
    {
    renderedSomething += this->TextActors[i]->RenderOpaqueGeometry(viewport);
    }

  renderedSomething = (renderedSomething > 0)?(1):(0);

  return renderedSomething;
}

void vtkScalarBarActor::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkActor2D::PrintSelf(os,indent);

  if ( this->LookupTable )
    {
    os << indent << "Lookup Table:\n";
    this->LookupTable->PrintSelf(os,indent.GetNextIndent());
    }
  else
    {
    os << indent << "Lookup Table: (none)\n";
    }

  os << indent << "Title: " << (this->Title ? this->Title : "(none)") << "\n";
  os << indent << "Maximum Number Of Colors: " 
     << this->MaximumNumberOfColors << "\n";
  os << indent << "Number Of Labels: " << this->NumberOfLabels << "\n";
  os << indent << "Number Of Labels Built: " << this->NumberOfLabelsBuilt << "\n";

  os << indent << "Orientation: ";
  if ( this->Orientation == VTK_ORIENT_HORIZONTAL )
    {
    os << "Horizontal\n";
    }
  else
    {
    os << "Vertical\n";
    }

  os << indent << "Font Family: ";
  if ( this->FontFamily == VTK_ARIAL )
    {
    os << "Arial\n";
    }
  else if ( this->FontFamily == VTK_COURIER )
    {
    os << "Courier\n";
    }
  else
    {
    os << "Times\n";
    }

  os << indent << "Bold: " << (this->Bold ? "On\n" : "Off\n");
  os << indent << "Italic: " << (this->Italic ? "On\n" : "Off\n");
  os << indent << "Shadow: " << (this->Shadow ? "On\n" : "Off\n");
  os << indent << "Label Format: " << this->LabelFormat << "\n";
}

vtkCoordinate *vtkScalarBarActor::GetPosition2Coordinate() 
{ 
    vtkDebugMacro(<< this->GetClassName() << " (" << this << "): returning Position2Coordinate address " << this->Position2Coordinate ); 
    return this->Position2Coordinate; 
} 
void vtkScalarBarActor::SetPosition2(float x[2]) 
{
  this->SetPosition2(x[0],x[1]);
} 
void vtkScalarBarActor::SetPosition2(float x, float y) 
{ 
  this->Position2Coordinate->SetCoordinateSystem(VTK_VIEWPORT); 
  this->Position2Coordinate->SetValue(x,y); 
} 
float *vtkScalarBarActor::GetPosition2() 
{ 
  return this->Position2Coordinate->GetValue(); 
}

void vtkScalarBarActor::ShallowCopy(vtkProp *prop)
{
  vtkScalarBarActor *a = vtkScalarBarActor::SafeDownCast(prop);
  if ( a != NULL )
    {
    this->SetPosition2(a->GetPosition2());
    this->SetLookupTable(a->GetLookupTable());
    this->SetMaximumNumberOfColors(a->GetMaximumNumberOfColors());
    this->SetOrientation(a->GetOrientation());
    this->SetBold(a->GetBold());
    this->SetItalic(a->GetItalic());
    this->SetShadow(a->GetShadow());
    this->SetFontFamily(a->GetFontFamily());
    this->SetLabelFormat(a->GetLabelFormat());
    this->SetTitle(a->GetTitle());

    this->GetPositionCoordinate()->SetCoordinateSystem(
      a->GetPositionCoordinate()->GetCoordinateSystem());    
    this->GetPositionCoordinate()->SetValue(
      a->GetPositionCoordinate()->GetValue());
    this->GetPosition2Coordinate()->SetCoordinateSystem(
      a->GetPosition2Coordinate()->GetCoordinateSystem());    
    this->GetPosition2Coordinate()->SetValue(
      a->GetPosition2Coordinate()->GetValue());
    }

  // Now do superclass
  this->vtkActor2D::ShallowCopy(prop);
}
