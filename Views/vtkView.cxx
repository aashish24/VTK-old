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

#include "vtkView.h"

#include "vtkAlgorithmOutput.h"
#include "vtkCollection.h"
#include "vtkCommand.h"
#include "vtkDataObject.h"
#include "vtkDataRepresentation.h"
#include "vtkObjectFactory.h"
#include "vtkSelection.h"
#include "vtkStringArray.h"
#include "vtkViewTheme.h"
#include "vtkSmartPointer.h"

#include <vtkstd/map>
#include <vtkstd/string>
#include <vtkstd/vector>

#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//----------------------------------------------------------------------------
class vtkView::Command : public vtkCommand
{
public:
  static Command* New() {  return new Command(); }
  virtual void Execute(vtkObject *caller, unsigned long eventId,
                       void *callData)
    {
    if (this->Target)
      {
      this->Target->ProcessEvents(caller, eventId, callData);
      }
    }
  void SetTarget(vtkView* t)
    {
    this->Target = t;
    }
private:
  Command() { this->Target = 0; }
  vtkView* Target;
};

//----------------------------------------------------------------------------
class vtkView::vtkInternal
{
public:
  vtkstd::map<vtkObject*, vtkstd::string> RegisteredProgress;
};

//----------------------------------------------------------------------------
class vtkView::vtkImplementation
{
public:
  vtkstd::vector<vtkstd::vector<vtkSmartPointer<vtkDataRepresentation> > > Ports;
};
  

vtkCxxRevisionMacro(vtkView, "$Revision$");
vtkStandardNewMacro(vtkView);
vtkCxxSetObjectMacro(vtkView, SelectionArrayNames, vtkStringArray);
//----------------------------------------------------------------------------
vtkView::vtkView()
{
  this->Internal = new vtkView::vtkInternal();
  this->Implementation = new vtkView::vtkImplementation();
  this->Observer = vtkView::Command::New();
  this->Observer->SetTarget(this);
  this->SelectionArrayNames = vtkStringArray::New();
  this->SelectionType = vtkSelection::INDICES;
  
  // Apply default theme
  vtkViewTheme* theme = vtkViewTheme::New();
  this->ApplyViewTheme(theme);
  theme->Delete();  
}

//----------------------------------------------------------------------------
vtkView::~vtkView()
{
//  this->Representations->Delete();
  this->RemoveAllRepresentations();

  this->Observer->SetTarget(0);
  this->Observer->Delete();
  this->SetSelectionArrayNames(0);
  delete this->Internal;
  delete this->Implementation;
}

//----------------------------------------------------------------------------
bool vtkView::IsItemPresent(vtkDataRepresentation* rep)
{
  unsigned int i, j;
  for( i = 0; i < this->Implementation->Ports.size(); i++ )
  {
    for( j = 0; j < this->Implementation->Ports[i].size(); j++ )
    {
      if( this->Implementation->Ports[i][j] == rep )
      {
        return true;
      }
    }
  }
      
  return false;
}

//----------------------------------------------------------------------------
bool vtkView::IsItemPresent(int i, vtkDataRepresentation* rep)
{
  unsigned int j;
  if( !this->CheckPort(i, 0) )
      return false;
  else
  {
    for( j = 0; j < this->Implementation->Ports[i].size(); j++ )
    {
      if( this->Implementation->Ports[i][j] == rep )
      {
        return true;
      }
    }
  }
      
  return false;
}

//----------------------------------------------------------------------------
void vtkView::SizePort(int i, int j)
{
  if( this->Implementation->Ports.size() < (unsigned int)(i+1) )
  {
    this->Implementation->Ports.resize(i+1);
  }
  
  if( this->Implementation->Ports[i].size() < (unsigned int)(j+1) )
  {
    int old_size = this->Implementation->Ports[i].size();
    this->Implementation->Ports[i].resize(j+1);
    for( int k = old_size; k < j+1; k++ )
    {
      this->Implementation->Ports[i][k] = NULL;
    }
  }
}

