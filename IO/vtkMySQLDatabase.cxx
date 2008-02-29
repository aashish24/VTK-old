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
#include "vtkMySQLDatabase.h"
#include "vtkMySQLDatabasePrivate.h"
#include "vtkMySQLQuery.h"

#include "vtkSQLDatabaseSchema.h"

#include "vtkObjectFactory.h"
#include "vtkStringArray.h"

#include <vtksys/SystemTools.hxx>
#include <vtksys/ios/sstream>

#include <assert.h>

#define VTK_MYSQL_DEFAULT_PORT 3306
 
vtkCxxRevisionMacro(vtkMySQLDatabase, "$Revision$");
vtkStandardNewMacro(vtkMySQLDatabase);

// ----------------------------------------------------------------------
vtkMySQLDatabase::vtkMySQLDatabase() :
  Private(new vtkMySQLDatabasePrivate())
{
  this->Tables = vtkStringArray::New();
  this->Tables->Register(this);
  this->Tables->Delete();
  
  // Initialize instance variables
  this->DatabaseType = 0;
  this->SetDatabaseType( "mysql" );
  this->HostName = 0;
  this->User = 0;
  this->Password = 0;
  this->DatabaseName = 0;
  this->ConnectOptions = 0;
  // Default: connect to local machine on standard port
  this->SetHostName( "localhost" );
  this->ServerPort = VTK_MYSQL_DEFAULT_PORT;
  //this->SetPassword( "" );
}

// ----------------------------------------------------------------------
vtkMySQLDatabase::~vtkMySQLDatabase()
{
  if ( this->IsOpen() )
    {
    this->Close();
    }
  this->SetDatabaseType( 0 );
  this->SetHostName( 0 );
  this->SetUser( 0 );
  this->SetPassword( 0 );
  this->SetDatabaseName( 0 );
  this->SetConnectOptions( 0 );

  this->Tables->UnRegister(this);

  delete this->Private;
}

// ----------------------------------------------------------------------
void vtkMySQLDatabase::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "DatabaseType: " << (this->DatabaseType ? this->DatabaseType : "NULL") << endl;
  os << indent << "HostName: " << (this->HostName ? this->HostName : "NULL") << endl;
  os << indent << "User: " << (this->User ? this->User : "NULL") << endl;
  os << indent << "Password: " << (this->Password ? this->Password : "NULL") << endl;
  os << indent << "DatabaseName: " << (this->DatabaseName ? this->DatabaseName : "NULL") << endl;
  os << indent << "ServerPort: " << this->ServerPort << endl;
  os << indent << "ConnectOptions: " << (this->ConnectOptions ? this->ConnectOptions : "NULL") << endl;
}

// ----------------------------------------------------------------------
bool vtkMySQLDatabase::IsSupported(int feature)
{
  switch (feature)
    {
    case VTK_SQL_FEATURE_BATCH_OPERATIONS:
    case VTK_SQL_FEATURE_NAMED_PLACEHOLDERS:
      return false;

    case VTK_SQL_FEATURE_POSITIONAL_PLACEHOLDERS:
#if MYSQL_VERSION_ID >= 40108
      return true;
#else
      return false;
#endif

    case VTK_SQL_FEATURE_PREPARED_QUERIES:
    {
    if (mysql_get_client_version() >= 40108 &&
        mysql_get_server_version(& this->Private->NullConnection) >= 40100)
      {
      return true;
      }
    else
      {
      return false;
      }
    };

    case VTK_SQL_FEATURE_QUERY_SIZE:
    case VTK_SQL_FEATURE_BLOB:
    case VTK_SQL_FEATURE_LAST_INSERT_ID:
    case VTK_SQL_FEATURE_UNICODE:
    case VTK_SQL_FEATURE_TRANSACTIONS:
      return true;
      
    default:
    {
    vtkErrorMacro(<< "Unknown SQL feature code " << feature << "!  See "
                  << "vtkSQLDatabase.h for a list of possible features.");
    return false;
    };
    }
}

