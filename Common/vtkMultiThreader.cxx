/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


Copyright (c) 1993-1998 Ken Martin, Will Schroeder, Bill Lorensen.

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
#include "vtkMultiThreader.h"

// These are the includes necessary for multithreaded rendering on an SGI
// using the sproc() call
#ifdef VTK_USE_SPROC
#include <sys/resource.h>
#include <sys/prctl.h>
#include <wait.h>
#endif

#ifdef VTK_USE_PTHREADS
#include <pthread.h>
#endif

// Initialize static member that controls global maximum number of threads
static int vtkMultiThreaderGlobalMaximumNumberOfThreads = 0;

void vtkMultiThreader::SetGlobalMaximumNumberOfThreads(int val)
{
  if (val == vtkMultiThreaderGlobalMaximumNumberOfThreads) return;
  vtkMultiThreaderGlobalMaximumNumberOfThreads = val;
}

int vtkMultiThreader::GetGlobalMaximumNumberOfThreads()
{
  return vtkMultiThreaderGlobalMaximumNumberOfThreads;
}

// Description:
// Constructor. Default all the methods to NULL. Since the
// ThreadInfoArray is static, the ThreadIDs can be initialized here
// and will not change.
vtkMultiThreader::vtkMultiThreader()
{
  int i;

  for ( i = 0; i < VTK_MAX_THREADS; i++ )
    {
    this->ThreadInfoArray[i].ThreadID           = i;
    this->ThreadInfoArray[i].ActiveFlag         = NULL;
    this->ThreadInfoArray[i].ActiveFlagLock     = NULL;
    this->MultipleMethod[i]                     = NULL;
    this->SpawnedThreadActiveFlag[i]            = 0;
    this->SpawnedThreadActiveFlagLock[i]        = NULL;
    this->SpawnedThreadInfoArray[i].ThreadID    = 0;
    }

  this->SingleMethod = NULL;

#ifdef VTK_USE_SPROC
  // Default the number of threads to be the number of available
  // processors if we are using sproc()
  this->NumberOfThreads             = prctl( PR_MAXPPROCS );
#endif

#ifdef VTK_USE_PTHREADS
  // Default the number of threads to be the number of available
  // processors if we are using pthreads()
#ifdef _SC_NPROCESSORS_ONLN
  this->NumberOfThreads             = sysconf( _SC_NPROCESSORS_ONLN );
#else
  this->NumberOfThreads             = 1;
#endif
#endif

#ifdef _WIN32
  {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    this->NumberOfThreads = sysInfo.dwNumberOfProcessors;
  }
#endif

#ifndef _WIN32
#ifndef VTK_USE_SPROC
#ifndef VTK_USE_PTHREADS
  // If we are not multithreading, the number of threads should
  // always be 1
  this->NumberOfThreads             = 1;
#endif  
#endif  
#endif
}

// Description:
// Destructor. Nothing allocated so nothing needs to be done here.
vtkMultiThreader::~vtkMultiThreader()
{
}

// Description:
// Set the user defined method that will be run on NumberOfThreads threads
// when SingleMethodExecute is called.
void vtkMultiThreader::SetSingleMethod( vtkThreadFunctionType f, 
					void *data )
{ 
  this->SingleMethod = f;
  this->SingleData   = data;
}

// Description:
// Set one of the user defined methods that will be run on NumberOfThreads
// threads when MultipleMethodExecute is called. This method should be
// called with index = 0, 1, ..,  NumberOfThreads-1 to set up all the
// required user defined methods
void vtkMultiThreader::SetMultipleMethod( int index, 
					  vtkThreadFunctionType f, void *data )
{ 
  // You can only set the method for 0 through NumberOfThreads-1
  if ( index >= this->NumberOfThreads ) {
    vtkErrorMacro( << "Can't set method " << index << 
    " with a thread count of " << this->NumberOfThreads );
    }
  else
    {
    this->MultipleMethod[index] = f;
    this->MultipleData[index]   = data;
    }
}

