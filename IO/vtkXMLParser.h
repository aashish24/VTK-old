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
// .NAME vtkXMLParser - Parse XML to handle element tags and attributes.
// .SECTION Description
// vtkXMLParser reads a stream and parses XML element tags and corresponding
// attributes.  Each element begin tag and its attributes are sent to
// the StartElement method.  Each element end tag is sent to the
// EndElement method.  Subclasses should replace these methods to actually
// use the tags.

#ifndef __vtkXMLParser_h
#define __vtkXMLParser_h

#include "vtkObject.h"

class VTK_IO_EXPORT vtkXMLParser : public vtkObject
{
public:
  vtkTypeRevisionMacro(vtkXMLParser,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkXMLParser* New();
  
  //BTX
  // Description:
  // Get/Set the input stream.
  vtkSetMacro(Stream, istream*);
  vtkGetMacro(Stream, istream*);
  //ETX
  
  // Description:
  // Parse the input stream.
  int Parse();
  
protected:
  vtkXMLParser();
  ~vtkXMLParser();
  
  // Input stream.  Set by user.
  istream* Stream;
  
  // Expat parser structure.  Exists only during call to Parse().
  void* Parser;
  
  // Called by Parse() to read the stream and call ParseBuffer.  Can
  // be replaced by subclasses to change how the stream is read.
  virtual int ParseStream();
  
  // Called before each block of input is read from the stream to
  // check if parsing is complete.  Can be replaced by subclasses to
  // change the terminating condition for parsing.  Parsing always
  // stops when the end of file is reached in the stream.
  virtual int ParsingComplete();
  
  // Called when a new element is opened in the XML source.  Should be
  // replaced by subclasses to handle each element.
  //  name = Name of new element.
  //  atts = Null-terminated array of attribute name/value pairs.
  //         Even indices are attribute names, and odd indices are values.
  virtual void StartElement(const char* name, const char** atts);
  
  // Called at the end of an element in the XML source opened when
  // StartElement was called.
  virtual void EndElement(const char* name);
  
  // Called when there is character data to handle.
  virtual void CharacterDataHandler(const char* data, int length);  
  
  // Called by begin handlers to report any stray attribute values.
  virtual void ReportStrayAttribute(const char* element, const char* attr,
                                    const char* value);
  
  // Called by begin handlers to report any missing attribute values.
  virtual void ReportMissingAttribute(const char* element, const char* attr);
  
  // Called by begin handlers to report bad attribute values.
  virtual void ReportBadAttribute(const char* element, const char* attr,
                                  const char* value);
  
  // Called by StartElement to report unknown element type.
  virtual void ReportUnknownElement(const char* element);
  
  // Called by Parse to report an XML syntax error.
  virtual void ReportXmlParseError();  
  
  // Get the current byte index from the beginning of the XML stream.
  unsigned long GetXMLByteIndex();
  
  // Send the given buffer to the XML parser.
  int ParseBuffer(const char* buffer, unsigned int count);
  
  // Send the given c-style string to the XML parser.
  int ParseBuffer(const char* buffer);
  
  // Utility for convenience of subclasses.  Wraps isspace C library
  // routine.
  static int IsSpace(char c);  
  
  // Begin element handler that is registered with the XML_Parser.
  // This just casts the user data to a vtkXMLParser and calls
  // StartElement.
  static void StartElementFunction(void* parser, const char* name,
                                   const char** atts);
  
  // End element handler that is registered with the XML_Parser.  This
  // just casts the user data to a vtkXMLParser and calls EndElement.
  static void EndElementFunction(void* parser, const char* name);

  // Character data handler that is registered with the XML_Parser.
  // This just casts the user data to a vtkXMLParser and calls
  // CharacterDataHandler.
  static void CharacterDataHandlerFunction(void* parser, const char* data,
                                           int length);
private:
  vtkXMLParser(const vtkXMLParser&);  // Not implemented.
  void operator=(const vtkXMLParser&);  // Not implemented.
};

#endif