//----------------------------------------------------------------------------
bool vtkView::CheckPort(int i, int j )
{
  if( this->Implementation->Ports.size() < (unsigned int)(i+1) )
  {
    return false;
  }
  
  if( this->Implementation->Ports[i].size() < (unsigned int)(j+1) )
  {
    return false;
  }
  return true;
}

//----------------------------------------------------------------------------
vtkDataRepresentation* vtkView::AddRepresentationFromInput(vtkDataObject* input)
{
  return this->AddRepresentationFromInputConnection(input->GetProducerPort());
}

//----------------------------------------------------------------------------
vtkDataRepresentation* vtkView::AddRepresentationFromInput(int i, vtkDataObject* input)
{
  return this->AddRepresentationFromInputConnection(i, input->GetProducerPort());
}

//----------------------------------------------------------------------------
vtkDataRepresentation* vtkView::SetRepresentationFromInput(vtkDataObject* input)
{
  return this->SetRepresentationFromInputConnection(input->GetProducerPort());
}

//----------------------------------------------------------------------------
vtkDataRepresentation* vtkView::SetRepresentationFromInput(int i, vtkDataObject* input)
{
  return this->SetRepresentationFromInputConnection(i, input->GetProducerPort());
}

//----------------------------------------------------------------------------
vtkDataRepresentation* vtkView::SetRepresentationFromInput(int i, int j, vtkDataObject* input)
{
  return this->SetRepresentationFromInputConnection(i, j, input->GetProducerPort());
}

//----------------------------------------------------------------------------
vtkDataRepresentation* vtkView::AddRepresentationFromInputConnection(vtkAlgorithmOutput* conn)
{
  vtkDataRepresentation* rep = vtkDataRepresentation::New();
  rep->SetInputConnection(conn);

//NOTE TO JS: Need to remove this next call after testing new functionality.  (I'm leaving it here to preserve current functionality during testing, but this is not correct for the function call being made...)
  this->RemoveAllRepresentations();

  this->AddRepresentation(rep);
  rep->Delete();
  return rep;
}

//----------------------------------------------------------------------------
vtkDataRepresentation* vtkView::AddRepresentationFromInputConnection(int i, vtkAlgorithmOutput* conn)
{
  vtkDataRepresentation* rep = vtkDataRepresentation::New();
  rep->SetInputConnection(conn);
  this->AddRepresentation(i, rep);
  rep->Delete();
  return rep;
}

//----------------------------------------------------------------------------
vtkDataRepresentation* vtkView::SetRepresentationFromInputConnection(vtkAlgorithmOutput* conn)
{
  vtkDataRepresentation* rep = vtkDataRepresentation::New();
  rep->SetInputConnection(conn);
  this->SetRepresentation(rep);
  rep->Delete();
  return rep;
}

//----------------------------------------------------------------------------
vtkDataRepresentation* vtkView::SetRepresentationFromInputConnection(int i, vtkAlgorithmOutput* conn)
{
  vtkDataRepresentation* rep = vtkDataRepresentation::New();
  rep->SetInputConnection(conn);
  this->SetRepresentation(i, rep);
  rep->Delete();
  return rep;
}

//----------------------------------------------------------------------------
vtkDataRepresentation* vtkView::SetRepresentationFromInputConnection(int i, int j, vtkAlgorithmOutput* conn)
{
  vtkDataRepresentation* rep = vtkDataRepresentation::New();
  rep->SetInputConnection(conn);
  this->SetRepresentation(i, j, rep);
  rep->Delete();
  return rep;
}

//----------------------------------------------------------------------------
void vtkView::AddRepresentation(vtkDataRepresentation* rep)
{
  this->AddRepresentation( 0, rep );
}

//----------------------------------------------------------------------------
void vtkView::AddRepresentation(int i, vtkDataRepresentation* rep)
{
  if( !this->CheckPort( i, 0 ) )
  {
    this->SetRepresentation(i, 0, rep);
  }
  else
  {
    if( !this->IsItemPresent(i, rep) )
    {
      if( rep->AddToView( this ) )
      {
        rep->AddObserver(vtkCommand::SelectionChangedEvent, this->GetObserver());
        this->AddInputConnection(rep->GetInputConnection(), rep->GetSelectionConnection());

        VTK_CREATE(vtkDataRepresentation, new_rep);
        
        int port_length = this->Implementation->Ports[i].size();
        this->SizePort( i, port_length );
        this->Implementation->Ports[i][port_length] = new_rep;
      }
    }
  }
}

