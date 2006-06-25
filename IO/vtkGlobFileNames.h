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
// .NAME vtkGlobFileNames - find files that match a wildcard pattern
// .SECTION Description
// vtkGlobFileNames is a utility for finding files and directories
// that match a given wildcard pattern.  Allowed wildcards are
// *, ?, [...], [!...]. The "*" wildcard matches any substring,
// the "?" matches any single character, the [...] matches any one of
// the enclosed characters, e.g. [abc] will match one of a, b, or c,
// while [0-9] will match any digit, and [!...] will match any single
// character except for the ones within the brackets.  Special
// treatment is given to "/" (or "\" on Windows) because these are
// path separators.  These are never matched by a wildcard, they are
// only matched with another file separator.
// .SECTION Caveats
// This function performs case-sensitive matches on UNIX and
// case-insensitive matches on Windows.
// .SECTION See Also
// vtkDirectory

#ifndef __vtkGlobFileNames_h
#define __vtkGlobFileNames_h

#include "vtkObject.h"

class vtkStringArray;

class VTK_IO_EXPORT vtkGlobFileNames : public vtkObject
{
public:
  // Description:
  // Return the class name as a string.
  vtkTypeRevisionMacro(vtkGlobFileNames,vtkObject);

  // Description:
  // Create a new vtkGlobFileNames object.
  static vtkGlobFileNames *New();

  // Description:
  // Print directory to stream.
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Reset the glob by clearing the list of output filenames.
  void Reset();

  // Description:
  // Search for all files that match the given expression,
  // sort them, and add them to the output.  This method can
  // be called repeatedly to add files matching additional patterns.
  // Returns 1 if successful, otherwise returns zero.
  int AddFileNames(const char* pattern);

  // Description:
  // Recurse into subdirectories.
  vtkSetMacro(Recurse, int);
  vtkBooleanMacro(Recurse, int);
  vtkGetMacro(Recurse, int);

  // Description:
  // Return the number of files found.
  int GetNumberOfFileNames();

  // Description:
  // Return the file at the given index, the indexing is 0 based.
  const char* GetFileName(int index);

  // Description:
  // Get an array that contains all the file names.
  vtkGetObjectMacro(FileNames, vtkStringArray);

protected:
  // Description:
  // Set the wildcard pattern.
  vtkSetStringMacro(Pattern);
  vtkGetStringMacro(Pattern);

  vtkGlobFileNames();
  ~vtkGlobFileNames();

private:
  char* Pattern;            // Wildcard pattern
  int Recurse;              // Recurse into subdirectories
  vtkStringArray *FileNames;    // VTK array of files

private:
  vtkGlobFileNames(const vtkGlobFileNames&);  // Not implemented.
  void operator=(const vtkGlobFileNames&);  // Not implemented.
};

#endif