// ----------------------------------------------------------------------
bool vtkMySQLDatabase::Open()
{

  if ( this->IsOpen() )
    {
    vtkGenericWarningMacro( "Open(): Database is already open." );
    return true;
    }

  assert(this->Private->Connection == NULL);

  this->Private->Connection = 
    mysql_real_connect( &this->Private->NullConnection, 
                        this->GetHostName(),
                        this->GetUser(),
                        this->GetPassword(), 
                        this->GetDatabaseName(),
                        this->GetServerPort(),
                        0, 0);
                                        
  if (this->Private->Connection == NULL)
    {
    vtkErrorMacro(<<"Open() failed with error: " 
                  << mysql_error(& this->Private->NullConnection));
    return false;
    }
  else
    {
    vtkDebugMacro(<<"Open() succeeded.");
    return true;
    }
}

// ----------------------------------------------------------------------
void vtkMySQLDatabase::Close()
{
  if (! this->IsOpen())
    {
    return; // not an error
    }
  else
    {
    mysql_close(this->Private->Connection);
    this->Private->Connection = NULL;
    }
}

// ----------------------------------------------------------------------
bool vtkMySQLDatabase::IsOpen()
{
  return (this->Private->Connection != NULL);
}

// ----------------------------------------------------------------------
vtkSQLQuery* vtkMySQLDatabase::GetQueryInstance()
{
  vtkMySQLQuery* query = vtkMySQLQuery::New();
  query->SetDatabase(this);
  return query;
}

// ----------------------------------------------------------------------
vtkStringArray* vtkMySQLDatabase::GetTables()
{
  this->Tables->Resize(0);
  if ( ! this->IsOpen() )
    {
    vtkErrorMacro(<<"GetTables(): Database is closed!");
    return this->Tables;
    }
  else
    {
    MYSQL_RES* tableResult = mysql_list_tables(
      this->Private->Connection, NULL );

    if ( ! tableResult )
      {
      vtkErrorMacro(<<"GetTables(): MySQL returned error: "
                    << mysql_error(this->Private->Connection));
      return this->Tables;
      }

    MYSQL_ROW row;
    int i=0;

    while ( tableResult )
      {
      mysql_data_seek( tableResult, i );
      row = mysql_fetch_row( tableResult );
      if ( ! row )
        {
        break;
        }

      this->Tables->InsertNextValue( row[0] );
      ++ i;
      }
      // Done with processing so free it
      mysql_free_result( tableResult );

    return this->Tables;
    }
}

// ----------------------------------------------------------------------
vtkStringArray* vtkMySQLDatabase::GetRecord(const char *table)
{
  vtkStringArray *results = vtkStringArray::New();

  if (!this->IsOpen())
    {
    vtkErrorMacro(<<"GetRecord: Database is not open!");
    return results;
    }

  MYSQL_RES *record = 
    mysql_list_fields(this->Private->Connection, table, 0);

  if (!record)
    {
    vtkErrorMacro(<<"GetRecord: MySQL returned error: "
                  << mysql_error(this->Private->Connection));
    return results;
    }

  MYSQL_FIELD *field;
  while ((field = mysql_fetch_field(record)))
    {
    results->InsertNextValue(field->name);
    }
  
  mysql_free_result(record);
  return results;
}


bool vtkMySQLDatabase::HasError()
{ 
  return (mysql_errno(this->Private->Connection)!=0);
}

const char* vtkMySQLDatabase::GetLastErrorText()
{
  return this->Private->Connection ?
    mysql_error( this->Private->Connection ) : 0;
}

// ----------------------------------------------------------------------
vtkStdString vtkMySQLDatabase::GetURL()
{
  vtkStdString url;
  url = this->GetDatabaseType();
  url += "://";
  if ( this->GetUser() && strlen( this->GetUser() ) )
    {
    url += this->GetUser();
    if ( this->GetPassword() && strlen( this->GetPassword() ) )
      {
      url += ":";
      url += this->GetPassword();
      }
    url += "@";
    }
  if ( this->GetHostName() && strlen( this->GetHostName() ) )
    {
    url += this->GetHostName();
    }
  else
    {
    url += "localhost";
    }
  if (
    this->GetServerPort() >= 0 &&
    this->GetServerPort() != VTK_MYSQL_DEFAULT_PORT
    )
    {
    vtksys_ios::ostringstream stream;
    stream << ":" << this->GetServerPort();
    url += stream.str();
    }
  url += "/";
  if ( this->GetDatabaseName() && strlen( this->GetDatabaseName() ) )
    url += this->GetDatabaseName();
  return url;
}

