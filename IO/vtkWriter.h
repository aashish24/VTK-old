/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

This file is part of the Visualization Library. No part of this file
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
// .NAME vlWriter - abstract class to write data to file(s)
// .SECTION Description
// vlWriter is an abstract class for mapper objects that write their data
// to disk (or into a communications port).

#ifndef __vlWriter_hh
#define __vlWriter_hh

#include "Object.hh"

class vlWriter : public vlObject 
{
public:
  char *GetClassName() {return "vlWriter";};
  void PrintSelf(ostream& os, vlIndent indent);

  // Description:
  // Write data to disk (or port).
  virtual void Write() = 0;

  void SetStartWrite(void (*f)(void *), void *arg);
  void SetEndWrite(void (*f)(void *), void *arg);

protected:
  void (*StartWrite)(void *);
  void *StartWriteArg;
  void (*EndWrite)(void *);
  void *EndWriteArg;

};

#endif


