/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPDataSetWriter - Manages writing pieces of a data set.
// .SECTION Description
// vtkPDataSetWriter will write a piece of a file, and will also create
// a metadata file that lists all of the files in a data set.


#ifndef __vtkPDataSetWriter_h
#define __vtkPDataSetWriter_h

#include "vtkDataSetWriter.h"
#include "vtkImageData.h"
#include "vtkStructuredGrid.h"
#include "vtkRectilinearGrid.h"


class VTK_PARALLEL_EXPORT vtkPDataSetWriter : public vtkDataSetWriter
{
public:
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeRevisionMacro(vtkPDataSetWriter,vtkDataSetWriter);
  static vtkPDataSetWriter *New();

  // Description:
  // Write the pvtk file and cooresponding vtk files.
  virtual void Write();

  // Description:
  // This is how many pieces the whole data set will be divided into.
  void SetNumberOfPieces(int num);
  vtkGetMacro(NumberOfPieces, int);

  // Description:
  // Extra ghost cells will be written out to each piece file
  // if this value is larger than 0.
  vtkSetMacro(GhostLevel, int);
  vtkGetMacro(GhostLevel, int);
  
  // Description:
  // This is the range of pieces that that this writer is 
  // responsible for writing.  All pieces must be written
  // by some process.  The process that writes piece 0 also
  // writes the pvtk file that lists all the piece file names.
  vtkSetMacro(StartPiece, int);
  vtkGetMacro(StartPiece, int);
  vtkSetMacro(EndPiece, int);
  vtkGetMacro(EndPiece, int);
  
  // Description:
  // This file pattern uses the file name and piece number
  // to contruct a file name for the piece file.
  vtkSetStringMacro(FilePattern);
  vtkGetStringMacro(FilePattern);

  // Description:
  // This flag determines whether to use absolute paths for the piece files.
  // By default the pieces are put in the main directory, and the piece file
  // names in the meta data pvtk file are relative to this directory.
  // This should make moving the whole lot to another directory, an easier task.
  vtkSetMacro(UseRelativeFileNames, int);
  vtkGetMacro(UseRelativeFileNames, int);
  vtkBooleanMacro(UseRelativeFileNames, int);

protected:
  vtkPDataSetWriter();
  ~vtkPDataSetWriter();
  vtkPDataSetWriter(const vtkPDataSetWriter&);
  void operator=(const vtkPDataSetWriter&);

//BTX
  ostream *OpenFile();
  void WriteUnstructuredMetaData(vtkDataSet *input, 
                          char *root, char *str, ostream *fptr);
  void WriteImageMetaData(vtkImageData *input, 
                          char *root, char *str, ostream *fptr);
  void WriteRectilinearGridMetaData(vtkRectilinearGrid *input,
                          char *root, char *str, ostream *fptr);
  void WriteStructuredGridMetaData(vtkStructuredGrid *input,
                          char *root, char *str, ostream *fptr);
//ETX

  int StartPiece;
  int EndPiece;
  int NumberOfPieces;
  int GhostLevel;

  int UseRelativeFileNames;

  char *FilePattern;
};

#endif
