/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1995 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
// .NAME vtkVolume16Reader - read 16 bit image files
// .SECTION Description
// vtkVolume16Reader is a source object that reads 16 bit image files.
//
// Volume16Reader creates structured point datasets. The dimension of the 
// dataset depends upon the number of files read. Reading a single file 
// results in a 2D image, while reading more than one file results in a 
// 3D volume.
//
// File names are created using FilePattern and FilePrefix as follows:
// sprintf (filename, FilePattern, FilePrefix, number);
// where number is in the range ImageRange[0] to ImageRange[1]. If
// ImageRange[1] <= ImageRange[0], then slice number ImageRange[0] is
// read. Thus to read an image set ImageRange[0] = ImageRange[1] = slice 
// number. The default behavior is to read a single file (i.e., image slice 1).
//
// The DataMask instance variable is used to read data files with imbedded
// connectivity or segmentation information. For example, some data has
// the high order bit set to indicate connected surface. The DataMask allows
// you to select this data. Other important ivars include HeaderSize, which
// allows you to skip over initial info, and SwapBytes, which turns on/off
// byte swapping.
// .SECTION See Also
// vtkSliceCubes vtkMarchingCubes

#ifndef __vtkVolume16Reader_h
#define __vtkVolume16Reader_h

#include <stdio.h>
#include "vtkShortScalars.hh"
#include "vtkStructuredPointsSource.hh"

class vtkVolume16Reader : public vtkStructuredPointsSource
{
public:
  vtkVolume16Reader();
  char *GetClassName() {return "vtkVolume16Reader";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify file prefix for the image file(s).
  vtkSetStringMacro(FilePrefix);
  vtkGetStringMacro(FilePrefix);

  // Description:
  // The sprintf format to use to build filename from FilePrefix and number.
  vtkSetStringMacro(FilePattern);
  vtkGetStringMacro(FilePattern);

  // Description:
  // Set the range of files to read.
  vtkSetVector2Macro(ImageRange,int);
  vtkGetVectorMacro(ImageRange,int,2);

  // Description:
  // Specify an aspect ratio for the data.
  vtkSetVector3Macro(DataAspectRatio,float);
  vtkGetVectorMacro(DataAspectRatio,float,3);

  // Description:
  // Specify the origin for the data.
  vtkSetVector3Macro(DataOrigin,float);
  vtkGetVectorMacro(DataOrigin,float,3);

  // Description:
  // Specify the dimensions for the data.
  vtkSetVector2Macro(DataDimensions,int);
  vtkGetVectorMacro(DataDimensions,int,2);

  // Description:
  // Specify a mask used to eliminate data in the data file (e.g.,
  // connectivity bits).
  vtkSetMacro(DataMask,short);
  vtkGetMacro(DataMask,short);

  // Description:
  // Specify the number of bytes to seek over at start of image
  vtkSetMacro(HeaderSize,int);
  vtkGetMacro(HeaderSize,int);

  // Description:
  // Turn on/off byte swapping
  vtkSetMacro(SwapBytes,int);
  vtkGetMacro(SwapBytes,int);
  vtkBooleanMacro(SwapBytes,int);

  // Other objects make use of these methods
  vtkShortScalars *ReadImage(int sliceNumber, int dim[2]);
  vtkShortScalars *ReadVolume(int first, int last, int dim[2]);

protected:
  void Execute();
  char *FilePrefix;
  char *FilePattern;
  int ImageRange[2];
  float DataAspectRatio[3];
  int   DataDimensions[2];
  float DataOrigin[3];
  short DataMask;
  int   SwapBytes;
  int   HeaderSize;

  int Read16BitImage(FILE *fp, short *pixels, int xsize, int ysize, int skip, int swapBytes);

};

#endif


