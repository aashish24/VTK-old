/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen 
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
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "vtkUnsignedLongArray.h"
#include "vtkObjectFactory.h"

//----------------------------------------------------------------------------
vtkUnsignedLongArray* vtkUnsignedLongArray::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkUnsignedLongArray");
  if(ret)
    {
    return (vtkUnsignedLongArray*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkUnsignedLongArray;
}

vtkDataArray *vtkUnsignedLongArray::MakeObject()
{
  vtkDataArray *a = vtkUnsignedLongArray::New();
  a->SetNumberOfComponents(this->NumberOfComponents);
  return a;
}

// Instantiate object.
vtkUnsignedLongArray::vtkUnsignedLongArray(vtkIdType numComp)
{
  this->NumberOfComponents = (numComp < 1 ? 1 : numComp);
  this->Array = NULL;
  this->TupleSize = 3;
  this->Tuple = new float[this->TupleSize]; //used for conversion
  this->SaveUserArray = 0;
}

vtkUnsignedLongArray::~vtkUnsignedLongArray()
{
  if ((this->Array) && (!this->SaveUserArray))
    {
    delete [] this->Array;
    }
  delete [] this->Tuple;
}


// This method lets the user specify data to be held by the array.  The 
// array argument is a pointer to the data.  size is the size of 
// the array supplied by the user.  Set save to 1 to keep the class
// from deleting the array when it cleans up or reallocates memory.
// The class uses the actual array provided; it does not copy the data 
// from the suppled array.
void vtkUnsignedLongArray::SetArray(unsigned long* array, vtkIdType size,
                                    int save)
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


// Allocate memory for this array. Delete old storage only if necessary.
int vtkUnsignedLongArray::Allocate(const vtkIdType sz,
                                   const vtkIdType vtkNotUsed(ext))
{
  if ( sz > this->Size )
    {
    if ((this->Array) && (!this->SaveUserArray))
      {
      delete [] this->Array;
      }
    this->Size = ( sz > 0 ? sz : 1);
    if ( (this->Array = new unsigned long[this->Size]) == NULL )
      {
      return 0;
      }
    this->SaveUserArray = 0;
    }

  this->MaxId = -1;

  return 1;
}

// Release storage and reset array to initial state.
void vtkUnsignedLongArray::Initialize()
{
  if (( this->Array != NULL ) && (!this->SaveUserArray))
    {
    delete [] this->Array;
    }
  this->Array = NULL;
  this->Size = 0;
  this->MaxId = -1;
}

// Deep copy of another unsigned long array.
void vtkUnsignedLongArray::DeepCopy(vtkDataArray *sa)
{
  // Do nothing on a NULL input.
  if (sa == NULL)
    {
    return;
    }

  if ( sa->GetDataType() != VTK_UNSIGNED_LONG )
    {
    vtkDataArray::DeepCopy(sa);
    return;
    }

  if ( this != sa )
    {
    if ((this->Array) && (!this->SaveUserArray))
      {
      delete [] this->Array;
      }

    this->NumberOfComponents = sa->GetNumberOfComponents();
    this->MaxId = sa->GetMaxId();
    this->Size = sa->GetSize();
    this->SaveUserArray = 0;

    this->Array = new unsigned long[this->Size];
    memcpy(this->Array, (unsigned long *)sa->GetVoidPointer(0),
           this->Size*sizeof(unsigned long));

    }
}

void vtkUnsignedLongArray::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkDataArray::PrintSelf(os,indent);

  if (this->Array)
    {
    os << indent << "Array: " << this->Array << "\n";
    }
  else
    {
    os << indent << "Array: (null)\n";
    }
}

//
// Private function does "reallocate"
//
unsigned long *vtkUnsignedLongArray::ResizeAndExtend(const vtkIdType sz)
{
  unsigned long *newArray;
  vtkIdType newSize;

  if ( sz > this->Size ) 
    {
    newSize = this->Size + sz;
    }
  else if (sz == this->Size)
    {
    return this->Array;
    }
  else 
    {
    newSize = sz;
    }

  if (newSize <= 0)
    {
    this->Initialize();
    return 0;
    }
  
  if ( (newArray = new unsigned long[newSize]) == NULL )
    {
    vtkErrorMacro(<< "Cannot allocate memory\n");
    return 0;
    }

  if (this->Array)
    {
    memcpy(newArray, this->Array, 
         (sz < this->Size ? sz : this->Size) * sizeof(unsigned long));
    if (!this->SaveUserArray)
      {
      delete [] this->Array;
      }
    }
  if (newSize < this->Size)
    {
    this->MaxId = newSize-1;
    }
  this->Size = newSize;
  this->Array = newArray;
  this->SaveUserArray = 0;

  return this->Array;
}

