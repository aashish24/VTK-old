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
#include "vtkImageWriter.h"

#include "vtkCommand.h"
#include "vtkErrorCode.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"

#if !defined(_WIN32) || defined(__CYGWIN__)
# include <unistd.h> /* unlink */
#else
# include <io.h> /* unlink */
#endif

vtkCxxRevisionMacro(vtkImageWriter, "$Revision$");
vtkStandardNewMacro(vtkImageWriter);

#ifdef write
#undef write
#endif

#ifdef close
#undef close
#endif


//----------------------------------------------------------------------------
vtkImageWriter::vtkImageWriter()
{
  this->FilePrefix = NULL;
  this->FilePattern = NULL;
  this->FileName = NULL;
  this->InternalFileName = NULL;
  this->FileNumber = 0;
  this->FileDimensionality = 2;

  this->FilePattern = new char[strlen("%s.%d") + 1];
  strcpy(this->FilePattern, "%s.%d");
  
  this->FileLowerLeft = 0;
  
  this->MinimumFileNumber = this->MaximumFileNumber = 0;
  this->FilesDeleted = 0;
}



//----------------------------------------------------------------------------
vtkImageWriter::~vtkImageWriter()
{
  // get rid of memory allocated for file names
  if (this->FilePrefix)
    {
    delete [] this->FilePrefix;
    this->FilePrefix = NULL;
    }
  if (this->FilePattern)
    {
    delete [] this->FilePattern;
    this->FilePattern = NULL;
    }
  if (this->FileName)
    {
    delete [] this->FileName;
    this->FileName = NULL;
    }
}


//----------------------------------------------------------------------------
void vtkImageWriter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "FileName: " <<
    (this->FileName ? this->FileName : "(none)") << "\n";
  os << indent << "FilePrefix: " << 
    (this->FilePrefix ? this->FilePrefix : "(none)") << "\n";
  os << indent << "FilePattern: " << 
    (this->FilePattern ? this->FilePattern : "(none)") << "\n";

  os << indent << "FileDimensionality: " << this->FileDimensionality << "\n";
}


//----------------------------------------------------------------------------
void vtkImageWriter::SetInput(vtkImageData *input)
{
  this->vtkProcessObject::SetNthInput(0, input);
}

//----------------------------------------------------------------------------
vtkImageData *vtkImageWriter::GetInput()
{
  if (this->NumberOfInputs < 1)
    {
    return NULL;
    }
  
  return (vtkImageData *)(this->Inputs[0]);
}



//----------------------------------------------------------------------------
// This function sets the name of the file. 
void vtkImageWriter::SetFileName(const char *name)
{
  if ( this->FileName && name && (!strcmp(this->FileName,name)))
    {
    return;
    }
  if (!name && !this->FileName)
    {
    return;
    }
  if (this->FileName)
    {
    delete [] this->FileName;
    }
  if (this->FilePrefix)
    {
    delete [] this->FilePrefix;
    this->FilePrefix = NULL;
    }  
  this->FileName = new char[strlen(name) + 1];
  strcpy(this->FileName, name);
  this->Modified();
}

//----------------------------------------------------------------------------
// This function sets the prefix of the file name. "image" would be the
// name of a series: image.1, image.2 ...
void vtkImageWriter::SetFilePrefix(char *prefix)
{
  if ( this->FilePrefix && prefix && (!strcmp(this->FilePrefix,prefix)))
    {
    return;
    }
  if (!prefix && !this->FilePrefix)
    {
    return;
    }
  if (this->FilePrefix)
    {
    delete [] this->FilePrefix;
    }
  if (this->FileName)
    {
    delete [] this->FileName;
    this->FileName = NULL;
    }  
  this->FilePrefix = new char[strlen(prefix) + 1];
  strcpy(this->FilePrefix, prefix);
  this->Modified();
}

//----------------------------------------------------------------------------
// This function sets the pattern of the file name which turn a prefix
// into a file name. "%s.%3d" would be the
// pattern of a series: image.001, image.002 ...
void vtkImageWriter::SetFilePattern(const char *pattern)
{
  if ( this->FilePattern && pattern && 
       (!strcmp(this->FilePattern,pattern)))
    {
    return;
    }
  if (!pattern && !this->FilePattern)
    {
    return;
    }
  if (this->FilePattern)
    {
    delete [] this->FilePattern;
    }
  if (this->FileName)
    {
    delete [] this->FileName;
    this->FileName = NULL;
    }
  this->FilePattern = new char[strlen(pattern) + 1];
  strcpy(this->FilePattern, pattern);
  this->Modified();
}

