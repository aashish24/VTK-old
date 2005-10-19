/*=========================================================================

  Program:   KWSys - Kitware System Library
  Module:    $RCSfile$

  Copyright (c) Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "kwsysPrivate.h"
#include KWSYS_HEADER(Glob.hxx)

#include KWSYS_HEADER(Configure.hxx)

#include KWSYS_HEADER(RegularExpression.hxx)
#include KWSYS_HEADER(SystemTools.hxx)
#include KWSYS_HEADER(Directory.hxx)
#include KWSYS_HEADER(stl/string)
#include KWSYS_HEADER(stl/vector)

// Work-around CMake dependency scanning limitation.  This must
// duplicate the above list of headers.
#if 0
# include "Glob.hxx.in"
# include "Configure.hxx.in"
# include "RegularExpression.hxx.in"
# include "SystemTools.hxx.in"
# include "kwsys_stl.hxx.in"
# include "kwsys_stl_string.hxx.in"
#endif

#include <ctype.h>
#include <stdio.h>

namespace KWSYS_NAMESPACE
{
#if defined( _WIN32 ) || defined( APPLE ) || defined( __CYGWIN__ )
  // On Windows and apple, no difference between lower and upper case
  #define KWSYS_GLOB_CASE_INDEPENDENT
#endif

#if defined( _WIN32 ) || defined( __CYGWIN__ )
  // Handle network paths
  #define KWSYS_GLOB_SUPPORT_NETWORK_PATHS
#endif

//----------------------------------------------------------------------------
class GlobInternals
{
public:
  std::vector<std::string> Files;
  std::vector<kwsys::RegularExpression> Expressions;
  std::vector<std::string> TextExpressions;
};

//----------------------------------------------------------------------------
Glob::Glob()
{
  m_Internals = new GlobInternals;
  m_Recurse = false;
}

//----------------------------------------------------------------------------
Glob::~Glob()
{
  delete m_Internals;
}

//----------------------------------------------------------------------------
void Glob::Escape(int ch, char* buffer)
{
  if (! (
      'a' <= ch && ch <= 'z' || 
      'A' <= ch && ch <= 'Z' || 
      '0' <= ch && ch <= '9') )
    {
    sprintf(buffer, "\\%c", ch);
    }
  else
    {
#if defined( KWSYS_GLOB_CASE_INDEPENDENT )
    // On Windows and apple, no difference between lower and upper case
    sprintf(buffer, "%c", tolower(ch));
#else
    sprintf(buffer, "%c", ch);
#endif
    }
}

//----------------------------------------------------------------------------
std::vector<std::string>& Glob::GetFiles()
{
  return m_Internals->Files;
}

//----------------------------------------------------------------------------
std::string Glob::ConvertExpression(const std::string& expr)
{
  
  std::string::size_type i = 0;
  std::string::size_type n = expr.size();

  std::string res = "^";
  std::string stuff = "";

  while ( i < n )
    {
    int c = expr[i];
    i = i+1;
    if ( c == '*' )
      {
      res = res + ".*";
      }
    else if ( c == '?' )
      {
      res = res + ".";
      }
    else if ( c == '[' )
      {
      std::string::size_type j = i;
      if ( j < n && ( expr[j] == '!' || expr[j] == '^' ) )
        {
        j = j+1;
        }
      if ( j < n && expr[j] == ']' )
        {
        j = j+1;
        } 
      while ( j < n && expr[j] != ']' )
        {
        j = j+1;
        }
      if ( j >= n )
        {
        res = res + "\\[";
        }
      else
        {
        stuff = "";
        std::string::size_type cc;
        for ( cc = i; cc < j; cc ++ )
          {
          if ( expr[cc] == '\\' )
            {
            stuff += "\\\\";
            }
          else
            {
            stuff += expr[cc];
            }
          }
        i = j+1;
        if ( stuff[0] == '!' || stuff[0] == '^' )
          {
          stuff = '^' + stuff.substr(1);
          }
        else if ( stuff[0] == '^' )
          {
          stuff = '\\' + stuff;
          }
        res = res + "[" + stuff + "]";
        }
      }
    else
      {
      char buffer[100];
      buffer[0] = 0;
      this->Escape(c, buffer);
      res = res + buffer;
      }
    }
  return res + "$";
}

//----------------------------------------------------------------------------
void Glob::RecurseDirectory(std::string::size_type start,
  const std::string& dir, bool dir_only)
{
  kwsys::Directory d;
  if ( !d.Load(dir.c_str()) )
    {
    return;
    }
  unsigned long cc;
  std::string fullname;
  std::string realname;
  std::string fname;
  for ( cc = 0; cc < d.GetNumberOfFiles(); cc ++ )
    {
    fname = d.GetFile(cc);
    if ( strcmp(fname.c_str(), ".") == 0 ||
      strcmp(fname.c_str(), "..") == 0  )
      {
      continue;
      }

    if ( start == 0 )
      {
      realname = dir + fname;
      }
    else
      {
      realname = dir + "/" + fname;
      }

#if defined( KWSYS_GLOB_CASE_INDEPENDENT )
    // On Windows and apple, no difference between lower and upper case
    fname = kwsys::SystemTools::LowerCase(fname);
#endif

    if ( start == 0 )
      {
      fullname = dir + fname;
      }
    else
      {
      fullname = dir + "/" + fname;
      }

    if ( !dir_only || !kwsys::SystemTools::FileIsDirectory(realname.c_str()) )
      {
      if ( m_Internals->Expressions[m_Internals->Expressions.size()-1].find(fname.c_str()) )
        {
        m_Internals->Files.push_back(realname);
        }
      }
    if ( kwsys::SystemTools::FileIsDirectory(realname.c_str()) )
      {
      this->RecurseDirectory(start+1, realname, dir_only);
      }
    }
}

//----------------------------------------------------------------------------
void Glob::ProcessDirectory(std::string::size_type start, 
  const std::string& dir, bool dir_only)
{
  //std::cout << "ProcessDirectory: " << dir << std::endl;
  bool last = ( start == m_Internals->Expressions.size()-1 );
  if ( last && m_Recurse )
    {
    this->RecurseDirectory(start, dir, dir_only);
    return;
    }
  kwsys::Directory d;
  if ( !d.Load(dir.c_str()) )
    {
    return;
    }
  unsigned long cc;
  std::string fullname;
  std::string realname;
  std::string fname;
  for ( cc = 0; cc < d.GetNumberOfFiles(); cc ++ )
    {
    fname = d.GetFile(cc);
    if ( strcmp(fname.c_str(), ".") == 0 ||
      strcmp(fname.c_str(), "..") == 0  )
      {
      continue;
      }

    if ( start == 0 )
      {
      realname = dir + fname;
      }
    else
      {
      realname = dir + "/" + fname;
      }

#if defined( KWSYS_GLOB_CASE_INDEPENDENT )
    // On Windows and apple, no difference between lower and upper case
    fname = kwsys::SystemTools::LowerCase(fname);
#endif

    if ( start == 0 )
      {
      fullname = dir + fname;
      }
    else
      {
      fullname = dir + "/" + fname;
      }

    //std::cout << "Look at file: " << fname << std::endl;
    //std::cout << "Match: " << m_Internals->TextExpressions[start].c_str() << std::endl;
    //std::cout << "Full name: " << fullname << std::endl;

    if ( (!dir_only || !last) && !kwsys::SystemTools::FileIsDirectory(realname.c_str()) )
      {
      continue;
      }

    if ( m_Internals->Expressions[start].find(fname.c_str()) )
      {
      if ( last )
        {
        m_Internals->Files.push_back(realname);
        }
      else
        {
        this->ProcessDirectory(start+1, realname + "/", dir_only);
        }
      }
    }
}

//----------------------------------------------------------------------------
bool Glob::FindFiles(const std::string& inexpr)
{
  std::string cexpr;
  std::string::size_type cc;
  std::string expr = inexpr;

  m_Internals->Expressions.clear();
  m_Internals->Files.clear();

  if ( !kwsys::SystemTools::FileIsFullPath(expr.c_str()) )
    {
    expr = kwsys::SystemTools::GetCurrentWorkingDirectory();
    expr += "/" + inexpr;
    }
  std::string fexpr = expr;

  int skip = 0;
  int last_slash = 0;
  for ( cc = 0; cc < expr.size(); cc ++ )
    {
    if ( cc > 0 && expr[cc] == '/' && expr[cc-1] != '\\' )
      {
      last_slash = cc;
      }
    if ( cc > 0 && 
      (expr[cc] == '[' || expr[cc] == '?' || expr[cc] == '*') &&
      expr[cc-1] != '\\' )
      {
      break;
      }
    }
  if ( last_slash > 0 )
    {
    //std::cout << "I can skip: " << fexpr.substr(0, last_slash) << std::endl;
    skip = last_slash;
    }
  if ( skip == 0 )
    {
#if defined( KWSYS_GLOB_SUPPORT_NETWORK_PATHS )
    // Handle network paths
    if ( expr[0] == '/' && expr[1] == '/' )
      {
      int cnt = 0;
      for ( cc = 2; cc < expr.size(); cc ++ )
        {
        if ( expr[cc] == '/' )
          {
          cnt ++;
          if ( cnt == 2 )
            {
            break;
            }
          }
        }
      skip = cc + 1;
      }
    else
#endif
      // Handle drive letters on Windows
      if ( expr[1] == ':' && expr[0] != '/' )
        {
        skip = 2;
        }
    }

  if ( skip > 0 )
    {
    expr = expr.substr(skip);
    }

  cexpr = "";
  for ( cc = 0; cc < expr.size(); cc ++ )
    {
    int ch = expr[cc];
    if ( ch == '/' )
      {
      if ( cexpr.size() > 0 )
        {
        this->AddExpression(cexpr.c_str());
        }
      cexpr = "";
      }
    else
      {
      cexpr.append(1, (char)ch);
      }
    }
  if ( cexpr.size() > 0 )
    {
    this->AddExpression(cexpr.c_str());
    }

  // Handle network paths
  if ( skip > 0 )
    {
    this->ProcessDirectory(0, fexpr.substr(0, skip) + "/",
      true);     
    }
  else
    {
    this->ProcessDirectory(0, "/", true);
    }
  return true;
}

void Glob::AddExpression(const char* expr)
{
  m_Internals->Expressions.push_back(
    kwsys::RegularExpression(
      this->ConvertExpression(expr).c_str()));
  m_Internals->TextExpressions.push_back(this->ConvertExpression(expr));
}

} // namespace KWSYS_NAMESPACE

