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
/*
 * Copyright 2009 Sandia Corporation.
 * Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
 * license for use of this work by or on behalf of the
 * U.S. Government. Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that this Notice and any
 * statement of authorship are reproduced on all copies.
 */
// .SECTION Thanks
// Thanks to Philippe Pebay for implementing this test.

#include <mpi.h>
#include <time.h>

#include "vtkContingencyStatistics.h"
#include "vtkPContingencyStatistics.h"

#include "vtkDoubleArray.h"
#include "vtkIdTypeArray.h"
#include "vtkIntArray.h"
#include "vtkMath.h"
#include "vtkMPIController.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkStdString.h"
#include "vtkTable.h"
#include "vtkVariantArray.h"

// For debugging purposes, output contingency table, which may be huge: it has the size O(span^2).
#define DEBUG_CONTINGENCY_TABLE 0

struct RandomContingencyStatisticsArgs
{
  int nVals;
  double span;
  int* retVal;
  int ioRank;
  int argc;
  char** argv;
};

// This will be called by all processes
void RandomContingencyStatistics( vtkMultiProcessController* controller, void* arg )
{
  // Get test parameters
  RandomContingencyStatisticsArgs* args = reinterpret_cast<RandomContingencyStatisticsArgs*>( arg );
  *(args->retVal) = 0;

  // Get MPI communicator
  vtkMPICommunicator* com = vtkMPICommunicator::SafeDownCast( controller->GetCommunicator() );

  // Get local rank
  int myRank = com->GetLocalProcessId();

  // Seed random number generator
  vtkMath::RandomSeed( static_cast<int>( time( NULL ) ) * ( myRank + 1 ) );

  // Generate an input table that contains samples of mutually independent discrete random variables
  int nVariables = 2;
  vtkIntArray* intArray[2];
  vtkStdString columnNames[] = { "Rounded Normal 0", 
                                 "Rounded Normal 1" };
  
  vtkTable* inputData = vtkTable::New();
  // Discrete rounded normal samples
  for ( int c = 0; c < nVariables; ++ c )
    {
    intArray[c] = vtkIntArray::New();
    intArray[c]->SetNumberOfComponents( 1 );
    intArray[c]->SetName( columnNames[c] );

    for ( int r = 0; r < args->nVals; ++ r )
      {
      intArray[c]->InsertNextValue( static_cast<int>( round( vtkMath::Gaussian() * args->span ) ) );
      }
    
    inputData->AddColumn( intArray[c] );
    intArray[c]->Delete();
    }

  // ************************** Contingency Statistics ************************** 

  // Synchronize and start clock
  com->Barrier();
  time_t t0;
  time ( &t0 );

  // Instantiate a parallel contingency statistics engine and set its ports
  vtkPContingencyStatistics* pcs = vtkPContingencyStatistics::New();
  pcs->SetInput( 0, inputData );
  vtkMultiBlockDataSet* outputMetaDS = vtkMultiBlockDataSet::SafeDownCast( pcs->GetOutputDataObject( 1 ) );

  // Select column pairs (uniform vs. uniform, normal vs. normal)
  pcs->AddColumnPair( columnNames[0], columnNames[1] );

  // Test (in parallel) with Learn, Derive, and Assess options turned on
  pcs->SetLearn( true );
  pcs->SetDerive( true );
  pcs->SetAssess( true );
  pcs->Update();

    // Synchronize and stop clock
  com->Barrier();
  time_t t1;
  time ( &t1 );

  if ( com->GetLocalProcessId() == args->ioRank )
    {
    cout << "\n## Completed parallel calculation of contingency statistics (with assessment):\n"
         << "   Wall time: "
         << difftime( t1, t0 )
         << " sec.\n";

    vtkTable* outputMeta = vtkTable::SafeDownCast( outputMetaDS->GetBlock( 0 ) );
    outputMeta->Dump();
    }

  // Verify that the distributed reduced contingency tables all result in a CDF value of 1.
  if ( com->GetLocalProcessId() == args->ioRank )
    {
    cout << "\n## Verifying that joint probabilities sum to 1.\n";
    }
  
  vtkTable* outputContingency = vtkTable::SafeDownCast( outputMetaDS->GetBlock( 1 ) );
  vtkIdTypeArray* keys = vtkIdTypeArray::SafeDownCast( outputContingency->GetColumnByName( "Key" ) );
  if ( ! keys )
    {
    cout << "*** Error: "
         << "Empty contingency table column 'Key' on process "
         << com->GetLocalProcessId()
         << ".\n";
    }

  vtkStdString proName = "P";
  vtkDoubleArray* prob = vtkDoubleArray::SafeDownCast( outputContingency->GetColumnByName( proName ) );
  if ( ! prob )
    {
    cout << "*** Error: "
         << "Empty contingency table column '"
         << proName
         << "' on process "
         << com->GetLocalProcessId()
         << ".\n";
    }

  // Calculate local CDFs
  double cdf_l = 0.;
  vtkIdType key = 0;
  int n = outputContingency->GetNumberOfRows();
  vtkIdType k;

  // Skip first entry which is reserved for the cardinality
  for ( vtkIdType r = 1; r < n; ++ r )
    {
    k = keys->GetValue( r );
    
    if ( k == key )
      {
      cdf_l += prob->GetValue( r );
      }
    }

  // Gather all local CDFs
  int numProcs = controller->GetNumberOfProcesses();
  double* cdf_g = new double[numProcs];
  com->AllGather( &cdf_l, 
                  cdf_g, 
                  1 );

  // Print out all CDFs
  if ( com->GetLocalProcessId() == args->ioRank )
    {
    for ( int i = 0; i < numProcs; ++ i )
      {
      cout << "   On process "
           << i
           << ", CDF = "
           << cdf_g[i]
           << "\n";
      
      if ( fabs ( 1. - cdf_g[i] ) > 1.e-6 )
        {
        vtkGenericWarningMacro("Incorrect CDF.");
        *(args->retVal) = 1;
        }
      }
    }
  
#if DEBUG_CONTINGENCY_TABLE
  outputContingency->Dump();
#endif // DEBUG_CONTINGENCY_TABLE

  // Clean up
  delete [] cdf_g;
  pcs->Delete();
  inputData->Delete();
}