// Description:
// Execute the method set as the SingleMethod on NumberOfThreads threads.
void vtkMultiThreader::SingleMethodExecute()
{
  int                thread_loop;

#ifdef _WIN32
  DWORD              threadId;
  HANDLE             process_id[VTK_MAX_THREADS];
#endif

#ifdef VTK_USE_SPROC
  siginfo_t          info_ptr;
  int                process_id[VTK_MAX_THREADS];
#endif

#ifdef VTK_USE_PTHREADS
  pthread_t          process_id[VTK_MAX_THREADS];
#endif

  if ( !this->SingleMethod )
    {
    vtkErrorMacro( << "No single method set!" );
    return;
    }

  // obey the global maximum number of threads limit
  if (vtkMultiThreaderGlobalMaximumNumberOfThreads &&
      this->NumberOfThreads > vtkMultiThreaderGlobalMaximumNumberOfThreads)
    {
    this->NumberOfThreads = vtkMultiThreaderGlobalMaximumNumberOfThreads;
    }
  
    
  // We are using sproc (on SGIs), pthreads(on Suns), or a single thread
  // (the default)  

#ifdef _WIN32
  // Using CreateThread on a PC
  //
  // We want to use CreateThread to start this->NumberOfThreads - 1 
  // additional threads which will be used to call this->SingleMethod().
  // The parent thread will also call this routine.  When it is done,
  // it will wait for all the children to finish. 
  //
  // First, start up the this->NumberOfThreads-1 processes.  Keep track
  // of their process ids for use later in the waitid call
  for (thread_loop = 1; thread_loop < this->NumberOfThreads; thread_loop++ )
    {
    this->ThreadInfoArray[thread_loop].UserData    = this->SingleData;
    this->ThreadInfoArray[thread_loop].NumberOfThreads = this->NumberOfThreads;
    process_id[thread_loop] = 
      CreateThread(NULL, 0, this->SingleMethod, 
	     ((void *)(&this->ThreadInfoArray[thread_loop])), 0, &threadId);
    if (process_id == NULL)
      {
      vtkErrorMacro("Error in thread creation !!!");
      } 
    }
  
  // Now, the parent thread calls this->SingleMethod() itself
  this->ThreadInfoArray[0].UserData = this->SingleData;
  this->ThreadInfoArray[0].NumberOfThreads = this->NumberOfThreads;
  this->SingleMethod((void *)(&this->ThreadInfoArray[0]));

  // The parent thread has finished this->SingleMethod() - so now it
  // waits for each of the other processes to exit
  for ( thread_loop = 1; thread_loop < this->NumberOfThreads; thread_loop++ )
    {
    WaitForSingleObject(process_id[thread_loop], INFINITE);
    }

  // close the threads
  for ( thread_loop = 1; thread_loop < this->NumberOfThreads; thread_loop++ )
    {
    CloseHandle(process_id[thread_loop]);
    }
#endif

#ifdef VTK_USE_SPROC
  // Using sproc() on an SGI
  //
  // We want to use sproc to start this->NumberOfThreads - 1 additional
  // threads which will be used to call this->SingleMethod(). The
  // parent thread will also call this routine.  When it is done,
  // it will wait for all the children to finish. 
  //
  // First, start up the this->NumberOfThreads-1 processes.  Keep track
  // of their process ids for use later in the waitid call
  for ( thread_loop = 1; thread_loop < this->NumberOfThreads; thread_loop++ )
    {
    this->ThreadInfoArray[thread_loop].UserData    = this->SingleData;
    this->ThreadInfoArray[thread_loop].NumberOfThreads = this->NumberOfThreads;
    process_id[thread_loop] = 
      sproc( this->SingleMethod, PR_SADDR, 
	     ( (void *)(&this->ThreadInfoArray[thread_loop]) ) );
    }
  
  // Now, the parent thread calls this->SingleMethod() itself
  this->ThreadInfoArray[0].UserData = this->SingleData;
  this->ThreadInfoArray[0].NumberOfThreads = this->NumberOfThreads;
  this->SingleMethod((void *)(&this->ThreadInfoArray[0]) );

  // The parent thread has finished this->SingleMethod() - so now it
  // waits for each of the other processes to exit
  for ( thread_loop = 1; thread_loop < this->NumberOfThreads; thread_loop++ )
    {
    waitid( P_PID, (id_t) process_id[thread_loop], &info_ptr, WEXITED );
    }
#endif

#ifdef VTK_USE_PTHREADS
  // Using POSIX threads
  //
  // We want to use pthread_create to start this->NumberOfThreads-1 additional
  // threads which will be used to call this->SingleMethod(). The
  // parent thread will also call this routine.  When it is done,
  // it will wait for all the children to finish. 
  //
  // First, start up the this->NumberOfThreads-1 processes.  Keep track
  // of their process ids for use later in the pthread_join call

  pthread_attr_t attr;

#ifdef VTK_HP_PTHREADS
  pthread_attr_create( &attr );
#else  
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
#endif
  
  for ( thread_loop = 1; thread_loop < this->NumberOfThreads; thread_loop++ )
    {
    this->ThreadInfoArray[thread_loop].UserData    = this->SingleData;
    this->ThreadInfoArray[thread_loop].NumberOfThreads = this->NumberOfThreads;

#ifdef VTK_HP_PTHREADS
    pthread_create( &(process_id[thread_loop]),
		    attr, this->SingleMethod,  
		    ( (void *)(&this->ThreadInfoArray[thread_loop]) ) );
#else
    pthread_create( &(process_id[thread_loop]),
		    &attr, this->SingleMethod,  
		    ( (void *)(&this->ThreadInfoArray[thread_loop]) ) );
#endif
    }
  
  // Now, the parent thread calls this->SingleMethod() itself
  this->ThreadInfoArray[0].UserData = this->SingleData;
  this->ThreadInfoArray[0].NumberOfThreads = this->NumberOfThreads;
  this->SingleMethod((void *)(&this->ThreadInfoArray[0]) );

  // The parent thread has finished this->SingleMethod() - so now it
  // waits for each of the other processes to exit
  for ( thread_loop = 1; thread_loop < this->NumberOfThreads; thread_loop++ )
    {
    pthread_join( process_id[thread_loop], NULL );
    }
#endif

#ifndef _WIN32
#ifndef VTK_USE_SPROC
#ifndef VTK_USE_PTHREADS
  // There is no multi threading, so there is only one thread.
  this->ThreadInfoArray[0].UserData    = this->SingleData;
  this->ThreadInfoArray[0].NumberOfThreads = this->NumberOfThreads;
  this->SingleMethod( (void *)(&this->ThreadInfoArray[0]) );
#endif
#endif
#endif
}