//----------------------------------------------------------------------------
// Writes all the data from the input.
void vtkImageWriter::Write()
{
  this->SetErrorCode(vtkErrorCode::NoError);
  
  // Error checking
  if ( this->GetInput() == NULL )
    {
    vtkErrorMacro(<<"Write:Please specify an input!");
    return;
    }
  if ( ! this->FileName && !this->FilePattern)
    {
    vtkErrorMacro(<<"Write:Please specify either a FileName or a file prefix and pattern");
    this->SetErrorCode(vtkErrorCode::NoFileNameError);
    return;
    }
  
  // Make sure the file name is allocated
  this->InternalFileName = 
    new char[(this->FileName ? strlen(this->FileName) : 1) +
            (this->FilePrefix ? strlen(this->FilePrefix) : 1) +
            (this->FilePattern ? strlen(this->FilePattern) : 1) + 10];
  
  // Fill in image information.
  this->GetInput()->UpdateInformation();
  this->GetInput()->SetUpdateExtent(this->GetInput()->GetWholeExtent());
  this->FileNumber = this->GetInput()->GetWholeExtent()[4];
  this->MinimumFileNumber = this->MaximumFileNumber = this->FileNumber;
  this->FilesDeleted = 0;

  // Write

  this->InvokeEvent(vtkCommand::StartEvent);
  this->UpdateProgress(0.0);
  this->RecursiveWrite(2, this->GetInput(), NULL);

  if (this->ErrorCode == vtkErrorCode::OutOfDiskSpaceError)
    {
    this->DeleteFiles();
    }

  this->UpdateProgress(1.0);
  this->InvokeEvent(vtkCommand::EndEvent);

  delete [] this->InternalFileName;
  this->InternalFileName = NULL;
}

//----------------------------------------------------------------------------
// Breaks region into pieces with correct dimensionality.
void vtkImageWriter::RecursiveWrite(int axis, vtkImageData *cache,
                                    ofstream *file)
{
  vtkImageData    *data;
  int             fileOpenedHere = 0;
  int             *ext;

  // if we need to open another slice, do it
  if (!file && (axis + 1) == this->FileDimensionality)
    {
    // determine the name
    if (this->FileName)
      {
      sprintf(this->InternalFileName,"%s",this->FileName);
      }
    else 
      {
      if (this->FilePrefix)
        {
        sprintf(this->InternalFileName, this->FilePattern, 
                this->FilePrefix, this->FileNumber);
        }
      else
        {
        sprintf(this->InternalFileName, this->FilePattern,this->FileNumber);
        }
      if (this->FileNumber < this->MinimumFileNumber)
        {
        this->MinimumFileNumber = this->FileNumber;
        }
      else if (this->FileNumber > this->MaximumFileNumber)
        {
        this->MaximumFileNumber = this->FileNumber;
        }
      }
    // Open the file
#ifdef _WIN32
    file = new ofstream(this->InternalFileName, ios::out | ios::binary);
#else
    file = new ofstream(this->InternalFileName, ios::out);
#endif
    fileOpenedHere = 1;
    if (file->fail())
      {
      vtkErrorMacro("RecursiveWrite: Could not open file " << 
                    this->InternalFileName);
      this->SetErrorCode(vtkErrorCode::CannotOpenFileError);
      delete file;
      return;
      }

    // Subclasses can write a header with this method call.
    this->WriteFileHeader(file, cache);
    file->flush();
    if (file->fail())
      {
      file->close();
      delete file;
      this->SetErrorCode(vtkErrorCode::OutOfDiskSpaceError);
      return;
      }
    ++this->FileNumber;
    }
  
  // Propagate the update extent so we can determine pipeline size
  this->GetInput()->PropagateUpdateExtent();

  // just get the data and write it out
  ext = cache->GetUpdateExtent();
  vtkDebugMacro("Getting input extent: " << ext[0] << ", " << 
                ext[1] << ", " << ext[2] << ", " << ext[3] << ", " << 
                ext[4] << ", " << ext[5] << endl);
  cache->Update();
  data = cache;
  this->RecursiveWrite(axis,cache,data,file);
  if (this->ErrorCode == vtkErrorCode::OutOfDiskSpaceError)
    {
    this->DeleteFiles();
    return;
    }
  if (file && fileOpenedHere)
    {
    this->WriteFileTrailer(file,cache);
    file->flush();
    if (file->fail())
      {
      this->SetErrorCode(vtkErrorCode::OutOfDiskSpaceError);
      }
    file->close();
    delete file;
    file = NULL;
    }
  return;
}


