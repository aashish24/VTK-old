/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 1993-1995 Ken Martin, Will Schroeder, Bill Lorensen.

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
#include "vtkImageData.h"

//----------------------------------------------------------------------------
// Description:
// Construct an instance of vtkImageData with no data.
vtkImageData::vtkImageData()
{
  int idx;

  this->Scalars = NULL;
  this->Allocated = 0;
  this->Type = VTK_IMAGE_VOID;
  this->PrintScalars = 0;
  for (idx = 0; idx < VTK_IMAGE_DIMENSIONS; ++idx)
    {
    this->Axes[idx] = idx;
    this->Increments[idx] = 0;
    this->Bounds[idx*2] = 0;
    this->Bounds[idx*2 + 1] = 0;
    }
}

//----------------------------------------------------------------------------
vtkImageData::~vtkImageData()
{
  if (this->Scalars)
    {
    this->Scalars->Delete();
    this->Scalars = NULL;
    }
}


//----------------------------------------------------------------------------
// A templated function to print different types of scalars.
template <class T>
void vtkImageDataPrintScalars(vtkImageData *self, T *ptr,
			      ostream& os, vtkIndent indent)
{
  int precisionSave = os.precision();
  int *temp;
  int idx0, idx1, idx2, idx3;
  int inc0, inc1, inc2, inc3;
  int min0, max0, min1, max1, min2, max2, min3, max3;
  T *ptr0, *ptr1, *ptr2, *ptr3;
  vtkIndent indent0, indent1, indent2, indent3;

  // Only print float values to 2 decimals
  os.precision(2);
  
  temp = self->GetIncrements();
  inc0 = temp[0];  inc1 = temp[1];  inc2 = temp[2];  inc3 = temp[2]; 
  temp = self->GetBounds();
  min0 = temp[0]; max0 = temp[1];  min1 = temp[2]; max1 = temp[3]; 
  min2 = temp[4]; max2 = temp[5];  min3 = temp[6]; max3 = temp[7]; 
  temp = self->GetAxes();
  
  indent3 = indent;
  ptr3 = ptr;
  if (max3 > min3)
    {
    os << indent3 << vtkImageAxisNameMacro(temp[3]) 
       << " range:(" << min3 << ", " << max3 << "), coordinant: (0, 0, 0, 0"
       << ")###########################\n";
    }
  for (idx3 = min3; idx3 <= max3; ++idx3)
    {
    indent2 = indent3.GetNextIndent();
    ptr2 = ptr3;
    if (max2 > min2)
      {
      os << indent2 << vtkImageAxisNameMacro(temp[2]) 
	 << " range:(" << min2 << ", " << max2 << "), coordinant: (0, 0, 0, "
	 << ", " << idx3 << ")===========================\n";
      }
    for (idx2 = min2; idx2 <= max2; ++idx2)
      {
      indent1 = indent2.GetNextIndent();
      ptr1 = ptr2;
      if (max1 > min1)
	{
	os << indent1 << vtkImageAxisNameMacro(temp[1]) 
	   << " range:(" << min1 << ", " << max1 << "), coordinant: (0, 0, "
	   << idx2 << ", " << idx3 << ")---------------------------\n";
	}
      for (idx1 = min1; idx1 <= max1; ++idx1)
	{

	indent0 = indent1.GetNextIndent();
	ptr0 = ptr1;
	os << indent0 << vtkImageAxisNameMacro(temp[0]) << ": " 
	   << (float)(*ptr0);
	ptr0 += inc0;
	for (idx0 = min0+1; idx0 <= max0; ++idx0)
	  {
	  os << ", " << (float)(*ptr0);
	  ptr0 += inc0;
	  }
	os << "\n";
	
	ptr1 += inc1;
	}

      ptr2 += inc2;
      }

    ptr3 += inc3;
    }
  
  // Set the precision value back to its original value.
  os.precision(precisionSave);
}




