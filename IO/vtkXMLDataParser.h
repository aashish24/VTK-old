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
// .NAME vtkXMLDataParser - Used by vtkXMLReader to parse VTK XML files.
// .SECTION Description
// vtkXMLDataParser provides a subclass of vtkXMLParser that
// constructs a representation of an XML data format's file using
// vtkXMLDataElement to represent each XML element.  This
// representation is then used by vtkXMLReader and its subclasses to
// traverse the structure of the file and extract data.

// .SECTION See Also
// vtkXMLDataElement

#ifndef __vtkXMLDataParser_h
#define __vtkXMLDataParser_h

#include "vtkXMLParser.h"

class vtkXMLDataElement;
class vtkInputStream;
class vtkDataCompressor;

class VTK_IO_EXPORT vtkXMLDataParser : public vtkXMLParser
{
public:
  vtkTypeRevisionMacro(vtkXMLDataParser,vtkXMLParser);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkXMLDataParser* New();
  
  //BTX
  // Description:
  // Enumerate big and little endian byte order settings.
  enum { BigEndian, LittleEndian };
  //ETX
  
  // Description:
  // Get the root element from the XML document.
  vtkXMLDataElement* GetRootElement();
  
  // Description:
  // Read inline data from inside the given element.  Returns the
  // number of words read.
  unsigned long ReadInlineData(vtkXMLDataElement* element, int isAscii,
                               void* buffer, int startWord, int numWords,
                               int wordType);
  unsigned long ReadInlineData(vtkXMLDataElement* element, int isAscii,
                               char* buffer, int startWord, int numWords)
    { return this->ReadInlineData(element, isAscii, buffer, startWord,
                                  numWords, VTK_CHAR); }
  
  // Description:
  // Read from an appended data section starting at the given appended
  // data offset.  Returns the number of words read.
  unsigned long ReadAppendedData(unsigned long offset, void* buffer,
                                 int startWord, int numWords, int wordType);
  unsigned long ReadAppendedData(unsigned long offset, char* buffer,
                                 int startWord, int numWords)
    { return this->ReadAppendedData(offset, buffer, startWord, numWords,
                                    VTK_CHAR); }
  
  // Description:
  // Read from an ascii data section starting at the current position in
  // the stream.  Returns the number of words read.
  unsigned long ReadAsciiData(void* buffer, int startWord, int numWords,
                              int wordType);
  
  // Description:
  // Read from a data section starting at the current position in the
  // stream.  Returns the number of words read.
  unsigned long ReadBinaryData(void* buffer, int startWord, int maxWords,
                               int wordType);
  
  // Description:
  // Get/Set the compressor used to decompress binary and appended data
  // after reading from the file.
  virtual void SetCompressor(vtkDataCompressor*);
  vtkGetObjectMacro(Compressor, vtkDataCompressor);
  
  // Description:
  // Get the size of a word of the given type.
  unsigned long GetWordTypeSize(int wordType);

  // Description:
  // Parse the XML input and check that the file is safe to read.
  // Returns 1 for okay, 0 for error.
  virtual int Parse();

protected:
  vtkXMLDataParser();
  ~vtkXMLDataParser();
  
  // This parser does not support parsing from a string.
  virtual int Parse(const char*);
  virtual int Parse(const char*, unsigned int);
  
  // Implement parsing methods.
  void StartElement(const char* name, const char** atts);
  void EndElement(const char*);
  int ParsingComplete();
  void ClearStreamEOF();
  int CheckPrimaryAttributes();
  void FindAppendedDataPosition();
  unsigned long FindInlineDataPosition(unsigned long start);
  int ParseBuffer(const char* buffer, unsigned int count);
  
  void AddElement(vtkXMLDataElement* element);
  void PushOpenElement(vtkXMLDataElement* element);
  vtkXMLDataElement* PopOpenElement();
  void PerformByteSwap(void* data, int numWords, int wordSize);
  
  // Data reading methods.
  void ReadCompressionHeader();
  unsigned int FindBlockSize(unsigned int block);
  int ReadBlock(unsigned int block, unsigned char* buffer);
  unsigned char* ReadBlock(unsigned int block);
  unsigned long ReadUncompressedData(unsigned char* data, unsigned long offset,
                                     unsigned long length);
  unsigned long ReadCompressedData(unsigned char* data, unsigned long offset,
                                   unsigned long length);
  
  // Ascii data reading methods.
  int ParseAsciiData(int wordType);
  void FreeAsciiBuffer();
    
  // The root XML element.
  vtkXMLDataElement* RootElement;
  
  // The stack of elements currently being parsed.
  vtkXMLDataElement** OpenElements;
  unsigned int NumberOfOpenElements;
  unsigned int OpenElementsSize;
  
  // The position of the appended data section, if found.
  unsigned long AppendedDataPosition;
  
  // How much of the string "<AppendedData" has been matched in input.
  int AppendedDataMatched;
  
  // The byte order of the binary input.
  int ByteOrder;
  
  // The input stream used to read data.  Set by ReadAppendedData and
  // ReadInlineData methods.
  vtkInputStream* DataStream;
  
  // The input stream used to read inline data.  May transparently
  // decode the data.
  vtkInputStream* InlineDataStream;
  
  // The stream to use for appended data.
  vtkInputStream* AppendedDataStream;
  
  // Decompression data.
  vtkDataCompressor* Compressor;
  unsigned int NumberOfBlocks;
  unsigned int BlockUncompressedSize;
  unsigned int PartialLastBlockUncompressedSize;
  unsigned int* BlockCompressedSizes;
  unsigned long* BlockStartOffsets;
  
  // Ascii data parsing.
  unsigned char* AsciiDataBuffer;
  int AsciiDataBufferLength;
  int AsciiDataWordType;
  unsigned long AsciiDataPosition;
  
private:
  vtkXMLDataParser(const vtkXMLDataParser&);  // Not implemented.
  void operator=(const vtkXMLDataParser&);  // Not implemented.
};

#endif
