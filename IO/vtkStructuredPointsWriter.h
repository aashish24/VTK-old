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
// .NAME vlStructuredPointsWriter - write vl structured points data file
// .SECTION Description
// vlStructuredPointsWriter is a source object that writes ASCII or binary 
// structured points data in vl file format. See text for format details.
// .SECTION Caveats
// Binary files written on one system may not be writeable on other systems.

#ifndef __vlStructuredPointsWriter_hh
#define __vlStructuredPointsWriter_hh

#include "vlDataW.hh"
#include "StrPts.hh"

class vlStructuredPointsWriter : public vlDataWriter
{
public:
  vlStructuredPointsWriter() {};
  ~vlStructuredPointsWriter() {};
  char *GetClassName() {return "vlStructuredPointsWriter";};
  void PrintSelf(ostream& os, vlIndent indent);

  void SetInput(vlStructuredPoints *input);
  void SetInput(vlStructuredPoints &input) {this->SetInput(&input);};
  vlStructuredPoints *GetInput() {return (vlStructuredPoints *)this->Input;};
                               
protected:
  void WriteData();

};

#endif