// ----------------------------------------------------------------------
vtkStdString vtkMySQLDatabase::GetColumnSpecification( vtkSQLDatabaseSchema* schema,
                                                       int tblHandle,
                                                       int colHandle )
{
  // With MySQL, the column name must be enclosed between backquotes
  vtksys_ios::ostringstream queryStr;
  queryStr << "`" << schema->GetColumnNameFromHandle( tblHandle, colHandle ) << "` ";

  // Figure out column type
  int colType = schema->GetColumnTypeFromHandle( tblHandle, colHandle ); 
  vtkStdString colTypeStr;

  switch ( static_cast<vtkSQLDatabaseSchema::DatabaseColumnType>( colType ) )
    {
    case vtkSQLDatabaseSchema::SERIAL:    
      colTypeStr = "INT NOT NULL AUTO_INCREMENT";
      break;
    case vtkSQLDatabaseSchema::SMALLINT:  
      colTypeStr = "SMALLINT";
      break;
    case vtkSQLDatabaseSchema::INTEGER:   
      colTypeStr = "INT";
      break;
    case vtkSQLDatabaseSchema::BIGINT:    
      colTypeStr = "BIGINT";
      break;
    case vtkSQLDatabaseSchema::VARCHAR:   
      colTypeStr = "VARCHAR";
      break;
    case vtkSQLDatabaseSchema::TEXT:      
      colTypeStr = "TEXT";
      break;
    case vtkSQLDatabaseSchema::REAL:      
      colTypeStr = "FLOAT";
      break;
    case vtkSQLDatabaseSchema::DOUBLE:    
      colTypeStr = "DOUBLE PRECISION";
      break;
    case vtkSQLDatabaseSchema::BLOB:      
      colTypeStr = "BLOB";
      break;
    case vtkSQLDatabaseSchema::TIME:      
      colTypeStr = "TIME";
      break;
    case vtkSQLDatabaseSchema::DATE:      
      colTypeStr = "DATE";
      break;
    case vtkSQLDatabaseSchema::TIMESTAMP: 
      colTypeStr = "TIMESTAMP";
      break;
    }

  if ( colTypeStr.size() )
    {
    queryStr << " " << colTypeStr;
    }
  else // if ( colTypeStr.size() )
    {
    vtkGenericWarningMacro( "Unable to get column specification: unsupported data type " << colType );
    return vtkStdString();
    }
  
  // Decide whether size is allowed, required, or unused
  int colSizeType = 0;

  switch ( static_cast<vtkSQLDatabaseSchema::DatabaseColumnType>( colType ) )
    {
    case vtkSQLDatabaseSchema::SERIAL:    
      colSizeType =  0;
      break;
    case vtkSQLDatabaseSchema::SMALLINT:  
      colSizeType =  1;
      break;
    case vtkSQLDatabaseSchema::INTEGER:   
      colSizeType =  1;
      break;
    case vtkSQLDatabaseSchema::BIGINT:    
      colSizeType =  1;
      break;
    case vtkSQLDatabaseSchema::VARCHAR:   
      colSizeType = -1;
      break;
    case vtkSQLDatabaseSchema::TEXT:      
      colSizeType =  1;
      break;
    case vtkSQLDatabaseSchema::REAL:      
      colSizeType =  0; // Eventually will make DB schemata handle (M,D) sizes
      break;
    case vtkSQLDatabaseSchema::DOUBLE:    
      colSizeType =  0; // Eventually will make DB schemata handle (M,D) sizes
      break;
    case vtkSQLDatabaseSchema::BLOB:      
      colSizeType =  1;
      break;
    case vtkSQLDatabaseSchema::TIME:      
      colSizeType =  0;
      break;
    case vtkSQLDatabaseSchema::DATE:      
      colSizeType =  0;
      break;
    case vtkSQLDatabaseSchema::TIMESTAMP: 
      colSizeType =  0;
      break;
    }

  // Specify size if allowed or required
  if ( colSizeType )
    {
    int colSize = schema->GetColumnSizeFromHandle( tblHandle, colHandle );
    // IF size is provided but absurd, 
    // OR, if size is required but not provided OR absurd,
    // THEN assign the default size.
    if ( ( colSize < 0 ) || ( colSizeType == -1 && colSize < 1 ) )
      {
      colSize = VTK_SQL_DEFAULT_COLUMN_SIZE;
      }
    
    // At this point, we have either a valid size if required, or a possibly null valid size
    // if not required. Thus, skip sizing in the latter case.
    if ( colSize > 0 )
      {
      queryStr << "(" << colSize << ")";
      }
    }

  vtkStdString attStr = schema->GetColumnAttributesFromHandle( tblHandle, colHandle );
  if ( attStr.size() )
    {
    queryStr << " " << attStr;
    }

  return queryStr.str();
}

