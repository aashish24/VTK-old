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
/*-------------------------------------------------------------------------
  Copyright 2008 Sandia Corporation.
  Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
  the U.S. Government retains certain rights in this software.
-------------------------------------------------------------------------*/
// .NAME vtkQtRichTextView - Superclass for QAbstractItemView-based views.
//
// .SECTION Description
// This superclass provides all the plumbing to integrate a QAbstractItemView 
// into the VTK view framework, including reporting selection changes and 
// detecting selection changes from linked views.
//
// .SECTION Thanks
// Thanks to Bob Kerr from Sandia National Laboratories for implementing
// this class

#ifndef __vtkQtRichTextView_h
#define __vtkQtRichTextView_h

#include "QVTKWin32Header.h"
#include "vtkQtView.h"
#include "vtkSmartPointer.h"
#include <QPointer>

class QTextEdit;
class QWebPage;
class QWebView;
class QNetworkProxy;
class QMainWindow;
class QDialog;
class QFrame;
class QPushButton;
class QLayout;


class vtkDataObjectToTable;

class QVTK_EXPORT vtkQtRichTextView : public vtkQtView
{
Q_OBJECT

public:
  static vtkQtRichTextView *New();
  vtkTypeRevisionMacro(vtkQtRichTextView, vtkQtView);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get the main container of this view (a  QWidget).
  // The application typically places the view with a call
  // to GetWidget(): something like this
  // this->ui->box->layout()->addWidget(this->View->GetWidget());
  virtual QWidget* GetWidget();

  //BTX
  enum
    {
    FIELD_DATA = 0,
    POINT_DATA = 1,
    CELL_DATA = 2,
    VERTEX_DATA = 3,
    EDGE_DATA = 4,
    ROW_DATA = 5,
    };
  //ETX
  
  // Description:
  // The field type to copy into the output table.
  // Should be one of FIELD_DATA, POINT_DATA, CELL_DATA, VERTEX_DATA, EDGE_DATA.
  vtkGetMacro(FieldType, int);
  void SetFieldType(int);

  vtkGetMacro(CurrentRow, int);
  vtkGetStringMacro(Text);

  // Description:
  // Updates the view.
  virtual void Update();
  
protected:
  
  vtkQtRichTextView();
  ~vtkQtRichTextView();
    
  // Description:
  // Connects the algorithm output to the internal pipeline.
  // This view only supports a single representation.
  virtual void AddInputConnection(
    vtkAlgorithmOutput* conn,
    vtkAlgorithmOutput* selectionConn);
  
  // Description:
  // Removes the algorithm output from the internal pipeline.
  virtual void RemoveInputConnection(
    vtkAlgorithmOutput* conn,
    vtkAlgorithmOutput* selectionConn);
    
//BTX
  vtkSmartPointer<vtkDataObjectToTable> DataObjectToTable;
//ETX

  QPointer<QPushButton> BackButton;
  QPointer<QWebPage> TextWidgetPage;
  QPointer<QWebView> TextWidgetView;
  QPointer<QFrame> TextWidgetFrame;
  QPointer<QLayout> TextLayout;
  
    

  char* Text;
  int FieldType;
  int CurrentRow;

private:
  vtkQtRichTextView(const vtkQtRichTextView&);  // Not implemented.
  void operator=(const vtkQtRichTextView&);  // Not implemented.

  int find_string(QString &myString,QString &searchString, int prev_loc);
  int insert_string(QString &myString, QString &htmlString, int location);
  
  
};

#endif
