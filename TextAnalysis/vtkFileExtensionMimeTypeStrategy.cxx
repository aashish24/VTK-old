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

#include <vtkFileExtensionMimeTypeStrategy.h>
#include <vtkObjectFactory.h>
#include <vtkStdString.h>

#include <boost/algorithm/string.hpp>

vtkCxxRevisionMacro(vtkFileExtensionMimeTypeStrategy, "$Revision$");
vtkStandardNewMacro(vtkFileExtensionMimeTypeStrategy);

class vtkFileExtensionMimeTypeStrategy::implementation
{
public:
  static bool Lookup(const vtkStdString& uri, const vtkStdString& suffix, const vtkStdString& mime_type, vtkStdString& result)
  {
    if(boost::iends_with(uri, suffix))
      {
      result = mime_type;
      return true;
      }
      
    return false;
  }
};

vtkFileExtensionMimeTypeStrategy::vtkFileExtensionMimeTypeStrategy()
{
}

vtkFileExtensionMimeTypeStrategy::~vtkFileExtensionMimeTypeStrategy()
{
}

void vtkFileExtensionMimeTypeStrategy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

vtkStdString vtkFileExtensionMimeTypeStrategy::Lookup(const vtkStdString& uri, const vtkTypeUInt8* vtkNotUsed(begin), const vtkTypeUInt8* vtkNotUsed(end))
{
  vtkStdString mime_type;

  if(implementation::Lookup(uri, ".ai", "application/postscript", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".bmp", "image/bmp", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".c", "text/plain", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".cpp", "text/plain", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".css", "text/css", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".csv", "text/csv", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".cxx", "text/plain", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".dll", "application/octet-stream", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".doc", "application/msword", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".dot", "application/msword", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".dvi", "application/x-dvi", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".eps", "application/postscript", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".exe", "application/octet-stream", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".gz", "application/x-gzip", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".gif", "image/gif", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".h", "text/plain", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".hpp", "text/plain", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".htm", "text/html", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".html", "text/html", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".hxx", "text/plain", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".ico", "image/x-icon", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".jpe", "image/jpeg", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".jpeg", "image/jpeg", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".jpg", "image/jpeg", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".js", "application/x-javascript", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".latex", "application/x-latex", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".lib", "application/octet-stream", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".pdf", "application/pdf", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".png", "image/png", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".ppt", "application/vnd.ms-powerpoint", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".ps", "application/postscript", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".shtml", "text/html", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".tcl", "application/x-tcl", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".tif", "image/tiff", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".tiff", "image/tiff", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".txt", "text/plain", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".xls", "application/vnd.ms-excel", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".xml", "text/xml", mime_type)) return mime_type;
  if(implementation::Lookup(uri, ".zip", "application/zip", mime_type)) return mime_type;

  return vtkStdString();
}

