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

#include "vtkContextScene.h"

#include "vtkContextItem.h"
#include "vtkContext2D.h"
#include "vtkTransform2D.h"
#include "vtkMatrix3x3.h"

// Get my new commands
#include "vtkCommand.h"

#include "vtkAnnotationLink.h"
#include "vtkInteractorStyle.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderWindow.h"
#include "vtkInteractorStyleRubberBand2D.h"
#include "vtkObjectFactory.h"

// My STL containers
#include <vtkstd/vector>

//-----------------------------------------------------------------------------
// Minimal command class to handle callbacks.
class vtkContextScene::Command : public vtkCommand
{
public:
  Command(vtkContextScene *scene) { this->Target = scene; }

  virtual void Execute(vtkObject *caller, unsigned long eventId, void *callData)
    {
    if (this->Target)
      {
      vtkInteractorStyle *style = vtkInteractorStyle::SafeDownCast(caller);
      vtkRenderWindowInteractor *interactor =
          vtkRenderWindowInteractor::SafeDownCast(style->GetInteractor());

      int x = interactor->GetEventPosition()[0];
      int y = interactor->GetEventPosition()[1];

      switch (eventId)
        {
        case vtkCommand::MouseMoveEvent :
          this->Target->MouseMoveEvent(x, y);
          break;
        case vtkCommand::LeftButtonPressEvent :
          this->Target->ButtonPressEvent(0, x, y);
          break;
        case vtkCommand::MiddleButtonPressEvent :
          this->Target->ButtonPressEvent(1, x, y);
          break;
        case vtkCommand::RightButtonPressEvent :
          this->Target->ButtonPressEvent(2, x, y);
          break;
        case vtkCommand::LeftButtonReleaseEvent :
          this->Target->ButtonReleaseEvent(0, x, y);
          break;
        case vtkCommand::MiddleButtonReleaseEvent :
          this->Target->ButtonReleaseEvent(1, x, y);
          break;
        case vtkCommand::RightButtonReleaseEvent :
          this->Target->ButtonReleaseEvent(2, x, y);
          break;
        case vtkCommand::SelectionChangedEvent :
          this->Target->ProcessSelectionEvent(caller, callData);
          break;
        default:
          this->Target->ProcessEvents(caller, eventId, callData);
        }
      }
    }

  void SetTarget(vtkContextScene* t) { this->Target = t; }

  vtkContextScene *Target;
};

//-----------------------------------------------------------------------------
// Minimal storage class for STL containers etc.
class vtkContextScene::Private
{
public:
  vtkstd::vector<vtkContextItem *> items;
  vtkstd::vector<bool> itemState;
  int itemMousePressCurrent; // Index of the item with a current mouse down
  vtkContextMouseEvent Event; // Mouse event structure
};

//-----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkContextScene, "$Revision$");
vtkStandardNewMacro(vtkContextScene);
vtkCxxSetObjectMacro(vtkContextScene, AnnotationLink, vtkAnnotationLink);
vtkCxxSetObjectMacro(vtkContextScene, Window, vtkRenderWindow);

//-----------------------------------------------------------------------------
vtkContextScene::vtkContextScene()
{
  this->Observer = new vtkContextScene::Command(this);
  this->Storage = new Private;
  this->Storage->itemMousePressCurrent = -1;
  this->Storage->Event.Button = -1;
  this->AnnotationLink = NULL;
  this->Window = NULL;
}

//-----------------------------------------------------------------------------
vtkContextScene::~vtkContextScene()
{
  delete this->Observer;
  this->Observer = 0;
  delete this->Storage;
  this->Storage = 0;
}

//-----------------------------------------------------------------------------
bool vtkContextScene::Paint(vtkContext2D *painter)
{
  vtkDebugMacro("Paint event called.");
  unsigned int size = this->Storage->items.size();
  for (unsigned int i = 0; i < size; ++i)
    {
    painter->SetTransform(this->Storage->items[i]->GetTransform());
    this->Storage->items[i]->Paint(painter);
    }
  return true;
}

//-----------------------------------------------------------------------------
void vtkContextScene::AddItem(vtkContextItem *item)
{
  this->Storage->items.push_back(item);
  this->Storage->itemState.push_back(false);
}

//-----------------------------------------------------------------------------
int vtkContextScene::NumberOfItems()
{
  return this->Storage->items.size();
}

//-----------------------------------------------------------------------------
vtkContextItem * vtkContextScene::GetItem(int index)
{
  if (index < this->NumberOfItems())
    {
    return this->Storage->items[index];
    }
}

//-----------------------------------------------------------------------------
void vtkContextScene::SetInteractorStyle(vtkInteractorStyle *interactor)
{
  cout << "Interactor style " << interactor << " " << interactor->GetClassName() << endl;
  interactor->AddObserver(vtkCommand::SelectionChangedEvent, this->Observer);
  interactor->AddObserver(vtkCommand::AnyEvent, this->Observer);
}