//----------------------------------------------------------------------------
void vtkView::SetRepresentation(vtkDataRepresentation* rep)
{
  this->RemoveAllRepresentations();
  this->AddRepresentation(rep);
}

//----------------------------------------------------------------------------
void vtkView::SetRepresentation(int i, vtkDataRepresentation* rep)
{ 
  this->RemoveAllRepresentations(i);
  this->AddRepresentation(i, rep);
}

//----------------------------------------------------------------------------
void vtkView::SetRepresentation(int i, int j, vtkDataRepresentation* rep)
{ 
  vtkDataRepresentation* old_rep = NULL;
  if( this->CheckPort( i, j ) )
  {
    old_rep = this->Implementation->Ports[i][j];
  }

  if( old_rep != rep )
  {
    if( rep->AddToView( this ) )
    {
      if( old_rep != NULL )
      {
        old_rep->RemoveFromView( this );
        old_rep->RemoveObserver(this->GetObserver());
        this->RemoveInputConnection(old_rep->GetInputConnection(), old_rep->GetSelectionConnection());
      }
      
      rep->AddObserver(vtkCommand::SelectionChangedEvent, this->GetObserver());
      this->AddInputConnection(rep->GetInputConnection(), rep->GetSelectionConnection());
      this->SizePort(i, j);
      this->Implementation->Ports[i][j] = rep;
    }
  }
}

//----------------------------------------------------------------------------
void vtkView::RemoveRepresentation(vtkDataRepresentation* rep)
{
  if (this->IsItemPresent(rep))
    {
    rep->RemoveFromView(this);
    rep->RemoveObserver(this->GetObserver());
    this->RemoveInputConnection(rep->GetInputConnection(), rep->GetSelectionConnection());
    this->RemoveItem(rep);
    }
}

//----------------------------------------------------------------------------
void vtkView::RemoveItem(vtkDataRepresentation* rep)
{
  unsigned int i;
  for( i = 0; i < this->Implementation->Ports.size(); i++ )
  {
    vtkstd::vector<vtkSmartPointer<vtkDataRepresentation> >::iterator port_iter = this->Implementation->Ports[i].begin();
    while( port_iter != this->Implementation->Ports[i].end() )
    {
      if( *port_iter == rep )
      {
        this->Implementation->Ports[i].erase( port_iter );
        break;
      }
      ++port_iter;
    }
  }
}      
  
//----------------------------------------------------------------------------
void vtkView::RemoveRepresentation(vtkAlgorithmOutput* conn)
{
  unsigned int i, j;
  for( i = 0; i < this->Implementation->Ports.size(); i++ )
  {
    for( j = 0; j < this->Implementation->Ports[i].size(); j++ )
    {
      vtkDataRepresentation* rep = this->Implementation->Ports[i][j];
      
      if (rep->GetInputConnection() == conn)
      {
        this->RemoveRepresentation(rep);
      }
    }
  }
}

//----------------------------------------------------------------------------
void vtkView::RemoveAllRepresentations()
{
  unsigned int i, j;
  for( i = 0; i < this->Implementation->Ports.size(); i++ )
  {
    for( j = 0; j < this->Implementation->Ports[i].size(); j++ )
    {
      vtkDataRepresentation* rep = this->Implementation->Ports[i][j];
      this->RemoveRepresentation(rep);
    }
    
    while( this->Implementation->Ports[i].size() > 0 )
    {
      this->Implementation->Ports[i].pop_back();
    }
  }
  
  while( this->Implementation->Ports.size() > 0 )
  {
    this->Implementation->Ports.pop_back();
  }
}

//----------------------------------------------------------------------------
void vtkView::RemoveAllRepresentations(int i)
{
  if( !this->CheckPort(i, 0) )
      return;
  
  unsigned int j;
  for( j = 0; j < this->Implementation->Ports[i].size(); j++ )
  {
    vtkDataRepresentation* rep = this->Implementation->Ports[i][j];
    this->RemoveRepresentation(rep);
  }
    
  while( this->Implementation->Ports[i].size() > 0 )
  {
    this->Implementation->Ports[i].pop_back();
  }
}

