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

#include "Object.hh"

// Description:
// Operator allows all subclasses of vlObject to be printed via <<.
ostream& operator<<(ostream& os, vlObject& o)
{
  o.Print(os);
  return os;
}

vlObject::vlObject()
{
  this->RefCount = 0;
  this->Debug = 0;

  this->Modified(); // Insures modified time > than any other time
}

vlObject::~vlObject() 
{
  if (this->RefCount > 0)
    {
    vlErrorMacro(<< "Trying to delete object with non-zero refCount");
    }

  vlDebugMacro(<< "Destructing!");
}

// Description:
// Increase the reference count (mark as used by another object).
void vlObject::Register(vlObject* o)
{
  this->RefCount++;
  vlDebugMacro(<< "Registered by " << o->GetClassName() << " (" << o << ")");
}

// Description:
// Decrease the reference count (release by another object).
void vlObject::UnRegister(vlObject* o)
{
  vlDebugMacro(<< "UnRegistered by " << o->GetClassName() << " (" << 0 << ")");

  if (--this->RefCount <= 0) delete this;
}

void vlObject::Print(ostream& os)
{
  vlIndent indent;

  this->PrintHeader(os,0); 
  this->PrintSelf(os, indent.GetNextIndent());
  this->PrintTrailer(os,0);
}

void vlObject::PrintHeader(ostream& os, vlIndent indent)
{
  os << indent << this->GetClassName() << " (" << this << ")\n";
}

// Description:
// Chaining method to print object instance variables as well as
// superclasses.
void vlObject::PrintSelf(ostream& os, vlIndent indent)
{
  if (this->ShouldIPrint(vlObject::GetClassName()))
    {
    os << indent << "Debug: " << (this->Debug ? "On\n" : "Off\n");
    os << indent << "Modified Time: " << this->GetMTime() << "\n";
    os << indent << "Reference Count: " << this->RefCount << "\n";
    }
}

void vlObject::PrintTrailer(ostream& os, vlIndent indent)
{
  os << indent << "\n";
}

// Description:
// Turn debug printout on.
void vlObject::DebugOn()
{
  this->Debug = 1;
}

// Description:
// Turn debug printout off.
void vlObject::DebugOff()
{
  this->Debug = 0;
}

// Description:
// Get the value of the debug flag.
int vlObject::GetDebug()
{
  return this->Debug;
}
