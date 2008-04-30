package require -exact vtkwidgets 5.3

if {[info commands ::vtk::init::require_package] != ""} {
  if {[::vtk::init::require_package vtkInfovisTCL 5.3]} {
    package provide vtkinfovis 5.3
  }
} else {
  if {[info commands vtkGraphLayout] != "" ||
    [::vtk::load_component vtkInfovisTCL] == ""} {
    package provide vtkinfovis 5.3
  }
}