void vtkMultiThreader::MultipleMethodExecute()
{
  int                thread_loop;

#ifdef _WIN32
  DWORD              threadId;
  HANDLE             process_id[VTK_MAX_THREADS];
#endif

#ifdef VTK_USE_SPROC
  siginfo_t          info_ptr;
  int                process_id[VTK_MAX_THREADS];
#endif

#ifdef VTK_USE_PTHREADS
  pthread_t          process_id[VTK_MAX_THREADS];
#endif


  // obey the global maximum number of threads limit
  if (vtkMultiThreaderGlobalMaximumNumberOfThreads &&
      this->NumberOfThreads > vtkMultiThreaderGlobalMaximumNumberOfThreads)
    {
    this->NumberOfThreads = vtkMultiThreaderGlobalMaximumNumberOfThreads;
    }

  for ( thread_loop = 0; thread_loop < this->NumberOfThreads; thread_loop++ )
    if ( this->MultipleMethod[thread_loop] == (vtkThreadFunctionType)NULL)
      {
      vtkErrorMacro( << "No multiple method set for: " << thread_loop );
      return;
      }

  // We are using sproc (on SGIs), pthreads(on Suns), CreateThread
  // on a PC or a single thread (the default)  

#ifdef _WIN32
  // Using CreateThread on a PC
  //
  // We want to use CreateThread to start this->NumberOfThreads - 1 
  // additional threads which will be used to call the NumberOfThreads-1
  // methods defined in this->MultipleMethods[](). The parent thread
  // will call this->MultipleMethods[NumberOfThreads-1]().  When it is done,
  // it will wait for all the children to finish. 
  //
  // First, start up the this->NumberOfThreads-1 processes.  Keep track
  // of their process ids for use later in the waitid call
  for ( thread_loop = 1; thread_loop < this->NumberOfThreads; thread_loop++ )
    {
    this->ThreadInfoArray[thread_loop].UserData = 
      this->MultipleData[thread_loop];
    this->ThreadInfoArray[thread_loop].NumberOfThreads = this->NumberOfThreads;
    process_id[thread_loop] = 
      CreateThread(NULL, 0, this->MultipleMethod[thread_loop], 
	     ((void *)(&this->ThreadInfoArray[thread_loop])), 0, &threadId);
    if (process_id == NULL)
      {
      vtkErrorMacro("Error in thread creation !!!");
      } 
    }
  
  // Now, the parent thread calls the last method itself
  this->ThreadInfoArray[0].UserData = this->MultipleData[0];
  this->ThreadInfoArray[0].NumberOfThreads = this->NumberOfThreads;
  (this->MultipleMethod[0])((void *)(&this->ThreadInfoArray[0]) );

  // The parent thread has finished its method - so now it
  // waits for each of the other processes (created with sproc) to
  // exit
  for ( thread_loop = 1; thread_loop < this->NumberOfThreads; thread_loop++ )
    {
    WaitForSingleObject(process_id[thread_loop], INFINITE);
    }

  // close the threads
  for ( thread_loop = 1; thread_loop < this->NumberOfThreads; thread_loop++ )
    {
    CloseHandle(process_id[thread_loop]);
    }
#endif

#ifdef VTK_USE_SPROC
  // Using sproc() on an SGI
  //
  // We want to use sproc to start this->NumberOfThreads - 1 additional
  // threads which will be used to call the NumberOfThreads-1 methods
  // defined in this->MultipleMethods[](). The parent thread
  // will call this->MultipleMethods[NumberOfThreads-1]().  When it is done,
  // it will wait for all the children to finish. 
  //
  // First, start up the this->NumberOfThreads-1 processes.  Keep track
  // of their process ids for use later in the waitid call
  for ( thread_loop = 1; thread_loop < this->NumberOfThreads; thread_loop++ )
    {
    this->ThreadInfoArray[thread_loop].UserData = 
      this->MultipleData[thread_loop];
    this->ThreadInfoArray[thread_loop].NumberOfThreads = this->NumberOfThreads;
    process_id[thread_loop] = 
      sproc( this->MultipleMethod[thread_loop], PR_SADDR, 
	     ( (void *)(&this->ThreadInfoArray[thread_loop]) ) );
    }
  
  // Now, the parent thread calls the last method itself
  this->ThreadInfoArray[0].UserData = this->MultipleData[0];
  this->ThreadInfoArray[0].NumberOfThreads = this->NumberOfThreads;
  (this->MultipleMethod[0])((void *)(&this->ThreadInfoArray[0]) );

  // The parent thread has finished its method - so now it
  // waits for each of the other processes (created with sproc) to
  // exit
  for ( thread_loop = 1; thread_loop < this->NumberOfThreads; thread_loop++ )
    {
    waitid( P_PID, (id_t) process_id[thread_loop], &info_ptr, WEXITED );
    }
#endif

#ifdef VTK_USE_PTHREADS
  // Using POSIX threads
  //
  // We want to use pthread_create to start this->NumberOfThreads - 1 
  // additional
  // threads which will be used to call the NumberOfThreads-1 methods
  // defined in this->MultipleMethods[](). The parent thread
  // will call this->MultipleMethods[NumberOfThreads-1]().  When it is done,
  // it will wait for all the children to finish. 
  //
  // First, start up the this->NumberOfThreads-1 processes.  Keep track
  // of their process ids for use later in the pthread_join call

  pthread_attr_t attr;

#ifdef VTK_HP_PTHREADS
  pthread_attr_create( &attr );
#else  
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
#endif

  for ( thread_loop = 1; thread_loop < this->NumberOfThreads; thread_loop++ )
    {
    this->ThreadInfoArray[thread_loop].UserData = 
      this->MultipleData[thread_loop];
    this->ThreadInfoArray[thread_loop].NumberOfThreads = this->NumberOfThreads;
#ifdef VTK_HP_PTHREADS
    pthread_create( &(process_id[thread_loop]),
		    attr, this->MultipleMethod[thread_loop],  
		    ( (void *)(&this->ThreadInfoArray[thread_loop]) ) );
#else
    pthread_create( &(process_id[thread_loop]),
		    &attr, this->MultipleMethod[thread_loop],  
		    ( (void *)(&this->ThreadInfoArray[thread_loop]) ) );
#endif
    }
  
  // Now, the parent thread calls the last method itself
  this->ThreadInfoArray[0].UserData = this->MultipleData[0];
  this->ThreadInfoArray[0].NumberOfThreads = this->NumberOfThreads;
  (this->MultipleMethod[0])((void *)(&this->ThreadInfoArray[0]) );

  // The parent thread has finished its method - so now it
  // waits for each of the other processes to exit
  for ( thread_loop = 1; thread_loop < this->NumberOfThreads; thread_loop++ )
    {
    pthread_join( process_id[thread_loop], NULL );
    }
#endif

#ifndef _WIN32
#ifndef VTK_USE_SPROC
#ifndef VTK_USE_PTHREADS
  // There is no multi threading, so there is only one thread.
  this->ThreadInfoArray[0].UserData    = this->MultipleData[0];
  this->ThreadInfoArray[0].NumberOfThreads = this->NumberOfThreads;
  (this->MultipleMethod[0])( (void *)(&this->ThreadInfoArray[0]) );
#endif
#endif
#endif
}

