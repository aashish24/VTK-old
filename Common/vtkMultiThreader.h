/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1999 Ken Martin, Will Schroeder, Bill Lorensen.

This software is copyrighted by Ken Martin, Will Schroeder and Bill Lorensen.
The following terms apply to all files associated with the software unless
explicitly disclaimed in individual files. This copyright specifically does
not apply to the related textbook "The Visualization Toolkit" ISBN
013199837-4 published by Prentice Hall which is covered by its own copyright.

The authors hereby grant permission to use, copy, and distribute this
software and its documentation for any purpose, provided that existing
copyright notices are retained in all copies and that this notice is included
verbatim in any distributions. Additionally, the authors grant permission to
modify this software and its documentation for any purpose, provided that
such modifications are not distributed without the explicit consent of the
authors and that existing copyright notices are retained in all copies. Some
of the algorithms implemented by this software are patented, observe all
applicable patent law.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================*/
// .NAME vtkMultiThreader - A class for performing multithreaded execution
// .SECTION Description
// vtkMultithreader is a class that provides support for multithreaded
// execution using sproc() on an SGI, or pthread_create on any platform
// supporting POSIX threads.  This class can be used to execute a single
// method on multiple threads, or to specify a method per thread. 

#ifndef __vtkMultiThreader_h
#define __vtkMultiThreader_h

#include "vtkObject.h"
#include "vtkMutexLock.h"

#ifdef VTK_USE_SPROC
#include <sys/types.h>
#include <unistd.h>
#endif

#ifdef VTK_USE_PTHREADS
#include <sys/types.h>
#include <unistd.h>
#endif

// If VTK_USE_SPROC is defined, then sproc() will be used to create
// multiple threads on an SGI. If VTK_USE_PTHREADS is defined, then
// pthread_create() will be used to create multiple threads (on
// a sun, for example)

// The maximum number of threads allowed
#ifdef VTK_USE_SPROC
#define VTK_MAX_THREADS              32
#endif

#ifdef VTK_USE_PTHREADS
#define VTK_MAX_THREADS              32
#endif

#ifdef _WIN32
#define VTK_MAX_THREADS              8
#endif

#ifndef _WIN32
#ifndef VTK_USE_SPROC
#ifndef VTK_USE_PTHREADS
#define VTK_MAX_THREADS              1
#endif
#endif
#endif

// If VTK_USE_PTHREADS is defined, then the multithreaded
// function is of type void *, and returns NULL
// Otherwise the type is void which is correct for WIN32
// and SPROC
//BTX
#ifdef VTK_USE_SPROC
typedef int vtkThreadProcessIDType;
#endif

#ifdef VTK_USE_PTHREADS
typedef void *(*vtkThreadFunctionType)(void *);
typedef pthread_t vtkThreadProcessIDType;
#define VTK_THREAD_RETURN_VALUE  NULL
#define VTK_THREAD_RETURN_TYPE   void *
#endif

#ifdef _WIN32
typedef LPTHREAD_START_ROUTINE vtkThreadFunctionType;
typedef HANDLE vtkThreadProcessIDType;
#define VTK_THREAD_RETURN_VALUE 0
#define VTK_THREAD_RETURN_TYPE DWORD __stdcall
#endif

#ifndef _WIN32
#ifndef VTK_USE_PTHREADS
typedef void (*vtkThreadFunctionType)(void *);
typedef int vtkThreadProcessIDType;
#define VTK_THREAD_RETURN_VALUE
#define VTK_THREAD_RETURN_TYPE void
#endif
#endif
//ETX

// Description:
// This is the structure that is passed to the thread that is
// created from the SingleMethodExecute, MultipleMethodExecute or
// the SpawnThread method. It is passed in as a void *, and it is
// up to the method to cast correctly and extract the information.
// The ThreadID is a number between 0 and NumberOfThreads-1 that indicates
// the id of this thread. The NumberOfThreads is this->NumberOfThreads for
// threads created from SingleMethodExecute or MultipleMethodExecute,
// and it is 1 for threads created from SpawnThread.
// The UserData is the (void *)arg passed into the SetSingleMethod,
// SetMultipleMethod, or SpawnThread method.

