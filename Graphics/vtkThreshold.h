/*=========================================================================

  Program:   Visualization Library
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Description:
---------------------------------------------------------------------------
This file is part of the Visualization Library. No part of this file
or its contents may be copied, reproduced or altered in any way
without the express written consent of the authors.

Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen 1993, 1994 

=========================================================================*/
//
// Extracts cells where scalar value of every point in cell satisfies 
// threshold criterion. 
//
#ifndef __vlThreshold_h
#define __vlThreshold_h

#include "DS2UGrid.hh"

class vlThreshold : public vlDataSetToUnstructuredGridFilter
{
public:
  vlThreshold();
  ~vlThreshold() {};
  char *GetClassName() {return "vlThreshold";};
  void PrintSelf(ostream& os, vlIndent indent);

/*  void ThresholdByLower(float lower) 
    {this->LowerThreshold = lower; this->ThresholdFunction = &(this->Lower);};
                           
  void ThresholdByUpper(float upper)
    {this->UpperThreshold = upper; 
     this->ThresholdFunction = Upper;};
                           
  void ThresholdBetween(float lower, float upper)
    {this->LowerThreshold = lower; this->UpperThreshold = upper;
     this->ThresholdFunction = this->Between;};
*/  
  vlGetMacro(UpperThreshold,float);
  vlGetMacro(LowerThreshold,float);

protected:
  // Usual data generation method
  void Execute();

  float LowerThreshold;
  float UpperThreshold;

  int (*ThresholdFunction)(float s);
  int Lower(float s) {return ( s <= this->LowerThreshold ? 1 : 0 );};
  int Upper(float s) {return ( s >= this->UpperThreshold ? 1 : 0 );};
  int Between(float s) {return ( s >= this->LowerThreshold ? 
                                 ( s <= this->UpperThreshold ? 1 : 0 ) : 0 );};
};

#endif


