package require -exact vtkrendering 5.4

if {[info commands ::vtk::init::require_package] != ""} {
  if {[::vtk::init::require_package vtkVolumeRenderingTCL 5.4]} {
    package provide vtkvolumerendering 5.4
  }
} else {
  if {[info commands vtkEncodedGradientShader] != "" ||
    [::vtk::load_component vtkVolumeRenderingTCL] == ""} {
    package provide vtkvolumerendering 5.4
  }
}
