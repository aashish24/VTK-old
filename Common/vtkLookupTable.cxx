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
#include <math.h>
#include "vtkLookupTable.h"
#include "vtkScalars.h"

// Construct with range=(0,1); and hsv ranges set up for rainbow color table 
// (from red to blue).
vtkLookupTable::vtkLookupTable(int sze, int ext)
{
  this->NumberOfColors = sze;
  this->Table = vtkUnsignedCharArray::New();
  this->Table->SetNumberOfComponents(4);
  this->Table->Allocate(4*sze,4*ext);

  this->HueRange[0] = 0.0;
  this->HueRange[1] = 0.66667;

  this->SaturationRange[0] = 1.0;
  this->SaturationRange[1] = 1.0;

  this->TableRange[0] = 0.0;
  this->TableRange[1] = 1.0;

  this->ValueRange[0] = 1.0;
  this->ValueRange[1] = 1.0;

  this->AlphaRange[0] = 1.0;
  this->AlphaRange[1] = 1.0;
}

vtkLookupTable::~vtkLookupTable()
{
  this->Table->Delete();
  this->Table = NULL;
}


// Scalar values greater than maximum range value are clamped to maximum
// range value.
void  vtkLookupTable::SetTableRange(float r[2])
{
  this->SetTableRange(r[0],r[1]);
}

// Set the minimum/maximum scalar values for scalar mapping. Scalar values
// less than minimum range value are clamped to minimum range value.
// Scalar values greater than maximum range value are clamped to maximum
// range value.
void  vtkLookupTable
::SetTableRange(float min, float max)
{
  if ( min >= max )
    {
    vtkErrorMacro (<<"Bad table range: " << min << " >= " << max);
    return;
    }

  if (this->TableRange[0] != min || this->TableRange[1] != max)
    {
    this->TableRange[0] = min;
    this->TableRange[1] = max;
    this->Modified();
    }
}

// Allocate a color table of specified size.
int vtkLookupTable::Allocate(int sz, int ext) 
{
  this->Modified();
  this->NumberOfColors = sz;
  return this->Table->Allocate(4*this->NumberOfColors,4*ext);
}


// Generate lookup table from hue, saturation, value, alpha min/max values. 
// Table is built from linear ramp of each value.
void vtkLookupTable::Build()
{
  int i, hueCase;
  float hue, sat, val, lx, ly, lz, frac, hinc, sinc, vinc, ainc;
  float rgba[4], alpha;
  unsigned char *c_rgba;

  if ( this->Table->GetNumberOfTuples() < 1 ||
  (this->GetMTime() > this->BuildTime && this->InsertTime < this->BuildTime) )
    {
    hinc = (this->HueRange[1] - this->HueRange[0])/(this->NumberOfColors-1);
    sinc = (this->SaturationRange[1] - this->SaturationRange[0])/(this->NumberOfColors-1);
    vinc = (this->ValueRange[1] - this->ValueRange[0])/(this->NumberOfColors-1);
    ainc = (this->AlphaRange[1] - this->AlphaRange[0])/(this->NumberOfColors-1);

    for (i=0; i < this->NumberOfColors; i++) 
      {
      hue = this->HueRange[0] + i * hinc;
      sat = this->SaturationRange[0] + i * sinc;
      val = this->ValueRange[0] + i * vinc;
      alpha = this->AlphaRange[0] + i * ainc;

      hueCase = (int)(hue * 6);
      frac = 6*hue - hueCase;
      lx = val*(1.0 - sat);
      ly = val*(1.0 - sat*frac);
      lz = val*(1.0 - sat*(1.0 - frac));

      switch (hueCase) 
      {

        /* 0<hue<1/6 */
      case 0:
      case 6:
        rgba[0] = val;
        rgba[1] = lz;
        rgba[2] = lx;
        break;
        /* 1/6<hue<2/6 */
      case 1:
        rgba[0] = ly;
        rgba[1] = val;
        rgba[2] = lx;
        break;
        /* 2/6<hue<3/6 */
      case 2:
        rgba[0] = lx;
        rgba[1] = val;
        rgba[2] = lz;
        break;
        /* 3/6<hue/4/6 */
      case 3:
        rgba[0] = lx;
        rgba[1] = ly;
        rgba[2] = val;
        break;
        /* 4/6<hue<5/6 */
      case 4:
        rgba[0] = lz;
        rgba[1] = lx;
        rgba[2] = val;
        break;
        /* 5/6<hue<1 */
      case 5:
        rgba[0] = val;
        rgba[1] = lx;
        rgba[2] = ly;
        break;
      }

      c_rgba = this->Table->WritePointer(4*i,4);

      c_rgba[0] = (unsigned char) 
        ((float)127.5*(1.0+(float)cos((1.0-(double)rgba[0])*3.141593)));
      c_rgba[1] = (unsigned char)
        ((float)127.5*(1.0+(float)cos((1.0-(double)rgba[1])*3.141593)));
      c_rgba[2] = (unsigned char)
        ((float)127.5*(1.0+(float)cos((1.0-(double)rgba[2])*3.141593)));
      c_rgba[3] = (unsigned char) (alpha*255.0);
    }
  }
  this->BuildTime.Modified();
}

