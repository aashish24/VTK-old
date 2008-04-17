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
#include "vtkSQLQuery.h"

#include "vtkObjectFactory.h"
#include "vtkSQLDatabase.h"
#include "vtkVariantArray.h"

#include "vtksys/SystemTools.hxx"

vtkCxxRevisionMacro(vtkSQLQuery, "$Revision$");

vtkSQLQuery::vtkSQLQuery()
{
  this->Query = 0;
  this->Database = 0;
  this->Active = false;
}

vtkSQLQuery::~vtkSQLQuery()
{
  this->SetQuery(0);
  if (this->Database)
    {
    this->Database->Delete();
    this->Database = NULL;
    }
}

vtkCxxSetObjectMacro(vtkSQLQuery, Database, vtkSQLDatabase);

void vtkSQLQuery::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Query: " << (this->Query ? this->Query : "NULL") << endl;
  os << indent << "Database: " << (this->Database ? "" : "NULL") << endl;
  if (this->Database)
    {
    this->Database->PrintSelf(os, indent.GetNextIndent());
    }
}

vtkStdString vtkSQLQuery::EscapeString( vtkStdString s, bool addSurroundingQuotes )
{
  vtkStdString d;
  if ( addSurroundingQuotes )
    {
    d.push_back( '\'' );
    }

  for ( vtkStdString::iterator it = s.begin(); it != s.end(); ++ it )
    {
    if ( *it == '\'' )
      d.push_back( '\'' ); // Single quotes are escaped by repeating them
    d.push_back( *it );
    }

  if ( addSurroundingQuotes )
    {
    d.push_back( '\'' );
    }
  return d;
}

char* vtkSQLQuery::EscapeString( const char* src, bool addSurroundingQuotes )
{
  vtkStdString sstr( src );
  vtkStdString dstr = this->EscapeString( sstr, addSurroundingQuotes );
  return vtksys::SystemTools::DuplicateString( dstr.c_str() );
}

