/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Library. No part of this file or its 
contents may be copied, reproduced or altered in any way without the express
written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
// .NAME SetGet Macros - standard macros for setting/getting instance variables
// .SECTION Description
// The SetGet macros are used to interface to instance variables
// in a standard fashion. This includes properly treating modified time
// and printing out debug information.
//    Macros are available for built-in types; for character strings; 
// vector arrays of built-in types size 2,3,4; for setting objects; and
// debug, warning, and error printout information.

#ifndef __vlSetGet_hh
#define __vlSetGet_hh

#include <string.h>

//
// Some constants used throughout code
//
#define LARGE_FLOAT 1.0e29
#define LARGE_INTEGER 2147483646 // 2^31 - 1

//
// Set built-in type.  Creates member Set"name"() (e.g., SetVisibility());
//
#define vlSetMacro(name,type) \
void Set##name (type _arg) \
  { \
  if (Debug)   cerr << "In " __FILE__ << ", line " << __LINE__ << "\n" << this->GetClassName() << " (" << this << "): setting " << #name " to " << _arg << "\n\n"; \
  if (name != _arg) \
    { \
    name = _arg; \
    this->Modified(); \
    } \
  } 

//
// Get built-in type.  Creates member Get"name"() (e.g., GetVisibility());
//
#define vlGetMacro(name,type) \
type Get##name () { \
  if (Debug)   cerr << "In " __FILE__ << ", line " << __LINE__ << "\n" << this->GetClassName() << " (" << this << "): returning " << #name " of " << name << "\n\n"; \
  return name; \
  } 

//
// Set character string.  Creates member Set"name"() 
// (e.g., SetFilename(char *));
//
#define vlSetStringMacro(name) \
void Set##name (char* _arg) \
  { \
  if (Debug)   cerr << "Debug: In " __FILE__ << ", line " << __LINE__ << "\n" << this->GetClassName() << " (" << this << "): setting " << #name " to " << _arg << "\n\n"; \
  if ( name && _arg ) \
    if ( !strcmp(name,_arg) ) return; \
  if (name) delete [] name; \
  if (_arg) \
    { \
    name = new char[strlen(_arg)+1]; \
    strcpy(name,_arg); \
    } \
   else \
    { \
    name = NULL; \
    } \
  this->Modified(); \
  } 

//
// Get character string.  Creates member Get"name"() 
// (e.g., char *GetFilename());
//
#define vlGetStringMacro(name) \
char* Get##name () { \
  if (Debug)   cerr << "Debug: In " __FILE__ << ", line " << __LINE__ << "\n" << this->GetClassName() << " (" << this << "): returning " << #name " of " << name << "\n\n"; \
  return name; \
  } 

//
// Set built-in type where value is constrained between min/max limits.
// Create member Set"name"() (e.q., SetRadius()). #defines are 
// convienience for clamping open-ended values.
//
#define vlSetClampMacro(name,type,min,max) \
void Set##name (type _arg) \
  { \
  if (Debug)   cerr << "Debug: In " __FILE__ << ", line " << __LINE__ << "\n" << this->GetClassName() << " (" << this << "): setting " << #name " to " << _arg << "\n\n"; \
  if (name != _arg) \
    { \
    name = (_arg<min?min:(_arg>max?max:_arg)); \
    this->Modified(); \
    } \
  } 

//
// Set pointer to object. Creates method Set"name"() (e.g., SetPoints()).
//
#define vlSetObjectMacro(name,type) \
void Set##name (type* _arg) \
  { \
  if (Debug)   cerr << "Debug: In " __FILE__ << ", line " << __LINE__ << "\n" << this->GetClassName() << " (" << this << "): setting " << #name " to " << &_arg << "\n\n"; \
  if (name != _arg) \
    { \
    name = _arg; \
    this->Modified(); \
    } \
  } \
void Set##name (type& _arg) \
  { \
  Set##name (&_arg);\
  } 

//
// Set pointer to object; uses vlRefCount reference counting methodology.
// Creates method Set"name"() (e.g., SetPoints()).
//
#define vlSetRefCountedObjectMacro(name,type) \
void Set##name (type* _arg) \
  { \
  if (Debug)   cerr << "Debug: In " __FILE__ << ", line " << __LINE__ << "\n" << this->GetClassName() << " (" << this << "): setting " << #name " to " << &_arg << "\n\n"; \
  if (name != _arg) \
    { \
    if (name != NULL) name->UnRegister(this); \
    name = _arg; \
    if (name != NULL) name->Register(this); \
    this->Modified(); \
    } \
  } 

//
// Get pointer to object.  Creates member Get"name" (e.g., GetPoints()).
//
#define vlGetObjectMacro(name,type) \
type *Get##name () \
  { \
  if (Debug)   cerr << "Debug: In " __FILE__ << ", line " << __LINE__ << "\n" << this->GetClassName() << " (" << this << "): returning " << #name " address " << name << "\n\n"; \
  return name; \
  } 