//----------------------------------------------------------------------------
void vtkImageData::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkIndent nextIndent = indent.GetNextIndent();
  
  vtkRefCount::PrintSelf(os,indent);
  os << indent << "Type: " << vtkImageDataTypeNameMacro(this->Type) << "\n";
  
  os << indent << "Axes: (";
  os << vtkImageAxisNameMacro(this->Axes[0]) << ", ";
  os << vtkImageAxisNameMacro(this->Axes[1]) << ", ";
  os << vtkImageAxisNameMacro(this->Axes[2]) << ", ";
  os << vtkImageAxisNameMacro(this->Axes[3]) << ")\n";
  
  os << indent << "Bounds: (";
  os << this->Bounds[0] << ", " << this->Bounds[1] << ", ";
  os << this->Bounds[2] << ", " << this->Bounds[3] << ", ";
  os << this->Bounds[4] << ", " << this->Bounds[5] << ", ";
  os << this->Bounds[6] << ", " << this->Bounds[7] << ")\n";

  os << indent << "Increments: (";
  os << this->Increments[0] << ", ";
  os << this->Increments[1] << ", ";
  os << this->Increments[2] << ", ";
  os << this->Increments[3] << ")\n";

  if ( ! this->Scalars)
    {
    os << indent << "Scalars: NULL\n";
    }
  else
    {
    os << indent << "Scalars:\n";
    this->Scalars->PrintSelf(os,nextIndent);
    // Adding this onto scalars (but in this class).
    if ( this->PrintScalars) 
      {
      void *ptr = this->GetVoidPointer();
      os << nextIndent << "Scalar Values:\n";
      switch (this->GetType())
	{
	case VTK_IMAGE_FLOAT:
	  vtkImageDataPrintScalars(this, (float *)(ptr), os, nextIndent);
	  break;
	case VTK_IMAGE_INT:
	  vtkImageDataPrintScalars(this, (int *)(ptr), os, nextIndent);
	  break;
	case VTK_IMAGE_SHORT:
	  vtkImageDataPrintScalars(this, (short *)(ptr), os, nextIndent);
	  break;
	case VTK_IMAGE_UNSIGNED_SHORT:
	  vtkImageDataPrintScalars(this, (unsigned short *)(ptr), 
				   os, nextIndent);
	  break;
	case VTK_IMAGE_UNSIGNED_CHAR:
	  vtkImageDataPrintScalars(this, (unsigned char *)(ptr),
				   os, nextIndent);
	  break;
	default:
	  os << nextIndent << "Cannot handle DataType.\n";
	}         
      }
    }
}



//----------------------------------------------------------------------------
// Description:
// This method sets the bounds of the data, and 
// should be called before the data object is allocated.
void vtkImageData::SetBounds(int min0, int max0, int min1, int max1, 
			     int min2, int max2, int min3, int max3,
			     int min4, int max4)
{
  vtkDebugMacro(<< "SetBounds: ...");

  if (this->GetRefCount() > 1)
    {
    vtkWarningMacro(<< "SetBounds: This object has more than one reference!");
    }
  
  if (this->Scalars)
    {
    vtkErrorMacro(<< "SetBounds: Data object has already been allocated.");
    return;
    }
  
  this->Modified();
  this->Bounds[0] = min0;
  this->Bounds[1] = max0;
  this->Bounds[2] = min1;
  this->Bounds[3] = max1;
  this->Bounds[4] = min2;
  this->Bounds[5] = max2;
  this->Bounds[6] = min3;
  this->Bounds[7] = max3;
  this->Bounds[8] = min4;
  this->Bounds[9] = max4;
}

//----------------------------------------------------------------------------
// Description:
// This Method translates the bounds of the data without modifying the data
// itself.  The result is to change the origin of the data.
void vtkImageData::Translate(int vector[VTK_IMAGE_DIMENSIONS])
{
  int idx;
  
  if (this->GetRefCount() > 1)
    {
    vtkWarningMacro(<< "Translate: This object has more than one reference!");
    }
  
  for (idx = 0; idx < VTK_IMAGE_DIMENSIONS; ++idx)
    {
    this->Bounds[idx*2] += vector[idx];
    this->Bounds[1+idx*2] += vector[idx];
    }
}



//----------------------------------------------------------------------------
// Description:
// This method tells the data object to handle a specific DataType.
// The method should be called before the data object is allocated.
void vtkImageData::SetType(int type)
{
  if (this->GetRefCount() > 1)
    {
    vtkWarningMacro(<< "SetType: This object has more than one reference!");
    }
  
  if (this->Scalars)
    {
    vtkErrorMacro(<< "SetType: Data object has already been allocated.");
    return;
    }
  
  this->Modified();
  this->Type = type;
}


//----------------------------------------------------------------------------
// Description:
// This method tells the data object how to order the axes in memory.
// It cannot be called after the object has been allocated.
void vtkImageData::SetAxes(int *axes)
{
  int idx;
  
  if (this->GetRefCount() > 1)
    {
    vtkWarningMacro(<< "SetAxes: This object has more than one reference!");
    }
  
  if (this->Scalars)
    {
    vtkErrorMacro(<< "SetAxes: Data object has already been allocated.");
    return;
    }
  
  this->Modified();
  for (idx = 0; idx < VTK_IMAGE_DIMENSIONS; ++idx)
    {
    this->Axes[idx] = axes[idx];
    }
}



