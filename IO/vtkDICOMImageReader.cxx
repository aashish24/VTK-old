
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

#include "vtkDICOMImageReader.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkDirectory.h"

#include "DICOMParser.h"
#include "DICOMAppHelper.h"

#include <vtkstd/vector>
#include <vtkstd/string>

vtkCxxRevisionMacro(vtkDICOMImageReader, "$Revision$");
vtkStandardNewMacro(vtkDICOMImageReader);

class vtkDICOMImageReaderVector : public vtkstd::vector<vtkstd::string>
{

};

vtkDICOMImageReader::vtkDICOMImageReader()
{
  this->Parser = new DICOMParser();
  this->AppHelper = new DICOMAppHelper();
  this->DirectoryName = NULL;
  this->DICOMFileNames = new vtkDICOMImageReaderVector();

}

vtkDICOMImageReader::~vtkDICOMImageReader()
{
  delete this->Parser;
  delete this->AppHelper;
  delete this->DICOMFileNames;
}

void vtkDICOMImageReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkImageReader2::PrintSelf(os, indent);
  if (this->DirectoryName)
    {
    os << "DirectoryName : " << this->DirectoryName << "\n";
    }
  else
    {
    os << "DirectoryName : (NULL)" << "\n";
    }
  if (this->FileName)
    {
    os << "FileName : " << this->FileName << "\n";
    }
  else
    {
    os << "FileName : (NULL)" << "\n";
    }

}

