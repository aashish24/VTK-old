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
// .NAME vtkWin32ProcessOutputWindow - Win32-specific output window class
// .SECTION Description
// vtkWin32ProcessOutputWindow executes a process and sends messages
// to its standard input pipe.  This is useful to have a separate
// process display VTK errors so that if a VTK application crashes,
// the error messages are still available.

#ifndef __vtkWin32ProcessOutputWindow_h
#define __vtkWin32ProcessOutputWindow_h

#include "vtkOutputWindow.h"

class VTK_COMMON_EXPORT vtkWin32ProcessOutputWindow : public vtkOutputWindow
{
public:
  vtkTypeRevisionMacro(vtkWin32ProcessOutputWindow,vtkOutputWindow);
  static vtkWin32ProcessOutputWindow* New();

  // Description:
  // Send text to the output window process.
  virtual void DisplayText(const char*);

  // Description:
  // Get/Set the executable that is executed and given messages.
  vtkSetStringMacro(Executable);
  vtkGetStringMacro(Executable);
protected:
  vtkWin32ProcessOutputWindow();
  ~vtkWin32ProcessOutputWindow();

  int Initialize();
  void Write(const char* data, int length);

  // The write end of the pipe to the child process.
  HANDLE OutputPipe;

  // The executable to run.
  char* Executable;

  // Whether the pipe has been broken.
  int Broken;
private:
  vtkWin32ProcessOutputWindow(const vtkWin32ProcessOutputWindow&);  // Not implemented.
  void operator=(const vtkWin32ProcessOutputWindow&);  // Not implemented.
};

#endif
