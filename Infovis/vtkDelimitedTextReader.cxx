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

#include "vtkDelimitedTextReader.h"
#include "vtkTable.h"
#include "vtkVariantArray.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkInformation.h"
#include "vtkStringArray.h"
#include "vtkStdString.h"

#include <vtkstd/algorithm>
#include <vtkstd/vector>
#include <vtkstd/string>

vtkCxxRevisionMacro(vtkDelimitedTextReader, "$Revision$");
vtkStandardNewMacro(vtkDelimitedTextReader);

struct vtkDelimitedTextReaderInternals
{
  ifstream *File;
};

// Forward function reference (definition at bottom :)
static int splitString(const vtkStdString& input, 
                       char fieldDelimiter,
                       char stringDelimiter,
                       bool useStringDelimiter,
                       vtkstd::vector<vtkStdString>& results, 
                       bool includeEmpties=true);


vtkDelimitedTextReader::vtkDelimitedTextReader()
{
  this->Internals = new vtkDelimitedTextReaderInternals();

  this->Internals->File = 0;
  this->FileName = 0;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
  this->ReadBuffer = new char[2048];

  this->FieldDelimiter = ',';
  this->StringDelimiter = '"';
  this->UseStringDelimiter = true;

}

vtkDelimitedTextReader::~vtkDelimitedTextReader()
{
  this->SetFileName(0);
  delete this->ReadBuffer;
  delete this->Internals;
}

// ----------------------------------------------------------------------

void vtkDelimitedTextReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " 
     << (this->FileName ? this->FileName : "(none)") << endl;
  os << indent << "Field delimiter: '" << this->FieldDelimiter 
     << "'" << endl;
  os << indent << "String delimiter: '" << this->StringDelimiter
     << "'" << endl;
  os << indent << "UseStringDelimiter: " 
     << (this->UseStringDelimiter ? "true" : "false") << endl;
  os << indent << "HaveHeaders: " 
     << (this->HaveHeaders ? "true" : "false") << endl;
}

// ----------------------------------------------------------------------

void vtkDelimitedTextReader::OpenFile()
{
  // If the file was open close it.
  if (this->Internals->File)
    {
    this->Internals->File->close();
    delete this->Internals->File;
    this->Internals->File = NULL;
    }
  
  // Open the new file.
  vtkDebugMacro(<< "vtkDelimitedTextReader is opening file: " << this->FileName);
  this->Internals->File = new ifstream(this->FileName, ios::in);

  // Check to see if open was successful
  if (! this->Internals->File || this->Internals->File->fail())
    {
    vtkErrorMacro(<< "vtkDelimitedTextReader could not open file " 
                  << this->FileName);
    return;
    }
}

// ----------------------------------------------------------------------

int vtkDelimitedTextReader::RequestData(
                                        vtkInformation*, 
                                        vtkInformationVector**, 
                                        vtkInformationVector* outputVector)
{
  // Check that the filename has been specified
  if (!this->FileName)
    {
    vtkErrorMacro("vtkDelimitedTextReader: You must specify a filename!");
    return 0;
    }
    
  // Open the file
  this->OpenFile();
  
  // Go to the top of the file
  this->Internals->File->seekg(0,ios::beg);

  // Store the text data into a vtkTable
  vtkTable* table = vtkTable::GetData(outputVector);
  
  // The first line of the file might contain the headers, so we want
  // to be a little bit careful about it.  If we don't have headers
  // we'll have to make something up.
  vtkstd::vector<vtkStdString> headers;
  vtkstd::string firstLine;
  vtkstd::vector<vtkStdString> firstLineFields;

  vtkstd::getline(*(this->Internals->File), firstLine);
   
  if (this->HaveHeaders)
    {
    splitString(firstLine,
                this->FieldDelimiter,
                this->StringDelimiter,
                this->UseStringDelimiter,
                headers);
    }
  else
    {
    splitString(firstLine,
                this->FieldDelimiter,
                this->StringDelimiter,
                this->UseStringDelimiter,
                firstLineFields);

    for (unsigned int i = 0; i < firstLineFields.size(); ++i)
      {
      // I know it's not a great idea to use sprintf.  It's safe right
      // here because an unsigned int will never take up enough
      // characters to fill up this buffer.
      char fieldName[64];
      sprintf(fieldName, "Field %d", i);
      headers.push_back(fieldName);
      }
    }

  // Now we can create the arrays that will hold the data for each
  // field.
  vtkstd::vector<vtkStdString>::const_iterator fieldIter;
  for(fieldIter = headers.begin(); fieldIter != headers.end(); ++fieldIter)
    { 
    vtkStringArray* array = vtkStringArray::New();
    array->SetName((*fieldIter).c_str());
    table->AddColumn(array);
    array->Delete();
    }
  
  // If the first line did not contain headers then we need to add it
  // to the table.
  if (!this->HaveHeaders)
    {
    vtkVariantArray* dataArray = vtkVariantArray::New();
    vtkstd::vector<vtkStdString>::const_iterator I;
    for(I = firstLineFields.begin(); I != firstLineFields.end(); ++I)
      {
      dataArray->InsertNextValue(vtkVariant(*I));
      }
    
    // Insert the data into the table
    table->InsertNextRow(dataArray);
    dataArray->Delete();
    }

  // Okay read the file and add the data to the table
  vtkstd::string nextLine;
  while (vtkstd::getline(*(this->Internals->File), nextLine))
    {
    vtkstd::vector<vtkStdString> dataVector;

    // Split string on the delimiters
    splitString(nextLine,
                this->FieldDelimiter,
                this->StringDelimiter,
                this->UseStringDelimiter,
                dataVector);
    
    // Add data to the output arrays

    // Convert from vector to variant array
    vtkVariantArray* dataArray = vtkVariantArray::New();
    vtkstd::vector<vtkStdString>::const_iterator I;
    for(I = dataVector.begin(); I != dataVector.end(); ++I)
      {
      dataArray->InsertNextValue(vtkVariant(*I));
      }
    
    // Insert the data into the table
    table->InsertNextRow(dataArray);
    dataArray->Delete();
    }
 
  return 1;
}

// ----------------------------------------------------------------------

static int 
splitString(const vtkStdString& input, 
            char fieldDelimiter,
            char stringDelimiter,
            bool useStringDelimiter,
            vtkstd::vector<vtkStdString>& results, 
            bool includeEmpties)
{
  if (input.size() == 0)
    {
    return 0;
    }

  bool inString = false;
  char thisCharacter = 0;

  vtkstd::string currentField;

  for (unsigned int i = 0; i < input.size(); ++i)
    {
    thisCharacter = input[i];

    // First: handle string beginning/end (if the user specified string
    // delimiters)
    if (useStringDelimiter && thisCharacter == stringDelimiter)
      {
      // this should just toggle inString
      inString = (inString == false);
      }
    else if (thisCharacter == fieldDelimiter)
      {
      // Second: handle field delimiters.  A delimiter starts a new
      // field unless we're in a string, in which case it's normal text.
      if (inString)
        {
        currentField += thisCharacter;
        }
      else
        {
        if (includeEmpties || currentField.size() > 0)
          {
          results.push_back(currentField);
          }
        currentField.clear();
        }
      }
    else
      {
      // The character is just plain text.  Accumulate it and move on.
      currentField += thisCharacter;
      }
    }
  
  // handle the string accumulated since the last delimiter, if any
  if (includeEmpties || currentField.size() > 0)
    {
    results.push_back(currentField);
    }
  
  return results.size();
}