// Given a scalar value v, return an rgba color value from lookup table.
unsigned char *vtkLookupTable::MapValue(float v)
{
  float findx;
  float maxIndex = this->NumberOfColors-1;
  float shift = -this->TableRange[0];
  float scale = this->NumberOfColors/(this->TableRange[1]+shift);

  findx = (v+shift)*scale;
  if (findx < 0)
    {
    findx = 0;
    }
  if (findx > maxIndex)
    {
    findx = maxIndex;
    }
  return this->Table->GetPointer(4*(int)findx);
}

// accelerate the mapping by copying the data in 32-bit chunks instead
// of 8-bit chunks
template<class T>
static void vtkLookupTableMapDataToRGBA(vtkLookupTable *self, T *input, 
				 unsigned char *output, int length, int incr)
{
  float findx;
  int i = length;
  float *range = self->GetTableRange();
  float maxIndex = self->GetNumberOfColors()-1;
  float shift = -range[0];
  float scale = self->GetNumberOfColors()/(range[1]-range[0]);
  unsigned char *table = self->GetPointer(0);
  unsigned char *cptr;

  while (--i >= 0) 
    {
    findx = (*input + shift)*scale;
    if (findx < 0)
      {
      findx = 0;
      }
    if (findx > maxIndex)
      {
      findx = maxIndex;
      }
    cptr = &table[4*(int)findx];
    *output++ = *cptr++;
    *output++ = *cptr++;
    *output++ = *cptr++;
    *output++ = *cptr++;     
    input += incr;
    }
}

void vtkLookupTable::MapScalarsThroughTable2(void *input, 
					    unsigned char *output,
					    int inputDataType, 
					    int numberOfValues,
					    int inputIncrement)
{
  switch (inputDataType)
    {
    case VTK_CHAR:
      vtkLookupTableMapDataToRGBA(this,(char *)input,output,numberOfValues,inputIncrement);
      break;
      
    case VTK_UNSIGNED_CHAR:
      vtkLookupTableMapDataToRGBA(this,(unsigned char *)input,output,numberOfValues,inputIncrement);
      break;
      
    case VTK_SHORT:
      vtkLookupTableMapDataToRGBA(this,(short *)input,output,numberOfValues,inputIncrement);
      break;
      
    case VTK_UNSIGNED_SHORT:
      vtkLookupTableMapDataToRGBA(this,(unsigned short *)input,output,numberOfValues,inputIncrement);
      break;
      
    case VTK_INT:
      vtkLookupTableMapDataToRGBA(this,(int *)input,output,numberOfValues,inputIncrement);
      break;
      
    case VTK_UNSIGNED_INT:
      vtkLookupTableMapDataToRGBA(this,(unsigned int *)input,output,numberOfValues,inputIncrement);
      break;
      
    case VTK_LONG:
      vtkLookupTableMapDataToRGBA(this,(long *)input,output,numberOfValues,inputIncrement);
      break;
      
    case VTK_UNSIGNED_LONG:
      vtkLookupTableMapDataToRGBA(this,(unsigned long *)input,output,numberOfValues,inputIncrement);
      break;
      
    case VTK_FLOAT:
      vtkLookupTableMapDataToRGBA(this,(float *)input,output,numberOfValues,inputIncrement);
      break;
      
    case VTK_DOUBLE:
      vtkLookupTableMapDataToRGBA(this,(double *)input,output,numberOfValues,inputIncrement);
      break;
      
    default:
      vtkErrorMacro(<< "MapImageThroughTable: Unknown input ScalarType");
      return;
    }
}  

