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
#include "DS2DSF.hh"
#include "PolyData.hh"

vlDataSetToDataSetFilter::vlDataSetToDataSetFilter()
{
  // prevents dangling reference to DataSet
  this->DataSet = new vlPolyData;
  this->DataSet->Register((void *)this);
}

vlDataSetToDataSetFilter::~vlDataSetToDataSetFilter()
{
  this->DataSet->UnRegister((void *)this);
}

void vlDataSetToDataSetFilter::Update()
{
  vlPointData *pd;

  vlDataSetFilter::Update();
  // Following copies data from this filter to internal dataset
  pd = this->DataSet->GetPointData();
  *pd = this->PointData;
}

void vlDataSetToDataSetFilter::Initialize()
{
  if ( this->Input )
    {
    this->DataSet->UnRegister((void *)this);
    // copies input geometry to internal data set
    this->DataSet = this->Input->MakeObject(); 
    this->DataSet->Register((void *)this);
    }
  else
    {
    return;
    }
}

vlMapper *vlDataSetToDataSetFilter::MakeMapper()
{
//
// A little tricky because mappers must be of concrete type, but this class 
// deals at abstract level of DataSet.  Depending upon Input member of this 
// filter, mapper may change.  Hence need to anticipate change in Input and 
// create new mappers as necessary.
//
  vlMapper *mapper;

  vlDataSetToDataSetFilter::Update(); // compiler bug, had to hard code call
  mapper = this->DataSet->MakeMapper();
  if ( !this->Mapper || mapper != this->Mapper )
    {
    if (this->Mapper) this->Mapper->UnRegister((void *)this);
    this->Mapper = mapper;
    this->Mapper->Register((void *)this);
    }
  return this->Mapper;
}

void vlDataSetToDataSetFilter::PrintSelf(ostream& os, vlIndent indent)
{
  if (this->ShouldIPrint(vlDataSetToDataSetFilter::GetClassName()))
    {
    this->PrintWatchOn(); // watch for multiple inheritance

    vlDataSet::PrintSelf(os,indent);
    vlDataSetFilter::PrintSelf(os,indent);

    if ( this->DataSet )
      {
      os << indent << "DataSet: (" << this->DataSet << ")\n";
      os << indent << "DataSet type: " << this->DataSet->GetClassName() << "\n";
      }
    else
      {
      os << indent << "DataSet: (none)\n";
      }

    this->PrintWatchOff(); // stop worrying about it now
   }
}
