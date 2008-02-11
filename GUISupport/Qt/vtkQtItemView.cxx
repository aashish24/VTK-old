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

#include "vtkQtItemView.h"
#include <QObject>
#include <QAbstractItemView>
#include <QAbstractItemModel>
#include <QItemSelectionModel>

#include "vtkAlgorithm.h"
#include "vtkAlgorithmOutput.h"
#include "vtkCommand.h"
#include "vtkDataRepresentation.h"
#include "vtkDataSet.h"
#include "vtkFieldData.h"
#include "vtkIdTypeArray.h"
#include "vtkInformation.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkSelection.h"
#include "vtkSelectionLink.h"
#include "vtkVariant.h"

vtkCxxRevisionMacro(vtkQtItemView, "$Revision$");
vtkStandardNewMacro(vtkQtItemView);


// Signal helper class
void vtkQtSignalHandler::slotSelectionChanged(const QItemSelection& s1, const QItemSelection& s2)
{
  this->Target->QtSelectionChanged(s1, s2);
}

//----------------------------------------------------------------------------
vtkQtItemView::vtkQtItemView()
{
  // Set my view and model adapter to NULL
  this->ItemViewPtr = 0;
  this->ModelAdapterPtr = 0;
  this->SelectionModel = 0;
  
  // By defualt use indices in the selection
  this->UseValueSelection = 0;
  this->ValueSelectionArrayName = 0;
  
  // Initialize selecting to false
  this->Selecting = false;

  this->IOwnSelectionModel = false;

  // Funny little hook to get around multiple inheritance
  this->SignalHandler.setTarget(this);
}

//----------------------------------------------------------------------------
vtkQtItemView::~vtkQtItemView()
{
  //if(this->IOwnSelectionModel && this->SelectionModel)
  //  {
  //  delete this->SelectionModel;
  //  this->SelectionModel = 0;
  //  }
}

// Description:
// Just a convenience function for making sure
// that the view and model pointers are valid
int vtkQtItemView::CheckViewAndModelError()
{
  // Sub-classes might use their own views, so don't insist that a view has been set

  //if (this->ItemViewPtr == 0)
  //  {
  //  vtkErrorMacro("Trying to use vtkQtItemView with in invalid View");
  //  return 1;
  //  }
  if (this->ModelAdapterPtr == 0)
    {
    vtkErrorMacro("Trying to use vtkQtItemView with in invalid ModelAdapter");
    return 1;
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkQtItemView::SetItemView(QAbstractItemView *qiv)
{
  // Set up my internals to point to the new view
  this->ItemViewPtr = qiv;
}

//----------------------------------------------------------------------------
QAbstractItemView* vtkQtItemView::GetItemView()
{
  return this->ItemViewPtr;
}

//----------------------------------------------------------------------------
void vtkQtItemView::SetItemModelAdapter(vtkQtAbstractModelAdapter* qma)
{
  // Set up my internals to point to the new view
  this->ModelAdapterPtr = qma;

  if(this->SelectionModel)
    {
    delete this->SelectionModel;
    this->SelectionModel = 0;
    this->IOwnSelectionModel = false;
    }
}

//----------------------------------------------------------------------------
vtkQtAbstractModelAdapter* vtkQtItemView::GetItemModelAdapter()
{
  return this->ModelAdapterPtr;
}


//----------------------------------------------------------------------------
QItemSelectionModel* vtkQtItemView::GetSelectionModel()
{
  // If a view has been set, use its selection model
  if(this->ItemViewPtr)
    {
    return this->ItemViewPtr->selectionModel();
    }

  // Otherwise, create one of our own (if we haven't already done so)
  // using the item model.
  if(!this->SelectionModel)
    {
    this->SelectionModel = new QItemSelectionModel(this->ModelAdapterPtr);
    this->IOwnSelectionModel = true;
    }

  return this->SelectionModel;
}

//----------------------------------------------------------------------------
void vtkQtItemView::AddInputConnection(vtkAlgorithmOutput* conn)
{
  // Make sure I have a view and a model
  if (CheckViewAndModelError()) return;
    
  // Hand VTK data off to the adapter
  conn->GetProducer()->Update();
  vtkDataObject *d = conn->GetProducer()->GetOutputDataObject(0);
    
  this->ModelAdapterPtr->SetVTKDataObject(d);
  
  // Sub-classes might use their own views, so don't assume the view has been set
  if(this->ItemViewPtr)
    {
    this->ItemViewPtr->setModel(this->ModelAdapterPtr);
    this->ItemViewPtr->update();

    // Setup selction links
    this->ItemViewPtr->setSelectionMode(QAbstractItemView::ExtendedSelection);
    }

  QObject::connect(this->GetSelectionModel(), SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)),
      &this->SignalHandler, SLOT(slotSelectionChanged(const QItemSelection&,const QItemSelection&)));
}

//----------------------------------------------------------------------------
void vtkQtItemView::RemoveInputConnection(vtkAlgorithmOutput* conn)
{
  // Make sure I have a view and a model
  if (CheckViewAndModelError()) return;
  
  // Remove VTK data from the adapter
  conn->GetProducer()->Update();
  vtkDataObject *d = conn->GetProducer()->GetOutputDataObject(0);
  if (this->ModelAdapterPtr->GetVTKDataObject() == d)
    {
    this->ModelAdapterPtr->SetVTKDataObject(0);
    // Sub-classes might use their own views, so don't assume the view has been set
    if(this->ItemViewPtr)
      {
      this->ItemViewPtr->update();
      }
    }
}

