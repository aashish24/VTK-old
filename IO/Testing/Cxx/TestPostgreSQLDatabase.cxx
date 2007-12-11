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
/*----------------------------------------------------------------------------
 Copyright (c) Sandia Corporation
 See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.
----------------------------------------------------------------------------*/
// .SECTION Thanks
// Thanks to Andrew Wilson from Sandia National Laboratories for implementing
// this test.

#include "vtkPostgreSQLDatabase.h"
#include "vtkSQLQuery.h"
#include "vtkRowQueryToTable.h"
#include "vtkStdString.h"
#include "vtkTable.h"
#include "vtkVariant.h"
#include "vtkVariantArray.h"
#include "vtkStringArray.h"
#include "vtkToolkits.h"

int TestPostgreSQLDatabase( int /*argc*/, char* /*argv*/[] )
{
  // This test requires a database named "test" to be present
  // on a Postgres server running on the local machine. The
  // default user must have permission to add and drop databases
  // as well as tables in the databases. A new database "vtktest"
  // will be created and then destroyed by this test.
  vtkPostgreSQLDatabase* db = vtkPostgreSQLDatabase::SafeDownCast( vtkSQLDatabase::CreateFromURL( VTK_PSQL_TEST_URL ) );
  bool status = db->Open();

  if ( ! status )
    {
    cerr << "Couldn't open database.\n";
    return 1;
    }

  vtkStringArray* dbNames = db->GetDatabases();
  cout << "Database list:\n";
  for ( int dbi = 0; dbi < dbNames->GetNumberOfValues(); ++ dbi )
    {
    cout << "+ " << dbNames->GetValue( dbi ) << endl;
    }
  dbNames->Delete();
  if ( ! db->CreateDatabase( "vtktest", true ) )
    {
    cerr << "Error: " << db->GetLastErrorText() << endl;
    }

  vtkSQLQuery* query = db->GetQueryInstance();

  vtkStdString dropQuery( "DROP TABLE people" );
  cout << dropQuery << endl;
  query->SetQuery( dropQuery.c_str() );
  if ( ! query->Execute() )
    {
    cerr << "Drop query failed" << endl;
    return 1;
    }

  vtkStdString createQuery( "CREATE TABLE people (name TEXT, age INTEGER, weight FLOAT)" );
  cout << createQuery << endl;
  query->SetQuery( createQuery.c_str() );
  if ( ! query->Execute() )
    {
    cerr << "Create query failed" << endl;
    return 1;
    }

  for ( int i = 0; i < 40; ++ i )
    {
    char insertQuery[200];
    sprintf( insertQuery, "INSERT INTO people VALUES('John Doe %d', %d, %d)", i, i, 10 * i );
    cout << insertQuery << endl;
    query->SetQuery( insertQuery );
    if ( ! query->Execute() )
      {
      cerr << "Insert query " << i << " failed" << endl;
      return 1;
      }
    }


  const char* queryText = "SELECT name, age, weight FROM people WHERE age <= 20";
  query->SetQuery( queryText );
  cerr << endl << "Running query: " << query->GetQuery() << endl;

  cerr << endl << "Using vtkSQLQuery directly to execute query:" << endl;
  if ( ! query->Execute() )
    {
    cerr << "Query failed" << endl;
    return 1;
    }

  for ( int col = 0; col < query->GetNumberOfFields(); ++ col )
    {
    if ( col > 0 )
      {
      cerr << ", ";
      }
    cerr << query->GetFieldName( col );
    }
  cerr << endl;
  while ( query->NextRow() )
    {
    for ( int field = 0; field < query->GetNumberOfFields(); ++ field )
      {
      if ( field > 0 )
        {
        cerr << ", ";
        }
      cerr << query->DataValue( field ).ToString().c_str();
      }
    cerr << endl;
    }

  cerr << endl << "Using vtkSQLQuery to execute query and retrieve by row:" << endl;
  if ( ! query->Execute() )
    {
    cerr << "Query failed" << endl;
    return 1;
    }
  for ( int col = 0; col < query->GetNumberOfFields(); ++ col )
    {
    if ( col > 0 )
      {
      cerr << ", ";
      }
    cerr << query->GetFieldName( col );
    }
  cerr << endl;
  vtkVariantArray* va = vtkVariantArray::New();
  while ( query->NextRow( va ) )
    {
    for ( int field = 0; field < va->GetNumberOfValues(); ++ field )
      {
      if ( field > 0 )
        {
        cerr << ", ";
        }
      cerr << va->GetValue( field ).ToString().c_str();
      }
    cerr << endl;
    }
  va->Delete();

  cerr << endl << "Using vtkRowQueryToTable to execute query:" << endl;
  vtkRowQueryToTable* reader = vtkRowQueryToTable::New();
  reader->SetQuery( query );
  reader->Update();
  vtkTable* table = reader->GetOutput();
  for ( vtkIdType col = 0; col < table->GetNumberOfColumns(); ++ col )
    {
    table->GetColumn( col )->Print( cerr );
    }
  cerr << endl;
  for ( vtkIdType row = 0; row < table->GetNumberOfRows(); ++ row )
    {
    for ( vtkIdType col = 0; col < table->GetNumberOfColumns(); ++ col )
      {
      vtkVariant v = table->GetValue( row, col );
      cerr << "row " << row << ", col " << col << " - "
        << v.ToString() << " (" << vtkImageScalarTypeNameMacro( v.GetType() ) << ")" << endl;
      }
    }

  db->Close();
  db->Open();
  // Delete the database until we run the test again
  if ( ! db->DropDatabase( "vtktest" ) )
    {
    cerr << db->GetLastErrorText() << endl;
    }

  reader->Delete();
  query->Delete();
  db->Delete();

  return 0;
}
