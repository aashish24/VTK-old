/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$
  Thanks:    Thanks to C. Charles Law who developed this class.

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
// .NAME vtkImageReader - Superclass of binary file readers.
// .SECTION Description
// vtkImageReader provides methods needed to read a region from a file.


#ifndef __vtkImageReader_h
#define __vtkImageReader_h

#include <iostream.h>
#include <fstream.h>
#include "vtkImageSource.h"
#include "vtkTransform.h"

#define VTK_FILE_BYTE_ORDER_BIG_ENDIAN 0
#define VTK_FILE_BYTE_ORDER_LITTLE_ENDIAN 1

class VTK_EXPORT vtkImageReader : public vtkImageSource
{
public:
  vtkImageReader();
  ~vtkImageReader();
  static vtkImageReader *New() {return new vtkImageReader;};
  const char *GetClassName() {return "vtkImageReader";};
  void PrintSelf(ostream& os, vtkIndent indent);   

  // Description:
  // Specify file name for the image file. You should specify either
  // a FileName or a FilePrefix. Use FilePrefix if the data is stored 
  // in multiple files.
  void SetFileName(char *);
  vtkGetStringMacro(FileName);

  // Description:
  // Specify file prefix for the image file(s).You should specify either
  // a FileName or FilePrefix. Use FilePrefix if the data is stored
  // in multiple files.
  void SetFilePrefix(char *);
  vtkGetStringMacro(FilePrefix);

  // Description:
  // The sprintf format used to build filename from FilePrefix and number.
  void SetFilePattern(char *);
  vtkGetStringMacro(FilePattern);

  void SetDataScalarTypeToFloat(){this->SetDataScalarType(VTK_FLOAT);}
  void SetDataScalarTypeToInt(){this->SetDataScalarType(VTK_INT);}
  void SetDataScalarTypeToShort(){this->SetDataScalarType(VTK_SHORT);}
  void SetDataScalarTypeToUnsignedShort()
    {this->SetDataScalarType(VTK_UNSIGNED_SHORT);}
  void SetDataScalarTypeToUnsignedChar()
    {this->SetDataScalarType(VTK_UNSIGNED_CHAR);}
  void SetDataScalarType(int type);
  // Description:
  // Get the file format.  Pixels are this type in the file.
  vtkGetMacro(DataScalarType, int);
  
  // Description:
  // Get/Set the extent of the data on disk.  
  void SetDataExtent(int num, int *extent);
  vtkImageSetExtentMacro(DataExtent);
  void GetDataExtent(int num, int *extent);
  vtkImageGetExtentMacro(DataExtent);
  
  // Description:
  // Set/get the data VOI. You can limit the reader to only
  // read a subset of the data. 
  void SetDataVOI(int num, int *extent);
  vtkImageSetExtentMacro(DataVOI);
  void GetDataVOI(int num, int *extent);
  vtkImageGetExtentMacro(DataVOI);
  
  // Description:
  // The number of scalar components refers to the number of scalars
  // per pixel to store color information.
  vtkSetMacro(NumberOfScalarComponents,int);
  vtkGetMacro(NumberOfScalarComponents,int);

  // Description:
  // The number of dimensions stored in a file. This defaults to two.
  vtkSetMacro(FileDimensionality, int);
  vtkGetMacro(FileDimensionality, int);
  
  // Description:
  // Set/Get the spacing of the data in the file.
  void SetDataSpacing(int num, float *ratio);
  vtkImageSetMacro(DataSpacing,float);
  void GetDataSpacing(int num, float *ratio);
  vtkImageGetMacro(DataSpacing,float);
  float *GetDataSpacing() {return this->DataSpacing;};  
  
  // Description:
  // Set/Get the origin of the data (location of first pixel in the file).
  void SetDataOrigin(int num, float *ratio);
  vtkImageSetMacro(DataOrigin,float);
  void GetDataOrigin(int num, float *ratio);
  vtkImageGetMacro(DataOrigin,float);
  float *GetDataOrigin() {return this->DataOrigin;};  
  
  void UpdateImageInformation();
  vtkImageCache *GetOutput();
  
  // Description:
  // Get the size of the header computed by this object.
  int GetHeaderSize();
  // Description:
  // If there is a tail on the file, you want to explicitly set the
  // header size.
  void SetHeaderSize(int size);
  
  // Description:
  // Set/Get the Data mask.
  vtkGetMacro(DataMask,unsigned short);
  void SetDataMask(int val) 
  {this->DataMask = ((unsigned short)(val)); this->Modified();};
  
  // Description:
  // Set/Get transformation matrix to transform the data from slice space
  // into world space. This matirx must be a permutation matrix. To qualify,
  // the sums of the rows must be + or - 1.
  vtkSetObjectMacro(Transform,vtkTransform);
  vtkGetObjectMacro(Transform,vtkTransform);

  // Description:
  // These methods should be used instead of the SwapBytes methods.
  // They indicate the byte ordering of the file you are trying
  // to read in. These methods will then either swap or not swap
  // the bytes depending on the byte ordering of the machine it is
  // being run on. For example, reading in a BigEndian file on a
  // BigEndian machine will result in no swapping. Trying to read
  // the same file on a LittleEndian machine will result in swapping.
  // As a quick note most UNIX machines are BigEndian while PC's
  // and VAX tend to be LittleEndian. So if the file you are reading
  // in was generated on a VAX or PC, SetDataByteOrderToLittleEndian 
  // otherwise SetDataByteOrderToBigEndian. 
  void SetDataByteOrderToBigEndian();
  void SetDataByteOrderToLittleEndian();
  int GetDataByteOrder();
  void SetDataByteOrder(int);
  char *GetDataByteOrderAsString();

  // Description:
  // Set/Get the byte swapping to explicitely swap the bytes of a file.
  vtkSetMacro(SwapBytes,int);
  vtkGetMacro(SwapBytes,int);
  vtkBooleanMacro(SwapBytes,int);

  // Warning !!!
  // following should only be used by methods or template helpers, not users
  ifstream *File;
  int DataIncrements[VTK_IMAGE_DIMENSIONS];
  int DataExtent[VTK_IMAGE_EXTENT_DIMENSIONS];
  unsigned short DataMask;  // Mask each pixel with ...
  int SwapBytes;
  void ComputeInverseTransformedExtent(int inExtent[8],
				       int outExtent[8]);
  void ComputeInverseTransformedIncrements(int inIncr[4],
					   int outIncr[4]);

  void OpenFile();
  void OpenAndSeekFile(int extent[8], int slice);
  char *InternalFileName;
  char *FileName;
  char *FilePrefix;
  char *FilePattern;
  int NumberOfScalarComponents;
  int FileLowerLeft;
  
protected:
  int FileDimensionality;
  int HeaderSize;
  int DataScalarType;
  int ManualHeaderSize;
  int Initialized;
  vtkTransform *Transform;

  void ComputeTransformedSpacing (float Spacing[4]);
  void ComputeTransformedOrigin (float origin[4]);
  void ComputeTransformedExtent(int inExtent[8],
				int outExtent[8]);
  void ComputeTransformedIncrements(int inIncr[4],
				    int outIncr[4]);

  int DataDimensions[VTK_IMAGE_DIMENSIONS];
  float DataSpacing[VTK_IMAGE_DIMENSIONS];
  float DataOrigin[VTK_IMAGE_DIMENSIONS];
  int DataVOI[VTK_IMAGE_EXTENT_DIMENSIONS];

  void Execute(vtkImageRegion *region);
  virtual void ComputeDataIncrements();
};

#endif