//----------------------------------------------------------------------------
// Description:
// This method returns 1 if the data object has already been allocated.
int vtkImageData::IsAllocated()
{
  if (this->Scalars)
    return 1;
  else
    return 0;
}



//----------------------------------------------------------------------------
// Description:
// This method allocates memory for the vtkImageData data.  The size of
// the data object should be set before this method is called.
// The method returns 1 if the allocation was sucessful, 0 if not.
int vtkImageData::Allocate()
{
  int idx, inc = 1;

  if (this->GetRefCount() > 1)
    {
    vtkWarningMacro(<< "Allocate: This object has more than one reference!");
    }
  
  // delete previous data
  // in the future try to reuse memory
  if (this->Scalars)
    {
    this->Scalars->Delete();
    this->Scalars = NULL;
    }
  
  // set up increments
  for (idx = 0; idx < VTK_IMAGE_DIMENSIONS; ++idx)
    {
    this->Increments[idx] = inc;
    inc *= (this->Bounds[idx*2+1] - this->Bounds[idx*2] + 1);
    }
  
  // special case zero length array
  if (inc <= 0)
    {
    this->Scalars = NULL;
    return 1;
    }
  
  // create the Scalars object.
  switch (this->Type)
    {
    case VTK_IMAGE_VOID:
      vtkErrorMacro(<< "Allocate: Type Unknown");
      break;
    case VTK_IMAGE_FLOAT:
      this->Scalars = new vtkFloatScalars;
      this->Allocated = this->Scalars->Allocate(inc);
      ((vtkFloatScalars *)(this->Scalars))->WritePtr(0,inc);
      break;
    case VTK_IMAGE_INT:
      this->Scalars = new vtkIntScalars;
      this->Allocated = this->Scalars->Allocate(inc);
      ((vtkIntScalars *)(this->Scalars))->WritePtr(0,inc);
      break;
    case VTK_IMAGE_SHORT:
      this->Scalars = new vtkShortScalars;
      this->Allocated =  this->Scalars->Allocate(inc);
      ((vtkShortScalars *)(this->Scalars))->WritePtr(0,inc);
      break;
    case VTK_IMAGE_UNSIGNED_SHORT:
      this->Scalars = new vtkUnsignedShortScalars;
      this->Allocated = this->Scalars->Allocate(inc);
      ((vtkUnsignedShortScalars *)(this->Scalars))->WritePtr(0,inc);
      break;
    case VTK_IMAGE_UNSIGNED_CHAR:
      this->Scalars = new vtkUnsignedCharScalars;
      this->Allocated = this->Scalars->Allocate(inc);
      ((vtkUnsignedCharScalars *)(this->Scalars))->WritePtr(0,inc);
      break;
    }


  return this->Allocated;
}


//----------------------------------------------------------------------------
// Description:
// You can set the scalars directly (instead of allocating), but
// you better make sure that the bounds are set properly 
// before this method is called. Old scalars are released, and
// the new scalars are registered by this object.  The type and
// increments are calculate as a side action of this call.
void vtkImageData::SetScalars(vtkScalars *scalars)
{
  int idx, inc=1, num;

  // Set the proper type.
  if (strcmp(scalars->GetDataType(), "float") == 0)
    {
    this->Type = VTK_IMAGE_FLOAT;
    num = ((vtkFloatScalars *)(scalars))->GetNumberOfScalars();
    }
  else if (strcmp(scalars->GetDataType(), "int") == 0)
    {
    this->Type = VTK_IMAGE_INT;
    num = ((vtkIntScalars *)(scalars))->GetNumberOfScalars();
    }
  else if (strcmp(scalars->GetDataType(), "short") == 0)
    {
    this->Type = VTK_IMAGE_SHORT;
    num = ((vtkShortScalars *)(scalars))->GetNumberOfScalars();
    }
  else if (strcmp(scalars->GetDataType(), "unsigned short") == 0)
    {
    this->Type = VTK_IMAGE_UNSIGNED_SHORT;
    num = ((vtkUnsignedShortScalars *)(scalars))->GetNumberOfScalars();
    }
  else if (strcmp(scalars->GetDataType(), "unsigned char") == 0)
    {
    this->Type = VTK_IMAGE_UNSIGNED_CHAR;
    num = ((vtkUnsignedCharScalars *)(scalars))->GetNumberOfScalars();
    }
  else
    {
    vtkErrorMacro(<< "SetScalars: Cannot handle " << scalars->GetClassName());
    return;
    }
  

  // delete previous data
  // in the future try to reuse memory
  if (this->Scalars)
    {
    this->Scalars->UnRegister(this);
    this->Scalars = NULL;
    }
  this->Scalars = scalars;
  this->Scalars->Register(this);
  
  // set up increments
  for (idx = 0; idx < VTK_IMAGE_DIMENSIONS; ++idx)
    {
    this->Increments[idx] = inc;
    inc *= (this->Bounds[idx*2+1] - this->Bounds[idx*2] + 1);
    }
  
  if (inc != num)
    {
    vtkWarningMacro(<< "SetScalars: Bounds (" << inc 
                    << " pixels) does not match "
                    << num << " scalars.");
    }
}