//-----------------------------------------------------------------------------
void vtkContextScene::ProcessEvents(vtkObject* caller, unsigned long eventId,
                             void*)
{
  vtkDebugMacro("ProcessEvents called! " << caller->GetClassName() << "\t"
      << vtkCommand::GetStringFromEventId(eventId)
      << "\n\t" << vtkInteractorStyleRubberBand2D::SafeDownCast(caller)->GetInteraction());
}

//-----------------------------------------------------------------------------
void vtkContextScene::ProcessSelectionEvent(vtkObject* caller, void* callData)
{
  cout << "ProcessSelectionEvent called! " << caller << "\t" << callData << endl;
  unsigned int *rect = reinterpret_cast<unsigned int *>(callData);
  cout << "Rect:";
  for (int i = 0; i < 5; ++i)
    {
    cout << "\t" << rect[i];
    }
  cout << endl;
}

//-----------------------------------------------------------------------------
void vtkContextScene::MouseMoveEvent(int x, int y)
{
  unsigned int size = this->Storage->items.size();
  vtkContextMouseEvent &event = this->Storage->Event;
  event.ScreenPos[0] = x;
  event.ScreenPos[1] = y;
  event.ScenePos[0] = x;
  event.ScenePos[1] = y;

  // Check if there is a selected item that needs to receive a move event
  if (this->Storage->itemMousePressCurrent >= 0)
    {
    this->PerformTransform(
        this->Storage->items[this->Storage->itemMousePressCurrent]->GetTransform(),
        event);
    this->Storage->items[this->Storage->itemMousePressCurrent]->MouseMoveEvent(event);
    }
  for (int i = size-1; i >= 0; --i)
    {
    if (this->Storage->itemMousePressCurrent == i)
      {
      // Don't send the mouse move event twice...
      continue;
      }
    this->PerformTransform(this->Storage->items[i]->GetTransform(), event);
    if (this->Storage->items[i]->Hit(event))
      {
      if (!this->Storage->itemState[i] && this->Storage->itemMousePressCurrent < 0)
        {
        this->Storage->itemState[i] = true;
        this->Storage->items[i]->MouseEnterEvent(event);
        }
      }
    else
      {
      if (this->Storage->itemState[i])
        {
        this->Storage->itemState[i] = false;
        this->Storage->items[i]->MouseLeaveEvent(event);
        }
      }
    }

  // Update the last positions now
  event.LastScreenPos[0] = event.ScreenPos[0];
  event.LastScreenPos[1] = event.ScreenPos[1];
  event.LastScenePos[0] = event.ScenePos[0];
  event.LastScenePos[1] = event.ScenePos[1];

  if (this->Window)
    {
    this->Window->Render();
    }
}

//-----------------------------------------------------------------------------
void vtkContextScene::ButtonPressEvent(int button, int x, int y)
{
  unsigned int size = this->Storage->items.size();
  vtkContextMouseEvent &event = this->Storage->Event;
  event.ScreenPos[0] = event.LastScreenPos[0] = x;
  event.ScreenPos[1] = event.LastScreenPos[1] = y;
  event.ScenePos[0] = event.LastScenePos[0] = x;
  event.ScenePos[1] = event.LastScenePos[1] = y;
  event.Button = button;
  for (int i = size-1; i >= 0; --i)
    {
    this->PerformTransform(this->Storage->items[i]->GetTransform(), event);
    if (this->Storage->items[i]->Hit(event))
      {
      if (this->Storage->items[i]->MouseButtonPressEvent(event))
        {
        // The event was accepted - stop propagating
        this->Storage->itemMousePressCurrent = i;
        return;
        }
      }
    }
}

//-----------------------------------------------------------------------------
void vtkContextScene::ButtonReleaseEvent(int button, int x, int y)
{
  if (this->Storage->itemMousePressCurrent >= 0)
    {
    vtkContextMouseEvent &event = this->Storage->Event;
    event.ScreenPos[0] = x;
    event.ScreenPos[1] = y;
    event.ScenePos[0] = x;
    event.ScenePos[1] = y;
    event.Button = button;
    this->PerformTransform(
        this->Storage->items[this->Storage->itemMousePressCurrent]->GetTransform(),
        event);
    this->Storage->items[this->Storage->itemMousePressCurrent]->MouseButtonReleaseEvent(event);
    this->Storage->itemMousePressCurrent = -1;
    event.Button = -1;
    }
}

//-----------------------------------------------------------------------------
inline void vtkContextScene::PerformTransform(vtkTransform2D *transform,
                                              vtkContextMouseEvent &mouse)
{
  if (transform)
    {
    transform->InverseTransformPoints(&mouse.ScenePos[0], &mouse.Pos[0], 1);
    }
  else
    {
    mouse.Pos[0] = mouse.ScenePos[0];
    mouse.Pos[1] = mouse.ScenePos[1];
    }
}

//-----------------------------------------------------------------------------
void vtkContextScene::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  // Print out the chart's geometry if it has been set
  os << indent << "Widthxheight: " << this->Geometry[0] << "\t" << this->Geometry[1]
     << endl;
}