//----------------------------------------------------------------------------
int main( int argc, char** argv )
{
  // **************************** MPI Initialization *************************** 
  vtkMPIController* controller = vtkMPIController::New();
  controller->Initialize( &argc, &argv );

  // If an MPI controller was not created, terminate in error.
  if ( ! controller->IsA( "vtkMPIController" ) )
    {
    vtkGenericWarningMacro("Failed to initialize a MPI controller.");
    controller->Delete();
    return 1;
    } 

  vtkMPICommunicator* com = vtkMPICommunicator::SafeDownCast( controller->GetCommunicator() );

  // ************************** Find an I/O node ******************************** 
  int* ioPtr;
  int ioRank;
  int flag;

  MPI_Attr_get( MPI_COMM_WORLD, 
                MPI_IO,
                &ioPtr,
                &flag );

  if ( ( ! flag ) || ( *ioPtr == MPI_PROC_NULL ) )
    {
    // Getting MPI attributes did not return any I/O node found.
    ioRank = MPI_PROC_NULL;
    vtkGenericWarningMacro("No MPI I/O nodes found.");

    // As no I/O node was found, we need an unambiguous way to report the problem.
    // This is the only case when a testValue of -1 will be returned
    controller->Finalize();
    controller->Delete();
    
    return -1;
    }
  else 
    {
    if ( *ioPtr == MPI_ANY_SOURCE )
      {
      // Anyone can do the I/O trick--just pick node 0.
      ioRank = 0;
      }
    else
      {
      // Only some nodes can do I/O. Make sure everyone agrees on the choice (min).
      com->AllReduce( ioPtr,
                      &ioRank,
                      1,
                      vtkCommunicator::MIN_OP );
      }
    }

  // ************************** Initialize test ********************************* 
  if ( com->GetLocalProcessId() == ioRank )
    {
    cout << "\n# Process "
         << ioRank
         << " will be the I/O node.\n";
    }
      
  // Check how many processes have been made available
  int numProcs = controller->GetNumberOfProcesses();
  if ( controller->GetLocalProcessId() == ioRank )
    {
    cout << "\n# Running test with "
         << numProcs
         << " processes...\n";
    }

  // Parameters for regression test.
  int testValue = 0;
  RandomContingencyStatisticsArgs args;
  args.nVals = 100000;
  args.span = 50.;
  args.retVal = &testValue;
  args.ioRank = ioRank;
  args.argc = argc;
  args.argv = argv;

  // Execute the function named "process" on both processes
  controller->SetSingleMethod( RandomContingencyStatistics, &args );
  controller->SingleMethodExecute();

  // Clean up and exit
  if ( com->GetLocalProcessId() == ioRank )
    {
    cout << "\n# Test completed.\n\n";
    }

  controller->Finalize();
  controller->Delete();
  
  return testValue;
}
