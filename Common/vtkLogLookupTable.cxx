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
#include "vtkLogLookupTable.h"
#include "vtkObjectFactory.h"



//------------------------------------------------------------------------------
vtkLogLookupTable* vtkLogLookupTable::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkLogLookupTable");
  if(ret)
    {
    return (vtkLogLookupTable*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkLogLookupTable;
}




// Construct with (minimum,maximum) range 1 to 10 (based on 
// logarithmic values).
vtkLogLookupTable::vtkLogLookupTable(int sze, int ext):
vtkLookupTable(sze,ext)
{
  this->LogMinRange = 0.0;
  this->LogMaxRange = 1.0;
  this->UseAbsoluteValue = 0;
}

// Set the minimum/maximum scalar values for scalar mapping. Scalar values
// less than minimum range value are clamped to minimum range value.
// Scalar values greater than maximum range value are clamped to maximum
// range value. (The log base 10 of these values is taken and mapping is
// performed in logarithmic space.)
void  vtkLogLookupTable::SetTableRange(float min, float max)
{
  if ( min >= max )
    {
    vtkErrorMacro (<<"Minimum value must be less than maximum value");
    return;
    }

  this->TableRange[0] = min;
  this->TableRange[1] = max;

  if ( max >= 0.0 && min <= 0.0 )
    {
    vtkErrorMacro(<<"Can't use logarithmic table on mixed negative/positive values");
    }
  else if ( max <= 0.0 ) // okay, all negative values
    {
    if ( max == 0.0 )
      {
      max = 1.0e-06 * (min - max);
      }
    this->UseAbsoluteValue = 1;
    this->LogMinRange = log10((double)(-min));
    this->LogMaxRange = log10((double)(-max));
    }
  else 
    {
    if ( min == 0.0 )
      {
      min = 1.0e-06 * (max - min);
      }
    this->UseAbsoluteValue = 0;
    this->LogMinRange = log10((double)min);
    this->LogMaxRange = log10((double)max);
    }
}

// Given a scalar value v, return an rgba color value from lookup table. 
// Mapping performed log base 10 (negative ranges are converted into positive
// values).
unsigned char *vtkLogLookupTable::MapValue(float v)
{
  int indx;

  if ( v < this->TableRange[0] )
    {
    v = this->TableRange[0];
    }
  else if ( v > this->TableRange[1] )
    {
    v = this->TableRange[1];
    }

  if ( this->UseAbsoluteValue )
    {
    indx = (int)( (log10((double)(-v)) - this->LogMinRange) /
                  (this->LogMaxRange - this->LogMinRange) * 
                  (this->NumberOfColors-1) );
    }
  else
    {
    indx = (int)( (log10((double)v) - this->LogMinRange) /
                  (this->LogMaxRange - this->LogMinRange) * 
                  (this->NumberOfColors-1) );
    }

  return this->Table->GetPointer(4*indx);
}

template <class T>
static void vtkLogLookupTableMapDataToRGBA(vtkLogLookupTable *self,
				    T *input, unsigned char *output,
				    int i, int incr) 
{
  unsigned char *cptr;
  
  while (--i >= 0) 
    {
    cptr = self->MapValue(*input);
    *output++ = *cptr++;
    *output++ = *cptr++;
    *output++ = *cptr++;
    *output++ = *cptr++;
    input += incr;
    }
}

void vtkLogLookupTable::MapScalarsThroughTable2(void *input, 
					       unsigned char *output,
					       int inputDataType, 
					       int numberOfValues,
					       int inputIncrement)
{
  switch (inputDataType)
    {
    case VTK_CHAR:
      vtkLogLookupTableMapDataToRGBA(this,(char *)input,output,numberOfValues,inputIncrement);
      break;
      
    case VTK_UNSIGNED_CHAR:
      vtkLogLookupTableMapDataToRGBA(this,(unsigned char *)input,output,numberOfValues,inputIncrement);
      break;
      
    case VTK_SHORT:
      vtkLogLookupTableMapDataToRGBA(this,(short *)input,output,numberOfValues,inputIncrement);
      break;
      
    case VTK_UNSIGNED_SHORT:
      vtkLogLookupTableMapDataToRGBA(this,(unsigned short *)input,output,numberOfValues,inputIncrement);
      break;
      
    case VTK_INT:
      vtkLogLookupTableMapDataToRGBA(this,(int *)input,output,numberOfValues,inputIncrement);
      break;
      
    case VTK_UNSIGNED_INT:
      vtkLogLookupTableMapDataToRGBA(this,(unsigned int *)input,output,numberOfValues,inputIncrement);
      break;
      
    case VTK_LONG:
      vtkLogLookupTableMapDataToRGBA(this,(long *)input,output,numberOfValues,inputIncrement);
      break;
      
    case VTK_UNSIGNED_LONG:
      vtkLogLookupTableMapDataToRGBA(this,(unsigned long *)input,output,numberOfValues,inputIncrement);
      break;
      
    case VTK_FLOAT:
      vtkLogLookupTableMapDataToRGBA(this,(float *)input,output,numberOfValues,inputIncrement);
      break;
      
    case VTK_DOUBLE:
      vtkLogLookupTableMapDataToRGBA(this,(double *)input,output,numberOfValues,inputIncrement);
      break;
      
    default:
      vtkErrorMacro(<< "MapImageThroughTable: Unknown input ScalarType");
      return;
    }
}  

void vtkLogLookupTable::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkLookupTable::PrintSelf(os,indent);

  os << indent << "Log Min Range: " <<this->LogMinRange << "\n";
  os << indent << "Log Max Range: " <<this->LogMaxRange << "\n";
}