//BTX
struct ThreadInfoStruct
{
  int                 ThreadID;
  int                 NumberOfThreads;
  int                 *ActiveFlag;
  vtkMutexLock        *ActiveFlagLock;
  void                *UserData;
};
//ETX

class VTK_EXPORT vtkMultiThreader : public vtkObject 
{
public:
  static vtkMultiThreader *New();

  const char *GetClassName() {return "vtkMultiThreader";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Get/Set the number of threads to create. It will be clamped to the range
  // 1 - VTK_MAX_THREADS, so the caller of this method should check that the
  // requested number of threads was accepted.
  vtkSetClampMacro( NumberOfThreads, int, 1, VTK_MAX_THREADS );
  vtkGetMacro( NumberOfThreads, int );

  // Description:
  // Set/Get the maximum number of threads to use when multithreading.
  // This limits and overrides any other settings for multithreading.
  // A value of zero indicates no limit.
  static void SetGlobalMaximumNumberOfThreads(int val);
  static int  GetGlobalMaximumNumberOfThreads();

  // Description:
  // Set/Get the value which is used to initialize the NumberOfThreads
  // in the constructor.  Initially this default is set to the number of 
  // processors or 8 (which ever is less).
  static void SetGlobalDefaultNumberOfThreads(int val);
  static int  GetGlobalDefaultNumberOfThreads();

  // These methods are excluded from Tcl wrapping 1) because the
  // wrapper barfs on them and 2) because they really shouldn't be
  // called from a script anyway.
  //BTX 
  
  // Description:
  // Execute the SingleMethod (as define by SetSingleMethod) using
  // this->NumberOfThreads threads.
  void SingleMethodExecute();

  // Description:
  // Execute the MultipleMethods (as define by calling SetMultipleMethod
  // for each of the required this->NumberOfThreads methods) using
  // this->NumberOfThreads threads.
  void MultipleMethodExecute();
  
  // Description:
  // Set the SingleMethod to f() and the UserData field of the
  // ThreadInfoStruct that is passed to it will be data.
  // This method (and all the methods passed to SetMultipleMethod)
  // must be of type vtkThreadFunctionType and must take a single argument of
  // type void *.
  void SetSingleMethod(vtkThreadFunctionType, void *data );
 
  // Description:
  // Set the MultipleMethod at the given index to f() and the UserData 
  // field of the ThreadInfoStruct that is passed to it will be data.
  void SetMultipleMethod( int index, vtkThreadFunctionType, void *data ); 

  // Description:
  // Create a new thread for the given function. Return a thread id
  // which is a number between 0 and VTK_MAX_THREADS - 1. This id should
  // be used to kill the thread at a later time.
  int SpawnThread( vtkThreadFunctionType, void *data );

  // Description:
  // Terminate the thread that was created with a SpawnThreadExecute()
  void TerminateThread( int thread_id );


protected:
  vtkMultiThreader();
  ~vtkMultiThreader();
  vtkMultiThreader(const vtkMultiThreader&) {};
  void operator=(const vtkMultiThreader&) {};

  // The number of threads to use
  int                        NumberOfThreads;

  // An array of thread info containing a thread id
  // (0, 1, 2, .. VTK_MAX_THREADS-1), the thread count, and a pointer
  // to void so that user data can be passed to each thread
  ThreadInfoStruct           ThreadInfoArray[VTK_MAX_THREADS];

  // The methods
  vtkThreadFunctionType      SingleMethod;
  vtkThreadFunctionType      MultipleMethod[VTK_MAX_THREADS];

  // Storage of MutexFunctions and ints used to control spawned 
  // threads and the spawned thread ids
  int                        SpawnedThreadActiveFlag[VTK_MAX_THREADS];
  vtkMutexLock               *SpawnedThreadActiveFlagLock[VTK_MAX_THREADS];
  vtkThreadProcessIDType     SpawnedThreadProcessID[VTK_MAX_THREADS];
  ThreadInfoStruct           SpawnedThreadInfoArray[VTK_MAX_THREADS];

//ETX

  // Internal storage of the data
  void                       *SingleData;
  void                       *MultipleData[VTK_MAX_THREADS];

};

#endif





