
# setup some common things for testing
vtkObject rtTempObject;
rtTempObject GlobalWarningDisplayOff;
vtkMath rtExMath
rtExMath RandomSeed 6
vtkDebugLeaks rtDebugLeaks
rtDebugLeaks PromptUserOff

# create the testing class to do the work
vtkTesting rtTester
for {set i 1} {$i < [expr $argc - 1]} {incr i} {
   rtTester AddArgument "[lindex $argv $i]"
}
set VTK_DATA_ROOT [rtTester GetDataRoot]

for {set i  1} {$i < [expr $argc - 1]} {incr i} {
   if {[lindex $argv $i] == "-A"} {
      foreach dir [split [lindex $argv [expr $i +1]] ":"] {
         lappend auto_path $dir
      }
   }
}

# load in the script
set file [lindex $argv 0]

# set the default threshold, the Tcl script may change this
set threshold -1

source $file
if {[info commands iren] == "iren"} {renWin Render}
# run the event loop quickly to map any tkwidget windows
wm withdraw .
update


# current directory
if {[rtTester IsValidImageSpecified] != 0} {
   # look for a renderWindow ImageWindow or ImageViewer
   # first check for some common names
   if {[info commands renWin] == "renWin"} {
      rtTester SetRenderWindow renWin
      if {$threshold == -1} {
         set threshold 10
      }
   } else {
      if {$threshold == -1} {
         set threshold 5
      }
      if {[info commands viewer] == "viewer"} {
         rtTester SetRenderWindow [viewer GetRenderWindow]
         viewer Render
      } else {
         if {[info commands imgWin] == "imgWin"} {
            rtTester SetRenderWindow imgWin
            imgWin Render
         } else {
            if {[info exists viewer]} {
               rtTester SetRenderWindow [$viewer GetRenderWindow]
            }
         }
      }
   }
   rtTester RegressionTest $threshold
#   puts [rtTester GetLastResultText]
}

vtkCommand DeleteAllObjects
catch {destroy .top}
catch {destroy .geo}

exit 0