//----------------------------------------------------------------------------
// same idea as the previous method, but it knows that the data is ready
void vtkImageWriter::RecursiveWrite(int axis, vtkImageData *cache,
                                    vtkImageData *data, ofstream *file)
{
  int idx, min, max;
  
  // if the file is already open then just write to it
  if (file)
    {
    this->WriteFile(file,data,cache->GetUpdateExtent());
    file->flush();
    if (file->fail())
      {
      file->close();
      delete file;
      this->SetErrorCode(vtkErrorCode::OutOfDiskSpaceError);
      }
    return;
    }
  
  // if we need to open another slice, do it
  if (!file && (axis + 1) == this->FileDimensionality)
    {
    // determine the name
    if (this->FileName)
      {
      sprintf(this->InternalFileName,"%s",this->FileName);
      }
    else 
      {
      if (this->FilePrefix)
        {
        sprintf(this->InternalFileName, this->FilePattern, 
                this->FilePrefix, this->FileNumber);
        }
      else
        {
        sprintf(this->InternalFileName, this->FilePattern,this->FileNumber);
        }
      if (this->FileNumber < this->MinimumFileNumber)
        {
        this->MinimumFileNumber = this->FileNumber;
        }
      else if (this->FileNumber > this->MaximumFileNumber)
        {
        this->MaximumFileNumber = this->FileNumber;
        }
      }
    // Open the file
#ifdef _WIN32
    file = new ofstream(this->InternalFileName, ios::out | ios::binary);
#else
    file = new ofstream(this->InternalFileName, ios::out);
#endif
    if (file->fail())
      {
      vtkErrorMacro("RecursiveWrite: Could not open file " << 
                    this->InternalFileName);
      this->SetErrorCode(vtkErrorCode::CannotOpenFileError);
      delete file;
      return;
      }

    // Subclasses can write a header with this method call.
    this->WriteFileHeader(file, cache);
    file->flush();
    if (file->fail())
      {
      file->close();
      delete file;
      this->SetErrorCode(vtkErrorCode::OutOfDiskSpaceError);
      return;
      }
    this->WriteFile(file,data,cache->GetUpdateExtent());
    file->flush();
    if (file->fail())
      {
      file->close();
      delete file;
      this->SetErrorCode(vtkErrorCode::OutOfDiskSpaceError);
      return;
      }
    ++this->FileNumber;
    this->WriteFileTrailer(file,cache);
    file->flush();
    if (file->fail())
      {
      this->SetErrorCode(vtkErrorCode::OutOfDiskSpaceError);
      }
    file->close();
    delete file;
    return;
    }
  
  // if the current region is too high a dimension forthe file
  // the we will split the current axis
  cache->GetAxisUpdateExtent(axis, min, max);
  
  // if it is the y axis then flip by default
  if (axis == 1 && !this->FileLowerLeft)
    {
    for(idx = max; idx >= min; idx--)
      {
      cache->SetAxisUpdateExtent(axis, idx, idx);
      if (this->ErrorCode != vtkErrorCode::OutOfDiskSpaceError)
        {
        this->RecursiveWrite(axis - 1, cache, data, file);
        }
      else
        {
        this->DeleteFiles();
        }
      }
    }
  else
    {
    for(idx = min; idx <= max; idx++)
      {
      cache->SetAxisUpdateExtent(axis, idx, idx);
      if (this->ErrorCode != vtkErrorCode::OutOfDiskSpaceError)
        {
        this->RecursiveWrite(axis - 1, cache, data, file);
        }
      else
        {
        this->DeleteFiles();
        }
      }
    }
  
  // restore original extent
  cache->SetAxisUpdateExtent(axis, min, max);
}
  