//
// Create members "name"On() and "name"Off() (e.g., DebugOn() DebugOff()).
// Set method must be defined to use this macro.
//
#define vlBooleanMacro(name,type) \
void name##On () { Set##name((type)1);}; \
void name##Off () { Set##name((type)0);}

//
// Following set macros for vectors define two members for each macro.  The first 
// allows setting of individual components (e.g, SetColor(float,float,float)), 
// the second allows setting from an array (e.g., SetColor(float* rgb[3])).
// The macros vary in the size of the vector they deal with.
//
#define vlSetVector2Macro(name,type) \
void Set##name (type _arg1, type _arg2) \
  { \
  if (Debug)   cerr << "Debug: In " __FILE__ << ", line " << __LINE__ << "\n" << this->GetClassName() << " (" << this << "): setting " << #name " to (" << _arg1 << "," << _arg2 << ")\n\n"; \
  if ((name[0] != _arg1)||(name[1] != _arg2)) \
    { \
    this->Modified(); \
    name[0] = _arg1; \
    name[1] = _arg2; \
    } \
  }; \
void Set##name (type _arg[2]) \
  { \
  Set##name (_arg[0], _arg[1]); \
  } 

#define vlSetVector3Macro(name,type) \
void Set##name (type _arg1, type _arg2, type _arg3) \
  { \
  if (Debug)   cerr << "Debug: In " __FILE__ << ", line " << __LINE__ << "\n" << this->GetClassName() << " (" << this << "): setting " << #name " to (" << _arg1 << "," << _arg2 << "," << _arg3 << ")\n\n"; \
  if ((name[0] != _arg1)||(name[1] != _arg2)||(name[2] != _arg3)) \
    { \
    this->Modified(); \
    name[0] = _arg1; \
    name[1] = _arg2; \
    name[2] = _arg3; \
    } \
  }; \
void Set##name (type _arg[3]) \
  { \
  Set##name (_arg[0], _arg[1], _arg[2]);\
  } 

#define vlSetVector4Macro(name,type) \
void Set##name (type _arg1, type _arg2, type _arg3, type _arg4) \
  { \
  if (Debug)   cerr << "Debug: In " __FILE__ << ", line " << __LINE__ << "\n" << this->GetClassName() << " (" << this << "): setting " << #name " to (" << _arg1 << "," << _arg2 << "," << _arg3 << "," << _arg4 << ")\n\n"; \
  if ((name[0] != _arg1)||(name[1] != _arg2)||(name[2] != _arg3)||(name[3] != _arg4)) \
    { \
    this->Modified(); \
    name[0] = _arg1; \
    name[1] = _arg2; \
    name[2] = _arg3; \
    name[3] = _arg4; \
    } \
  }; \
void Set##name (type _arg[4]) \
  { \
  Set##name (_arg[0], _arg[1], _arg[2], _arg[3]);\
  } 

//
// General set vector macro creates a single method that copies specified
// number of values into object.
// Examples: void SetColor(c,3)
//
#define vlSetVectorMacro(name,type,count) \
void Set##name(type data[], int count) \
{ \
  for (int i=0; i<count; i++) if ( data[i] != name[i] ) break; \
  if ( i < count ) \
    { \
    this->Modified(); \
    for (i=0; i<count; i++) name[i] = data[i]; \
    } \
}

//
// Get vector macro defines two methods. One returns pointer to type 
// (i.e., array of type). This is for efficiency. The second copies data
// into user provided array. This is more object-oriented.
// Examples: float *GetColor() and void GetColor(float c[count]).
//
#define vlGetVectorMacro(name,type,count) \
type *Get##name () \
{ \
  if (Debug)   cerr << "Debug: In " __FILE__ << ", line " << __LINE__ << "\n" << this->GetClassName() << " (" << this << "): returning " << #name " pointer " << name << "\n\n"; \
  return name; \
} \
void Get##name (type data[count]) \
{ \
  for (int i=0; i<count; i++) data[i] = name[i]; \
}

//
// This macro is used for  debug statements in instance methods
// vlDebugMacro(<< "this is debug info" << this->SomeVariable);
//
#define vlDebugMacro(x) \
  if (Debug) cerr << "Debug: In " __FILE__ << ", line " << __LINE__ << "\n" << this->GetClassName() << " (" << this << "): " x << "\n\n"

//
// This macro is used to print out warning messages.
// vlWarningMacro(<< "Warning message" << variable);
//
#define vlWarningMacro(x) \
  cerr << "Warning: In " __FILE__ << ", line " << __LINE__ << "\n" << this->GetClassName() << " (" << this << "): " x << "\n\n"

//
// This macro is used to print out errors
// vlErrorMacro(<< "Error message" << variable);
//
#define vlErrorMacro(x) \
  cerr << "ERROR In " __FILE__ << ", line " << __LINE__ << "\n" << this->GetClassName() << " (" << this << "): " x << "\n\n"

#endif