int vtkDICOMImageReader::CanReadFile(const char* fname)
{
  bool canOpen = this->Parser->OpenFile((char*) fname);
  if (canOpen == false)
    {
    vtkErrorMacro("DICOMParser couldn't open : " << fname);
    return 0;
    }
  bool canRead = this->Parser->IsDICOMFile();
  if (canRead == true)
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

void vtkDICOMImageReader::ExecuteInformation()
{
  if (this->FileName == NULL && this->DirectoryName == NULL)
    {
    return;
    }

  if (this->FileName)
    {
    this->DICOMFileNames->clear();

    this->AppHelper->ClearSeriesUIDMap();
    this->AppHelper->ClearSliceNumberMap();

    this->Parser->ClearAllDICOMTagCallbacks();
 
    this->Parser->OpenFile(this->FileName);
    this->AppHelper->SetFileName(this->FileName);
    this->AppHelper->RegisterCallbacks(this->Parser);

    this->Parser->ReadHeader();
    this->SetupOutputInformation(1);
    }
  else if (this->DirectoryName)
    {
    vtkDirectory* dir = vtkDirectory::New();
    int opened = dir->Open(this->DirectoryName);
    if (!opened)
      {
      vtkErrorMacro("Couldn't open " << this->DirectoryName);
      dir->Delete();
      return;
      }
    int numFiles = dir->GetNumberOfFiles();

    vtkDebugMacro( << "There are " << numFiles << " files in the directory.");

    this->DICOMFileNames->clear();
    this->AppHelper->ClearSeriesUIDMap();
    this->AppHelper->ClearSliceNumberMap();

    for (int i = 0; i < numFiles; i++)
      {
      if (strcmp(dir->GetFile(i), ".") == 0 ||
          strcmp(dir->GetFile(i), "..") == 0)
        {
        continue;
        }

      vtkstd::string temp = this->DirectoryName;
      vtkstd::string temp2 = dir->GetFile(i);
      vtkstd::string delim = "/";
      vtkstd::string fileString = temp + delim + temp2;

      int val = this->CanReadFile(fileString.c_str());

      if (val == 1)
        {
        vtkDebugMacro( << "Adding " << fileString.c_str() << " to DICOMFileNames.");
        this->DICOMFileNames->push_back(fileString);
        }
      else
        {
        vtkDebugMacro( << fileString.c_str() << " - DICOMParser CanReadFile returned : " << val);
        }

      }
    vtkstd::vector<vtkstd::string>::iterator iter;
        
    for (iter = this->DICOMFileNames->begin();
         iter != this->DICOMFileNames->end();
         iter++)
      {
      char* fn = (char*) (*iter).c_str();
      vtkDebugMacro( << "Trying : " << fn);
 
      bool couldOpen = this->Parser->OpenFile(fn);
      if (!couldOpen)
        {
        dir->Delete();
        return;
        }

      //HERE
      this->Parser->ClearAllDICOMTagCallbacks();
      this->AppHelper->RegisterCallbacks(this->Parser);

      this->AppHelper->SetFileName(fn);
      this->Parser->ReadHeader();

      vtkDebugMacro( << "File name : " << fn );
      vtkDebugMacro( << "Slice number : " << this->AppHelper->GetSliceNumber());
      }
    
    vtkstd::vector<vtkstd::pair<int, vtkstd::string> > sortedFiles;
    this->AppHelper->GetSliceNumberFilenamePairs(sortedFiles);

    this->SetupOutputInformation(static_cast<int>(sortedFiles.size()));

    //this->AppHelper->OutputSeries();

    if (sortedFiles.size() > 0)
      {
      this->DICOMFileNames->clear();
      vtkstd::vector<vtkstd::pair<int, vtkstd::string> >::iterator siter;
      for (siter = sortedFiles.begin();
           siter != sortedFiles.end();
           siter++)
        {
        vtkDebugMacro(<< "Sorted filename : " << (*siter).second.c_str());
        vtkDebugMacro(<< "Adding file " << (*siter).second.c_str() << " at slice : " << (*siter).first);
        this->DICOMFileNames->push_back((*siter).second);
        }
      }
    else
      {
      vtkErrorMacro( << "Couldn't get sorted files. Slices may be in wrong order!");
      }
    dir->Delete();
    }
    
}

void vtkDICOMImageReader::ExecuteData(vtkDataObject *output)
{
  vtkImageData *data = this->AllocateOutputData(output);
  data->GetPointData()->GetScalars()->SetName("DICOMImage");

  if (this->FileName)
    {
    vtkDebugMacro( << "Single file : " << this->FileName);
    this->Parser->ClearAllDICOMTagCallbacks();
    this->Parser->OpenFile(this->FileName);
    this->AppHelper->ClearSeriesUIDMap();
    this->AppHelper->ClearSliceNumberMap();
    this->AppHelper->SetFileName(this->FileName);
    this->AppHelper->RegisterCallbacks(this->Parser);
    this->AppHelper->RegisterPixelDataCallback();

    this->Parser->ReadHeader();

    void* imgData = NULL;
    DICOMParser::VRTypes dataType;
    unsigned long imageDataLength;

    this->AppHelper->GetImageData(imgData, dataType, imageDataLength);

    void* buffer = data->GetScalarPointer();
    memcpy(buffer, imgData, imageDataLength);
    }
  else if (this->DICOMFileNames->size() > 0)
    {
    vtkDebugMacro( << "Multiple files (" << static_cast<int>(this->DICOMFileNames->size()) << ")");
    this->Parser->ClearAllDICOMTagCallbacks();
    this->AppHelper->ClearSeriesUIDMap();
    this->AppHelper->ClearSliceNumberMap();
    this->AppHelper->RegisterCallbacks(this->Parser);
    this->AppHelper->RegisterPixelDataCallback();

    void* buffer = data->GetScalarPointer();
    
    vtkstd::vector<vtkstd::string>::iterator fiter; 

    int count = 0;
    int numFiles = static_cast<int>(this->DICOMFileNames->size());

    for (fiter = this->DICOMFileNames->begin();
         fiter != this->DICOMFileNames->end();
         fiter++)
      {
      count++;
      vtkDebugMacro( << "File : " << (*fiter).c_str());
      this->Parser->OpenFile((char*)(*fiter).c_str());
      this->AppHelper->SetFileName((char*)(*fiter).c_str());

      this->Parser->ReadHeader();

      void* imgData = NULL;
      DICOMParser::VRTypes dataType;
      unsigned long imageDataLengthInBytes;

      this->AppHelper->GetImageData(imgData, dataType, imageDataLengthInBytes);

      memcpy(buffer, imgData, imageDataLengthInBytes);
      buffer = ((char*) buffer) + imageDataLengthInBytes;

      this->UpdateProgress(float(count)/float(numFiles));
      int len = static_cast<int> (strlen((char*) (*fiter).c_str()));
      char* filename = new char[len+1];
      strcpy(filename, (char*) (*fiter).c_str());
      this->SetProgressText(filename);

      }
    }
  else
    {
    vtkDebugMacro( << "Execute data -- no files!");
    }
}

void vtkDICOMImageReader::SetupOutputInformation(int num_slices)
{
  int width = this->AppHelper->GetWidth();
  int height = this->AppHelper->GetHeight();
  int bit_depth = this->AppHelper->GetBitsAllocated();
  int num_comp = this->AppHelper->GetNumberOfComponents();

  this->DataExtent[0] = 0;
  this->DataExtent[1] = width - 1;
  this->DataExtent[2] = 0;
  this->DataExtent[3] = height - 1;
  this->DataExtent[4] = 0;
  this->DataExtent[5] = num_slices - 1;

  bool isFloat = this->AppHelper->RescaledImageDataIsFloat();

  bool sign = this->AppHelper->RescaledImageDataIsSigned();

  if (isFloat)
    {
    this->SetDataScalarTypeToFloat();
    }
  else if (bit_depth <= 8)
    {
    this->SetDataScalarTypeToUnsignedChar();
    }
  else
    {
    if (sign)
      {
      this->SetDataScalarTypeToShort();
      }
    else
      {
      this->SetDataScalarTypeToUnsignedShort();
      }
    }
  this->SetNumberOfScalarComponents(num_comp);

  this->vtkImageReader2::ExecuteInformation();
}

void vtkDICOMImageReader::SetDirectoryName(const char* dn)
{
  if (dn == NULL)
    {
    return;
    }
  int len = static_cast<int>(strlen(dn));
  if (this->DirectoryName != NULL)
    {
    delete [] this->DirectoryName;
    }
  this->DirectoryName = new char[len+1];
  strcpy(this->DirectoryName, dn);
  this->FileName = NULL;
  this->Modified();
}