//----------------------------------------------------------------------------
// Writes a region in a file.  Subclasses can override this method
// to produce a header. This method only hanldes 3d data (plus components).
void vtkImageWriter::WriteFile(ofstream *file, vtkImageData *data,
                               int extent[6])
{
  int idxY, idxZ;
  int rowLength; // in bytes
  void *ptr;
  unsigned long count = 0;
  unsigned long target;
  float progress = this->Progress;
  float area;
  int *wExtent;
  
  // Make sure we actually have data.
  if ( !data->GetPointData()->GetScalars())
    {
    vtkErrorMacro(<< "Could not get data from input.");
    return;
    }

  // take into consideration the scalar type
  switch (data->GetScalarType())
    {
    case VTK_DOUBLE:
      rowLength = sizeof(double);
      break;
    case VTK_FLOAT:
      rowLength = sizeof(float);
      break;
    case VTK_LONG:
      rowLength = sizeof(long);
      break;
    case VTK_UNSIGNED_LONG:
      rowLength = sizeof(unsigned long); 
      break;
    case VTK_INT:
      rowLength = sizeof(int);
      break;
    case VTK_UNSIGNED_INT:
      rowLength = sizeof(unsigned int); 
      break;
    case VTK_SHORT:
      rowLength = sizeof(short);
      break;
    case VTK_UNSIGNED_SHORT:
      rowLength = sizeof(unsigned short); 
      break;
    case VTK_CHAR:
      rowLength = sizeof(char);
      break;
    case VTK_UNSIGNED_CHAR:
      rowLength = sizeof(unsigned char); 
      break;
    default:
      vtkErrorMacro(<< "Execute: Unknown output ScalarType");
      return; 
    }
  rowLength *= data->GetNumberOfScalarComponents();
  rowLength *= (extent[1] - extent[0] + 1);

  wExtent = this->GetInput()->GetWholeExtent();
  area = (float) ((extent[5] - extent[4] + 1)*
                  (extent[3] - extent[2] + 1)*
                  (extent[1] - extent[0] + 1)) / 
         (float) ((wExtent[5] -wExtent[4] + 1)*
                  (wExtent[3] -wExtent[2] + 1)*
                  (wExtent[1] -wExtent[0] + 1));
    
  target = (unsigned long)((extent[5]-extent[4]+1)*
                           (extent[3]-extent[2]+1)/(50.0*area));
  target++;

  int ystart = extent[3];
  int yend = extent[2] - 1;
  int yinc = -1;
  if (this->FileLowerLeft)
    {
    ystart = extent[2];
    yend = extent[3]+1;
    yinc = 1;
    }
  
  for (idxZ = extent[4]; idxZ <= extent[5]; ++idxZ)
    {
    for (idxY = ystart; idxY != yend; idxY = idxY + yinc)
      {
      if (!(count%target))
        {
        this->UpdateProgress(progress + count/(50.0*target));
        }
      count++;
      ptr = data->GetScalarPointer(extent[0], idxY, idxZ);
      if ( ! file->write((char *)ptr, rowLength))
        {
        return;
        }
      }
    }
}

void vtkImageWriter::DeleteFiles()
{
  if (this->FilesDeleted)
    {
    return;
    }
  int i;
  char *fileName;
  
  vtkErrorMacro("Ran out of disk space; deleting file(s) already written");
  
  if (this->FileName)
    {
    unlink(this->FileName);
    }
  else
    {
    if (this->FilePrefix)
      {
      fileName =
        new char[strlen(this->FilePrefix) + strlen(this->FilePattern) + 10];
      
      for (i = this->MinimumFileNumber; i <= this->MaximumFileNumber; i++)
        {
        sprintf(fileName, this->FilePattern, this->FilePrefix, i);
        unlink(fileName);
        }
      delete [] fileName;
      }
    else
      {
      fileName = new char[strlen(this->FilePattern) + 10];
      
      for (i = this->MinimumFileNumber; i <= this->MaximumFileNumber; i++)
        {
        sprintf(fileName, this->FilePattern, i);
        unlink(fileName);
        }
      delete [] fileName;
      }
    }
  this->FilesDeleted = 1;
}
