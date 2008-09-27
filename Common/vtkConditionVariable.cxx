#include "vtkConditionVariable.h"

#include "vtkObjectFactory.h"
#ifdef VTK_USE_WIN32_THREADS
# include "vtkWindows.h"
#endif

#include <errno.h>

vtkStandardNewMacro(vtkConditionVariable);
vtkCxxRevisionMacro(vtkConditionVariable,"$Revision$");

#ifndef EPERM
#  define EPERM 1
#endif
#ifndef ENOMEM
#  define ENOMEM 12
#endif
#ifndef EBUSY
#  define EBUSY 16
#endif
#ifndef EINVAL
#  define EINVAL 22
#endif
#ifndef EAGAIN
#  define EAGAIN 35
#endif

#if ! defined(VTK_USE_PTHREADS) && ! defined(VTK_HP_PTHREADS) && ! defined(VTK_USE_WIN32_THREADS)
typedef int pthread_condattr_t;

int pthread_cond_init( vtkConditionType* cv, const pthread_condattr_t* )
{
  *cv = 0;
  return 0;
}

int pthread_cond_destroy( vtkConditionType* cv )
{
  if ( *cv )
    return EBUSY;
  return 0;
}

int pthread_cond_signal( vtkConditionType* cv )
{
  *cv = 1;
}

int pthread_cond_broadcast( vtkConditionType* cv )
{
  *cv = 1;
}

int pthread_cond_wait( vtkConditionType* cv, vtkMutexType* lock )
{
#ifdef VTK_USE_SPROC
  release_lock( lock );
#else // VTK_USE_SPROC
  *lock = 0;
#endif // VTK_USE_SPROC
  while ( ! *cv );
#ifdef VTK_USE_SPROC
  spin_lock( lock );
#else // VTK_USE_SPROC
  *lock = 1;
#endif // VTK_USE_SPROC
}
#endif // ! defined(VTK_USE_PTHREADS) && ! defined(VTK_HP_PTHREADS) && ! defined(VTK_USE_WIN32_THREADS)

#ifdef VTK_USE_WIN32_THREADS
typedef int pthread_condattr_t;

int pthread_cond_init( pthread_cond_t* cv, const pthread_condattr_t* )
{
  cv->WaitingThreadCount = 0;
  cv->WasBroadcast = 0;
  cv->Semaphore = CreatedSemaphore(
    NULL,       // no security
    0,          // initially 0
    0x7fffffff, // max count
    NULL );     // unnamed 
  InitializeCriticalSection( &cv->WaitingThreadCountLock );
  cv->DoneWaiting = CreateEvent(
    NULL,   // no security
    FALSE,  // auto-reset
    FALSE,  // non-signaled initially
    NULL ); // unnamed

  return 0;
}

int pthread_cond_wait( pthread_cond_t* cv, vtkMutexType* external_mutex )
{
  // Avoid race conditions.
  EnterCriticalSection( &cv->WaitingThreadCountLock );
  ++ cv->WaitingThreadCount;
  LeaveCriticalSection( &cv->WaitingThreadCountLock );

  // This call atomically releases the mutex and waits on the
  // semaphore until <pthread_cond_signal> or <pthread_cond_broadcast>
  // are called by another thread.
  SignalObjectAndWait( *external_mutex, cv->Semaphore, INFINITE, FALSE );

  // Reacquire lock to avoid race conditions.
  EnterCriticalSection( &cv->WaitingThreadCountLock );

  // We're no longer waiting...
  -- cv->WaitingThreadCount;

  // Check to see if we're the last waiter after <pthread_cond_broadcast>.
  int last_waiter = cv->WasBroadcast && cv->WaitingThreadCount == 0;

  LeaveCriticalSection( &cv->WaitingThreadCountLock );

  // If we're the last waiter thread during this particular broadcast
  // then let all the other threads proceed.
  if ( last_waiter )
    {
    // This call atomically signals the <DoneWaiting> event and waits until
    // it can acquire the <external_mutex>.  This is required to ensure fairness. 
    SignalObjectAndWait( cv->DoneWaiting, *external_mutex, INFINITE, FALSE );
    }
  else
    {
    // Always regain the external mutex since that's the guarantee we
    // give to our callers. 
    WaitForSingleObject( *external_mutex );
    }
  return 0;
}

