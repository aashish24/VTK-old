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
// .NAME vtkIdType - Integer type used for point and cell IDs.
// .SECTION Description
// vtkIdType is the integer type used for point and cell
// identification.  This type may be either 32-bit or 64-bit,
// depending on whether VTK_USE_64BIT_IDS is defined.

#ifndef __vtkIdType_h
#define __vtkIdType_h

#ifndef __VTK_SYSTEM_INCLUDES__INSIDE
Do_not_include_vtkIdType_directly__vtkSystemIncludes_includes_it;
#endif

// Choose an implementation for vtkIdType.
#define VTK_HAS_ID_TYPE
#ifdef VTK_USE_64BIT_IDS
# define VTK_ID_TYPE_IS_NOT_BASIC_TYPE
# define VTK_SIZEOF_ID_TYPE 8
# ifdef _WIN32
typedef __int64 vtkIdType;
#  define VTK_NEED_ID_TYPE_STREAM_OPERATORS
# else // _WIN32
typedef long long vtkIdType;
#  define VTK_NEED_ID_TYPE_STREAM_OPERATORS
# endif // _WIN32
#else // VTK_USE_64BIT_IDS
# define VTK_SIZEOF_ID_TYPE VTK_SIZEOF_INT
typedef int vtkIdType;
#endif // VTK_USE_64BIT_IDS

// Visual Studio 6 does not provide these operators.
#if defined(VTK_USE_64BIT_IDS) && defined(_MSC_VER) && (_MSC_VER < 1300)
# if !defined(VTK_NO_INT64_OSTREAM_OPERATOR)
VTK_COMMON_EXPORT ostream& vtkIdTypeOutput(ostream& os, __int64 id);
inline ostream& operator << (ostream& os, __int64 id)
{
  return vtkIdTypeOutput(os, id);
}
# endif
# if !defined(VTK_NO_INT64_ISTREAM_OPERATOR)
VTK_COMMON_EXPORT istream& vtkIdTypeInput(istream& is, __int64& id);
inline istream& operator >> (istream& is, __int64& id)
{
  return vtkIdTypeInput(is, id);
}
# endif
#endif

#endif
