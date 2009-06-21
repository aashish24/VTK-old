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

#include "vtkAlgorithm.h"
#include "vtkAlgorithmOutput.h"
#include "vtkAnnotationLink.h"
#include "vtkCommand.h"
#include "vtkConvertSelection.h"
#include "vtkDataObjectToTable.h"
#include "vtkDataRepresentation.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkQtRichTextView.h"
#include "vtkSelection.h"
#include "vtkSelectionNode.h"
#include "vtkSelectionSource.h"
#include "vtkSmartPointer.h"
#include "vtkTable.h"
#include "vtkVariantArray.h"

#include <ui_vtkQtRichTextView.h>

#include <QFrame>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QObject>
#include <QPointer>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWebFrame>
#include <QWebHistory>
#include <QWebView>

vtkCxxRevisionMacro(vtkQtRichTextView, "$Revision$");
vtkStandardNewMacro(vtkQtRichTextView);

/////////////////////////////////////////////////////////////////////////////
// vtkQtRichTextView::Implementation

class vtkQtRichTextView::Implementation
{
public:
  ~Implementation()
  {
    delete this->Widget;
  }

  // Handles conversion of our input data to a table for display
  vtkSmartPointer<vtkDataObjectToTable> DataObjectToTable;

  // Caches displayed content so we can navigate backwards to it
  vtkStdString Content;

  QPointer<QWidget> Widget;
  Ui::vtkQtRichTextView UI;
};

/////////////////////////////////////////////////////////////////////////////
// vtkQtRichTextView

vtkQtRichTextView::vtkQtRichTextView()
{
  this->ProxyPort = 0;
  this->ProxyURL = NULL;
  this->Internal = new Implementation();
  this->Internal->DataObjectToTable = vtkSmartPointer<vtkDataObjectToTable>::New();
  this->Internal->DataObjectToTable->SetFieldType(ROW_DATA);

  this->Internal->Widget = new QWidget();
  this->Internal->UI.setupUi(this->Internal->Widget);
  this->Internal->UI.WebView->setHtml("");
  QNetworkProxy proxy(QNetworkProxy::HttpCachingProxy,"wwwproxy.sandia.gov",80);
  QNetworkProxy::setApplicationProxy(proxy);

  QObject::connect(this->Internal->UI.BackButton, SIGNAL(clicked()), this, SLOT(onBack()));
  QObject::connect(this->Internal->UI.WebView, SIGNAL(loadProgress(int)), this, SLOT(onLoadProgress(int)));
}

vtkQtRichTextView::~vtkQtRichTextView()
{
  delete this->Internal;
}

void vtkQtRichTextView::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

QWidget* vtkQtRichTextView::GetWidget()
{
  return this->Internal->Widget;
}

void vtkQtRichTextView::SetFieldType(int type)
{
  this->Internal->DataObjectToTable->SetFieldType(type);
  this->Update();
}

int vtkQtRichTextView::GetFieldType()
{
  return this->Internal->DataObjectToTable->GetFieldType();
}

void vtkQtRichTextView::Update()
{
  // Make sure the input connection is up to date ...
  vtkDataRepresentation* const representation = this->GetRepresentation();
  if(!representation)
    {
    this->Internal->UI.WebView->setHtml("");
    return;
    }
  representation->Update();

  if(this->Internal->DataObjectToTable->GetTotalNumberOfInputConnections() == 0
      || this->Internal->DataObjectToTable->GetInputConnection(0, 0) != representation->GetInternalOutputPort(0))
    {
    this->Internal->DataObjectToTable->SetInputConnection(0, representation->GetInternalOutputPort(0));
    }
  this->Internal->DataObjectToTable->Update();

  // Get our input table ...
  vtkTable* const table = this->Internal->DataObjectToTable->GetOutput();
  if(!table)
    {
    this->Internal->UI.WebView->setHtml("");
    return;
    }

  vtkIdType row = 0;
  bool row_valid = false;

  // Special-case: if the table is empty, we're done ...
  if(0 == table->GetNumberOfRows())
    {
    this->Internal->UI.WebView->setHtml("");
    return;
    }
  // Special-case #2: If the table has only one row just use it.
  else if( 1 == table->GetNumberOfRows())
    {
      row = 0;
      row_valid=true;
    }
  // For everything else, use the SelectionLink.
  else
    {
    // Figure-out which row of the table we're going to display (if any) ...
    row_valid = false;
    if(vtkSelection* const selection = representation->GetAnnotationLink()->GetCurrentSelection())
      {
      if(vtkSelectionNode* const selection_node = selection->GetNumberOfNodes() ? selection->GetNode(0) : 0)
        {
        if(vtkIdTypeArray* const selection_array = vtkIdTypeArray::SafeDownCast(selection_node->GetSelectionList()))
          {
          if(selection_array->GetNumberOfTuples() && selection_array->GetValue(0) >= 0 && selection_array->GetValue(0) < table->GetNumberOfRows())
            {
            row = selection_array->GetValue(0);
            row_valid = true;
            }
          }
        }
      }
    }

  this->Internal->UI.WebView->history()->clear(); // Workaround for a quirk in QWebHistory

  if(row_valid)
    {
    this->Internal->Content = table->GetValueByName(row, "html").ToString();
    }
  else
    {
    this->Internal->Content.clear();
    }

  //cerr << this->Internal->Content << endl;
  this->Internal->UI.WebView->setHtml(this->Internal->Content.c_str());
}

void vtkQtRichTextView::onBack()
{
  // This logic is a workaround for a quirk in QWebHistory
  if(this->Internal->UI.WebView->history()->currentItemIndex() <= 1)
    {
    this->Internal->UI.WebView->setHtml(this->Internal->Content.c_str());
    this->Internal->UI.WebView->history()->clear();
    }
  else
    {
    this->Internal->UI.WebView->back();
    }
}

void vtkQtRichTextView::onLoadProgress(int progress)
{
  ViewProgressEventCallData callData("Web Page Loading", progress/100.0);
  this->InvokeEvent(vtkCommand::ViewProgressEvent, &callData);
}
