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
// .NAME vtkAbstractMap - a dynamic map data structure
// .SECTION Description
// vtkAbstractMap is a an abstract templated superclass of all
// containers that implement map data structure.
//
// Map data structure is a one dimensional sequence of pairs
// of key and data. On the higher level, it implements mapping
// from key values to data elements. It can be implemented using
// array of pairs, hash table, or different trees.

// .SECTION See also
// vtkContainer

#include "vtkContainer.h"

#ifndef __vtkAbstractMap_h
#define __vtkAbstractMap_h

// Since some compilers have problems with keyword typename, we have 
// to do this with macros.
// These macros define types for pointers to auxilary functions.
#define vtkAbstractMapCompareFunction(KeyType, CompareFunction) \
    int (*CompareFunction)(const KeyType&  k1, const KeyType& k2)
#define vtkAbstractMapCreateFunction(KeyType, CreateFunction) \
    void (*CreateFunction)(KeyType& k1, const KeyType& k2)
#define vtkAbstractMapDeleteFunction(KeyType, DeleteFunction) \
    void (*DeleteFunction)(KeyType& k1)


// This is an item of the map.
template<class KeyType, class DataType>
class vtkAbstractMapItem
{
public:
  KeyType Key;
  DataType Data;
};

template<class KeyType, class DataType>
class vtkAbstractMap : public vtkContainer
{
public:
  // Cannot use this macro because of the comma in the type name.
  // The CPP splits that in two and we ae in trouble.
  //vtkContainerTypeMacro((vtkAbstractMap<KeyType,DataType>), vtkContainer);

  typedef vtkContainer Superclass; 
  virtual const char *GetClassName() 
    {return "vtkAbstractMap";} 
  static int IsTypeOf(const char *type) 
  { 
    if ( !strcmp("vtkAbstractMap",type) )
      { 
      return 1;
      }
    return Superclass::IsTypeOf(type);
  }
  virtual int IsA(const char *type)
  {
    return this->vtkAbstractMap<KeyType,DataType>::IsTypeOf(type);
  }

  // Just to avoid typing over and over, let us define some typedefs.
  // They will not work in subclasses, but this header file will 
  // be more readable.
  typedef vtkAbstractMapCompareFunction(KeyType, KeyCompareFunctionType);
  typedef vtkAbstractMapCreateFunction(KeyType,  KeyCreateFunctionType);
  typedef vtkAbstractMapDeleteFunction(KeyType,  KeyDeleteFunctionType);
  typedef vtkAbstractMapCompareFunction(DataType, DataCompareFunctionType);
  typedef vtkAbstractMapCreateFunction(DataType,  DataCreateFunctionType);
  typedef vtkAbstractMapDeleteFunction(DataType,  DataDeleteFunctionType);

  // Description:
  // Sets the item at with specific key to data.
  // It overwrites the old item.
  // It returns VTK_OK if successfull.
  virtual int SetItem(const KeyType& key, const DataType& data) = 0;
  
  // Description:
  // Remove an Item with the key from the map.
  // It returns VTK_OK if successfull.
  virtual int RemoveItem(const KeyType& key) = 0;
  
  // Description:
  // Return the data asociated with the key.
  // It returns VTK_OK if successfull.
  virtual int GetItem(const KeyType& key, DataType& data) = 0;
  
  // Description:
  // Return the number of items currently held in this container. This
  // different from GetSize which is provided for some containers. GetSize
  // will return how many items the container can currently hold.
  virtual vtkIdType GetNumberOfItems() = 0;  

  // Description:
  // Set compare, create, and delete functions for keys and data.
  void SetKeyCreateFunction(KeyCreateFunctionType cf)
  { this->KeyCreateFunction = cf; }
  void SetKeyCompareFunction(KeyCompareFunctionType cf)
  { this->KeyCompareFunction = cf;}
  void SetKeyDeleteFunction(KeyDeleteFunctionType cf)
  { this->KeyDeleteFunction = cf; }
  void SetDataCreateFunction(DataCreateFunctionType cf)
  { this->DataCreateFunction = cf; }
  void SetDataCompareFunction(DataCompareFunctionType cf)
  { this->DataCompareFunction = cf;}
  void SetDataDeleteFunction(DataDeleteFunctionType cf)
  { this->DataDeleteFunction = cf; }

//protected:
  vtkAbstractMap();
  // Description:
  // These are pointers to auxilary functions to abstract certain object
  // operations out of the map.
  // The create function has to be able to create object out of 
  // another object. It should efectivly duplicate the object.
  // The compare function has to compare two object, return 0 if they
  // are equal, -1 if first one is smaller or 1 if second one is 
  // smaller. Current implementation of the map, only checks if they
  // are equal or not.
  // The delete function has to do all the necessary memory deallocation,
  // so that map can forget the object.
  KeyCreateFunctionType  KeyCreateFunction;
  KeyCompareFunctionType KeyCompareFunction;  
  KeyDeleteFunctionType  KeyDeleteFunction;  
  DataCreateFunctionType  DataCreateFunction;
  DataCompareFunctionType DataCompareFunction;  
  DataDeleteFunctionType  DataDeleteFunction;  
};

// Description:
// This function sets the map to have key of C string.
// If the key is not a C string (const char*), there will
// be a compile time error.
template<class KeyType, class DataType>
void vtkAbstractMapKeyIsString(vtkAbstractMap<KeyType,DataType>*);

// Description:
// This function sets the map to have a reference counted data.
// If the datra is not a reference counted object (pointer with 
// methods Register(void*) and UnRegister(void*)), there will
// be a compile time error.
template<class KeyType, class DataType>
void vtkAbstractMapDataIsReferenceCounted(vtkAbstractMap<KeyType,DataType>*);

#ifdef VTK_NO_EXPLICIT_TEMPLATE_INSTANTIATION
#include "vtkAbstractMap.txx"
#endif 

#endif