int vtkMultiThreader::SpawnThread( vtkThreadFunctionType f, void *UserData )
{
  int id;

#ifdef _WIN32
  DWORD              threadId;
#endif

  id = 0;

  while ( id < VTK_MAX_THREADS )
    {
    if ( this->SpawnedThreadActiveFlagLock[id] == NULL ) break;
    id++;
    }

  if ( id >= VTK_MAX_THREADS )
    {
    vtkErrorMacro( << "You have too many active threads!" );
    return -1;
    }

  this->SpawnedThreadActiveFlagLock[id] = vtkMutexFunctionLock::New();
  vtkMutexLockFuncMacro(this->SpawnedThreadActiveFlagLock[id],this->SpawnedThreadActiveFlag[id]=1);

  this->SpawnedThreadInfoArray[id].UserData        = UserData;
  this->SpawnedThreadInfoArray[id].NumberOfThreads = 1;
  this->SpawnedThreadInfoArray[id].ActiveFlag = 
    &this->SpawnedThreadActiveFlag[id];
  this->SpawnedThreadInfoArray[id].ActiveFlagLock = 
    this->SpawnedThreadActiveFlagLock[id];

  // We are using sproc (on SGIs), pthreads(on Suns or HPs), 
  // CreateThread (on win32), or generating an error  

#ifdef _WIN32
  // Using CreateThread on a PC
  //
  this->SpawnedThreadProcessID[id] = 
      CreateThread(NULL, 0, f, 
	     ((void *)(&this->SpawnedThreadInfoArray[id])), 0, &threadId);
  if (this->SpawnedThreadProcessID[id] == NULL)
    {
    vtkErrorMacro("Error in thread creation !!!");
    } 
#endif

#ifdef VTK_USE_SPROC
  // Using sproc() on an SGI
  //
  this->SpawnedThreadProcessID[id] = 
    sproc( f, PR_SADDR, ( (void *)(&this->SpawnedThreadInfoArray[id]) ) );

#endif

#ifdef VTK_USE_PTHREADS
  // Using POSIX threads
  //
  pthread_attr_t attr;

#ifdef VTK_HP_PTHREADS
  pthread_attr_create( &attr );
#else  
  pthread_attr_init(&attr);
  pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
#endif
  
#ifdef VTK_HP_PTHREADS
  pthread_create( &(this->SpawnedThreadProcessID[id]),
		  attr, f,  
		  ( (void *)(&this->SpawnedThreadInfoArray[id]) ) );
#else
  pthread_create( &(this->SpawnedThreadProcessID[id]),
		  &attr, f,  
		  ( (void *)(&this->SpawnedThreadInfoArray[id]) ) );
#endif

#endif

#ifndef _WIN32
#ifndef VTK_USE_SPROC
#ifndef VTK_USE_PTHREADS
  // There is no multi threading, so there is only one thread.
  // This won't work - so give an error message.
  vtkErrorMacro( << "Cannot spawn thread in a single threaded environment!" );
  this->SpawnedThreadActiveFlagLock[id]->Delete();
  id = -1;
#endif
#endif
#endif

  return id;
}