//----------------------------------------------------------------------------
void vtkQtItemView::QtSelectionChanged(const QItemSelection&, const QItemSelection&)
{
  // Make sure I have a view and a model
  if (CheckViewAndModelError()) return;
  
  this->Selecting = true;
  vtkSelection* selection = vtkSelection::New();
  
  // Do they want the selection to be indices or values?
  if (this->UseValueSelection && (this->ValueSelectionArrayName != NULL))
    {
    selection->GetProperties()->Set(vtkSelection::CONTENT_TYPE(), vtkSelection::VALUES);
    vtkDataObject *d = this->ModelAdapterPtr->GetVTKDataObject();
    vtkAbstractArray *array = 0;
    if (vtkDataSet::SafeDownCast(d))
      {
      selection->GetProperties()->Set(vtkSelection::FIELD_TYPE(), vtkSelection::POINT);
      array = (vtkDataSet::SafeDownCast(d))->GetPointData()->GetAbstractArray(this->ValueSelectionArrayName);
      }
    else
      {
      selection->GetProperties()->Set(vtkSelection::FIELD_TYPE(), vtkSelection::FIELD);
      array = d->GetFieldData()->GetAbstractArray(this->ValueSelectionArrayName);
      }
                           
    // Does the array exist?
    if (array != 0)
      {     
      // Create a new array of the same type
      vtkAbstractArray *sel_array = vtkAbstractArray::CreateArray(array->GetDataType());
      sel_array->SetName(this->ValueSelectionArrayName);
      selection->SetSelectionList(sel_array);
      sel_array->Delete();
      const QModelIndexList list = this->GetSelectionModel()->selectedRows();
      for (int i = 0; i < list.size(); i++)
        {
        vtkIdType pid = this->ModelAdapterPtr->QModelIndexToPedigree(list.at(i));
        sel_array->InsertNextTuple(this->ModelAdapterPtr->PedigreeToId(pid), array);
        }
      }
    else
      {
      vtkErrorMacro("Couldn't find array: " << this->ValueSelectionArrayName);
      }
    }
    
  // Index selection
  else
    {
    selection->GetProperties()->Set(vtkSelection::CONTENT_TYPE(), vtkSelection::INDICES);
    vtkIdTypeArray* idarr = vtkIdTypeArray::New();
    selection->SetSelectionList(idarr);
    idarr->Delete();
    const QModelIndexList list = this->GetSelectionModel()->selectedRows();
    
    // For index selection do this odd little dance with two maps :)
    for (int i = 0; i < list.size(); i++)
      {
      vtkIdType pid = this->ModelAdapterPtr->QModelIndexToPedigree(list.at(i));
      idarr->InsertNextValue(this->ModelAdapterPtr->PedigreeToId(pid));
      }  
    }
   
  // Call select on the representation
  this->GetRepresentation()->Select(this, selection);
  
  // Invoke selection changed event
  selection->Delete();
  this->Selecting = false;
}

//----------------------------------------------------------------------------
void vtkQtItemView::ProcessEvents(
  vtkObject* caller, 
  unsigned long eventId, 
  void* callData)
{
  Superclass::ProcessEvents(caller, eventId, callData);
}

//----------------------------------------------------------------------------
void vtkQtItemView::Update()
{
  // Make sure I have a view and a model
  if (CheckViewAndModelError()) return;
  
  vtkDataRepresentation* rep = this->GetRepresentation();
  if (!rep)
    {
    return;
    }
  
  // Make the selection current
  if (this->Selecting)
    {
    // If we initiated the selection, do nothing.
    return;
    }
  vtkSelection* selection = rep->GetSelectionLink()->GetSelection();
  QItemSelection list;
  if (selection->GetProperties()->Get(vtkSelection::CONTENT_TYPE()) == vtkSelection::INDICES)
    {
    vtkIdTypeArray* arr = vtkIdTypeArray::SafeDownCast(selection->GetSelectionList());
    if (!arr)
      {
      vtkErrorMacro("INDICES selection should have idtype array.");
      return;
      }
    for (vtkIdType i = 0; i < arr->GetNumberOfTuples(); i++)
      {
      vtkIdType id = arr->GetValue(i);
      QModelIndex index = 
        this->ModelAdapterPtr->PedigreeToQModelIndex(
        this->ModelAdapterPtr->IdToPedigree(id));
      list.select(index, index);
      }
    }
  else
    {
    vtkWarningMacro("vtkQtItemView cannot currently 'consume' a selection type of: " 
      << selection->GetProperties()->Get(vtkSelection::CONTENT_TYPE()));
    return;
    }
  this->GetSelectionModel()->select(list, 
    QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);  
  
  // Make the data current
  vtkAlgorithm* alg = rep->GetInputConnection()->GetProducer();
  alg->Update();
  vtkDataObject *d = alg->GetOutputDataObject(0);
  this->ModelAdapterPtr->SetVTKDataObject(d);
  // Sub-classes might use their own views, so don't assume the view has been set
  if(this->ItemViewPtr)
    {
    this->ItemViewPtr->update();
    }
}

//----------------------------------------------------------------------------
void vtkQtItemView::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

