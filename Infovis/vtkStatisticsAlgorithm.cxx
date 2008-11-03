/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/

#include "vtkToolkits.h"

#include "vtkStatisticsAlgorithm.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStringArray.h"
#include "vtkTable.h"

vtkCxxRevisionMacro(vtkStatisticsAlgorithm, "$Revision$");

// ----------------------------------------------------------------------
vtkStatisticsAlgorithm::vtkStatisticsAlgorithm()
{
  this->SetNumberOfInputPorts( 2 );
  this->SetNumberOfOutputPorts( 3 );

  // If not told otherwise, only run Learn option
  this->Learn = true;
  this->Derive = true;
  this->Assess = false;
  this->AssessNames = vtkStringArray::New();
  this->AssessParameters = 0;
}

// ----------------------------------------------------------------------
vtkStatisticsAlgorithm::~vtkStatisticsAlgorithm()
{
  if ( this->AssessNames )
    {
    this->AssessNames->Delete();
    }
}

// ----------------------------------------------------------------------
void vtkStatisticsAlgorithm::PrintSelf( ostream &os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
  os << indent << "NumberOfVariables: " << this->NumberOfVariables << endl;
  os << indent << "SampleSize: " << this->SampleSize << endl;
  os << indent << "Learn: " << this->Learn << endl;
  os << indent << "Derive: " << this->Derive << endl;
  os << indent << "Assess: " << this->Assess << endl;
  if ( this->AssessParameters )
    {
    this->AssessParameters->PrintSelf( os, indent.GetNextIndent() );
    }
  if ( this->AssessNames )
    {
    this->AssessNames->PrintSelf( os, indent.GetNextIndent() );
    }
}

// ----------------------------------------------------------------------
void vtkStatisticsAlgorithm::SetAssessParameter( vtkIdType id, vtkStdString name )
{
  if ( id >= 0 && id < this->AssessParameters->GetNumberOfValues() )
    {
    this->AssessParameters->SetValue( id, name );
    this->Modified();
    }
} 

// ----------------------------------------------------------------------
vtkStdString vtkStatisticsAlgorithm::SetAssessParameter( vtkIdType id )
{
  if ( id >= 0 && id < this->AssessParameters->GetNumberOfValues() )
    {
    return this->AssessParameters->GetValue( id );
    }
  return 0;
} 

// ----------------------------------------------------------------------
int vtkStatisticsAlgorithm::RequestData( vtkInformation*,
                                         vtkInformationVector** inputVector,
                                         vtkInformationVector* outputVector )
{
  // Extract input data table
  vtkTable* inData = vtkTable::GetData( inputVector[0], 0 );
  if ( ! inData )
    {
    return 1;
    }

  this->SampleSize = inData->GetNumberOfRows();

  // Extract output tables
  vtkTable* outData = vtkTable::GetData( outputVector, 0 );
  vtkTable* outMeta1 = vtkTable::GetData( outputVector, 1 );
  vtkTable* outMeta2 = vtkTable::GetData( outputVector, 2 );

  outData->ShallowCopy( inData );

  vtkTable* inMeta;
  if ( this->Learn )
    {
    this->ExecuteLearn( inData, outMeta1 );
    }
  else
    {
    // Extract input meta table
    inMeta = vtkTable::GetData( inputVector[1], 0 );

    if ( ! inMeta )
      {
      vtkWarningMacro( "No model available. Doing nothing." );
      return 1;
      }

    outMeta1->ShallowCopy( inMeta );
    }

  if ( this->Derive )
    {
    this->ExecuteDerive( outMeta1 );
    }

  if ( this->Assess )
    {
    this->ExecuteAssess( inData, outMeta1, outData, outMeta2 );
    }

  return 1;
}

// ----------------------------------------------------------------------
int vtkStatisticsAlgorithm::FillInputPortInformation( int port, vtkInformation* info )
{
  if ( port == 0 )
    {
    info->Set( vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkTable" );
    return 1;
    }
  else if ( port == 1 )
    {
    info->Set( vtkAlgorithm::INPUT_IS_OPTIONAL(), 1 );
    info->Set( vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkTable" );
    return 1;
    }

  return 0;
}

// ----------------------------------------------------------------------
int vtkStatisticsAlgorithm::FillOutputPortInformation( int port, vtkInformation* info )
{
  if ( port >= 0 )
    {
    info->Set(vtkDataObject::DATA_TYPE_NAME(), "vtkTable" );
    return 1;
    }

  return 0;
}

//---------------------------------------------------------------------------
void vtkStatisticsAlgorithm::SetInputStatisticsConnection( vtkAlgorithmOutput* in )
{ 
  this->SetInputConnection( 1, in );
}
