/*
 * Copyright 2007 Sandia Corporation.
 * Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
 * license for use of this work by or on behalf of the
 * U.S. Government. Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that this Notice and any
 * statement of authorship are reproduced on all copies.
 */

#include "vtkIdTypeArray.h"
#include "vtkDoubleArray.h"
#include "vtkTable.h"
#include "vtkCorrelativeStatistics.h"

//=============================================================================
int TestCorrelativeStatistics( int, char *[] )
{
  int testIntValue = 0;

  double mingledData[] = 
    {
    46,
    45,
    47,
    49,
    46,
    47,
    46,
    46,
    47,
    46,
    47,
    49,
    49,
    49,
    47,
    45,
    50,
    50,
    46,
    46,
    51,
    50,
    48,
    48,
    52,
    54,
    48,
    47,
    52,
    52,
    49,
    49,
    53,
    54,
    50,
    50,
    53,
    54,
    50,
    52,
    53,
    53,
    50,
    51,
    54,
    54,
    49,
    49,
    52,
    52,
    50,
    51,
    52,
    52,
    49,
    47,
    48,
    48,
    48,
    50,
    46,
    48,
    47,
    47,
    };
  int nVals = 32;

  vtkDoubleArray* dataset1Arr = vtkDoubleArray::New();
  dataset1Arr->SetNumberOfComponents( 1 );
  dataset1Arr->SetName( "Metric 0" );

  vtkDoubleArray* dataset2Arr = vtkDoubleArray::New();
  dataset2Arr->SetNumberOfComponents( 1 );
  dataset2Arr->SetName( "Metric 1" );

  vtkDoubleArray* dataset3Arr = vtkDoubleArray::New();
  dataset3Arr->SetNumberOfComponents( 1 );
  dataset3Arr->SetName( "Metric 2" );

  for ( int i = 0; i < nVals; ++ i )
    {
    int ti = i << 1;
    dataset1Arr->InsertNextValue( mingledData[ti] );
    dataset2Arr->InsertNextValue( mingledData[ti + 1] );
    dataset3Arr->InsertNextValue( -1. );
    }

  vtkTable* datasetTable = vtkTable::New();
  datasetTable->AddColumn( dataset1Arr );
  dataset1Arr->Delete();
  datasetTable->AddColumn( dataset2Arr );
  dataset2Arr->Delete();
  datasetTable->AddColumn( dataset3Arr );
  dataset3Arr->Delete();

  vtkTable* paramsTable = vtkTable::New();
  int nMetricPairs = 3;
  vtkIdType columnPairs[] = { 0, 1, 1, 0, 2, 1 };
  double centers[] = { 49.2188, 49.5 };
  double covariance[] = { 5.98286, 7.54839, 6.14516 };
  double threshold = .2 ;

  vtkIdTypeArray* idTypeCol = vtkIdTypeArray::New();
  idTypeCol->SetName( "Column X" );
  idTypeCol->InsertNextValue( 0 );
  paramsTable->AddColumn( idTypeCol );
  idTypeCol->Delete();

  idTypeCol = vtkIdTypeArray::New();
  idTypeCol->SetName( "Column Y" );
  idTypeCol->InsertNextValue( 1 );
  paramsTable->AddColumn( idTypeCol );
  idTypeCol->Delete();

  vtkDoubleArray* doubleCol = vtkDoubleArray::New();
  doubleCol->SetName( "Nominal X" );
  doubleCol->InsertNextValue( centers[0] );
  paramsTable->AddColumn( doubleCol );
  doubleCol->Delete();

  doubleCol = vtkDoubleArray::New();
  doubleCol->SetName( "Nominal Y" );
  doubleCol->InsertNextValue( centers[1] );
  paramsTable->AddColumn( doubleCol );
  doubleCol->Delete();

  doubleCol = vtkDoubleArray::New();
  doubleCol->SetName( "Variance X" );
  doubleCol->InsertNextValue( covariance[0] );
  paramsTable->AddColumn( doubleCol );
  doubleCol->Delete();

  doubleCol = vtkDoubleArray::New();
  doubleCol->SetName( "Variance Y" );
  doubleCol->InsertNextValue( covariance[1] );
  paramsTable->AddColumn( doubleCol );
  doubleCol->Delete();

  doubleCol = vtkDoubleArray::New();
  doubleCol->SetName( "Covariance" );
  doubleCol->InsertNextValue( covariance[2] );
  paramsTable->AddColumn( doubleCol );
  doubleCol->Delete();

  doubleCol = vtkDoubleArray::New();
  doubleCol->SetName( "Threshold" );
  doubleCol->InsertNextValue( threshold );
  paramsTable->AddColumn( doubleCol );
  doubleCol->Delete();

  vtkCorrelativeStatistics* haruspex = vtkCorrelativeStatistics::New();
  haruspex->SetInput( 0, datasetTable );
  haruspex->SetInput( 1, paramsTable );
  vtkTable* outputTable = haruspex->GetOutput();

  datasetTable->Delete();
  paramsTable->Delete();

// -- Select Column Pairs of Interest ( Learn Mode ) -- 
  haruspex->AddColumnPair( 0, 1 ); // A valid pair
  haruspex->AddColumnPair( 1, 0 ); // The same valid pair, just reversed
  haruspex->AddColumnPair( 2, 1 ); // Another valid pair
  for ( int i = 0; i< nMetricPairs; i += 2 )
    {  // Try to add all valid pairs once more
    haruspex->AddColumnPair( columnPairs[i], columnPairs[i+1] );
    }
  haruspex->AddColumnPair( 1, 3 ); // An invalid pair

// -- Test Learn Mode -- 
  haruspex->SetExecutionMode( vtkStatisticsAlgorithm::LearnMode );
  haruspex->Update();
  vtkIdType n = haruspex->GetSampleSize();

  cout << "## Calculated the following statistics ( "
       << n
       << " entries per column ):\n";
  for ( vtkIdType r = 0; r < outputTable->GetNumberOfRows(); ++ r )
    {
    cout << "   (X, Y) = ("
         << datasetTable->GetColumnName( outputTable->GetValue( r, 0 ).ToInt() )
         << ", "
         << datasetTable->GetColumnName( outputTable->GetValue( r, 1 ).ToInt() )
         << ")";

    for ( int i = 2; i < 7; ++ i )
      {
      cout << ", "
           << outputTable->GetColumnName( i )
           << "="
           << outputTable->GetValue( r, i ).ToDouble();
      }

    if ( ! outputTable->GetValue( r,  7 ).ToInt () )
      {
      cout << "\n   Y = "
           << outputTable->GetValue( r,  8 ).ToDouble()
           << " * X + "
           << outputTable->GetValue( r,  9 ).ToDouble()
           << ", X = "
           << outputTable->GetValue( r, 10 ).ToDouble()
           << " * Y + "
           << outputTable->GetValue( r, 11 ).ToDouble()
           << ", corr. coeff.: "
           << outputTable->GetValue( r, 12 ).ToDouble()
           << "\n";
      }
    else
      {
      cout << "\n   Degenerate input, linear correlation was not calculated.\n";
      }
    }

// -- Select Column Pairs of Interest ( Evince Mode ) -- 
  haruspex->ResetColumnPairs(); // Clear existing pairs
  haruspex->AddColumnPair( columnPairs[0], columnPairs[1] ); // A valid pair

// -- Test Evince Mode -- 
  cout << "## Searching for the following outliers:\n";
  for ( vtkIdType i = 0; i < paramsTable->GetNumberOfRows(); ++ i )
    {
    cout << "   (X, Y) = ("
         << datasetTable->GetColumnName( columnPairs[0] )
         << ", "
         << datasetTable->GetColumnName( columnPairs[1] )
         << "), Gaussian, mean=("
         << centers[0]
         << ", "
         << centers[1]
         << "), cov=["
         << covariance[0]
         << ", "
         << covariance[2]
         << " ; "
         << covariance[2]
         << ", "
         << covariance[1]
         << "], relPDF < "
         << threshold
         << "\n";
    }

  haruspex->SetExecutionMode( vtkStatisticsAlgorithm::EvinceMode );
  haruspex->Update();

  testIntValue = outputTable->GetNumberOfRows();
  if ( testIntValue != 7 )
    {
    cerr << "Reported an incorrect number of outliers: "
         << testIntValue
         << " != 7.\n";
    return 1;
    }

  cout << "Found "
       << testIntValue
       << " outliers:\n";

  for ( vtkIdType r = 0; r < outputTable->GetNumberOfRows(); ++ r )
    {
    vtkIdType idX = outputTable->GetValue( r, 0 ).ToInt();
    vtkIdType idY = outputTable->GetValue( r, 1 ).ToInt();
    vtkIdType i = outputTable->GetValue( r, 2 ).ToInt();
    cout << "   "
         << i
         << ": ( "
         << datasetTable->GetValue( i, idX ).ToDouble()
         << " , "
         << datasetTable->GetValue( i, idY ).ToDouble()
         << " ) has a relative PDF of "
         << outputTable->GetValue( r, 3 ).ToDouble()
         << "\n";
    }

  haruspex->Delete();

  return 0;
}
