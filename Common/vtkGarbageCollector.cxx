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
#include "vtkGarbageCollector.h"

#include <vtkstd/queue>
#include <vtkstd/set>
#include <vtkstd/stack>
#include <vtkstd/vector>

vtkCxxRevisionMacro(vtkGarbageCollector, "$Revision$");

//----------------------------------------------------------------------------
class vtkGarbageCollectorInternals
{
public:
  // Store garbage collection entries keyed by object.
  struct Entry
  {
    Entry(vtkObjectBase* obj): Object(obj), Root(), InComponent(0),
                               VisitOrder(0), Queued(0), References() {}

    // The object corresponding to this entry.
    vtkObjectBase* Object;

    // The candidate root for the component containing this object.
    Entry* Root;

    // Mark whether the object has been assigned to a component.
    int InComponent;

    // Mark the order in which object's are visited by Tarjan's algorithm.
    int VisitOrder;

    // Whether this entry has been queued while computing net reference count.
    int Queued;

    // The list of references reported by this entry's object.
    typedef vtkstd::vector<Entry*> ReferencesType;
    ReferencesType References;
  };

  // Order entries by object pointer for quick lookup.
  struct EntryCompare
  {
    std::less<vtkObjectBase*> Compare;
    vtkstd_bool operator()(Entry* l, Entry* r) const
      { return Compare(l->Object, r->Object); }
  };

  // The set of objects that have been visited.
  typedef vtkstd::set<Entry*, EntryCompare> EntriesBase;
  struct EntriesType: public EntriesBase
  {
    typedef EntriesBase::iterator iterator;
    Entry* Find(vtkObjectBase* obj)
      {
      Entry e(obj);
      iterator i = this->find(&e);
      return (i != end())? *i : 0;
      }
    ~EntriesType() { for(iterator i=begin(); i != end(); ++i) { delete *i; } }
  };
  EntriesType Entries;

  // The main garbage collector instance.
  vtkGarbageCollector* External;

  // The stack of objects forming the connected components.
  typedef vtkstd::stack<Entry*> StackType;
  StackType Stack;

  // The object currently being explored.
  Entry* Current;

  // Count for visit order.
  int Count;

  // The objects in the root's connected component.
  typedef vtkstd::vector<vtkObjectBase*> ComponentType;
  ComponentType Component;

  // Find the strongly connected components reachable from the given root.
  Entry* FindStrongComponents(vtkObjectBase*);

  // Callback from objects to report references.
  void ReportReference(vtkObjectBase*);

  // Node visitor for Tarjan's algorithm.
  Entry* VisitTarjan(vtkObjectBase*);

  // Find/Print/Delete the set of objects in the root's strongly
  // connected component.
  int FindComponent(Entry*);
  void PrintComponent(ostream& os);
  void DeleteComponent();
};

//----------------------------------------------------------------------------
vtkGarbageCollector::vtkGarbageCollector(vtkGarbageCollectorInternals* internal)
{
  this->Internal = internal;
  this->Internal->External = this;
  this->Internal->Current = 0;
}

//----------------------------------------------------------------------------
vtkGarbageCollector::~vtkGarbageCollector()
{
  this->SetReferenceCount(0);
}

//----------------------------------------------------------------------------
void vtkGarbageCollector::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
void vtkGarbageCollector::Register(vtkObjectBase*)
{
}

//----------------------------------------------------------------------------
void vtkGarbageCollector::UnRegister(vtkObjectBase*)
{
}

//----------------------------------------------------------------------------
void vtkGarbageCollector::Check(vtkObjectBase* root)
{
  // Allocate as much as possible on the runtime stack.  This code
  // runs every time UnRegister is called on an object supporting
  // garbage collection.  It may be worth modifying the stack and map
  // to use a local array for storage until the size grows beyond some
  // constant.
  vtkGarbageCollectorInternals internal;
  vtkGarbageCollector collector(&internal);

  // Uncomment these lines to get reference loop debugging for objects
  // with Debug flag set:
  //if(vtkObject* obj = vtkObject::SafeDownCast(root))
  //  {
  //  collector.SetDebug(obj->GetDebug());
  //  }

  collector.SetDebug(1);

  // Do collection if necessary.
  collector.CheckReferenceLoops(root);

  // Avoid destruction message.
  collector.SetDebug(0);
}

