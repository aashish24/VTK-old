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

// .NAME TestCxxFeatures
// .SECTION Description
// Provides a reference for the set of C++ features that can be used
// by VTK.

#include "vtkSystemIncludes.h"

//----------------------------------------------------------------------------

/* Test full template specialization of functions.  */
template <class T>
int FullySpecializedFunction(T*)
{
  return 0;
}

#if 0
// Fails on kulu.crd IRIX64-6.5-CC-o3 (old SGI compiler).
template <>
int FullySpecializedFunction<int>(int*)
{
  return 1;
}
#else
// Let overload resolution pick this one instead.
int FullySpecializedFunction(int*)
{
  return 1;
}
#endif

int TestFullySpecializedFunction()
{
  int result = 1;
  int should_be_0 = FullySpecializedFunction(static_cast<float*>(0));
  if(should_be_0 != 0)
    {
    cerr << "FullySpecializedFunction<float*>() returned "
         << should_be_0 << ", not 0.\n";
    result = 0;
    }
  int should_be_1 = FullySpecializedFunction(static_cast<int*>(0));
  if(should_be_1 != 1)
    {    
    cerr << "FullySpecializedFunction(int*) returned "
         << should_be_1 << ", not 1.\n";
    result = 0;
    }
  return result;
}

//----------------------------------------------------------------------------

/* Test use of standard "bool" type and values.  */

#if 0
// Fails on kulu.crd IRIX64-6.5-CC-o3 (old SGI compiler).
bool GetFalse()
{
  return false;
}

bool GetTrue()
{
  return true;
}

int TestBool()
{
  int result = 1;
  bool should_be_false = GetFalse();
  bool should_be_true = GetTrue();
  if(should_be_false)
    {
    cerr << "GetFalse() returned " << should_be_false << ", not false.\n";
    result = 0;
    }
  if(!should_be_true)
    {
    cerr << "GetTrue() returned " << should_be_true << ", not true.\n";
    result = 0;
    }
  return result;
}
#endif
//----------------------------------------------------------------------------

/* Test full template specialization of classes.  */

#if 0
// Fails on kulu.crd IRIX64-6.5-CC-o3 (old SGI compiler).
template <class T>
struct FullySpecializedClass
{
  static int Method() { return 0; }
};

template <>
struct FullySpecializedClass<int>
{
  static int Method() { return 1; }
};

int TestFullySpecializedClass()
{
  int result = 1;
  int should_be_0 = FullySpecializedClass<float>::Method();
  if(should_be_0 != 0)
    {
    cerr << "FullySpecializedClass<float>::Method() returned "
         << should_be_0 << ", not 0.\n";
    result = 0;
    }
  int should_be_1 = FullySpecializedClass<int>::Method();
  if(should_be_1 != 1)
    {    
    cerr << "FullySpecializedClass<int>::Method() returned "
         << should_be_1 << ", not 1.\n";
    result = 0;
    }
  return result;
}
#endif
//----------------------------------------------------------------------------

/* Test if(int x = f()) style scoping.  */

int TestIfScopeHelper(int i)
{
  int result = 1;
  if(int x = i)
    {
    if(x != i)
      {
      cerr << "TestIfScope: x != " << i << "\n";
      result = 0;
      }
    }
  else
    {
    if(x != i)
      {
      cerr << "TestIfScope: x != " << i << "\n";
      result = 0;
      }
    }
  return result;
}

int TestIfScope()
{
  int result = 1;
  if(!TestIfScopeHelper(1))
    {
    result = 0;
    }
  if(!TestIfScopeHelper(0))
    {
    result = 0;
    }
  return result;
}

//----------------------------------------------------------------------------

int main()
{
  int result = 0;
  if(!TestFullySpecializedFunction())
    {
    result = 1;
    }
#if 0
  // Fails on kulu.crd IRIX64-6.5-CC-o3 (old SGI compiler).
  if(!TestBool())
    {
    result = 1;
    }
#endif
#if 0
  // Fails on kulu.crd IRIX64-6.5-CC-o3 (old SGI compiler).
  if(!TestFullySpecializedClass())
    {
    result = 1;
    }
#endif
  if(!TestIfScope())
    {
    result = 1;
    }
  return result;
}
