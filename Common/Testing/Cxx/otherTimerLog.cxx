/*==========================================================================

  Program: 
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  ==========================================================================*/

// .NAME 
// .SECTION Description
// this program tests the TimerLog

#include <stdio.h>
#include "vtkTimerLog.h"
#include "vtkDebugLeaks.h"

void Test(ostream& strm)
{
  // actual test
  float a = 1.0, b = 2.0;
  int i, j;
  strm << "Test vtkTimerLog Start" << endl;
  vtkTimerLog *timer1 = vtkTimerLog::New();

  timer1->SetMaxEntries (3);
  timer1->StartTimer();
  for (j = 0; j < 10; j++)
    {
    timer1->FormatAndMarkEvent("%s%d", "start", j);
    for (i = 0; i < 10000000; i++)
      {
      a *= b;
      }
#ifndef WIN32
    sleep (1);
#else
    Sleep(1000);
#endif
    timer1->FormatAndMarkEvent("%s%d", "end", j);
    }
  timer1->StopTimer();
  strm << *timer1;
  strm << "GetElapsedTime: " << timer1->GetElapsedTime() << endl;
  strm << "GetCPUTime: " << timer1->GetCPUTime() << endl;
  timer1->DumpLog( "timing" );
  timer1->ResetLog ();
  unlink("timing");
  timer1->Delete();
  strm << "Test vtkTimerLog End" << endl;
}


int main(int argc, char* argv[])
{
  vtkDebugLeaks::PromptUserOff();

  Test(cout);

  return 0;
} 
