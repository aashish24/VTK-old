/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkContainer - a base class for templated containers
// .SECTION Description
// vtkContainer is a superclass for all container classes. 
// Since it does not provide any actuall data access methods, it
// is not templated, but it provides a set of method that can 
// be used on all containers. It also provide a simple reference 
// counting scheme.

// .SECTION Caveates
// Since vtkContainer and vtkAbstractList provide some pure 
// virtual methods, each object of type container will have
// v-tabe.

// .SECTION See Also
// vtkAbstractIterator, vtkAbstractList, vtkAbstractMap

#include "vtkObject.h"
#include "vtkString.h"

#ifndef __vtkContainer_h
#define __vtkContainer_h

class VTK_COMMON_EXPORT vtkContainer
{
public:
  // Description:
  // Return the class name as a string.
  virtual const char* GetClassName() const { return "vtkContainer"; }

  // Description:
  // Return the number of items currently held in this container. This
  // different from GetSize which is provided for some containers. GetSize
  // will return how many items the container can currently hold.
  //virtual vtkIdType GetNumberOfItems() = 0;
  
  // Description:
  // Removes all items from the container.
  //virtual void RemoveAllItems() = 0;
  
  // Description:
  // The counterpart to New(), Delete simply calls UnRegister to lower the
  // reference count by one. It is no different than calling UnRegister.
  void Delete() { this->UnRegister(); }
  
  // Description:
  // Increase the reference count of this container.
  void Register();
  void Register(vtkObject *) { this->Register(); }
  
  // Description:
  // Decrease the reference count (release by another object). This has
  // the same effect as invoking Delete() (i.e., it reduces the reference
  // count by 1).
  void UnRegister();
  void UnRegister(vtkObject *) { this->UnRegister(); }

protected:
  vtkIdType ReferenceCount;   
  vtkContainer();
  virtual ~vtkContainer();
};

template<class DType>
int vtkContainerDefaultCompare(DType& k1, DType& k2)
{
  return ( k1 < k2 ) ? ( -1 ) : ( ( k1 == k2 ) ? ( 0 ) : ( 1 ) );
}

static inline int vtkContainerCompareMethod(vtkObject* d1, vtkObject* d2)
{ return vtkContainerDefaultCompare(d1,d2); }
static inline int vtkContainerCompareMethod(char d1, char d2) 
{ return vtkContainerDefaultCompare(d1,d2); }
static inline int vtkContainerCompareMethod(short d1, short d2)
{ return vtkContainerDefaultCompare(d1,d2); }
static inline int vtkContainerCompareMethod(int d1, int d2)
{ return vtkContainerDefaultCompare(d1,d2); }
static inline int vtkContainerCompareMethod(long d1, long d2)
{ return vtkContainerDefaultCompare(d1,d2); }
static inline int vtkContainerCompareMethod(unsigned char d1, unsigned char d2)
{ return vtkContainerDefaultCompare(d1,d2); }
static inline int vtkContainerCompareMethod(unsigned short d1, unsigned short d2)
{ return vtkContainerDefaultCompare(d1,d2); }
static inline int vtkContainerCompareMethod(unsigned int d1, unsigned int d2)
{ return vtkContainerDefaultCompare(d1,d2); }
static inline int vtkContainerCompareMethod(unsigned long d1, unsigned long d2)
{ return vtkContainerDefaultCompare(d1,d2); }
static inline int vtkContainerCompareMethod(float d1, float d2)
{ return vtkContainerDefaultCompare(d1,d2); }
static inline int vtkContainerCompareMethod(double d1, double d2)
{ return vtkContainerDefaultCompare(d1,d2); }
static inline int vtkContainerCompareMethod(const char* d1, const char* d2)
{ return vtkString::Compare(d1, d2); }
static inline int vtkContainerCompareMethod(char* d1, char* d2)
{ return vtkString::Compare(d1, d2); }
static inline int vtkContainerCompareMethod(void* d1, void* d2)
{ return vtkContainerDefaultCompare(d1,d2); }

template<class DType>
DType vtkContainerDefaultCreate(DType k2) { return k2; }

static inline vtkObject* vtkContainerCreateMethod(vtkObject* d1)
{ if ( d1) { d1->Register(0); } return d1; }
static inline char vtkContainerCreateMethod(char d1) 
{ return vtkContainerDefaultCreate(d1); }
static inline short vtkContainerCreateMethod(short d1)
{ return vtkContainerDefaultCreate(d1); }
static inline int vtkContainerCreateMethod(int d1)
{ return vtkContainerDefaultCreate(d1); }
static inline long vtkContainerCreateMethod(long d1)
{ return vtkContainerDefaultCreate(d1); }
static inline unsigned char vtkContainerCreateMethod(unsigned char d1)
{ return vtkContainerDefaultCreate(d1); }
static inline unsigned short vtkContainerCreateMethod(unsigned short d1)
{ return vtkContainerDefaultCreate(d1); }
static inline unsigned int vtkContainerCreateMethod(unsigned int d1)
{ return vtkContainerDefaultCreate(d1); }
static inline unsigned long vtkContainerCreateMethod(unsigned long d1)
{ return vtkContainerDefaultCreate(d1); }
static inline float vtkContainerCreateMethod(float d1)
{ return vtkContainerDefaultCreate(d1); }
static inline double vtkContainerCreateMethod(double d1)
{ return vtkContainerDefaultCreate(d1); }
static inline const char* vtkContainerCreateMethod(const char* d1)
{ return vtkString::Duplicate(d1); }
static inline char* vtkContainerCreateMethod(char* d1)
{ return vtkString::Duplicate(d1); }
static inline void* vtkContainerCreateMethod(void* d1)
{ return vtkContainerDefaultCreate(d1); }

static inline void vtkContainerDeleteMethod(vtkObject* d1) 
{ if ( d1 ) { d1->UnRegister(0); } /* cout << "UR(d1)" << endl; */ }
static inline void vtkContainerDeleteMethod(char) {}
static inline void vtkContainerDeleteMethod(short) {}
static inline void vtkContainerDeleteMethod(int) {}
static inline void vtkContainerDeleteMethod(long) {}
static inline void vtkContainerDeleteMethod(unsigned char) {}
static inline void vtkContainerDeleteMethod(unsigned short) {}
static inline void vtkContainerDeleteMethod(unsigned int) {}
static inline void vtkContainerDeleteMethod(unsigned long) {}
static inline void vtkContainerDeleteMethod(float) {}
static inline void vtkContainerDeleteMethod(double) {}
static inline void vtkContainerDeleteMethod(const char* d1) 
{ char *ch = const_cast<char*>(d1); delete [] ch; } 
static inline void vtkContainerDeleteMethod(char* d1) { delete [] d1; }
static inline void vtkContainerDeleteMethod(void*) {}


#endif 