//----------------------------------------------------------------------------
#ifdef VTK_LEAN_AND_MEAN
void vtkGarbageCollector::ReportReference(vtkObjectBase* obj, const char*)
{
  // Forward call to the internal implementation.
  if(obj)
    {
    this->Internal->ReportReference(obj);
    }
}
#else
void vtkGarbageCollector::ReportReference(vtkObjectBase* obj, const char* desc)
{
  if(obj)
    {
    // Report debugging information if requested.
    if(this->Debug && vtkObject::GetGlobalWarningDisplay())
      {
      vtkObjectBase* current = this->Internal->Current->Object;
      ostrstream msg;
      msg << "ReportReference: "
          << current->GetClassName() << "(" << current << ") "
          << (desc?desc:"")
          << " -> " << obj->GetClassName() << "(" << obj << ")";
      msg << ends;
      vtkDebugMacro(<< msg.str());
      msg.rdbuf()->freeze(0);
      }

    // Forward call to the internal implementation.
    this->Internal->ReportReference(obj);
    }
}
#endif

//----------------------------------------------------------------------------
void vtkGarbageCollector::ForwardReportReferences(vtkObjectBase* obj)
{
#ifndef VTK_LEAN_AND_MEAN
  // Report debugging information if requested.
  if(this->Debug && vtkObject::GetGlobalWarningDisplay())
    {
    vtkObjectBase* current = this->Internal->Current->Object;
    vtkDebugMacro("Requesting references from "
                  << current->GetClassName() << "("
                  << current << ") with reference count "
                  << current->GetReferenceCount());
    }
#endif
  obj->ReportReferences(this);
}

//----------------------------------------------------------------------------
void vtkGarbageCollector::ForwardRemoveReferences(vtkObjectBase* obj)
{
  obj->RemoveReferences();
}

//----------------------------------------------------------------------------
void vtkGarbageCollector::ForwardGarbageCollectionStarting(vtkObjectBase* obj)
{
  obj->GarbageCollectionStarting();
}

//----------------------------------------------------------------------------
void vtkGarbageCollector::ForwardGarbageCollectionFinishing(vtkObjectBase* obj)
{
  obj->GarbageCollectionFinishing();
}

//----------------------------------------------------------------------------
void vtkGarbageCollector::CheckReferenceLoops(vtkObjectBase* root)
{
  // This traverses the reference graph associated with the given root
  // object.  If the total reference count of the strongly connected
  // component is 0 when not counting internal references, the entire
  // component is deleted.

  vtkDebugMacro("Starting reference graph walk with root "
                << root->GetClassName() << "(" << root << ")");

  // Find the strongly connected components reachable from this root.
  vtkGarbageCollectorInternals::Entry*
    rootEntry = this->Internal->FindStrongComponents(root);

  vtkDebugMacro("Finished reference graph walk with root "
                << root->GetClassName() << "(" << root << ")");

  // Find the net reference count of the component containing the root.
  int netCount = this->Internal->FindComponent(rootEntry);

#ifndef VTK_LEAN_AND_MEAN
  if(this->Debug && vtkObject::GetGlobalWarningDisplay())
    {
    ostrstream msg;
    msg << "Identified strongly connected component with net reference count "
        << netCount << ":";
    this->Internal->PrintComponent(msg);
    msg << ends;
    vtkDebugMacro(<< msg.str());
    msg.rdbuf()->freeze(0);
    }
#endif

  // If the net reference count is zero, delete the component.
  if(netCount == 0)
    {
    vtkDebugMacro("Deleting strongly connected component of reference graph.");
    this->Internal->DeleteComponent();
    }
}