//----------------------------------------------------------------------------
// Description:
// This Method returns a pointer to a location in the vtkImageData.
// Coordinates are in pixel units and are relative to the whole
// image origin.
void *vtkImageData::GetVoidPointer(int coordinates[VTK_IMAGE_DIMENSIONS])
{
  int idx;
    
  // error checking: since most acceses will be from pointer arithmatic.
  // this should not waste much time.
  for (idx = 0; idx < VTK_IMAGE_DIMENSIONS; ++idx)
    {
    if (coordinates[idx] < this->Bounds[idx*2] ||
	coordinates[idx] > this->Bounds[idx*2+1])
      {
      vtkErrorMacro(<< "GetVoidPointer: Pixel (" << coordinates[0] << ", " 
                    << coordinates[1] << ", " << coordinates[2] << ", "
                    << coordinates[3] << ") not in memory.");
      return NULL;
      }
    }
  
  // Note the VTK data model (Scalars) does not exactly fit with
  // Image data model. We need a switch to get a void pointer.
  idx = ((coordinates[0] - this->Bounds[0]) * this->Increments[0]
	 + (coordinates[1] - this->Bounds[2]) * this->Increments[1]
	 + (coordinates[2] - this->Bounds[4]) * this->Increments[2]
	 + (coordinates[3] - this->Bounds[6]) * this->Increments[3]
	 + (coordinates[4] - this->Bounds[8]) * this->Increments[4]);
  
  return this->Scalars->GetVoidPtr(idx);
}


//----------------------------------------------------------------------------
// Description:
// This Method returns a pointer to the origin of the vtkImageData.
void *vtkImageData::GetVoidPointer()
{
  return this->Scalars->GetVoidPtr(0);
}



/*****************************************************************************
  Stuff for copying data (double templated).
*****************************************************************************/

  
//----------------------------------------------------------------------------
// Second templated function for copying.
// The fifth dimension should be colapsed, but I havent completely
// adopted this protocall yet, so ....
template <class IT, class OT>
void vtkImageDataCopyData(vtkImageData *outData, OT *outPtr,
			  vtkImageData *inData, IT *inPtr)
{
  int *p;
  IT *inPtr0, *inPtr1, *inPtr2, *inPtr3, *inPtr4;
  OT *outPtr0, *outPtr1, *outPtr2, *outPtr3, *outPtr4;
  int inInc0, inInc1, inInc2, inInc3, inInc4;
  int outInc0, outInc1, outInc2, outInc3, outInc4;
  int outMin0, outMax0, outMin1, outMax1, 
    outMin2, outMax2, outMin3, outMax3, outMin4, outMax4;
  int idx0, idx1, idx2, idx3, idx4;

  // Get information to loop through data.
  p = inData->GetIncrements();
  inInc0 = p[0]; inInc1 = p[1]; inInc2 = p[2]; inInc3 = p[3]; inInc4 = p[4];
  p = outData->GetIncrements();
  outInc0= p[0]; outInc1= p[1]; outInc2= p[2]; outInc3= p[3]; outInc4= p[4];
  p = outData->GetBounds();
  outMin0= p[0]; outMin1= p[2]; outMin2= p[4]; outMin3= p[6]; outMin4= p[8]; 
  outMax0= p[1]; outMax1= p[3]; outMax2= p[5]; outMax3= p[7]; outMax4= p[9]; 
  
  inPtr4 = inPtr;
  outPtr4 = outPtr;
  for (idx4 = outMin4; idx4 <= outMax4; ++idx4)
    {
    inPtr3 = inPtr4;
    outPtr3 = outPtr4;
    for (idx3 = outMin3; idx3 <= outMax3; ++idx3)
      {
      inPtr2 = inPtr3;
      outPtr2 = outPtr3;
      for (idx2 = outMin2; idx2 <= outMax2; ++idx2)
	{
	inPtr1 = inPtr2;
	outPtr1 = outPtr2;
	for (idx1 = outMin1; idx1 <= outMax1; ++idx1)
	  {
	  inPtr0 = inPtr1;
	  outPtr0 = outPtr1;
	  for (idx0 = outMin0; idx0 <= outMax0; ++idx0)
	    {
	    *outPtr0 = (OT)(*inPtr0);
	    inPtr0 += inInc0;
	    outPtr0 += outInc0;
	    }
	  inPtr1 += inInc1;
	  outPtr1 += outInc1;
	  }
	inPtr2 += inInc2;
	outPtr2 += outInc2;
	}
      inPtr3 += inInc3;
      outPtr3 += outInc3;
      }
    inPtr4 += inInc4;
    outPtr4 += outInc4;
    }
}
  
  

