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
// .NAME vtkBitArray - dynamic, self-adjusting array of bits
// .SECTION Description
// vtkBitArray is an array of bits (0/1 data value). The array is packed 
// so that each byte stores eight bits. vtkBitArray provides methods
// for insertion and retrieval of bits, and will automatically resize 
// itself to hold new data.

#ifndef __vtkBitArray_h
#define __vtkBitArray_h

#include "vtkDataArray.h"

class VTK_EXPORT vtkBitArray : public vtkDataArray
{
public:
  static vtkBitArray *New();
  vtkTypeMacro(vtkBitArray,vtkDataArray);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Allocate memory for this array. Delete old storage only if necessary.
  int Allocate(const int sz, const int ext=1000);

  // Description:
  // Release storage and reset array to initial state.
  void Initialize();

  // satisfy vtkDataArray API
  vtkDataArray *MakeObject();
  int GetDataType() {return VTK_BIT;};
  
  // Description:
  // Set the number of n-tuples in the array.
  void SetNumberOfTuples(const int number);

  // Description:
  // Get a pointer to a tuple at the ith location. This is a dangerous method
  // (it is not thread safe since a pointer is returned).
  float *GetTuple(const int i);

  // Description:
  // Copy the tuple value into a user-provided array.
  void GetTuple(const int i, float * tuple);
  void GetTuple(const int i, double * tuple);
  
  // Description:
  // Set the tuple value at the ith location in the array.
  void SetTuple(const int i, const float * tuple);
  void SetTuple(const int i, const double * tuple);
  
  // Description:
  // Insert (memory allocation performed) the tuple into the ith location
  // in the array.
  void InsertTuple(const int i, const float * tuple);
  void InsertTuple(const int i, const double * tuple);

  // Description:
  // Insert (memory allocation performed) the tuple onto the end of the array.
  int InsertNextTuple(const float * tuple);
  int InsertNextTuple(const double * tuple);

  // Description:
  // Free any uunrequired memory.
  void Squeeze();

  // Description:
  // Get the data at a particular index.
  int GetValue(const int id);

  // Description:
  // Fast method based setting of values without memory checks. First
  // use SetNumberOfValues then use SetValue to actually set them.
  // Specify the number of values for this object to hold. Does an
  // allocation as well as setting the MaxId ivar. Used in conjunction with
  // SetValue() method for fast insertion.
  void SetNumberOfValues(const int number);

  // Description:
  // Set the data at a particular index. Does not do range checking. Make sure
  // you use the method SetNumberOfValues() before inserting data.
  void SetValue(const int id, const int value);

  // Description:
  // Insets values and checks to make sure there is enough memory
  void InsertValue(const int id, const int i);
  int InsertNextValue(const int i);

  // Description:
  // Direct manipulation of the underlying data.
  unsigned char *GetPointer(const int id) {return this->Array + id/8;}

  // Description:
  // Get the address of a particular data index. Make sure data is allocated
  // for the number of items requested. Set MaxId according to the number of
  // data values requested.
  unsigned char *WritePointer(const int id, const int number);
  void *GetVoidPointer(const int id) {return (void *)this->GetPointer(id);};

  // Description:
  // Deep copy of another bit array.
  void DeepCopy(vtkDataArray *da);

  // Description:
  // This method lets the user specify data to be held by the array.  The 
  // array argument is a pointer to the data.  size is the size of 
  // the array supplied by the user.  Set save to 1 to keep the class
  // from deleting the array when it cleans up or reallocates memory.
  // The class uses the actual array provided; it does not copy the data 
  // from the suppled array.
  void SetArray(unsigned char* array, int size, int save);
  void SetVoidArray(void *array,int size, int save) 
    {this->SetArray((unsigned char *)array, size, save);};

  // Description:
  // For legacy compatibility. Do not use.
  void DeepCopy(vtkBitArray &da) {this->DeepCopy(&da);}

protected:
  vtkBitArray(int numComp=1);
  ~vtkBitArray();
  vtkBitArray(const vtkBitArray&) {};
  void operator=(const vtkBitArray&) {};

  unsigned char *Array;   // pointer to data
  unsigned char *Resize(const int sz);  // function to resize data

  int TupleSize; //used for data conversion
  float *Tuple;

  int SaveUserArray;

private:
  // hide superclass' DeepCopy() from the user and the compiler
  void DeepCopy(vtkDataArray &da) {this->vtkDataArray::DeepCopy(&da);}
  
};

inline unsigned char *vtkBitArray::WritePointer(const int id, const int number)
{
  int newSize=id+number;
  if ( newSize > this->Size )
    {
    this->Resize(newSize);
    }
  if ( (--newSize) > this->MaxId )
    {
    this->MaxId = newSize;
    }
  return this->Array + id/8;
}

inline void vtkBitArray::SetNumberOfValues(const int number) 
{
  this->Allocate(number);
  this->MaxId = number - 1;
}

inline void vtkBitArray::SetValue(const int id, const int value) 
{
  if (value)
    {
    this->Array[id/8] |= (0x80 >> id%8);
    }
  else
    {
    this->Array[id/8] &= (~(0x80 >> id%8));
    }
}

inline void vtkBitArray::InsertValue(const int id, const int i)
{
  if ( id >= this->Size )
    {
    this->Resize(id+1);
    }
  if (i)
    {
    this->Array[id/8] |= (0x80 >> id%8);
    }
  else
    {
    this->Array[id/8] &= (~(0x80 >> id%8));
    }
  if ( id > this->MaxId )
    {
    this->MaxId = id;
    }
}

inline int vtkBitArray::InsertNextValue(const int i)
{
  this->InsertValue (++this->MaxId,i); return this->MaxId;
}

inline void vtkBitArray::Squeeze() {this->Resize (this->MaxId+1);}

#endif

