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
// .NAME vtkUnsignedLongArray - dynamic, self-adjusting unsigned long integer array
// .SECTION Description
// vtkUnsignedLongArray is an array of unsigned long integer numbers. It
// provides methods for insertion and retrieval of integer values, and will
// automatically resize itself to hold new data.

#ifndef __vtkUnsignedLongArray_h
#define __vtkUnsignedLongArray_h

#include "vtkDataArray.h"

class VTK_COMMON_EXPORT vtkUnsignedLongArray : public vtkDataArray 
{
public:
  static vtkUnsignedLongArray *New();

  vtkTypeMacro(vtkUnsignedLongArray,vtkDataArray);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Allocate memory for this array. Delete old storage only if necessary.
  // Note that ext is no longer used.
  int Allocate(const vtkIdType sz, const vtkIdType ext=1000);

  // Description:
  // Release storage and reset array to initial state.
  void Initialize();

  // Description:
  // Create the same type object as this (virtual constructor).
  vtkDataArray *MakeObject();

  // Description:
  // Get the data type.
  int GetDataType() {return VTK_UNSIGNED_LONG;};
  
  // Description:
  // Set the number of n-tuples in the array.
  void SetNumberOfTuples(const vtkIdType number);

  // Description:
  // Get a pointer to a tuple at the ith location. This is a dangerous method
  // (it is not thread safe since a pointer is returned).
  float *GetTuple(const vtkIdType i);

  // Description:
  // Copy the tuple value into a user-provided array.
  void GetTuple(const vtkIdType i, float * tuple);
  void GetTuple(const vtkIdType i, double * tuple);
  
  // Description:
  // Set the tuple value at the ith location in the array.
  void SetTuple(const vtkIdType i, const float * tuple);
  void SetTuple(const vtkIdType i, const double * tuple);

  // Description:
  // Insert (memory allocation performed) the tuple into the ith location
  // in the array.
  void InsertTuple(const vtkIdType i, const float * tuple);
  void InsertTuple(const vtkIdType i, const double * tuple);

  // Description:
  // Insert (memory allocation performed) the tuple onto the end of the array.
  vtkIdType InsertNextTuple(const float * tuple);
  vtkIdType InsertNextTuple(const double * tuple);
  
  // Description:
  // Get the data at a particular index.
  unsigned long GetValue(const vtkIdType id) {return this->Array[id];};
  
  // Description:
  // Set the data at a particular index. Does not do range checking. Make sure
  // you use the method SetNumberOfValues() before inserting data.
  void SetValue(const vtkIdType id, const unsigned long value) {
    this->Array[id] = value;};

  // Description:
  // Specify the number of values for this object to hold. Does an
  // allocation as well as setting the MaxId ivar. Used in conjunction with
  // SetValue() method for fast insertion.
  void SetNumberOfValues(const vtkIdType number);

  // Description:
  // Insert data at a specified position in the array.
  void InsertValue(const vtkIdType id, const unsigned long i);

  // Description:
  // Insert data at the end of the array. Return its location in the array.
  vtkIdType InsertNextValue(const unsigned long);

  // Description:
  // Get the address of a particular data index. Performs no checks
  // to verify that the memory has been allocated etc.
  unsigned long *GetPointer(const vtkIdType id) {return this->Array + id;}
  void *GetVoidPointer(const vtkIdType id)
    {return (void *)this->GetPointer(id);};

  // Description:
  // Get the address of a particular data index. Make sure data is allocated
  // for the number of items requested. Set MaxId according to the number of
  // data values requested.
  unsigned long *WritePointer(const vtkIdType id, const vtkIdType number);

  // Description:
  // Deep copy of another unsigned long array.
  void DeepCopy(vtkDataArray *da);

  // Description:
  // This method lets the user specify data to be held by the array.  The 
  // array argument is a pointer to the data.  size is the size of 
  // the array supplied by the user.  Set save to 1 to keep the class
  // from deleting the array when it cleans up or reallocates memory.
  // The class uses the actual array provided; it does not copy the data 
  // from the suppled array.
  void SetArray(unsigned long* array, vtkIdType size, int save);
  void SetVoidArray(void *array, vtkIdType size, int save) 
    {this->SetArray((unsigned long*)array, size, save);};

  // Description:
  // Resize object to just fit data requirement. Reclaims extra memory.
  void Squeeze() {this->ResizeAndExtend (this->MaxId+1);};

  // Description:
  // Resize the array while conserving the data.
  virtual void Resize(vtkIdType numTuples);

#ifndef VTK_REMOVE_LEGACY_CODE
  // Description:
  // For legacy compatibility. Do not use.
  void DeepCopy(vtkDataArray &da) 
    {VTK_LEGACY_METHOD(DeepCopy,"3.2"); this->DeepCopy(&da);}
#endif
  

protected:
  vtkUnsignedLongArray(vtkIdType numComp=1);
  ~vtkUnsignedLongArray();

  unsigned long *Array;   // pointer to data
  unsigned long *ResizeAndExtend(const vtkIdType sz);
    // function to resize data

  int TupleSize; //used for data conversion
  float *Tuple;

  int SaveUserArray;
private:
  vtkUnsignedLongArray(const vtkUnsignedLongArray&);  // Not implemented.
  void operator=(const vtkUnsignedLongArray&);  // Not implemented.
};

inline void vtkUnsignedLongArray::SetNumberOfValues(const vtkIdType number) 
{
  this->Allocate(number);
  this->MaxId = number - 1;
}

inline unsigned long *vtkUnsignedLongArray::WritePointer(const vtkIdType id,
                                                         const vtkIdType number) 
{
  vtkIdType newSize=id+number;
  if ( newSize > this->Size )
    {
    this->ResizeAndExtend(newSize);
    }
  if ( (--newSize) > this->MaxId )
    {
    this->MaxId = newSize;
    }
  return this->Array + id;
}

inline void vtkUnsignedLongArray::InsertValue(const vtkIdType id,
                                              const unsigned long i)
{
  if ( id >= this->Size )
    {
    this->ResizeAndExtend(id+1);
    }
  this->Array[id] = i;
  if ( id > this->MaxId )
    {
    this->MaxId = id;
    }
}

inline vtkIdType vtkUnsignedLongArray::InsertNextValue(const unsigned long i)
{
  this->InsertValue (++this->MaxId,i); 
  return this->MaxId;
}


#endif