//----------------------------------------------------------------------------
// First templated function for copying.
template <class T>
void vtkImageDataCopyData(vtkImageData *self, vtkImageData *inData, T *inPtr)
{
  void *outPtr;
  
  outPtr = self->GetVoidPointer();
  
  switch (self->GetType())
    {
    case VTK_IMAGE_FLOAT:
      vtkImageDataCopyData(self, (float *)(outPtr), inData, inPtr);
      break;
    case VTK_IMAGE_INT:
      vtkImageDataCopyData(self, (int *)(outPtr), inData, inPtr);
      break;
    case VTK_IMAGE_SHORT:
      vtkImageDataCopyData(self, (short *)(outPtr), inData, inPtr);
      break;
    case VTK_IMAGE_UNSIGNED_SHORT:
      vtkImageDataCopyData(self, (unsigned short *)(outPtr), inData, inPtr);
      break;
    case VTK_IMAGE_UNSIGNED_CHAR:
      vtkImageDataCopyData(self, (unsigned char *)(outPtr), inData, inPtr);
      break;
    default:
      cerr << "vtkImageDataCopyData: Cannot handle DataType.\n\n";
    }   
}

//----------------------------------------------------------------------------
// Description:
// Copies data into this object.  If Type is not set, the default type
// is set to the incoming type.  Otherwise, the dat is converted
// with a simple type cast.  It will not deal with reducing precision
// intelligently.
void vtkImageData::CopyData(vtkImageData *data)
{
  void *inPtr;
  int *inBounds, *outBounds;
  int origin[VTK_IMAGE_DIMENSIONS];
  int idx;
  
  // Make sure our bounds are containedin the incoming data.
  inBounds = data->GetBounds();
  outBounds = this->GetBounds();
  for (idx = 0; idx < VTK_IMAGE_DIMENSIONS; ++idx)
    {
    if (outBounds[2*idx] < inBounds[2*idx] ||
	outBounds[2*idx+1] > inBounds[2*idx+1])
      {
      vtkErrorMacro(<< "CopyData: Bounds mismatch.");
      return;
      }
    origin[idx] = outBounds[2*idx];
    }

  // If the data type is not set, default to same as input.
  if (this->GetType() == VTK_IMAGE_VOID)
    {
    this->SetType(data->GetType());
    }
  
  // Make sure the region is allocated
  if ( ! this->IsAllocated())
    {
    this->Allocate();
    }
  if ( ! this->IsAllocated())
    {
    vtkErrorMacro(<< "CopyData: Could not allocate data.");
    return;
    }
  
  inPtr = data->GetVoidPointer(origin);
  
  switch (data->GetType())
    {
    case VTK_IMAGE_FLOAT:
      vtkImageDataCopyData(this, data, (float *)(inPtr));
      break;
    case VTK_IMAGE_INT:
      vtkImageDataCopyData(this, data, (int *)(inPtr));
      break;
    case VTK_IMAGE_SHORT:
      vtkImageDataCopyData(this, data, (short *)(inPtr));
      break;
    case VTK_IMAGE_UNSIGNED_SHORT:
      vtkImageDataCopyData(this, data, (unsigned short *)(inPtr));
      break;
    case VTK_IMAGE_UNSIGNED_CHAR:
      vtkImageDataCopyData(this, data, (unsigned char *)(inPtr));
      break;
    default:
      vtkErrorMacro(<< "CopyData: Cannot handle Type.");
    }   
}













