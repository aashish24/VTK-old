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
// .NAME vtkTimerLog - Timer support and logging
// .SECTION Description
// vtkTimerLog contains walltime and cputime measurements associated
// with a given event.  These results can be later analyzed when
// "dumping out" the table.
//
// In addition, vtkTimerLog allows the user to simply get the current
// time, and to start/stop a simple timer separate from the timing
// table logging.

#ifndef __vtkTimerLog_h
#define __vtkTimerLog_h

#include <stdio.h>
#include <fstream.h>

#ifdef _WIN32
#include <winsock.h>  // for Sleep?
#include <sys/types.h>
#include <sys/timeb.h>
#else
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/times.h>
#endif

// var args
#ifndef _WIN32
#include <unistd.h>
#endif
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

// select stuff here is for sleep method
#ifndef NO_FD_SET
#   define SELECT_MASK fd_set
#else
#   ifndef _AIX
	typedef long fd_mask;
#   endif
#   if defined(_IBMR2)
#	define SELECT_MASK void
#   else
#	define SELECT_MASK int
#   endif
#endif


#include "vtkObject.h"
#define VTK_LOG_EVENT_LENGTH 40

//BTX
typedef struct
{
  float WallTime;
  int CpuTicks;
  char Event[VTK_LOG_EVENT_LENGTH];
} vtkTimerLogEntry;
//ETX

class VTK_EXPORT vtkTimerLog : public vtkObject 
{
public:
  static vtkTimerLog *New();

  const char *GetClassName() {return "vtkTimerLog";};
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set/Get the maximum number of entries allowed in the timer log
  static void SetMaxEntries(int a);
  static int  GetMaxEntries();

//BTX
  // Description:
  // Record a timing event.  The event is represented by a formatted
  // string.
  static void FormatAndMarkEvent(char *EventString, ...);
//ETX
  
  // Description:
  // Write the timing table out to a file.  Calculate some helpful
  // statistics (deltas and  percentages) in the process.
  static void DumpLog(char *filename);

  // Description:
  // Record a timing event and capture walltime and cputicks.
  static void MarkEvent(char *EventString);

  // Description:
  // Clear the timing table.  walltime and cputime will also be set
  // to zero when the first new event is recorded.
  static void ResetLog();

  // Description:
  // Allocate timing table with MaxEntries elements.
  static void AllocateLog();

  // Description:
  // Returns the elapsed number of seconds since January 1, 1970. This
  // is also called Universal Coordinated Time.
  static double GetCurrentTime();

  // Description:
  // Returns the CPU time for this process
  // On Win32 platforms this actually returns wall time.
  static double GetCPUTime();

  // Description:
  // Set the StartTime to the current time. Used with GetElapsedTime().
  void StartTimer();

  // Description:
  // Sets EndTime to the current time. Used with GetElapsedTime().
  void StopTimer();

  // Description:
  // Returns the difference between StartTime and EndTime as 
  // a floating point value indicating the elapsed time in seconds.
  double GetElapsedTime();

  // Description:
  // Will be a platform independant sleep.
  // Only works on unix for now.
  static void Sleep(int ms);

protected:
  vtkTimerLog() {}; //insure constructur/destructor protected
  ~vtkTimerLog() {};
  vtkTimerLog(const vtkTimerLog&) {};
  void operator=(const vtkTimerLog&) {};

  static int               MaxEntries;
  static int               NextEntry;
  static int               WrapFlag;
  static int               TicksPerSecond;
  static vtkTimerLogEntry *TimerLog;

#ifdef _WIN32
  static timeb             FirstWallTime;
  static timeb             CurrentWallTime;
#else
  static timeval           FirstWallTime;
  static timeval           CurrentWallTime;
  static tms               FirstCpuTicks;
  static tms               CurrentCpuTicks;
#endif

  // instance variables to support simple timing functionality,
  // separate from timer table logging.
  double StartTime;
  double EndTime;

  //BTX
  static void DumpEntry(ostream& os, int index, float time, float deltatime,
                        int tick, int deltatick, char *event);
  //ETX

};


//
// Set built-in type.  Creates member Set"name"() (e.g., SetVisibility());
//
#define vtkTimerLogMacro(string) \
  { \
      vtkTimerLog::FormatAndMarkEvent("Mark: In %s, line %d, class %s: %s", \
			      __FILE__, __LINE__, this->GetClassName(), string); \
  }

#endif