// Specify the number of values (i.e., colors) in the lookup
// table. This method simply allocates memory and prepares the table
// for use with SetTableValue(). It differs from Build() method in
// that the allocated memory is not initialized according to HSVA ramps.
void vtkLookupTable::SetNumberOfTableValues(int number)
{
  number = (number < 0 ? 1 : (number > 65536 ? 65536 : number));
  this->Table->SetNumberOfTuples(number);
}

// Directly load color into lookup table. Use [0,1] float values for color
// component specification. Make sure that you've either used the
// Build() method or used SetNumberOfTableValues() prior to using this method.
void vtkLookupTable::SetTableValue (int indx, float rgba[4])
{
  unsigned char *_rgba;

  _rgba = this->Table->WritePointer(4*indx,4);
  for (int i=0; i<4; i++)
    {
    _rgba[i] = (unsigned char) ((float)255.0 * rgba[i]);
    }

  indx = (indx < 0 ? 0 : (indx >= this->NumberOfColors ? this->NumberOfColors-1 : indx));
  this->InsertTime.Modified();
  this->Modified();
}

// Directly load color into lookup table. Use [0,1] float values for color 
// component specification.
void vtkLookupTable::SetTableValue(int indx, float r, float g, float b, float a)
{
  float rgba[4];
  rgba[0] = r; rgba[1] = g; rgba[2] = b; rgba[3] = a;
  this->SetTableValue(indx,rgba);
}

// Return a rgba color value for the given index into the lookup Table. Color
// components are expressed as [0,1] float values.
float *vtkLookupTable::GetTableValue (int indx)
{
  unsigned char *_rgba;

  indx = (indx < 0 ? 0 : (indx >= this->NumberOfColors ? this->NumberOfColors-1 : indx));
  _rgba = this->Table->GetPointer(indx*4);

  // For some strange reason the Sun compiler crashes on the following 
  // line with a -O5 optimization level 
  // for (int i=0; i<4; i++) rgba[i] = _rgba[i] / 255.0;

  this->RGBA[0] = _rgba[0] / 255.0;
  this->RGBA[1] = _rgba[1] / 255.0;
  this->RGBA[2] = _rgba[2] / 255.0;
  this->RGBA[3] = _rgba[3] / 255.0;

  return this->RGBA;
}

// Return a rgba color value for the given index into the lookup table. Color
// components are expressed as [0,1] float values.
void vtkLookupTable::GetTableValue (int indx, float rgba[4])
{
  float *_rgba = this->GetTableValue(indx);

  for (int i=0; i<4; i++)
    {
    rgba[i] = _rgba[i];
    }
}

void vtkLookupTable::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkScalarsToColors::PrintSelf(os,indent);

  os << indent << "Build Time: " <<this->BuildTime.GetMTime() << "\n";
  os << indent << "Table Range: (" << this->TableRange[0] << ", "
     << this->TableRange[1] << ")\n";
  os << indent << "Hue Range: (" << this->HueRange[0] << ", "
     << this->HueRange[1] << ")\n";
  os << indent << "Insert Time: " <<this->InsertTime.GetMTime() << "\n";
  os << indent << "Number Of Colors: " << this->GetNumberOfColors() << "\n";
  os << indent << "Saturation Range: (" << this->SaturationRange[0] << ", "
     << this->SaturationRange[1] << ")\n";
  os << indent << "Value Range: (" << this->ValueRange[0] << ", "
     << this->ValueRange[1] << ")\n";
  os << indent << "Alpha Range: (" << this->AlphaRange[0] << ", "
     << this->AlphaRange[1] << ")\n";
}
