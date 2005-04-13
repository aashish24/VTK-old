/*=========================================================================

  Program:   KWSys - Kitware System Library
  Module:    $RCSfile$

  Copyright (c) Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "kwsysPrivate.h"
#include KWSYS_HEADER(Directory.hxx)
#include KWSYS_HEADER(Process.h)
#include KWSYS_HEADER(ios/iostream)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "Directory.hxx.in"
# include "Process.h.in"
# include "kwsys_ios_iostream.h.in"
#endif

int main()
{
  kwsys::Directory();
  kwsysProcess* kp = kwsysProcess_New();
  const char* cmd[] = {"echo", "Hello, World!", 0};
  kwsysProcess_SetCommand(kp, cmd);
  kwsysProcess_Execute(kp);
  char* data = 0;
  int length = 0;
  while(kwsysProcess_WaitForData(kp, &data, &length, 0))
    {
    kwsys_ios::cout.write(data, length);
    }
  kwsysProcess_WaitForExit(kp, 0);
  if(kwsysProcess_GetState(kp) == kwsysProcess_State_Error)
    {
    kwsys_ios::cout << kwsysProcess_GetErrorString(kp) << kwsys_ios::endl;
    }
  else if(kwsysProcess_GetState(kp) == kwsysProcess_State_Exception)
    {
    kwsys_ios::cout << kwsysProcess_GetExceptionString(kp) << kwsys_ios::endl;
    }
  kwsysProcess_Delete(kp);
  kwsys_ios::cout << kwsys_ios::endl;
  return 0;
}