//----------------------------------------------------------------------------
int vtkGarbageCollectorInternals::FindComponent(Entry* root)
{
  // The queue of objects while checking the net reference count.
  typedef vtkstd::queue<Entry*> QueueType;
  QueueType entryQueue;

  // Initialize the queue with the root object.
  int netCount = root->Object->GetReferenceCount();
  this->Component.push_back(root->Object);
  root->Queued = 1;
  entryQueue.push(root);

  // Loop until the queue is empty.
  while(!entryQueue.empty())
    {
    // Get the next object from the queue.
    Entry* v = entryQueue.front();
    entryQueue.pop();

    // Process the references to objects in the component.
    for(Entry::ReferencesType::iterator r = v->References.begin();
        r != v->References.end(); ++r)
      {
      Entry* w = *r;
      if(w->Root == root)
        {
        if(!w->Queued)
          {
          // Include the references to this object.
          netCount += w->Object->GetReferenceCount();

          // Add the object to the list of objects in the component.
          this->Component.push_back(w->Object);

          // Queue the object.
          w->Queued = 1;
          entryQueue.push(w);
          }

        // This is an internal reference, so decrement the net count.
        --netCount;
        }
      }
    }

  return netCount;
}

//----------------------------------------------------------------------------
vtkGarbageCollectorInternals::Entry*
vtkGarbageCollectorInternals::FindStrongComponents(vtkObjectBase* root)
{
  // Use Tarjan's algorithm to visit the reference graph and mark
  // strongly connected components.
  this->Count = 0;
  return this->VisitTarjan(root);
}

//----------------------------------------------------------------------------
void vtkGarbageCollectorInternals::ReportReference(vtkObjectBase* obj)
{
  // Get the source and destination of this reference.
  Entry* v = this->Current;
  Entry* w = this->Entries.Find(obj);

  // Visit the destination of this reference if it has not been visited.
  if(!w)
    {
    w = this->VisitTarjan(obj);
    }

  // If the destination has not yet been assigned to a component,
  // check if it is a better potential root for the current object.
  if(!w->InComponent)
    {
    if(w->Root->VisitOrder < v->Root->VisitOrder)
      {
      v->Root = w->Root;
      }
    }

  // Save this reference.
  v->References.push_back(w);
}

//----------------------------------------------------------------------------
vtkGarbageCollectorInternals::Entry*
vtkGarbageCollectorInternals::VisitTarjan(vtkObjectBase* obj)
{
  // Create an entry for the object.
  Entry* v = new Entry(obj);
  this->Entries.insert(v);

  // Initialize the entry and push it onto the stack of graph nodes.
  v->Root = v;
  v->InComponent = 0;
  v->VisitOrder = ++this->Count;
  this->Stack.push(v);

  // Process the references from this node.
  Entry* saveCurrent = this->Current;
  this->Current = v;
  this->External->ForwardReportReferences(v->Object);
  this->Current = saveCurrent;

  // If we have found a component mark its members.
  if(v->Root == v)
    {
    Entry* w;
    do
      {
      w = this->Stack.top();
      this->Stack.pop();
      w->InComponent = 1;
      w->Root = v;
      } while(w != v);
    }

  return v;
}

//----------------------------------------------------------------------------
void vtkGarbageCollectorInternals::PrintComponent(ostream& os)
{
  for(ComponentType::iterator i = this->Component.begin();
      i != this->Component.end(); ++i)
    {
    os << "\n  " << (*i)->GetClassName() << "(" << *i << ")";
    }
}

//----------------------------------------------------------------------------
void vtkGarbageCollectorInternals::DeleteComponent()
{
  ComponentType::iterator obj;

  // Notify all objects they are about to be garbage collected.
  // They will disable reference loop checking.
  for(obj = this->Component.begin(); obj != this->Component.end(); ++obj)
    {
    vtkGarbageCollector::ForwardGarbageCollectionStarting(*obj);
    }

  // Disconnect the reference graph.
  for(obj = this->Component.begin(); obj != this->Component.end(); ++obj)
    {
    vtkGarbageCollector::ForwardRemoveReferences(*obj);
    }

  // Notify all objects they have been garbage collected.  They will
  // delete themselves.
  for(obj = this->Component.begin(); obj != this->Component.end(); ++obj)
    {
    vtkGarbageCollector::ForwardGarbageCollectionFinishing(*obj);
    }
}