//----------------------------------------------------------------------------
int vtkView::GetNumberOfRepresentations()
{
  int counter = 0;
  if( this->CheckPort(0,0) )
  {
    counter = this->Implementation->Ports[0].size();
  }
  return counter;
}

//----------------------------------------------------------------------------
int vtkView::GetNumberOfRepresentations(int i)
{
  if( this->Implementation->Ports.size() > (unsigned int)i )
  {
    return this->Implementation->Ports[i].size();
  }
  return 0;
}

//----------------------------------------------------------------------------
vtkDataRepresentation* vtkView::GetRepresentation(int index)
{
  if( this->CheckPort( 0, index ) )
  {
    return this->Implementation->Ports[0][index];
  }
  return NULL;
}

//----------------------------------------------------------------------------
vtkDataRepresentation* vtkView::GetRepresentation(int i, int j)
{
  if( this->CheckPort( i, j ) )
  {
    return this->Implementation->Ports[i][j];
  }
  return NULL;
}

//----------------------------------------------------------------------------
void vtkView::SetSelectionArrayName(const char* name)
{
  if (!this->SelectionArrayNames)
    {
    this->SelectionArrayNames = vtkStringArray::New();
    }
  this->SelectionArrayNames->Initialize();
  this->SelectionArrayNames->InsertNextValue(name);
}

//----------------------------------------------------------------------------
const char* vtkView::GetSelectionArrayName()
{
  if (this->SelectionArrayNames &&
      this->SelectionArrayNames->GetNumberOfTuples() > 0)
    {
    return this->SelectionArrayNames->GetValue(0);
    }
  return 0;
}

//----------------------------------------------------------------------------
vtkCommand* vtkView::GetObserver()
{
  return this->Observer;
}

//----------------------------------------------------------------------------
void vtkView::ProcessEvents(vtkObject* caller, unsigned long eventId, 
  void* callData)
{
  vtkDataRepresentation* caller_rep = vtkDataRepresentation::SafeDownCast( caller );
  if (this->IsItemPresent(caller_rep) && eventId == vtkCommand::SelectionChangedEvent)
    {
    this->InvokeEvent(vtkCommand::SelectionChangedEvent);
    }

  if (eventId == vtkCommand::ProgressEvent)
    {
    vtkstd::map<vtkObject*, vtkstd::string>::iterator iter = 
      this->Internal->RegisteredProgress.find(caller);
    if (iter != this->Internal->RegisteredProgress.end())
      {
      ViewProgressEventCallData eventdata(iter->second.c_str(),
        *(reinterpret_cast<const double*>(callData)));
      this->InvokeEvent(vtkCommand::ViewProgressEvent, &eventdata);
      }
    }
}

//----------------------------------------------------------------------------
void vtkView::RegisterProgress(vtkObject* algorithm, const char* message/*=NULL*/)
{
  if (algorithm)
    {
    const char* used_message = message? message : algorithm->GetClassName();
    this->Internal->RegisteredProgress[algorithm] = used_message;
    algorithm->AddObserver(vtkCommand::ProgressEvent, this->Observer);
    }
}

//----------------------------------------------------------------------------
void vtkView::UnRegisterProgress(vtkObject* algorithm)
{
  if (algorithm)
    {
    vtkstd::map<vtkObject*, vtkstd::string>::iterator iter = 
      this->Internal->RegisteredProgress.find(algorithm);
    if (iter != this->Internal->RegisteredProgress.end())
      {
      this->Internal->RegisteredProgress.erase(iter);
      algorithm->RemoveObservers(vtkCommand::ProgressEvent, this->Observer);
      }
    }
}

//----------------------------------------------------------------------------
void vtkView::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "SelectionType: " << this->SelectionType << endl;
  os << indent << "SelectionArrayNames: " << (this->SelectionArrayNames ? "" : "(null)") << endl;
  if (this->SelectionArrayNames)
    {
    this->SelectionArrayNames->PrintSelf(os, indent.GetNextIndent());
    }
}