int pthread_cond_signal( pthread_cond_t* cv )
{
  EnterCriticalSection( &cv->WaitingThreadCountLock );
  int have_waiters = cv->WaitingThreadCount > 0;
  LeaveCriticalSection( &cv->WaitingThreadCountLock );

  // If there aren't any waiters, then this is a no-op.  
  if ( have_waiters )
    {
    ReleaseSemaphore( cv->Semaphore, 1, 0 );
    }
  return 0;
}

int pthread_cond_broadcast( pthread_cond_t* cv )
{
  // This is needed to ensure that <WaitingThreadCount> and <WasBroadcast> are
  // consistent relative to each other.
  EnterCriticalSection( &cv->WaitingThreadCountLock );
  int have_waiters = 0;

  if ( cv->WaitingThreadCount > 0 )
    {
    // We are broadcasting, even if there is just one waiter...
    // Record that we are broadcasting, which helps optimize
    // <pthread_cond_wait> for the non-broadcast case.
    cv->WasBroadcast = 1;
    have_waiters = 1;
    }

  if (have_waiters)
    {
    // Wake up all the waiters atomically.
    ReleaseSemaphore( cv->Semaphore, cv->WaitingThreadCount, 0 );
    LeaveCriticalSection( &cv->WaitingThreadCountLock );

    // Wait for all the awakened threads to acquire the counting semaphore. 
    WaitForSingleObject( cv->DoneWaiting, INFINITE );
    // This assignment is okay, even without the <WaitingThreadCountLock> held 
    // because no other waiter threads can wake up to access it.
    cv->WasBroadcast = 0;
    }
  else
    {
    LeaveCriticalSection( &cv->WaitingThreadCountLock );
    }

  return 0;
}

int pthread_cond_destroy( pthread_cond_t* cv )
{
  if ( cv->WaitingThreadCount > 0 && ! cv->DoneWaiting )
    {
    return EBUSY;
    }
  return 0;
}
#endif // VTK_USE_WIN32_THREADS

vtkSimpleConditionVariable::vtkSimpleConditionVariable()
{
  int result = pthread_cond_init( &this->ConditionVariable, 0 );
  switch ( result )
    {
  case EINVAL:
      {
      vtkGenericWarningMacro( "Invalid condition variable attributes." );
      }
    break;
  case ENOMEM:
      {
      vtkGenericWarningMacro( "Not enough memory to create a condition variable." );
      }
    break;
  case EAGAIN:
      {
      vtkGenericWarningMacro( "Temporarily not enough memory to create a condition variable." );
      }
    break;
    }
}

vtkSimpleConditionVariable::~vtkSimpleConditionVariable()
{
  int result = pthread_cond_destroy( &this->ConditionVariable );
  switch ( result )
    {
  case EINVAL:
      {
      vtkGenericWarningMacro( "Could not destroy condition variable (invalid value)" );
      }
    break;
  case EBUSY:
      {
      vtkGenericWarningMacro( "Could not destroy condition variable (locked by another thread)" );
      }
    break;
    }
}

void vtkSimpleConditionVariable::Signal()
{
  pthread_cond_signal( &this->ConditionVariable );
}

void vtkSimpleConditionVariable::Broadcast()
{
  pthread_cond_broadcast( &this->ConditionVariable );
}

void vtkSimpleConditionVariable::Wait( vtkSimpleMutexLock& lock )
{
  pthread_cond_wait( &this->ConditionVariable, &lock.MutexLock );
}

void vtkConditionVariable::PrintSelf( ostream& os, vtkIndent indent )
{
  os << indent << "SimpleConditionVariable: " << &this->SimpleConditionVariable << "\n";
}

