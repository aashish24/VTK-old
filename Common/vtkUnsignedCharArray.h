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
// .NAME vtkUnsignedCharArray - dynamic, self-adjusting unsigned character array
// .SECTION Description
// vtkUnsignedCharArray is an array of unsigned character values. It provides 
// methods for insertion and retrieval of characters, and will automatically 
// resize itself to hold new data.

#ifndef __vtkUnsignedCharArray_h
#define __vtkUnsignedCharArray_h

#include "vtkDataArray.h"

class VTK_EXPORT vtkUnsignedCharArray : public vtkDataArray
{
public:

// Description:
// Instantiate object.
  vtkUnsignedCharArray(int numComp=1);

  ~vtkUnsignedCharArray();

// Description:
// Allocate memory for this array. Delete old storage only if necessary.
  int Allocate(const int sz, const int ext=1000);


// Description:
// Release storage and reset array to initial state.
  void Initialize();

  static vtkUnsignedCharArray *New() {return new vtkUnsignedCharArray;};
  const char *GetClassName() {return "vtkUnsignedCharArray";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // satisfy vtkDataArray API
  vtkDataArray *MakeObject() {return new vtkUnsignedCharArray(this->NumberOfComponents);};
  int GetDataType() {return VTK_UNSIGNED_CHAR;};

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


// Description:
// Set the tuple value at the ith location in the array.
  void SetTuple(const int i, const float * tuple);


// Description:
// Insert (memory allocation performed) the tuple into the ith location
// in the array.
  void InsertTuple(const int i, const float * tuple);


// Description:
// Insert (memory allocation performed) the tuple onto the end of the array.
  int InsertNextTuple(const float * tuple);

  void Squeeze();

  // overload vtkDataArray for efficiency

// Description:
// Return the data component at the ith tuple and jth component location.
// Note that i<NumberOfTuples and j<NumberOfComponents.
  float GetComponent(const int i, const int j);


// Description:
// Set the data component at the ith tuple and jth component location.
// Note that i<NumberOfTuples and j<NumberOfComponents. Make sure enough
// memory has been allocated (use SetNumberOfTuples() and 
// SetNumberOfComponents()).
  void SetComponent(const int i, const int j, const float c);


// Description:
// Insert the data component at ith tuple and jth component location. 
// Note that memory allocation is performed as necessary to hold the data.
  void InsertComponent(const int i, const int j, const float c);


  // native access/insertion methods
  unsigned char GetValue(const int id);
  void SetNumberOfValues(const int number);
  void SetValue(const int id, const unsigned char value);
  void InsertValue(const int id, const unsigned char c);
  int InsertNextValue(const unsigned char c);
  unsigned char *GetPointer(const int id) {return this->Array + id;}
  unsigned char *WritePointer(const int id, const int number);
  void *GetVoidPointer(const int id) {return (void *)this->GetPointer(id);};

// Description:
// Deep copy of another unsigned char array.
  void DeepCopy(vtkDataArray& da);



// Description:
// This method lets the user specify data to be held by the array.  The 
// array argument is a pointer to the data.  size is the size of 
// the array supplied by the user.  Set save to 1 to keep the class
// from deleting the array when it cleans up or reallocates memory.
// The class uses the actual array provided; it does not copy the data 
// from the suppled array.
  void SetArray(unsigned char* array, int size, int save);


private:
  unsigned char *Array;   // pointer to data
  unsigned char *Resize(const int sz);  // function to resize data

  int TupleSize; //used for data conversion
  float *Tuple;

  int SaveUserArray;
};

// Description:
// Get the data at a particular index.
inline unsigned char vtkUnsignedCharArray::GetValue(const int id) {return this->Array[id];}

// Description:
// Specify the number of values for this object to hold. Does an
// allocation as well as setting the MaxId ivar. Used in conjunction with
// SetValue() method for fast insertion.
inline void vtkUnsignedCharArray::SetNumberOfValues(const int number) 
{
  this->Allocate(number);
  this->MaxId = number - 1;
}

// Description:
// Set the data at a particular index. Does not do range checking. Make sure
// you use the method SetNumberOfValues() before inserting data.
inline void vtkUnsignedCharArray::SetValue(const int id, const unsigned char value) 
{
  this->Array[id] = value;
}

// Description:
// Get the address of a particular data index. Make sure data is allocated
// for the number of items requested. Set MaxId according to the number of
// data values requested.
inline unsigned char *vtkUnsignedCharArray::WritePointer(const int id, const int number) 
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
  return this->Array + id;
}

// Description:
// Insert data at a specified position in the array.
inline void vtkUnsignedCharArray::InsertValue(const int id, const unsigned char c)
{
  if ( id >= this->Size )
    {
    this->Resize(id+1);
    }
  this->Array[id] = c;
  if ( id > this->MaxId )
    {
    this->MaxId = id;
    }
}

// Description:
// Insert data at the end of the array. Return its location in the array.
inline int vtkUnsignedCharArray::InsertNextValue(const unsigned char c)
{
  this->InsertValue (++this->MaxId,c); 
  return this->MaxId;
}

// Description:
// Resize object to just fit data requirement. Reclaims extra memory.
inline void vtkUnsignedCharArray::Squeeze() {this->Resize (this->MaxId+1);}

#endif