// ----------------------------------------------------------------------
vtkStdString vtkMySQLDatabase::GetIndexSpecification( vtkSQLDatabaseSchema* schema,
                                                      int tblHandle,
                                                      int idxHandle,
                                                      bool& skipped )
{
  skipped = false;
  vtkStdString queryStr = ", ";
  bool mustUseName = true;

  int idxType = schema->GetIndexTypeFromHandle( tblHandle, idxHandle );
  switch ( idxType )
    {
    case vtkSQLDatabaseSchema::PRIMARY_KEY:
      queryStr += "PRIMARY KEY ";
      mustUseName = false;
      break;
    case vtkSQLDatabaseSchema::UNIQUE:
      queryStr += "UNIQUE ";
      break;
    case vtkSQLDatabaseSchema::INDEX:
      queryStr += "INDEX ";
      break;
    default:
      return vtkStdString();
    }
  
  // No index_name for PRIMARY KEYs
  if ( mustUseName )
    {
    queryStr += schema->GetIndexNameFromHandle( tblHandle, idxHandle );
    }
  queryStr += " (";
        
  // Loop over all column names of the index
  int numCnm = schema->GetNumberOfColumnNamesInIndex( tblHandle, idxHandle );
  if ( numCnm < 0 )
    {
    vtkGenericWarningMacro( "Unable to get index specification: index has incorrect number of columns " << numCnm );
    return vtkStdString();
    }

  bool firstCnm = true;
  for ( int cnmHandle = 0; cnmHandle < numCnm; ++ cnmHandle )
    {
    if ( firstCnm )
      {
      firstCnm = false;
      }
    else
      {
      queryStr += ",";
      }
    // With MySQL, the column name must be enclosed between backquotes
    queryStr += "`";
    queryStr += schema->GetIndexColumnNameFromHandle( tblHandle, idxHandle, cnmHandle );
    queryStr += "` ";
    }
  queryStr += ")";

  return queryStr;
}


bool vtkMySQLDatabase::CreateDatabase( const char* dbName, bool dropExisting = false )
{
  if ( dropExisting )
    {
    this->DropDatabase( dbName );
    }
  vtkStdString queryStr;
  queryStr = "CREATE DATABASE ";
  queryStr += dbName;
  vtkSQLQuery* query = this->GetQueryInstance();
  query->SetQuery( queryStr.c_str() );
  bool status = query->Execute();
  query->Delete();
  // Close and re-open in case we deleted and recreated the current database
  this->Close();
  this->Open();
  return status;
}

bool vtkMySQLDatabase::DropDatabase( const char* dbName )
{
  vtkStdString queryStr;
  queryStr = "DROP DATABASE IF EXISTS ";
  queryStr += dbName;
  vtkSQLQuery* query = this->GetQueryInstance();
  query->SetQuery( queryStr.c_str() );
  bool status = query->Execute();
  query->Delete();
  return status;
}

