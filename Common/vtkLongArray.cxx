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
#include "vtkLongArray.h"

// Description:
// Instantiate object.
vtkLongArray::vtkLongArray(int numComp)
{
  this->NumberOfComponents = (numComp < 1 ? 1 : numComp);
  this->Array = NULL;
  this->TupleSize = 3;
  this->Tuple = new float[this->TupleSize]; //used for conversion
  this->SaveUserArray = 0;
}

vtkLongArray::~vtkLongArray()
{
  if ((this->Array) && (!this->SaveUserArray)) delete [] this->Array;
  delete [] this->Tuple;
}

// Description:
// This method lets the user specify data to be held by the array.  The 
// array argument is a pointer to the data.  size is the size of 
// the array supplied by the user.  Set save to 1 to keep the class
// from deleting the array when it cleans up or reallocates memory.
// The class uses the actual array provided; it does not copy the data 
// from the suppled array.
void vtkLongArray::SetArray(long* array, int size, int save)
{
  
  if ((this->Array) && (!this->SaveUserArray))
    {
    vtkDebugMacro (<< "Deleting the array...");
    delete [] this->Array;
    }
  else 
    {
    vtkDebugMacro (<<"Warning, array not deleted, but will point to new array.");
    }

  vtkDebugMacro(<<"Setting array to: " << array);

  this->Array = array;
  this->Size = size;
  this->MaxId = size-1;
  this->SaveUserArray = save;
}


// Description:
// Allocate memory for this array. Delete old storage only if necessary.
int vtkLongArray::Allocate(const int sz, const int ext)
{
  if ( sz > this->Size || this->Array == NULL )
    {
    if ((this->Array) && (!this->SaveUserArray))
      {
      delete [] this->Array;
      }
    this->Size = ( sz > 0 ? sz : 1);
    if ( (this->Array = new long[this->Size]) == NULL ) return 0;
    this->SaveUserArray = 0;
    }

  this->Extend = ( ext > 0 ? ext : 1);
  this->MaxId = -1;

  return 1;
}

// Description:
// Release storage and reset array to initial state.
void vtkLongArray::Initialize()
{
  if (( this->Array != NULL ) && (!this->SaveUserArray))
    {
    delete [] this->Array;
    }
  this->Array = NULL;
  this->Size = 0;
  this->MaxId = -1;
  this->SaveUserArray = 0;
}

// Description:
// Deep copy of another long array.
void vtkLongArray::DeepCopy(vtkDataArray& sa)
{
  if ( sa.GetDataType() != VTK_LONG )
    {
      vtkDataArray::DeepCopy(sa);
      return;
    }

  if ( this != &sa )
    {
    if ((this->Array) && (!this->SaveUserArray))
      {
      delete [] this->Array;
      }

    this->NumberOfComponents = sa.GetNumberOfComponents();
    this->MaxId = sa.GetMaxId();
    this->Size = sa.GetSize();
    this->Extend = sa.GetExtend();
    this->SaveUserArray = 0;

    this->Array = new long[this->Size];
    memcpy(this->Array, (long *) sa.GetVoidPointer(0), this->Size*sizeof(long));
    }
}

void vtkLongArray::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataArray::PrintSelf(os,indent);

  os << indent << "Array: " << this->Array << "\n";
}

//
// Private function does "reallocate"
//
long *vtkLongArray::Resize(const int sz)
{
  long *newArray;
  int newSize;

  if ( sz > this->Size ) 
    {
    newSize = this->Size + this->Extend*(((sz-this->Size)/this->Extend)+1);
    }
  else if (sz == this->Size)
    {
    return this->Array;
    }
  else 
    {
    newSize = sz;
    }

  if ( (newArray = new long[newSize]) == NULL )
    {
    vtkErrorMacro(<< "Cannot allocate memory\n");
    return 0;
    }

  if (this->Array)
    {
    memcpy(newArray, this->Array, 
         (sz < this->Size ? sz : this->Size) * sizeof(long));
    if (!this->SaveUserArray)
      {
      delete [] this->Array;
      }
    }

  this->Size = newSize;
  this->Array = newArray;
  this->SaveUserArray = 0;

  return this->Array;
}

// Description:
// Set the number of n-tuples in the array.
void vtkLongArray::SetNumberOfTuples(const int number)
{
  this->SetNumberOfValues(number*this->NumberOfComponents);
}

// Description:
// Get a pointer to a tuple at the ith location. This is a dangerous method
// (it is not thread safe since a pointer is returned).
float *vtkLongArray::GetTuple(const int i) 
{
  if ( this->TupleSize < this->NumberOfComponents )
    {
    this->TupleSize = this->NumberOfComponents;
    delete [] this->Tuple;
    this->Tuple = new float[this->TupleSize];
    }

  long *t = this->Array + this->NumberOfComponents*i;
  for (int j=0; j<this->NumberOfComponents; j++) this->Tuple[j] = (float)t[j];
  return this->Tuple;
}

// Description:
// Copy the tuple value into a user-provided array.
void vtkLongArray::GetTuple(const int i, float * tuple)
{
  long *t = this->Array + this->NumberOfComponents*i;
  for (int j=0; j<this->NumberOfComponents; j++) tuple[j] = (float)t[j];
}

// Description:
// Set the tuple value at the ith location in the array.
void vtkLongArray::SetTuple(const int i, const float * tuple)
{
  int loc = i * this->NumberOfComponents; 
  for (int j=0; j<this->NumberOfComponents; j++) 
    this->Array[loc+j] = (long)tuple[j];
}

// Description:
// Insert (memory allocation performed) the tuple into the ith location
// in the array.
void vtkLongArray::InsertTuple(const int i, const float * tuple)
{
  long *t = this->WritePointer(i*this->NumberOfComponents,this->NumberOfComponents);

  for (int j=0; j<this->NumberOfComponents; j++) *t++ = (long)*tuple++;
}

// Description:
// Insert (memory allocation performed) the tuple onto the end of the array.
int vtkLongArray::InsertNextTuple(const float * tuple)
{
  int i = this->MaxId + 1;
  long *t = this->WritePointer(i,this->NumberOfComponents);

  for (i=0; i<this->NumberOfComponents; i++) *t++ = (long)*tuple++;

  return this->MaxId / this->NumberOfComponents;
}
