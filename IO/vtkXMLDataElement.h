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
// .NAME vtkXMLDataElement - Represents an XML element and those nested inside.
// .SECTION Description
// vtkXMLDataElement is used by vtkXMLDataParser to represent an XML
// element.  It provides methods to access the element's attributes
// and nested elements in a convenient manner.  This allows easy
// traversal of an input XML file by vtkXMLReader and its subclasses.

// .SECTION See Also
// vtkXMLDataParser

#ifndef __vtkXMLDataElement_h
#define __vtkXMLDataElement_h

#include "vtkObject.h"

class vtkXMLDataParser;

class VTK_IO_EXPORT vtkXMLDataElement : public vtkObject
{
public:
  vtkTypeRevisionMacro(vtkXMLDataElement,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  static vtkXMLDataElement* New();
  
  // Description:
  // Set/Get the name of the element.  This is its XML tag.
  vtkGetStringMacro(Name);
  vtkSetStringMacro(Name);
  
  // Description:
  // Set/Get the value of the id attribute of the element, if any.
  vtkGetStringMacro(Id);
  vtkSetStringMacro(Id);
  
  // Description:
  // Get the attribute with the given name.  If it doesn't exist,
  // returns 0.
  const char* GetAttribute(const char* name);

  // Description:
  // Set the attribute with the given name and value. If it doesn't exist,
  // adds it.
  void SetAttribute(const char* name, const char* value);
  
  // Description:
  // Get the attribute with the given name and converted to a scalar
  // value.  Returns whether value was extracted.
  int GetScalarAttribute(const char* name, int& value);
  int GetScalarAttribute(const char* name, float& value);
  int GetScalarAttribute(const char* name, double& value);
  int GetScalarAttribute(const char* name, unsigned long& value);
#ifdef VTK_ID_TYPE_IS_NOT_BASIC_TYPE
  int GetScalarAttribute(const char* name, vtkIdType& value);
#endif

  // Description:
  // Set the attribute with the given name.
  void SetScalarAttribute(const char* name, int value);
  void SetScalarAttribute(const char* name, float value);
  void SetScalarAttribute(const char* name, double value);
  void SetScalarAttribute(const char* name, unsigned long value);
#ifdef VTK_ID_TYPE_IS_NOT_BASIC_TYPE
  void SetScalarAttribute(const char* name, vtkIdType value);
#endif
  
  // Description:
  // Get the attribute with the given name and converted to a scalar
  // value.  Returns length of vector read.
  int GetVectorAttribute(const char* name, int length, int* value);
  int GetVectorAttribute(const char* name, int length, float* value);
  int GetVectorAttribute(const char* name, int length, double* value);
  int GetVectorAttribute(const char* name, int length, unsigned long* value);
#ifdef VTK_ID_TYPE_IS_NOT_BASIC_TYPE
  int GetVectorAttribute(const char* name, int length, vtkIdType* value);
#endif
  
  // Description:
  // Set the attribute with the given name.
  void SetVectorAttribute(const char* name, int length, int* value);
  void SetVectorAttribute(const char* name, int length, float* value);
  void SetVectorAttribute(const char* name, int length, double* value);
  void SetVectorAttribute(const char* name, int length, unsigned long* value);
#ifdef VTK_ID_TYPE_IS_NOT_BASIC_TYPE
  void SetVectorAttribute(const char* name, int length, vtkIdType* value);
#endif
  
  // Description:
  // Get the attribute with the given name and converted to a word type
  // such as VTK_FLOAT or VTK_UNSIGNED_LONG.
  int GetWordTypeAttribute(const char* name, int& value);
  
  // Description:
  // Get the number of attributes.
  vtkGetMacro(NumberOfAttributes, int);

  // Description:
  // Get the n-th attribute name.
  // Returns 0 if there is no such attribute.
  const char* GetAttributeName(int idx);

  // Description:
  // Set/Get the parent of this element.
  vtkXMLDataElement* GetParent();
  void SetParent(vtkXMLDataElement* parent);
  
  // Description:
  // Get the number of elements nested in this one.
  int GetNumberOfNestedElements();
  
  // Description:
  // Get the element nested in this one at the given index.
  vtkXMLDataElement* GetNestedElement(int index);
  
  // Description:
  // Add nested element
  void AddNestedElement(vtkXMLDataElement* element);

  // Description:
  // Find a nested element with the given id, given name, or given name and id.
  vtkXMLDataElement* FindNestedElement(const char* id);
  vtkXMLDataElement* FindNestedElementWithName(const char* name);
  vtkXMLDataElement* FindNestedElementWithNameAndId(const char* name, 
                                                    const char* id);
  
  // Description:
  // Lookup the element with the given id, starting at this scope.
  vtkXMLDataElement* LookupElement(const char* id);
  
  // Description:
  // Set/Get the offset from the beginning of the XML document to this element.
  vtkGetMacro(XMLByteIndex, unsigned long);
  vtkSetMacro(XMLByteIndex, unsigned long);
  
protected:
  vtkXMLDataElement();
  ~vtkXMLDataElement();  
  
  // The name of the element from the XML file.
  char* Name;
  
  // The value of the "id" attribute, if any was given.
  char* Id;
  
  // The offset into the XML stream where the element begins.
  unsigned long XMLByteIndex;
  
  // The offset into the XML stream where the inline data begins.
  unsigned long InlineDataPosition;
  
  // The raw property name/value pairs read from the XML attributes.
  char** AttributeNames;
  char** AttributeValues;
  int NumberOfAttributes;
  int AttributesSize;
  
  // The set of nested elements.
  int NumberOfNestedElements;
  int NestedElementsSize;
  vtkXMLDataElement** NestedElements;
  
  // The parent of this element.
  vtkXMLDataElement* Parent;
  
  // Method used by vtkXMLFileParser to setup the element.
  void ReadXMLAttributes(const char** atts);  
  void SeekInlineDataPosition(vtkXMLDataParser* parser);
  
  void PrintXML(ostream& os, vtkIndent indent);
  
  // Internal utility methods.
  vtkXMLDataElement* LookupElementInScope(const char* id);
  vtkXMLDataElement* LookupElementUpScope(const char* id);
  static int IsSpace(char c);
  
  //BTX
  friend class vtkXMLDataParser;
  //ETX
  
private:
  vtkXMLDataElement(const vtkXMLDataElement&);  // Not implemented.
  void operator=(const vtkXMLDataElement&);  // Not implemented.
};

#endif
