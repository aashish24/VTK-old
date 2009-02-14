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
 * Copyright 2008 Sandia Corporation.
 * Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
 * license for use of this work by or on behalf of the
 * U.S. Government. Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that this Notice and any
 * statement of authorship are reproduced on all copies.
 */
// .SECTION Thanks
// Thanks to Philippe Pebay, David Thompson and Janine Bennett from Sandia National Laboratories 
// for implementing this test.

#include <mpi.h>
#include <time.h>

#include "vtkDescriptiveStatistics.h"
#include "vtkPDescriptiveStatistics.h"
#include "vtkCorrelativeStatistics.h"
#include "vtkPCorrelativeStatistics.h"
#include "vtkMultiCorrelativeStatistics.h"
#include "vtkPMultiCorrelativeStatistics.h"
#include "vtkPPCAStatistics.h"

#include "vtkDoubleArray.h"
#include "vtkMath.h"
#include "vtkMPIController.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkStdString.h"
#include "vtkTable.h"
#include "vtkVariantArray.h"

// For debugging purposes, serial engines can be run on each slice of the distributed data set
#define DEBUG_WITH_SERIAL_STATS 0 

struct RandomSampleStatisticsArgs
{
  int nVals;
  int* retVal;
  int ioRank;
  int argc;
  char** argv;
};

