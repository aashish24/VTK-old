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

#include "vtkContainer.h"

#include "vtkDebugLeaks.h"

vtkContainer::vtkContainer() 
{ 
}

vtkContainer::~vtkContainer() 
{
}

int vtkContainerCompareMethod(const char* d1, const char* d2)
{ 
  if ( !d1 && !d2 ) 
    { 
    return 0;
    }
  if ( !d1 )
    {
    return 1;
    }
  if ( !d2 )
    {
    return -1;
    }
  return strcmp(d1, d2); 
}

int vtkContainerCompareMethod(char* d1, char* d2)
{ 
  if ( !d1 && !d2 ) 
    { 
    return 0;
    }
  if ( !d1 )
    {
    return 1;
    }
  if ( !d2 )
    {
    return -1;
    }
  return strcmp(d1, d2); 
}

const char* vtkContainerCreateMethod(const char* d1)
{ 
  if ( !d1 )
    {
    return 0;
    }
  char* str = new char[ strlen(d1) + 1];
  strcpy(str, d1);
  return str;
}

char* vtkContainerCreateMethod(char* d1)
{ 
  if ( !d1 )
    {
    return 0;
    }
  char* str = new char[ strlen(d1) + 1];
  strcpy(str, d1);
  return str;
}
