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
// .NAME vtkObject - abstract base class for most VTK objects
// .SECTION Description
// vtkObject is the base class for most objects in the visualization
// toolkit. vtkObject provides methods for tracking modification time,
// debugging, printing, and event callbacks. Most objects created
// within the VTK framework should be a subclass of vtkObject or one
// of its children.  The few exceptions tend to be very small helper
// classes that usually never get instantiated or situations where
// multiple inheritance gets in the way.  vtkObject also performs
// reference counting: objects that are reference counted exist as
// long as another object uses them. Once the last reference to a
// reference counted object is removed, the object will spontaneously
// destruct.

// .SECTION Caveats
// Note: in VTK objects should always be created with the New() method
// and deleted with the Delete() method. VTK objects cannot be
// allocated off the stack (i.e., automatic objects) because the
// constructor is a protected method.

// .SECTION See also
// vtkCommand vtkTimeStamp

#ifndef __vtkObject_h
#define __vtkObject_h

#include "vtkObjectBase.h"
#include "vtkSetGet.h"
#include "vtkTimeStamp.h"

class vtkSubjectHelper;
class vtkCommand;

class VTK_COMMON_EXPORT vtkObject : public vtkObjectBase
{
public:
//BTX
  typedef vtkObjectBase Superclass;
//ETX

  // Description:
  // Return the class name as a string. This method is defined
  // in all subclasses of vtkObject with the vtkTypeRevisionMacro found
  // in vtkSetGet.h.
  virtual const char *GetClassName() const {return "vtkObject";};

  // Description:
  // Return 1 if this class type is the same type of (or a subclass of)
  // the named class. Returns 0 otherwise. This method works in
  // combination with vtkTypeRevisionMacro found in vtkSetGet.h.
  static int IsTypeOf(const char *name);

  // Description:
  // Return 1 if this class is the same type of (or a subclass of)
  // the named class. Returns 0 otherwise. This method works in
  // combination with vtkTypeRevisionMacro found in vtkSetGet.h.
  virtual int IsA(const char *name);

  // Description:
  // Will cast the supplied object to vtkObject* is this is a safe operation
  // (i.e., a safe downcast); otherwise NULL is returned. This method is
  // defined in all subclasses of vtkObject with the vtkTypeRevisionMacro 
  // found in vtkSetGet.h.
  static vtkObject *SafeDownCast(vtkObject *o);

  // Description:
  // Create an object with Debug turned off, modified time initialized 
  // to zero, and reference counting on.
  static vtkObject *New();

#ifdef _WIN32
  // avoid dll boundary problems
  void* operator new( size_t tSize );
  void operator delete( void* p );
#endif 
  
  // Description:
  // Turn debugging output on.
  virtual void DebugOn();

  // Description:
  // Turn debugging output off.
  virtual void DebugOff();
  
  // Description:
  // Get the value of the debug flag.
  unsigned char GetDebug();
  
  // Description:
  // Set the value of the debug flag. A non-zero value turns debugging on.
  void SetDebug(unsigned char debugFlag);
  
  // Description:
  // This method is called when vtkErrorMacro executes. It allows 
  // the debugger to break on error.
  static void BreakOnError();
  
  // Description:
  // Update the modification time for this object. Many filters rely on
  // the modification time to determine if they need to recompute their
  // data. The modification time is a unique monotonically increasing
  // unsigned long integer.
  virtual void Modified();
  
  // Description: 
  // Return this object's modified time.
  virtual unsigned long GetMTime();

  // Description:
  // Methods invoked by print to print information about the object
  // including superclasses. Typically not called by the user (use
  // Print() instead) but used in the hierarchical print process to
  // combine the output of several classes.
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // This is a global flag that controls whether any debug, warning
  // or error messages are displayed.
  static void SetGlobalWarningDisplay(int val);
  static void GlobalWarningDisplayOn(){vtkObject::SetGlobalWarningDisplay(1);};
  static void GlobalWarningDisplayOff() 
    {vtkObject::SetGlobalWarningDisplay(0);};
  static int  GetGlobalWarningDisplay();
  
  // Description:
  // Increase the reference count (mark as used by another object).
  virtual void Register(vtkObjectBase* o);

  // Description:
  // Decrease the reference count (release by another object). This has
  // the same effect as invoking Delete() (i.e., it reduces the reference
  // count by 1).
  virtual void UnRegister(vtkObjectBase* o);

  // Description:
  // Allow people to add/remove/invoke observers (callbacks) to any VTK
  // object.  This is an implementation of the subject/observer design
  // pattern. An observer is added by specifying an event to respond to
  // and a vtkCommand to execute. It returns an unsigned long tag which
  // can be used later to remove the event or retrieve the command.
  // When events are invoked, the observers are called in the order they
  // were added. If a priority value is specified, then the higher 
  // priority commands are called first. A command may set an abort
  // flag to stop processing of the event. (See vtkCommand.h for more
  // information.)
  //BTX
  unsigned long AddObserver(unsigned long event, vtkCommand *, 
                            float priority=0.0);
  unsigned long AddObserver(const char *event, vtkCommand *, 
                            float priority=0.0);
  vtkCommand *GetCommand(unsigned long tag);
  void InvokeEvent(unsigned long event, void *callData);
  void InvokeEvent(const char *event, void *callData);
  void RemoveObserver(vtkCommand*);
  //ETX
  void InvokeEvent(unsigned long event) { this->InvokeEvent(event, NULL); };
  void InvokeEvent(const char *event) { this->InvokeEvent(event, NULL); };
  void RemoveObserver(unsigned long tag);
  void RemoveObservers(unsigned long event);
  void RemoveObservers(const char *event);
  int HasObserver(unsigned long event);
  int HasObserver(const char *event);
  
protected:
  vtkObject(); 
  virtual ~vtkObject(); 

  virtual void CollectRevisions(ostream& os);
  
  unsigned char Debug;     // Enable debug messages
  vtkTimeStamp MTime;      // Keep track of modification time
  vtkSubjectHelper *SubjectHelper;

private:
  vtkObject(const vtkObject&);  // Not implemented.
  void operator=(const vtkObject&);  // Not implemented.
};

#endif

