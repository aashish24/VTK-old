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
#include "vtkString.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkString, "$Revision$");
vtkStandardNewMacro(vtkString);
 
//----------------------------------------------------------------------------
// Description:
// This method returns the size of string. If the string is empty,
// it returns 0. It can handle null pointers.
vtkIdType vtkString::Length(const char* str)
{
  if ( !str )
    {
    return 0;
    }
  return static_cast<vtkIdType>(strlen(str));
}

  
//----------------------------------------------------------------------------
// Description:
// Copy string to the other string.
void vtkString::Copy(char* dest, const char* src)
{
  if ( !dest )
    {
    return;
    }
  if ( !src )
    {
    *dest = 0;
    return;
    }
  strcpy(dest, src);
}

//----------------------------------------------------------------------------
// Description:
// This method makes a duplicate of the string similar to
// C function strdup but it uses new to create new string, so
// you can use delete to remove it. It returns empty string 
// "" if the input is empty.
char* vtkString::Duplicate(const char* str)
{    
  if ( str )
    {
    char *newstr = new char [ vtkString::Length(str) + 1 ];
    vtkString::Copy(newstr, str);
    return newstr;
    }
  return 0;
}

//----------------------------------------------------------------------------
// Description:
// This method compare two strings. It is similar to strcmp,
// but it can handle null pointers.
int vtkString::Compare(const char* str1, const char* str2)
{
  if ( !str1 )
    {
    return -1;
    }
  if ( !str2 )
    {
    return 1;
    }
  return strcmp(str1, str2);
}

//----------------------------------------------------------------------------
// Description:
// Check if the first string starts with the second one.
int vtkString::StartsWith(const char* str1, const char* str2)
{
  if ( !str1 || !str2 || strlen(str1) < strlen(str2) )
    {
    return 0;
    }
  return !strncmp(str1, str2, strlen(str2));  
}

//----------------------------------------------------------------------------
// Description:
// Check if the first string starts with the second one.
int vtkString::EndsWith(const char* str1, const char* str2)
{
  if ( !str1 || !str2 || strlen(str1) < strlen(str2) )
    {
    return 0;
    }
  return !strncmp(str1 + (strlen(str1)-strlen(str2)), str2, strlen(str2));
}

//----------------------------------------------------------------------------
char* vtkString::Append(const char* str1, const char* str2)
{
  if ( !str1 && !str2 )
    {
    return 0;
    }
  char *newstr = 
    new char[ vtkString::Length(str1) + vtkString::Length(str2)+1];
  if ( !newstr )
    {
    return 0;
    }
  newstr[0] = 0;
  if ( str1 )
    {
    strcat(newstr, str1);
    }
  if ( str2 )
    {
    strcat(newstr, str2);
    }
  return newstr;
}

#if defined( _WIN32 ) && !defined(__CYGWIN__)
#  if defined(__BORLANDC__)
#    define STRCASECMP stricmp
#  else
#    define STRCASECMP _stricmp
#  endif
#else
#  define STRCASECMP strcasecmp
#endif

//----------------------------------------------------------------------------
int vtkString::CompareCase(const char* str1, const char* str2)
{
  if ( !str1 )
    {
    return -1;
    }
  if ( !str2 )
    {
    return 1;
    }
  return STRCASECMP(str1, str2);
}

//----------------------------------------------------------------------------
// Description:
// Transform the string to lowercase (inplace).
char* vtkString::ToLower(char *str)
{
  if (str)
    {
    char *ptr = str;
    while (*ptr)
      {
      *ptr = (char)tolower(*ptr);
      ++ptr;
      }
    }
  return str;
}

//----------------------------------------------------------------------------
// Description:
// Transform the string to uppercase (inplace).
char* vtkString::ToUpper(char *str)
{
  if (str)
    {
    char *ptr = str;
    while (*ptr)
      {
      *ptr = (char)toupper(*ptr);
      ++ptr;
      }
    }
  return str;
}