void vtkMultiThreader::TerminateThread( int ThreadID )
{

  if ( !this->SpawnedThreadActiveFlag[ThreadID] ) {
    return;
  }

  vtkMutexLockFuncMacro(this->SpawnedThreadActiveFlagLock[ThreadID],this->SpawnedThreadActiveFlag[ThreadID]=0);


#ifdef _WIN32
  WaitForSingleObject(this->SpawnedThreadProcessID[ThreadID], INFINITE);
  CloseHandle(this->SpawnedThreadProcessID[ThreadID]);
#endif

#ifdef VTK_USE_SPROC
  siginfo_t info_ptr;

  waitid( P_PID, (id_t) this->SpawnedThreadProcessID[ThreadID], 
	  &info_ptr, WEXITED );
#endif

#ifdef VTK_USE_PTHREADS
  pthread_join( this->SpawnedThreadProcessID[ThreadID], NULL );
#endif

#ifndef _WIN32
#ifndef VTK_USE_SPROC
#ifndef VTK_USE_PTHREADS
  // There is no multi threading, so there is only one thread.
  // This won't work - so give an error message.
  vtkErrorMacro(<< "Cannot terminate thread in single threaded environment!");
#endif
#endif
#endif

  this->SpawnedThreadActiveFlagLock[ThreadID]->Delete();
  this->SpawnedThreadActiveFlagLock[ThreadID] = NULL;

}

// Description:
// Print method for the multithreader
void vtkMultiThreader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os,indent); 

  os << indent << "Thread Count: " << this->NumberOfThreads << "\n";
  os << indent << "Global Maximum Number Of Threads: " << 
    vtkMultiThreaderGlobalMaximumNumberOfThreads << endl;

}