// This will be called by all processes
void RandomSampleStatistics( vtkMultiProcessController* controller, void* arg )
{
  // Get test parameters
  RandomSampleStatisticsArgs* args = reinterpret_cast<RandomSampleStatisticsArgs*>( arg );
  *(args->retVal) = 0;

  // Get MPI communicator
  vtkMPICommunicator* com = vtkMPICommunicator::SafeDownCast( controller->GetCommunicator() );

  // Get local rank
  int myRank = com->GetLocalProcessId();

  // Seed random number generator
  vtkMath::RandomSeed( static_cast<int>( time( NULL ) ) * ( myRank + 1 ) );

  // Generate an input table that contains samples of mutually independent random variables over [0, 1]
  int nUniform = 2;
  int nNormal  = 2;
  int nVariables = nUniform + nNormal;

  vtkTable* inputData = vtkTable::New();
  vtkDoubleArray* doubleArray[4];
  vtkStdString columnNames[] = { "Standard Uniform 0", 
                                 "Standard Uniform 1",
                                 "Standard Normal 0",
                                 "Standard Normal 1" };
  
  // Standard uniform samples
  for ( int c = 0; c < nUniform; ++ c )
    {
    doubleArray[c] = vtkDoubleArray::New();
    doubleArray[c]->SetNumberOfComponents( 1 );
    doubleArray[c]->SetName( columnNames[c] );

    double x;
    for ( int r = 0; r < args->nVals; ++ r )
      {
      x = vtkMath::Random();
      doubleArray[c]->InsertNextValue( x );
      }
    
    inputData->AddColumn( doubleArray[c] );
    doubleArray[c]->Delete();
    }

  // Standard normal samples
  for ( int c = nUniform; c < nVariables; ++ c )
    {
    doubleArray[c] = vtkDoubleArray::New();
    doubleArray[c]->SetNumberOfComponents( 1 );
    doubleArray[c]->SetName( columnNames[c] );

    double x;
    for ( int r = 0; r < args->nVals; ++ r )
      {
      x = vtkMath::Gaussian();
      doubleArray[c]->InsertNextValue( x );
      }
    
    inputData->AddColumn( doubleArray[c] );
    doubleArray[c]->Delete();
    }

  // Reference values
  double sigmaRuleVal[] = { 68.27 , 95.45, 99.73 };     // "68-95-99.7 rule"
  double sigmaRuleTol[] = { 1., .5, .1 };

  // ************************** Descriptive Statistics ************************** 

#if DEBUG_WITH_SERIAL_STATS
  // Instantiate a (serial) descriptive statistics engine and set its ports
  vtkDescriptiveStatistics* ds = vtkDescriptiveStatistics::New();
  ds->SetInput( 0, inputData );

  // Select all columns
  for ( int c = 0; c < nVariables; ++ c )
    {
    ds->AddColumn( columnNames[c] );
    }

  // Test (serially) with Learn and Derive options only
  ds->SetLearn( true );
  ds->SetDer ive( true );
  ds->SetAssess( true );
  ds->Update();

  cout << "\n## Proc "
       << myRank
       << " calculated the following statistics ( "
       << ds->GetSampleSize()
       << " entries per column ):\n";
  for ( vtkIdType r = 0; r < ds->GetOutput( 1 )->GetNumberOfRows(); ++ r )
    {
    cout << "   ";
    for ( int i = 0; i < ds->GetOutput( 1 )->GetNumberOfColumns(); ++ i )
      {
      cout << ds->GetOutput( 1 )->GetColumnName( i )
           << "="
           << ds->GetOutput( 1 )->GetValue( r, i ).ToString()
           << "  ";
      }
    cout << "\n";
    }
  
  // Clean up
  ds->Delete();
#endif //DEBUG_WITH_SERIAL_STATS

  // Synchronize and start clock
  com->Barrier();
  time_t t0;
  time ( &t0 );
  
  // Instantiate a parallel descriptive statistics engine and set its ports
  vtkPDescriptiveStatistics* pds = vtkPDescriptiveStatistics::New();
  pds->SetInput( 0, inputData );
  vtkTable* outputData = pds->GetOutput( 0 );
  vtkTable* outputMeta = pds->GetOutput( 1 );

  // Select all columns
  for ( int c = 0; c < nVariables; ++ c )
    {
    pds->AddColumn( columnNames[c] );
    }

  // Test (in parallel) with Learn, Derive, and Assess options turned on
  pds->SetLearn( true );
  pds->SetDerive( true );
  pds->SetAssess( true );
  pds->SignedDeviationsOff(); // Use unsigned deviations
  pds->Update();

  // Synchronize and stop clock
  com->Barrier();
  time_t t1;
  time ( &t1 );

  if ( com->GetLocalProcessId() == args->ioRank )
    {
    cout << "\n## Completed parallel calculation of descriptive statistics (with assessment):\n"
         << "   Total sample size: "
         << pds->GetSampleSize()
         << " \n"
         << "   Wall time: "
         << difftime( t1, t0 )
         << " sec.\n";

   for ( vtkIdType r = 0; r < outputMeta->GetNumberOfRows(); ++ r )
      {
      cout << "   ";
      for ( int c = 0; c < outputMeta->GetNumberOfColumns(); ++ c )
        {
        vtkStdString colName = outputMeta->GetColumnName( c );

        // Do not report M aggregates
        if ( colName.at(0) == 'M' && colName.at(1) != 'e' )
          {
          continue;
          }

        cout << colName
             << "="
             << outputMeta->GetValue( r, c ).ToString()
             << "  ";
        }
      cout << "\n";
      }
    }
  
  // Verify that the DISTRIBUTED standard normal samples indeed statisfy the 68-95-99.7 rule
  if ( com->GetLocalProcessId() == args->ioRank )
    {
    cout << "\n## Verifying whether the distributed standard normal samples satisfy the 68-95-99.7 rule:\n";
    }
  
  vtkDoubleArray* relDev[2];
  relDev[0] = vtkDoubleArray::SafeDownCast(
    outputData->GetColumnByName( "d(Standard Normal 0)" ) );
  relDev[1] = vtkDoubleArray::SafeDownCast(
    outputData->GetColumnByName( "d(Standard Normal 1)" ) );

  if ( !relDev[0] || ! relDev[1] )
    {
    cout << "*** Error: "
         << "Empty output column(s) on process "
         << com->GetLocalProcessId()
         << ".\n";
    }

  for ( int c = 0; c < nNormal; ++ c )
    {
    int outsideStdv_l[] = { 0, 0, 0 };
    double dev;
    int n = outputData->GetNumberOfRows();
    for ( vtkIdType r = 0; r < n; ++ r )
      {
      dev = relDev[c]->GetValue( r );
      if ( dev >= 1. )
        {
        ++ outsideStdv_l[0];
        if ( dev >= 2. )
          {
          ++ outsideStdv_l[1];
          if ( dev >= 3. )
            {
            ++ outsideStdv_l[2];
            }
          }
        }
      }

    // Sum all local counters
    int outsideStdv_g[3];
    com->AllReduce( outsideStdv_l, 
                           outsideStdv_g, 
                           3, 
                           vtkCommunicator::SUM_OP );

    // Print out percentages of sample points within 1, 2, and 3 standard deviations of the mean.
    if ( com->GetLocalProcessId() == args->ioRank )
      {
      cout << "   "
           << outputData->GetColumnName( nUniform + c )
           << ":\n";
      for ( int i = 0; i < 3; ++ i )
        {
        double testVal = ( 1. - outsideStdv_g[i] / static_cast<double>( pds->GetSampleSize() ) ) * 100.;

        cout << "      " 
             << testVal
             << "\% within "
             << i + 1
             << " standard deviation(s) from the mean.\n";

        if ( fabs ( testVal - sigmaRuleVal[i] ) > sigmaRuleTol[i] )
          {
          vtkGenericWarningMacro("Incorrect value.");
          *(args->retVal) = 1;
          }
        }
      }
    }

  // Clean up
  pds->Delete();

  // ************************** Correlative Statistics ************************** 

  // Synchronize and start clock
  com->Barrier();
  time_t t2;
  time ( &t2 );

  // Instantiate a parallel correlative statistics engine and set its ports
  vtkPCorrelativeStatistics* pcs = vtkPCorrelativeStatistics::New();
  pcs->SetInput( 0, inputData );
  outputData = pcs->GetOutput( 0 );
  outputMeta = pcs->GetOutput( 1 );

  // Select column pairs (uniform vs. uniform, normal vs. normal)
  pcs->AddColumnPair( columnNames[0], columnNames[1] );
  pcs->AddColumnPair( columnNames[2], columnNames[3] );

  // Test (in parallel) with Learn, Derive, and Assess options turned on
  pcs->SetLearn( true );
  pcs->SetDerive( true );
  pcs->SetAssess( true );
  pcs->Update();

    // Synchronize and stop clock
  com->Barrier();
  time_t t3;
  time ( &t3 );

  if ( com->GetLocalProcessId() == args->ioRank )
    {
    cout << "\n## Completed parallel calculation of correlative statistics (with assessment):\n"
         << "   Total sample size: "
         << pcs->GetSampleSize()
         << " \n"
         << "   Wall time: "
         << difftime( t3, t2 )
         << " sec.\n";

    for ( vtkIdType r = 0; r < outputMeta->GetNumberOfRows(); ++ r )
      {
      cout << "   ";
      for ( int c = 0; c < outputMeta->GetNumberOfColumns(); ++ c )
        {
        vtkStdString colName = outputMeta->GetColumnName( c );

        // Do not report M aggregates
        if ( colName.at(0) == 'M' && colName.at(1) != 'e' )
          {
          continue;
          }

        cout << colName
             << "="
             << outputMeta->GetValue( r, c ).ToString()
             << "  ";
        }
      cout << "\n";
      }
    }

  // Clean up
  pcs->Delete();

  // ************************** Multi-Correlative Statistics ************************** 

  // Synchronize and start clock
  com->Barrier();
  time_t t4;
  time ( &t4 );

  // Instantiate a parallel correlative statistics engine and set its ports
  vtkPMultiCorrelativeStatistics* pmcs = vtkPMultiCorrelativeStatistics::New();
  pmcs->SetInput( 0, inputData );
  outputData = pmcs->GetOutput( 0 );

  // Select column pairs (uniform vs. uniform, normal vs. normal)
  pmcs->SetColumnStatus( columnNames[0], true );
  pmcs->SetColumnStatus( columnNames[1], true );
  pmcs->RequestSelectedColumns();

  pmcs->ResetAllColumnStates();
  pmcs->SetColumnStatus( columnNames[2], true );
  pmcs->SetColumnStatus( columnNames[3], true );
  pmcs->RequestSelectedColumns();

  pmcs->ResetAllColumnStates();
  pmcs->SetColumnStatus( columnNames[0], true );
  pmcs->SetColumnStatus( columnNames[1], true );
  pmcs->SetColumnStatus( columnNames[2], true );
  pmcs->SetColumnStatus( columnNames[3], true );
  pmcs->RequestSelectedColumns();

  // Test (in parallel) with Learn, Derive, and Assess options turned on
  pmcs->SetLearn( true );
  pmcs->SetDerive( true );
  pmcs->SetAssess( true );
  pmcs->Update();

    // Synchronize and stop clock
  com->Barrier();
  time_t t5;
  time ( &t5 );

  if ( com->GetLocalProcessId() == args->ioRank )
    {
    cout << "\n## Completed parallel calculation of multi-correlative statistics (with assessment):\n"
         << "   Total sample size: "
         << pmcs->GetSampleSize()
         << " \n"
         << "   Wall time: "
         << difftime( t5, t4 )
         << " sec.\n";

    vtkMultiBlockDataSet* outputMetaDS = vtkMultiBlockDataSet::SafeDownCast( pmcs->GetOutputDataObject( 1 ) ); 
    for ( unsigned int b = 1; b < outputMetaDS->GetNumberOfBlocks(); ++ b )
      {
      outputMeta = vtkTable::SafeDownCast( outputMetaDS->GetBlock( b ) );
      outputMeta->Dump();
      }
    }

  // Clean up
  pmcs->Delete();

  // ************************** PCA Statistics ************************** 

  // Synchronize and start clock
  com->Barrier();
  time_t t6;
  time ( &t6 );

  // Instantiate a parallel pca statistics engine and set its ports
  vtkPPCAStatistics* pcas = vtkPPCAStatistics::New();
  pcas->SetInput( 0, inputData );
  outputData = pcas->GetOutput( 0 );

  // Select column pairs (uniform vs. uniform, normal vs. normal)
  pcas->SetColumnStatus( columnNames[0], true );
  pcas->SetColumnStatus( columnNames[1], true );
  pcas->RequestSelectedColumns();

  pcas->ResetAllColumnStates();
  pcas->SetColumnStatus( columnNames[2], true );
  pcas->SetColumnStatus( columnNames[3], true );
  pcas->RequestSelectedColumns();

  pcas->ResetAllColumnStates();
  pcas->SetColumnStatus( columnNames[0], true );
  pcas->SetColumnStatus( columnNames[1], true );
  pcas->SetColumnStatus( columnNames[2], true );
  pcas->SetColumnStatus( columnNames[3], true );
  pcas->RequestSelectedColumns();

  // Test (in parallel) with Learn, Derive, and Assess options turned on
  pcas->SetLearn( true );
  pcas->SetDerive( true );
  pcas->SetAssess( true );
  pcas->Update();

    // Synchronize and stop clock
  com->Barrier();
  time_t t7;
  time ( &t7 );

  if ( com->GetLocalProcessId() == args->ioRank )
    {
    cout << "\n## Completed parallel calculation of pca statistics (with assessment):\n"
         << "   Total sample size: "
         << pcas->GetSampleSize()
         << " \n"
         << "   Wall time: "
         << difftime( t7, t6 )
         << " sec.\n";

    vtkMultiBlockDataSet* outputMetaDS = vtkMultiBlockDataSet::SafeDownCast( pcas->GetOutputDataObject( 1 ) ); 
    for ( unsigned int b = 1; b < outputMetaDS->GetNumberOfBlocks(); ++ b )
      {
      outputMeta = vtkTable::SafeDownCast( outputMetaDS->GetBlock( b ) );
      outputMeta->Dump();
      }
    }

  // Clean up
  pcas->Delete();

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
    // Houston, we'we had a problem: no I/O node found.
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
    cout << "\n# Houston, this is process "
         << ioRank
         << " speaking. I'll be the I/O node.\n";
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
  RandomSampleStatisticsArgs args;
  args.nVals = 100000;
  args.retVal = &testValue;
  args.ioRank = ioRank;
  args.argc = argc;
  args.argv = argv;

  // Execute the function named "process" on both processes
  controller->SetSingleMethod( RandomSampleStatistics, &args );
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