void vtkUnsignedLongArray::Resize(vtkIdType sz)
{
  unsigned long *newArray;
  vtkIdType newSize = sz*this->NumberOfComponents;
 
  if (newSize == this->Size)
    {
    return;
    }

  if (newSize <= 0)
    {
    this->Initialize();
    return;
    }
  
  if ( (newArray = new unsigned long[newSize]) == NULL )
    {
    vtkErrorMacro(<< "Cannot allocate memory\n");
    return;
    }

  if (this->Array)
    {
    memcpy(newArray, this->Array, 
         (newSize < this->Size ? newSize : this->Size) * sizeof(unsigned long));
    if (!this->SaveUserArray)
      {
      delete [] this->Array;
      }
    }
  if (newSize < this->Size)
    {
    this->MaxId = newSize-1;
    }
  this->Size = newSize;
  this->Array = newArray;
  this->SaveUserArray = 0;

  return;
}


// Set the number of n-tuples in the array.
void vtkUnsignedLongArray::SetNumberOfTuples(const vtkIdType number)
{
  this->SetNumberOfValues(number*this->NumberOfComponents);
}

// Get a pointer to a tuple at the ith location. This is a dangerous method
// (it is not thread safe since a pointer is returned).
float *vtkUnsignedLongArray::GetTuple(const vtkIdType i) 
{
  if ( this->TupleSize < this->NumberOfComponents )
    {
    this->TupleSize = this->NumberOfComponents;
    delete [] this->Tuple;
    this->Tuple = new float[this->TupleSize];
    }

  unsigned long *t = this->Array + this->NumberOfComponents*i;
  for (int j=0; j<this->NumberOfComponents; j++)
    {
    this->Tuple[j] = (float)t[j];
    }
  return this->Tuple;
}

// Copy the tuple value into a user-provided array.
void vtkUnsignedLongArray::GetTuple(const vtkIdType i, float * tuple) 
{
  unsigned long *t = this->Array + this->NumberOfComponents*i;
  for (int j=0; j<this->NumberOfComponents; j++)
    {
    tuple[j] = (float)t[j];
    }
}

void vtkUnsignedLongArray::GetTuple(const vtkIdType i, double * tuple) 
{
  unsigned long *t = this->Array + this->NumberOfComponents*i;
  for (int j=0; j<this->NumberOfComponents; j++)
    {
    tuple[j] = (double)t[j];
    }
}

// Set the tuple value at the ith location in the array.
void vtkUnsignedLongArray::SetTuple(const vtkIdType i, const float * tuple)
{
  vtkIdType loc = i * this->NumberOfComponents; 

  for (int j=0; j<this->NumberOfComponents; j++) 
    {
    this->Array[loc+j] = (unsigned long)tuple[j];
    }
}

void vtkUnsignedLongArray::SetTuple(const vtkIdType i, const double * tuple)
{
  vtkIdType loc = i * this->NumberOfComponents; 

  for (int j=0; j<this->NumberOfComponents; j++) 
    {
    this->Array[loc+j] = (unsigned long)tuple[j];
    }
}

// Insert (memory allocation performed) the tuple into the ith location
// in the array.
void vtkUnsignedLongArray::InsertTuple(const vtkIdType i, const float * tuple)
{
  unsigned long *t = this->WritePointer(i*this->NumberOfComponents,this->NumberOfComponents);

  for (int j=0; j<this->NumberOfComponents; j++)
    {
    *t++ = (unsigned long)*tuple++;
    }
}

void vtkUnsignedLongArray::InsertTuple(const vtkIdType i, const double * tuple)
{
  unsigned long *t = this->WritePointer(i*this->NumberOfComponents,this->NumberOfComponents);

  for (int j=0; j<this->NumberOfComponents; j++)
    {
    *t++ = (unsigned long)*tuple++;
    }
}

// Insert (memory allocation performed) the tuple onto the end of the array.
vtkIdType vtkUnsignedLongArray::InsertNextTuple(const float * tuple)
{
  vtkIdType i = this->MaxId + 1;
  unsigned long *t = this->WritePointer(i,this->NumberOfComponents);

  for (i=0; i<this->NumberOfComponents; i++)
    {
    *t++ = (unsigned long)*tuple++;
    }

  return this->MaxId / this->NumberOfComponents;
}

vtkIdType vtkUnsignedLongArray::InsertNextTuple(const double * tuple)
{
  vtkIdType i = this->MaxId + 1;
  unsigned long *t = this->WritePointer(i,this->NumberOfComponents);

  for (i=0; i<this->NumberOfComponents; i++)
    {
    *t++ = (unsigned long)*tuple++;
    }

  return this->MaxId / this->NumberOfComponents;
}

void vtkUnsignedLongArray::InsertComponent(const vtkIdType i, const int j, 
                                           const float c)
{
  this->InsertValue(i*this->NumberOfComponents + j, 
                    static_cast<unsigned long>(c));
}
